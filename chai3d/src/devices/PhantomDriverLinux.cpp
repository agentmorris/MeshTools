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
    \author:    Dan Morris
    \version    1.0
    \date       03/2006
*/
//===========================================================================

#ifndef _DISABLE_PHANTOM_SUPPORT

#include "PhantomDriverLinux.h"
#include "assert.h"
#include <iostream>
#include <HD/hd.h>
#include <HDU/hduVector.h>
#include <HDU/hduError.h>

// =============================================================================
// Global variables
// =============================================================================

// Tells us how many phantoms are open; incremented with each call to OpenPhantom
// and reset with each call to ClosePhantoms.
int numdev = 0;

// Handle for the scheduler
HDSchedulerHandle gSchedulerCallback = HD_INVALID_HANDLE;

hduVector3Dd pos[2];
hduVector3Dd force[2];
hduVector3Dd vel[2];
hduVector3Dd torque[2];
typedef HDdouble matrix[16];
matrix mat[2];

#define NUM_BUTTONS 2
HDint stylus_switch[2];

HHD myPhantom[2] = {HD_INVALID_HANDLE, HD_INVALID_HANDLE};
char myPhantomNames[2][100];


bool phantomStatus[2] = {false, false};
bool forceStatus[2] = {false, false};


double device_workspace_center_x[2];
double device_workspace_center_y[2];
double device_workspace_center_z[2];

// This value is used to scale device positions (in mm) to normalized
// positions (-1 --> 1).  I.e. normalized pos = pos / cube_side
double cube_side[2];

int model_type;


//=============================================================================
// Callbacks
//=============================================================================
HDCallbackCode HDCALLBACK HapticCallBack(void *data)
{
  int valid; 
  int single = 0;

  if (myPhantom[0] == HD_INVALID_HANDLE)
    single = 1;
  else
    valid = 0;

  if (myPhantom[1] == HD_INVALID_HANDLE)
    single = 1;
  else
    valid = 1;
    
  // if only one of the two devices is active
  if (single)
  {
    hdBeginFrame(myPhantom[valid]);
    
    // read stuff
    hdGetDoublev(HD_CURRENT_POSITION,pos[valid]);
    hdGetDoublev(HD_CURRENT_VELOCITY,vel[valid]);
    hdGetDoublev(HD_CURRENT_TRANSFORM,mat[valid]);

    // In general, we want the position of the device at which 
    // forces are applied, but if we do want the tip position,
    // this is how we would get it:
#if 0
    // If this is an omni, xlate to the tip
    if (model_type == 5) {

      HDdouble* xform = mat[valid];
      hduVector3Dd zaxis;

      // The xform matrix is column major, so the z-axis is
      // in elements (8,9,10)
      zaxis[0] = xform[8];    
      zaxis[1] = xform[9];  
      zaxis[2] = xform[10];  
      
// The distance (in mm) of the omni tip along the current +z axis
#define OMNI_TIP_TRANSLATE -39

      (pos[valid])[0] += zaxis[0] * OMNI_TIP_TRANSLATE;
      (pos[valid])[1] += zaxis[1] * OMNI_TIP_TRANSLATE;
      (pos[valid])[2] += zaxis[2] * OMNI_TIP_TRANSLATE;
      
    }

#endif

    hdGetIntegerv(HD_CURRENT_BUTTONS,&stylus_switch[valid]);
    // write forces
    hdSetDoublev(HD_CURRENT_FORCE, force[valid]);
    // if device is 6DOF, render torques too
    HDint num_dof;

    // For debugging Phantom errors...
#define PRINT_HD_ERRORS
#ifdef PRINT_HD_ERRORS
    HDErrorInfo error;
    
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
      // hduPrintError(stderr, &error, "HDAPI device error encountered");      
      HDstring s = hdGetErrorString(error.errorCode);
      if (s == 0) s = "[unknown]";
      // _cprintf("Phantom error 0x%x: %s\n",error.errorCode,s);
    }
#endif
    
    hdGetIntegerv(HD_OUTPUT_DOF ,&num_dof);
    if ((int)num_dof == 6) {
      hdSetDoublev(HD_CURRENT_TORQUE, torque[valid]);
    }

    /*

    // For debugging force issues

    hdDisable(HD_SOFTWARE_FORCE_LIMIT);
    hdEnable(HD_MAX_FORCE_CLAMPING);

    int clamping,ramping,limit,implimit;
    clamping = hdIsEnabled(HD_MAX_FORCE_CLAMPING);
    ramping = hdIsEnabled(HD_FORCE_RAMPING);
    limit = hdIsEnabled(HD_SOFTWARE_FORCE_LIMIT);
    implimit = hdIsEnabled(HD_SOFTWARE_FORCE_IMPULSE_LIMIT);
  
    // _cprintf("Enabled: %d %d %d %d\n",clamping,ramping,limit,implimit);

    //_cprintf("Force %lf %lf %lf\n",force[valid][0],force[valid][1],force[valid][2]);

    */

    hdEndFrame(myPhantom[valid]);

  } // we have only one device

  else  // else do it for both
  {
    hdBeginFrame(myPhantom[0]);
    hdGetDoublev(HD_CURRENT_POSITION,pos[0]);
    hdGetDoublev(HD_CURRENT_VELOCITY,vel[0]);
    hdGetDoublev(HD_CURRENT_TRANSFORM,mat[0]);
    hdGetIntegerv(HD_CURRENT_BUTTONS,&stylus_switch[0]);

    hdBeginFrame(myPhantom[1]);
    hdGetDoublev(HD_CURRENT_POSITION,pos[1]);
    hdGetDoublev(HD_CURRENT_VELOCITY,vel[1]);
    hdGetDoublev(HD_CURRENT_TRANSFORM,mat[1]);
    hdGetIntegerv(HD_CURRENT_BUTTONS,&stylus_switch[1]);


    hdMakeCurrentDevice(myPhantom[0]);
    hdSetDoublev(HD_CURRENT_FORCE, force[0]);
    HDint num_dof;
    hdGetIntegerv(HD_OUTPUT_DOF ,&num_dof);
    if ((int)num_dof == 6) {
      hdSetDoublev(HD_CURRENT_TORQUE, torque[0]);
    }
    
    hdEndFrame(myPhantom[0]);

    hdMakeCurrentDevice(myPhantom[1]);
    hdSetDoublev(HD_CURRENT_FORCE, force[1]);
    hdGetIntegerv(HD_OUTPUT_DOF ,&num_dof);
    if ((int)num_dof == 6) {
      hdSetDoublev(HD_CURRENT_TORQUE, torque[1]);
    }

    hdEndFrame(myPhantom[1]);

  } // we have two devices

  return HD_CALLBACK_CONTINUE;
}


