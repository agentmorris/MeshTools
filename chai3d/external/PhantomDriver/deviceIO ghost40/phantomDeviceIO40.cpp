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
    \version    1.2
    \date       01/2004
*/
//===========================================================================



#include "stdafx.h"
#include "assert.h"
#define _BUILD_DLL_
#include <gstPHANToM.h>
#include <iostream.h>
#include <gstDeviceIO.h>
#include "phantomDeviceIO40.h"

typedef struct _servoLoopData 
{
    int id;
    float maxStiffness; 
} servoLoopData;


// =============================================================================
// Constants
// =============================================================================
const int NO_PHANTOM = -1000;

// =============================================================================
// Global Variables
// =============================================================================
int init_servo = 0;

int device[2] = {NO_PHANTOM, NO_PHANTOM};
int numdev = 0;

static int forces_enabled[2] = {0,0}; // forces are disables at first

static double posX[2];
static double posY[2];
static double posZ[2];

static double FX[2];
static double FY[2];
static double FZ[2];

double mat0[3][3];
double mat1[3][3];

bool stylus_switch[2];
int counter = 0;

double device_workspace_center_x[2];
double device_workspace_center_y[2];
double device_workspace_center_z[2];
double cube_side[2];

// model type:
// 0 - desktop
int model_type;


GST_SCHEDULER_CALLBACK  servo(void *pUserData);

//=============================================================================
//  DLL entry point
//=============================================================================
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
           )
{

  return TRUE;
}



//=============================================================================
//  Functions accessible from outside
//=============================================================================


