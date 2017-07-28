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
    \author:    Francois Conti
    \version    1.1
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CGenericDeviceH
#define CGenericDeviceH
//---------------------------------------------------------------------------

#include "CCallback.h"

//===========================================================================
/*!
    \file   CGenericDevice.h
    \brief  The following constants define a set of generic commands
            supported by the cGenericDevice:command method. For each
            generic command, we describe the data type and information
            that must be passed by parameter for 'iIndex' and 'iData'.
            'iCommand' contains of course the command number corresponding
            to the following list of command constants.
*/
//===========================================================================

//---------------------------------------------------------------------------
// GENERIC DEVICE or BOARD:
//---------------------------------------------------------------------------
//! Purpose:    Query device to check if is operating correctly.
//! iData:      integer type value. 1 means device is ok
//!             0 means device is not ready.
const int   CHAI_CMD_GET_DEVICE_STATE   = 1010;

//---------------------------------------------------------------------------
// GENERIC I/O BOARDS:
//---------------------------------------------------------------------------
//! Purpose:    Read the value of an encoder N.
//! iData:      integer type value.
const int   CHAI_CMD_GET_ENCODER_0      = 1020;
const int   CHAI_CMD_GET_ENCODER_1      = 1021;
const int   CHAI_CMD_GET_ENCODER_2      = 1022;
const int   CHAI_CMD_GET_ENCODER_3      = 1023;
const int   CHAI_CMD_GET_ENCODER_4      = 1024;
const int   CHAI_CMD_GET_ENCODER_5      = 1025;
const int   CHAI_CMD_GET_ENCODER_6      = 1026;
const int   CHAI_CMD_GET_ENCODER_7      = 1027;

//! Purpose:    Reset the value of an encoder N.
//! iData:      integer type value.
const int   CHAI_CMD_RESET_ENCODER_0      = 1040;
const int   CHAI_CMD_RESET_ENCODER_1      = 1041;
const int   CHAI_CMD_RESET_ENCODER_2      = 1042;
const int   CHAI_CMD_RESET_ENCODER_3      = 1043;
const int   CHAI_CMD_RESET_ENCODER_4      = 1044;
const int   CHAI_CMD_RESET_ENCODER_5      = 1045;
const int   CHAI_CMD_RESET_ENCODER_6      = 1046;
const int   CHAI_CMD_RESET_ENCODER_7      = 1047;

//! Purpose:    Set value to a DAC.
//! iData:      integer type value.
const int   CHAI_CMD_SET_DAC_0          = 1030;
const int   CHAI_CMD_SET_DAC_1          = 1031;
const int   CHAI_CMD_SET_DAC_2          = 1032;
const int   CHAI_CMD_SET_DAC_3          = 1033;
const int   CHAI_CMD_SET_DAC_4          = 1034;
const int   CHAI_CMD_SET_DAC_5          = 1035;
const int   CHAI_CMD_SET_DAC_6          = 1036;
const int   CHAI_CMD_SET_DAC_7          = 1037;

//---------------------------------------------------------------------------
// GENERIC POINT CONTACT 3/6 DOF HAPTIC DEVICES:
//---------------------------------------------------------------------------
//! Purpose:    Read position (px, py, pz) in _millimeters_ of 3d point contact device.
//! iData:      cVector3d type value.
const int   CHAI_CMD_GET_POS_3D         = 2000;

//! Purpose:    Read normalized position (px, py, pz) of 3d point contact device.
//!             typically the value of each component of the vector position will
//!             be included in the interval [-1,1], accounting for the maximum
//!             usable workspace of the device.
//! iData:      cVector3d type value.
const int   CHAI_CMD_GET_POS_NORM_3D    = 2001;

//! Purpose:    Read velocity (vx, vy, vz) of 3d point contact device in millimeters/s.
//! iData:      cVector3d type value.
const int   CHAI_CMD_GET_VEL_3D         = 2002;

