#include <iostream>
#include <cstdlib>
#include <ctime>
#include "base32.h"
#include <Windows.h>
#include <atlbase.h>
#include <stdlib.h>

using namespace std;

#define INPUT_BYTE 10
#define OUTPUT_BYTE INPUT_BYTE*8/5

int main(){

	BYTE buffer[INPUT_BYTE] = {};
	char secret[OUTPUT_BYTE+1] = {};
	

	Base32 b32;

	srand(time(NULL));
	for (int i = 0; i < INPUT_BYTE; i++){
		buffer[i] = rand() % 128;
	}

	// base32 encoding
	b32.base32_encode(buffer, OUTPUT_BYTE, (BYTE*)secret, sizeof(secret));

	cout << secret << endl;

}