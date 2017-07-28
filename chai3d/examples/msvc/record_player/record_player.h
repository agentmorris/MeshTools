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
    \author:    Chris Sewell and Dan Morris
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#if !defined(AFX_record_player_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_record_player_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

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
#include "CDriverSensoray626.h"

#include "bass.h"
#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include <mmsystem.h>

const int MAX_VAL_SLIDERS_P = 100000;
const int MAX_VAL_SLIDERS_I = 10000000;
const int FREQ = 1000;

// length of time for chart (in 50ms blocks)
const int MAX_TIME = 50;
const double chartClock = 50;

// Creates texture coordinate for a record
void createTexCoords(cMesh *a_mesh, double radius);

// Get information about the loaded resources
int getNumRecords();
const char* getRecordName(int a_index);

class Crecord_playerApp : public CWinApp {
public:
	Crecord_playerApp();

	int LoadModel(char* filename);

  
  // Call this in place of a destructor to clean up
  void uninitialize();

  virtual int render_loop();

  // haptic tool
  cMeta3dofPointer* tool;

  // The high-precision timer that's used (optionally) to run
  // the haptic loop
  cPrecisionTimer timer;

  // animates the object
  void animateObject(cVector3d force);
	void load_record(int a_index);
	void commandRFD(double desPos);

  // record mesh
  cMesh* m_recordMesh;

  // interaction force between proxy and disc in global coordinates
  cVector3d m_interactionForce;
  cVector3d m_proxyPos;
  double m_torque;

  // clock
  cPrecisionClock m_clock;

  //=========================
  // RFD RELATED
  //=========================

  cDriverSensoray626 * sensorayBoard;
  double DACvalue;

  // RFD angle
  double m_angle;
  // total reduction
  double m_reduction;
  // encoder counts per turn
  double m_cpt;
  // desired point
  double m_desiredPos;
  // P controller gain
  double m_P;
  // I controller gain
  double m_I;
  // integrator memory
  double m_integratorVal;
  // D controller term
  double m_D;
  // position at the last time step
  double m_lastAngle;
  double m_velocity;
  double m_velocityOld;

  // actual commanded action from P, I, K term
  double m_actionK;
  double m_actionI;
  double m_actionD;

  // define clock for D term
  cPrecisionClock *m_time;

  double m_RFDInitialAngle;
  bool m_inContact;
  double m_lastGoodPosition;
  
  // properties of the object
  double m_rotPos;
  double m_rotVel;
  double m_inertia;

  double cameraAngleH;
  double cameraAngleV;
  double cameraDistance;
  cVector3d cameraPosition;
  bool flagCameraInMotion;
  int mouseX, mouseY;

  // An object of some kind, to be rendered in the scene
  cMesh* object;

  // Sets the camera zoom level
  void zoom(int zoom_level);

  // Handles mouse-scroll events (moves or rotates the selected object)
  void scroll(CPoint p, int left_button = 1);

  // Handles mouse clicks (marks the front-most clicked object as 'selected')
  void select(CPoint p);
  
  // The currently selected object (or zero when there's no selection)
  cGenericObject* selected_object;

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
  cViewport* viewport;
  cLight* light;
  
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Crecord_playerApp)
	public:
	virtual BOOL InitInstance();
  virtual int Run();
  //}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(Crecord_playerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_record_player_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
