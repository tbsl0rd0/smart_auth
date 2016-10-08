#include "base32.h"
#include "HMAC_SHA1.h"
#include <ctime>

#define BASE32_SIZE 16
#define BASE32_SIZE2 BASE32_SIZE*5/8
#define HASH_LENGTH 6
#define SHA1_DIGEST_LENGTH 20
#define KEY_PATH "SOFTWARE\\WinAuth"
#define KEY_NAME "GoogleOTPKey"

int OtpCheck(long keyNumber);
int verify_code(BYTE* secret, int secretLen, unsigned long t);