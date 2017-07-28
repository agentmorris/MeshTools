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
	\author:    Stephen Sinclair (MPBT)
	\version    1.0
	\date       05/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CFreedom6SDevice.h"
//---------------------------------------------------------------------------
#include <windows.h>
#include "CVector3D.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
HINSTANCE hf6sDLL = NULL;

//---------------------------------------------------------------------------
// Copied from f6s.h in the Freedom6S API

typedef enum
{
	F6SRC_NOERROR = 0,
	F6SRC_ALREADYEXIST = -1,      /* A Freedom6S device is already open in the system                                */
	F6SRC_BADVALUE = -2,        /* Value out of range                                                              */
	F6SRC_BADPOINTER = -3,        /* Bad pointer passed to function                                                  */
	F6SRC_MEMORY = -4,          /* Out of memory                                                                   */
	F6SRC_REGISTRY = -5,        /* Error reading registry values (will user defaults)                              */
	F6SRC_INIFILE_READ = -6,      /* Error reading ini file (settings)                                               */
	F6SRC_INIFILE_WRITE = -7,      /* Error writing ini file (settings)                                               */
	F6SRC_NOTINITIALIZED = -8,      /* Attempt to call a function before f6s_Initialize()                              */
	F6SRC_BADHANDLE = -9,        /* A function received a bad HF6S value                                            */
	F6SRC_BADMOTORTEMP = -10,     /* Motor temperatures were out of range or not read correctly (warning only)       */
	F6SRC_JOINTVELINIT = -11,     /* Attempt to read velocity without joint velocity computation enabled             */
	F6SRC_CALIBRATION = -12,       /* Unable to calibrate, require mechanical calibration                             */
	F6SRC_ROLLANGLE = -13,       /* Unable to calculate roll angle, sensors 4 & 5 require mechanical re-calibration */
	F6SRC_DRIVERINIT = -14,       /* Unable to initialize the drivers for ADC or DAC hardware                        */
	F6SRC_IOERROR = -15,         /* Error returned from ADC or DAC drivers                                          */
	F6SRC_DAQCONFIG = -16,       /* Unknown DAQ configuration                             */
	F6SRC_HOTMOTOR = -17,     /* One or more motors have been flagged hot, causing the max current to decrease   */
	F6SRC_FAILURE = -18  /* Operation failed                                                                */
} F6SRC;

typedef void* HF6S;

F6SRC(*f6s_Initialize)(HF6S* phf6s);
F6SRC(*f6s_ComputeJointVel)(HF6S hf6s, float ftimeStep, int inewBufferSize);
F6SRC(*f6s_Cleanup)(HF6S hf6s);
F6SRC(*f6s_SetHoldDist)(HF6S hf6s, float fdist);
F6SRC(*f6s_SetForceTorque)(HF6S hf6s, const double force[3], const double torque[3]);
F6SRC(*f6s_GetPositionMatrixGL)(HF6S hf6s, double kineMat[16]);
F6SRC(*f6s_UpdateKinematics)(HF6S hf6s);
F6SRC(*f6s_GetVelocityGL)(HF6S hf6s, double linearVel[3], double angularVel[3]);

// Initialize dhd dll reference count
int cFreedom6SDevice::m_activeFreedom6SDevices = 0;

//===========================================================================
/*!
	Constructor of cFreedom6SDevice.
	Loads interface DLL.

	\fn     cFreedom6SDevice::cFreedom6SDevice()
*/
//===========================================================================
cFreedom6SDevice::cFreedom6SDevice() : cGenericDevice()
{
	m_systemReady = false;
	m_systemAvailable = false;
	m_hf6s = 0;

	m_activeFreedom6SDevices++;

	if (hf6sDLL == NULL)
	{
		hf6sDLL = LoadLibrary("freedom6s.dll");

		if (hf6sDLL == NULL)
			return;

		f6s_Initialize = (F6SRC(*)(HF6S*)) GetProcAddress(hf6sDLL, "f6s_Initialize");
		f6s_ComputeJointVel = (F6SRC(*)(HF6S, float, int)) GetProcAddress(hf6sDLL, "f6s_ComputeJointVel");
		f6s_Cleanup = (F6SRC(*)(HF6S)) GetProcAddress(hf6sDLL, "f6s_Cleanup");
		f6s_SetHoldDist = (F6SRC(*)(HF6S, float)) GetProcAddress(hf6sDLL, "f6s_SetHoldDist");
		f6s_SetForceTorque = (F6SRC(*)(HF6S, const double[3], const double[3])) GetProcAddress(hf6sDLL, "f6s_SetForceTorque");
		f6s_GetPositionMatrixGL = (F6SRC(*)(HF6S, double[16])) GetProcAddress(hf6sDLL, "f6s_GetPositionMatrixGL");
		f6s_UpdateKinematics = (F6SRC(*)(HF6S)) GetProcAddress(hf6sDLL, "f6s_UpdateKinematics");
		f6s_GetVelocityGL = (F6SRC(*)(HF6S hf6s, double[3], double[3])) GetProcAddress(hf6sDLL, "f6s_GetVelocityGL");

		if (!f6s_Initialize
			|| !f6s_ComputeJointVel
			|| !f6s_Cleanup
			|| !f6s_SetHoldDist
			|| !f6s_SetForceTorque
			|| !f6s_GetPositionMatrixGL
			|| !f6s_UpdateKinematics
			|| !f6s_GetVelocityGL)
		{
			FreeLibrary(hf6sDLL);
			hf6sDLL = NULL;
		}
	}

	m_systemAvailable = true;
}

