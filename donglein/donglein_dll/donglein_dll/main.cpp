
// 윈도우 애플리케이션용 헤더 파일
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// 디바이스 드라이버 제어에 필요한 헤더 파일
#include "setupapi.h"
#include "initguid.h"
#include "winioctl.h"

// 드라이버 접근에 필요한 헤더 파일
#include "main.h"

#define DEVICE_PATH_LENGTH 1024

#define OFFSET_INTERFACE_DESCRPTOR  9
#define OFFSET_INTERFACE_CLASS      (OFFSET_INTERFACE_DESCRPTOR + 5)
#define OFFSET_INTERFACE_SUBCLASS   (OFFSET_INTERFACE_DESCRPTOR + 6)
#define OFFSET_INTERFACE_PROTOCOL   (OFFSET_INTERFACE_DESCRPTOR + 7)

#define BULK_ONLY_PROTOCOL  0x50

#define WRITE_FILE_NAME L"read10.dat"
#define READ_FILE_NAME L"write12.dat"

#pragma comment (lib, "Setupapi.lib")

// 전역 변수
UCHAR InterfaceClass;
UCHAR InterfaceSubClass;
UCHAR InterfaceProtocol;

HANDLE
CreateDevice()
{
	HANDLE deviceHandle = NULL;
	HDEVINFO	hwDeviceInfo;
	SP_DEVICE_INTERFACE_DATA devInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA devInterfaceDetailData = NULL;
	ULONG predictedLength = 0;
	ULONG requiredLength = 0, bytes = 0;
	ULONG i = 0;

	hwDeviceInfo = SetupDiGetClassDevs(
		(LPGUID)&MTOM_GUID,
		NULL,
		NULL,
		(DIGCF_PRESENT | DIGCF_DEVICEINTERFACE)
	);
	if (hwDeviceInfo == INVALID_HANDLE_VALUE) {
		printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
		return 0;
	}
	devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	printf("List of USB Device Interfaces\n");

	do {
		if (SetupDiEnumDeviceInterfaces(hwDeviceInfo,
			0,
			(LPGUID)&MTOM_GUID,
			i,
			&devInterfaceData)) {

			if (devInterfaceDetailData) {
				free(devInterfaceDetailData);
				devInterfaceDetailData = NULL;
			}

			if (!SetupDiGetDeviceInterfaceDetail(
				hwDeviceInfo,
				&devInterfaceData,
				NULL,
				0,
				&requiredLength,
				NULL)) {
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
					printf("SetupDiGetDeviceInterfaceDetail failed %d\n", GetLastError());
					SetupDiDestroyDeviceInfoList(hwDeviceInfo);
					return FALSE;
				}
			}
			predictedLength = requiredLength;

			devInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(predictedLength);

			if (devInterfaceDetailData) {
				devInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			}
			else {
				printf("Couldn't allocate &d bytes for device interface details. \n", predictedLength);
				SetupDiDestroyDeviceInfoList(hwDeviceInfo);
				return FALSE;
			}

			if (!SetupDiGetDeviceInterfaceDetail(
				hwDeviceInfo,
				&devInterfaceData,
				devInterfaceDetailData,
				predictedLength,
				&requiredLength,
				NULL)) {
				printf("Error in SetupDiGetDeviceInterfaceDetail\n");
				SetupDiDestroyDeviceInfoList(hwDeviceInfo);
				free(devInterfaceDetailData);
				return FALSE;
			}
			printf("%d) %s\n", ++i, devInterfaceDetailData->DevicePath);
		}
		else if (ERROR_NO_MORE_ITEMS != GetLastError()) {
			free(devInterfaceDetailData);
			devInterfaceDetailData = NULL;
			continue;
		}
		else
			break;
	} while (1);

	SetupDiDestroyDeviceInfoList(hwDeviceInfo);

	if (!devInterfaceDetailData) {
		printf("No device interfaces present\n");
		return 0;
	}
	// 핸들 열기
	printf("\nOpening the last interface: \n%s\n", devInterfaceDetailData->DevicePath);

	deviceHandle = CreateFile(devInterfaceDetailData->DevicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	return deviceHandle;
}
BOOL
GetConfigurationDescriptor(HANDLE hUsb)
{
	PUCHAR buffer;
	DWORD nBytes;
	BOOL ret;

	buffer = (PUCHAR)malloc(256); // 컨피규레이션 데이터가 256바이트 이상되지 않을 것이다.
	if (!buffer)
		return FALSE;

	printf("Getbuffer:");

	// 컨피규레이션 디스크립터를 얻는다.
	ret = DeviceIoControl(hUsb,
		IOCTL_GET_CONFIGURATION_DESCRIPTOR, // 제어 코드
		NULL, 0,         // 입력
		buffer, 256,     // 출력
		&nBytes,        // 디바이스로부터의 데이터 크기
		NULL);
	if (ret == TRUE) {
		// 성공했다면 전역 변수에 값을 설정하고 데이터를 표시한다.
		DWORD i;
		printf(" length=%d \n", nBytes);
		InterfaceClass = buffer[OFFSET_INTERFACE_CLASS];
		InterfaceSubClass = buffer[OFFSET_INTERFACE_SUBCLASS];
		InterfaceProtocol = buffer[OFFSET_INTERFACE_PROTOCOL];
		printf("Class 0x%02x SubClass 0x%02x Protocol 0x%02x \n",
			InterfaceClass, InterfaceSubClass, InterfaceProtocol);
		for (i = 0; i<nBytes; i++)
			printf("%02x ", buffer[i]);
		printf("\n");
	}
	else
		printf("Can't get Condfiguration descriptor.\n");
	free(buffer);

	return ret;
}

