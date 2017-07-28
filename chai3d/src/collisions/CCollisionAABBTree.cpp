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
#include "CCollisionAABBTree.h"
//---------------------------------------------------------------------------

//! Pointer for creating new AABB tree nodes, declared in CCollisionAABB.cpp.
extern cCollisionAABBInternal* g_nextFreeNode;

//===========================================================================
/*!
	Determine whether the two given boxes intersect each other.

	\fn       bool intersect(const cCollisionAABBBox& a_0, const cCollisionAABBBox& a_1)
	\param    a_0   First box; may intersect with second box.
	\param    a_1   Second box; may intersect with first box.
	\return   Return whether there is any overlap of the two boxes.
*/
//===========================================================================
inline bool intersect(const cCollisionAABBBox& a_0, const cCollisionAABBBox& a_1)
{
	// check for overlap along each axis
	if (a_0.getLowerX() > a_1.getUpperX()) return false;
	if (a_0.getLowerY() > a_1.getUpperY()) return false;
	if (a_0.getLowerZ() > a_1.getUpperZ()) return false;
	if (a_1.getLowerX() > a_0.getUpperX()) return false;
	if (a_1.getLowerY() > a_0.getUpperY()) return false;
	if (a_1.getLowerZ() > a_0.getUpperZ()) return false;

	// if the boxes are not separated along any axis, a collision has occurred
	return true;
}


//===========================================================================
/*!
	Render bounding box of leaf node if it is at level a_depth in the tree.

	\fn       void cCollisionAABBLeaf::render(int a_depth)
	\param    a_depth  Only draw nodes at this depth in the tree.
					   a_depth < 0 render _up to_ abs(a_depth).
*/
//===========================================================================
void cCollisionAABBLeaf::render(int a_depth)
{
	if (((a_depth < 0) && (abs(a_depth) >= m_depth)) || a_depth == m_depth)
	{
		if (a_depth < 0)
		{
			cColorf c(1.0, 0.0, 0.0, 1.0);
			glColor4fv(c.pColor());
		}
		m_bbox.render();
	}
}


//===========================================================================
/*!
	  Create a bounding box to enclose the three vertices of the triangle
	  belonging to the leaf node.

	  \fn       void cCollisionAABBLeaf::fitBBox()
*/
//===========================================================================
void cCollisionAABBLeaf::fitBBox()
{
	// empty box
	m_bbox.setEmpty();

	// enclose all three vertices of triangle
	if (m_triangle != NULL)
	{
		m_bbox.enclose(m_triangle->getVertex0()->getPos());
		m_bbox.enclose(m_triangle->getVertex1()->getPos());
		m_bbox.enclose(m_triangle->getVertex2()->getPos());
	}
}


//===========================================================================
/*!
	Determine whether the given line intersects the triangle belonging to
	this leaf node by calling the triangle's collision detection method.

	\fn       bool cCollisionAABBLeaf::computeCollision(cVector3d& a_segmentPointA,
			  cVector3d& a_segmentDirection, cCollisionAABBBox &a_lineBox,
			  cTriangle*& a_colTriangle, cVector3d& a_colPoint, double& a_colSquareDistance)
	\param    a_segmentPointA  Initial point of segment.
	\param    a_segmentDirection  Direction of ray from first to second
								  segment points (i.e., proxy to goal).
	\param    a_lineBox  A bounding box for the incoming segment, for quick
						 discarding of collision tests.
	\param    a_colTriangle  Returns pointer to nearest collided triangle.
	\param    a_colPoint  Returns position of nearest collision.
	\param    a_colSquareDistance  Returns distance between ray origin and
								   collision point.
	\return   Return true if the line segment intersects the leaf's triangle.
*/
//===========================================================================
bool cCollisionAABBLeaf::computeCollision(cVector3d& a_segmentPointA,
	cVector3d& a_segmentDirection,
	cCollisionAABBBox &a_lineBox,
	cTriangle*& a_colTriangle,
	cVector3d& a_colPoint,
	double& a_colSquareDistance)
{
	// check for a collision between this leaf's triangle and the segment by
	// calling the triangle's collision detection method; it will only
	// return true if the distance between the segment origin and this
	// triangle is less than the current closest intersecting triangle
	// (whose distance squared is kept in colSquareDistance)
	cGenericObject* colObject;
	bool result = m_triangle->computeCollision(a_segmentPointA, a_segmentDirection,
		colObject, a_colTriangle, a_colPoint, a_colSquareDistance);

	// return result
	return (result);
}