//===========================================================================
/*!
	Destructor of cFreedom6SDevice.

	\fn     cFreedom6SDevice::~cFreedom6SDevice()
*/
//===========================================================================
cFreedom6SDevice::~cFreedom6SDevice()
{
	if (m_hf6s != 0)
		f6s_Cleanup(m_hf6s);
	m_hf6s = 0;

	m_activeFreedom6SDevices--;

	if (m_activeFreedom6SDevices == 0 && hf6sDLL)
	{
		FreeLibrary(hf6sDLL);
		hf6sDLL = NULL;
	}
}

//===========================================================================
/*!
	Open connection to Freedom6S device.

	\fn     int cFreedom6SDevice::open()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cFreedom6SDevice::open()
{
	return 0;
}

//===========================================================================
/*!
	Close connection to Freedom6S device.

	\fn     int cFreedom6SDevice::close()
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cFreedom6SDevice::close()
{
	return 0;
}

//===========================================================================
/*!
	Calibrate Freedom6S device. Initializes the driver, loading appropriate
  settings according to current Freedom6S configuration.

	\fn     int cFreedom6SDevice::initialize(const bool a_resetEncoders=false)
	\param  a_resetEncoders Ignored; exists for forward compatibility.
	\return Return 0 is operation succeeds, -1 if an error occurs.
*/
//===========================================================================
int cFreedom6SDevice::initialize(const bool a_resetEncoders)
{
	if (m_hf6s != 0)
		return -1;

	F6SRC rc = f6s_Initialize(&m_hf6s);
	if (m_hf6s && rc == F6SRC_NOERROR)
	{
		// Joint velocity computation:
		//   timestep = 1ms
		//   sample buffer size = 15
		f6s_ComputeJointVel(m_hf6s, 0.001f, 15);
		return 0;
	}

	m_hf6s = 0;
	return -1;
}

//===========================================================================
/*!
	Set command for the Freedom6S device

	\fn     int cFreedom6SDevice::command(int iCommand, void* iData)
	\param  iCommand    Selected command.
	\param  iData       Pointer to the corresponding data structure.
	\return Return status of command.
*/
//===========================================================================
int cFreedom6SDevice::command(int a_command, void* a_data)
{
	if (m_hf6s == 0)
		return CHAI_MSG_SYSTEM_NOT_READY;

	cVector3d *v;
	double kinemat[16];
	double force[3];
	double torque[3];
	double velLinear[3], velAngular[3];
	torque[0] = torque[1] = torque[2] = 0;

	switch (a_command)
	{
	case CHAI_CMD_GET_POS_3D:
	case CHAI_CMD_GET_POS_NORM_3D:
		f6s_UpdateKinematics(m_hf6s);
		f6s_GetPositionMatrixGL(m_hf6s, kinemat);

		// kinemat is a row-major 4x4 rotation/translation matrix

		v = (cVector3d*)a_data;

		v->x = kinemat[14];
		v->y = kinemat[12];
		v->z = kinemat[13];

		// workspace is approximately from -0.1 to 0.1 meters.
		if (a_command == CHAI_CMD_GET_POS_NORM_3D) {
			v->div(0.1);
		}

		// convert to mm
		else {
			v->mul(1000.0);
		}

		break;

	case CHAI_CMD_SET_FORCE_3D:
		v = (cVector3d*)a_data;

		// coordinates need to be changed for the Freedom6S API
		// torque is assumed to be zero

		force[0] = v->y;
		force[1] = -v->x;
		force[2] = v->z;
		f6s_SetForceTorque(m_hf6s, force, torque);

		break;

	case CHAI_CMD_GET_VEL_3D:
		f6s_GetVelocityGL(m_hf6s, velLinear, velAngular);

		v = (cVector3d*)a_data;
		v->x = velLinear[2];
		v->y = velLinear[0];
		v->z = velLinear[1];

		// Now convert from m/s to mm/s
		v->mul(1000.0);

		break;

		// read scale factor from normalized coords to mm
	case CHAI_CMD_GET_NORMALIZED_SCALE_FACTOR:
	{
		double* scale = (double*)a_data;

		// Multiply .1 to get meters back
		*scale = 0.1;

		// Then multiply by 1000 to get millimeters
		*scale *= 1000.0;
	}
	break;

	default:
		return CHAI_MSG_NOT_IMPLEMENTED;
	}

	return CHAI_MSG_OK;
}
