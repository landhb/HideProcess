#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ntddk.h>
#include <winapifamily.h>


// Driver unload function
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

// Default IPR dispatcher
NTSTATUS defaultDispatch(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);