// =============================================================================
// Functions accessible from outside
// =============================================================================
int OpenPhantom(int name)
{
  return OpenPhantom("Default PHANToM");
}


//==========================================================================
/*!
  This function opens starts a scene if no phantom has been previously defined.
  Also this function attempts to create a phantom instance with name "char * name".
  If two phantoms exist or if the name of the phantom is wrong a negative int
  is returned. Otherwise a handle for the phantom is returned (which will be
  0 or 1 in value).

  \fn       OpenPhantom(char* name)
  \param  char * name      

*/
//===========================================================================
int OpenPhantom(char* name)
{
  /*
  Useful for assessing the currently installed version of OpenHaptics:
  HDstring s = hdGetString(HD_VERSION);
  _cprintf("Version: %s\n",s);
  */
  
  //printErrorMessages(false);
  int ret_val;
  
  // stop the scheduler if it is already running so it can be reinitialized 
  // with multiple devices
  if (gSchedulerCallback != HD_INVALID_HANDLE)
  {
    hdStopScheduler();
        hdUnschedule(gSchedulerCallback);
    gSchedulerCallback = HD_INVALID_HANDLE;
  }

  if (numdev == 2)
  {
    // overflow error
    // in this case return without incrementing numdev
    return PH_OF;
  }

  // if phantom already exists we want to create one in the free spot we have
  if (numdev == 1)
  {
      myPhantom[1] = hdInitDevice(name);
      strcpy(myPhantomNames[1], name);
      HDErrorInfo error;
      if (HD_DEVICE_ERROR(error = hdGetError()))
        return PH_INIT_ERR;
      else
        ret_val = 1;    
    }

  if (numdev == 0)
  {
      myPhantom[0] = hdInitDevice(name);
      strcpy(myPhantomNames[0], name);
      HDErrorInfo error;
      if (HD_DEVICE_ERROR(error = hdGetError()))
        return PH_INIT_ERR;
      else
        ret_val = 0;         

  }
  // printErrorMessages(true);
  
  // Set options for the current device

  // Turn force output on
  hdEnable(HD_FORCE_OUTPUT);

  // If there's an error, slowly ramp the forces back up
  hdEnable(HD_FORCE_RAMPING);

  // Don't generate an error if the software force limit is exceeded 
  // (hardware limits, temperature warnings, etc. may still generate
  // errors)
  //
  // Actually doing this, strangely, seems to cause all sort of errors
  // telling us that the device exceeded its maximum force... isn't that
  // ironic?
  
  // hdDisable(HD_SOFTWARE_FORCE_LIMIT);

  // Clamp forces to the device's maximum force
  // hdEnable(HD_MAX_FORCE_CLAMPING);

  
  numdev++;
  int x_center, y_center, z_center;
  if (ret_val >= 0)
  {
    HDdouble usableBounds[6];
    hdGetDoublev(HD_USABLE_WORKSPACE_DIMENSIONS, usableBounds);

    // determine model type
    switch ((int)usableBounds[3])
    {
    case 80:
      {
        // desktop or Omni?
        if (usableBounds[4] == 70)
        {
          // desktop
          model_type = 0;
          // pick the center of the workspace
          x_center = 0;
          y_center = 10;
          z_center = 10;
          break;
        }
        else
        {
          // Omni
          model_type = 5;
          // pick the center of the workspace
          x_center = 0;
          y_center = 10;
          z_center = 10;
          break;
        }

      }
    case 85:
      {
        // premium A 1.0
        model_type = 1;
        // pick the center of the workspace
        x_center = 0;
        y_center = 0;
        z_center = 0;
        break;
      }
    case 140:
      {
        // premium 1.5 or premium 3
        model_type = 2;
        // pick the center of the workspace
        x_center = 0;
        y_center = 10;
        z_center = 32;
        break;
      }
    case 130:
      // 6dof 1.5
      model_type = 3;
      // pick the center of the workspace
      x_center = 0;
      y_center = 10;
      z_center = 32;
      break;
    default:
      // all other phantom models, we'll have a conservative approach with them
      model_type = 4;
    }

    // get dimensions of usable workspace
    double x_total = usableBounds[3] - usableBounds[0];
    double y_total = usableBounds[4] - usableBounds[1];
    double z_total = usableBounds[5] - usableBounds[2];


    // pick the smallest which will be the side of the cube
    int side;
    if ((x_total <= y_total) && (x_total <= z_total))
      side = (int)x_total;
    else 
      if ((y_total <= x_total) && (y_total <= z_total))
        side = (int)y_total;
      else 
        side = (int)z_total;
    
    device_workspace_center_x[ret_val] = (double) x_center;
    device_workspace_center_y[ret_val] = (double) y_center;
    device_workspace_center_z[ret_val] = (double) z_center;
    cube_side[ret_val] = ((double) side) / 2.0;   
  }

  // this is the handle for this phantom from the user perspective
  return ret_val;
}




