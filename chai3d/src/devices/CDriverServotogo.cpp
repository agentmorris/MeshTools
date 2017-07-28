//===========================================================================
/*
	This file is part of the CHAI 3D visualization and haptics libraries.
	Copyright (C) 2003-2004 by CHAI 3D. All rights reserved.

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License("GPL") version 2
	as published by the Free Software Foundation.

	For using the CHAI 3D libraries with software that can not be combined
	with the GNU GPL, and for taking advantage of the additional benefits
	of our support services, please contact CHAI 3D about acquiring a
	Professional Edition License.

	\author:    <http://www.chai3d.org>
	\author:    Federico Barbagli
	\author:    Francois Conti
	\version    1.2
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CDriverServotogo.h"
//---------------------------------------------------------------------------
#define CNT0_D     0x00
#define CNT1_D     0x01
#define CNT0_C     0x02
#define CNT1_C     0x03
#define CNT2_D     0x04
#define CNT3_D     0x05
#define CNT2_C     0x06
#define CNT3_C     0x07
#define CNT4_D     0x08
#define CNT5_D     0x09
#define CNT4_C     0x0a
#define CNT5_C     0x0b
#define CNT6_D     0x0c
#define CNT7_D     0x0d
#define CNT6_C     0x0e
#define CNT7_C     0x0f
#define DAC_0      0x10
#define DAC_1      0x12
#define DAC_2      0x14
#define DAC_3      0x16
#define DAC_4      0x18
#define DAC_5      0x1a
#define DAC_6      0x1c
#define DAC_7      0x1e
#define ADC        0x410
#define ADC_0      0x410
#define ADC_1      0x412
#define ADC_2      0x414
#define ADC_3      0x416
#define ADC_4      0x418
#define ADC_5      0x41a
#define ADC_6      0x41c
#define ADC_7      0x41e
#define CNTRL0     0x401
#define DIO_A      0x400
#define DIO_B      0x402
#define DIO_C      0x404
#define DIO_D      0x401
#define PORT_A     0x400
#define PORT_B     0x402
#define PORT_C     0x404
#define PORT_D     0x405
#define INTC       0x405
#define BRDTST     0x403
#define MIO_1      0x406
#define ABC_DIR    0x406
#define MIO_2      0x407
#define D_DIR      0x407
#define ODDRST     0x407
#define TIMER_0    0x408
#define TIMER_1    0x40a
#define TIMER_2    0x40c
#define TMRCMD     0x40e
#define ICW1       0x409
#define ICW2       0x40b
#define OCW1       0x40b
#define OCW2       0x409
#define OCW3       0x409
#define IRRreg     0x409
#define ISR        0x409
#define IDLEN      0x409
#define IMR        0x40b
#define SELDI      0x40b
#define IDL        0x40d
#define CNTRL1     0x40f

//---------------------------------------------------------------------------
// Some bit masks for various registers
//---------------------------------------------------------------------------
// for IRR, ISR, and IMR
#define IXEVN      0x80
#define IXODD      0x40
#define LIXEVN     0x20
#define LIXODD     0x10
#define EOC        0x08
#define TP0        0x04
#define USR_INT    0x02
#define TP2        0x01

// for INTC
#define AUTOZERO   0x80
#define IXLVL      0x40
#define IXS1       0x20
#define IXS0       0x10
#define USRINT     0x08
#define IA2        0x04
#define IA1        0x02
#define IA0        0x01

#define CNTRL0_AZ   0x80
#define CNTRL0_AD2  0x40
#define CNTRL0_AD1  0x20
#define CNTRL0_AD0  0x10
#define CNTRL0_CAL  0x08
#define CNTRL0_IA2  0x04
#define CNTRL0_IA1  0x02
#define CNTRL0_IA0  0x01

#define CNTRL1_WDTOUT    0x80
#define CNTRL1_INT_G2    0x40
#define CNTRL1_INT_T0    0x10
#define CNTRL1_INT_T2    0x20
#define CNTRL1_NOT_SLAVE 0x08
#define CNTRL1_IEN_G2    0x04
#define CNTRL1_IEN_T0    0x01
#define CNTRL1_IEN_T2    0x02

#define BRDTST_EOC       0x08
#define IRQSL            0x84

//---------------------------------------------------------------------------
// Input Output redefinitions
//---------------------------------------------------------------------------
inline void fOutP(unsigned short port, unsigned char data)
{
	DlPortWritePortUchar(port, data);
}

inline void fOutPW(unsigned short port, unsigned short data)
{
	DlPortWritePortUshort(port, data);
}

inline unsigned char fInP(unsigned short port)
{
	return (DlPortReadPortUchar(port));
}

inline unsigned short fInPW(unsigned short port)
{
	return (DlPortReadPortUshort(port));
}

//---------------------------------------------------------------------------
// hardware direction bit definitions
//---------------------------------------------------------------------------
#define A_DIR_BIT      0x10
#define B_DIR_BIT      0x02
#define C_LOW_DIR_BIT  0x01
#define C_HI_DIR_BIT   0x08
#define D_DIR_BIT      0x10
#define D_LOW_DIR_BIT  0x01
#define D_HI_DIR_BIT   0x08

//---------------------------------------------------------------------------
// parameters to the SelectInterruptPeriod Command
//---------------------------------------------------------------------------
#define _500_MICROSECONDS        500
#define _1_MILLISECOND           1000
#define _2_MILLISECONDS          2000
#define _3_MILLISECONDS          3000
#define _4_MILLISECONDS          4000
#define _5_MILLISECONDS          5000
#define _10_MILLISECONDS         10000
#define _100_MILLISECONDS        100000L
#define _1_SECOND                1000000L
#define MAX_PERIOD               -1L

#define NO_BOARD       1
#define BOARD_PRESENT  0

#define MODEL_NO_ID    0
#define MODEL1         1
#define MODEL2         2

const int MAX_AXIS = 8;
const int DAC_RANGE_STG = 4095;
const double VOLT_RANGE_STG = 10.0;


//==========================================================================
/*!
	  Constructor of cDriverServotogo.

	  \fn       cDriverServotogo::cDriverServotogo()
*/
//===========================================================================
cDriverServotogo::cDriverServotogo()
{
	m_systemReady = false;
	m_systemAvailable = false;
	m_wAxesInSys = 0;
	m_wIrq = 5;
}


