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
#include "CFreedom6S3dofPointer.h"
#include "CWorld.h"
//---------------------------------------------------------------------------

//==========================================================================
/*!
	  Constructor of cFreedom6S3dofPointer.

	  \fn       cFreedom6S3dofPointer::cFreedom6S3dofPointer(cWorld* a_world)
	  \param    a_world  World in which the tool will operate.
*/
//===========================================================================
cFreedom6S3dofPointer::cFreedom6S3dofPointer(cWorld* a_world)
	: cGeneric3dofPointer(a_world)
{
	// set a default device for the moment
	m_device = new cFreedom6SDevice();
}


//==========================================================================
/*!
	  Destructor of cFreedom6S3dofPointer.

	  \fn       cFreedom6S3dofPointer::~cFreedom6S3dofPointer()
*/
//===========================================================================
cFreedom6S3dofPointer::~cFreedom6S3dofPointer()
{
}

