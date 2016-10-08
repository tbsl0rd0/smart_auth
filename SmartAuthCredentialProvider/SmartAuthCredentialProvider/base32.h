#ifndef __BASE32_H__
#define __BASE32_H__

typedef unsigned char BYTE;

class Base32{

public:

	//Base32();
	//~Base32();

	int base32_decode(const BYTE *encoded, BYTE *result, int bufSize);
	int base32_encode(const BYTE *data, int length, BYTE *result, int bufSize);

};

#endif