//==========================================================================
/*!
	  Destructor of cDriverServotogo.

	  \fn       cDriverServotogo::~cDriverServotogo()
*/
//===========================================================================
cDriverServotogo::~cDriverServotogo()
{
	close();
}


//==========================================================================
/*!
	Open board.

	\fn     cDriverServotogo::open()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cDriverServotogo::open()
{
	// find base address of board
	m_wBaseAddress = findBaseAddress();
	stg_Init(m_wBaseAddress);

	if (m_wNoBoardFlag == NO_BOARD)
	{
		m_systemReady = false;
		m_systemAvailable = false;
		return (-1);
	}



	// initialize motors to zero.
	for (int i = 0; i < m_wAxesInSys; i++)
	{
		long appo;
		m_homeposition[i] = 0;
		getEncoder(i, &appo);
		m_homeposition[i] = appo;
	}

	m_systemReady = true;
	m_systemAvailable = true;
	return (0);
}


//==========================================================================
/*!
	  Set volt value dVolts for the i-th dac channel. If a_num is out of the range
	  of DACs for the board no action is taken.

	  \fn       void cDriverServotogo::setDac(int a_num, double a_volts)
	  \param    a_num  dac number.
	  \param    a_volts  value to be applied to dacs
*/
//===========================================================================
void cDriverServotogo::setDac(int a_num, double a_volts)
{
	long lCounts;

	if (a_volts > VOLT_RANGE_STG)
	{
		a_volts = VOLT_RANGE_STG;
	}

	if (a_volts < -VOLT_RANGE_STG)
	{
		a_volts = -VOLT_RANGE_STG;
	}

	if ((a_num >= 0) && (a_num < m_wAxesInSys))
	{
		// convert value from volts to a value between -4095 and 4096
		lCounts = (long(DAC_RANGE_STG * (a_volts / VOLT_RANGE_STG)));
		rawDAC(a_num, lCounts);
	}
}


