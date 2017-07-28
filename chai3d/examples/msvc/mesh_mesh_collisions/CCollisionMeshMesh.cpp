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

#include "stdafx.h"
#include "CCollisionMeshMesh.h"

// Overloaded less-than operator so that our priority queue of search tree
// nodes will descend paths with greatest overlap first
bool operator<(cCollisionStack c1, cCollisionStack c2)
{
    return (c1.dist < c2.dist);
}


//===========================================================================
/*!
    Determine if the given meshes intersect; if so, return one pair of 
    intersecting triangles

    \fn       int cCollisionMeshMesh::computeCollision(cMesh* a_mesh1, cMesh* a_mesh2, 
                                            cTriangle &a_tri1, cTriangle &a_tri2)
    \param    a_mesh1  First mesh for collision test
    \param    a_mesh2  Second mesh for collision test
    \param    a_tri1   Returns one (or all if m_findAll) triangle pairs in collision 
    \param    a_tri2   Returns one (or all if m_findAll) triangle pairs in collision 
    \return   Do a_mesh1 and a_mesh2 intersect?
*/
//===========================================================================
int cCollisionMeshMesh::computeCollision(cMesh* a_mesh1, cMesh* a_mesh2, 
                                         std::vector<cTriangle*> &a_tri1, 
                                         std::vector<cTriangle*> &a_tri2)
{
    // If we are using neighbor lists, first check neighbors of recently 
    // collided triangles, since they are most likely to collide
    if (m_useNeighbors && !m_findAll && m_lastContact1 && m_lastContact2)
    {
        // First see if the triangles that collided last time still collide
        if (primitiveTest(*m_lastContact1, *m_lastContact2))
            return 1;

        // Then try all pairs of neighbors of these triangles
        for (unsigned int i=0; i<m_lastContact1->m_neighbors->size(); i++)
        {
            cTriangle* t1 = (*(m_lastContact1->m_neighbors))[i];    
            if (!t1) continue;
            for (unsigned int j=0; j<m_lastContact2->m_neighbors->size(); j++)
            {
                cTriangle* t2 = (*(m_lastContact2->m_neighbors))[i];
                if (!t2) continue;
                if (primitiveTest(*t1, *t2))
                {
                    a_tri1.push_back(t1);
                    a_tri2.push_back(t2);
                    return 1;
                }
            }
        }
    }

    // If we are using "brute force", just compare all pairs of triangles
    if (m_useBrute)
    {
        // Get the vectors of triangles from both meshes
        vector<cTriangle>* mesh1_tris = a_mesh1->pTriangles(); 
        vector<cTriangle>* mesh2_tris = a_mesh2->pTriangles(); 

        // Loop through all pairs
        for (unsigned int i=0; i<mesh1_tris->size(); i++)
        {
            for (unsigned int j=0; j<mesh2_tris->size(); j++)
            {
                // Do a triangle-triangle intersection test
                if (primitiveTest((*mesh1_tris)[i], (*mesh2_tris)[j]))
                {
                    a_tri1.push_back(&(*mesh1_tris)[i]);
                    a_tri2.push_back(&(*mesh2_tris)[j]);
                    if (!m_findAll)
                      return 1;
                }
            }
        }
        return 0;
    }

    // Otherwise, get the sphere tree bounding volume hierarchies from both meshes
    cCollisionSpheres* cd1 = dynamic_cast<cCollisionSpheres*>(a_mesh1->getCollisionDetector());
    cCollisionSpheres* cd2 = dynamic_cast<cCollisionSpheres*>(a_mesh2->getCollisionDetector());

    // If both meshes have sphere trees, start the iterative search on their roots
    if (cd1 && cd2 && cd1->m_root != NULL && cd2->m_root != NULL)
    return collisionSearch(cd1->m_root, cd2->m_root, a_mesh1, a_mesh2, a_tri1, a_tri2); 
    return 0;
}


