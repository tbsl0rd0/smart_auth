#pragma warning(disable:4200)  
#pragma warning(disable:4201)  
#pragma warning(disable:4214)  

#include <initguid.h>
#include <wdm.h>
#include "usbdi.h"
#include "usbdlib.h"
#include "MainEntry.h"

#pragma warning(default:4200)
#pragma warning(default:4201)
#pragma warning(default:4214)

#include <wdf.h>
#include <wdfusb.h>

#define POOL_TAG	'SBCr'

typedef struct _DEVICE_CONTEXT {
	PUSB_CONFIGURATION_DESCRIPTOR	ConfigurationDescriptor;
	USHORT							wTotalLength;
	WDFUSBDEVICE                    WdfUsbTargetDevice;
	WDFUSBPIPE						BulkReadPipe;
	WDFUSBPIPE						BulkWritePipe;
	WDFUSBPIPE                      BulkHaltedPipe;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext)

PDRIVER_OBJECT ThisDriverObject;

NTSTATUS
MtoM_EvtDriverDeviceAdd(
IN WDFDRIVER       Driver,
IN PWDFDEVICE_INIT  DeviceInit
);

NTSTATUS
MtoM_EvtDeviceD0Entry(
IN WDFDEVICE  Device,
IN WDF_POWER_DEVICE_STATE  PreviousState
);

NTSTATUS
MtoM_EvtDeviceD0Exit(
IN WDFDEVICE  Device,
IN WDF_POWER_DEVICE_STATE  TargetState
);

NTSTATUS
MtoM_EvtDevicePrepareHardware(
IN WDFDEVICE Device,
IN WDFCMRESLIST ResourceList,
IN WDFCMRESLIST ResourceListTranslated
);

NTSTATUS
MtoM_GetConfiguratonDescriptor(
IN WDFDEVICE  		Device,
IN PDEVICE_CONTEXT	DeviceContext
);

VOID
MtoM_InitializePipeInformation(
IN WDFDEVICE Device,
IN PDEVICE_CONTEXT DeviceContext,
IN WDFUSBINTERFACE UsbInterface,
IN UCHAR NumberConfiguredPipes
);

NTSTATUS
MtoM_EvtDeviceReleaseHardware(
IN WDFDEVICE Device,
IN WDFCMRESLIST ResourceListTranslated
);

VOID
MtoM_EvtDeviceSurpriseRemoval(
IN WDFDEVICE  Device
);

VOID
MtoM_EvtDeviceFileCreate(
IN WDFDEVICE            Device,
IN WDFREQUEST           Request,
IN WDFFILEOBJECT        FileObject
);

VOID
MtoM_EvtFileClose(
IN WDFFILEOBJECT  FileObject
);

VOID
MtoM_EvtIoRead(
IN WDFQUEUE         Queue,
IN WDFREQUEST       Request,
IN size_t           Length
);

VOID
MtoM_EvtIoWrite(
IN WDFQUEUE         Queue,
IN WDFREQUEST       Request,
IN size_t           Length
);

VOID
MtoM_SendReadWriteRequest(
IN WDFQUEUE         Queue,
IN WDFREQUEST       Request,
IN size_t           Length,
IN BOOLEAN          IsRead
);

VOID
MtoM_EvtRequestCompletionRoutine(
IN WDFREQUEST                  Request,
IN WDFIOTARGET                 Target,
PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
IN WDFCONTEXT                  Context
);

VOID
MtoM_EvtIoDeviceControl(
IN WDFQUEUE   Queue,
IN WDFREQUEST Request,
IN size_t     OutputBufferLength,
IN size_t     InputBufferLength,
IN ULONG      IoControlCode
);

NTSTATUS MtoM_ClassRequest(
	IN WDFREQUEST Request,
	IN WDFUSBDEVICE UsbTargetDevice,
	OUT ULONG_PTR *CompleteLength
	);

#if DBG