//==========================================================================
/*!
	  Checks if board is present and what is its model number

	  \fn       unsigned short cDriverServotogo::brdtstOK(unsigned short a_baseAddress)
	  \param    a_baseAddress  The address of the board in I/O space

*/
//===========================================================================
unsigned short cDriverServotogo::brdtstOK(unsigned short a_baseAddress)
{

	unsigned short BrdtstAddress;
	unsigned short SerSeq, HighNibble;

	BrdtstAddress = a_baseAddress + BRDTST;

	SerSeq = 0;
	for (int j = 7; j >= 0; j--)
	{
		HighNibble = fInP(BrdtstAddress) >> 4;
		if (HighNibble & 8)     // is SER set
		{
			// shift bit to position specifed by Q2, Q1, Q0
			// which are the lower three bits.  Put bit in SerSeq.
			SerSeq |= 1 << (HighNibble & 7);
		}
	}
	if (SerSeq == 0x75)        // SER sequence is 01110101
	{
		m_wModel = MODEL1;
		return (1);
	}
	else if (SerSeq == 0x74)   // SER sequence is 01110100
	{
		m_wModel = MODEL2;
		return (1);
	}
	else
	{
		m_wModel = MODEL_NO_ID;
		return (0);
	}
}


//==========================================================================
/*!
	  Initializes various aspects of the board

	  \fn       void cDriverServotogo::stg_Init(unsigned short a_wAdd)
	  \param    a_wAdd Address of the board in I/O space.

*/
//===========================================================================
void cDriverServotogo::stg_Init(unsigned short a_wAdd)
{
	m_byIndexPollAxis = 0;
	m_byIndexPulsePolarity = 1;

	m_wBaseAddress = a_wAdd;
	if (!brdtstOK(a_wAdd))    // determines model and fills in wModel
	{
		m_wBaseAddress = 0;
		m_wNoBoardFlag = NO_BOARD;
		m_systemReady = false;
		m_systemAvailable = false;
		return;
	}

	m_wNoBoardFlag = BOARD_PRESENT;

	if (m_wModel == MODEL2)
	{
		fOutP(m_wBaseAddress + CNTRL1, CNTRL1_NOT_SLAVE);
	}

	// stop interrupts: we're not using them
	fOutP(m_wBaseAddress + MIO_2, 0x8b);  // initialize CNTRL0 as output reg.

	fOutP(m_wBaseAddress + CNTRL1,
		(fInP(m_wBaseAddress + CNTRL1) & CNTRL1_NOT_SLAVE) | 0xf0);
	encoderInit();
}


//==========================================================================
/*!
	  Initializes the encoder chips of the board

	  \fn       void cDriverServotogo::encoderInit()

*/
//===========================================================================
void cDriverServotogo::encoderInit()
{

	// It is possible that the encoder counts are being held by battery
	// backup, so we'll read the encoders, and save the values
	// Then we'll initialize the encoder chips, since it's more likely that
	// the encoders were not kept alive by battery and need to be initialized

	LONGBYTE enc[8];
	encReadAll(enc);

	for (unsigned short wAdd = m_wBaseAddress + CNT0_C;
		wAdd <= m_wBaseAddress + CNT6_C; wAdd += 4)
	{
		// we're going to be slick and do two chips at a time, that's why
		// the registers are arranged data, data, control, control.  You
		// can do two at a time, by using word operations, instead of
		// byte operations.  Not a big deal for initializing, but reading is
		// done pretty often.

		fOutPW(wAdd, 0x2020);   // master reset

		// Set Counter Command Register - Input Control, OL Load (P3),
		// and Enable Inputs A and B (INA/B).

		fOutPW(wAdd, 0x6868);

		// Set Counter Command Register - Output Control

		fOutPW(wAdd, 0x8080);

		// Set Counter Command Register - Quadrature

		fOutPW(wAdd, 0xc3c3);

		fOutPW(wAdd, 0x0404);  //reset counter to zero
	}

	//  Figure out how many axes are on the card

	for (unsigned short wA = m_wBaseAddress + CNT0_D; wA <= m_wBaseAddress + CNT6_D; wA += 4)
	{
		unsigned short const wTestPat = 0x5aa5;

		// reset address pointer

		fOutPW(wA + 2, 0x0101);

		// write a pattern to the preset register

		fOutPW(wA, wTestPat);
		fOutPW(wA, wTestPat);
		fOutPW(wA, wTestPat);

		// transfer the preset register to the count register

		fOutPW(wA + 2, 0x0909);

		// transfer counter to output latch

		fOutPW(wA + 2, 0x0202);

		// read the output latch and see if it matches

		if (fInPW(wA) != wTestPat)
			break;
		if (fInPW(wA) != wTestPat)
			break;
		if (fInPW(wA) != wTestPat)
			break;

		// now replace the values that you saved previously, in case the
		// encoder was battery backed up

		fOutP(wA, enc[m_wAxesInSys].Byte[0]);
		fOutP(wA, enc[m_wAxesInSys].Byte[1]);
		fOutP(wA, enc[m_wAxesInSys].Byte[2]);

		fOutP(wA + 1, enc[m_wAxesInSys + 1].Byte[0]);
		fOutP(wA + 1, enc[m_wAxesInSys + 1].Byte[1]);
		fOutP(wA + 1, enc[m_wAxesInSys + 1].Byte[2]);

		// transfer the preset register to the count register

		fOutPW(wA + 2, 0x0909);

		// transfer counter to output latch  debug

		fOutPW(wA + 2, 0x0202);  // debug

		m_wAxesInSys += 2;
	}
}

