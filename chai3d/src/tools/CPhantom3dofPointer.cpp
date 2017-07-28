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
#include "CPhantom3dofPointer.h"
//---------------------------------------------------------------------------

//==========================================================================
/*!
	  Constructor of cPhantom3dofPointer. First parameter is connected
	  to the name of the device that is declared in the control panel and
	  defaults to "Default PHANToM". If the phantom does not start this may
	  be the reason.

	  \fn       cPhantom3dofPointer::cPhantom3dofPointer(cWorld* a_world,
							unsigned int a_deviceNumber, bool dio_access)
	  \param    a_world  World in which the tool will operate.
	  \param    a_deviceNumber  0-based index of the Phantom that this
								tool should talk to.
	  \param    dio_access  If this is 'true', the tool will use the
				Ghost API's direct-i/o model, if it's available.  Otherwise
				the gstEffect i/o model will be used.
*/
//===========================================================================
cPhantom3dofPointer::cPhantom3dofPointer(cWorld* a_world, unsigned int a_deviceNumber, bool dio_access)
	:cGeneric3dofPointer(a_world)
{
	// open phantom driver
#ifndef _DISABLE_PHANTOM_SUPPORT
	m_device = new cPhantomDevice(a_deviceNumber, dio_access);
#else
	m_device = new cGenericDevice();
#endif
}


//==========================================================================
/*!
	  Destructor of cPhantom3dofPointer.

	  \fn       cPhantom3dofPointer::~cPhantom3dofPointer()
*/
//===========================================================================
cPhantom3dofPointer::~cPhantom3dofPointer()
{

}
