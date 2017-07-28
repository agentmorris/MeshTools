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

#include "stdafx.h"
#include "sounds.h"
#include "soundsDlg.h"
#include "CCallback.h"
#include <conio.h>
#include <process.h>

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

const double MESH_SCALE_SIZE = 1.0;

// vector of sound-enabled meshes in the world
std::vector<cSoundMesh*> sound_meshes;
cGenericObject* contactObject;
cVector3d normalForce, tangentialForce;


class cSoundsHapticCallback : public cCallback
{
public:
  cGeneric3dofPointer* m_tool;
  void callback()
  {
	  m_tool->updatePose();
	  m_tool->computeForces();
	  if (m_tool->getProxy()->getContactObject())
      {
        contactObject = m_tool->getProxy()->getContactObject();
        normalForce = m_tool->getProxy()->getNormalForce();
        tangentialForce = m_tool->getProxy()->getTangentialForce();
      }
	  m_tool->applyForces();
  }
};


/***

  This example demonstrates two ways to drive a haptic loop
  in CHAI.  One can use the cPrecisionTimer class, which uses
  the Windows Multimedia Timer system to request a 1kHz callback,
  which calls the program's haptic loop.  One can also spawn
  a high-priority thread that runs the haptic loop in a while(1)
  loop.

  The former approach uses less CPU but may not really deliver
  1kHz in all cases.  The latter allows complete control of the CPU
  but may slow down other threads.

  This #define controls which approach is used by this application.
***/


/***

  CHAI supports two modes of communication with the Phantom... via
  the gstEffect Ghost class, and via Ghost's direct i/o model.  This
  #define allows you to switch between them.  Each may perform better
  in specific situations.

***/

#define USE_PHANTOM_DIRECT_IO 1


#define ALLOCATE_SCOPED_GLOBALS
#include "sounds_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(CsoundsApp, CWinApp)
    //{{AFX_MSG_MAP(CsoundsApp)
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

CsoundsApp theApp;

//HSTREAM stream;


// Write the requested data from the .wav file to the sound card
DWORD CALLBACK MyStreamWriter(HSTREAM handle, void *buf, DWORD len, DWORD user)
{
    char *cb=(char*)buf;

    cSoundMesh* sound_mesh;
    sound_mesh = sound_meshes[user];
    for (unsigned int i=0; i<len; i++) 
    cb[i] = sound_mesh->getSound()->play(/*sound_mesh->getContactForce()*/);
    return len; 
}


CsoundsApp::CsoundsApp() {

	selected_object = 0;

	haptics_enabled = 0;

	tool = 0;

	AllocConsole();

	HWND con_wnd = GetConsoleWindow();

	// We want the console to pop up over any non-CHAI windows, although
	// we'll put the main dialog window above it.
	::SetForegroundWindow(con_wnd);
	  
	SetWindowPos(con_wnd,HWND_TOP,0,0,0,0,SWP_NOSIZE);

	g_main_app = this;
}


void CsoundsApp::uninitialize() {
	toggle_haptics(TOGGLE_HAPTICS_DISABLE);
}


void CsoundsApp::createObject(std::string fileName, cVector3d loc, cColorf color, sounds type, int tag)
{
    cSoundMesh* new_mesh = new cSoundMesh(world);
    new_mesh->loadFromFile(fileName);

    new_mesh->rotate(cVector3d(1,0,0), -M_PI/2.0);

    // compute size of object
    new_mesh->computeBoundaryBox(true);
    cVector3d min = new_mesh->getBoundaryMin();
    cVector3d max = new_mesh->getBoundaryMax();

    // This is the "size" of the object
    cVector3d span = cSub(max, min);
    double size = cMax(span.x, cMax(span.y, span.z));

    // We'll center all vertices, then multiply by this amount,
    // to scale to the desired size.
    double scaleFactor = MESH_SCALE_SIZE / size;
    new_mesh->scale(scaleFactor);

    // compute size of object again
    new_mesh->computeBoundaryBox(true);

    // Build a collision-detector for this object, so
    // the proxy will work nicely when haptics are enabled.
    new_mesh->createAABBCollisionDetector(true,true);

    // set size of frame
    new_mesh->setFrameSize(0.2, 1.0, true);

    // set size of normals
    new_mesh->setNormalsProperties(0.01, cColorf(1.0, 0.0, 0.0, 1.0), true);

    // update global position
    new_mesh->computeGlobalPositions();

    // set material properties
    cMaterial material;
    material.m_ambient.set( color.getR(), color.getG(), color.getB(), color.getA() );
    material.m_diffuse.set( color.getR(), color.getG(), color.getB(), color.getA() );
    material.m_specular.set( color.getR(), color.getG(), color.getB(), color.getA() );
    material.setShininess(100);
    new_mesh->setMaterial(material, true);
    new_mesh->useMaterial(true, true);

    // set sound parameters and create stream
    new_mesh->getSound()->setParams(type);
    new_mesh->getSound()->stream=BASS_StreamCreate(44100, 1 ,BASS_DEVICE_8BITS,&MyStreamWriter,tag);

    // translate and set stiffness and friction
    new_mesh->translate(loc.x, loc.y, loc.z);
    new_mesh->setStiffness(15.0, true);
    new_mesh->setFriction(0.6, 0.4, true);

    // add this to the world and to the vector of sound meshes
    sound_meshes.push_back(new_mesh);
    world->addChild(new_mesh);
}


