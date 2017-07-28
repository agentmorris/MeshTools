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
#ifndef CGenericPotentialFieldH
#define CGenericPotentialFieldH
//---------------------------------------------------------------------------
#include "CGenericObject.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file       CGenericPotentialField.h
      \class      cGenericPotentialField
      \brief      cGenericPotentialField describes a generic class to create
                  objects which are describe by implicit functions
*/
//===========================================================================
class cGenericPotentialField : public cGenericObject
{
  public:

    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cGenericPotentialField.
    cGenericPotentialField() {};
    //! Destructor of cGenericPotentialField.
    virtual ~cGenericPotentialField() {};

    // METHODS:
    //! Compute interaction forces between a probe and this object, descending through child objects
    virtual cVector3d computeForces(const cVector3d& a_probePosition);

  protected:

    // VIRTUAL METHODS:

    //! Compute the interaction force for this object in its local frame
    virtual cVector3d computeLocalForce(const cVector3d& a_localProbePosition)  { return (cVector3d(0,0,0)); };

    // PROPERTIES:
    //! Force
    cVector3d m_globalForce;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
