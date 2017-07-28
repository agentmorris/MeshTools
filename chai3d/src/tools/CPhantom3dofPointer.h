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
#ifndef CPhantom3dofPointerH
#define CPhantom3dofPointerH
//---------------------------------------------------------------------------
#include "CGeneric3dofPointer.h"
#include "CPhantomDevices.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \file       CPhantom3dofPointer.h
	  \class      cPhantom3dofPointer
	  \brief      cPhantom3dofPointer connects the Phantom haptic device as
				  a simple pointer in space. The tool is visually graphically
				  rendered by a small sphere illustrating the tip of the
				  device.
*/
//===========================================================================
class cPhantom3dofPointer : public cGeneric3dofPointer
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cPhantom3dofPointer.
	cPhantom3dofPointer(cWorld* a_world, unsigned int a_deviceNumber = 0, bool dio_access = false);
	//! Destructor of cPhantom3dofPointer.
	virtual ~cPhantom3dofPointer();
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