VOID
MtoM_PrintDriverObject(VOID)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Print_ThisDriverObject------------------------------\n");
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "AddDevice %p\n", ThisDriverObject->DriverExtension->AddDevice);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "DriverUnload %p\n", ThisDriverObject->DriverUnload);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_PNP] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_PNP]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_POWER] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_POWER]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_CREATE] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_CREATE]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_CLOSE] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_CLOSE]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_DEVICE_CONTROL] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_READ] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_READ]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MajorFunction[IRP_MJ_WRITE] %p\n", ThisDriverObject->MajorFunction[IRP_MJ_WRITE]);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "------------------------------------------------\n");
}

#endif

NTSTATUS
DriverEntry(
IN PDRIVER_OBJECT  DriverObject,
IN PUNICODE_STRING RegistryPath
)

{
	WDF_DRIVER_CONFIG       config;
	NTSTATUS                ntStatus;

	WDF_DRIVER_CONFIG_INIT(&config, MtoM_EvtDriverDeviceAdd);

	ntStatus = WdfDriverCreate(
		DriverObject,
		RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		WDF_NO_HANDLE);

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"WdfDriverCreate st 0x%x \n", ntStatus);
#endif

	return ntStatus;
}

NTSTATUS
MtoM_EvtDriverDeviceAdd(
IN WDFDRIVER       Driver,
IN PWDFDEVICE_INIT  DeviceInit
)
{
	WDFDEVICE   device;
	NTSTATUS    ntStatus;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	WDF_FILEOBJECT_CONFIG fileConfig;
	WDF_IO_QUEUE_CONFIG ioQueueConfig;
	WDFQUEUE  queue;
	WDF_OBJECT_ATTRIBUTES deviceAttributes;

	UNREFERENCED_PARAMETER(Driver); // 컴파일러 경고 메시지 삭제

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "enter MtoM_EvtDriverDeviceAdd \n");
#endif

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDevicePrepareHardware = MtoM_EvtDevicePrepareHardware;
	pnpPowerCallbacks.EvtDeviceReleaseHardware = MtoM_EvtDeviceReleaseHardware;
	pnpPowerCallbacks.EvtDeviceSurpriseRemoval = MtoM_EvtDeviceSurpriseRemoval;
	pnpPowerCallbacks.EvtDeviceD0Entry = MtoM_EvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit = MtoM_EvtDeviceD0Exit;
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

	WDF_FILEOBJECT_CONFIG_INIT(
		&fileConfig,
		MtoM_EvtDeviceFileCreate,
		MtoM_EvtFileClose,
		WDF_NO_EVENT_CALLBACK
		);
	WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileConfig,
		WDF_NO_OBJECT_ATTRIBUTES);

	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

	WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);
	WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);
	ntStatus = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);
	if (!NT_SUCCESS(ntStatus)) {
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfDeviceCreate err 0x%x \n", ntStatus);
#endif
		return ntStatus;
	}

	ntStatus = WdfDeviceCreateDeviceInterface(device,
		(LPGUID)&MTOM_GUID, NULL);
	if (!NT_SUCCESS(ntStatus)) {
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfDeviceCreateDeviceInterface err 0x%x \n", ntStatus);
#endif
		return ntStatus;
	}

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
		WdfIoQueueDispatchParallel);
	ioQueueConfig.EvtIoDeviceControl = MtoM_EvtIoDeviceControl;
	ioQueueConfig.EvtIoRead = MtoM_EvtIoRead;
	ioQueueConfig.EvtIoWrite = MtoM_EvtIoWrite;
	ntStatus = WdfIoQueueCreate(device,
		&ioQueueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue);// pointer to default queue
	if (!NT_SUCCESS(ntStatus)) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfIoQueueCreate err 0x%x \n", ntStatus);
		return ntStatus;
	}

	return ntStatus;
}

NTSTATUS
MtoM_EvtDeviceD0Entry(
IN WDFDEVICE  Device,
IN WDF_POWER_DEVICE_STATE  PreviousState
)
{
	UNREFERENCED_PARAMETER(Device);
#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtDeviceD0Entry PreviousState %d \n", PreviousState);
#else
	UNREFERENCED_PARAMETER(PreviousState);
#endif
	return STATUS_SUCCESS;
}