//! Purpose:    Set a force (fx, fy, fz) to a 3d point contact device (in Newtons).
//! iData:      cVector3d type value.
const int   CHAI_CMD_SET_FORCE_3D       = 2010;

//! Purpose:    Set a normalized force (fx, fy, fz) to a 3d point contact device.
//!             A normalized force has a maximum length of 1.0 corresponding
//!             to the highest force that the device can generate.
//! iData:      cVector3d type value.
const int   CHAI_CMD_SET_FORCE_NORM_3D  = 2011;

//! Purpose:    Set a force (fx, fy, fz) and a torque (tx, ty, tz) to a 6d point contact device.
//! iData:      array of 2 cVector3d type value.  Units are N and N*mm.
const int   CHAI_CMD_SET_FORCE_TORQUE_3D       = 2012;

//! Purpose:    Read orientation angles (ax, ay, az) of a 3d wrist or stylus.
//! iData:      cVector3d type value.
const int   CHAI_CMD_GET_ROT_ANGLES     = 2020;

//! Purpose:    Read orientation matrix of a 3d wrist or stylus.
//! iData:      cMatrix3d type value.
const int   CHAI_CMD_GET_ROT_MATRIX     = 2021;

//! Purpose:    Set a torque (tx, ty, tz) to a 3d wrist or stylus.
//! iData:      cVector3d type value.  Units are N*mm.
const int   CHAI_CMD_SET_TORQUE_3D      = 2030;

//! Purpose:    Read status of user switch.
//! iData:      Integer type value.
const int   CHAI_CMD_GET_SWITCH_0       = 2041;
const int   CHAI_CMD_GET_SWITCH_1       = 2042;
const int   CHAI_CMD_GET_SWITCH_2       = 2043;

// Reads all switches into a bit mask with bit 0 = button 0, etc.
const int   CHAI_CMD_GET_SWITCH_MASK       = 2044;

//! Purpose:    Get the scale factor from normalized coordinates to mm
//! iData:      double scale factor... mm = scale * normalized_coords
const int   CHAI_CMD_GET_NORMALIZED_SCALE_FACTOR = 2045;

//===========================================================================
/*!
    \brief  The following constants define the possible return values
            of the method cGenericDevice:: command().
*/
//===========================================================================
const int   CHAI_MSG_OK                 =   0;
const int   CHAI_MSG_ERROR              =  -1;
const int   CHAI_MSG_NOT_IMPLEMENTED    =  -2;
const int   CHAI_MSG_SYSTEM_NOT_READY   =  -3;


//===========================================================================
/*!
    \class  cGenericDevice
    \brief  cGenericDevice describes a virtual class from which all
            devices and hardware interfaces are derived.
*/
//===========================================================================
class cGenericDevice
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cGenericDevice.
    cGenericDevice() { m_systemAvailable = false; m_systemReady = false; m_callback = 0; };
    //! Destructor of cGenericDevice.
    virtual ~cGenericDevice() {};

    // METHODS:
    //! Open connection to device (0 indicates success)
    virtual int open() { return -1; }
    //! Close connection to device (0 indicates success)
    virtual int close() { return -1; }
    //! Initialize or calibrate device (0 indicates success)
    virtual int initialize(const bool a_resetEncoders=false) { return -1; }
    //! Send a command to the device (0 indicates success)
    virtual int command(int a_command, void* a_data) { return CHAI_MSG_NOT_IMPLEMENTED; }

    //! Returns true if the device is available for communication
    bool isSystemAvailable() { return m_systemAvailable; }
    //! Returns true if the device is ready for us
    bool isSystemReady() { return m_systemReady; }

    //! Ask the device to call me back periodically
    virtual bool setCallback(cCallback* m_callback);

  protected:
    //! Flag that indicates is hardware device is available.
    bool m_systemAvailable;
    //! Flag that indicates if connection to system was opened successfully.
    bool m_systemReady;
    //! A callback method for this device (or zero if none has been registered)
    cCallback* m_callback;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
