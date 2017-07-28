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

	\author     Chris Sewell
	\file       CCollisionSpheres.h
	\version    1.0
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CCollisionSpheresH
#define CCollisionSpheresH
#ifdef _MSVC
// Turn off annoying compiler warnings in MSVC.
#pragma warning(disable: 4305)
#pragma warning(disable: 4800) 
#pragma warning(disable: 4786)
#endif
//---------------------------------------------------------------------------
#include "CMaths.h"
#include "CCollisionSpheresGeometry.h"
#include "CVertex.h"
#include "CTriangle.h"
#include "CGenericCollision.h"
#include "CDraw3D.h"
#include "CColor.h"
#include "CMesh.h"
#include <assert.h>
#include <list>
#include <queue>
#include <vector>
#include "GL/glu.h"
//---------------------------------------------------------------------------
// TYPEDEFS:
//! Pointer to list of shape primitives.
typedef vector<cCollisionSpheresGenericShape*> Plist;

// FORWARD DECLARATIONS:
//! Nodes of the collision sphere tree (abstract class).
class cCollisionSpheresSphere;
//! Leaf nodes of the sphere tree.
class cCollisionSpheresLeaf;
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \file     CCollisionSpheres.h
	  \class    cCollisionSpheres
	  \brief    cCollisionSpheres provides methods to create a sphere tree for
				collision detection, and to use this tree to check for the
				intersection of a line with a mesh.
*/
//===========================================================================
class cCollisionSpheres : public cGenericCollision
{
public:
	// CONSTRUCTOR:
	//! Constructor of cCollisionSpheres.
	cCollisionSpheres(vector<cTriangle> *a_triangles, bool a_useNeighbors);
	//! Destructor of cCollisionSpheres.
	virtual ~cCollisionSpheres();

	// METHODS:
	//! Build the sphere tree based on the given triangles.
	void initialize();
	//! Draw the collision spheres in OpenGL.
	void render();
	//! Return the nearest triangle intersected by the given segment, if any.
	bool computeCollision(cVector3d& a_segmentPointA,
		cVector3d& a_segmentPointB, cGenericObject*& a_colObject,
		cTriangle*& a_colTriangle, cVector3d& a_colPoint,
		double& a_colSquareDistance, int a_proxyCall = -1);

	// PROPERTIES:
	//! Pointer to the sphere at the root of the sphere tree.
	cCollisionSpheresSphere *m_root;
	//! Pointer to the list of triangles in the mesh.
	vector<cTriangle> *m_trigs;
	//! Triangle returned by last successful collision test.
	cTriangle *m_lastCollision;
	//! Use neighbor list to speed up collision detection?
	bool m_useNeighbors;
	//! Pointer to the beginning of list of leaf nodes.
	cCollisionSpheresLeaf *m_firstLeaf;
	cTriangle* secret;
};


//===========================================================================
/*!
	  \class    cCollisionSpheresSphere
	  \brief    cCollisionSpheresSphere is an abstract class for nodes of
				the collision sphere tree.
*/
//===========================================================================
class cCollisionSpheresSphere
{
	// FRIENDS:
	//! Internal nodes of the collision sphere tree.
	friend class cCollisionSpheresNode;
	//! Leaf nodes of the collision sphere tree.
	friend class cCollisionSpheresLeaf;

public:
	// CONSTRUCTORS AND DESTRUCTOR:
	//! Constructor of cCollisionSpheresSphere.
	cCollisionSpheresSphere(cCollisionSpheresSphere *a_parent);
	//! Default constructor of cCollisionSpheresSphere.
	cCollisionSpheresSphere() : m_center(0, 0, 0), m_parent(0), m_depth(0) { };
	//! Destructor of cCollisionsSpheresSphere.
	virtual ~cCollisionSpheresSphere() {};

	// METHODS:
	//! Return the center of the sphere.
	inline const cVector3d &getCenter() { return m_center; }
	//! Return the radius of the sphere.
	inline double getRadius() { return m_radius; }
	//! Return whether the node is a leaf node.
	virtual int isLeaf() = 0;
	//! Draw the collision sphere for this node, if at the given depth.
	virtual void draw(int a_depth = -1) = 0;
	//! Calculate the distance between the two given collision spheres.
	static bool computeCollision(cCollisionSpheresSphere *a_sa,
		cGenericObject*& a_colObject, cTriangle*& a_colTriangle,
		cVector3d& a_colPoint, double& a_colSquareDistance,
		cCollisionSpheresSphere *a_sb);

