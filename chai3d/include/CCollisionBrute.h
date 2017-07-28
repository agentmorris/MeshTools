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
#ifndef CCollisionBruteH
#define CCollisionBruteH
//---------------------------------------------------------------------------
#include "CMaths.h"
#include "CTriangle.h"
#include "CVertex.h"
#include "CGenericCollision.h"
#include <vector>
//---------------------------------------------------------------------------
using std::vector;
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file     CCollisionBrute.h
      \class    cCollisionBrute
      \brief    cCollisionBrute provides methods to check for the intersection
                of a line segment with a mesh by checking all triangles in the
                mesh.
*/
//===========================================================================
class cCollisionBrute : public cGenericCollision
{
  public:
    // CONSTRUCTOR & DESTRUCTOR
    //! Constructor of cCollisionBrute.
    cCollisionBrute(vector<cTriangle> *a_triangles) : m_triangles(a_triangles) {}
    //! Destructor of cCollisionBrute.
    virtual ~cCollisionBrute() { }

    // VIRTUAL METHODS:
    //! No initialization is necessary for the brute force method.
    virtual void initialize() {};
    //! There isn't really a useful "visualization" of "check all triangles".
    virtual void render() {};
    //! Return the nearest triangle intersected by the given segment, if any.
    virtual bool computeCollision(cVector3d& a_segmentPointA,
            cVector3d& a_segmentPointB, cGenericObject*& a_colObject,
            cTriangle*& a_colTriangle, cVector3d& a_colPoint,
            double& a_colSquareDistance, int a_proxyCall = -1);

  protected:
    //! Pointer to the list of triangles in the mesh.
    vector<cTriangle> *m_triangles;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

