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
#ifndef CCollisionAABBH
#define CCollisionAABBH
//---------------------------------------------------------------------------
#include "CTriangle.h"
#include "CVertex.h"
#include "CMaths.h"
#include "CGenericCollision.h"
#include "CCollisionAABBBox.h"
#include "CCollisionAABBTree.h"
#include <vector>
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file     CCollisionAABB.h
      \class    cCollisionAABB
      \brief    cCollisionAABB provides methods to create an Axis-Aligned
                Bounding Box collision detection tree, and to use
                this tree to check for the intersection of a line segment
                with a mesh.
*/
//===========================================================================
class cCollisionAABB : public cGenericCollision
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cAABBTree.
    cCollisionAABB(vector<cTriangle>* a_triangles, bool a_useNeighbors);
    //! Destructor of cAABBTree.
    virtual ~cCollisionAABB();

    // METHODS:
    //! Build the AABB Tree for the first time.
    void initialize();
    //! Draw the bounding boxes in OpenGL.
    void render();
    //! Return the nearest triangle intersected by the given segment, if any.
    bool computeCollision(cVector3d& a_segmentPointA,
            cVector3d& a_segmentPointB, cGenericObject*& a_colObject,
            cTriangle*& a_colTriangle, cVector3d& a_colPoint,
            double& a_colSquareDistance, int a_proxyCall = -1);
    //! Return the root node of the collision tree.
    cCollisionAABBNode* getRoot() { return (m_root); }

  protected:
    // MEMBERS:
    //! Pointer to the list of triangles in the mesh.
    vector<cTriangle> *m_triangles;
    //! Pointer to an array of leaf nodes for the AABB Tree.
    cCollisionAABBLeaf *m_leaves;
    //! Pointer to the root of the AABB Tree.
    cCollisionAABBNode *m_root;
    //! The number of triangles in the mesh.
    unsigned int m_numTriangles;
    //! Triangle returned by last successful collision test.
    //!
    //! This is only modified by _proxy_ collision tests.
    cTriangle* m_lastCollision;
    //! Use list of triangles' neighbors to speed up collision detection?
    bool m_useNeighbors;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
