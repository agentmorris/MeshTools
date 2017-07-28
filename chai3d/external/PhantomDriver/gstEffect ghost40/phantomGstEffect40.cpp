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
#include <gstScene.h>
#include <gstPHANToM.h>
#include <gstPHANToMInfo.h>
#include <iostream.h>
#include "gstEffect.h"
#include "phantomGstEffect40.h"


// =============================================================================
// Constants
// =============================================================================
const int NO_PHANTOM = -1000;
const int SUCCESS = 1;

// =============================================================================
// Global variables
// =============================================================================

// this is zero when no scene has been defined, 1 otherwise.
// basically use this to be sure not to define a gstScene twice, every time you add
// a phantom.
int scene_initialized = 0;

// predefined value that expresses the absence of a phantom.
int numdev = 0;



static double posX[2];
static double posY[2];
static double posZ[2];

static double velX[2];
static double velY[2];
static double velZ[2];

static double FX[2];
static double FY[2];
static double FZ[2];

static double TX[2];
static double TY[2];
static double TZ[2];

gstTransformMatrix mat0;
gstTransformMatrix mat1;

int stylus_switch[2];

gstScene *scene;
gstSeparator *root;
gstPHANToM *myPhantom[2] = {NULL, NULL}; 
bool phantomStatus[2] = {false, false};
bool forceStatus[2] = {false, false};

class  BasicForce0; 
class  BasicForce1;

BasicForce0 *effect0;
BasicForce1 *effect1;

double device_workspace_center_x[2];
double device_workspace_center_y[2];
double device_workspace_center_z[2];
double cube_side[2];

int model_type;


//=============================================================================
// Callbacks
//=============================================================================

class  BasicForce0: public gstEffect 
{
public:

    // Constructor.
    BasicForce0(){};

    // Calculate force for effect at each servo tick.
    virtual gstVector calcEffectForce(void *phantom); 
    virtual gstVector calcEffectForce(void *phantom, gstVector &torques); 

};

class  BasicForce1: public gstEffect 
{
public:

    // Constructor.
    BasicForce1(){};

    // Calculate force for effect at each servo tick.
    virtual gstVector calcEffectForce(void *phantom); 
    virtual gstVector calcEffectForce(void *phantom, gstVector &torques); 

};

gstVector BasicForce0::calcEffectForce(void *phantom)
{

    gstPHANToM *PHANToM = (gstPHANToM *)phantom;
    gstVector pos = PHANToM->getPosition_WC();
	gstVector vel = PHANToM->getVelocity();
	// save such data in the shared mem position vector
	posX[0] = pos[0];	
	posY[0] = pos[1];
	posZ[0] = pos[2];

	velX[0] = vel[0];	
	velY[0] = vel[1];
	velZ[0] = vel[2];


	// stylus switch
	stylus_switch[0] = myPhantom[0]->getStylusSwitch();

	mat0 = myPhantom[0]->getRotationMatrix();

	// make a force vector from shared memory
	// check if forces are enabled or not
	gstVector force;
	force = gstVector(FX[0], FY[0], FZ[0]) ;
	
	return force;
}

gstVector BasicForce0::calcEffectForce(void *phantom, gstVector & torques) 
{
    // Not using torque
    torques = gstVector(TX[0],TY[0],TZ[0]);
    return calcEffectForce(phantom);
}




gstVector BasicForce1::calcEffectForce(void *phantom)
{
    gstPHANToM *PHANToM = (gstPHANToM *)phantom;
    gstVector pos = PHANToM->getPosition_WC();
	gstVector vel = PHANToM->getVelocity();

	// save such data in the shared mem position vector
	posX[1] = pos[0];	
	posY[1] = pos[1];
	posZ[1] = pos[2];

	velX[1] = vel[0];	
	velY[1] = vel[1];
	velZ[1] = vel[2];

	// stylus switch
	stylus_switch[1] = myPhantom[1]->getStylusSwitch();

	// orientation
	mat1 = myPhantom[1]->getRotationMatrix();

	gstVector force;
	force = gstVector(FX[1], FY[1], FZ[1]) ;
	return force;
}



gstVector BasicForce1::calcEffectForce(void *phantom, gstVector & torques) 
{
    // Not using torque
    torques = gstVector(TX[1],TY[1],TZ[1]);
    return calcEffectForce(phantom);
}




