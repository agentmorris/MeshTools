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
    \version    1.0
    \date       05/2006
*/
//===========================================================================

#include "CCollisionSpheres.h"
#include "CMesh.h"


//===========================================================================
/*!
      \class      cCollisionStack
      \brief      cCollisionStack is used to store pairs of nodes from two
                  sphere tree bounding volume hierarchies for storing on 
                  the stack of the collision search algorithm
*/
//===========================================================================
class cCollisionStack
{
  public:
    cCollisionSpheresSphere* node1;
    cCollisionSpheresSphere* node2;
    cMesh* mesh1;
    cMesh* mesh2;
    float dist;
};


//===========================================================================
/*!
      \class      cMeshMeshMesh
      \brief      cCollisionMeshMesh searches for a collision between two
                  meshes by using their cCollisionSphere bounding volume
                  hierarchies
*/
//===========================================================================
class cCollisionMeshMesh 
{
  public:
    // CONSTRUCTOR AND DESTRUCTOR:
    //! Constructor of cCollisionMeshMesh
    cCollisionMeshMesh() { m_useBrute = 0; m_useNeighbors = 1; m_lastContact1 = 0; m_lastContact2 = 0; m_findAll = 0; };

    // METHODS:
    //! Determine if the given meshes intersect; if so, return one pair of intersecting triangles
    int computeCollision(cMesh* a_mesh1, cMesh* a_mesh2, std::vector<cTriangle*> &a_tri1, std::vector<cTriangle*> &a_tri2);
    //! Iterative function to search for a collision between two sphere (sub)trees rooted at the given nodes
    int collisionSearch(cCollisionSpheresSphere* a_node1, cCollisionSpheresSphere* a_node2, cMesh* a_mesh1, cMesh* a_mesh2, 
                        std::vector<cTriangle*> &a_tri1, std::vector<cTriangle*> &a_tri2);
    //! Determine whether two triangles intersect by setting up call to NoDivMeshMeshIsect
    int primitiveTest(cTriangle a_tri1, cTriangle a_tri2);
    //! Determine whether the two given triangles intersect using Tomas Moller's algorithm
    int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
                         float U0[3],float U1[3],float U2[3]);
    //! Helper function for NoDivTriTriIsect for intersection test in special case of two coplanar triangles
    int coplanar_tri_tri(float N[3],float V0[3],float V1[3],float V2[3],
                         float U0[3],float U1[3],float U2[3]);
    //! Set whether to use brute-force (rather than sphere trees) for collision detection
    void setUseBrute(int a_useBrute) { m_useBrute = a_useBrute; }
    //! Get whether brute-force collision detection is being used
    int getUseBrute() { return m_useBrute; }
    //! Set whether to use neighbor lists for collision detection
    void setUseNeighbors(int a_useNeighbors) { m_useNeighbors = a_useNeighbors; }
    //! Get whether neighbor lists are being used
    int getUseNeighbors() { return m_useNeighbors; }
    //! Set whether we need to find all pairs of colliding triangles or just one
    void setFindAll(int a_findAll) { m_findAll = a_findAll; }
    //! Get whether we are trying to find all pairs of colliding triangles or just one
    int getFindAll() { return m_findAll; }
    
  protected:
    // PROPERTIES:
    //! Use brute-force (checking all pairs of triangles) rather than using sphere trees?
    int m_useBrute;
    //! Use neighbor lists to first check for collisions between neighbors of last colliding triangles?
    int m_useNeighbors;
    //! Do we need to report all pairs of colliding triangles?
    int m_findAll;
    //! Save a pair of triangles from the last collision to check first in the next iteration
    cTriangle* m_lastContact1;
    //! Save a pair of triangles from the last collision to check first in the next iteration
    cTriangle* m_lastContact2;
};