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
#ifndef CDelta3dofPointerH
#define CDelta3dofPointerH
//---------------------------------------------------------------------------
#include "CGeneric3dofPointer.h"
#include "CDeltaDevices.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file      CDelta3dofPointer.h
      \class     cDelta3dofPointer
      \brief     cDelta3dofPointer inherites from all tool properties
                 of cGeneric3dofPointer.
*/
//===========================================================================
class cDelta3dofPointer : public cGeneric3dofPointer
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cDelta3dofPointer.
    cDelta3dofPointer(cWorld* a_world, unsigned int a_deviceNumber);
    //! Destructor of cDelta3dofPointer.
    virtual ~cDelta3dofPointer();
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


