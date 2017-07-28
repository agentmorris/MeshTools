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
#ifndef CPhantomDevicesH
#define CPhantomDevicesH
/*!
    \file CPhantomDevices.h
*/
// Allow Phantom support to be compiled out
#ifndef _DISABLE_PHANTOM_SUPPORT

//---------------------------------------------------------------------------
#include "CGenericDevice.h"

#ifdef _WIN32
#include "PhantomDriver.h"
#else
#include "PhantomDriverLinux.h"
#endif

#include "CMatrix3d.h"
#include "CVector3d.h"
//---------------------------------------------------------------------------
/*!
    \class cPhantomDevice
    \brief
    Class to interface with Phantom devices
*/
class cPhantomDevice : public cGenericDevice
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cPhantomDevice.
    cPhantomDevice(int a_num = 0, bool a_dio_access = false);
    //! Destructor of cPhantomDevices.
    ~cPhantomDevice();

    // METHODS:
    //! Open connection to phantom device.
    virtual int open();
    //! Close connection to phantom device.
    virtual int close();
    //! Calibrate phantom device.
    virtual int initialize(const bool a_resetEncoders=false);
    //! Send a command to the phantom device.
    virtual int command(int a_command, void* a_data);

#ifdef _WIN32
    //! Ask the device to call me back periodically
    virtual bool setCallback(cCallback* m_callback);
#endif

  private:
   //! handle for specific phantom use.
   int m_phantomHandle;

   //! Number of Phantoms that have been started.
   static int m_num_phantoms;

   //! Callback function passed to phantom dll
   static void callbackFunc(void* a_data);
};

#endif // #ifndef _DISABLE_PHANTOM_SUPPORT

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