BOOL
SendScsiCommandPacket(
	HANDLE hUsb,
	PUCHAR Packet,
	UCHAR  PackeLength,
	ULONG  TransferLength,
	BOOL   isRead
)
{
	BOOL  success;
	DWORD nBytes;

	if (InterfaceProtocol == BULK_ONLY_PROTOCOL) {
		PULONG dataLength;
		UCHAR cbw[32];

		// 벌크-온리 프로토콜로 SCSI 명령어를 발행한다.
		// CBW를 작성해 송신한다.
		memset(cbw, 0, 32);

		// dCBWSignature
		cbw[0] = 0x55; cbw[1] = 0x53;
		cbw[2] = 0x42; cbw[3] = 0x43;

		// dCBWTag
		cbw[4] = 0x80; cbw[5] = 0x00;
		cbw[6] = 0x00; cbw[7] = 0x00;

		// dCBWDataTransferLength
		dataLength = (PULONG)&cbw[8];
		*dataLength = TransferLength;

		// bmCBWFlags
		if (isRead)
			cbw[12] = 0x80;
		else
			cbw[12] = 0x00;

		// bCBWCBLength
		cbw[14] = PackeLength; // SCSI 패킷의 길이

							   // SCSI 명령어 패킷을 복사한다.
		memcpy(&cbw[15], Packet, PackeLength);

		// CBW를 벌크 OUT으로 전송한다.
		success = WriteFile(hUsb, &cbw, 31, &nBytes, NULL);
	}
	else {
		UCHAR ufiCommand[12];

		// CB/CBI 프로토콜로 SCSI 패킷을 발행한다.

		// 제어 전송으로 12바이트를 보낸다. (UFI 사양은 12바이트 고정이다)
		memset(ufiCommand, 0, 12);
		memcpy(ufiCommand, Packet, PackeLength);

		// ADSC 리퀘스트를 발행한다.
		success = DeviceIoControl(hUsb,
			IOCTL_SEND_CLASS_REQUEST, // 제어 코드
			ufiCommand, 12,   //입력
			NULL, 0,          //출력
			&nBytes,         //디바이스로부터 데이터 크기
			NULL);
	}
	return success;
}

BOOL
ReadCompleteStatus(HANDLE hUsb)
{
	// 벌크-온리 프로토콜 방식인 경우에만 CSW를 수신한다.
	if (InterfaceProtocol == BULK_ONLY_PROTOCOL) {
		UCHAR csw[13];
		DWORD nBytes;

		// 일단 수신만 한다.
		return ReadFile(hUsb, &csw, 13, &nBytes, NULL);
	}
	else
		return TRUE;
}

VOID
ResetPipe(HANDLE hUsb)
{
	DWORD nBytes;
	DeviceIoControl(hUsb,
		IOCTL_RESET_PIPE,   // 제어 코드
		NULL, 0,             // 입력
		NULL, 0,             // 출력
		&nBytes,            // 디바이스로부터 데이터 크기
		NULL);
}