NTSTATUS
MtoM_EvtDeviceD0Exit(
IN WDFDEVICE  Device,
IN WDF_POWER_DEVICE_STATE  TargetState
)
{
	UNREFERENCED_PARAMETER(Device);

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtDeviceD0Exit TargetState %d \n", TargetState);
#else
	UNREFERENCED_PARAMETER(TargetState);
#endif
	return STATUS_SUCCESS;
}

NTSTATUS
MtoM_EvtDevicePrepareHardware(
IN WDFDEVICE Device,
IN WDFCMRESLIST ResourceList,
IN WDFCMRESLIST ResourceListTranslated
)
{
	NTSTATUS ntStatus;
	PDEVICE_CONTEXT deviceContext;
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;

	UNREFERENCED_PARAMETER(ResourceList);
	UNREFERENCED_PARAMETER(ResourceListTranslated);

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtDevicePrepareHardware \n");
#endif

	deviceContext = GetDeviceContext(Device);

	ntStatus = WdfUsbTargetDeviceCreate(Device,
		WDF_NO_OBJECT_ATTRIBUTES,
		&deviceContext->WdfUsbTargetDevice);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

	ntStatus = MtoM_GetConfiguratonDescriptor(
		Device,
		deviceContext);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE(&configParams);
	ntStatus = WdfUsbTargetDeviceSelectConfig(
		deviceContext->WdfUsbTargetDevice,
		WDF_NO_OBJECT_ATTRIBUTES,
		&configParams);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

	MtoM_InitializePipeInformation(
		Device,
		deviceContext,
		configParams.Types.SingleInterface.ConfiguredUsbInterface,
		configParams.Types.SingleInterface.NumberConfiguredPipes);

	return STATUS_SUCCESS;

}

NTSTATUS
MtoM_GetConfiguratonDescriptor(
IN WDFDEVICE  		Device,
IN PDEVICE_CONTEXT	DeviceContext
)
{
	UNREFERENCED_PARAMETER(Device);

	NTSTATUS ntStatus;
	WDF_OBJECT_ATTRIBUTES attributes;
	WDFMEMORY   memory;

	DeviceContext->ConfigurationDescriptor = NULL;

	ntStatus = WdfUsbTargetDeviceRetrieveConfigDescriptor(
		DeviceContext->WdfUsbTargetDevice,
		NULL,
		&DeviceContext->wTotalLength);
	if (ntStatus != STATUS_BUFFER_TOO_SMALL)
		return ntStatus;


	// 부모 오브젝트로 WDFUSBDEVICE 오브젝트를 지정한다.
	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	attributes.ParentObject = DeviceContext->WdfUsbTargetDevice;

	// wTotalLength 만큼 메모리를 할당한다.
	ntStatus = WdfMemoryCreate(&attributes,
		NonPagedPool,
		POOL_TAG,
		DeviceContext->wTotalLength,
		&memory,
		&DeviceContext->ConfigurationDescriptor);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

	// 전체 컨피규레이션 디스크립터를 얻는다.
	ntStatus = WdfUsbTargetDeviceRetrieveConfigDescriptor(
		DeviceContext->WdfUsbTargetDevice,
		DeviceContext->ConfigurationDescriptor,
		&DeviceContext->wTotalLength);
#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"MtoM_GetConfiguratonDescriptor st=%x bConfigurationValue=%d \n",
		ntStatus, DeviceContext->ConfigurationDescriptor->bConfigurationValue);
#endif

	return ntStatus;
}