//===========================================================================
/*!
	Destructor of cCollisionAABBInternal.

	\fn       cCollisionAABBInternal::~cCollisionAABBInternal()
*/
//===========================================================================
cCollisionAABBInternal::~cCollisionAABBInternal()
{

	// Note that we don't delete leaf nodes here; they're stored in one big
	// array and can't be deleted individually...
}


//===========================================================================
/*!
	Draw the edges of the bounding box for an internal tree node if it is
	at depth a_depth in the tree, and call the draw function for its children.

	\fn       void cCollisionAABBInternal::render(int a_depth)
	\param    a_depth   Only draw nodes at this level in the tree.
						a_depth < 0 render _up to_ this level.
*/
//===========================================================================
void cCollisionAABBInternal::render(int a_depth)
{
	// render current node
	if (((a_depth < 0) && (abs(a_depth) >= m_depth)) || a_depth == m_depth)
	{
		if (a_depth < 0)
		{
			cColorf c(1.0, 0.0, 0.0, 1.0);
			glColor4fv(c.pColor());
		}
		m_bbox.render();
	}

	// render left sub tree
	m_leftSubTree->render(a_depth);

	// render right sub tree
	m_rightSubTree->render(a_depth);
}


//===========================================================================
/*!
	Create an internal AABB tree node.

	\fn       cCollisionAABBInternal::cCollisionAABBInternal(
			  unsigned int a_numLeaves, cCollisionAABBLeaf *a_leaves,
			  unsigned int a_depth)
	\param    a_numLeaves  Number of leaves in subtree rooted at this node.
	\param    a_leaves  Pointer to the location in the array of leafs for the
						first leaf under this internal node.
	\param    a_depth  Depth of this node in the collision tree.
	\return   Return a pointer to a new cCollisionAABBInternal node.
*/
//===========================================================================
cCollisionAABBInternal::cCollisionAABBInternal(unsigned int a_numLeaves,
	cCollisionAABBLeaf *a_leaves,
	unsigned int a_depth)
{
	// set depth of this node and initialize left and right subtree pointers
	m_depth = a_depth;
	m_leftSubTree = NULL;
	m_rightSubTree = NULL;
	m_testLineBox = true;

	// create a box to enclose all the leafs below this internal node
	m_bbox.setEmpty();
	for (unsigned int j = 0; j < a_numLeaves; ++j)
	{
		m_bbox.enclose(a_leaves[j].m_bbox);
	}

	// move leafs with smaller coordinates (on the longest axis) towards the
	// beginning of the array and leaves with larger coordinates towards the
	// end of the array
	int axis = m_bbox.longestAxis();
	unsigned int i = 0;
	unsigned int mid = a_numLeaves;
	while (i < mid)
	{
		if (a_leaves[i].m_bbox.getCenter().get(axis) < m_bbox.getCenter().get(axis))
		{
			++i;
		}
		else
		{
			std::swap(a_leaves[i], a_leaves[--mid]);
		}
	}

	// we expect mid, used as the right iterator in the "insertion sort" style
	// rearrangement above, to have moved roughly to the middle of the array;
	// however, if it never moved left or moved all the way left, set it to
	// the middle of the array so that neither the left nor right subtree will
	// be empty
	if (mid == 0 || mid == a_numLeaves)
	{
		mid = a_numLeaves / 2;
	}

	// if the right subtree contains multiple triangles, create new internal node
	if (mid >= 2)
	{
		m_rightSubTree = g_nextFreeNode;
		new(g_nextFreeNode++) cCollisionAABBInternal(mid, &a_leaves[0], m_depth + 1);
	}

	// if there is only one triangle in the right subtree, the right subtree
	// pointer should just point to the leaf node
	else
	{
		m_rightSubTree = &a_leaves[0];
		if (m_rightSubTree != NULL) m_rightSubTree->m_depth = m_depth + 1;
	}

	// if the left subtree contains multiple triangles, create new internal node
	if (a_numLeaves - mid >= 2)
	{
		m_leftSubTree = g_nextFreeNode;
		new(g_nextFreeNode++) cCollisionAABBInternal(a_numLeaves - mid, &a_leaves[mid], m_depth + 1);
	}

	// if there is only one triangle in the left subtree, the left subtree
	// pointer should just point to the leaf node
	else
	{
		m_leftSubTree = &a_leaves[mid];
		if (m_leftSubTree) m_leftSubTree->m_depth = m_depth + 1;
	}
}