//=============================================================================
// DLL entry point
// =============================================================================

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

	return TRUE;
}



// =============================================================================
// Functions accessible from outside
// =============================================================================


//==========================================================================
/*!
	This function opens starts a scene if no phantom has been previously defined.
	Also this function attempts to create a phantom instance with name "char * name".
	If two phantoms exist or if the name of the phantom is wrong a negative int
	is returned. Otherwise a handle for the phantom is returned (which will be
	0 or 1 in value).

      \fn       OpenPhantom(char * name)
	  \param	char * name      

*/
//===========================================================================


FUNCTION int __stdcall   OpenPhantom(char * name)
{
	printErrorMessages(false);
	if (!scene_initialized)
	{
		// initialize the gstScene and the gstRoot;
		// Create a GHOST scene object.
		scene = new gstScene;

		// create the root separator and set it as the root of the scene graph
		root = new gstSeparator();
		scene->setRoot(root);
		scene_initialized = 1;
	}


	int ret_val;	

	if (numdev == 2)
	{
		// overflow error
		// in this case return without incrementing numdev
		return PH_OF;
	}

	// if phantom already exists we want to create one in the free spot we have
	if (numdev == 1)
		{
			myPhantom[1] = new gstPHANToM(name);
			if (!myPhantom[1] || !myPhantom[1]->getValidConstruction()) 
				return PH_INIT_ERR;
			else
				ret_val = 1;

			root->addChild(myPhantom[1]);
			
			// set the effect for myPhantom[1]
			effect1 = new BasicForce1;
			myPhantom[1]->setEffect(effect1);

		}

	if (numdev == 0)
	{
		myPhantom[0] = new gstPHANToM(name);
		if (!myPhantom[0] || !myPhantom[0]->getValidConstruction()) 
			return PH_INIT_ERR;
		else
			ret_val = 0;
		
		root->addChild(myPhantom[0]);
			
		// set the effect for myPhantom[0]
		effect0 = new BasicForce0;
		myPhantom[0]->setEffect(effect0);

	}
	printErrorMessages(true);
	
	numdev++;

	if (ret_val >= 0)
	{
		gstDimensionsStruct max = myPhantom[ret_val]->getInfo()->maxUsableWorkspace;
		int x_center, y_center, z_center;

		// determine model type
		switch (max.xMax)
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
			// no other phantom models are currently supported
			model_type = 4;
		}

		// get dimensions of usable workspace
		int x_total = max.xMax - max.xMin;
		int y_total = max.yMax - max.yMin;
		int z_total = max.zMax - max.zMin;


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

		
	// this is the handle for this phantom from the user perspective
	return ret_val;
}




//==========================================================================
/*!
	This function closes stops the servoloop for all phantoms. This should
	be the last opeartion called before exiting the program. Also OpenPhantom()
	should not be called after this point.

      \fn       void ClosePhantoms()
	  
*/
//===========================================================================


FUNCTION void __stdcall   ClosePhantoms()
{
	if (scene_initialized)
	{
		scene->stopServoLoop();
		if (myPhantom[0] != NULL)
		{
			root->removeChild(myPhantom[0]);
			myPhantom[0]->stopEffect();
			delete myPhantom[0];
		}
		if (myPhantom[1] != NULL)
		{
			root->removeChild(myPhantom[1]);
			myPhantom[1]->stopEffect();
		}	delete myPhantom[1];
	}
	numdev = 0;
}



//==========================================================================
/*!
	This function resets encoders of phantom num. Returns 1 in case of success, 
	a negative value (see error_codes.h) in case of error.

      \fn       int ResetPhantomEncoders(int num)
	  \param	int num

*/
//===========================================================================

FUNCTION int __stdcall  ResetPhantomEncoders(int num)
{
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))	
	{
		int isDesktopModel = myPhantom[num]->getInfo()->isDesktop;
		if (!isDesktopModel)
			myPhantom[num]->resetEncoders();
		return SUCCESS;
	}
	else
		return PH_OF;
}


//==========================================================================
/*!
	This function starts effect on phantom i. Note that the servoloop is still
	running even if the effect is off. This is to allow one phantom to be off 
	while the other one is on.

      \fn       int StartCommunication(int i)
	  \param	int i
*/
//===========================================================================
FUNCTION int __stdcall   StartCommunication(int i)
{
	if ((i< numdev) && (i >= 0) && (scene_initialized) && (myPhantom[i] != NULL))
	{
		scene->stopServoLoop();
		myPhantom[i]->startEffect();	
		scene->startServoLoop();
		phantomStatus[i] = true;
		forceStatus[i] = false;
		return SUCCESS;
	}
	else
		return PH_OF;
}


