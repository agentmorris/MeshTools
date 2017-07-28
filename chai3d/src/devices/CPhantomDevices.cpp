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
#include "CPhantomDevices.h"

#ifndef _DISABLE_PHANTOM_SUPPORT

//---------------------------------------------------------------------------
int cPhantomDevice::m_num_phantoms = 0;
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cPhantomDevice.
	No servo loop is yet created, encoders are NOT reset.

	\fn     cPhantomDevice::cPhantomDevice(int a_num, bool a_dio_access)

	\param  a_num  0-based index of the Phantom that this
				 tool should talk to.
	\param  a_dio_access  If this is 'true', the tool will use the
			Ghost API's direct-i/o model, if it's available.  Otherwise
			the gstEffect i/o model will be used.
*/
//===========================================================================
cPhantomDevice::cPhantomDevice(int a_num, bool a_dio_access)
{

#ifdef _WIN32
	if (a_dio_access)
	{
		PhantomAccess(1);
	}
	else
	{
		PhantomAccess(0);
	}
#endif


	m_phantomHandle = OpenPhantom(a_num);

	if (m_phantomHandle < 0)
	{
		m_systemReady = false;
		m_systemAvailable = false;
	}
	else
	{
		m_systemReady = false;
		m_systemAvailable = true;
		m_num_phantoms++;
	}
}


//===========================================================================
/*!
	Destructor of cPhantomDevice.

	\fn     cPhantomDevice::~cPhantomDevice()
*/
//===========================================================================
cPhantomDevice::~cPhantomDevice()
{
	if (m_systemReady)
	{
		setCallback(NULL);
		StopCommunicationPhantom(m_phantomHandle);
		m_num_phantoms--;
		if (m_num_phantoms == 0)
		{
			ClosePhantoms();
		}
	}
}


