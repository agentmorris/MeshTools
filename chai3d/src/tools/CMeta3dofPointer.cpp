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
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CMeta3dofPointer.h"
#include "CMacrosGL.h"
#ifdef _WIN32
#include <process.h>
#endif

//---------------------------------------------------------------------------

//==========================================================================
/*!
	  Constructor of cMeta3dofPointer.

	  \fn       cMeta3dofPointer::cMeta3dofPointer(cWorld* a_world,
								unsigned int a_deviceNumber, bool dio_access)
	  \param    a_world  World in which the tool will operate.
	  \param    a_deviceNumber  0-based index used to try to open a phantom
								device.

	  \param    dio_access  Also used only if the tool ends up being a phantom AND
				you're running GHOST, not OpenHaptics.  If you're not using a Phantom
				or you're using OpenHaptics, ignore this parameter.

				For Ghost, if dio_access is 'true', the tool will use the Ghost API's
				direct-i/o model, if it's available.  Otherwise the gstEffect i/o model
				will be used.
*/
//===========================================================================
cMeta3dofPointer::cMeta3dofPointer(cWorld* a_world, unsigned int a_deviceNumber, bool a_dio_access)
	:cGeneric3dofPointer(a_world)
{
	bool systemAvailable;

	m_physicalDevice = -1;

	// Allow Delta support to be compiled out
#ifndef _DISABLE_DELTA_SUPPORT

	// try to open a connection to a Delta or Omega haptic device
	m_device = new cDeltaDevice(a_deviceNumber);
	systemAvailable = m_device->isSystemAvailable();

	if (systemAvailable)
	{
		m_device->open();
		m_physicalDevice = DEVICE_DHD;
		return;
	}
	else
	{
		delete m_device;
		m_device = NULL;
	}
#endif

	// Allow Phantom support to be compiled out
#ifndef _DISABLE_PHANTOM_SUPPORT

	// try to open phantom driver
	m_device = new cPhantomDevice(a_deviceNumber, a_dio_access);
	systemAvailable = m_device->isSystemAvailable();

	if (systemAvailable)
	{
		m_device->open();
		m_physicalDevice = DEVICE_PHANTOM;
		return;
	}
	else
	{
		delete m_device;
		m_device = NULL;
	}

#endif

	// No linux support yet for the virtual device or the f6s...

#ifdef _WIN32

	// try to open Freedom6S device
	m_device = new cFreedom6SDevice();
	systemAvailable = m_device->isSystemAvailable();

	if (systemAvailable)
	{
		m_device->open();
		m_physicalDevice = DEVICE_MPB;
		return;
	}
	else
	{
		delete m_device;
		m_device = NULL;
	}

#endif

	// No Linux virtual device yet...
#ifdef _WIN32
	// try to open a virtual haptic device
	m_device = new cVirtualDevice();
	systemAvailable = m_device->isSystemAvailable();

	// Is the virtual device already running?
	if (systemAvailable)
	{
		m_device->open();
		m_physicalDevice = DEVICE_VIRTUAL;
		return;
	}

	// The virtual device is not running yet...
	else
	{
		// launch a virtual device
		m_device->close();
		delete m_device;
		m_device = NULL;

		// dvdVirtual.exe is expected to be in the system path
		char* path = getenv("PATH");
		char* chai_base = getenv("CHAI_BASE");

		int result = _spawnlp(_P_NOWAIT, DHD_VIRTUAL_EXE_NAME, DHD_VIRTUAL_EXE_NAME, NULL);
		if (result == -1) {

			// Try to open it in the chai_base directory, which may not be in
			// the user's path.
			if (chai_base != 0) {

				// Remove any trailing '\'
				char chaibase_copy[_MAX_PATH];
				strcpy(chaibase_copy, chai_base);
				if (chaibase_copy[strlen(chaibase_copy) - 1] == '\\')
					chaibase_copy[strlen(chaibase_copy) - 1] = '\0';

				// Put together the new path
				char newpath[_MAX_PATH];
				sprintf(newpath, "%s\\bin\\%s", chaibase_copy, DHD_VIRTUAL_EXE_NAME);

				// Try again...
				result = _spawnlp(_P_NOWAIT, newpath, DHD_VIRTUAL_EXE_NAME, NULL);
			}
		}

#define MAX_VIRTUAL_DEVICE_SLEEP_CYCLES 20

		// Wait for the virtual delta process to start up...
		for (int i = 0; i < MAX_VIRTUAL_DEVICE_SLEEP_CYCLES; i++)
		{
			m_device = new cVirtualDevice();
			systemAvailable = m_device->isSystemAvailable();
			if (systemAvailable) break;
			delete m_device;
			m_device = 0;
			Sleep(50);
		}

		if (!systemAvailable)
		{
			CHAI_DEBUG_PRINT("Could not open virtual device...\n");
			if (m_device)
			{
				m_device->close();
				delete m_device;
			}
			m_device = NULL;
			return;
		}
		else
		{
			m_physicalDevice = DEVICE_VIRTUAL;
		}
	}

	// Linux...
#else
	m_device = NULL;
	return;
#endif

}


//==========================================================================
/*!
	  Destructor of cMeta3dofPointer.

	  \fn       cMeta3dofPointer::~cMeta3dofPointer()
*/
//===========================================================================
cMeta3dofPointer::~cMeta3dofPointer()
{
}