BOOL CsoundsApp::InitInstance() {

    AfxEnableControlContainer();

#ifdef _AFXDLL
    Enable3dControls();         // Call this when using MFC in a shared DLL
#else
    Enable3dControlsStatic();   // Call this when linking to MFC statically
#endif

    g_main_dlg = new CsoundsDlg;
	m_pMainWnd = g_main_dlg;

	g_main_dlg->Create(IDD_sounds_DIALOG,NULL);
	    
	// Now we should have a display context to work with...

	world = new cWorld();

	// set background color
	world->setBackgroundColor(0.0f,0.0f,0.0f);


	// Create a camera
	camera = new cCamera(world);

	// set camera position and orientation
	// 
	// We choose to put it out on the positive z axis, so things appear
	// the way OpenGL users expect them to appear, with z going in and
	// out of the plane of the screen.
	int result = camera->set(cVector3d(0,0,1.5),   // position of camera
				cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
				cVector3d(0.0, 1.0, 0.0));  // orientation of camera (standing up)

	if (result == 0) {
		_cprintf("Could not create camera...\n");
	}

	// Turn on one light...
	light = new cLight(world);

	light->setEnabled(true);

	// Use a purely directional light, pointing from
	// somewhere off to the left...
	light->setDirectionalLight(true);
	light->setPos(cVector3d(-1, 1, 1));
	  
	// Create a display for graphic rendering
	viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);

		// Initialize sound device and create audio stream
	if (!BASS_Init(1,44100,BASS_DEVICE_8BITS,0,NULL))
		_cprintf("Init error %d\n", BASS_ErrorGetCode());

		// create a sound-enabled bell
	createObject("resources\\models\\handbell.3ds", cVector3d(-0.9,-0.4,0), cColorf(0.82, 0.7, 0.0, 1.0), BELL, 0);

	// create a sound-enabled teapot
	createObject("resources\\models\\teapot.3ds", cVector3d(0.1,-0.4,0), cColorf(0.5, 0.5, 0.5, 1.0), TEAPOT, 1);

	world->computeGlobalPositions(false); 

	return TRUE;
}


int CsoundsApp::Run() {

  MSG msg;

  // Position the console window so it looks nice...
  HWND con_wnd = GetConsoleWindow();
  RECT r;
  ::GetWindowRect(con_wnd,&r);

  // Move the real window up to the front
  ::SetWindowPos(g_main_dlg->m_hWnd,HWND_TOP,0,r.bottom,0,0,SWP_NOSIZE);  
  ::SetForegroundWindow(g_main_dlg->m_hWnd);

  // Loop forever looking for messages and rendering...
  while (1) {

    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE) {

      if (GetMessage(&msg, NULL, 0, 0)) {

        TranslateMessage(&msg);

        if (msg.message == WM_KEYDOWN) {

        }

        else if (msg.message == WM_KEYUP) {
        
        }

        DispatchMessage(&msg);

      }

      // Quit if GetMessage(...) fails
      else return TRUE;

    }    
    
    render_loop();

    // Sleep to be extra well-behaved, not necessary for
    // a game-like app that is happy to hog the CPU
    // Sleep(1);

  }
  
} // Run()