//==========================================================================
/*!
  This function closes stops the servoloop for all phantoms. This should
  be the last operation called before exiting the program. Also OpenPhantom()
  should not be called after this point.

  \fn       void ClosePhantoms()   
*/
//===========================================================================
void ClosePhantoms()
{
    hdStopScheduler();
    hdUnschedule(gSchedulerCallback);
    if (myPhantom[0] != HD_INVALID_HANDLE)
    {
        hdDisableDevice(myPhantom[0]);
        myPhantom[0] = HD_INVALID_HANDLE;
    }

    if (myPhantom[1] != HD_INVALID_HANDLE)
    {
        hdDisableDevice(myPhantom[1]);
        myPhantom[1] = HD_INVALID_HANDLE;
    }
  
  numdev = 0;
}



//==========================================================================
/*!
  This function resets encoders of phantom num. Returns PH_SUCCESS in case of
  success, a negative value (see error_codes.h) in case of error.

  \fn       int ResetPhantomEncoders(int num)
  \param  int num

*/
//===========================================================================
int   ResetPhantomEncoders(int num)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    int supportedCalibrationStyles;
    int calibrationStyle;
    
    hdGetIntegerv(HD_CALIBRATION_STYLE, &supportedCalibrationStyles);
    if (supportedCalibrationStyles & HD_CALIBRATION_ENCODER_RESET)
    {
      calibrationStyle = HD_CALIBRATION_ENCODER_RESET;
    }
    if(supportedCalibrationStyles & HD_CALIBRATION_INKWELL)
    {
      calibrationStyle = HD_CALIBRATION_INKWELL;
      }
    if (supportedCalibrationStyles & HD_CALIBRATION_AUTO)
    {
        calibrationStyle = HD_CALIBRATION_AUTO;
    }

      if (calibrationStyle == HD_CALIBRATION_ENCODER_RESET)
    {
          hdUpdateCalibration(calibrationStyle);
        if (hdCheckCalibration() == HD_CALIBRATION_OK)
        return PH_SUCCESS;
      else 
        return PH_RES_ENC_ERR;
    }
    else 
      return PH_SUCCESS;
  }
  else
    return PH_OF;
}


