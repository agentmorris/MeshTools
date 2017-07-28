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
    \author:    Francois Conti
    \version    1.1
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CGenericCollisionH
#define CGenericCollisionH
//---------------------------------------------------------------------------
#include "CVector3d.h"
#include "CMatrix3d.h"
#include "CVertex.h"
#include "CTriangle.h"
#include "CMaterial.h"
#include <vector>
//---------------------------------------------------------------------------
using std::vector;
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file     cGenericCollision.h
      \class    cGenericCollision
      \brief    cGenericCollision is an abstract class for collision-detection
                algorithms for meshes with line segments.
*/
//===========================================================================
class cGenericCollision
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cGenericCollision
    cGenericCollision();
    //! Destructor of cGenericCollision
    virtual ~cGenericCollision() {};

    // VIRTUAL METHODS:
    //! Do any necessary initialization, such as building trees.
    virtual void initialize() {};
    //! Provide a visual representation of the method.
    virtual void render() {};
    //! Return the nearest triangle intersected by the given segment, if any.
    virtual bool computeCollision(cVector3d& a_segmentPointA,
        cVector3d& a_segmentPointB, cGenericObject*& a_colObject,
        cTriangle*& a_colTriangle, cVector3d& a_colPoint,
        double& a_colSquareDistance, int a_proxyCall = -1)
        { return (false); }

    // METHODS:
    //! Set level of collision tree to display.
    void setDisplayDepth(unsigned int a_depth) { m_displayDepth = a_depth; }
    //! Read level of collision tree being displayed.
    double getDisplayDepth() const { return (m_displayDepth); } 
    //! Color properties of the collision object
    cMaterial m_material;

  protected:
    // MEMBERS:
    //! Level of collision tree to render... negative values force rendering
    //! up to and including this level, positive values render _just_ this level
    int m_displayDepth;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

