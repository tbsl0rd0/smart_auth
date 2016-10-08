#ifndef __AUTH_H__
#define __AUTH_H__

#include <windows.h>
#include "OtpCheck.h"

class Authentication{
public :
	void googleOTP(PWSTR otpKey, PWSTR &username);
};

#endif