//==========================================================================
/*!
  This function starts effect on phantom num. Note that the servoloop is still
  running even if the effect is off. This is to allow one phantom to be off 
  while the other one is on.

  \fn     int StartCommunicationPhantom(int num)
  \param  int num  the Phantom to start talking to (a handle)
*/
//===========================================================================
int StartCommunicationPhantom(int num)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
      if (gSchedulerCallback == HD_INVALID_HANDLE)
    {
      hdStartScheduler();
      HDErrorInfo error;
      if (HD_DEVICE_ERROR(error = hdGetError()))
      {
        return PH_ENABLE_FORCES;
      }
      
      gSchedulerCallback = hdScheduleAsynchronous(
        HapticCallBack, 0, HD_DEFAULT_SCHEDULER_PRIORITY);
      phantomStatus[num] = true;
      forceStatus[num] = false;
      force[num][0] = force[num][1] = force[num][2] = 0.0;
      return PH_SUCCESS;
    }
    else
    {
      hdMakeCurrentDevice(myPhantom[num]);
      hdEnable(HD_FORCE_OUTPUT);
      phantomStatus[num] = true;
      forceStatus[num] = false;
      force[num][0] = force[num][1] = force[num][2] = 0.0;
      return PH_SUCCESS;
    }
  }
  else
    return PH_OF;
}


//==========================================================================
/*!
  This function turns effect off on phantom i. Note that the servoloop is still
  running even if the effect is off. This is to allow one phantom to be off   while the other one is on.

  \fn     int StopCommunicationPhantom(int num)
  \param  int num The Phantom handle to stop talking to.
*/
//===========================================================================

int StopCommunicationPhantom(int num)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    phantomStatus[num] = false;
    hdMakeCurrentDevice(myPhantom[num]);
    hdDisable(HD_FORCE_OUTPUT);
    return PH_SUCCESS;
  }
  else
    return PH_OF;
}


//==========================================================================
/*!
  This function reads the position of the tip of the phantom in cartesian space.
  the position vector is passed by reference. the function returns PH_SUCCESS in case of
  success, a negative value in case of error. Note that positions are returned
  in the phantom reference frames, i.e. x horizontal to the right, y is vertical
  upward, z points toward you. Changes of references may be necessary to work
  in chai.

      \fn       int ReadPositionPhantom(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
    \param  int num   handle to the phantom
    \param  double iPosX, double iPosY, double iPosZ: position vector
*/
//===========================================================================
int ReadPositionPhantom(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    if (phantomStatus[num])
    {
      iPosX = pos[num][0];
      iPosY = pos[num][1];
      iPosZ = pos[num][2];
      return PH_SUCCESS;
    }
    else
    {
      iPosX = 0.0;
      iPosY = 0.0;
      iPosZ = 0.0;
      return PH_SUCCESS;
    }
    
  }
  else
    return PH_OF;
}


