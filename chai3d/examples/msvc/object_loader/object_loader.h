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

#if !defined(AFX_object_loader_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_object_loader_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

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
#include "CLabelPanel.h"
#include "CShaders.h"
#include "CCallback.h"

typedef enum {
  MOUSE_BUTTON_RIGHT=0,MOUSE_BUTTON_LEFT,MOUSE_BUTTON_MIDDLE
} mouse_buttons;

// A global function for sticking a cube in the given mesh
// 
// Manually creates the 12 triangles (two per face) required to
// model a cube
void createCube(cMesh *mesh, float edge);


// A function used to pop up a dialog box and ask the user to select
// a file (for selecting game files) (last five parameters are optional).
int FileBrowse(char* buffer, int length, int save=0, char* forceExtension=0,
               char* extension_string=0, char* title=0);

class Cobject_loaderApp : public CWinApp, cCallback {
public:
	Cobject_loaderApp();
  
  // Call this in place of a destructor to clean up
  void uninitialize();

  virtual int render_loop();

  // A GLSL shader object, not used by default but inserted into
  // the scenegraph when the user loads a shader
  cGLSLShader* shader;

  // An object of some kind, to be rendered in the scene
  cMesh* object;

  // Light source
  cLight* light;

  // A text label that tells me what file I most recently loaded
  cLabelPanel* label;

  // Grab relevant options from checkboxes and sliders in the GUI
  //
  // Optionally preserve properties loaded directly from file
  void update_options_from_gui(int preserve_loaded_properties=0);

  // Move variables loaded from a mesh out to the GUI  
  void copy_rendering_options_to_gui();

  // Move the on-screen object if necessary
  //
  // This function is run on the haptics thread if it's running,
  // on the graphics thread otherwise
  void animate();

  // Loads the specified shader from a supported shader format file
  int LoadShader(const char* filename);

  // Loads the specified model from a supported 3d model format file
  int LoadModel(char* filename);

  // Loads a texture (image) file and applies it to the current
  // object
  int LoadTexture(char* filename);
  
  // Sets the camera zoom level
  void zoom(int zoom_level);

  // Handles mouse-scroll events (moves or rotates the selected object)  
  void scroll(CPoint p, int button = MOUSE_BUTTON_LEFT);

  // Handles mouse clicks (marks the front-most clicked object as 'selected')
  void select(CPoint p);

  // The currently selected object (or zero when there's no selection)
  cGenericObject* selected_object;

  // The interface to the haptic device...
  cMeta3dofPointer *tool;

  // The high-precision timer that's used (optionally) to run
  // the haptic loop
  cPrecisionTimer timer;

  // A flag that indicates whether haptics are currently enabled
  int haptics_enabled;

  // A flag that indicates that we should turn haptics off for just a bit
  bool disable_haptics_temporarily;

  // Used to indicate that the haptics thread "got the message"
  bool disable_haptics_temporarily_received;

  // A flag that indicates whether the haptics thread is currently running
  //
  // This flag does not get set when the haptic callback is driven from a 
  // multimedia timer 
  int haptics_thread_running;

  // A flag that indicates whether we're currently moving the selected object
  int moving_object;

#define TOGGLE_HAPTICS_TOGGLE  -1
#define TOGGLE_HAPTICS_DISABLE  0
#define TOGGLE_HAPTICS_ENABLE   1

  // If the parameter is -1, haptics are toggled on/off
  // If it's 0 haptics are turned off
  // If it's 1 haptics are turned on
  void toggle_haptics(int enable = TOGGLE_HAPTICS_TOGGLE);

  // Enables or disables the background animation of the current object
  void toggle_animation();

  // Keep track of the time that elapsed between iterations of
  // the animate() function, and the wall-clock time at which the
  // last iteration ran
  double m_last_animation_time;
  
  // The current velocity at which the object is moving (units/s)
  cVector3d m_animation_velocity;

  // Reinitializes the CHAI viewport
  void reinitialize_viewport(int stereo_enabled=0);

  // virtual world
  cWorld* world;
  cCamera* camera;
  cViewport* viewport;

  int keys_to_handle[1024];
  
  void callback();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cobject_loaderApp)
	public:
	virtual BOOL InitInstance();
  virtual int Run();
  //}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(Cobject_loaderApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_object_loader_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
