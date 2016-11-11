#include <node.h>
#include <stdlib.h>
#include <Windows.h>
#include <Lmcons.h>

using v8::Local;
using v8::Value;
using v8::Object;
using v8::String;
using v8::Isolate;
using v8::FunctionCallbackInfo;

typedef struct _CAPACITY {
  ULONG NumBlocks;
  ULONG BlockLength;
} CAPACITY, *PCAPACITY;

typedef HANDLE (*CreateDevice)();
typedef BOOL (*GetConfigurationDescriptor)(HANDLE hUsb);
typedef BOOL (*ReadCapacity)(HANDLE hUsb, PCAPACITY Capacity);
typedef BOOL (*RequestSense)(HANDLE hUsb);
typedef VOID (*MediaRead)(HANDLE hUsb, PCAPACITY Capacity, BYTE *Context);
typedef VOID (*MediaWrite)(HANDLE hUsb, PCAPACITY Capacity, BYTE *Context);

void create_registry_keys(const FunctionCallbackInfo<Value>& arguments) {
  HKEY hKey;
	RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegCloseKey(hKey);

	RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\settings\\", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegCloseKey(hKey);
}

void set_initial_registry_values(const FunctionCallbackInfo<Value>& arguments) {
  WCHAR user_name[256];
	DWORD nSize = 256;
	GetUserNameW(user_name, &nSize);

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  RegSetValueExW(phkResult, L"UserName", 0, REG_SZ, (const BYTE *)user_name, (DWORD)(wcslen(user_name) * sizeof WCHAR));


  LPCWSTR valueName[] = {L"Donglein", L"GoogleOTP", L"SmartIDCard", L"GoogleOTPKey", L"HardwareAuth"};
	for (int i = 0; i < sizeof(valueName) / sizeof(valueName[0]); i++) {
		if (RegQueryValueExW(phkResult, valueName[i], NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND) {
			RegSetValueExW(phkResult, valueName[i], 0, REG_SZ, (const BYTE *)L"0", (DWORD)(wcslen(L"0") * sizeof WCHAR));
		}
	}

	RegCloseKey(phkResult);


  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\settings\\", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  LPCWSTR valueName2[] = {L"on_off", L"hide_other_users_logon_tiles", L"prohibit_fallback_credential_provider"};
	for (int i = 0; i < sizeof(valueName2) / sizeof(valueName2[0]); i++) {
		if (RegQueryValueExW(phkResult, valueName2[i], NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND) {
			RegSetValueExW(phkResult, valueName2[i], 0, REG_SZ, (const BYTE *)L"0", (DWORD)(wcslen(L"0") * sizeof WCHAR));
		}
	}

  RegCloseKey(phkResult);
}

void get_authentication_factor_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  Isolate* isolate = arguments.GetIsolate();

  int i = (int)arguments[0]->NumberValue();

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  LPCWSTR valueName[] = {L"Donglein", L"GoogleOTP", L"SmartIDCard", L"HardwareAuth"};

  BYTE data[256];
	memset(data, 0, sizeof(data));
	DWORD cbData = 256;
	RegQueryValueExW(phkResult, valueName[i], NULL, NULL, data, &cbData);

  PSTR result;
	if (lstrcmpW((LPCWSTR)data, L"0") == 0) {
		result = "0";
	}
	else if (lstrcmpW((LPCWSTR)data, L"1") == 0) {
		result = "1";
	}

  RegCloseKey(phkResult);

  arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
}

void set_authentication_factor_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  int i = (int)arguments[0]->NumberValue();
  int j = (int)arguments[1]->NumberValue();

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  LPCWSTR valueName[] = {L"Donglein", L"GoogleOTP", L"SmartIDCard", L"HardwareAuth"};

  LPCWSTR data;
  if (j == 0) {
    data = L"0";
  }
  else if (j == 1) {
    data = L"1";
  }

  RegSetValueExW(phkResult, valueName[i], 0, REG_SZ, (const BYTE *)data, (DWORD)(wcslen(data) * sizeof WCHAR));

  RegCloseKey(phkResult);
}

void get_setting_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  Isolate* isolate = arguments.GetIsolate();

  int i = (int)arguments[0]->NumberValue();

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\settings", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  LPCWSTR valueName[] = {L"on_off", L"hide_other_users_logon_tiles", L"prohibit_fallback_credential_provider"};

  BYTE data[256];
  memset(data, 0, sizeof(data));
  DWORD cbData = 256;
  RegQueryValueExW(phkResult, valueName[i], NULL, NULL, data, &cbData);

  LPCSTR result;
  if (lstrcmpW((LPCWSTR)data, L"0") == 0) {
    result = "0";
  }
  else if (lstrcmpW((LPCWSTR)data, L"1") == 0) {
    result = "1";
  }

  RegCloseKey(phkResult);

  arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
}

