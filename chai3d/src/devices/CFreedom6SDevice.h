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
	\author:    Stephen Sinclair
	\version    1.0
	\date       05/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CFreedom6SDeviceH
#define CFreedom6SDeviceH
//---------------------------------------------------------------------------
#include "CGenericDevice.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	\file   CFreedom6SDevice.h
	\class  cFreedom6SDevice
	\brief  cFreedom6SDevice describes an interface to the Freedom6S haptic
			device from MPB Technologies Inc.
*/
//===========================================================================
class cFreedom6SDevice : public cGenericDevice
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cFreedom6SDevice.
	cFreedom6SDevice();
	//! Destructor of cFreedom6SDevice.
	virtual ~cFreedom6SDevice();

	// METHODS:
	//! Open connection to MPB device.
	virtual int open();
	//! Close connection to MPB device.
	virtual int close();
	//! Initialize MPB device.
	virtual int initialize(const bool a_resetEncoders = false);
	//! Set a command to the MPB device.
	virtual int command(int a_command, void* a_data);

protected:

	//! Reference count used to control access to the dll
	static int m_activeFreedom6SDevices;

	void* m_hf6s;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