//===========================================================================
/*!
    Iterative function to search for a collision between two sphere (sub)trees 
    rooted at the given nodes

    \fn       int cCollisionMeshMesh::collisionSearch(cCollisionSpheresSphere* a_node1, 
                          cCollisionSpheresSphere* a_node2, cMesh* a_mesh1, 
                          cMesh* a_mesh2, cTriangle &a_tri1, cTriangle &a_tri2)
    \param    a_node1  Root of first sphere tree for collision test
    \param    a_node2  Root of second sphere tree for collision test
    \param    a_mesh1  First mesh for collision test
    \param    a_mesh2  Second mesh for collision test
    \param    a_tri1   Returns one (or all if m_findAll) triangle pairs in collision
    \param    a_tri2   Returns one (or all if m_findAll) triangle pairs in collision
    \return   Do any triangless in the two sphere (sub)trees intersect?
*/
//===========================================================================
int cCollisionMeshMesh::collisionSearch(cCollisionSpheresSphere* a_node1, 
                                      cCollisionSpheresSphere* a_node2, 
                                      cMesh* a_mesh1, cMesh* a_mesh2, 
                                      std::vector<cTriangle*> &a_tri1, 
                                      std::vector<cTriangle*> &a_tri2)
{
    cCollisionStack collision_struct;
    collision_struct.node1 = a_node1; 
    collision_struct.node2 = a_node2;
    collision_struct.mesh1 = a_mesh1;
    collision_struct.mesh2 = a_mesh2;
    collision_struct.dist = 0.0;

    std::priority_queue<cCollisionStack> stack;
    stack.push(collision_struct);

    int hit = 0;
    while ((!stack.empty()) && ((hit == 0) || (m_findAll == 1)))
    {
        cCollisionStack cur_struct = stack.top();
        stack.pop();

        // Get global coordinates of the two trees' centers
        cVector3d center1 = cMul(cur_struct.mesh1->getGlobalRot(), cur_struct.node1->getCenter());
        cVector3d center2 = cMul(cur_struct.mesh2->getGlobalRot(), cur_struct.node2->getCenter());
        center1.add(cur_struct.mesh1->getGlobalPos());
        center2.add(cur_struct.mesh2->getGlobalPos());

        // If the distance between the centers is greater than the sum of the radii, 
        // there can be no intersection between these two trees
        float distsq = (float)(cDistance(center1, center2));   
        float radius_sum = (float)
          (cur_struct.node1->getRadius() + cur_struct.node2->getRadius());
        radius_sum = radius_sum*radius_sum;
        if (distsq > radius_sum) 
            continue;

        // If both nodes are leafs, we need to do a triangle-triangle primitive test
        // between the triangles associated with these leafs
        if ((cur_struct.node1->isLeaf()) && (cur_struct.node2->isLeaf()))
        {
            cCollisionSpheresLeaf* leaf1 = (cCollisionSpheresLeaf*)cur_struct.node1;
            cCollisionSpheresLeaf* leaf2 = (cCollisionSpheresLeaf*)cur_struct.node2;
            cCollisionSpheresTri* prim1 = (cCollisionSpheresTri*)(leaf1->m_prim);
            cCollisionSpheresTri* prim2 = (cCollisionSpheresTri*)(leaf2->m_prim);
            int test = primitiveTest(*(prim1->getOriginal()), *(prim2->getOriginal()));

            // If these triangles do intersect, return them
            if (test == 1)
            {
                m_lastContact1 = prim1->getOriginal();
                m_lastContact2 = prim2->getOriginal();

                a_tri1.push_back(m_lastContact1);
                a_tri2.push_back(m_lastContact2);

                hit = 1;
                if (!m_findAll)
                    return 1;
            }
            continue;
        }

        // If the first node is a leaf and the second is an internal node, recurse by
        // comparing the leaf to the left and to the right subtrees of the internal node
        if ((cur_struct.node1->isLeaf()) && (!cur_struct.node2->isLeaf()))
        {
			// Compare leaf node 1 to left child of node 2
            cCollisionSpheresNode* internal2 = (cCollisionSpheresNode*)cur_struct.node2;
            cCollisionStack next_struct;
            next_struct.node1 = internal2->m_left;
            next_struct.node2 = cur_struct.node1;
            next_struct.mesh1 = cur_struct.mesh2;
            next_struct.mesh2 = cur_struct.mesh1;

			// Get the overlap between these two spheres
            cVector3d c2 = cMul(cur_struct.mesh2->getGlobalRot(), internal2->m_left->getCenter());
            c2.add(cur_struct.mesh2->getGlobalPos());    
            float r = (float)
              (cur_struct.node1->getRadius() + internal2->m_left->getRadius());
            next_struct.dist = (float)(cDistance(center1, c2) - r*r);
            stack.push(next_struct);

			// Compare leaf node 1 to right child of node 2
            next_struct.node1 = internal2->m_right;
            next_struct.node2 = cur_struct.node1;
            next_struct.mesh1 = cur_struct.mesh2;
            next_struct.mesh2 = cur_struct.mesh1;

			// Get the overlap between these two spheres
            c2 = cMul(cur_struct.mesh2->getGlobalRot(), internal2->m_right->getCenter());
            c2.add(cur_struct.mesh2->getGlobalPos());    
            r = (float)
              (cur_struct.node1->getRadius() + internal2->m_right->getRadius());
            next_struct.dist = (float)(cDistance(center1, c2) - r*r);
            stack.push(next_struct);

            continue;
        }

        // If the second node is a leaf and the fist is an internal node, recurse by
        // comparing the leaf to the left and to the right subtrees of the internal node
        if ((cur_struct.node2->isLeaf()) && (!cur_struct.node1->isLeaf()))
        {
			// Compare leaf node 2 to left child of node 1
            cCollisionSpheresNode* internal1 = (cCollisionSpheresNode*)cur_struct.node1;
            cCollisionStack next_struct;
            next_struct.node1 = internal1->m_left;
            next_struct.node2 = cur_struct.node2;
            next_struct.mesh1 = cur_struct.mesh1;
            next_struct.mesh2 = cur_struct.mesh2;
            
			// Get the overlap between these two spheres
            cVector3d c1 = cMul(cur_struct.mesh1->getGlobalRot(), internal1->m_left->getCenter());
            c1.add(cur_struct.mesh1->getGlobalPos());   
            float r = (float)
              (cur_struct.node2->getRadius() + internal1->m_left->getRadius());
            next_struct.dist = (float)(cDistance(c1, center2) - r*r);
            stack.push(next_struct);

			// Compare leaf node 2 to right child of node 1
            next_struct.node1 = internal1->m_right;
            next_struct.node2 = cur_struct.node2;
            next_struct.mesh1 = cur_struct.mesh1;
            next_struct.mesh2 = cur_struct.mesh2;
            
			// Get the overlap between these two spheres
            c1 = cMul(cur_struct.mesh1->getGlobalRot(), internal1->m_right->getCenter());
            c1.add(cur_struct.mesh1->getGlobalPos());    
            r = (float)
              (cur_struct.node2->getRadius() + internal1->m_right->getRadius());
            next_struct.dist = (float)(cDistance(c1, center2) - r*r);
            stack.push(next_struct);

            continue;
        }

        // Otherwise, both nodes are internal nodes, and we will want to recurse by breaking
        // the larger into its children, so swap nodes if necessary so that the larger one
        // is the first one
        if (cur_struct.node1->getRadius() < cur_struct.node2->getRadius())
        {
            cCollisionSpheresSphere* temp = cur_struct.node1;
            cur_struct.node1 = cur_struct.node2;
            cur_struct.node2 = temp;
            cMesh* temp_mesh = cur_struct.mesh1;
            cur_struct.mesh1 = cur_struct.mesh2;
            cur_struct.mesh2 = temp_mesh;
        }

        // Recurse by comparing the second internal node to the left and to the right subtrees
        // of the first internal node
        cCollisionSpheresNode* internal1 = (cCollisionSpheresNode*)cur_struct.node1;

		// Compare node 2 to left child of node 1
        cCollisionStack next_struct;
        next_struct.node1 = internal1->m_left;
        next_struct.node2 = cur_struct.node2;
        next_struct.mesh1 = cur_struct.mesh1;
        next_struct.mesh2 = cur_struct.mesh2;
        
		// Get the overlap between these two spheres
        cVector3d c1 = cMul(cur_struct.mesh1->getGlobalRot(), internal1->m_left->getCenter());
        c1.add(cur_struct.mesh1->getGlobalPos());    
        float r = (float)
          (cur_struct.node2->getRadius() + internal1->m_left->getRadius());
        next_struct.dist = (float)(cDistance(c1, center2) - r*r);
        stack.push(next_struct);

		// Compare node 2 to right child of node 1
        next_struct.node1 = internal1->m_right;
        next_struct.node2 = cur_struct.node2;
        next_struct.mesh1 = cur_struct.mesh1;
        next_struct.mesh2 = cur_struct.mesh2;
        
		// Get the overlap between these two spheres
        c1 = cMul(cur_struct.mesh1->getGlobalRot(), internal1->m_right->getCenter());
        c1.add(cur_struct.mesh1->getGlobalPos());    
        r = (float)
          (cur_struct.node2->getRadius() + internal1->m_right->getRadius());
        next_struct.dist = (float)(cDistance(c1, center2) - r*r);

        stack.push(next_struct);
    }

    m_lastContact1 = 0;
    m_lastContact2 = 0;
    return hit;
}


