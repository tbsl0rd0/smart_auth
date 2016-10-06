#include <iostream>
#include "HMAC_SHA1.h"

using namespace std;

int main(){
	
	CHMAC_SHA1 HMAC_SHA1;

	BYTE Key[20];
	BYTE digest[20];
	memset(Key, 0x0b, 20);
	unsigned char test[] = "Hi There";
	cout << sizeof(test) << endl;
	cout << sizeof(Key) << endl;
	HMAC_SHA1.HMAC_SHA1(test, sizeof(test)-1, Key, sizeof(Key), digest);

	printf("%d\n", sizeof(digest));
	for (int i = 0; i < 20; i++)
	{
	printf("%0x ", digest[i]);
	}
	printf("\n");
	
	

	return 0;
}