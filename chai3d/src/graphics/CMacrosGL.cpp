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
	\author:    Dan Morris
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CMacrosGL.h"
#include "CVector3d.h"
#include "CMaths.h"

//---------------------------------------------------------------------------
void cLookAt(const cVector3d& a_eye, const cVector3d& a_at, const cVector3d& a_up)
{

	// Define our look vector (z axis)
	cVector3d look = a_at - a_eye;
	look.normalize();

	// Define our new x axis
	cVector3d xaxis;
	xaxis = cCross(look, a_up);
	xaxis.normalize();

	// Define our new y axis as the cross of the x and z axes
	cVector3d upv = cCross(xaxis, look);

	// Turn around the z axis
	look.mul(-1.0);

	// Put it all into a GL-friendly matrix
	double dm[16];
	dm[0] = xaxis.x;
	dm[1] = xaxis.y;
	dm[2] = xaxis.z;
	dm[3] = 0.f;
	dm[4] = upv.x;
	dm[5] = upv.y;
	dm[6] = upv.z;
	dm[7] = 0.f;
	dm[8] = look.x;
	dm[9] = look.y;
	dm[10] = look.z;
	dm[11] = 0.f;
	dm[12] = a_eye.x;
	dm[13] = a_eye.y;
	dm[14] = a_eye.z;
	dm[15] = 1.f;

	// Push it onto the matrix stack
	glMultMatrixd(dm);

}