VOID
MtoM_InitializePipeInformation(
IN WDFDEVICE Device,
IN PDEVICE_CONTEXT DeviceContext,
IN WDFUSBINTERFACE UsbInterface,
IN UCHAR NumberConfiguredPipes
)
{
	UNREFERENCED_PARAMETER(Device);

	UCHAR now;
	WDFUSBPIPE pipe;
	WDF_USB_PIPE_INFORMATION pipeInformation;

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_InitializePipeInformation \n");
#endif

	DeviceContext->BulkReadPipe = WDF_NO_HANDLE;
	DeviceContext->BulkWritePipe = WDF_NO_HANDLE;
	DeviceContext->BulkHaltedPipe = WDF_NO_HANDLE;
	pipeInformation.Size = sizeof(WDF_USB_PIPE_INFORMATION);

	for (now = 0; now<NumberConfiguredPipes; now++) {
		pipe = WdfUsbInterfaceGetConfiguredPipe(
			UsbInterface,
			now,
			&pipeInformation);
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "type:%d EP:0x%x \n",
			pipeInformation.PipeType, pipeInformation.EndpointAddress);
#endif
		if (pipeInformation.PipeType == WdfUsbPipeTypeBulk) {
			if (WDF_USB_PIPE_DIRECTION_IN(pipeInformation.EndpointAddress)) {
				DeviceContext->BulkReadPipe = pipe;
				WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipe); // wMaxPacketSize의 배수 체크를 하지 않도록 한다. 
#if DBG
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "  Bulk in \n");
#endif
			}
			else {
				DeviceContext->BulkWritePipe = pipe;
#if DBG
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "  Bulk out \n");
#endif
			}
		}
	}
}

NTSTATUS
MtoM_EvtDeviceReleaseHardware(
IN WDFDEVICE Device,
IN WDFCMRESLIST ResourceListTranslated
)
{
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS  configParams;
	NTSTATUS	ntStatus;
	PDEVICE_CONTEXT deviceContext;

	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(ResourceListTranslated);

	deviceContext = GetDeviceContext(Device);

	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_DECONFIG(&configParams);
	ntStatus = WdfUsbTargetDeviceSelectConfig(
		deviceContext->WdfUsbTargetDevice,
		WDF_NO_OBJECT_ATTRIBUTES,
		&configParams);

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtDeviceReleaseHardware ntStatus 0x%x \n", ntStatus);
#endif	
	return STATUS_SUCCESS;
}


VOID
MtoM_EvtDeviceSurpriseRemoval(
IN WDFDEVICE  Device
)
{
	UNREFERENCED_PARAMETER(Device);
#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtDeviceSurpriseRemoval \n");
#endif
}

#if DBG
VOID
MtoM_DbgPrintFileName(
IN WDFFILEOBJECT FileObject
)
{
	PUNICODE_STRING fileName;

	fileName = WdfFileObjectGetFileName(FileObject);
	if (fileName->Length > 0) {
		ULONG nameLength = (fileName->Length / sizeof(WCHAR));

		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, \
			"Filename = %ws nameLength = %d\n", fileName->Buffer, nameLength);
	}
	else
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, \
		"Filename not found.\n");
}
#endif DBG

VOID
MtoM_EvtDeviceFileCreate(
IN WDFDEVICE            Device,
IN WDFREQUEST           Request,
IN WDFFILEOBJECT        FileObject
)
{
	UNREFERENCED_PARAMETER(Device);
	UNREFERENCED_PARAMETER(FileObject);

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, \
		"enter MtoM_EvtDeviceFileCreate\n");
	MtoM_DbgPrintFileName(FileObject);
#endif

	WdfRequestComplete(Request, STATUS_SUCCESS);

}


VOID
MtoM_EvtFileClose(
IN WDFFILEOBJECT  FileObject
)
{
	UNREFERENCED_PARAMETER(FileObject);

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, \
		"enter MtoM_EvtFileClose\n");
	MtoM_DbgPrintFileName(FileObject);
#endif

}

VOID
MtoM_EvtIoRead(
IN WDFQUEUE         Queue,
IN WDFREQUEST       Request,
IN size_t           Length
)
{
#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtIoRead len=%d \r\n", Length);
#endif

	MtoM_SendReadWriteRequest(
		Queue,
		Request,
		Length,
		TRUE);
	return;
}

