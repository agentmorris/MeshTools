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
	\author:    Chris Sewell
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CVertex.h"
#include "CTriangle.h"
#include "CCollisionBrute.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Check if the given line segment intersects any triangle of the mesh. This
	method is called "brute force" because all triangles are checked by
	invoking their collision-detection methods.  This method is simple but very
	inefficient.

	\fn       bool cCollisionBrute::computeCollision(cVector3d& a_segmentPointA,
			  cVector3d& a_segmentPointB, cGenericObject*& a_colObject,
			  cTriangle*& a_colTriangle, cVector3d& a_colPoint,
			  double& a_colSquareDistance, int a_proxyCall)
	\param    a_segmentPointA  Initial point of segment.
	\param    a_segmentPointB  End point of segment.
	\param    a_colObject  Returns pointer to nearest collided object.
	\param    a_colTriangle Returns pointer to nearest collided triangle.
	\param    a_colPoint  Returns position of nearest collision.
	\param    a_colSquareDistance  Returns distance between ray origin and
								   collision point.
	\param    a_proxyCall  If this is > 0, this is a call from a proxy, and the
						   value of a_proxyCall specifies which call this is.
						   When checking for the second and third constraint
						   planes, only the neighbors of the triangle intersected
						   in the first call need be checked, not the whole tree.
						   Call with a_proxyCall = -1 for non-proxy calls.
	\return   Return true if the line segment intersects a triangle.
*/
//===========================================================================
bool cCollisionBrute::computeCollision(cVector3d& a_segmentPointA,
	cVector3d& a_segmentPointB,
	cGenericObject*& a_colObject,
	cTriangle*& a_colTriangle,
	cVector3d& a_colPoint,
	double& a_colSquareDistance,
	int a_proxyCall)
{
	// temp variables for storing results
	cGenericObject* colObject;
	cTriangle* colTriangle;
	cVector3d colPoint;
	bool hit = false;

	// convert two point segment into a segment described by a point and
	// a directional vector
	cVector3d dir;
	a_segmentPointB.subr(a_segmentPointA, dir);

	// compute the squared length of the segment
	double colSquareDistance = dir.lengthsq();

	// check all triangles for collision and return the nearest one
	unsigned int ntriangles = m_triangles->size();
	for (unsigned int i = 0; i < ntriangles; i++)
	{

		// check for a collision between this triangle and the segment by
		// calling the triangle's collision detection method; it will only
		// return true if the distance between the segment origin and this
		// triangle is less than the current closest intersecting triangle
		// (whose distance squared is kept in colSquareDistance)
		if ((*m_triangles)[i].computeCollision(
			a_segmentPointA, dir, colObject, colTriangle, colPoint, colSquareDistance))
		{
			a_colObject = colObject;
			a_colTriangle = colTriangle;
			a_colPoint = colPoint;
			a_colSquareDistance = colSquareDistance;
			hit = true;
		}
	}

	// return result
	return (hit);
}