BOOL
IssueSCSICommand(
	HANDLE  hUsb,
	PUCHAR  Packet,
	UCHAR   PacketLength,
	PVOID   Buffer,
	ULONG   TransferLength,
	PULONG  CompleteLength,
	BOOL    isRead
)
{
	BOOL success;

	// SCSI/ATAPI 명령어 패킷을 발행한다.
	success = SendScsiCommandPacket(
		hUsb,
		Packet,
		PacketLength,
		TransferLength,
		isRead);
	if (success == FALSE)
		goto IssueSCSI_error;

	// 필요하면 데이터를 전송한다.
	if (TransferLength > 0) {
		if (isRead)
			success = ReadFile(hUsb,
				Buffer, TransferLength, CompleteLength, NULL);
		else
			success = WriteFile(hUsb,
				Buffer, TransferLength, CompleteLength, NULL);
		if (success == FALSE)
			goto IssueSCSI_error;
	}

	// 마지막에 상태를 얻는다.
	success = ReadCompleteStatus(hUsb);

	return success;

IssueSCSI_error:
	// 일단 리셋한다.
	ResetPipe(hUsb);

	// 벌크-온리 디바이스의 경우 반드시 CSW를 얻고 종료해야 한다.
	// (CB/CBI 프로토콜의 경우 ReadCompleteStatus 함수에서는 아무것도 하지 않는다.)
	ReadCompleteStatus(hUsb);
	return FALSE;
}

VOID
Inquiry(HANDLE hUsb)
{
	UCHAR scsi[6], buffer[36];
	ULONG nBytes, i;
	BOOL success;

	printf("Inquiry test:\n");

	// SCSI 명령어 패킷 (INQUIRY)를 작성한다.
	memset(scsi, 0, 6);
	scsi[0] = 0x12;     // OP Code
	scsi[4] = 36;       // Allocate Length

						// SCSI 명령어를 발행한다.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI 명령어 패킷
		6,          // SCSI 명령어 패킷의 크기
		buffer,     // 데이터 버퍼
		36,         // 데이터 버퍼의 크기
		&nBytes,    // 전송 완료 크기
		TRUE);      // 디바이스로부터의 데이터 입력
	if (success == FALSE) {
		printf("Inquiry Error \n");
		return;
	}

	// 제조업체명과 제품명을 표시한다.
	printf("Vendor:");
	for (i = 8; i<16; i++) printf("%c", (CHAR)buffer[i]);
	printf("\nProduct:");
	for (i = 16; i<32; i++) printf("%c", (CHAR)buffer[i]);
#if 1
	printf("\nINQUIRY DATA:\n");
	for (i = 0; i<nBytes; i++)
		printf("%02x ", buffer[i]);
#endif
	printf("\n");
}

BOOL
ReadCapacity(HANDLE hUsb, PCAPACITY Capacity)
{
	UCHAR scsi[10], buffer[8];
	ULONG nBytes;//, i;
	BOOL success;

	// SCSI 명령어 패킷(READ_CAPACITY)을 작성한다.
	memset(scsi, 0, 10);
	scsi[0] = 0x25;  // OP Code 

					 // SCSI 명령어를 발행한다.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI 명령어 패킷
		10,         // SCSI 명령어 패킷의 크기
		buffer,     // 데이터 버퍼
		8,          // 데이터 버퍼의 크기
		&nBytes,    // 전송 완료 크기
		TRUE);      // 디바이스로부터의 데이터 입력
	if (success == FALSE) {
		printf("ReadCapacity Error \n");
		return FALSE;
	}

	// 엔디언 변환 (표 2-35 참조)
	Capacity->NumBlocks = (ULONG)buffer[3];
	Capacity->NumBlocks |= (ULONG)buffer[2] << 8;
	Capacity->NumBlocks |= (ULONG)buffer[1] << 16;
	Capacity->NumBlocks |= (ULONG)buffer[0] << 24;
	Capacity->NumBlocks++;    // 블록 수 = 마지막 블록 번호 + 1

	Capacity->BlockLength = (ULONG)buffer[7];
	Capacity->BlockLength |= (ULONG)buffer[6] << 8;
	Capacity->BlockLength |= (ULONG)buffer[5] << 16;
	Capacity->BlockLength |= (ULONG)buffer[4] << 24;

	return TRUE;
}

//VOID
BOOL
RequestSense(HANDLE hUsb)
{
	UCHAR scsi[6], buffer[18];
	ULONG nBytes;
	UCHAR asc, ascq;
	BOOL success;

	// SCSI 명령어 패킷(REQUEST_SENSE)을 작성한다.
	memset(scsi, 0, 6);
	scsi[0] = 0x03;     // OP Code
	scsi[4] = 18;       // Allocate Length

						// SCSI 명령어를 발행한다.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI 명령어 패킷
		6,          // SCSI 명령어 패킷의 크기
		buffer,     // 데이터 버퍼
		18,         // 데이터 버퍼의 크기
		&nBytes,    // 전송 완료 크기
		TRUE);      // 디바이스로부터의 데이터 입력
	if (success == FALSE) {
		printf("RequestSense Error \n");
		return FALSE;
	}

	// 표 2-37 참고
	asc = buffer[12];
	ascq = buffer[13];
	printf("RequestSense: asc=0x%x ascq=0x%x \n", asc, ascq);

	return TRUE;
}

