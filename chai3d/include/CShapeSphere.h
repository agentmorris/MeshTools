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
    \date       06/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CShapeSphereH
#define CShapeSphereH
//---------------------------------------------------------------------------
#include "CGenericPotentialField.h"
#include "CMaterial.h"
#include "CTexture2D.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file       CShapeSphere.h
      \class      cShapeSphere
      \brief      cShapeSphere describes a simple spherical potential field
*/
//===========================================================================
class cShapeSphere : public cGenericPotentialField
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cShapeSphere.
    cShapeSphere(const double& a_radius);
    //! Destructor of cSphere.
    virtual ~cShapeSphere() {};

    // MEMBERS:
    //! Material properties
    cMaterial m_material;
    //! Texture property
    cTexture2D* m_texture;

    // METHODS:
    //! Render object in OpenGL.
    virtual void render(const int a_renderMode=0);

    //! Update bounding box of current object
    virtual void updateBoundaryBox();

    //! Object scaling
    virtual void scaleObject(const cVector3d& a_scaleFactors);

    //! Compute interaction force for current object in local frame
    virtual cVector3d computeLocalForce(const cVector3d& a_localPosition);

    //! Set radius of sphere
    void setRadius(double a_radius) { m_radius = cAbs(a_radius); }

    //! Get radius of sphere
    double getRadius() { return (m_radius); }

  protected:
    //! radius of sphere
    double m_radius;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
