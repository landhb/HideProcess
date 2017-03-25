#include <Ntifs.h>
#include "driver.h"


ULONG find_eprocess_pid_offset() {


	ULONG pid_ofs = 0; // The offset we're looking for
	int idx = 0;                // Index 
	ULONG pids[3];				// List of PIDs for our 3 processes
	PEPROCESS eprocs[3];		// Process list, will contain 3 processes

	//Select 3 process PIDs and get their EPROCESS Pointer
	for (int i = 16; idx<3; i += 4)
	{
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)i, &eprocs[idx])))
		{
			pids[idx] = i;
			idx++;
		}
	}

	/*
	Now go through EPROCESS structure and look for the PID
	we can start at 0x20 because UniqueProcessId should
	not be in the first 0x20 bytes,
	also we should stop after 0x200 bytes with no success
	but this should never occur on the system with unpatched EPROCESS pids
	*/

	for (int i = 0x20; i<0x200; i += 4)
	{
		if ((*(ULONG *)((UCHAR *)eprocs[0] + i) == pids[0])
			&& (*(ULONG *)((UCHAR *)eprocs[1] + i) == pids[1])
			&& (*(ULONG *)((UCHAR *)eprocs[2] + i) == pids[2]))
		{
			pid_ofs = i;
			break;
		}
	}

	ObDereferenceObject(eprocs[0]);
	ObDereferenceObject(eprocs[1]);
	ObDereferenceObject(eprocs[2]);


	return pid_ofs;
} 