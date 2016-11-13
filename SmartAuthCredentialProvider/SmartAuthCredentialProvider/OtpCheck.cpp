#include "OtpCheck.h"
#include <atlbase.h>

int OtpCheck(long keyNumber){
	char key[BASE32_SIZE] = {}; // base32 encode key
	char decodedKey[BASE32_SIZE2] = {};

	//Registry Open - Base32 key value
	
	HKEY hKey;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 128;
	TCHAR szBuffer[128];
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, KEY_PATH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		RegQueryValueEx(hKey, KEY_NAME, NULL, &dwType, (LPBYTE)szBuffer, &dwSize);
	}

	RegCloseKey(hKey);

	int key_size = strlen(szBuffer);
	
	for (int i = 0; i < BASE32_SIZE; i++){
		key[i] = szBuffer[i];
	}
	
	// base32 decoding
	Base32 b32;
	b32.base32_decode((BYTE*)key, (BYTE*)decodedKey, sizeof(key)* 5 / 8);

	// current time
	long date = time(NULL);
	long t = date / 30;
	int window = 3;

	for (int i = -window; i <= window; ++i){
		long hash = verify_code((BYTE*)decodedKey, sizeof(decodedKey), t + i);

		if (hash == keyNumber){
			return 0;
		}
	}
	return -1;
}

int verify_code(BYTE* secret, int secretLen, unsigned long t){
	BYTE hash[SHA1_DIGEST_LENGTH] = {};
	BYTE challenge[8];

	for (int i = 8; i-- > 0; t >>= 8){
		challenge[i] = t;
	}

	CHMAC_SHA1 HMAC_SHA1;
	HMAC_SHA1.HMAC_SHA1(challenge, 8, secret, secretLen, hash);

	int offset = hash[SHA1_DIGEST_LENGTH - 1] & 0xF;

	unsigned int truncatedHash = 0;
	for (int i = 0; i < 4; ++i) {
		truncatedHash <<= 8;
		truncatedHash |= (hash[offset + i]);
	}

	// Truncate to a smaller number of digits.
	truncatedHash &= 0x7FFFFFFF;
	truncatedHash %= 1000 * 1000;

	return truncatedHash;
}