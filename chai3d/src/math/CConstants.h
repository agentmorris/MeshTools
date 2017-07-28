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
#ifndef CConstantsH
#define CConstantsH
//---------------------------------------------------------------------------

//===========================================================================
/*!
	\file CConstants.h
	\brief
	Definitions for often used mathematical constants.
*/
//===========================================================================
//! PI constant.
#define CHAI_PI                 3.14159265358979323846
//! Conversion from degrees to radians.
#define CHAI_DEG2RAD	        0.01745329252
//! Conversion from radians to degrees.
#define CHAI_RAD2DEG	        57.2957795131
//! Smallest value near zero for a double.
#define CHAI_TINY               1e-49
//! Small value near zero.
#define CHAI_SMALL              0.000000001
//! Biggest value for a double.
#define CHAI_LARGE              1e+49


//===========================================================================
/*!
	\brief
	Definition for often used three dimentional vectors, such as origin and unit
	vectors in the principal directions.
*/
//===========================================================================
//! Zero vector (0,0,0)
#define CHAI_VECTOR_ZERO        cVector3d(0, 0, 0);
//! Unit vector along Axis X (1,0,0)
#define CHAI_VECTOR_X           cVector3d(1, 0, 0);
//! Unit vector along Axis Y (0,1,0)
#define CHAI_VECTOR_Y           cVector3d(0, 1, 0);
//! Unit vector along Axis Z (0,0,1)
#define CHAI_VECTOR_Z           cVector3d(0, 0, 1);
//! Origin (0,0,0)
#define CHAI_ORIGIN             cVector3d(0, 0, 0);


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