//===========================================================================
/*!
	Open connection to phantom device.

	\fn     int cPhantomDevice::open()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cPhantomDevice::open()
{
	if (m_systemAvailable)
	{
		StartCommunicationPhantom(m_phantomHandle);
		m_systemReady = true;
		return 0;
	}

	return -1;
}


//===========================================================================
/*!
	Close connection to phantom device.

	\fn     int cPhantomDevice::close()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cPhantomDevice::close()
{
	if (m_systemReady)
	{
		StopCommunicationPhantom(m_phantomHandle);
		return 0;
	}

	return -1;
}


//===========================================================================
/*!
	Initialize the phantom device.

	For desktops and omnis, the a_resetEncoders parameter is ignored.
	For premiums, if you specify a_resetEncoders as true, you should
	be holding the Phantom in its rest position when this is called.

	\fn     int cPhantomDevice::initialize(const bool a_resetEncoders=false)
	\param  a_resetEncoders Should I re-zero the encoders?  (affects premiums only...)
	\return Return 0 if operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cPhantomDevice::initialize(const bool a_resetEncoders)
{
	if (m_systemReady)
	{
		if (a_resetEncoders) ResetPhantomEncoders(m_phantomHandle);
		return 0;
	}

	return -1;
}


//===========================================================================
/*!
	Set command for the phantom device

	\fn     int cPhantomDevice::command(int iCommand, void* iData)
	\param  iCommand    Selected command.
	\param  iData       Pointer to the corresponding data structure.
	\return Return status of command.
*/
//===========================================================================
int cPhantomDevice::command(int a_command, void* a_data)
{
	int result = CHAI_MSG_OK;

	if (m_systemReady)
	{
		switch (a_command)
		{
			// read position of phantom device
		case CHAI_CMD_GET_POS_3D:
		{
			cVector3d* position = (cVector3d *)a_data;
			// Note: we're doing a change in reference here to pass from the phantom
			// convention to the CHAI one.
			result = ReadPositionPhantom(m_phantomHandle, position->y, position->z, position->x);
		}
		break;

		// read normalized position of phantom device
		case CHAI_CMD_GET_POS_NORM_3D:
		{
			cVector3d* position = (cVector3d *)a_data;
			// Note: we're doing a change in reference here to pass from the phantom
			// convention to the CHAI one.
			result = ReadNormalizedPositionPhantom(m_phantomHandle, position->y, position->z, position->x);
		}
		break;

		// read velocity of phantom device
		case CHAI_CMD_GET_VEL_3D:
		{
			cVector3d* velocity = (cVector3d *)a_data;
			// Note: we're doing a change in reference here to pass from the phantom
			// convention to the CHAI one.
			result = ReadVelocityPhantom(m_phantomHandle, velocity->y, velocity->z, velocity->x);
		}
		break;

		// read orientation matrix of phantom
		case CHAI_CMD_GET_ROT_MATRIX:
		{
			cMatrix3d* mat3 = (cMatrix3d *)a_data;
			double m[9];
			// Note: we're doing a change in reference here to pass from the phantom
			// convention to the CHAI one.
			result = ReadOrientMat3DOFPhantom(m_phantomHandle, m);
			mat3->set(m[8], m[2], m[5], m[6], m[0], m[3], m[7], m[1], m[4]);
		}
		break;

		// set force to phantom device
		case CHAI_CMD_SET_FORCE_3D:
		{
			cVector3d *force;
			force = (cVector3d *)a_data;
			result = SetForcePhantom(m_phantomHandle, force->y, force->z, force->x);
		}
		break;

		// set force and torque to phantom stylus
		case CHAI_CMD_SET_FORCE_TORQUE_3D:
		{
			cVector3d *genforce;
			genforce = (cVector3d *)a_data;
			result = SetForceTorquePhantom(m_phantomHandle,
				genforce[0].y, genforce[0].z, genforce[0].x,
				genforce[1].y, genforce[1].z, genforce[1].x);
		}
		break;

		// read user switch from phantom stylus
		case CHAI_CMD_GET_SWITCH_0:
		{
			int* switchstate = (int *)a_data;
			*switchstate = ReadSwitchPhantom(m_phantomHandle);

			// Only care about button 0
			*switchstate = (*switchstate & 1) ? 1 : 0;

			result = *switchstate;
		}
		break;

		// read user switch from phantom stylus
		case CHAI_CMD_GET_SWITCH_1:
		{
			int* switchstate = (int *)a_data;
			*switchstate = ReadSwitchPhantom(m_phantomHandle);

			// Only care about button 1
			*switchstate = (*switchstate & 2) ? 1 : 0;

			result = *switchstate;
		}
		break;

		// read all user switches from phantom stylus
		case CHAI_CMD_GET_SWITCH_MASK:
		{
			int* switchstate = (int *)a_data;
			*switchstate = ReadSwitchPhantom(m_phantomHandle);
			result = *switchstate;
		}
		break;

		// read scale factor from normalized coords to mm
		case CHAI_CMD_GET_NORMALIZED_SCALE_FACTOR:
		{
			double* scale = (double*)a_data;
			result = GetWorkspaceScalePhantom(m_phantomHandle, *scale);
		}
		break;

		// function is not implemented for phantom devices
		default:
			result = CHAI_MSG_NOT_IMPLEMENTED;
		}
	}
	else
	{
		result = CHAI_MSG_SYSTEM_NOT_READY;
	}

	return result;
}


//===========================================================================
/*!
	Ask the device to call me back periodically.  If this device supports
	timed callbacks, this function will return 'true' and will call the
	supplied m_callback method at haptic rates.  If not, this function will
	return 'false', and you should create your own haptic thread.

	\fn  cPhantomDevice::setCallback(cCallback* m_callback)
	\param  m_callback  The callback to trigger periodically, or 0 to cancel
						an existing callback.
	\return true if this device supports callbacks, false otherwise
*/
//===========================================================================
#ifdef _WIN32
bool cPhantomDevice::setCallback(cCallback* a_callback)
{
	m_callback = a_callback;
	if (m_callback != NULL) {
		if (SetCallbackPhantom(&callbackFunc, this) != PH_DLL_PROBLEM) return true;
	}
	else {
		if (SetCallbackPhantom(NULL, NULL) != PH_DLL_PROBLEM) return true;
	}
	return false;
}
#endif

//===========================================================================
/*!
	Function passed to SetCallbackPhantom
	Calls m_callback->callback()

	\fn  cPhantomDevice::callbackFunc(void* a_data)
	\param a_data  Pointer to instance of cPhantomDevice
*/
//===========================================================================
void cPhantomDevice::callbackFunc(void* a_data)
{
	reinterpret_cast<cPhantomDevice*>(a_data)->m_callback->callback();
}
#endif // _DISABLE_PHANTOM_SUPPORT

