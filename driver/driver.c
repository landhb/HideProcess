#include "driver.h"


DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

UNICODE_STRING  usDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Rootkit");
UNICODE_STRING  usSymbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Rootkit");


// Driver Entry point
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {

	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(RegistryPath);
	PDEVICE_OBJECT deviceObject = NULL;
	

	DbgPrint("Hello World!");
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "DKOM: Driver loaded!\n"));

	
	// Use default dispatcher for 99.9% of IRP requests
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = defaultIrpHandler;
	}

	// Specify the IRP requests we'll actually use
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IrpCallRootkit;

	// Create the IOCTL Device to handle requests
	status = IoCreateDevice(
		DriverObject,
		0,
		&usDeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&deviceObject);

	// Check to ensure it initialized properly
	if (!NT_SUCCESS(status)) {
		return status;
	}

	// Create a symbolic link between the two name
	status = IoCreateSymbolicLink(&usSymbolicLink, &usDeviceName);

	// If the symbolic link fails, delete the IOCTL device and exit
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(deviceObject);
		return status;
	} 

	// Create reference to unload Driver
	DriverObject->DriverUnload = DriverUnload;

	return (status);
}



// Driver unload point
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Driver Unloaded\n"));
	IoDeleteSymbolicLink(&usSymbolicLink);
	IoDeleteDevice(DriverObject->DeviceObject);
	return;
}

