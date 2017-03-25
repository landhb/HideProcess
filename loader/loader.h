#include <stdio.h>
#include <tchar.h>
#include <ctype.h>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <winioctl.h>


// -----------------------------------------------------------------
// Tools
// -----------------------------------------------------------------

BOOL IsElevated(); 			// Checks if the program is elevated - privilages.c

unsigned int FindProcessId(const char *processname);  // Given a process name returns the PID - process.c

const char * GetLastErrorAsString(); 	// Return the last error as a string - errorhandling.c