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
/*!
	\file CCollisionAABBTree.h
*/
//---------------------------------------------------------------------------
#ifndef CCollisionAABBTreeH
#define CCollisionAABBTreeH
//---------------------------------------------------------------------------
#include "CMaths.h"
#include "CVertex.h"
#include "CTriangle.h"
#include "CCollisionAABBBox.h"
//---------------------------------------------------------------------------

typedef enum {
	AABB_NODE_INTERNAL = 0,
	AABB_NODE_LEAF,
	AABB_NODE_GENERIC
} aabb_node_types;

//===========================================================================
/*!
	  \class    cCollisionAABBNode
	  \brief    cCollisionAABBNode is an abstract class that contains methods
				to set up internal and leaf nodes of an AABB tree
				and to use them to detect for collision with a line.
*/
//===========================================================================
class cCollisionAABBNode
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Constructor of cCollisionAABBNode.
	cCollisionAABBNode() : m_parent(0), m_depth(0), m_nodeType(AABB_NODE_GENERIC) { }
	//! Constructor of cCollisionAABBNode.
	cCollisionAABBNode(aabb_node_types a_nodeType, int a_depth) : m_parent(0),
		m_nodeType(a_nodeType), m_depth(a_depth) { }

	//! Destructor of cCollisionAABBNode.
	virtual ~cCollisionAABBNode() {}

	// METHODS:
	//! Create a bounding box for the portion of the model at or below the node.
	virtual void fitBBox() {}
	//! Draw the edges of the bounding box for this node, if at the given depth.
	virtual void render(int a_depth = -1) = 0;
	//! Determine whether line intersects mesh bounded by subtree rooted at node.
	virtual bool computeCollision(cVector3d& a_segmentPointA,
		cVector3d& a_segmentDirection, cCollisionAABBBox &a_lineBox,
		cTriangle*& a_colTriangle, cVector3d& a_colPoint,
		double& a_colSquareDistance) = 0;
	//! Return true if this node contains the specified triangle tag.
	virtual bool contains_triangle(int a_tag) = 0;
	//! Set the parent of this node.
	virtual void setParent(cCollisionAABBNode* a_parent, int a_recusive) = 0;

	// MEMBERS:
	//! The bounding box for this node.
	cCollisionAABBBox m_bbox;
	//! The depth of this node in the collision tree.
	int m_depth;
	//! Parent node of this node.
	cCollisionAABBNode* m_parent;

	//! The node type, used only for proper deletion right now
	int m_nodeType;
};


//===========================================================================
/*!
	  \class    cCollisionAABBLeaf
	  \brief    cCollisionAABBLeaf contains methods to set up leaf
				nodes of an AABB tree and to use them to detect for collision
				with a line.
*/
//===========================================================================
class cCollisionAABBLeaf : public cCollisionAABBNode
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Default constructor of cCollisionAABBLeaf.
	cCollisionAABBLeaf() : cCollisionAABBNode(AABB_NODE_LEAF, 0) { }
	//! Constructor of cCollisionAABBLeaf.
	cCollisionAABBLeaf(cTriangle *a_triangle) : m_triangle(a_triangle)
	{
		fitBBox();
	}
	//! Destructor of cCollisionAABBLeaf.
	virtual ~cCollisionAABBLeaf() {}

	// METHODS:
	//! Create a bounding box to enclose triangle belonging to this leaf node.
	void fitBBox();
	//! Draw the edges of the bounding box for this leaf if it is at depth a_depth.
	void render(int a_depth = -1);
	//! Determine whether the given line intersects this leaf's triangle.
	bool computeCollision(cVector3d& a_segmentPointA,
		cVector3d& a_segmentDirection, cCollisionAABBBox &a_lineBox,
		cTriangle*& a_colTriangle, cVector3d& a_colPoint,
		double& a_colSquareDistance);
	//! Return true if this node contains the specified triangle tag.
	virtual bool contains_triangle(int a_tag)
	{
		return (m_triangle != 0 && m_triangle->m_tag == a_tag);
	}
	//! Return parent of this node.
	virtual void setParent(cCollisionAABBNode* a_parent, int a_recusive)
	{
		m_parent = a_parent;
	}

	// MEMBERS:
	//! The triangle bounded by the leaf.
	cTriangle *m_triangle;
};


//===========================================================================
/*!
	  \class    cCollisionAABBInternal
	  \brief    cCollisionAABBInternal contains methods to set up internal
				nodes of an AABB tree and to use them to detect for collision
				with a line.
*/
//===========================================================================
class cCollisionAABBInternal : public cCollisionAABBNode
{
public:
	// CONSTRUCTOR & DESTRUCTOR:
	//! Default constructor of cCollisionAABBInternal.
	cCollisionAABBInternal() : cCollisionAABBNode(AABB_NODE_INTERNAL, 0) { }

	//! Constructor of cCollisionAABBInternal.
	cCollisionAABBInternal(unsigned int a_numLeaves, cCollisionAABBLeaf *a_leaves,
		unsigned int a_depth = -1);
	//! Destructor of cCollisionAABBInternal.
	virtual ~cCollisionAABBInternal();

	// METHODS:
	//! Size the bounding box for this node to enclose its children.
	void fitBBox() { m_bbox.enclose(m_leftSubTree->m_bbox, m_rightSubTree->m_bbox); }
	//! Draw the edges of the bounding box for this node if it is at depth a_depth.
	void render(int a_depth = -1);
	//! Determine whether given line intersects the tree rooted at this node.
	bool computeCollision(cVector3d& a_segmentPointA,
		cVector3d& a_segmentDirection, cCollisionAABBBox &a_lineBox,
		cTriangle*& a_colTriangle, cVector3d& a_colPoint,
		double& a_colSquareDistance);
	//! Return true if this node contains the specified triangle tag.
	virtual bool contains_triangle(int a_tag);
	//! Return parent node and optionally propagate the assignment to children.
	virtual void setParent(cCollisionAABBNode* a_parent, int a_recursive);

	// MEMBERS:
	//! The root of this node's left subtree.
	cCollisionAABBNode *m_leftSubTree;
	//! The root of this node's right subtree.
	cCollisionAABBNode *m_rightSubTree;
	//! Test box for collision with ray itself (as compared to line's box)?
	bool m_testLineBox;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