//===========================================================================
/*!
    Determine whether two triangles intersect by setting up call to 
    NoDivTriTriIsect, converting cVector3ds to float arrays and all
    vertices to global coordinates

    \fn       int cCollisionMeshMesh::primitiveTest(cTriangle a_tri1, cTriangle a_tri2)
    \param    a_tri1   First triangle to test for intersection
    \param    a_tri2   Second triangle to test for intersection
    \return   Do a_tri1 and a_tri2 intersect?
*/
//===========================================================================
int cCollisionMeshMesh::primitiveTest(cTriangle a_tri1, cTriangle a_tri2)
{
    float V0[3], V1[3], V2[3], U0[3], U1[3], U2[3];

    // Set up vertices of first triangle
    cVector3d v = a_tri1.getVertex0()->getGlobalPos(); 
    V0[0] = (float)v.x; V0[1] = (float)v.y; V0[2] = (float)v.z; 
    v = a_tri1.getVertex1()->getGlobalPos(); 
    V1[0] = (float)v.x; V1[1] = (float)v.y; V1[2] = (float)v.z; 
    v = a_tri1.getVertex2()->getGlobalPos(); 
    V2[0] = (float)v.x; V2[1] = (float)v.y; V2[2] = (float)v.z; 

    // Set up vertices of second triangle
    v = a_tri2.getVertex0()->getGlobalPos(); 
    U0[0] = (float)v.x; U0[1] = (float)v.y; U0[2] = (float)v.z; 
    v = a_tri2.getVertex1()->getGlobalPos(); 
    U1[0] = (float)v.x; U1[1] = (float)v.y; U1[2] = (float)v.z; 
    v = a_tri2.getVertex2()->getGlobalPos(); 
    U2[0] = (float)v.x; U2[1] = (float)v.y; U2[2] = (float)v.z; 

    // Call Moller's function
    return NoDivTriTriIsect(V0, V1, V2, U0, U1, U2);
}


