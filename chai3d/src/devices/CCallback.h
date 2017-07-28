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
	\author:    Dan Morris
	\version    1.0
	\date       03/2006
*/
//===========================================================================

#ifndef CCallbackH
#define CCallbackH

//===========================================================================
/*!
	\file   CCallback.h
	\class  cCallback
	\brief  cCallback is an abstract class that allows subclasses
	to define a single callback function, for example to be called by
	a device when it's time to compute haptic forces.  This feature is _not_
	supported by all devices; see cGenericDevice::setCallback().
*/
//===========================================================================
class cCallback
{
public:
	cCallback() {}
	virtual ~cCallback() {}
	virtual void callback() = 0;
};

#endif


