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
    \author:    Dan Morris
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#if !defined(AFX_mass_springs_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_mass_springs_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

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

// A couple of new object types required for our mass-spring
// simulation

class CBall;

// A spring (rendered as a line) used to connect two "balls"
class CSpring : public cGenericObject {

public:

  CSpring();
  virtual ~CSpring();

  // The graphic radius of the spring
  float m_radius;

  // The rest length (in world units) of the spring
  double m_rest_length;

  // The stiffness constant for the spring
  double m_spring_constant;

  // The objects to which this spring is connected
  CBall* m_endpoint_1;
  CBall* m_endpoint_2;

  // Transiently computed at each time step
  double m_current_length;

  // Render the spring graphically
  virtual void render(const int a_renderMode=0);

};


// A ball (rendered as a sphere) that can be pulled around by 
// springs
class CBall : public cGenericObject {

public:

  CBall();
  virtual ~CBall();

  // The graphic radius of the ball
  float m_radius;

  // The mass of the ball
  double m_mass;

  // Springs to which I'm attached
  std::vector<CSpring*> m_springs;

  // My current world-space velocity
  cVector3d m_velocity;

  virtual void render (const int a_renderMode=0);
  
  // Transiently computed at each time step
  cVector3d current_force;

  // A material property set for rendering the ball
  cMaterial m_material;

};

// Various physical constants

#define FLOOR_X_SIZE 200.0
#define FLOOR_Z_SIZE 200.0
#define FLOOR_Y_POSITION -0.3
#define DEFAULT_FLOOR_SPRING_CONSTANT 100.0
#define INITIAL_BALL_SPACING 0.5
#define DEFAULT_OBJECT_MASS 0.5

// Should be a negative value
#define DAMPING_CONSTANT -1.0
#define GRAVITY_CONSTANT -1.0
#define DEFAULT_SPRING_CONSTANT 20.0

// The initial number of masses
#define INITIAL_NUM_BALLS 6

// A constant to translate virtual forces into real haptic
// forces
#define HAPTIC_FORCE_CONSTANT 0.1


class Cmass_springsApp : public CWinApp {
public:
	
  // Called from the haptics loop if it's running (from the
  // graphics loop otherwise) to compute and integrate forces
  //
  // Updates the position and velocity of all the masses
  void compute_spring_forces();

  // The masses and springs currently being simulated 
  std::vector<CBall*> m_active_balls;
  std::vector<CSpring*> m_active_springs;

  // The computed force to be applied to the haptic device
  cVector3d m_haptic_force;
  
  // A mesh representing the "floor"
  cMesh* m_floor;

  // The spring constant applied to pull the balls up above the floor
  float m_floor_spring_constant;

  Cmass_springsApp();
  
  // Call this in place of a destructor to clean up
  void uninitialize();

  // The graphic rendering loop
  virtual int render_loop();

  // Flags to synchronize ball addition/removal
  int addball_pending, removeball_pending;

  // Add a mass to or remove a mass from the end of the chain
  void add_ball();
  void remove_ball();

  // Sets the camera zoom level
  void zoom(int zoom_level);

  // Handles mouse-scroll events (moves or rotates the selected object)
  void scroll(CPoint p, int left_button = 1);

  // Handles mouse clicks (marks the front-most clicked object as 'selected')
  void select(CPoint p);
  
  // The currently selected object (or zero when there's no selection)
  cGenericObject* selected_object;

  // The interface to the haptic device...
  cGeneric3dofPointer *tool;

  // The high-precision timer that's used (optionally) to run
  // the haptic loop
  cPrecisionTimer timer;

  // A flag that indicates whether haptics are currently enabled
  int haptics_enabled;

  // A flag that indicates whether the haptics thread is currently running
  //
  // This flag does not get set when the haptic callback is driven from a 
  // multimedia timer 
  int haptics_thread_running;

#define TOGGLE_HAPTICS_TOGGLE  -1
#define TOGGLE_HAPTICS_DISABLE  0
#define TOGGLE_HAPTICS_ENABLE   1

  // If the parameter is -1, haptics are toggled on/off
  // If it's 0 haptics are turned off
  // If it's 1 haptics are turned on
  void toggle_haptics(int enable = TOGGLE_HAPTICS_TOGGLE);

  // virtual world
  cWorld* world;
  cCamera* camera;
  cLight* light;
  cViewport* viewport;

  
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cmass_springsApp)
	public:
	virtual BOOL InitInstance();
  virtual int Run();
  //}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(Cmass_springsApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_mass_springs_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