//===========================================================================
/*!
    The rest of the code in this file is from Tomas Moller's implementation
    of a triangle-triangle intersection test, as described in the paper
    "A Fast Triangle-Triangle Intersection Test" by Tomas Moller, Journal
    of Graphics Tools, 2(2):25-30, 1997.  The code is on-line at 
    http://www.cs.lth.se/home/Tomas_Akenine_Moller/code/opttritri.txt.
*/
//===========================================================================

#define FABS(x) (float(fabs(x)))        /* implement as is fastest on your machine */

/* if USE_EPSILON_TEST is true then we do a check:
         if |dv|<EPSILON then dv=0.0;
   else no check is done (which is less robust)
*/
#define USE_EPSILON_TEST TRUE
#define EPSILON 0.000001


/* some macros */
#define CROSS(dest,v1,v2){                     \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];}

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2){         \
            dest[0]=v1[0]-v2[0]; \
            dest[1]=v1[1]-v2[1]; \
            dest[2]=v1[2]-v2[2];}

/* sort so that a<=b */
#define SORT(a,b)       \
             if(a>b)    \
             {          \
               float c; \
               c=a;     \
               a=b;     \
               b=c;     \
             }


/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return 1;                      \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return 1;                      \
    }                                                 \
  }

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  float Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  float a,b,c,d0,d1,d2;                     \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return 1;                 \
  }                                         \
}