//==========================================================================
/*!
   int OpenPhantom(char * name),  creates and initializes phantom with name char* name
    If the phantom is opened successfully the function returns a handle 
    (non-negative integer) for the phantom. If something goes wrong a negative value
    is returned.

      \fn       int OpenPhantom(char * name)
    \param  char * name - name of the phantom to be opened
*/
//===========================================================================
FUNCTION int __stdcall   OpenPhantom(char * name)
{
  int ret_val; 
  gstPHANToMInfoStruct info;

  if (numdev == 2)
  {
    // overflow error
    // in this case return without incrementing numdev
    return PH_OF;
  }

  // if phantom already exists we want to create one in the free spot we have
  if (numdev == 1)
    {
      device[1] = gstInitializePhantom(name);
      if (device[1] < 0)
        ret_val = PH_INIT_ERR;
      else
        ret_val = 1;
    }

  if (numdev == 0)
  {
    device[0] = gstInitializePhantom(name);
    if (device[0] < 0)
      ret_val = PH_INIT_ERR;
    else
      ret_val = 0;
  }

  int x_center, y_center, z_center;
  numdev++;
  if (ret_val >= 0)
  {
    gstGetPhantomInfo(ret_val, info);
    
    // determine model type
    switch (info.maxUsableWorkspace.xMax)
    {
    case 80:
      {
        // desktop
        model_type = 0;
        // pick the center of the workspace
        x_center = 0;
        y_center = 10;
        z_center = 10;
        break;
      }
    case 85:
      {
        // premiumA 1.0
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
    int x_total = info.maxUsableWorkspace.xMax - info.maxUsableWorkspace.xMin;
    int y_total = info.maxUsableWorkspace.yMax - info.maxUsableWorkspace.yMin;
    int z_total = info.maxUsableWorkspace.zMax - info.maxUsableWorkspace.zMin;

    // pick the smallest which will be the side of the cube
    int side;
    if ((x_total <= y_total) && (x_total <= z_total))
      side = x_total;
    else 
      if ((y_total <= x_total) && (y_total <= z_total))
        side = y_total;
      else 
        side = z_total;
    
    device_workspace_center_x[ret_val] = (double) x_center;
    device_workspace_center_y[ret_val] = (double) y_center;
    device_workspace_center_z[ret_val] = (double) z_center;
    cube_side[ret_val] = ((double) side) / 2.0;
  }
    
  // this is the handle for this phantom from the user's perspective
  return ret_val;
}




//==========================================================================
/*!
    This function closes the phantom with handle num. In case no phantom currently
    exists the value returned is negative, 1 in case of success.

    \fn       int ClosePhantom(int num)
    \param  int num
*/
//===========================================================================
FUNCTION void __stdcall   ClosePhantoms()
{
    StopCommunication(device[0]); 
    StopCommunication(device[1]); 
    numdev = 0;
}



//==========================================================================
/*!
    This function starts the Communication with phantom num. Returns 1 if everything
      went OK, a negative value otherwise.


      \fn       int StartCommunication(int num)
    \param  int num
*/
//===========================================================================

FUNCTION int __stdcall   StartCommunication(int num)
{

  if ((device[num] != NO_PHANTOM) && (num < numdev) && (num >= 0))
  {
    int retval;
    forces_enabled[num] = 1;
    retval = gstEnablePhantomForces(device[num]);
    return retval; 
  }
  else
  {
    return PH_OF;
  }
}



//==========================================================================
/*!
  StopCommunication(int num), Stops the effect of phantom num, basically disabling forces and position
  reading for such phantom. Note that the overall servoloop will still be running since the other phantom
  may be not disabled.
  The function returns 1 if everything went ok, a negative value otherwise (check list of errors)
    
    \fn       int StopCommunication(int num)
    \param  int num
*/
//===========================================================================

FUNCTION int __stdcall   StopCommunication(int num)
{
  if ((device[num] != NO_PHANTOM) && (num < numdev) && (num >= 0))
  {
    int retval;
    forces_enabled[num] = 0;
    retval = gstDisablePhantomForces(device[num]);

    return retval; 
  }
  else
  {
    return PH_OF;
  }
}


//==========================================================================
/*!
  This function resets encoders of phantom num. Returns 1 in case of success, 
  a negative value (see error_codes.h) in case of error.

      \fn       int ResetPhantomEncoders(int num)
    \param  int num

*/
//===========================================================================
FUNCTION int __stdcall  ResetPhantomEncoders(int num)
{
  if ((num < numdev) && (num >= 0) && (device[num] != NO_PHANTOM))
  {
    // returns 1 if it was ok, PH_RES_ENC_ERR if there was a problem with the encoders
    // PH_OF if overflow
    if( gstIsPhantomResetNeeded(device[num]) )
      if( gstResetPhantomEncoders(device[num]) != PHTM_OK )
        return PH_RES_ENC_ERR;
    return PH_SUCCESS;    
  }
  else
  {
    // return overflow error, the device does not exist
    // this takes care of when you try to access devices without being there
    return PH_OF;
  }

}




//==========================================================================
/*!
  This function reads the position of the tip of the phantom in cartesian space.
  the position vector is passed by reference. the function returns 1 in case of
  success, a negative value in case of error

      \fn       int ReadPosition(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
    \param  int num   handle to the phantom
    \param  double iPosX, double iPosY, double iPosZ: position vector
*/
//===========================================================================
FUNCTION int __stdcall   ReadPosition(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
{
  if ((num < numdev) && (num >= 0) && (device[num] != NO_PHANTOM) && (forces_enabled[num]))
  {
    int status = gstUpdatePhantom(device[num]);
    gstVector pos;
    gstGetPhantomPosition(device[num],pos);
    iPosX = pos.x();
    iPosY = pos.y();
    iPosZ = pos.z();
    return PH_SUCCESS;
  }
  else
    return PH_OF;
}



//==========================================================================
/*!
  ReadNormalizedPosition(int num,double &iPosX,double &iPosY,double &iPosZ); reads tip position for phantom num
  the function returns 1 if everything went ok, a negative value otherwise (check list of errors)
  Note that positions are expressed with a value included in the interval [-1,1] for a cube centered in the device's workspace center.
  This is to ensure that a same demo may be used using different devices without having to change any of the code

      \fn       int ReadNormalizedPosition(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
    \param  int num   handle to the phantom
    \param  double iPosX, double iPosY, double iPosZ: position vector
*/
//===========================================================================
FUNCTION int __stdcall   ReadNormalizedPosition(int num, 
                    double &iPosX,
                    double &iPosY,
                    double &iPosZ)
{
  if ((num < numdev) && (num >= 0) && (device[num] != NO_PHANTOM) && (forces_enabled[num]))
  {
    int status = gstUpdatePhantom(device[num]);
    gstVector pos;
    gstGetPhantomPosition(device[num],pos);
    iPosX = (pos.x() - device_workspace_center_x[num])/cube_side[num];
    iPosY = (pos.y() - device_workspace_center_y[num])/cube_side[num];
    iPosZ = (pos.z() - device_workspace_center_z[num])/cube_side[num];
    return PH_SUCCESS;
  }
  else
    return PH_OF;
}
 

//==========================================================================
/*!
  SetForce(int num,const double &iForceX,const double &iForceY,const double &iForceZ); writes force to phantom num
  the function returns 1 if everything went ok, a negative value otherwise (check list of errors)
  Note that forces are expressed in Newtons with respect to a Ghost reference frame (X: right, Y: up, Z: toward user)
  Note: no safety features are implemented other than the standard Ghost ones.
  
  

      \fn       int SetForce
    \param  int num
    \param  int iForceX
    \param  int iForceY
    \param  int iForceZ
*/
//===========================================================================
FUNCTION int __stdcall   SetForce(int num, 
                  const double &iForceX,
                          const double &iForceY,
                  const double &iForceZ)
{
  if ((num < numdev) && (num >= 0) && (device[num] != NO_PHANTOM) && (forces_enabled[num]))
  {
    gstVector phantomForce(iForceX, iForceY, iForceZ);
    int res = gstSetPhantomForce(device[num], phantomForce);
    if (res<0)
    {
      gstDisablePhantomForces(device[num]);
      gstEnablePhantomForces(device[num]);
    }
    int status = gstUpdatePhantom(device[num]);
    return PH_SUCCESS;
  }
  else
    return PH_OF;
}

//==========================================================================
/*!
  SetForceTorque(int num, const double &iForceX, const double &iForceY, const double &iForceZ, const double &iTorqueX, const double &iTorqueY, const double &iTorqueZ);
  writes Forces and Torques to phantom num.
  the function returns 1 if everything went ok, a negative value otherwise (check list of errors)
  Note that forces are expressed in Newtons and torques are expressed in Newtons Meter with respect to a Phantom reference frame (X: right, Y: up, Z: toward user)
  Note: no safety features are implemented other than the standard Ghost ones.
  
  

      \fn       int SetForceTorque
    \param  int num
    \param  int iForceX
    \param  int iForceY
    \param  int iForceZ
    \param  int iTorqueX
    \param  int iTorqueY
    \param  int iTorqueZ
*/
//===========================================================================
FUNCTION int __stdcall   SetForceTorque(int num, 
                  const double &iForceX,
                          const double &iForceY,
                  const double &iForceZ,
                  const double &iTorqueX,
                          const double &iTorqueY,
                  const double &iTorqueZ)
{
  if ((num < numdev) && (num >= 0) && (device[num] != NO_PHANTOM) && (forces_enabled[num]))
  {
    gstVector phantomForce(iForceX, iForceY, iForceZ);
    gstVector phantomTorque(iTorqueX, iTorqueY, iTorqueZ);
    gstSetPhantomForce(device[num], phantomForce, phantomTorque);
    int status = gstUpdatePhantom(device[num]);
    return PH_SUCCESS;
  }
  else
    return PH_OF;
}

//==========================================================================
/*!
  This function reads the orientation of the phantom stylus in the case of a 
  3dof wrist with respect to a SAI reference frame.

      \fn       int ReadOrientMat3DOF(int num, 
                    double *m)
    \param  int num   handle to the phantom
    \param  double *m: array representing matrix
*/
//===========================================================================
FUNCTION int __stdcall   ReadOrientMat3DOF(int num, 
                    double *m)
{
  if (num == 0)
  {
    if (device[num] == NO_PHANTOM)
      return PH_OF;
    else
    {
      gstTransformMatrix gmat;
      gstGetStylusMatrix(device[num], gmat);
      gmat.getRotationMatrix(mat0);
      for (int i = 0; i<3; i++)
        for (int j = 0; j<3; j++)
          m[i*3 + j] = mat0[i][j];
      return PH_SUCCESS;
    }
  }

  if (num == 1)
  {
    if (device[num] == NO_PHANTOM)
      return PH_OF;
    else
    {
      gstTransformMatrix gmat;
      gstGetStylusMatrix(device[num], gmat);
      gmat.getRotationMatrix(mat1);
      for (int i = 0; i<3; i++)
        for (int j = 0; j<3; j++)
          m[i*3 + j] = mat1[i][j];

      return PH_SUCCESS;
    }
  }
    
  return PH_MAT_ERR;

}



//==========================================================================
/*!
  Returns 0 if switch is not pressed, 1 if it is pressed.

      \fn       int ReadSwitch(int num)
    \param  int num   handle to the phantom
    
*/
//===========================================================================
FUNCTION int __stdcall   ReadSwitch(int num)
{
  int SwitchState = gstGetStylusSwitchState(device[num]);
  return SwitchState;
}



//==========================================================================
/*!
    This function returns the max force that can be commanded to the phantom

    \fn       double GetMaxForce(int num)
    \param  int num   handle to the phantom
    \return   double: max force value
*/
//===========================================================================
FUNCTION double __stdcall   GetMaxForce(int num)
{
  if ((model_type == 0) || (model_type == 1) || (model_type == 4))
    return 6.0;
  else
    return 8.0;
}


//==========================================================================
/*!
  Reads the velocity of phantom num in phantom reference frame and returns it
  in iVelX, iVelY, iVelZ. Returns 1 if everything went ok, a negative value otherwise.

      \fn       int ReadVelocity(int num, double &iVelX, double &iVelY, double &iVelZ)

    \param  int num   handle to the phantom
    \param  double iVelX, iVelY, iVelZ: velocity components
*/
//===========================================================================
FUNCTION int __stdcall   ReadVelocity(int num, double &iVelX, double &iVelY, double &iVelZ)
{
  if ((num < numdev) && (num >= 0) && (device[num] != NO_PHANTOM) && (forces_enabled[num]))
  {
    int status = gstUpdatePhantom(device[num]);
    gstVector vel;
    gstGetPhantomVelocity(device[num],vel);
    iVelX = vel.x();
    iVelY = vel.y();
    iVelZ = vel.z();
    return PH_SUCCESS;
  }
  else
    return PH_OF;

}