//==========================================================================
/*!
	This function turns effect off on phantom i. Note that the servoloop is still
	running even if the effect is off. This is to allow one phantom to be off 	while the other one is on.

      \fn       int StopCommunication(int i)
	  \param	int i
*/
//===========================================================================

FUNCTION int __stdcall   StopCommunication(int i)
{
	if ((i < numdev) && (i >= 0) && (scene_initialized) && (myPhantom[i] != NULL))
	{
		phantomStatus[i] = false;
		FX[i] = 0.0;
		FY[i] = 0.0;
		FZ[i] = 0.0;
		TX[i] = 0.0;
		TY[i] = 0.0;
		TZ[i] = 0.0;

		return SUCCESS;
	}
	else
		return PH_OF;
}


//==========================================================================
/*!
	This function reads the position of the tip of the phantom in cartesian space.
	the position vector is passed by reference. the function returns 1 in case of
	success, a negative value in case of error. Note that positions are returned
	in the phantom reference frames, i.e. x horizontal to the right, y is vertical
	upward, z points toward you. Changes of references may be necessary to work
	in chai.

      \fn       int ReadPosition(int num, 
									  double &iPosX,
									  double &iPosY,
									  double &iPosZ)
	  \param	int num		handle to the phantom
	  \param	double iPosX, double iPosY, double iPosZ: position vector
*/
//===========================================================================
FUNCTION int __stdcall   ReadPosition(int num, 
									  double &iPosX,
									  double &iPosY,
									  double &iPosZ)
{
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))
	{
		if (phantomStatus[num])
		{
			iPosX = posX[num];
			iPosY = posY[num];
			iPosZ = posZ[num];
			return SUCCESS;
		}
		else
		{
			iPosX = 0.0;
			iPosY = 0.0;
			iPosZ = 0.0;
			return SUCCESS;
		}
		
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
	  \param	int num		handle to the phantom
	  \param	double iPosX, double iPosY, double iPosZ: position vector
*/
//===========================================================================

FUNCTION int __stdcall   ReadNormalizedPosition(int num, 
									  double &iPosX,
									  double &iPosY,
									  double &iPosZ)
{
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))
	{
		if (phantomStatus[num])
		{
			iPosX = (posX[num] - device_workspace_center_x[num])/cube_side[num];
			iPosY = (posY[num] - device_workspace_center_y[num])/cube_side[num];
			iPosZ = (posZ[num] - device_workspace_center_z[num])/cube_side[num];
			return SUCCESS;
		}
		else
		{
			iPosX = 0.0;
			iPosY = 0.0;
			iPosZ = 0.0;
			return SUCCESS;
		}
		
	}
	else
		return PH_OF;
}		


//==========================================================================
/*!
	This function sets the forces that will be applied to the tip of the phantom 
	in cartesian space. The force vector is passed by reference. the function 
	returns 1 in case of success, a negative value in case of error. Note that 
	forces are considered in the phantom reference frames, i.e. x horizontal to the right, y is vertical
	upward, z points toward you. Changes of references may be necessary to work
	in chai.

      \fn       int SetForce(int num, 
									  double &iForceX,
									  double &iForceY,
									  double &iForceZ)
	  \param	int num		handle to the phantom
	  \param	double iForceX, double iForceY, double iForceZ: force vector
*/
//===========================================================================
FUNCTION int __stdcall   SetForce(int num, 
								  const double &iForceX,
				                  const double &iForceY,
								  const double &iForceZ)
{
	// make sure the first forces we pass to the device are zero
	if ((fabs(iForceX) == 0.0) && (fabs(iForceY) == 0.0) && (fabs(iForceZ) == 0.0) && (!forceStatus[num]))
		forceStatus[num] = true;

	
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))
	{
		if ((phantomStatus[num]) && (forceStatus[num]))
		{
			FX[num] = iForceX;
			FY[num] = iForceY;
			FZ[num] = iForceZ;
			return SUCCESS;
		}
		else
		{
			FX[num] = 0.0;
			FY[num] = 0.0;
			FZ[num] = 0.0;
			return SUCCESS;
		}
	}
	else
		return PH_OF;

}