VOID
MtoM_EvtIoWrite(
IN WDFQUEUE         Queue,
IN WDFREQUEST       Request,
IN size_t           Length
)
{
#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtIoWrite len=%d \r\n", Length);
#endif

	MtoM_SendReadWriteRequest(
		Queue,
		Request,
		Length,
		FALSE);
	return;
}

VOID
MtoM_SendReadWriteRequest(
IN WDFQUEUE         Queue,
IN WDFREQUEST       Request,
IN size_t           Length,
IN BOOLEAN          IsRead
)
{
	UNREFERENCED_PARAMETER(Length);

	NTSTATUS ntStatus;
	WDFMEMORY requestMemory;
	PDEVICE_CONTEXT deviceContext;
	BOOLEAN ret;
	WDFUSBPIPE pipe;

	deviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));

	if (IsRead) {
		pipe = deviceContext->BulkReadPipe;
		ntStatus = WdfRequestRetrieveOutputMemory(Request, &requestMemory);
	}
	else {
		pipe = deviceContext->BulkWritePipe;
		ntStatus = WdfRequestRetrieveInputMemory(Request, &requestMemory);
	}
	if (!NT_SUCCESS(ntStatus)) {
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfRequestRetrieveMemory error 0x%x pipe %p \n", ntStatus, pipe);
#endif  
		goto Request_Exit;
	}

	if (IsRead) {
		pipe = deviceContext->BulkReadPipe;
		ntStatus = WdfUsbTargetPipeFormatRequestForRead(
			pipe,
			Request,
			requestMemory,
			NULL
			);
	}
	else {
		pipe = deviceContext->BulkWritePipe;
		ntStatus = WdfUsbTargetPipeFormatRequestForWrite(
			pipe,
			Request,
			requestMemory,
			NULL
			);
	}
	if (!NT_SUCCESS(ntStatus)) {
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfUsbTargetPipeFormatRequest error 0x%x \n", ntStatus);
#endif       
		goto Request_Exit;
	}

	WdfRequestSetCompletionRoutine(
		Request,
		MtoM_EvtRequestCompletionRoutine,
		deviceContext);

	ret = WdfRequestSend(
		Request,
		WdfUsbTargetPipeGetIoTarget(pipe),
		WDF_NO_SEND_OPTIONS);
	if (ret == FALSE) {
		ntStatus = WdfRequestGetStatus(Request);
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfRequestSend error 0x%x \n", ntStatus);
#endif       
		goto Request_Exit;
	}

Request_Exit:
	if (!NT_SUCCESS(ntStatus)) {
		WdfRequestCompleteWithInformation(Request, ntStatus, 0);
	}

	return;
}

VOID
MtoM_EvtRequestCompletionRoutine(
IN WDFREQUEST                  Request,
IN WDFIOTARGET                 Target,
PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
IN WDFCONTEXT                  Context
)
{
	NTSTATUS    ntStatus;
	size_t      completeLength = 0;
	PDEVICE_CONTEXT deviceContext;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;
	WDFUSBPIPE  pipe;

	UNREFERENCED_PARAMETER(Target);

	deviceContext = (PDEVICE_CONTEXT)Context;

	ntStatus = CompletionParams->IoStatus.Status;

	usbCompletionParams = CompletionParams->Parameters.Usb.Completion;

	if (usbCompletionParams->Type == WdfUsbRequestTypePipeRead) {
		completeLength = usbCompletionParams->Parameters.PipeRead.Length;
		pipe = deviceContext->BulkReadPipe;
	}
	else {
		completeLength = usbCompletionParams->Parameters.PipeWrite.Length;
		pipe = deviceContext->BulkWritePipe;
	}

	if (NT_ERROR(ntStatus))
		deviceContext->BulkHaltedPipe = pipe;

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtRequestCompletionRoutine len=%d usb=0x%x pipe %p\r\n",
		completeLength, ntStatus, pipe);
#endif

	WdfRequestCompleteWithInformation(Request, ntStatus, completeLength);

	return;
}

