////////////////////////////////////////////////////////////////////////////////
// Module    : WIN626.H
// Function  : Declarations for pointers to DLL functions.
// Usage     : Included by all Windows apps that access S626.DLL functions.
// Author    : Jim Lamberson
// Copyright : (C) 2002 Sensoray
////////////////////////////////////////////////////////////////////////////////

#include "App626.h"
#include <windows.h>

// Error codes returned by S626_DLLOpen().
#define ERR_LOAD_DLL	1		// Failed to open S626.DLL.
#define ERR_FUNCADDR	2		// Failed to find function name in S626.DLL.

#ifndef XFUNC626	// If compiling an app module, declare externals in WIN626.C ------------------

// Pointers to DLL functions:
#define XFUNC626(RTNTYPE,FUNCNAME)		extern "C" RTNTYPE (__stdcall *FUNCNAME)

// S626.DLL Startup and Shutdown functions: 
extern "C" unsigned long S626_DLLOpen();		// Open DLL and initialize function pointers.
extern "C" void S626_DLLClose();	// Close DLL.

#endif	// ----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////
// Pointers to functions that are exported by S626.DLL.
///////////////////////////////////////////////////////////////////////////////////

// Status and control functions.
XFUNC626( unsigned long,	S626_GetAddress				)( unsigned long hbd );
XFUNC626( unsigned long,	S626_GetErrors				)( unsigned long hbd );
XFUNC626( void,				S626_OpenBoard				)( unsigned long hbd, unsigned long PhysLoc, FPTR_ISR IntFunc, unsigned long Priority );
XFUNC626( void,				S626_CloseBoard				)( unsigned long hbd );
XFUNC626( void,				S626_InterruptEnable		)( unsigned long hbd, unsigned short enable );
XFUNC626( void,				S626_InterruptStatus		)( unsigned long hbd, unsigned short *status );
XFUNC626( void,				S626_SetErrCallback			)( unsigned long hbd, FPTR_ERR Callback );

// Diagnostics.
XFUNC626( unsigned char,	S626_I2CRead				)( unsigned long hbd, unsigned char addr );
XFUNC626( void,				S626_I2CWrite				)( unsigned long hbd, unsigned char addr, unsigned char value );
XFUNC626( unsigned short,	S626_RegRead				)( unsigned long hbd, unsigned short addr );
XFUNC626( void,				S626_RegWrite				)( unsigned long hbd, unsigned short addr, unsigned short value );

// Analog I/O functions.
XFUNC626( void,				S626_ReadADC				)( unsigned long hbd, short *databuf );
XFUNC626( void,				S626_ResetADC				)( unsigned long hbd, unsigned char *pollist );
XFUNC626( void,				S626_StartADC				)( unsigned long hbd);
XFUNC626( void,				S626_WaitDoneADC			)( unsigned long hbd, short *pdata);
XFUNC626( void,				S626_WriteDAC				)( unsigned long hbd, unsigned short chan, long value );
XFUNC626( void,				S626_WriteTrimDAC			)( unsigned long hbd, unsigned char chan, unsigned char value );

// Digital I/O functions.
XFUNC626( unsigned short,	S626_DIOReadBank			)( unsigned long hbd, unsigned short group );
XFUNC626( unsigned short,	S626_DIOWriteBankGet		)( unsigned long hbd, unsigned short group );
XFUNC626( void,				S626_DIOWriteBankSet		)( unsigned long hbd, unsigned short group, unsigned short value );
XFUNC626( unsigned short,	S626_DIOEdgeGet				)( unsigned long hbd, unsigned short group );
XFUNC626( void,				S626_DIOEdgeSet				)( unsigned long hbd, unsigned short group, unsigned short value );
XFUNC626( void,				S626_DIOCapEnableSet		)( unsigned long hbd, unsigned short group, unsigned short chanmask, unsigned short enable );
XFUNC626( unsigned short,	S626_DIOCapEnableGet		)( unsigned long hbd, unsigned short group );
XFUNC626( void,				S626_DIOCapStatus			)( unsigned long hbd, unsigned short group );
XFUNC626( void,				S626_DIOCapReset			)( unsigned long hbd, unsigned short group, unsigned short value );
XFUNC626( unsigned short,	S626_DIOIntEnableGet		)( unsigned long hbd, unsigned short group );
XFUNC626( void,				S626_DIOIntEnableSet		)( unsigned long hbd, unsigned short group, unsigned short value );
XFUNC626( unsigned short,	S626_DIOModeGet				)( unsigned long hbd, unsigned short group );
XFUNC626( void,				S626_DIOModeSet				)( unsigned long hbd, unsigned short group, unsigned short value );

// Counter functions.
XFUNC626( void,				S626_CounterModeSet			)( unsigned long hbd, unsigned short chan, unsigned short mode );
XFUNC626( unsigned short,	S626_CounterModeGet			)( unsigned long hbd, unsigned short chan );
XFUNC626( void,				S626_CounterEnableSet		)( unsigned long hbd, unsigned short chan, unsigned short enable );
XFUNC626( void,				S626_CounterPreload			)( unsigned long hbd, unsigned short chan, unsigned long value );
XFUNC626( void,				S626_CounterLoadTrigSet		)( unsigned long hbd, unsigned short chan, unsigned short value );
XFUNC626( void,				S626_CounterLatchSourceSet	)( unsigned long hbd, unsigned short chan, unsigned short value );
XFUNC626( unsigned long,	S626_CounterReadLatch		)( unsigned long hbd, unsigned short chan );
XFUNC626( unsigned short,	S626_CounterCapStatus		)( unsigned long hbd );
XFUNC626( void,				S626_CounterCapFlagsReset	)( unsigned long hbd, unsigned short chan );
XFUNC626( void,				S626_CounterSoftIndex		)( unsigned long hbd, unsigned short chan );
XFUNC626( void,				S626_CounterIntSourceSet	)( unsigned long hbd, unsigned short chan, unsigned short value );

// Battery functions:
XFUNC626( unsigned short,	S626_BackupEnableGet		)( unsigned long hbd );
XFUNC626( void,				S626_BackupEnableSet		)( unsigned long hbd, unsigned short en );
XFUNC626( unsigned short,	S626_ChargeEnableGet		)( unsigned long hbd );
XFUNC626( void,				S626_ChargeEnableSet		)( unsigned long hbd, unsigned short en );

// Watchdog functions:
XFUNC626( unsigned short,	S626_WatchdogTimeout		)( unsigned long hbd );
XFUNC626( unsigned short,	S626_WatchdogEnableGet		)( unsigned long hbd );
XFUNC626( void,				S626_WatchdogEnableSet		)( unsigned long hbd, unsigned short en );
XFUNC626( unsigned short,	S626_WatchdogPeriodGet		)( unsigned long hbd );
XFUNC626( void,				S626_WatchdogPeriodSet		)( unsigned long hbd, unsigned short val );
XFUNC626( void,				S626_WatchdogReset			)( unsigned long hbd );


