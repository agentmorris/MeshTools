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
#include "CDelta3dofPointer.h"
//---------------------------------------------------------------------------

//==========================================================================
/*!
	  Constructor of cDelta3dofPointer.

	  \fn       cDelta3dofPointer::cDelta3dofPointer(cWorld* a_world,
				unsigned int a_deviceNumber)
	  \param    a_world  World in which the tool will operate.
	  \param    a_deviceNumber  ID Number of the device to connect to.
*/
//===========================================================================
cDelta3dofPointer::cDelta3dofPointer(cWorld* a_world, unsigned int a_deviceNumber)
	: cGeneric3dofPointer(a_world)
{
#ifndef _DISABLE_DELTA_SUPPORT
	// open delta driver
	m_device = new cDeltaDevice();
#else
	m_device = new cGenericDevice();
#endif
}


//==========================================================================
/*!
	  Destructor of cDelta3dofPointer.

	  \fn       cDelta3dofPointer::~cDelta3dofPointer()
*/
//===========================================================================
cDelta3dofPointer::~cDelta3dofPointer()
{
}