VOID
MtoM_EvtIoDeviceControl(
IN WDFQUEUE   Queue,
IN WDFREQUEST Request,
IN size_t     OutputBufferLength,
IN size_t     InputBufferLength,
IN ULONG      IoControlCode
)
{
	UNREFERENCED_PARAMETER(InputBufferLength);

	NTSTATUS            ntStatus;

	PUCHAR              systemBuffer;
	ULONG_PTR           completeLength;
	WDFDEVICE           device;
	PDEVICE_CONTEXT     deviceContext;

#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"enter MtoM_EvtIoDeviceControl code=0x%08x in:out %d:%d \n",
		IoControlCode, InputBufferLength, OutputBufferLength);
#endif

	completeLength = 0;
	device = WdfIoQueueGetDevice(Queue);
	deviceContext = GetDeviceContext(device);

	switch (IoControlCode) {

	case IOCTL_GET_CONFIGURATION_DESCRIPTOR:
		ntStatus = WdfRequestRetrieveOutputBuffer(
			Request,
			OutputBufferLength,
			(PVOID *)&systemBuffer,
			NULL);

		if (NT_SUCCESS(ntStatus))  {
			completeLength = OutputBufferLength;
			if (completeLength > deviceContext->wTotalLength)
				completeLength = deviceContext->wTotalLength;

			RtlCopyMemory(
				systemBuffer,
				deviceContext->ConfigurationDescriptor,
				completeLength);
		}
		else
			completeLength = 0;
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"IOCTL_GET_CONFIGURATION_DESCRIPTOR  completeLength=%d \n",
			completeLength);
#endif
		break;

	case IOCTL_SEND_CLASS_REQUEST:
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"IOCTL_SEND_CLASS_REQUEST\n");
#endif
		ntStatus = MtoM_ClassRequest(
			Request,
			deviceContext->WdfUsbTargetDevice,
			&completeLength);
		break;

	case IOCTL_RESET_PIPE:
		if (deviceContext->BulkHaltedPipe != WDF_NO_HANDLE) {
			ntStatus = WdfUsbTargetPipeResetSynchronously(
				deviceContext->BulkHaltedPipe,
				WDF_NO_HANDLE, NULL);
			deviceContext->BulkHaltedPipe = WDF_NO_HANDLE;
		}
		else
			ntStatus = STATUS_SUCCESS;
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"IOCTL_RESET_PIPE st=0x%x\n", ntStatus);
#endif
		break;

	default:
		ntStatus = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	WdfRequestCompleteWithInformation(Request, ntStatus, completeLength);
}

NTSTATUS MtoM_ClassRequest(
	IN WDFREQUEST Request,
	IN WDFUSBDEVICE UsbTargetDevice,
	OUT ULONG_PTR *CompleteLength
	)
{
	NTSTATUS ntStatus;
	WDF_USB_CONTROL_SETUP_PACKET setup;
	WDF_MEMORY_DESCRIPTOR memoryDescriptor;
	ULONG    sentLength;
	WDFMEMORY memory;
	size_t length;

	ntStatus = WdfRequestRetrieveInputMemory(Request, &memory);
	if (!NT_SUCCESS(ntStatus)) {
#if DBG
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"WdfRequestRetrieveMemory failed 0x%x", ntStatus);
#endif
		return ntStatus;
	}
	WDF_MEMORY_DESCRIPTOR_INIT_HANDLE(&memoryDescriptor, memory, NULL);

	(PVOID)WdfMemoryGetBuffer(memory, &length);

	WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(&setup,
		BmRequestHostToDevice,
		BmRequestToInterface,
		0x00,  // Request
		0x00,  // Value
		0x00); // Index
	setup.Packet.wLength = (USHORT)length;

	ntStatus = WdfUsbTargetDeviceSendControlTransferSynchronously(
		UsbTargetDevice,
		NULL,
		NULL,
		&setup,
		&memoryDescriptor,
		&sentLength);
#if DBG
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"USB_ClassRequest state - 0x%x sentLength=%d \n", ntStatus, sentLength);
#endif

	*CompleteLength = sentLength;

	return ntStatus;
}
