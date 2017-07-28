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
    \author:    Chris Sewell
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#if !defined(AFX_dynamic_ode_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_dynamic_ode_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include "CWorld.h"
#include "CViewport.h"
#include "CGenericObject.h"
#include "CPhantom3dofPointer.h"
#include "CMeta3dofPointer.h"
#include "CPrecisionTimer.h"
#include "CLight.h"
#include "CPrecisionClock.h"
#include "CODEMesh.h"
#include "CProxyPointForceAlgo.h"

// A slightly modified proxy algorithm is required to support ODE;
// we override the relevant functions in a subclass...
class cODEProxy : public cProxyPointForceAlgo
{    
  
protected:
    //! Remove the approximate test for whether the proxy has reached the goal point; use exact distances
    virtual bool goalAchieved(const cVector3d& a_proxy, const cVector3d& a_goal) const
    {
        // Always fail this test to force the proxy to continue...
        return false;
    }

    //! Remove the offsetting of the goal to account for proxy volume; use exact collision vectors
    virtual void offsetGoalPosition(cVector3d& a_goal, const cVector3d& a_proxy) const
    {
        // Leave the goal where it is...
        return;
    }
  
public:
    //! A constructor that copies relevant initialization state from another proxy
    cODEProxy(cProxyPointForceAlgo* a_oldProxy)
    {
        m_deviceGlobalPos = a_oldProxy->getDeviceGlobalPosition();
        m_proxyGlobalPos = a_oldProxy->getProxyGlobalPosition();
        m_lastGlobalForce.zero();
        m_world = a_oldProxy->getWorld();
        m_radius = a_oldProxy->getProxyRadius();
    }
};


// A global function for sticking a cube in the given mesh
// 
// Manually creates the 12 triangles (two per face) required to
// model a cube
void createCube(cMesh *mesh, float edge, int include_top = 1);

class Cdynamic_odeApp : public CWinApp {
public:
  Cdynamic_odeApp();
  
  // Call this in place of a destructor to clean up
  void uninitialize();

  virtual int render_loop();

	cODEMesh* create_wall(double a_x, double a_y, double a_z, bool a_ground);

  // The box to push around using ODE
  cODEMesh* object;

  // Cubes to form a room in which to push the box
  cODEMesh* bottom_wall;
  cODEMesh* left_wall;
  cODEMesh* right_wall;
  cODEMesh* back_wall;
  cODEMesh* front_wall;

  // Light source
  cLight* light;

  // Grab relevant options from checkboxes and sliders in the GUI
  //
  // Optionally preserve properties loaded directly from file
  void update_options_from_gui(int preserve_loaded_properties=0);

  // Move variables loaded from a mesh out to the GUI  
  void copy_rendering_options_to_gui();
  
  // Sets the camera zoom level
  void zoom(int zoom_level);

  // Handles mouse-scroll events (moves or rotates the selected object)
  void scroll(CPoint p, int left_button = 1);

  // Handles mouse clicks (marks the front-most clicked object as 'selected')
  void select(CPoint p);

  // The currently selected object (or zero when there's no selection)
  cGenericObject* selected_object;

  // The interface to the haptic device...
  cMeta3dofPointer *tool1;

  // A second haptic interface...
  cMeta3dofPointer *tool2;

  // The high-precision timer that's used (optionally) to run
  // the haptic loop
  cPrecisionTimer haptics_timer;

	// ODE clock
  cPrecisionClock *ode_clock;

  // A flag that indicates whether the haptic loop should be running
  int haptics_enabled;

  // Flags that indicate whether haptics are currently enabled on each device
	int first_device_enabled;
  int second_device_enabled;

  // A flag that indicates whether the haptics thread is currently running
  //
  // This flag does not get set when the haptic callback is driven from a 
  // multimedia timer 
  int haptics_thread_running;

	// Callback function for ODE
  static void nearCallback (void *data, dGeomID o1, dGeomID o2);

  bool ready;

	// This flag is set when the user first moves the tool into the area surrounded
  // by the walls; force rendering is delayed until then so that you don't start
	// out stuck outside the area of interest
  int tool1_ready;
  int tool2_ready;

#define TOGGLE_HAPTICS_TOGGLE  -1
#define TOGGLE_HAPTICS_DISABLE  0
#define TOGGLE_HAPTICS_ENABLE   1

  // If the parameter is -1, haptics are toggled on/off
  // If it's 0 haptics are turned off
  // If it's 1 haptics are turned on
  void toggle_haptics(int enable = TOGGLE_HAPTICS_TOGGLE);
  void toggle_second_device(int enable = TOGGLE_HAPTICS_TOGGLE);
  
  // Reinitializes the CHAI viewport
  void reinitialize_viewport(int stereo_enabled=0);

  // virtual world, camera, and viewport
  cWorld* world;
  cCamera* camera;
  cViewport* viewport;

	//! ODE world
	dWorldID		ode_world;
	//! ODE space
	dSpaceID		ode_space;
	//! ODE contact group
	dJointGroupID	ode_contact_group;
	//! Stepsize for ODE simulation
	float ode_step;
	//! Pointer to callback function
	dNearCallback  *ode_collision_callback;
	//! Pointer to any extra data
	void *ode_collision_callback_data;

  int keys_to_handle[1024];
  
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cdynamic_odeApp)
	public:
	virtual BOOL InitInstance();
  virtual int Run();
  //}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(Cdynamic_odeApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_dynamic_ode_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
