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
    \author:    Stephen Sinclair
    \version    1.0
    \date       05/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CFreedom6S3dofPointerH
#define CFreedom6S3dofPointerH
//---------------------------------------------------------------------------
#include "CGeneric3dofPointer.h"
#include "CFreedom6SDevice.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file       CFreedom6S3dofPointer.h
      \class      cFreedom6S3dofPointer
      \brief      cFreedom6S3dofPointer provides a cursor-like tool which is
                  graphically rendered by a small sphere illustrating the tip
                  of the device. The tools interacts with the enviornment
                  by using the finger-proxy algorithm
*/
//===========================================================================
class cFreedom6S3dofPointer : public cGeneric3dofPointer
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cGeneric3dofPointer.
    cFreedom6S3dofPointer(cWorld* a_world);
    //! Destructor of cGeneric3dofPointer.
    virtual ~cFreedom6S3dofPointer();

};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