//==========================================================================
/*!
  ReadNormalizedPositionPhantom(int num,double &iPosX,double &iPosY,double &iPosZ); reads tip position for phantom num
  the function returns PH_SUCCESS if everything went ok, a negative value otherwise (check list of errors)
  Note that positions are expressed with a value included in the interval [-1,1] for a cube centered in the device's workspace center.
  This is to ensure that a same demo may be used using different devices without having to change any of the code

      \fn       int ReadNormalizedPositionPhantom(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
    \param  int num   handle to the phantom
    \param  double iPosX, double iPosY, double iPosZ: position vector
*/
//===========================================================================
int ReadNormalizedPositionPhantom(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    if (phantomStatus[num])
    {
      iPosX = (pos[num][0] - device_workspace_center_x[num])/cube_side[num];
      iPosY = (pos[num][1] - device_workspace_center_y[num])/cube_side[num];
      iPosZ = (pos[num][2] - device_workspace_center_z[num])/cube_side[num];
      return PH_SUCCESS;
    }
    else
    {
      iPosX = 0.0;
      iPosY = 0.0;
      iPosZ = 0.0;
      return PH_SUCCESS;
    }
    
  }
  else
    return PH_OF;
}   


//==========================================================================
/*!
  This function sets the forces that will be applied to the tip of the phantom 
  in cartesian space. The force vector is passed by reference. the function 
  returns PH_SUCCESS in case of success, a negative value in case of error. Note that 
  forces are considered in the phantom reference frames, i.e. x horizontal to the right, y is vertical
  upward, z points toward you. Changes of references may be necessary to work
  in chai.

  \fn       int SetForcePhantom(int num, 
                    const double &iForceX,
                    const double &iForceY,
                    const double &iForceZ)
  \param  int num   handle to the phantom
  \param  double iForceX, double iForceY, double iForceZ: force vector
*/
//===========================================================================
int SetForcePhantom(int num, const double &iForceX, const double &iForceY,
  const double &iForceZ)
{
  // make sure the first forces we pass to the device are zero
  
  // This is now enforced on the client-side, in CHAI
  // if ((fabs(iForceX) == 0.0) && (fabs(iForceY) == 0.0) && (fabs(iForceZ) == 0.0) && (!forceStatus[num]))
    forceStatus[num] = true;
    
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    if ((phantomStatus[num]) && (forceStatus[num]))
    {
      force[num][0] = iForceX;
      force[num][1] = iForceY;
      force[num][2] = iForceZ;           
      return PH_SUCCESS;
    }
    else
    {
      force[num][0] = 0.0;
      force[num][1] = 0.0;
      force[num][2] = 0.0;
      return PH_SUCCESS;
    }
  }
  else
    return PH_OF;
}


//==========================================================================
/*!
  This function sets the forces and torques that will be applied to the tip of the phantom 
  in cartesian space. The force and torque vectors are passed by reference. the function 
  returns PH_SUCCESS in case of success, a negative value in case of error. Note that 
  torque are considered with respect to the phantom reference frames, i.e. x horizontal to the right, y is vertical
  upward, z points toward you. Changes of references may be necessary to work
  in chai.

      \fn       int SetForceTorquePhantom(int num, 
                  const double &iForceX,
                          const double &iForceY,
                  const double &iForceZ,
                  const double &iTorqueX,
                          const double &iTorqueY,
                  const double &iTorqueZ)

    \param  int num   handle to the phantom
    \param  double iTorqueX, double iTorqueY, double iTorqueZ: Torque vector
*/
//===========================================================================
int SetForceTorquePhantom(int num, const double &iForceX,
  const double &iForceY, const double &iForceZ,
  const double &iTorqueX, const double &iTorqueY,
  const double &iTorqueZ)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    if (phantomStatus[num])
    {
      force[num][0] = iForceX;
      force[num][1] = iForceY;
      force[num][2] = iForceZ;
      torque[num][0] = iTorqueX;
      torque[num][1] = iTorqueY;
      torque[num][2] = iTorqueZ;

      return PH_SUCCESS;
    }
    else
    {
      force[num][0] = 0.0;
      force[num][1] = 0.0;
      force[num][2] = 0.0;
      torque[num][0] = 0.0;
      torque[num][1] = 0.0;
      torque[num][2] = 0.0;
      return PH_SUCCESS;
    }
  }
  else
    return PH_OF;
}


