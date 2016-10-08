#ifndef __REGCHECK_H__
#define __REGCHECK_H__

#include <windows.h>

#define REG_PATH "SOFTWARE\\WinAuth"
#define REG_ON 1
#define REG_OFF 0

#define Auth0 "GoogleOTP"
#define Auth1 "HardwareAuth"

struct RegFlag {
	BYTE REG_GoogleOTP;
	BYTE REG_HardwareAuth;
};

RegFlag SetReg();

#endif