///////////////////////////////////////////////////////////////////////////////
// Module	 : App626.h
// Function  : Header file for Sensoray 626 applications.
// Usage	 : Included by all apps using 626 boards (platform independent).
// Author	 : Jim Lamberson
// Copyright : (C) 2002 Sensoray
///////////////////////////////////////////////////////////////////////////////

// Special types *********************************************************************************

typedef void(*FPTR_ISR)();					// Pointer to application's interrupt callback function.
typedef void(*FPTR_ERR)(long ErrFlags);	// Pointer to application's error callback function.

// Error codes returned by GetErrors() ***********************************************************

												// System errors that are not reset by S626_GetErrors():
#define ERR_OPEN			0x00000001			//	Can't open driver.
#define ERR_CARDREG			0x00000002			//	Can't attach driver to board.
#define ERR_ALLOC_MEMORY	0x00000004			//	Memory allocation error.
#define ERR_LOCK_BUFFER		0x00000008			//	Error locking DMA buffer.
#define ERR_THREAD			0x00000010			//	Error starting a thread.
#define ERR_INTERRUPT		0x00000020			//	Error enabling interrupt.
#define ERR_LOST_IRQ		0x00000040			//  Missed interrupt.
#define ERR_INIT			0x00000080			//  Board object not instantiated.
#define ERR_VERSION			0x00000100			//  Unsupported WinDriver version.
#define ERR_SUBIDS			0x00000200			//  PCI SubDevice/SubVendor ID mismatch.
#define ERR_CFGDUMP			0x00000400			//  PCI configuration dump failed.

												// Board errors that are reset by S626_GetErrors():
#define ERR_ILLEGAL_PARM	0x00010000			//  Illegal function parameter value was specified.
#define ERR_I2C				0x00020000			//	I2C error.
#define ERR_DACTIMEOUT		0x00100000			//	DAC FB_BUFFER write timeout.
#define ERR_COUNTERSETUP	0x00200000			//	Illegal setup specified for counter channel.

// ADC poll list constants *************************************************************************

#define ADC_EOPL			0x80				// End-Of-Poll-List marker.
#define ADC_RANGE_10V		0x00				// Range code for ±10V range.
#define ADC_RANGE_5V		0x10				// Range code for ±5V range.
#define ADC_CHANMASK		0x0F				// Channel number mask.

// Counter constants *******************************************************************************

												// LoadSrc values:
#define LOADSRC_INDX		0					//	Preload core in response to Index.
#define LOADSRC_OVER		1					//	Preload core in response to Overflow.
#define LOADSRCB_OVERA		2					//	Preload B core in response to A Overflow.
#define LOADSRC_NONE		3					//	Never preload core.

												// IntSrc values:
#define INTSRC_NONE 		0 					//	Interrupts disabled.
#define INTSRC_OVER 		1 					//	Interrupt on Overflow.
#define INTSRC_INDX 		2					//	Interrupt on Index.
#define INTSRC_BOTH 		3					//	Interrupt on Index or Overflow.

												// LatchSrc values:
#define LATCHSRC_AB_READ	0					//	Latch on read.
#define LATCHSRC_A_INDXA	1					//	Latch A on A Index.
#define LATCHSRC_B_INDXB	2					//	Latch B on B Index.
#define LATCHSRC_B_OVERA	3					//	Latch B on A Overflow.

												// IndxSrc values:
#define INDXSRC_HARD		0					//	Hardware or software index.
#define INDXSRC_SOFT		1					//	Software index only.

												// IndxPol values:
#define INDXPOL_POS 		0					//	Index input is active high.
#define INDXPOL_NEG 		1					//	Index input is active low.

												// ClkSrc values:
#define CLKSRC_COUNTER		0					//	Counter mode.
#define CLKSRC_TIMER		2					//	Timer mode.
#define CLKSRC_EXTENDER		3					//	Extender mode.

												// ClkPol values:
#define CLKPOL_POS			0					//	Counter/Extender clock is active high.
#define CLKPOL_NEG			1					//	Counter/Extender clock is active low.
#define CNTDIR_UP			0					//	Timer counts up.
#define CNTDIR_DOWN 		1					//	Timer counts down.

												// ClkEnab values:
#define CLKENAB_ALWAYS		0					//	Clock always enabled.
#define CLKENAB_INDEX		1					//	Clock is enabled by index.

												// ClkMult values:
#define CLKMULT_4X 			0					//	4x clock multiplier.
#define CLKMULT_2X 			1					//	2x clock multiplier.
#define CLKMULT_1X 			2					//	1x clock multiplier.

												// Bit Field positions in COUNTER_SETUP structure:
#define BF_LOADSRC			9					//	Preload trigger.
#define BF_INDXSRC			7					//	Index source.
#define BF_INDXPOL			6					//	Index polarity.
#define BF_CLKSRC			4					//	Clock source.
#define BF_CLKPOL			3					//	Clock polarity/count direction.
#define BF_CLKMULT			1					//	Clock multiplier.
#define BF_CLKENAB			0					//	Clock enable.

												// Counter channel numbers:
#define CNTR_0A 			0					//	Counter 0A.
#define CNTR_1A 			1					//	Counter 1A.
#define CNTR_2A 			2					//	Counter 2A.
#define CNTR_0B 			3					//	Counter 0B.
#define CNTR_1B 			4					//	Counter 1B.
#define CNTR_2B 			5					//	Counter 2B.

// Counter overflow/index event flag masks for S626_CounterCapStatus() and S626_InterruptStatus().
#define INDXMASK(C)		( 1 << ( ( (C) > 2 ) ? ( (C) * 2 - 1 ) : ( (C) * 2 +  4 ) ) )
#define OVERMASK(C)		( 1 << ( ( (C) > 2 ) ? ( (C) * 2 + 5 ) : ( (C) * 2 + 10 ) ) )