//==========================================================================
/*!
	  Latches and reads all encoders at once

	  \fn       void cDriverServotogo::encReadAll(LONGBYTE * a_lbEnc)
	  \param    a_lbEnc  array of the encoder values
*/
//===========================================================================
void cDriverServotogo::encReadAll(LONGBYTE* a_lbEnc)
{
	WORDBYTE wbTransfer;
	static unsigned char byOldByte2[MAX_AXIS];
	static unsigned char byEncHighByte[MAX_AXIS];
	short i;

	// Disable interrupts here?  No, the timer will latch new data in the
	// hardware anyway.  Maybe we should stop the timer?  In an interrupt
	// service routine, you're synchronized with the timer; so the readings
	// will never change while you're reading them.  If you're polling, you
	// would first latch the encoder counts with the EncoderLatch() function.
	// But, the timer could latch the counts again, in the middle of the read.
	// A critical section will help in some extreme cases.

	// reset counter internal addr ptr to point to first byte

	encoderLatch();

	for (int add = m_wBaseAddress + CNT0_C; add <= m_wBaseAddress + CNT6_C; add += 4)
		fOutPW(add, 0x0101);

	for (i = 0; i < 3; i++)            // 24 bits means get 3 bytes each
	{
		wbTransfer.Word = fInPW(m_wBaseAddress + CNT0_D);

		a_lbEnc[0].Byte[i] = wbTransfer.Byte.high;
		a_lbEnc[1].Byte[i] = wbTransfer.Byte.low;

		wbTransfer.Word = fInPW(m_wBaseAddress + CNT2_D);

		a_lbEnc[2].Byte[i] = wbTransfer.Byte.high;
		a_lbEnc[3].Byte[i] = wbTransfer.Byte.low;

		wbTransfer.Word = fInPW(m_wBaseAddress + CNT4_D);

		a_lbEnc[4].Byte[i] = wbTransfer.Byte.high;
		a_lbEnc[5].Byte[i] = wbTransfer.Byte.low;

		wbTransfer.Word = fInPW(m_wBaseAddress + CNT6_D);

		a_lbEnc[6].Byte[i] = wbTransfer.Byte.high;
		a_lbEnc[7].Byte[i] = wbTransfer.Byte.low;
	}

	// maintain the high byte, to extend the counter to 32 bits
	//
	// base decisions to increment or decrement the high byte
	// on the highest 2 bits of the 24 bit value.  To get the
	// highest 2 bits, use 0xc0 as a mask on byte [2] (the third
	// byte).

	for (i = 0; i < MAX_AXIS; i++)
	{
		// check for -1 to 0 transition

		if (((byOldByte2[i] & 0xc0) == 0xc0) // 11xxxxxx
			&& ((a_lbEnc[i].Byte[2] & 0xc0) == 0)    // 00xxxxxx
			)
			byEncHighByte[i]++;

		// check for 0 to -1 transition

		if (((byOldByte2[i] & 0xc0) == 0)    // 00xxxxxx
			&& ((a_lbEnc[i].Byte[2] & 0xc0) == 0xc0) // 11xxxxxx
			)
			byEncHighByte[i]--;

		a_lbEnc[i].Byte[3] = byEncHighByte[i];
		byOldByte2[i] = a_lbEnc[i].Byte[2];    // current byte 2 becomes old one
	}
}