void set_setting_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  int i = (int)arguments[0]->NumberValue();
  int j = (int)arguments[1]->NumberValue();

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\settings", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  LPCWSTR valueName[] = {L"on_off", L"hide_other_users_logon_tiles", L"prohibit_fallback_credential_provider"};

  LPCWSTR data;
  if (j == 0) {
    data = L"0";
  }
  else if (j == 1) {
    data = L"1";
  }
  RegSetValueExW(phkResult, valueName[i], 0, REG_SZ, (const BYTE *)data, (DWORD)(wcslen(data) * sizeof WCHAR));

  RegCloseKey(phkResult);
}

void set_google_otp_key_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  String::Utf8Value t(arguments[0]->ToString());

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\SmartAuth\\", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  wchar_t data[256] = {0};
  mbstowcs(data, (const char *)*t, strlen(*t));

  RegSetValueExW(phkResult, L"GoogleOTPKey", 0, REG_SZ, (const BYTE *)data, (DWORD)(wcslen(data) * sizeof WCHAR));

  RegCloseKey(phkResult);
}

void set_credential_provider_registry_keys_and_values(const FunctionCallbackInfo<Value>& arguments) {
  HKEY hKey;
	RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{2014aaaa-2016-abcd-2016-0123456789ab}", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegCloseKey(hKey);

  RegCreateKeyExW(HKEY_CLASSES_ROOT, L"CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
  RegCloseKey(hKey);

  RegCreateKeyExW(HKEY_CLASSES_ROOT, L"CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}\\InprocServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	RegCloseKey(hKey);


  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{2014aaaa-2016-abcd-2016-0123456789ab}", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);
  RegSetValueExW(phkResult, L"", 0, REG_SZ, (const BYTE *)L"SmartAuthCredentialProvider", (DWORD)(wcslen(L"SmartAuthCredentialProvider") * sizeof WCHAR));
	RegCloseKey(phkResult);

  RegOpenKeyExW(HKEY_CLASSES_ROOT, L"CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);
  RegSetValueExW(phkResult, L"", 0, REG_SZ, (const BYTE *)L"SmartAuthCredentialProvider", (DWORD)(wcslen(L"SmartAuthCredentialProvider") * sizeof WCHAR));
	RegCloseKey(phkResult);

  RegOpenKeyExW(HKEY_CLASSES_ROOT, L"CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}\\InprocServer32", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);
  RegSetValueExW(phkResult, L"", 0, REG_SZ, (const BYTE *)L"SmartAuthCredentialProvider.dll", (DWORD)(wcslen(L"SmartAuthCredentialProvider.dll") * sizeof WCHAR));
  RegSetValueExW(phkResult, L"ThreadingModel", 0, REG_SZ, (const BYTE *)L"Apartment", (DWORD)(wcslen(L"Apartment") * sizeof WCHAR));
  RegCloseKey(phkResult);
}

void delete_credential_provider_registry_keys(const FunctionCallbackInfo<Value>& arguments) {
  RegDeleteKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{2014aaaa-2016-abcd-2016-0123456789ab}", KEY_WOW64_64KEY, 0);
  RegDeleteKeyExW(HKEY_CLASSES_ROOT, L"CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}\\InprocServer32", KEY_WOW64_64KEY, 0);
  Sleep(200);
  RegDeleteKeyExW(HKEY_CLASSES_ROOT, L"CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}", KEY_WOW64_64KEY, 0);
}

void set_excluded_credential_provider_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  int i = (int)arguments[0]->NumberValue();

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  LPCWSTR data;
  if (i == 0) {
    data = L"";
  }
  else if (i == 1) {
    data = L"{6f45dc1e-5384-457a-bc13-2cd81b0d28ed}";
  }

  RegSetValueExW(phkResult, L"ExcludedCredentialProviders", 0, REG_SZ, (const BYTE *)data, (DWORD)(wcslen(data) * sizeof WCHAR));

	RegCloseKey(phkResult);
}

void set_fallback_credential_provider_registry_value(const FunctionCallbackInfo<Value>& arguments) {
  int i = (int)arguments[0]->NumberValue();

  HKEY phkResult;
  RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers", REG_OPTION_OPEN_LINK, KEY_ALL_ACCESS, &phkResult);

  RegSetValueExW(phkResult, L"ProhibitFallbacks", 0, REG_DWORD, (const BYTE *)&i, sizeof i);

	RegCloseKey(phkResult);
}

void get_donglein_key(const FunctionCallbackInfo<Value>& arguments) {
  Isolate* isolate = arguments.GetIsolate();

  HINSTANCE hInst = LoadLibraryW(L"donglein_dll.dll");

	CreateDevice fCreateDevice = (CreateDevice)GetProcAddress(hInst, "CreateDevice");
	GetConfigurationDescriptor fGetConfigurationDescriptor = (GetConfigurationDescriptor)GetProcAddress(hInst, "GetConfigurationDescriptor");
	ReadCapacity fReadCapacity = (ReadCapacity)GetProcAddress(hInst, "ReadCapacity");
	RequestSense fRequestSense = (RequestSense)GetProcAddress(hInst, "RequestSense");
	MediaRead fMediaRead = (MediaRead)GetProcAddress(hInst, "MediaRead");
	MediaWrite fMediaWrite = (MediaWrite)GetProcAddress(hInst, "MediaWrite");

  HANDLE hUsb;
	CAPACITY mediaCapacity;
	BYTE Context[512] = { 0 };

	hUsb = fCreateDevice();

	if (hUsb == INVALID_HANDLE_VALUE)
	{
    arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, "can't connect"));
		return;
	}

	if (fGetConfigurationDescriptor(hUsb) == FALSE)
	{
		arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, "can't connect"));
		return;
	}

	while (fReadCapacity(hUsb, &mediaCapacity) == FALSE) {
		fRequestSense(hUsb);
		Sleep(100);
	}

	fMediaRead(hUsb, &mediaCapacity, Context);

  CHAR result[512] = { 0 };
	for (int i = 300; i < 316; i++) {
		result[i - 300] = Context[i];
	}

  FreeLibrary(hInst);

  arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, result));
}

