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
	\date       05/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CVector3d.h"
#include "CMaths.h"
//---------------------------------------------------------------------------


//-----------------------------------------------------------------------
/*!
	Returns the squared distance from this segment to a_point and the
	position along the segment (from 0.0 to 1.0) of the closest point

	\fn     cSegment3D::distanceSquaredToPoint(const cVector3d& a_point, double& a_t, cVector3d* a_closestPoint)
	\param    a_point Point to test
	\param    a_t return value for the position along the segment
	\param    a_closestPoint The closest point on this segment to the supplied point
	\return   The distance from a_point to this segment
*/
//-----------------------------------------------------------------------
double cSegment3D::distanceSquaredToPoint(const cVector3d& a_point, double& a_t, cVector3d* a_closestPoint)
{
	double mag = cDistance(m_start, m_end);

	// Project this point onto the line
	a_t = (a_point - m_start) * (m_end - m_start) / (mag * mag);

	// Clip to segment endpoints
	if (a_t < 0.0) a_t = 0.0;
	else if (a_t > 1.0) a_t = 1.0;

	// Find the intersection point
	cVector3d intersection = m_start + a_t * (m_end - m_start);

	if (a_closestPoint) *a_closestPoint = intersection;

	// Compute distance
	return cDistanceSq(a_point, intersection);
}
