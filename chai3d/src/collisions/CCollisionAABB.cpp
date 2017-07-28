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
#include "CCollisionAABB.h"
//---------------------------------------------------------------------------

//! Pointer to first free location in array of AABB tree nodes.
cCollisionAABBInternal* g_nextFreeNode;

//===========================================================================
/*!
	Constructor of cCollisionAABB.

	\fn       cCollisionAABB::cCollisionAABB(vector<cTriangle> *a_triangles, bool a_useNeighbors)
	\param    a_triangles     Pointer to array of triangles.
	\param    a_useNeighbors  Use neighbor lists to speed up collision detection?
*/
//===========================================================================
cCollisionAABB::cCollisionAABB(vector<cTriangle> *a_triangles, bool a_useNeighbors)
{
	// list of triangles used when building the tree
	m_triangles = a_triangles;

	// initialize members
	m_root = NULL;
	m_leaves = NULL;
	m_numTriangles = 0;
	m_useNeighbors = a_useNeighbors;
}


//===========================================================================
/*!
	Destructor of cCollisionAABB.

	\fn       cCollisionAABB::~cCollisionAABB()
*/
//===========================================================================
cCollisionAABB::~cCollisionAABB()
{

	// clear collision tree
	if (m_root != NULL) delete[] m_root;

	// Delete the allocated array of leaf nodes
	//
	// If there's only one triangle, m_root = m_leaves
	// and we've already deleted the leaves...
	if (m_numTriangles > 1)
		if (m_leaves) delete[] m_leaves;
}


//===========================================================================
/*!
	Build the Axis-Aligned Bounding Box collision-detection tree.  Each
	leaf is associated with one triangle and with a bounding box of minimal
	dimensions such that it fully encloses the triangle and is aligned with
	the coordinate axes (no rotations).  Each internal node is associated
	with a bounding box of minimal dimensions such that it fully encloses
	the bounding boxes of its two children and is aligned with the axes.

	\fn       void cCollisionAABB::initialize()
*/
//===========================================================================
void cCollisionAABB::initialize()
{
	unsigned int i;
	m_lastCollision = NULL;

	// if a previous tree was created, delete it
	if (m_root != NULL)
	{
		delete[] m_root;
	}

	// reset triangle counter
	m_numTriangles = 0;

	// count the number of allocated triangles that will be used to create
	// the tree.
	for (i = 0; i < m_triangles->size(); ++i)
	{
		cTriangle* nextTriangle = &(*m_triangles)[i];
		if (nextTriangle->allocated())
		{
			m_numTriangles++;
		}
	}

	// check if the number of triangles is equal to zero
	if (m_numTriangles == 0)
	{
		m_root = NULL;
		return;
	}

	// create a leaf node for each triangle
	m_leaves = new cCollisionAABBLeaf[m_numTriangles];
	for (i = 0; i < m_numTriangles; ++i)
	{
		cTriangle* nextTriangle = &(*m_triangles)[i];
		if (nextTriangle->allocated())
		{
			new(&(m_leaves[i])) cCollisionAABBLeaf(nextTriangle);
		}
	}

	// allocate an array to hold all internal nodes of the binary tree
	if (m_numTriangles >= 2)
	{
		g_nextFreeNode = new cCollisionAABBInternal[m_numTriangles];
		m_root = g_nextFreeNode;
		new(g_nextFreeNode++) cCollisionAABBInternal(m_numTriangles, m_leaves, 0);
	}

	// there is only one triangle, so the tree consists of just one leaf
	else
	{
		m_root = &m_leaves[0];
	}

	// assign parent relationships in the tree
	m_root->setParent(0, 1);
}