//===========================================================================
/*!
	Determine whether the given ray intersects the bounding box.  Based on code
	by Andrew Woo from "Graphics Gems", Academic Press, 1990.

	\fn       bool hitBoundingBox(double a_minB[3], double a_maxB[3],
								  double a_origin[3], double a_dir[3])
	\param    a_minB[3]   Minimum coordinates (along each axis) of bounding box.
	\param    a_maxB[3]   Maximum coordinates (along each axis) of bounding box.
	\param    a_origin[3] Origin of the ray.
	\param    a_dir[3]    Direction of the ray.
	\return   Return true if line segment intersects the bounding box.
*/
//===========================================================================
bool hitBoundingBox(const double a_minB[3], const double a_maxB[3], const double a_origin[3], const double a_dir[3])
{
	const int RIGHT = 0;
	const int LEFT = 1;
	const int MIDDLE = 2;

	double coord[3];
	char inside = true;
	char quadrant[3];
	register int i;
	int whichPlane;
	double maxT[3];
	double candidatePlane[3];

	// Find candidate planes; this loop can be avoided if
	// rays cast all from the eye (assume perspective view)
	for (i = 0; i < 3; i++)
	{
		if (a_origin[i] < a_minB[i])
		{
			quadrant[i] = LEFT;
			candidatePlane[i] = a_minB[i];
			inside = false;
		}
		else if (a_origin[i] > a_maxB[i])
		{
			quadrant[i] = RIGHT;
			candidatePlane[i] = a_maxB[i];
			inside = false;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
	}

	// Ray origin inside bounding box
	if (inside)
	{
		//coord = origin;
		return (true);
	}

	// Calculate T distances to candidate planes
	for (i = 0; i < 3; i++)
	{
		if (quadrant[i] != MIDDLE && a_dir[i] != 0.)
			maxT[i] = (candidatePlane[i] - a_origin[i]) / a_dir[i];
		else
			maxT[i] = -1.;
	}

	// Get largest of the maxT's for final choice of intersection
	whichPlane = 0;
	for (i = 1; i < 3; i++)
		if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	// Check final candidate actually inside box
	if (maxT[whichPlane] < 0.) return (false);
	for (i = 0; i < 3; i++)
	{
		if (whichPlane != i)
		{
			coord[i] = a_origin[i] + maxT[whichPlane] * a_dir[i];
			if (coord[i] < a_minB[i] || coord[i] > a_maxB[i])
				return (false);
		}
		else
		{
			coord[i] = candidatePlane[i];
		}
	}

	// Ray hits box...
	return (true);
}


//===========================================================================
/*!
	Determine whether the given line intersects the mesh covered by the
	AABB Tree rooted at this internal node.  If so, return (in the output
	parameters) information about the intersected triangle of the mesh closest
	to the segment origin.

	\fn       bool cCollisionAABBInternal::computeCollision(
			  cVector3d& a_segmentPointA, cVector3d& a_segmentDirection,
			  cCollisionAABBBox &a_lineBox, cTriangle*& a_colTriangle,
			  cVector3d& a_colPoint, double& a_colSquareDistance)
	\param    a_segmentPointA  Initial point of segment.
	\param    a_segmentDirection  Direction of ray from first to second
								  segment points (i.e., proxy to goal).
	\param    a_lineBox  A bounding box for the incoming segment, for quick
						 discarding of collision tests.
	\param    a_colTriangle  Returns pointer to nearest collided triangle.
	\param    a_colPoint  Returns position of nearest collision.
	\param    a_colSquareDistance  Returns distance between ray origin and
								   collision point.
	\return   Return true if line segment intersects a triangle in the subtree.
*/
//===========================================================================
bool cCollisionAABBInternal::computeCollision(cVector3d& a_segmentPointA,
	cVector3d& a_segmentDirection,
	cCollisionAABBBox &a_lineBox,
	cTriangle*& a_colTriangle,
	cVector3d& a_colPoint,
	double& a_colSquareDistance)
{
	// if a line's bounding box does not intersect the node's bounding box,
	// there can be no intersection
	if (!intersect(m_bbox, a_lineBox))
	{
		return (false);
	}

	if (m_testLineBox)
	{
		// Avoid unnecessary copying by casting straight to double...
		//double minB[3];
		//double maxB[3];
		//double origin[3];
		//double dir[3];
		//minB[0] = m_bbox.getLowerX(); minB[1] = m_bbox.getLowerY(); minB[2] = m_bbox.getLowerZ();
		//maxB[0] = m_bbox.getUpperX(); maxB[1] = m_bbox.getUpperY(); maxB[2] = m_bbox.getUpperZ();
		//origin[0] = a_segmentPointA.x; origin[1] = a_segmentPointA.y; origin[2] = a_segmentPointA.z;
		//dir[0] = a_segmentDirection.x; dir[1] = a_segmentDirection.y; dir[2] = a_segmentDirection.z;
		//if (!hitBoundingBox(minB, maxB, origin, dir))
		if (!hitBoundingBox(
			(const double*)(&m_bbox.m_min),
			(const double*)(&m_bbox.m_max),
			(const double*)(&a_segmentPointA),
			(const double*)(&a_segmentDirection)))
			return (false);
	}

	// initialize objects for calls to left and right subtrees
	cTriangle *l_colTriangle, *r_colTriangle;
	cVector3d l_colPoint, r_colPoint;
	double l_colSquareDistance = a_colSquareDistance;
	double r_colSquareDistance = a_colSquareDistance;
	bool l_result = false;
	bool r_result = false;

	// check collision between line and left subtree node; it will only
	// return true if the distance between the segment origin and this
	// triangle is less than the current closest intersecting triangle
	// (whose distance squared is in l_colSquareDistance)
	if (m_leftSubTree && m_leftSubTree->computeCollision(a_segmentPointA,
		a_segmentDirection, a_lineBox, l_colTriangle, l_colPoint,
		l_colSquareDistance))
	{
		l_result = true;
	}

	// check collision between line and right subtree node; it will only
	// return true if the distance between the segment origin and this
	// triangle is less than the current closest intersecting triangle
	// (whose distance squared is in r_colSquareDistance)
	if (m_rightSubTree && m_rightSubTree->computeCollision(a_segmentPointA,
		a_segmentDirection, a_lineBox, r_colTriangle, r_colPoint,
		r_colSquareDistance))
	{
		r_result = true;
	}

	// if there is an intersection in either subtree, return the closest one
	if ((l_result && !r_result) || (l_result && r_result &&
		(l_colSquareDistance <= r_colSquareDistance)))
	{
		a_colTriangle = l_colTriangle;
		a_colPoint = l_colPoint;
		a_colSquareDistance = l_colSquareDistance;
	}
	else
	{
		a_colTriangle = r_colTriangle;
		a_colPoint = r_colPoint;
		a_colSquareDistance = r_colSquareDistance;
	}

	// return result
	return (l_result || r_result);
}


//===========================================================================
/*!
	Return whether this node contains the specified triangle tag.

	\fn       void cCollisionAABBInternal::contains_triangle(int tag)
	\param    tag  Tag to inquire about
*/
//===========================================================================
bool cCollisionAABBInternal::contains_triangle(int a_tag)
{
	return (m_leftSubTree->contains_triangle(a_tag) ||
		m_rightSubTree->contains_triangle(a_tag));
}


//===========================================================================
/*!
	Sets this node's parent pointer and optionally propagate
	assignments to its children (setting their parent pointers to this node).

	\fn       void cCollisionAABBInternal::setParent(cCollisionAABBNode* a_parent,
			  int a_recursive);
	\param    a_parent     Pointer to this node's parent.
	\param    a_recursive  Propagate assignment down the tree?
*/
//===========================================================================
void cCollisionAABBInternal::setParent(cCollisionAABBNode* a_parent, int a_recursive)
{
	m_parent = a_parent;
	if (m_leftSubTree && a_recursive)  m_leftSubTree->setParent(this, 1);
	if (m_rightSubTree && a_recursive) m_rightSubTree->setParent(this, 1);
}
