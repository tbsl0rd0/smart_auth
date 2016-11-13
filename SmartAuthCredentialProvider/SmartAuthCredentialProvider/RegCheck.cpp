#include "RegCheck.h"

RegFlag SetReg() {

	RegFlag rf;

	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 128;
	TCHAR szBuffer[128];


	// Google OTP
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, Auth0, NULL, &dwType, (LPBYTE)szBuffer, &dwSize) == ERROR_SUCCESS) {

			int size = strlen(szBuffer);

			if (size == 1) {
				rf.REG_GoogleOTP = szBuffer[0];
			}
		}
		else rf.REG_GoogleOTP = '0';
	}


	// Sample 하드웨어 인증
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
	{
		if (RegQueryValueEx(hKey, Auth1, NULL, &dwType, (LPBYTE)szBuffer, &dwSize) == ERROR_SUCCESS) {

			int size = strlen(szBuffer);

			if (size == 1) {
				rf.REG_HardwareAuth = szBuffer[0];
			}
		}
		else rf.REG_HardwareAuth = '0';
	}

	RegCloseKey(hKey);

	return rf;

}