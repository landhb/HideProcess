#include "driver.h"


DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

// Driver Entry point
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {

	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("Hello World!");
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM: Driver loaded!\n"));


	// Use default dispatcher for 99.9% of IRP requests
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = defaultDispatch;
	}

	// Unload Driver
	//(*DriverObject).DriverUnload = DriverUnload;
	DriverObject->DriverUnload = DriverUnload;

	return (status);
}

// Driver unload point
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Driver Unloaded\n"));
}

// Default IRP dispatcher
NTSTATUS defaultDispatch(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP IrpMessage) {

	UNREFERENCED_PARAMETER(DeviceObject);

	// Set status as success
	IrpMessage->IoStatus.Status = STATUS_SUCCESS;
	IrpMessage->IoStatus.Information = 0;

	// Complete request
	IoCompleteRequest(IrpMessage, IO_NO_INCREMENT);

	return(STATUS_SUCCESS);
}
