
// ������ ���ø����̼ǿ� ��� ����
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

// ����̽� ����̹� ��� �ʿ��� ��� ����
#include "setupapi.h"
#include "initguid.h"
#include "winioctl.h"

// ����̹� ���ٿ� �ʿ��� ��� ����
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

// ���� ����
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
	// �ڵ� ����
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

	buffer = (PUCHAR)malloc(256); // ���ǱԷ��̼� �����Ͱ� 256����Ʈ �̻���� ���� ���̴�.
	if (!buffer)
		return FALSE;

	printf("Getbuffer:");

	// ���ǱԷ��̼� ��ũ���͸� ��´�.
	ret = DeviceIoControl(hUsb,
		IOCTL_GET_CONFIGURATION_DESCRIPTOR, // ���� �ڵ�
		NULL, 0,         // �Է�
		buffer, 256,     // ���
		&nBytes,        // ����̽��κ����� ������ ũ��
		NULL);
	if (ret == TRUE) {
		// �����ߴٸ� ���� ������ ���� �����ϰ� �����͸� ǥ���Ѵ�.
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

		// ��ũ-�¸� �������ݷ� SCSI ��ɾ �����Ѵ�.
		// CBW�� �ۼ��� �۽��Ѵ�.
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
		cbw[14] = PackeLength; // SCSI ��Ŷ�� ����

							   // SCSI ��ɾ� ��Ŷ�� �����Ѵ�.
		memcpy(&cbw[15], Packet, PackeLength);

		// CBW�� ��ũ OUT���� �����Ѵ�.
		success = WriteFile(hUsb, &cbw, 31, &nBytes, NULL);
	}
	else {
		UCHAR ufiCommand[12];

		// CB/CBI �������ݷ� SCSI ��Ŷ�� �����Ѵ�.

		// ���� �������� 12����Ʈ�� ������. (UFI ����� 12����Ʈ �����̴�)
		memset(ufiCommand, 0, 12);
		memcpy(ufiCommand, Packet, PackeLength);

		// ADSC ������Ʈ�� �����Ѵ�.
		success = DeviceIoControl(hUsb,
			IOCTL_SEND_CLASS_REQUEST, // ���� �ڵ�
			ufiCommand, 12,   //�Է�
			NULL, 0,          //���
			&nBytes,         //����̽��κ��� ������ ũ��
			NULL);
	}
	return success;
}