//==========================================================================
/*!
	  Latches all encoders

	  \fn       void cDriverServotogo::encoderLatch()

*/
//===========================================================================
void cDriverServotogo::encoderLatch()
{
	// normally you'll have the timer latch the data in hardware, but */
	// if the timer isn't running, we need to latch it ourselves. */

	// BUG FIX-- don't go past 4 axes on 4 axis board
	fOutPW(m_wBaseAddress + CNT0_C, 0x0303);
	fOutPW(m_wBaseAddress + CNT2_C, 0x0303);
	if (m_wAxesInSys > 4)
	{
		fOutPW(m_wBaseAddress + CNT4_C, 0x0303);
		fOutPW(m_wBaseAddress + CNT6_C, 0x0303);
	}
}


//==========================================================================
/*!
	  Finds the base address for the board

	  \fn       unsigned short cDriverServotogo::findBaseAddress()

*/
//===========================================================================
unsigned short cDriverServotogo::findBaseAddress()
{
	short i;
	unsigned short io_add;

	// search for all possible addresses
	for (i = 15; i >= 0; i--)
	{
		io_add = i * 0x20 + 0x200;
		if (brdtstOK(io_add))
			return (io_add);
	}
	return(0);
}


//==========================================================================
/*!
	  Returns board base address.

	  \fn       int cDriverServotogo::getBaseAddress()

*/
//===========================================================================
int cDriverServotogo::getBaseAddress()
{
	return (m_wBaseAddress);
}


//==========================================================================
/*!
	  Read the value of encoder iNum. Returns -1 if the encoder doesn't exist, 1 if read was OK

	  \fn       int cDriverServotogo::getEncoder(int a_num, long *a_value)
	  \param    a_num number of encoder to be read
	  \param    a_value pointer to value read

*/
//===========================================================================
int cDriverServotogo::getEncoder(int a_num, long *a_value)
{
	LONGBYTE lbEnc[MAX_AXIS];

	encReadAll(lbEnc);

	if ((a_num >= 0) || (a_num < m_wAxesInSys))
	{
		*a_value = lbEnc[a_num].Long - m_homeposition[a_num];
		return a_num;
	}
	else
	{
		return -1;
	}
}


//==========================================================================
/*!
	  Writes to DAC nAxis value lCounts

	  \fn       void cDriverServotogo::rawDAC(unsigned short nAxis, long lCounts)
	  \param    nAxis    value of DAC to which to write
	  \param    lCounts  value to be written to the DAC

*/
//===========================================================================
void cDriverServotogo::rawDAC(unsigned short nAxis, long lCounts)
{
	// input / output:
	//
	//    lCounts (decimal) ... -lCounts ... +0x1000 ... volts
	//
	//     0x1000  (4096)     0xfffff000           0       +10
	//          0                      0      0x1000         0
	// 0xfffff001 (-4095)          0xfff      0x1fff       -10

	// So, the domain might be different than you expected. I expected:
	//     0xf000 (-4096)  to  0xfff (4095), rather than
	//     0xf001 (-4095)  to 0x1000 (4096)

	// reverse slope so positive counts give positive voltage
	lCounts = -lCounts;

	// shift for DAC
	lCounts += 0x1000;

	if (lCounts > 0x1FFF)    // clamp + output
	{
		lCounts = 0x1FFF;
	}
	if (lCounts < 0)         // clamp - output
	{
		lCounts = 0;
	}

	fOutPW(m_wBaseAddress + DAC_0 + (nAxis << 1), (unsigned short)lCounts);
}


