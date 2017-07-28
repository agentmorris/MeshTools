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
#ifndef CDraw3DH
#define CDraw3DH
//---------------------------------------------------------------------------
#include "CMacrosGL.h"
#include <GL/glu.h>
//---------------------------------------------------------------------------

//===========================================================================
/*!
	\file CDraw3D.h
	\brief

	The following functions provide useful macros to draw complex 3D objects.

	In general, they don't set nice colors or modify the OpenGL state, they
	just draw polygons.
*/
//===========================================================================

//! Draw an x-y-z frame.
void cDrawFrame(const double a_scale = 1.0, const bool a_modifyMaterialState = true);

//! Draw an x-y-z frame.
void cDrawFrame(const double a_axisLengthScale, const double a_axisThicknessScale,
	const bool a_modifyMaterialState);

//! Draw a box using lines.
void cDrawWireBox(const double a_xMin, const double a_xMax,
	const double a_yMin, const double a_yMax,
	const double a_zMin, const double a_zMax);

//! Draw a sphere.
void cDrawSphere(const double a_radius,
	const unsigned int a_numSlices = 10, const unsigned int a_numStacks = 10);

//! Draw a pretty arrow on the z-axis using a cone and a cylinder (using GLUT)
void cDrawArrow(const cVector3d& a_arrowStart, const cVector3d& a_arrowTip, const double a_width = 0.05);

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