BOOL
ReadCompleteStatus(HANDLE hUsb)
{
	// ��ũ-�¸� �������� ����� ��쿡�� CSW�� �����Ѵ�.
	if (InterfaceProtocol == BULK_ONLY_PROTOCOL) {
		UCHAR csw[13];
		DWORD nBytes;

		// �ϴ� ���Ÿ� �Ѵ�.
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
		IOCTL_RESET_PIPE,   // ���� �ڵ�
		NULL, 0,             // �Է�
		NULL, 0,             // ���
		&nBytes,            // ����̽��κ��� ������ ũ��
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

	// SCSI/ATAPI ��ɾ� ��Ŷ�� �����Ѵ�.
	success = SendScsiCommandPacket(
		hUsb,
		Packet,
		PacketLength,
		TransferLength,
		isRead);
	if (success == FALSE)
		goto IssueSCSI_error;

	// �ʿ��ϸ� �����͸� �����Ѵ�.
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

	// �������� ���¸� ��´�.
	success = ReadCompleteStatus(hUsb);

	return success;

IssueSCSI_error:
	// �ϴ� �����Ѵ�.
	ResetPipe(hUsb);

	// ��ũ-�¸� ����̽��� ��� �ݵ�� CSW�� ��� �����ؾ� �Ѵ�.
	// (CB/CBI ���������� ��� ReadCompleteStatus �Լ������� �ƹ��͵� ���� �ʴ´�.)
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

	// SCSI ��ɾ� ��Ŷ (INQUIRY)�� �ۼ��Ѵ�.
	memset(scsi, 0, 6);
	scsi[0] = 0x12;     // OP Code
	scsi[4] = 36;       // Allocate Length

						// SCSI ��ɾ �����Ѵ�.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI ��ɾ� ��Ŷ
		6,          // SCSI ��ɾ� ��Ŷ�� ũ��
		buffer,     // ������ ����
		36,         // ������ ������ ũ��
		&nBytes,    // ���� �Ϸ� ũ��
		TRUE);      // ����̽��κ����� ������ �Է�
	if (success == FALSE) {
		printf("Inquiry Error \n");
		return;
	}

	// ������ü��� ��ǰ���� ǥ���Ѵ�.
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

	// SCSI ��ɾ� ��Ŷ(READ_CAPACITY)�� �ۼ��Ѵ�.
	memset(scsi, 0, 10);
	scsi[0] = 0x25;  // OP Code 

					 // SCSI ��ɾ �����Ѵ�.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI ��ɾ� ��Ŷ
		10,         // SCSI ��ɾ� ��Ŷ�� ũ��
		buffer,     // ������ ����
		8,          // ������ ������ ũ��
		&nBytes,    // ���� �Ϸ� ũ��
		TRUE);      // ����̽��κ����� ������ �Է�
	if (success == FALSE) {
		printf("ReadCapacity Error \n");
		return FALSE;
	}

	// ����� ��ȯ (ǥ 2-35 ����)
	Capacity->NumBlocks = (ULONG)buffer[3];
	Capacity->NumBlocks |= (ULONG)buffer[2] << 8;
	Capacity->NumBlocks |= (ULONG)buffer[1] << 16;
	Capacity->NumBlocks |= (ULONG)buffer[0] << 24;
	Capacity->NumBlocks++;    // ��� �� = ������ ��� ��ȣ + 1

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

	// SCSI ��ɾ� ��Ŷ(REQUEST_SENSE)�� �ۼ��Ѵ�.
	memset(scsi, 0, 6);
	scsi[0] = 0x03;     // OP Code
	scsi[4] = 18;       // Allocate Length

						// SCSI ��ɾ �����Ѵ�.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI ��ɾ� ��Ŷ
		6,          // SCSI ��ɾ� ��Ŷ�� ũ��
		buffer,     // ������ ����
		18,         // ������ ������ ũ��
		&nBytes,    // ���� �Ϸ� ũ��
		TRUE);      // ����̽��κ����� ������ �Է�
	if (success == FALSE) {
		printf("RequestSense Error \n");
		return FALSE;
	}

	// ǥ 2-37 ����
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

	// ATAPI ��ɾ� ��Ŷ (Read10)�� �ۼ��Ѵ�.
	memset(scsi, 0, 10);
	scsi[0] = 0x28;  // OP Code 

	scsi[2] = (UCHAR)(Addr >> 24); // ����� ��ȯ
	scsi[3] = (UCHAR)(Addr >> 16);
	scsi[4] = (UCHAR)(Addr >> 8);
	scsi[5] = (UCHAR)Addr;

	scsi[7] = (UCHAR)(Blocks >> 8); // ����� ��ȯ
	scsi[8] = (UCHAR)Blocks;

	// ������ ����Ʈ ���� ����Ѵ�.
	length = (ULONG)Blocks * Capacity->BlockLength;

	// SCSI ��ɾ �����Ѵ�.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI ��ɾ� ��Ŷ
		10,         // SCSI ��ɾ� ��Ŷ�� ũ��
		Buffer,     // ������ ����
		length,     // ������ ������ ũ��
		&nBytes,    // ���� �Ϸ� ũ��
		TRUE);      // ����̽��κ����� ������ �Է�
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

	// ATAPI ��ɾ� ��Ŷ (Write12)�� �ۼ��Ѵ�.
	memset(scsi, 0, 10);
	scsi[0] = 0x2A;  // OP Code 

	scsi[2] = (UCHAR)(Addr >> 24); // ����� ��ȯ
	scsi[3] = (UCHAR)(Addr >> 16);
	scsi[4] = (UCHAR)(Addr >> 8);
	scsi[5] = (UCHAR)Addr;

	scsi[7] = (UCHAR)(Blocks >> 8); // ����� ��ȯ
	scsi[8] = (UCHAR)Blocks;

	// ������ ����Ʈ ���� ����Ѵ�.
	length = (ULONG)Blocks * Capacity->BlockLength;

	// SCSI ��ɾ �����Ѵ�.
	success = IssueSCSICommand(
		hUsb,
		scsi,       // SCSI ��ɾ� ��Ŷ
		10,         // SCSI ��ɾ� ��Ŷ�� ũ��
		Buffer,     // ������ ����
		length,     // ������ ������ ũ��
		&nBytes,    // ���� �Ϸ� ũ��
		FALSE);      // ����̽��κ����� ������ �Է�
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

	// �Է¹��� ���� üũ�Ѵ�.
	if (address >= Capacity->NumBlocks || blocks >= Capacity->NumBlocks) {
		printf("Input Parameter Error \n");
		return;
	}

	// ������ ���۸� �Ҵ��Ѵ�.
	buffer = calloc(blocks * Capacity->BlockLength, 1);
	if (!buffer) {
		printf("Memory Allocate Error \n");
		return;
	}

	// Read10 ��ɾ �����Ѵ�.
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

	// �Է¹��� ���� üũ�Ѵ�.
	if (address >= Capacity->NumBlocks || blocks >= Capacity->NumBlocks) {
		printf("Input Parameter Error \n");
		return;
	}

	// ������ ���۸� �Ҵ��Ѵ�.
	buffer = calloc(blocks * Capacity->BlockLength, 1);
	if (!buffer) {
		printf("Memory Allocate Error \n");
		return;
	}

	memcpy(buffer, Context, 512);

	// Write12 ��ɾ �����Ѵ�.
	success = Write12(
		hUsb,
		address,
		(USHORT)blocks,
		buffer,
		Capacity);

	free(buffer);
}
