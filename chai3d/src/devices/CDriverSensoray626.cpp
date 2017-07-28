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
	\version    1.2
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CDriverSensoray626.h"
#include <stdio.h>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
const int MAX_AXIS_626 = 6;
const int NUM_DACs_626 = 4;
const int DAC_RANGE_626 = 8191;
const double VOLT_RANGE_626 = 10.0;
int cDriverSensoray626::m_BOARD_NUM = 0;
//---------------------------------------------------------------------------

//==========================================================================
/*!
	  Constructor of cDriverSensoray626.

	  \fn       cDriverSensoray626::cDriverSensoray626()
*/
//===========================================================================
cDriverSensoray626::cDriverSensoray626()
{
	m_boardHandle = m_BOARD_NUM;
	m_BOARD_NUM++;
	m_systemReady = false;
	m_systemAvailable = false;
}


//==========================================================================
/*!
	  Destructor of cDriverSensoray626.

	  \fn       cDriverSensoray626::~cDriverSensoray626()
*/
//===========================================================================
cDriverSensoray626::~cDriverSensoray626()
{
	close();
}

//==========================================================================
/*!
	  Open connection to Sensoray board

	  \fn     int cDriverSensoray626::open()
	  \return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cDriverSensoray626::open()
{
	// check if device is not already opened
	if (m_systemReady) { return (0); }

	// number of encoders used.
	for (int i = 0; i < MAX_AXIS_626; i++)
	{
		homeposition[i] = 0;
	}

	// open DLL to talk to the board
	if (S626_DLLOpen() == 0)
	{
		m_systemAvailable = true;

		// Declare Model 626 board to driver and launch the interrupt thread.
		// NOTE: we're supposing to only use one board. With two board we need
		// to specify the physical address for each board used.
		// ALSO: we're NOT using interrupts from the board.
		S626_OpenBoard(m_boardHandle, 0, 0, THREAD_PRIORITY_ABOVE_NORMAL);
		unsigned long ErrCode = S626_GetErrors(0);
		if (ErrCode)
		{
			m_systemReady = false;
			return (-1);
		}
		else
		{
			m_systemReady = true;
		}

		m_wBaseAddress = 0;
		encoderInit();
		for (int i = 0; i < NUM_DACs_626; i++)
		{
			S626_WriteDAC(m_boardHandle, (int)i, 0);
		}
		return (0);
	}
	else
	{
		m_systemReady = false;
		m_systemAvailable = false;
		return (-1);
	}

	return (0);
}


//==========================================================================
/*!
	Close connection to the board, write a zero value to all DACs.

	\fn       int cDriverSensoray626::close()
	\return   Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cDriverSensoray626::close()
{
	if (m_systemReady)
	{
		for (int i = 0; i < NUM_DACs_626; i++)
		{
			S626_WriteDAC(m_boardHandle, (int)i, 0);
		}

		S626_CloseBoard(m_boardHandle);
	}

	m_systemReady = false;
	m_systemAvailable = false;

	return (0);
}


//==========================================================================
/*!
	Sets all counters on the board to be used as encoders.

	\fn     void cDriverSensoray626::encoderInit()
*/
//===========================================================================
void cDriverSensoray626::encoderInit()
{
	// Initialize all encoders at once
	for (int i = 0; i < 6; i++)
	{
		S626_CounterModeSet(m_boardHandle, i,
			(LOADSRC_INDX << BF_LOADSRC) |
			(INDXSRC_SOFT << BF_INDXSRC) |
			(INDXPOL_POS << BF_INDXPOL) |
			(CLKSRC_COUNTER << BF_CLKSRC) |
			(CLKPOL_POS << BF_CLKPOL) |
			(CLKMULT_4X << BF_CLKMULT) |
			(CLKENAB_ALWAYS << BF_CLKENAB));

		// Set the counter CNTR_0A on BOARD to 0
		S626_CounterPreload(m_boardHandle, i, 100000);
		S626_CounterLoadTrigSet(m_boardHandle, i, 1);
		S626_CounterLatchSourceSet(m_boardHandle, i, LATCHSRC_AB_READ);
		S626_CounterIntSourceSet(m_boardHandle, i, INTSRC_INDX);
		homeposition[i] = S626_CounterReadLatch(m_boardHandle, i);
	}
}