//==========================================================================
/*!
  This function reads the orientation of the phantom stylus in the case of a 
  3dof wrist with respect to a chai reference frame. Returns PH_SUCCESS in case of success,
  a negative value in case of error.

  \fn int ReadOrientMat3DOFPhantom(int num,double *m)
  \param  num   handle to the phantom
  \param  m Returned orientation matrix
*/
//===========================================================================
int ReadOrientMat3DOFPhantom(int num, double *m)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    if (phantomStatus[num])
    {
      m[0] = mat[num][0];
      m[1] = mat[num][1];
      m[2] = mat[num][2];
      m[3] = mat[num][4];
      m[4] = mat[num][5];
      m[5] = mat[num][6];
      m[6] = mat[num][8];
      m[7] = mat[num][9];
      m[8] = mat[num][10];

      return PH_SUCCESS;
    }
    else
    {
      m[0] = 1;
      m[1] = 0;
      m[2] = 0;
      m[3] = 0;
      m[4] = 1;
      m[5] = 0;
      m[6] = 0;
      m[7] = 0;
      m[8] = 1;
      return PH_SUCCESS;
    }
    
  }
  else
    return PH_OF;

}


//==========================================================================
/*!
  This function reads the Phantom switch. Returns 0 or 1 if everything is
  fine, negative value in case of error.

  \fn       int ReadSwitchPhantom(int num)
  \param  int num   handle to the phantom
  \return -1 for an error, or a bitmask with button 0 in bit 0, etc.
*/
//===========================================================================
int ReadSwitchPhantom(int num)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
    return stylus_switch[num];
  else
    return PH_OF;
}


//==========================================================================
/*!
  This function reads the max force that can be applied by the Phantom switch. 

  \fn       double GetMaxForce(int num)
  \param  int num   handle to the phantom
*/
//===========================================================================
double GetMaxForce(int num)
{
  hdMakeCurrentDevice(myPhantom[num]);
  HDdouble forceMagnitude;
  hdGetDoublev(HD_NOMINAL_MAX_FORCE, &forceMagnitude);
  return (double) forceMagnitude;
}




//==========================================================================
/*!
  This function reads the velocity of the tip of the phantom in cartesian space.
  the velocity vector is passed by reference. the function returns PH_SUCCESS in case of
  success, a negative value in case of error. Note that velocity is returned
  in mm/sec with respect to a phantom reference frames, i.e. x horizontal to the right, y is vertical
  upward, z points toward you. Changes of references may be necessary to work
  in chai.

      \fn       int ReadVelocityPhantom(int num, 
                    double &iVelX,
                    double &iVelY,
                    double &iVelZ)
    \param  int num   handle to the phantom
    \param  double iVelX, double iVelY, double iVelZ: velocity vector
*/
//===========================================================================
int ReadVelocityPhantom(int num, double &iVelX, double &iVelY, double &iVelZ)
{
  if ((num < numdev) && (num >= 0) && (myPhantom[num] != HD_INVALID_HANDLE))  
  {
    if (phantomStatus[num])
    {
      iVelX = vel[num][0];
      iVelY = vel[num][1];
      iVelZ = vel[num][2];
      return PH_SUCCESS;
    }
    else
    {
      iVelX = 0.0;
      iVelY = 0.0;
      iVelZ = 0.0;
      return PH_SUCCESS;
    }
    
  }
  else
    return PH_OF;
}

 
//==========================================================================
/*!
  Reads the scale factor from mm to normalized
  coordinates for Phantom num.  Multiply normalized coordinates by this value to
  get back too mm.

  \fn     int GetWorkspaceScalePhantom(const int& num, double& scale);
  \param  num    Which Phantom should we be reading?
  \param  scale  The returned scale factor.
  \return error code (PH_SUCCESS for success)
*/
//===========================================================================
int GetWorkspaceScalePhantom(const int& num, double& scale)
{
  if ((num >= numdev) || (num < 0) || (myPhantom[num] == HD_INVALID_HANDLE || phantomStatus[num] == false)) {
    scale = 0.0;
    return PH_OF;    
  }

  else {
    scale = cube_side[num];
    return PH_SUCCESS;
  }
}


//==========================================================================
/*!
  Assigns a custom haptic callback to this Phantom.

  \fn     int SetCallbackPhantom(cCallback* a_callback);
  \param  a_callback  The function to call at each haptic loop.
  \return error code
*/
//===========================================================================
int SetCallbackPhantom(cCallback* a_callback)
{
  return PH_DLL_PROBLEM;
}

#endif // _DISABLE_PHANTOM_SUPPORT