//==========================================================================
/*!
	This function sets the forces and torques that will be applied to the tip of the phantom 
	in cartesian space. The force and torque vectors are passed by reference. the function 
	returns 1 in case of success, a negative value in case of error. Note that 
	torque are considered with respect to the phantom reference frames, i.e. x horizontal to the right, y is vertical
	upward, z points toward you. Changes of references may be necessary to work
	in chai.

      \fn       int SetForceTorque(int num, 
								  const double &iForceX,
				                  const double &iForceY,
								  const double &iForceZ,
								  const double &iTorqueX,
				                  const double &iTorqueY,
								  const double &iTorqueZ)

	  \param	int num		handle to the phantom
	  \param	double iTorqueX, double iTorqueY, double iTorqueZ: Torque vector
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
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))
	{
		if (phantomStatus[num])
		{
			FX[num] = iForceX;
			FY[num] = iForceY;
			FZ[num] = iForceZ;
			TX[num] = iTorqueX;
			TY[num] = iTorqueY;
			TZ[num] = iTorqueZ;

			return SUCCESS;
		}
		else
		{
			FX[num] = 0.0;
			FY[num] = 0.0;
			FZ[num] = 0.0;
			TX[num] = 0.0;
			TY[num] = 0.0;
			TZ[num] = 0.0;
			return SUCCESS;
		}
	}
	else
		return PH_OF;

}

//==========================================================================
/*!
	This function reads the orientation of the phantom stylus in the case of a 
	3dof wrist with respect to a chai reference frame. Returns 1 in case of success,
	a negative value in case of error.

      \fn       int ReadOrientMat3DOF(int num, 
 									  double *m)
	  \param	int num		handle to the phantom
	  \param	double *m: array representing matrix
*/
//===========================================================================


FUNCTION int __stdcall   ReadOrientMat3DOF(int num, 
 									  double *m)
{
	if (num == 0)
	{
		if (myPhantom[num] == NULL)
			return PH_OF;
		else
		{
			for (int i = 0; i<3; i++)
				for (int j = 0; j<3; j++)
					m[i*3 + j] = mat0[i][j];
			
			return SUCCESS;
		}
	}

	if (num == 1)
	{
		if (myPhantom[num] == NULL)
			return PH_OF;
		else
		{
			for (int i = 0; i<3; i++)
				for (int j = 0; j<3; j++)
					m[i*3 + j] = mat1[i][j];			
			
			return SUCCESS;
		}
	}
			
	return PH_OF;

}



//==========================================================================
/*!
	This function reads the Phantom switch. Returns 0 or 1 if everything is
	fine, negative value in case of error.

      \fn       int ReadSwitch(int num)
	  \param	int num		handle to the phantom
*/
//===========================================================================
FUNCTION int __stdcall   ReadSwitch(int num)
{
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))
		return stylus_switch[num];
	else
		return PH_OF;
}




//==========================================================================
/*!
	This function reads the max force that can be applied by the Phantom switch. 


      \fn       double GetMaxForce(int num)
	  \param	int num		handle to the phantom
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
	This function reads the velocity of the tip of the phantom in cartesian space.
	the velocity vector is passed by reference. the function returns 1 in case of
	success, a negative value in case of error. Note that velocity is returned
	in mm/sec with respect to a phantom reference frames, i.e. x horizontal to the right, y is vertical
	upward, z points toward you. Changes of references may be necessary to work
	in chai.

      \fn       int ReadVelocity(int num, 
									  double &iVelX,
									  double &iVelY,
									  double &iVelZ)
	  \param	int num		handle to the phantom
	  \param	double iVelX, double iVelY, double iVelZ: velocity vector
*/
//===========================================================================
FUNCTION int __stdcall   ReadVelocity(int num, 
									  double &iVelX,
									  double &iVelY,
									  double &iVelZ)
{
	if ((num < numdev) && (num >= 0) && (scene_initialized) && (myPhantom[num] != NULL))
	{
		if (phantomStatus[num])
		{
			iVelX = velX[num];
			iVelY = velY[num];
			iVelZ = velZ[num];
			return SUCCESS;
		}
		else
		{
			iVelX = 0.0;
			iVelY = 0.0;
			iVelZ = 0.0;
			return SUCCESS;
		}
		
	}
	else
		return PH_OF;
}

 