//===========================================================================
/*!
	Check if the given line segment intersects any triangle of the mesh.  If so,
	return true, as well as (through the output parameters) pointers to the
	intersected triangle, the mesh of which this triangle is a part, the point
	of intersection, and the distance from the origin of the segment to the
	collision point.  If more than one triangle is intersected, return the one
	closest to the origin of the segment.  The method uses the pre-computed
	AABB boxes, starting at the root and recursing through the tree, breaking
	the recursion along any path in which the bounding box of the line segment
	does not intersect the bounding box of the node.  At the leafs,
	triangle-segment intersection testing is called.

	\fn       bool cCollisionAABB::computeCollision(cVector3d& a_segmentPointA,
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
bool cCollisionAABB::computeCollision(cVector3d& a_segmentPointA,
	cVector3d& a_segmentPointB,
	cGenericObject*& a_colObject,
	cTriangle*& a_colTriangle,
	cVector3d& a_colPoint,
	double& a_colSquareDistance,
	int a_proxyCall)
{
	// convert two point segment into a segment described by a point and
	// a directional vector
	cVector3d dir;
	a_segmentPointB.subr(a_segmentPointA, dir);

	// if this is a subsequent call from the proxy algorithm after detecting
	// an initial collision, and if the flag to use neighbor checking is set,
	// only neighbors of the triangle from the first collision detection
	// need to be checked
	if ((m_useNeighbors) && (a_proxyCall > 1) && (m_root != NULL) &&
		(m_lastCollision != NULL) && (m_lastCollision->m_neighbors != NULL))
	{

		// initialize temp variables for output parameters
		cGenericObject* colObject;
		cTriangle* colTriangle;
		cVector3d colPoint;
		double colSquareDistance = dir.lengthsq();
		bool firstHit = true;

		// check each neighbor, and find the closest for which there is a
		// collision, if any
		unsigned int ntris = m_lastCollision->m_neighbors->size();
		std::vector<cTriangle*>* neighbors = m_lastCollision->m_neighbors;
		for (unsigned int i = 0; i < ntris; i++)
		{
			cTriangle* tri = (*neighbors)[i];
			if (tri == 0) {
				CHAI_DEBUG_PRINT("Oops... invalid neighbor\n");
				continue;
			}
			if (tri->computeCollision(
				a_segmentPointA, dir, colObject, colTriangle,
				colPoint, colSquareDistance))
			{
				// if this intersected triangle is closer to the segment origin
				// than any other found so far, set the output parameters
				if (firstHit || (colSquareDistance < a_colSquareDistance))
				{
					m_lastCollision = colTriangle;
					a_colObject = colObject;
					a_colTriangle = colTriangle;
					a_colPoint = colPoint;
					a_colSquareDistance = colSquareDistance;
					firstHit = false;
				}
			}
		}

		// if at least one neighbor triangle was intersected, return true
		if (!firstHit)  return true;

		// otherwise there was no collision; return false
		if (a_proxyCall != -1) m_lastCollision = NULL;
		return false;
	}

	// otherwise, if this is the first call in an iteration of the proxy
	// algorithm (or a call from any other algorithm), check the AABB tree

	// if the root is null, the tree is empty, so there can be no collision
	if (m_root == NULL)
	{
		if (a_proxyCall != -1) m_lastCollision = NULL;
		return (false);
	}

	// create an axis-aligned bounding box for the line
	cCollisionAABBBox lineBox;
	lineBox.setEmpty();
	lineBox.enclose(a_segmentPointA);
	lineBox.enclose(a_segmentPointB);

	// test for intersection between the line segment and the root of the
	// collision tree; the root will recursively call children down the tree
	a_colSquareDistance = dir.lengthsq();
	bool result = m_root->computeCollision(a_segmentPointA, dir, lineBox,
		a_colTriangle, a_colPoint, a_colSquareDistance);

	// if there was a collision, set m_lastCollision to the intersected triangle
	// returned by the call to the root of the tree, and set the output
	// parameter for the intersected mesh to the parent of this triangle
	if (result)
	{
		if (a_proxyCall != -1) m_lastCollision = a_colTriangle;
		a_colObject = a_colTriangle->getParent();
	}
	else
	{
		if (a_proxyCall != -1) m_lastCollision = NULL;
	}

	// return whether there was an intersection
	return result;
}


//===========================================================================
/*!
	Render the bounding boxes of the collision tree in OpenGL.

	\fn       void cCollisionAABB::render()
*/
//===========================================================================
void cCollisionAABB::render()
{
	if (m_root != NULL)
	{
		// set rendering settings
		glDisable(GL_LIGHTING);
		glLineWidth(1.0);
		glColor4fv(m_material.m_ambient.pColor());

		// render tree by calling the root, which recursively calls the children
		m_root->render(m_displayDepth);

		// restore lighting settings
		glEnable(GL_LIGHTING);
	}
}



