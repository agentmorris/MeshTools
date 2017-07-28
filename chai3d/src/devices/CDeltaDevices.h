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
	\author:    Francois Conti
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CDeltaDevicesH
#define CDeltaDevicesH

#ifndef _DISABLE_DELTA_SUPPORT
//---------------------------------------------------------------------------
#include "CGenericDevice.h"
#include "CPrecisionClock.h"
//---------------------------------------------------------------------------
/*!
	\file CDeltaDevices.h
*/
// Constants defining the available ForceDimension device types
#define DHD_DEVICE_UNINITIALIZED  -1
#define DHD_DEVICE_3DOF           31
#define DHD_DEVICE_6DOF           61
#define DHD_DEVICE_6DOF_500       62
#define DHD_DEVICE_OMEGA          32
#define DHD_DEVICE_OMEGA3         33
#define DHD_DEVICE_OMEGA33        34
#define DHD_DEVICE_OMEGA331       35

//===========================================================================
/*!
	\class cDeltaDevice
	\brief
	Interface to delta and omega devices
*/
//===========================================================================
class cDeltaDevice : public cGenericDevice
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cDeltaDevices.
	cDeltaDevice(unsigned int a_deviceNumber = 0);

	//! Destructor of cGenericDevice.
	virtual ~cDeltaDevice();

	// METHODS:
	//! Open connection to delta device.
	virtual int open();

	//! Close connection to delta device
	virtual int close();

	//! Calibrate delta device.
	virtual int initialize(const bool a_resetEncoders = false);

	//! Set a command to the delta device
	virtual int command(int a_command, void* a_data);

	//! Which ForceDimension device is actually connected to this object?
	virtual int getDeviceType() { return m_deviceType; }

protected:

	//! Reference count used to control access to the dhd dll
	static int m_activeDeltaDevices;

	//! Device ID number
	int m_deviceID;

	//! Which FD device is actually instantiated here?
	int m_deviceType;

	//! Half size of the workspace (in meters)
	double m_halfSizeWorkspace;

	//! Maximum forces
	double m_maximumForces;

	//! Last position of user switch
	int m_userSwitchCount[8];
	int m_userSwitchStatus[8];
	cPrecisionClock m_userSwitchClock[8];

	//! Read user switch from end-effector
	int getUserSwitch(int a_deviceID);
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

#endif
