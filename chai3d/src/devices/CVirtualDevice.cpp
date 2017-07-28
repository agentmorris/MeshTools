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
#include "CVirtualDevice.h"
#include "CVector3d.h"
#include "CMatrix3d.h"
//---------------------------------------------------------------------------
const double DEVICE_WORKSPACE_HALF_SIZE = 0.1;
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cVirtualDevice.

	\fn     cVirtualDevice::cVirtualDevice()
*/
//===========================================================================
cVirtualDevice::cVirtualDevice()
{
	m_systemAvailable = false;
	m_systemReady = false;

	// search for virtual device
	m_hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		"dhdVirtual");

	// no virtual device available
	if (m_hMapFile == NULL)
	{
		m_systemReady = false;
		m_systemAvailable = false;
		return;
	}

	// open connection to virtual device
	m_lpMapAddress = MapViewOfFile(
		m_hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		0);

	// check whether connection succeeded
	if (m_lpMapAddress == NULL)
	{
		m_systemReady = false;
		m_systemAvailable = false;
		return;
	}

	// map memory
	m_pDevice = (cVirtualDeviceData*)m_lpMapAddress;

	// virtual device is available
	m_systemAvailable = true;
}


//===========================================================================
/*!
	Destructor of cVirtualDevice.

	\fn         cVirtualDevice::~cVirtualDevice()
*/
//===========================================================================
cVirtualDevice::~cVirtualDevice()
{
	if (m_systemAvailable)
	{
		CloseHandle(m_hMapFile);
	}
}


//===========================================================================
/*!
	Open connection to virtual device.

	\fn     int cVirtualDevice::open()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cVirtualDevice::open()
{
	if (m_systemAvailable)
	{
		m_systemReady = true;
	}
	return (0);
}


//===========================================================================
/*!
	Close connection to virtual device

	\fn     int cVirtualDevice::close()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cVirtualDevice::close()
{
	m_systemReady = false;

	return (0);
}


//===========================================================================
/*!
	Initialize virtual device.  a_resetEncoders is ignored

	\fn     void cVirtualDevice::initialize(const bool a_resetEncoders=false)
	\param  a_resetEncoders ignored
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cVirtualDevice::initialize(const bool a_resetEncoders)
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
	Set command to the virtual device

	\fn         int cVirtualDevice::command(int a_command, void* a_data)
	\param      a_command  Selected command.
	\param      a_data  Pointer to the corresponding data structure.
	\return     Return status of command.
*/
//===========================================================================
int cVirtualDevice::command(int a_command, void* a_data)
{
	int result = CHAI_MSG_OK;
	double x, y, z;

	if (m_systemReady)
	{
		switch (a_command)
		{
			// read position of delta device
		case CHAI_CMD_GET_POS_3D:
		{
			x = (double)(*m_pDevice).PosX;
			y = (double)(*m_pDevice).PosY;
			z = (double)(*m_pDevice).PosZ;

			cVector3d* position = (cVector3d *)a_data;
			position->set(x, y, z);
		}
		break;

		case CHAI_CMD_GET_POS_NORM_3D:
		{
			x = (double)(*m_pDevice).PosX;
			y = (double)(*m_pDevice).PosY;
			z = (double)(*m_pDevice).PosZ;

			cVector3d* position = (cVector3d *)a_data;
			position->set(x, y, z);

			position->div(DEVICE_WORKSPACE_HALF_SIZE);
		}
		break;

		// read orientation angles
		case CHAI_CMD_GET_ROT_ANGLES:
		{
			cVector3d* angles = (cVector3d *)a_data;
			angles->set(0, 0, 0);
		}
		break;

		// read orientation matrix of wrist
		case CHAI_CMD_GET_ROT_MATRIX:
		{
			cMatrix3d* matrix = (cMatrix3d *)a_data;
			matrix->identity();
		}
		break;

		// set normalized force to device
		case CHAI_CMD_SET_FORCE_3D:
		{
			cVector3d* force = (cVector3d *)a_data;

			((*m_pDevice).ForceX) = force->x;
			((*m_pDevice).ForceY) = force->y;
			((*m_pDevice).ForceZ) = force->z;
		}
		break;

		// set torque to delta wrist
		case CHAI_CMD_SET_TORQUE_3D:
		{
		}
		break;

		// read user switch from wrist
		case CHAI_CMD_GET_SWITCH_MASK:
		case CHAI_CMD_GET_SWITCH_0:
		{
			int* result = (int *)a_data;
			*result = ((bool)(*m_pDevice).Button0) ? 1 : 0;
		}
		break;

		// function is not implemented
		default:
			result = CHAI_MSG_NOT_IMPLEMENTED;
		}
	}
	else
	{
		result = CHAI_MSG_SYSTEM_NOT_READY;
	}

	return (result);
}