// Our main drawing loop...
int CsoundsApp::render_loop() {

	// see if there is a collision with a sound-enabled object (including
	// objects who have a sound-enabled parent mesh)
	cSoundMesh* sound_mesh = NULL;
	if (tool)
	{
		if (contactObject)
		{
			cGenericObject* parent = contactObject;
			contactObject = NULL;
			sound_mesh = dynamic_cast<cSoundMesh*>(parent);
			while ((parent) && (!sound_mesh))
			{
				parent = parent->getParent();
				sound_mesh = dynamic_cast<cSoundMesh*>(parent);
			}
		}

		// call play methods for sound-enabled objects
		for (unsigned int i=0; i<sound_meshes.size(); i++)
		{
			
			// send the current force to the currently contacted object, if any
			if (sound_meshes[i] == sound_mesh)
			    sound_mesh->getSound()->setContactForce(normalForce, tangentialForce);
			else
				sound_meshes[i]->getSound()->setContactForce(cVector3d(0,0,0), cVector3d(0,0,0));

			// if there is a new contact, reset and restart playing the sound
			if ((sound_meshes[i]->getSound()->getNormalForce().lengthsq() > 0.01) && 
				(sound_meshes[i]->getSound()->getPreviousForce().lengthsq() <= 0.01))
			{
				// reset for new contact
				sound_meshes[i]->getSound()->reset();

				// restart playing this stream
				if (!(BASS_ChannelPlay(sound_meshes[i]->getSound()->stream,TRUE)))
					_cprintf("error %d\n", BASS_ErrorGetCode());
			}
		}
	}

	// Just draw the scene...
	viewport->render();

    return 0;
}


// Called when the camera zoom slider changes
void CsoundsApp::zoom(int zoom_level) {

  //camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
  camera->setFieldViewAngle(((float)(zoom_level)));
  
}


// Called when the user moves the mouse in the main window
// while a button is held down
void CsoundsApp::scroll(CPoint p, int left_button) {

}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void CsoundsApp::select(CPoint p) {

}



/***
 
  Enable or disable haptics; called when the user clicks
  the enable/disable haptics button.  The "enable" parameter
  is one of :

  #define TOGGLE_HAPTICS_TOGGLE  -1
  #define TOGGLE_HAPTICS_DISABLE  0
  #define TOGGLE_HAPTICS_ENABLE   1

***/
void CsoundsApp::toggle_haptics(int enable) {

  
	if (enable == TOGGLE_HAPTICS_TOGGLE) {

		if (haptics_enabled) toggle_haptics(TOGGLE_HAPTICS_DISABLE);
		else toggle_haptics(TOGGLE_HAPTICS_ENABLE);

	}

	else if (enable == TOGGLE_HAPTICS_ENABLE) {
	  
		if (haptics_enabled) return;

		haptics_enabled = 1;

		// create a phantom tool with its graphical representation
		//
		// Use device zero, and use either the gstEffect or direct 
		// i/o communication mode, depending on the USE_PHANTOM_DIRECT_IO
		// constant
		if (tool == 0) {

		// tool = new cPhantom3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);
		tool = new cMeta3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);

		world->addChild(tool);
	    
		// set up the device
		tool->initialize();

		// set up a nice-looking workspace for the phantom so 
		// it fits nicely with our shape
		tool->setWorkspace(2.0,2.0,2.0);
	    
		// Rotate the tool so its axes align with our opengl-like axes
		tool->rotate(cVector3d(0,0,1),-90.0*M_PI/180.0);
		tool->rotate(cVector3d(1,0,0),-90.0*M_PI/180.0);
		tool->setRadius(0.05);
	      
		}
	    
		// I need to call this so the tool can update its internal
		// transformations before performing collision detection, etc.
		tool->computeGlobalPositions();

		// Open communication with the device
		tool->start();      

		// Enable forces
		tool->setForcesON();

		// Tell the proxy algorithm associated with this tool to enable its
		// "dynamic mode", which allows interaction with moving objects
	    
		// The dynamic proxy is in a pretty beta state, so we turn it off for now...
		// tool->getProxy()->enableDynamicProxy(1);

			// Use Zilles Friction algorithm (to get tangential forces)
			tool->getProxy()->setUseZillesFriction(true);
			tool->getProxy()->setUseMelderFriction(false);
	    
		// start haptic thread
		haptics_thread_running = 1;

		cSoundsHapticCallback* cb = new cSoundsHapticCallback();
		cb->m_tool = tool;
		((cPhantomDevice*)(tool->getDevice()))->setCallback(cb);
	       
	} // enabling

	else if (enable == TOGGLE_HAPTICS_DISABLE) {

		// Don't do anything if haptics are already off
		if (haptics_enabled == 0) return;

		// tell the haptic thread to quit
		haptics_enabled = 0;
	    
		// Stop the haptic device...
		tool->setForcesOFF();
		tool->stop();
	    
		// SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);    

	} // disabling

} // toggle_haptics()