void set_donglein_key(const FunctionCallbackInfo<Value>& arguments) {
  Isolate* isolate = arguments.GetIsolate();

  String::Utf8Value t(arguments[0]->ToString());
  CHAR t2[512] = { 0 };
  strcpy(t2, *t);

  HINSTANCE hInst = LoadLibraryW(L"donglein_dll.dll");

	CreateDevice fCreateDevice = (CreateDevice)GetProcAddress(hInst, "CreateDevice");
	GetConfigurationDescriptor fGetConfigurationDescriptor = (GetConfigurationDescriptor)GetProcAddress(hInst, "GetConfigurationDescriptor");
	ReadCapacity fReadCapacity = (ReadCapacity)GetProcAddress(hInst, "ReadCapacity");
	RequestSense fRequestSense = (RequestSense)GetProcAddress(hInst, "RequestSense");
	MediaRead fMediaRead = (MediaRead)GetProcAddress(hInst, "MediaRead");
	MediaWrite fMediaWrite = (MediaWrite)GetProcAddress(hInst, "MediaWrite");

  HANDLE hUsb;
	CAPACITY mediaCapacity;
	BYTE Context[512] = { 0 };

	hUsb = fCreateDevice();

	if (hUsb == INVALID_HANDLE_VALUE)
	{
    arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, "can't connect"));
		return;
	}

	if (fGetConfigurationDescriptor(hUsb) == FALSE)
	{
		arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, "can't connect"));
		return;
	}

	while (fReadCapacity(hUsb, &mediaCapacity) == FALSE) {
		fRequestSense(hUsb);
		Sleep(100);
	}

	fMediaRead(hUsb, &mediaCapacity, Context);

	for (int i = 300; i < 316; i++) {
		 Context[i] = t2[i - 300];
	}

  fMediaWrite(hUsb, &mediaCapacity, Context);

  FreeLibrary(hInst);

  arguments.GetReturnValue().Set(String::NewFromUtf8(isolate, "success"));
}

void initialize(Local<Object> exports) {
  NODE_SET_METHOD(exports, "create_registry_keys", create_registry_keys);

  NODE_SET_METHOD(exports, "set_initial_registry_values", set_initial_registry_values);

  NODE_SET_METHOD(exports, "get_authentication_factor_registry_value", get_authentication_factor_registry_value);
  NODE_SET_METHOD(exports, "set_authentication_factor_registry_value", set_authentication_factor_registry_value);

  NODE_SET_METHOD(exports, "get_setting_registry_value", get_setting_registry_value);
  NODE_SET_METHOD(exports, "set_setting_registry_value", set_setting_registry_value);

  NODE_SET_METHOD(exports, "set_google_otp_key_registry_value", set_google_otp_key_registry_value);

  NODE_SET_METHOD(exports, "set_credential_provider_registry_keys_and_values", set_credential_provider_registry_keys_and_values);
  NODE_SET_METHOD(exports, "delete_credential_provider_registry_keys", delete_credential_provider_registry_keys);

  NODE_SET_METHOD(exports, "set_excluded_credential_provider_registry_value", set_excluded_credential_provider_registry_value);

  NODE_SET_METHOD(exports, "set_fallback_credential_provider_registry_value", set_fallback_credential_provider_registry_value);

  NODE_SET_METHOD(exports, "get_donglein_key", get_donglein_key);
  NODE_SET_METHOD(exports, "set_donglein_key", set_donglein_key);
}

NODE_MODULE(addon, initialize)