BOOL
Read10(
	HANDLE      hUsb,
	ULONG       Addr,
	USHORT      Blocks,
	PVOID       Buffer,
	PCAPACITY   Capacity
)
{
	UCHAR scsi[10];
	DWORD nBytes;
	ULONG length;
	BOOL  success;

	// ATAPI 명령어 패킷 (Read10)을 작성한다.
	memset(scsi, 0, 10);
	scsi[0] = 0x28;  // OP Code 

	scsi[2] = (UCHAR)(Addr >> 24); // 엔디언 변환
	scsi[3] = (UCHAR)(Addr >> 16);
	scsi[4] = (UCHAR)(Addr >> 8);
	scsi[5] = (UCHAR)Addr;

	scsi[7] = (UCHAR)(Blocks >> 8); // 엔디언 변환
	scsi[8] = (UCHAR)Blocks;

	// 전송할 바이트 수를 계산한다.
	length = (ULONG)Blocks * Capacity->BlockLength;

	// SCSI 명령어를 발행한다.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI 명령어 패킷
		10,         // SCSI 명령어 패킷의 크기
		Buffer,     // 데이터 버퍼
		length,     // 데이터 버퍼의 크기
		&nBytes,    // 전송 완료 크기
		TRUE);      // 디바이스로부터의 데이터 입력
	if (success == FALSE)
		printf("Read10 Error \n");

	return success;
}

BOOL
Write12(
	HANDLE      hUsb,
	ULONG       Addr,
	USHORT      Blocks,
	PVOID       Buffer,
	PCAPACITY   Capacity
)
{
	UCHAR scsi[10];
	DWORD nBytes;
	ULONG length;
	BOOL  success;

	// ATAPI 명령어 패킷 (Write12)을 작성한다.
	memset(scsi, 0, 10);
	scsi[0] = 0x2A;  // OP Code 

	scsi[2] = (UCHAR)(Addr >> 24); // 엔디언 변환
	scsi[3] = (UCHAR)(Addr >> 16);
	scsi[4] = (UCHAR)(Addr >> 8);
	scsi[5] = (UCHAR)Addr;

	scsi[7] = (UCHAR)(Blocks >> 8); // 엔디언 변환
	scsi[8] = (UCHAR)Blocks;

	// 전송할 바이트 수를 계산한다.
	length = (ULONG)Blocks * Capacity->BlockLength;

	// SCSI 명령어를 발행한다.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI 명령어 패킷
		10,         // SCSI 명령어 패킷의 크기
		Buffer,     // 데이터 버퍼
		length,     // 데이터 버퍼의 크기
		&nBytes,    // 전송 완료 크기
		FALSE);      // 디바이스로부터의 데이터 입력
	if (success == FALSE)
		printf("Write12 Error \n");

	return success;
}

VOID
MediaRead(HANDLE hUsb, PCAPACITY Capacity, BYTE *Context)
{
	ULONG   address = 0;
	ULONG   blocks = 1;
	PVOID   buffer;
	BOOL    success;
	int i;

	// 입력받은 값을 체크한다.
	if (address >= Capacity->NumBlocks || blocks >= Capacity->NumBlocks) {
		printf("Input Parameter Error \n");
		return;
	}

	// 데이터 버퍼를 할당한다.
	buffer = calloc(blocks * Capacity->BlockLength, 1);
	if (!buffer) {
		printf("Memory Allocate Error \n");
		return;
	}

	// Read10 명령어를 발행한다.
	success = Read10(
		hUsb,
		address,
		(USHORT)blocks,
		buffer,
		Capacity);

	for (i = 0; i < 512; i++) {
		Context[i] = *((BYTE *)buffer + i);
		printf("%x ", Context[i]);
	}
	printf("\n");

	free(buffer);
}

VOID
MediaWrite(HANDLE hUsb, PCAPACITY Capacity, BYTE *Context)
{
	ULONG   address = 0;
	ULONG   blocks = 1;
	PVOID   buffer;
	BOOL    success;

	// 입력받은 값을 체크한다.
	if (address >= Capacity->NumBlocks || blocks >= Capacity->NumBlocks) {
		printf("Input Parameter Error \n");
		return;
	}

	// 데이터 버퍼를 할당한다.
	buffer = calloc(blocks * Capacity->BlockLength, 1);
	if (!buffer) {
		printf("Memory Allocate Error \n");
		return;
	}

	memcpy(buffer, Context, 512);

	// Write12 명령어를 발행한다.
	success = Write12(
		hUsb,
		address,
		(USHORT)blocks,
		buffer,
		Capacity);

	free(buffer);
}