//==========================================================================
/*!
	  Sets all DACs to zero

	\fn       void cDriverServotogo::close()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cDriverServotogo::close()
{
	// set all the DAC outputs to 0.
	for (int i = 0; i < m_wAxesInSys; i++)
	{
		setDac(i, 0);
	}

	return (0);
}



//===========================================================================
/*!
	Set command to the Servotogo board

	\fn     int cDriverServotogo::command(int iCommand, void* iData)
	\param  iCommand    Selected command.
	\param  iData       Pointer to the corresponding data structure.
	\return Return status of command.
*/
//===========================================================================
int cDriverServotogo::command(int a_command, void* a_data)
{
	int retval = CHAI_MSG_OK;
	if (m_systemReady)
	{
		switch (a_command)
		{
		case CHAI_CMD_GET_DEVICE_STATE:
		{
			if (m_systemReady)
			{
				*(int *)a_data = 0;
			}
			else
			{
				*(int *)a_data = -1;
			}
		}
		break;
		// read encoder 0
		case CHAI_CMD_GET_ENCODER_0:
		{
			long* iValue = (long *)a_data;
			getEncoder(0, iValue);
		}
		break;
		// read encoder 1
		case CHAI_CMD_GET_ENCODER_1:
		{
			long* iValue = (long *)a_data;
			getEncoder(1, iValue);
		}
		break;
		// read encoder 2
		case CHAI_CMD_GET_ENCODER_2:
		{
			long* iValue = (long *)a_data;
			getEncoder(2, iValue);
		}
		break;
		// read encoder 3
		case CHAI_CMD_GET_ENCODER_3:
		{
			long* iValue = (long *)a_data;
			getEncoder(3, iValue);
		}
		break;
		// read encoder 4
		case CHAI_CMD_GET_ENCODER_4:
		{
			long* iValue = (long *)a_data;
			if (getEncoder(4, iValue) < 0)
				*iValue = 0;
		}
		break;
		// read encoder 5
		case CHAI_CMD_GET_ENCODER_5:
		{
			long* iValue = (long *)a_data;
			if (getEncoder(5, iValue) < 0)
				*iValue = 0;
		}
		break;
		// read encoder 6
		case CHAI_CMD_GET_ENCODER_6:
		{
			// check what model of board this is...
			long* iValue = (long *)a_data;
			if (getEncoder(6, iValue) < 0)
				*iValue = 0;
		}
		break;
		// read encoder 7
		case CHAI_CMD_GET_ENCODER_7:
		{
			long* iValue = (long *)a_data;
			if (getEncoder(7, iValue) < 0)
				*iValue = 0;
		}
		break;

		// write motor 0
		case CHAI_CMD_SET_DAC_0:
		{
			double *iVolts = (double *)a_data;
			setDac(0, *iVolts);
		}
		break;
		// write motor 1
		case CHAI_CMD_SET_DAC_1:
		{
			double *iVolts = (double *)a_data;
			setDac(1, *iVolts);
		}
		break;
		// write motor 2
		case CHAI_CMD_SET_DAC_2:
		{
			double *iVolts = (double *)a_data;
			setDac(2, *iVolts);
		}
		break;
		// write motor 3
		case CHAI_CMD_SET_DAC_3:
		{
			double *iVolts = (double *)a_data;
			setDac(3, *iVolts);
		}
		break;
		// write motor 4
		case CHAI_CMD_SET_DAC_4:
		{
			double *iVolts = (double *)a_data;
			setDac(4, *iVolts);
		}
		break;
		// write motor 5
		case CHAI_CMD_SET_DAC_5:
		{
			double *iVolts = (double *)a_data;
			setDac(5, *iVolts);
		}
		break;
		// write motor 6
		case CHAI_CMD_SET_DAC_6:
		{
			double *iVolts = (double *)a_data;
			setDac(6, *iVolts);
		}
		break;
		// write motor 7
		case CHAI_CMD_SET_DAC_7:
		{
			double *iVolts = (double *)a_data;
			setDac(7, *iVolts);
		}
		break;
		// function is not implemented for phantom devices
		default:
			retval = CHAI_MSG_NOT_IMPLEMENTED;
		}
	}
	else
	{
		retval = CHAI_MSG_ERROR;
	}
	return (retval);
}


//==========================================================================
/*!
	Initializes board. In this implementation there's really nothing to do
	that hasn't been done in the opening phase.

	\fn     void cDriverServotogo::initialize(const bool a_resetEncoders=false)
	\return Return 0 is operation succeeds, -1 if an error occurs.

*/
//===========================================================================
int cDriverServotogo::initialize(const bool a_resetEncoders)
{
	if (m_systemReady)
	{
		return (0);
	}
	else
	{
		return (-1);
	}
}