//===========================================================================
/*!
    Helper function for NoDivTriTriIsect for intersection test in special case 
    of two coplanar triangles

    \fn       int cCollisionMeshMesh::coplanar_tri_tri(float N[3],float V0[3],
                    float V1[3],float V2[3],float U0[3],float U1[3],float U2[3])
    \param    N[3]  Normal of the plane on which both triangles lie
    \param    V0[3] Coordinates of the first vertex of the first triangle
    \param    V1[3] Coordinates of the second vertex of the first triangle
    \param    V2[3] Coordinates of the third vertex of the first triangle
    \param    U0[3] Coordinates of the first vertex of the second triangle
    \param    U1[3] Coordinates of the second vertex of the second triangle
    \param    U2[3] Coordinates of the third vertex of the second triangle
    \return   Do the two triangles intersect?
*/
//===========================================================================
int cCollisionMeshMesh::coplanar_tri_tri(float N[3],float V0[3],float V1[3],float V2[3],
                                         float U0[3],float U1[3],float U2[3])
{
   float A[3];
   short i0,i1;
   /* first project onto an axis-aligned plane, that maximizes the area */
   /* of the triangles, compute indices: i0,i1. */
   A[0]=FABS(N[0]);
   A[1]=FABS(N[1]);
   A[2]=FABS(N[2]);
   if(A[0]>A[1])
   {
      if(A[0]>A[2])
      {
          i0=1;      /* A[0] is greatest */
          i1=2;
      }
      else
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
   }
   else   /* A[0]<=A[1] */
   {
      if(A[2]>A[1])
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
      else
      {
          i0=0;      /* A[1] is greatest */
          i1=2;
      }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(V0,U0,U1,U2);
    POINT_IN_TRI(U0,V0,V1,V2);

    return 0;
}



#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
{ \
        if(D0D1>0.0f) \
        { \
                /* here we know that D0D2<=0.0 */ \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else if(D0D2>0.0f)\
        { \
                /* here we know that d0d1<=0.0 */ \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D1*D2>0.0f || D0!=0.0f) \
        { \
                /* here we know that d0d1<=0.0 or that D0!=0.0 */ \
                A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
        } \
        else if(D1!=0.0f) \
        { \
                A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D2!=0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else \
        { \
                /* triangles are coplanar */ \
                return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2); \
        } \
}


//===========================================================================
/*!
    Determine whether the two given triangles intersect using Tomas Moller's 
    algorithm

    \fn       int cCollisionMeshMesh::NoDivTriTriIsect(float V0[3],float V1[3],
                     float V2[3],float U0[3],float U1[3],float U2[3])
    \param    V0[3] Coordinates of the first vertex of the first triangle
    \param    V1[3] Coordinates of the second vertex of the first triangle
    \param    V2[3] Coordinates of the third vertex of the first triangle
    \param    U0[3] Coordinates of the first vertex of the second triangle
    \param    U1[3] Coordinates of the second vertex of the second triangle
    \param    U2[3] Coordinates of the third vertex of the second triangle
    \return   Do the two triangles intersect?
*/
//===========================================================================
int cCollisionMeshMesh::NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
                     float U0[3],float U1[3],float U2[3])
{
  float E1[3],E2[3];
  float N1[3],N2[3],d1,d2;
  float du0,du1,du2,dv0,dv1,dv2;
  float D[3];
  float isect1[2], isect2[2];
  float du0du1,du0du2,dv0dv1,dv0dv2;
  short index;
  float vp0,vp1,vp2;
  float up0,up1,up2;
  float bb,cc,max;

  /* compute plane equation of triangle(V0,V1,V2) */
  SUB(E1,V1,V0);
  SUB(E2,V2,V0);
  CROSS(N1,E1,E2);

  if ((fabs(N1[0]) < 0.0001) && (fabs(N1[1]) < 0.0001) && (fabs(N1[2]) < 0.0001))
      return 0;

  d1=-DOT(N1,V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
  du0=DOT(N1,U0)+d1;
  du1=DOT(N1,U1)+d1;
  du2=DOT(N1,U2)+d1;

  /* coplanarity robustness check */
#if USE_EPSILON_TEST==TRUE
  if(FABS(du0)<EPSILON) du0=0.0;
  if(FABS(du1)<EPSILON) du1=0.0;
  if(FABS(du2)<EPSILON) du2=0.0;
#endif
  du0du1=du0*du1;
  du0du2=du0*du2;

  if(du0du1>0.0f && du0du2>0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;                    /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  SUB(E1,U1,U0);
  SUB(E2,U2,U0);
  CROSS(N2,E1,E2);

  if ((fabs(N2[0]) < 0.0001) && (fabs(N2[1]) < 0.0001) && (fabs(N2[2]) < 0.0001))
      return 0;

  d2=-DOT(N2,U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0=DOT(N2,V0)+d2;
  dv1=DOT(N2,V1)+d2;
  dv2=DOT(N2,V2)+d2;

#if USE_EPSILON_TEST==TRUE
  if(FABS(dv0)<EPSILON) dv0=0.0;
  if(FABS(dv1)<EPSILON) dv1=0.0;
  if(FABS(dv2)<EPSILON) dv2=0.0;
#endif

  dv0dv1=dv0*dv1;
  dv0dv2=dv0*dv2;

  if(dv0dv1>0.0f && dv0dv2>0.0f) /* same sign on all of them + not equal 0 ? */
    return 0;                    /* no intersection occurs */

  /* compute direction of intersection line */
  CROSS(D,N1,N2);

  /* compute and index to the largest component of D */
  max=(float)FABS(D[0]);
  index=0;
  bb=(float)FABS(D[1]);
  cc=(float)FABS(D[2]);
  if(bb>max) max=bb,index=1;
  if(cc>max) max=cc,index=2;

  /* this is the simplified projection onto L*/
  vp0=V0[index];
  vp1=V1[index];
  vp2=V2[index];

  up0=U0[index];
  up1=U1[index];
  up2=U2[index];

  /* compute interval for triangle 1 */
  float a,b,c,x0,x1;
  NEWCOMPUTE_INTERVALS(vp0,vp1,vp2,dv0,dv1,dv2,dv0dv1,dv0dv2,a,b,c,x0,x1);

  /* compute interval for triangle 2 */
  float d,e,f,y0,y1;
  NEWCOMPUTE_INTERVALS(up0,up1,up2,du0,du1,du2,du0du1,du0du2,d,e,f,y0,y1);

  float xx,yy,xxyy,tmp;
  xx=x0*x1;
  yy=y0*y1;
  xxyy=xx*yy;

  tmp=a*xxyy;
  isect1[0]=tmp+b*x1*yy;
  isect1[1]=tmp+c*x0*yy;

  tmp=d*xxyy;
  isect2[0]=tmp+e*xx*y1;
  isect2[1]=tmp+f*xx*y0;

  SORT(isect1[0],isect1[1]);
  SORT(isect2[0],isect2[1]);

  if(isect1[1]<isect2[0] || isect2[1]<isect1[0]) return 0;
  return 1;
}
