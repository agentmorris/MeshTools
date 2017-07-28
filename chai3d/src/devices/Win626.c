////////////////////////////////////////////////////////////////////////////////
// Module    : WIN626.C
// Function  : DLL Startup/Shutdown functions and pointers to DLL functions.
// Usage     : Linked by all Windows apps that access S626.DLL functions.
// Author    : Jim Lamberson
// Copyright : (C) 2000 Sensoray
////////////////////////////////////////////////////////////////////////////////

/********************* IMPORTANT NOTE FOR MFC PROJECTS *********************
If you are developing an MFC project, you must include the option
"Not using precompiled headers" in the project properties for this file.
***************************************************************************/

// Enable Win626.h function pointers to be created instead of referenced as externals.
#define XFUNC626( RTNTYPE, FUNCNAME )		RTNTYPE ( __stdcall *FUNCNAME )

#include "Win626.h"

static HINSTANCE hlib;

///////////////////////////////////////////////////////////////////
// Initialize all pointers to exported DLL functions.
// Returns 0 if pointers successfully initialized.

#define GETFPTR(FPTR,FNAM)	if ( !( (FARPROC)FPTR = GetProcAddress(hlib, FNAM) ) ) return -1;

DWORD GetFuncPtrs()
{
	// Status and control functions.
	GETFPTR(S626_GetAddress, "S626_GetAddress");
	GETFPTR(S626_GetErrors, "S626_GetErrors");
	GETFPTR(S626_OpenBoard, "S626_OpenBoard");
	GETFPTR(S626_CloseBoard, "S626_CloseBoard");
	GETFPTR(S626_InterruptEnable, "S626_InterruptEnable");
	GETFPTR(S626_InterruptStatus, "S626_InterruptStatus");
	// Diagnostics.
	GETFPTR(S626_I2CRead, "S626_I2CRead");
	GETFPTR(S626_I2CWrite, "S626_I2CWrite");
	GETFPTR(S626_RegRead, "S626_RegRead");
	GETFPTR(S626_RegWrite, "S626_RegWrite");
	// Analog I/O functions.
	GETFPTR(S626_ReadADC, "S626_ReadADC");
	GETFPTR(S626_ResetADC, "S626_ResetADC");
	GETFPTR(S626_WriteDAC, "S626_WriteDAC");
	GETFPTR(S626_WriteTrimDAC, "S626_WriteTrimDAC");
	// Digital I/O functions.
	GETFPTR(S626_DIOReadBank, "S626_DIOReadBank");
	GETFPTR(S626_DIOWriteBankGet, "S626_DIOWriteBankGet");
	GETFPTR(S626_DIOWriteBankSet, "S626_DIOWriteBankSet");
	GETFPTR(S626_DIOEdgeGet, "S626_DIOEdgeGet");
	GETFPTR(S626_DIOEdgeSet, "S626_DIOEdgeSet");
	GETFPTR(S626_DIOCapEnableSet, "S626_DIOCapEnableSet");
	GETFPTR(S626_DIOCapEnableGet, "S626_DIOCapEnableGet");
	GETFPTR(S626_DIOCapReset, "S626_DIOCapReset");
	GETFPTR(S626_DIOCapStatus, "S626_DIOCapStatus");
	GETFPTR(S626_DIOIntEnableGet, "S626_DIOIntEnableGet");
	GETFPTR(S626_DIOIntEnableSet, "S626_DIOIntEnableSet");
	GETFPTR(S626_DIOModeGet, "S626_DIOModeGet");
	GETFPTR(S626_DIOModeSet, "S626_DIOModeSet");
	// Counter functions.
	GETFPTR(S626_CounterModeSet, "S626_CounterModeSet");
	GETFPTR(S626_CounterModeGet, "S626_CounterModeGet");
	GETFPTR(S626_CounterEnableSet, "S626_CounterEnableSet");
	GETFPTR(S626_CounterPreload, "S626_CounterPreload");
	GETFPTR(S626_CounterLoadTrigSet, "S626_CounterLoadTrigSet");
	GETFPTR(S626_CounterLatchSourceSet, "S626_CounterLatchSourceSet");
	GETFPTR(S626_CounterReadLatch, "S626_CounterReadLatch");
	GETFPTR(S626_CounterCapStatus, "S626_CounterCapStatus");
	GETFPTR(S626_CounterCapFlagsReset, "S626_CounterCapFlagsReset");
	GETFPTR(S626_CounterSoftIndex, "S626_CounterSoftIndex");
	GETFPTR(S626_CounterIntSourceSet, "S626_CounterIntSourceSet");
	// Battery functions.
	GETFPTR(S626_BackupEnableGet, "S626_BackupEnableGet");
	GETFPTR(S626_BackupEnableSet, "S626_BackupEnableSet");
	GETFPTR(S626_ChargeEnableGet, "S626_ChargeEnableGet");
	GETFPTR(S626_ChargeEnableSet, "S626_ChargeEnableSet");
	// Watchdog functions.
	GETFPTR(S626_WatchdogTimeout, "S626_WatchdogTimeout");
	GETFPTR(S626_WatchdogEnableGet, "S626_WatchdogEnableGet");
	GETFPTR(S626_WatchdogEnableSet, "S626_WatchdogEnableSet");
	GETFPTR(S626_WatchdogPeriodGet, "S626_WatchdogPeriodGet");
	GETFPTR(S626_WatchdogPeriodSet, "S626_WatchdogPeriodSet");
	GETFPTR(S626_WatchdogReset, "S626_WatchdogReset");

	return 0;		// Indicate all is OK.
}

////////////////////////////////////////////////////////////////////
// Open S626.DLL and get pointers to exported DLL functions.

DWORD S626_DLLOpen()
{
	// Dynamically link to S626.DLL, exit with error if link failed.
	hlib = LoadLibrary("S626.DLL");
	if (hlib == NULL)
		return ERR_LOAD_DLL;

	// Fill pointers to S626.DLL functions, exit with error if attempt failed.
	if (GetFuncPtrs())
	{
		FreeLibrary(hlib);
		return ERR_FUNCADDR;
	}

	// Normal return.
	return 0;
}

///////////////////////////////////////////////////////////////////
// Release dynamic link to S626.DLL.

VOID S626_DLLClose()
{
	// Unlink from S626.DLL.
	if (hlib)
		FreeLibrary(hlib);
}