	//protected:
	  // PROPERTIES:
	  //! The parent of the node in the tree.
	cCollisionSpheresSphere *m_parent;
	//! The center of the node.
	cVector3d m_center;
	//! The radius of the node.
	double m_radius;
	//! The depth of this node in the collision tree.
	int m_depth;

	int m_num;

};


//===========================================================================
/*!
	  \class    cCollisionSpheresNode
	  \brief    cCollisionSpheresNode defines internal nodes of the collision
				sphere tree and provides methods for constructing the nodes
				and using them to determine collisions.
*/
//===========================================================================
class cCollisionSpheresNode : public cCollisionSpheresSphere
{
public:
	// CONSTRUCTORS AND DESTRUCTOR:
	//! Constructor of cCollisionSpheresNode.
	cCollisionSpheresNode(Plist &a_primList,
		cCollisionSpheresSphere *a_parent = NULL);
	//! Constructor of cCollisionSpheresNode.
	cCollisionSpheresNode(std::vector<cTriangle> *a_tris,
		cCollisionSpheresSphere *a_parent = NULL);
	//! Default constructor of cCollisionSpheresNode.
	cCollisionSpheresNode() : cCollisionSpheresSphere(), m_left(0), m_right(0) { };
	//! Destructor of cCollisionSpheseNode.
	virtual ~cCollisionSpheresNode() {};

	// METHODS:
	//! Create subtrees by splitting primitives into left and right lists.
	void ConstructChildren(Plist &a_primList);
	//! Return whether the node is a leaf node. (In this class, it is not.)
	int isLeaf() { return 0; }
	//! Draw the collision sphere if at the given depth.
	void draw(int a_depth);
	//! Check for intersection between given nodes, the first an internal node.
	static bool computeCollision(cCollisionSpheresNode *a_sa,
		cGenericObject*& a_colObject, cTriangle*& a_colTriangle,
		cVector3d& a_colPoint, double& a_colSquareDistance,
		cCollisionSpheresSphere *a_sb);
	//! Exchange the two given pointers.
	static void swapptr(void **a_a, void **a_b);

	// PROPERTIES:
	//! Pointer to the left child in the sphere tree.
	cCollisionSpheresSphere *m_left;
	//! Pointer to the right child in the sphere tree.
	cCollisionSpheresSphere *m_right;
};


//===========================================================================
/*!
	  \class    cCollisionSpheresLeaf
	  \brief    cCollisionSpheresLeaf defines leaf nodes of the collision
				sphere tree and provides methods for constructing the nodes
				and using them to determine collisions.
*/
//===========================================================================
class cCollisionSpheresLeaf : public cCollisionSpheresSphere
{
public:
	// CONSTRUCTORS AND DESTRUCTOR:
	//! Constructor of cCollisionSpheresLeaf.
	cCollisionSpheresLeaf(cCollisionSpheresGenericShape *a_prim,
		cCollisionSpheresSphere *a_parent = NULL);
	//! Constructor of cCollisionSpheresLeaf.
	cCollisionSpheresLeaf(cTriangle *a_tri,
		cCollisionSpheresSphere *a_parent = NULL);
	//! Default constructor of cCollisionSpheresLeaf
	cCollisionSpheresLeaf() : cCollisionSpheresSphere() { m_prim = 0; }
	//! Destructor of cCollisionSpheresLeaf()
	virtual ~cCollisionSpheresLeaf() { if (m_prim) delete m_prim; }

	// METHODS:
	//! Return whether the node is a leaf node. (In this class, it is.)
	int isLeaf() { return 1; }
	//! Draw the collision sphere if at the given depth.
	void draw(int a_depth);
	//! Check for intersection between the two given leaf nodes.
	static bool computeCollision(cCollisionSpheresLeaf *a_sa,
		cGenericObject*& a_colObject, cTriangle*& a_colTriangle,
		cVector3d& a_colPoint, double& a_colSquareDistance,
		cCollisionSpheresLeaf *a_sb);

	// PROPERTIES:
	//! The shape primitive bounded by the sphere leaf.
	cCollisionSpheresGenericShape *m_prim;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