//==========================================================================
/*!
	  Initializes board. In this implementation there's really nothing to do
	  that hasn't been done in the opening phase.

	  \fn     int cDriverSensoray626::initialize(const bool a_resetEncoders=false)
	  \param  a_resetEncoders  Ignored; reserved for forward compatibility
	  \return Return 0 is operation succeeds, -1 if an error occurs.

*/
//===========================================================================
int cDriverSensoray626::initialize(const bool a_resetEncoders)
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


//===========================================================================
/*!
	Set command to the Sensoray626 board

	\fn     int cDriverSensoray626::command(int a_command, void* a_data)
	\param  a_command    Selected command.
	\param  a_data       Pointer to the corresponding data structure.
	\return Return status of command.
*/
//===========================================================================
int cDriverSensoray626::command(int a_command, void* a_data)
{
	int retval = CHAI_MSG_OK;
	if (m_systemReady)
	{
		switch (a_command)
		{
		case CHAI_CMD_GET_DEVICE_STATE:
		{
			*(int *)a_data = m_systemReady;
		}
		break;
		// reset encoder 0
		case CHAI_CMD_RESET_ENCODER_0:
		{
			// Read in encoder positions here
			unsigned long cur_pos;
			cur_pos = S626_CounterReadLatch(m_boardHandle, 0);
			homeposition[0] = cur_pos;
		}
		break;
		// reset encoder 1
		case CHAI_CMD_RESET_ENCODER_1:
		{
			// Read in encoder positions here
			unsigned long cur_pos;
			cur_pos = S626_CounterReadLatch(m_boardHandle, 1);
			homeposition[1] = cur_pos;
		}
		break;
		// reset encoder 2
		case CHAI_CMD_RESET_ENCODER_2:
		{
			// Read in encoder positions here
			unsigned long cur_pos;
			cur_pos = S626_CounterReadLatch(m_boardHandle, 2);
			homeposition[2] = cur_pos;
		}
		break;
		// reset encoder 3
		case CHAI_CMD_RESET_ENCODER_3:
		{
			// Read in encoder positions here
			unsigned long cur_pos;
			cur_pos = S626_CounterReadLatch(m_boardHandle, 3);
			homeposition[3] = cur_pos;
		}
		break;
		// reset encoder 4
		case CHAI_CMD_RESET_ENCODER_4:
		{
			// Read in encoder positions here
			unsigned long cur_pos;
			cur_pos = S626_CounterReadLatch(m_boardHandle, 4);
			homeposition[4] = cur_pos;
		}
		break;
		// reset encoder 5
		case CHAI_CMD_RESET_ENCODER_5:
		{
			// Read in encoder positions here
			unsigned long cur_pos;
			cur_pos = S626_CounterReadLatch(m_boardHandle, 5);
			homeposition[5] = cur_pos;
		}
		break;
		// read encoder 0
		case CHAI_CMD_GET_ENCODER_0:
		{
			long* iValue = (long *)a_data;
			unsigned long cur_pos;
			// Read in encoder positions here
			cur_pos = S626_CounterReadLatch(m_boardHandle, 0) - homeposition[0];
			*iValue = (long)cur_pos;
		}
		break;
		// read encoder 1
		case CHAI_CMD_GET_ENCODER_1:
		{
			long* iValue = (long *)a_data;
			unsigned long cur_pos;
			// Read in encoder positions here
			cur_pos = S626_CounterReadLatch(m_boardHandle, 1) - homeposition[1];
			*iValue = (long)cur_pos;
		}
		break;
		// read encoder 2
		case CHAI_CMD_GET_ENCODER_2:
		{
			long* iValue = (long *)a_data;
			unsigned long cur_pos;
			// Read in encoder positions here
			cur_pos = S626_CounterReadLatch(m_boardHandle, 2) - homeposition[2];
			*iValue = (long)cur_pos;
		}
		break;
		// read encoder 3
		case CHAI_CMD_GET_ENCODER_3:
		{
			long* iValue = (long *)a_data;
			unsigned long cur_pos;
			// Read in encoder positions here
			cur_pos = S626_CounterReadLatch(m_boardHandle, 3) - homeposition[3];
			*iValue = (long)cur_pos;
		}
		break;
		// read encoder 4
		case CHAI_CMD_GET_ENCODER_4:
		{
			long* iValue = (long *)a_data;
			unsigned long cur_pos;
			// Read in encoder positions here
			cur_pos = S626_CounterReadLatch(m_boardHandle, 4) - homeposition[4];
			*iValue = (long)cur_pos;
		}
		break;
		// read encoder 5
		case CHAI_CMD_GET_ENCODER_5:
		{
			long* iValue = (long *)a_data;
			unsigned long cur_pos;
			// Read in encoder positions here
			cur_pos = S626_CounterReadLatch(m_boardHandle, 5) - homeposition[5];
			*iValue = (long)cur_pos;
		}
		break;
		// write motor 0
		case CHAI_CMD_SET_DAC_0:
		{
			short lCounts;
			double *iVolts = (double *)a_data;
			if (*iVolts > VOLT_RANGE_626)
				*iVolts = VOLT_RANGE_626;
			if (*iVolts < -VOLT_RANGE_626)
				*iVolts = -VOLT_RANGE_626;
			// convert value from volts to a value between -DAC_RANGE_626 and DAC_RANGE_626
			lCounts = (short((double)DAC_RANGE_626 * (*iVolts / VOLT_RANGE_626)));
			S626_WriteDAC(m_boardHandle, (int)0, lCounts);
		}
		break;
		// write motor 1
		case CHAI_CMD_SET_DAC_1:
		{
			short lCounts;
			double *iVolts = (double *)a_data;
			if (*iVolts > VOLT_RANGE_626)
				*iVolts = VOLT_RANGE_626;
			if (*iVolts < -VOLT_RANGE_626)
				*iVolts = -VOLT_RANGE_626;
			// convert value from volts to a value between -DAC_RANGE_626 and DAC_RANGE_626
			lCounts = (short((double)DAC_RANGE_626 * (*iVolts / VOLT_RANGE_626)));
			S626_WriteDAC(m_boardHandle, (int)1, lCounts);
		}
		break;
		// write motor 2
		case CHAI_CMD_SET_DAC_2:
		{
			short lCounts;
			double *iVolts = (double *)a_data;
			if (*iVolts > VOLT_RANGE_626)
				*iVolts = VOLT_RANGE_626;
			if (*iVolts < -VOLT_RANGE_626)
				*iVolts = -VOLT_RANGE_626;
			// convert value from volts to a value between -DAC_RANGE_626 and DAC_RANGE_626
			lCounts = (short((double)DAC_RANGE_626 * (*iVolts / VOLT_RANGE_626)));
			S626_WriteDAC(m_boardHandle, (int)2, lCounts);
		}
		break;
		// write motor 3
		case CHAI_CMD_SET_DAC_3:
		{
			short lCounts;
			double *iVolts = (double *)a_data;
			if (*iVolts > VOLT_RANGE_626)
				*iVolts = VOLT_RANGE_626;
			if (*iVolts < -VOLT_RANGE_626)
				*iVolts = -VOLT_RANGE_626;
			// convert value from volts to a value between -DAC_RANGE_626 and DAC_RANGE_626
			lCounts = (short((double)DAC_RANGE_626 * (*iVolts / VOLT_RANGE_626)));
			S626_WriteDAC(m_boardHandle, (int)3, lCounts);
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
	return retval;
}

