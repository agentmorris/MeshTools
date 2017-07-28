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
    \author:    Federico Barbagli
    \version    1.1
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CMeta3dofPointerH
#define CMeta3dofPointerH
//---------------------------------------------------------------------------
#include "CGeneric3dofPointer.h"
#include "CPhantomDevices.h"
#include "CDeltaDevices.h"

#ifdef _WIN32
#include "CVirtualDevice.h"
#include "CFreedom6S3dofPointer.h"
#endif

//---------------------------------------------------------------------------

#define DHD_VIRTUAL_EXE_NAME "dhdVirtual.exe"

// Constants representing each of the devices supported by the meta pointer
#define DEVICE_PHANTOM 0
#define DEVICE_DHD     1
#define DEVICE_VIRTUAL 2
#define DEVICE_MPB     3
#define DEVICE_UNINITIALIZED -1

//===========================================================================
/*!
      \file       CMeta3dofPointer.h
      \class      cMeta3dofPointer
      \brief      cMeta3dofPointer connects a generic haptic device as
                  a simple pointer in space. The tool is visually graphically
                  rendered by a small sphere illustrating the tip of the
                  device. The tool tries to connect to a a SensAble device, a
                  ForceDimension device (delta/omega), and a Freedom-6 (in that
                  order); if none are available it loads a virtual haptic device.
*/
//===========================================================================
class cMeta3dofPointer : public cGeneric3dofPointer
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cPhantom3dofPointer.
    cMeta3dofPointer(cWorld* a_world, unsigned int a_deviceNumber = 0, bool a_dio_access = false);
    //! Destructor of cPhantom3dofPointer.
    virtual ~cMeta3dofPointer();

    //! Asks which physical device the meta pointer is communicating with; returns
    //! -1 for uninitialized or a value from the constants above
    int getPhysicalDevice() const { return m_physicalDevice; }

  private:
    // Which physical device is currently in use? (-1 when uninitialized)
    int m_physicalDevice;

};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


