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
#ifndef cDriverSensoray626H
#define cDriverSensoray626H
//---------------------------------------------------------------------------
#include "CGenericDevice.h"
#include "Win626.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \file       CDriverSensoray626.h
	  \class      cDriverSensoray626
	  \brief      cDriverSensoray626 offers an interface to the Sensoray 626 boards.
*/
//===========================================================================
class cDriverSensoray626 : public cGenericDevice
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cDriverSensoray626
	cDriverSensoray626();
	//! Destructor of cDriverSensoray626
	~cDriverSensoray626();

	//! Open connection to Sensoray626 board
	int open();

	//! Close connection to Sensoray626 board
	int close();

	//! Initialize Sensoray626 board
	int initialize(const bool a_resetEncoders = false);

	//! Send a command to the Sensoray626 board
	//! possible commands are:
	//! CHAI_CMD_GET_DEVICE_STATE: returns an int (1 board is ready, 0 board is NOT ready)
	//! CHAI_CMD_GET_ENCODER_0: reads encoder 0, returns counts value in a long
	//! CHAI_CMD_GET_ENCODER_1: reads encoder 1, returns counts value in a long
	//! CHAI_CMD_GET_ENCODER_2: reads encoder 2, returns counts value in a long
	//! CHAI_CMD_GET_ENCODER_3: reads encoder 3, returns counts value in a long
	//! CHAI_CMD_GET_ENCODER_4: reads encoder 4, returns counts value in a long
	//! CHAI_CMD_GET_ENCODER_5: reads encoder 5, returns counts value in a long
	//! CHAI_CMD_SET_DAC_0: writes a voltage to DAC 0 a value between +10 and -10 volts, which is a double
	//! CHAI_CMD_SET_DAC_1: writes a voltage to DAC 1 a value between +10 and -10 volts, which is a double
	//! CHAI_CMD_SET_DAC_2: writes a voltage to DAC 2 a value between +10 and -10 volts, which is a double
	//! CHAI_CMD_SET_DAC_3: writes a voltage to DAC 3 a value between +10 and -10 volts, which is a double
	int command(int iCommand, void* iData);

private:
	// METHODS:
	//! Set encoders to the right operation mode
	void encoderInit();

	// MEMBERS:
	//! Handle to current board.
	int m_boardHandle;

	//! Number of handles that have been initialized
	static int m_BOARD_NUM;

	//! Board base address
	unsigned short m_wBaseAddress;

	//! Initial values of the encoders to reset them
	long homeposition[6];
};


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

