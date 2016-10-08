#include "Auth.h"

void Authentication::googleOTP(PWSTR otpKey, PWSTR &username) {
	static PWSTR name = NULL;
	
	if (!wcscmp(username, L"####")) username = name;

	long key = _wtoi(otpKey);

	if (OtpCheck(key) != 0) {
		name = username;
		username = L"####";
	}	
}
