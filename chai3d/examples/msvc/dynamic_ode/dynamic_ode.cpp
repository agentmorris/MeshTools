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
    \date       03/2005
*/
//===========================================================================

#include "stdafx.h"
#include "dynamic_ode.h"
#include "dynamic_odeDlg.h"
#include <conio.h>
#include <process.h>
#include "celapsed.h"
#include "ode/ode.h"

// Maximum number of contact points handled by ODE
#define MAXCONTACTS 20

// Time step for ODE dynamics simulation
#define TIME_STEP 0.001//sec 300Hz

// Default stiffness and friction for the walls
#define K 20
#define MU_S 0.6
#define MU_D 0.6

// Size and position of the walls that form the room
// in which the box is pushed around
#define LEFT_WALL_OFFSET -12.0
#define RIGHT_WALL_OFFSET 12.0
#define FRONT_WALL_OFFSET 12.0
#define BACK_WALL_OFFSET -12.0
#define WALL_Z_OFFSET -8.0
#define BOTTOM_WALL_OFFSET -10.7
#define WALL_SIZE 20.0


// Turn off the annoying precision warning in msvc...
#ifdef _MSVC
#pragma warning(disable: 4244)
#endif


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

// #define USE_MM_TIMER_FOR_HAPTICS


/***

  CHAI supports two modes of communication with the Phantom... via
  the gstEffect Ghost class, and via Ghost's direct i/o model.  This
  #define allows you to switch between them.  Each may perform better
  in specific situations.

***/

#define USE_PHANTOM_DIRECT_IO 1


#define ALLOCATE_SCOPED_GLOBALS
#include "dynamic_ode_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

// Default initial position for the object that will appear in the main window
cVector3d g_initial_object_pos(0, 1, 0);

cMaterial material;
  
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(Cdynamic_odeApp, CWinApp)
  //{{AFX_MSG_MAP(Cdynamic_odeApp)
  //}}AFX_MSG
  ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

Cdynamic_odeApp theApp;

Cdynamic_odeApp::Cdynamic_odeApp() {
  selected_object = 0;
  haptics_enabled = 0;
  first_device_enabled = 0;
  second_device_enabled = 0;
  tool1 = 0;
  tool2 = 0;
  ready = true;
  tool1_ready = 0;
  tool2_ready = 0;

  AllocConsole();

  HWND con_wnd = GetConsoleWindow();

  // We want the console to pop up over any non-chai windows, although
  // we'll put the main dialog window above it.
  ::SetForegroundWindow(con_wnd);
  
  SetWindowPos(con_wnd,HWND_TOP,0,0,0,0,SWP_NOSIZE);

  g_main_app = this;
}


void Cdynamic_odeApp::uninitialize() {

  toggle_haptics(TOGGLE_HAPTICS_DISABLE);
  delete world;
  delete viewport;
  
}


// callback function for ODE
void Cdynamic_odeApp::nearCallback (void *data, dGeomID o1, dGeomID o2) {
  Cdynamic_odeApp *pFrame = (Cdynamic_odeApp*)data;

  int i;
  dBodyID b1 = dGeomGetBody(o1);
  dBodyID b2 = dGeomGetBody(o2);

  dContact contact[MAXCONTACTS];
  for (i=0; i<MAXCONTACTS; i++) {
    contact[i].surface.mode = dContactApprox1 | dContactBounce;
    contact[i].surface.mu = 0.2;
    contact[i].surface.bounce = 0.2;
    contact[i].surface.bounce_vel = 0.2;
  }
  if (int numc = dCollide (o1,o2,MAXCONTACTS,&contact[0].geom,sizeof(dContact))) {
    for (i=0; i<numc; i++) {
      dJointID c = dJointCreateContact (pFrame->ode_world,pFrame->ode_contact_group,contact+i);
      dJointAttach (c,b1,b2);
    }
  }
}


BOOL Cdynamic_odeApp::InitInstance() {

  AfxEnableControlContainer();

#ifdef _AFXDLL
  Enable3dControls();     // Call this when using MFC in a shared DLL
#else
  Enable3dControlsStatic(); // Call this when linking to MFC statically
#endif

  memset(keys_to_handle,0,sizeof(keys_to_handle));
  
  g_main_dlg = new Cdynamic_odeDlg;
  m_pMainWnd = g_main_dlg;

  g_main_dlg->Create(IDD_dynamic_ode_DIALOG,NULL);
    
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
  int result = camera->set(
              cVector3d(5,0,0),           // position of camera
              cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
              cVector3d(0.0, 0.0, 1.0)  // orientation of camera (standing up)
              );

  if (result == 0) {
    _cprintf("Could not create camera...\n");
  }

  // Create a display for graphic rendering
  viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);

  // Initialize the ODE context
  ode_world = dWorldCreate();
  dWorldSetGravity (ode_world,0,0,-5);
  ode_step = TIME_STEP;
  ode_space = dSimpleSpaceCreate(0);
  ode_contact_group = dJointGroupCreate(0);
  ode_collision_callback    = Cdynamic_odeApp::nearCallback;
  ode_collision_callback_data  = this;

  // Create a light source
  light = new cLight(world);
  
  // camera->addChild(light);
  world->addChild(light);
  light->setEnabled(true);

  // A purely directional light, pointing into the monitor
  // and a little to the right... note that to accomplish
  // that, I "place" the light off to the left and behind the
  // origin (positive z relative to the origin).
  light->setDirectionalLight(false);
  light->setPos(cVector3d(4,1,1));

  // create cube
  object = new cODEMesh(world, ode_world, ode_space);
  world->addChild(object);

  double size = 1.0;
  createCube(object,size);

  // initialize ODE parameters
  // Move the object over some so the Phantom will not initially be
  // inside the object.
  object->initDynamic(DYNAMIC_OBJECT, g_initial_object_pos.x , g_initial_object_pos.y, g_initial_object_pos.z);
  object->setMass(0.2);

  // Give a color to each vertex
  for (unsigned int i=0; i<object->getNumVertices(); i++) 
  {
    cVertex* nextVertex = object->getVertex(i);
    cColorb color;
    color.set(
      GLuint(0xff*(size + nextVertex->getPos().x ) / (2.0 * size)),
      GLuint(0xff*(size + nextVertex->getPos().y ) / (2.0 * size)),
      GLuint(0xff* nextVertex->getPos().z / 2*size)
      );

    nextVertex->setColor(color);
  }

  // Initialize the box   
  object->computeBoundaryBox(true);
  object->computeAllNormals();
  object->setShowBox(true);
  object->setShowFrame(true,true);
  object->computeGlobalPositions(1);
  object->createAABBCollisionDetector(true,true);

  // Give him some material properties...
  //object->m_material.m_ambient.set( 0.4, 0.2, 0.2, 1.0 );
  //object->m_material.m_diffuse.set( 0.8, 0.6, 0.6, 1.0 );
  //object->m_material.m_specular.set( 0.9, 0.9, 0.9, 1.0 );
  //object->m_material.setShininess(100);
  object->setMaterial(material);
  
  // Create a room, bounded by cubes, in which we can push around our little box
  bottom_wall = create_wall(0,0,BOTTOM_WALL_OFFSET,1);
  left_wall = create_wall(0,LEFT_WALL_OFFSET,WALL_Z_OFFSET,0);
  right_wall = create_wall(0,RIGHT_WALL_OFFSET,WALL_Z_OFFSET,0);
  back_wall = create_wall(BACK_WALL_OFFSET,0,WALL_Z_OFFSET,0);
  front_wall = create_wall(FRONT_WALL_OFFSET,0,0,0);

  world->computeGlobalPositions(false);

  // This will set rendering and haptic options as they're defined in
  // the GUI...
  update_options_from_gui();

  return TRUE;
}


cODEMesh* Cdynamic_odeApp::create_wall(double a_x, double a_y, double a_z, bool a_ground) { 
  cODEMesh* new_mesh = new cODEMesh(world, ode_world, ode_space); 
  createCube(new_mesh, WALL_SIZE, a_ground);
  new_mesh->m_material.m_ambient.set( 0.7, 0.7, 0.7, 1.0 );
  new_mesh->m_material.m_diffuse.set( 0.7, 0.7, 0.7, 1.0 );
  new_mesh->m_material.m_specular.set( 0.7, 0.7, 0.7, 1.0 );
  new_mesh->m_material.setShininess(100);
  new_mesh->m_material.setStiffness(K);
  new_mesh->setFriction(MU_S, MU_D, 1);
  new_mesh->initDynamic(STATIC_OBJECT,a_x, a_y, a_z);
  world->addChild(new_mesh);
  new_mesh->computeAllNormals();

  // If this is the ground, actually make two boxes -- one used by ODE and one for
  // haptics, with the one used by ODE slightly above.  This will make the box "float"
  // slightly above the ground, avoiding the problem of when the proxy gets stuck 
  // between the falling box and the ground and has to penetrate one or the other
  if (a_ground)
  {
    new_mesh->setHapticEnabled(false, true);
    cMesh* offset_bottom = new cMesh(world);
    createCube(offset_bottom, WALL_SIZE, a_ground);
    offset_bottom->m_material.m_ambient.set( 0.7, 0.7, 0.7, 1.0 );
    offset_bottom->m_material.m_diffuse.set( 0.7, 0.7, 0.7, 1.0 );
    offset_bottom->m_material.m_specular.set( 0.7, 0.7, 0.7, 1.0 );
    offset_bottom->m_material.setShininess(100);
    offset_bottom->m_material.setStiffness(K);
    offset_bottom->setFriction(MU_S, MU_D, 1);
    offset_bottom->translate(a_x, a_y, a_z-0.1);
    world->addChild(offset_bottom);
    offset_bottom->computeAllNormals();
  }
  
  return new_mesh;
}


int Cdynamic_odeApp::Run() {

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
        int message_processed = PreTranslateMessage(&msg);
        if (message_processed == 0) TranslateMessage(&msg);

        // Record any keystrokes that need to be handled by the rendering
        // or haptics loops...
        if (msg.message == WM_KEYDOWN) {
          keys_to_handle[msg.wParam] = 1;    
        }
        else if (msg.message == WM_KEYUP) {        
          keys_to_handle[msg.wParam] = 0;
        }

        if (message_processed == 0) DispatchMessage(&msg);
      }

      // Quit if GetMessage(...) fails
      else return TRUE;
    }    
    
    render_loop();

    // We can sleep (yield) here to be extra well-behaved, not necessary
    // for a game-like app that is happy to eat CPU
    // Sleep(1);

  }
  
} // Run()


// Our main drawing loop...
int Cdynamic_odeApp::render_loop() {

  // If the viewport is rendering in stereo, it renders a full pair here...
  viewport->render();

  return 0;
}


void Cdynamic_odeApp::copy_rendering_options_to_gui() {

  g_main_dlg->m_usecolors_check = (object->getColorsEnabled()?BST_INDETERMINATE:BST_UNCHECKED);
  g_main_dlg->m_material_check = (object->getMaterialEnabled()?BST_INDETERMINATE:BST_UNCHECKED);
  g_main_dlg->m_transparency_check = (object->getTransparencyEnabled()?BST_INDETERMINATE:BST_UNCHECKED);
  g_main_dlg->UpdateData(FALSE);
  
}

void Cdynamic_odeApp::update_options_from_gui(int preserve_loaded_properties) {

  // The second parameter in each case transfers options to
  // any children the object has...
  object->setWireMode(g_main_dlg->m_usewireframe_check, true);
  object->setShowFrame(g_main_dlg->m_showframe_check, true);
  object->showNormals(g_main_dlg->m_shownormals_check, true);
  object->setShowBox(g_main_dlg->m_showbox_check, true);
  object->useCulling(g_main_dlg->m_culling_check, true);
  object->useTexture(g_main_dlg->m_usetexture_check, true);

  if (preserve_loaded_properties == 0) {
    if (g_main_dlg->m_usecolors_check!=BST_INDETERMINATE)
      object->useColors(g_main_dlg->m_usecolors_check, true);
    if (g_main_dlg->m_material_check!=BST_INDETERMINATE)
      object->useMaterial(g_main_dlg->m_material_check, true);
    if (g_main_dlg->m_transparency_check!=BST_INDETERMINATE)
      object->enableTransparency(g_main_dlg->m_transparency_check, true);
  }
  
  object->setStiffness(g_main_dlg->m_stiffness,1);
  object->setFriction(g_main_dlg->m_static_friction,g_main_dlg->m_dynamic_friction,1);

  camera->setStereoEyeSeparation(g_main_dlg->m_separation);
  camera->setStereoFocalLength(g_main_dlg->m_focus);
}


// Called when the camera zoom slider changes
void Cdynamic_odeApp::zoom(int zoom_level) {

  //camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
  camera->setFieldViewAngle(((float)(zoom_level)));
  
}


// Called when the user moves the mouse in the main window
// while a button is held down
void Cdynamic_odeApp::scroll(CPoint p, int left_button) {

  if (selected_object == 0) return;

  // If the user hasn't clicked on any objects, we don't
  // have to move or rotate anyone
  cGenericObject* object_to_move = selected_object;

  // We want to move the _parent_ mesh around, so let's
  // walk up the tree, looking for the highest-level object
  // that's still a cMesh...
  //
  // The reason we do this is that maybe I've loaded, say,
  // a model of a person from a file, that might contain
  // separate meshes for each arm.  If I click on that mesh,
  // CHAI will indeed find that mesh, but really I don't
  // want the user to pull the model's arm off, I want to move
  // the larger model around.
  cGenericObject* tmp = object_to_move;

  // Walk up the tree...
  while(1) {

    tmp = tmp->getParent();
    
    // If there was no parent, we hit the highest-level mesh
    if (tmp == 0) break;

    // If the parent was not a cMesh, we hit the highest-level mesh
    cMesh* mesh = dynamic_cast<cMesh*>(tmp);
    if (mesh == 0) break;

    // Okay, this is a mesh...
    object_to_move = tmp;

  }

  if (left_button) {
    cVector3d axis1(-1,0,0);
    object_to_move->rotate(axis1,-1.0*(float)p.y / 50.0);
  
    cVector3d axis2(0,1,0);
    object_to_move->rotate(axis2,(float)p.x / 50.0);
  }
  else {
    object_to_move->translate((float)p.x / 100.0, 0, 0);
    object_to_move->translate(0, -1.0*(float)p.y / 100.0, 0);
  }

  object->computeGlobalPositions(1);
}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void Cdynamic_odeApp::select(CPoint p) {

  if (viewport->select(p.x, p.y, true)) {
    selected_object = viewport->getLastSelectedObject();         
  }
  else {    
    selected_object = 0;
  }
}


// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied Cdynamic_odeApp

// A single iteration through the loop...
void dynamic_ode_haptic_iteration(void* param) {
  Cdynamic_odeApp* app = (Cdynamic_odeApp*)(param);

  // set it up so that wherever your tool is in the real world when you enable forces, it will be at (1,0,3) in the
  // virtual world, so that it is in side the virtual "room" and not penetrating anything
  if (app->first_device_enabled && app->tool1) 
  {
    // update the tool's pose and compute and apply forces
    app->tool1->updatePose();
    
    if (!app->tool1_ready)
    {
      // Turn off haptic collision detection for the objects in the world 
      for (unsigned int i=0; i<app->world->getNumChildren(); i++)
        app->world->getChild(i)->setHapticEnabled(0, 1);

      if ( (app->tool1->m_deviceGlobalPos.x > BACK_WALL_OFFSET+WALL_SIZE/2+0.2) &&  
           (app->tool1->m_deviceGlobalPos.x < FRONT_WALL_OFFSET-WALL_SIZE/2-0.2) && 
           (app->tool1->m_deviceGlobalPos.y > LEFT_WALL_OFFSET+WALL_SIZE/2+0.2) &&
           (app->tool1->m_deviceGlobalPos.y < RIGHT_WALL_OFFSET-WALL_SIZE/2-0.2) &&
           (app->tool1->m_deviceGlobalPos.z > BOTTOM_WALL_OFFSET+WALL_SIZE/2+0.2)) 
        app->tool1_ready = 1;
    }
    else 
    {
      // Turn on haptic collision detection for the objects in the world
      for (unsigned int i=0; i<app->world->getNumChildren(); i++)
        if (app->world->getChild(i) != app->bottom_wall)
          app->world->getChild(i)->setHapticEnabled(1, 1);
    }

    app->tool1->computeForces();
    if (app->tool1_ready)
      app->tool1->applyForces();
  }

  // if two devices are active, update the second one also...
  if (app->second_device_enabled && app->tool2) 
  {
    app->tool2->updatePose();

    if (!app->tool2_ready)
    {
      // Turn off haptic collision detection for the objects in the world 
      for (unsigned int i=0; i<app->world->getNumChildren(); i++)
        app->world->getChild(i)->setHapticEnabled(0, 1);

      if ( (app->tool2->m_deviceGlobalPos.x > BACK_WALL_OFFSET+WALL_SIZE/2+0.2) &&  
           (app->tool2->m_deviceGlobalPos.x < FRONT_WALL_OFFSET-WALL_SIZE/2-0.2) && 
           (app->tool2->m_deviceGlobalPos.y > LEFT_WALL_OFFSET+WALL_SIZE/2+0.2) &&
           (app->tool2->m_deviceGlobalPos.y < RIGHT_WALL_OFFSET-WALL_SIZE/2-0.2) &&
           (app->tool2->m_deviceGlobalPos.z > BOTTOM_WALL_OFFSET+WALL_SIZE/2+0.2)) 
        app->tool2_ready = 1;
    }
    else 
    {
      // Turn on haptic collision detection for the objects in the world
      for (unsigned int i=0; i<app->world->getNumChildren(); i++)
        if (app->world->getChild(i) != app->bottom_wall)
          app->world->getChild(i)->setHapticEnabled(1, 1);
    }

    app->tool2->computeForces();
    if (app->tool2_ready)
      app->tool2->applyForces();
  }

  app->object->m_historyValid = false;

  // code to get forces from CHAI and apply them to the appropriate meshes, calling ODE functions to calculate the dynamics

  if (app->ode_clock->on()) 
  {  
    if ( (app->ode_clock->timeoutOccurred()) && app->ready) 
    {
      app->ready = false;

      if (app->tool1 && app->first_device_enabled)
      {
        cProxyPointForceAlgo *proxy   = dynamic_cast<cProxyPointForceAlgo*>(app->tool1->getProxy());

        if (proxy && proxy->getContactObject() != NULL) 
        {
          float x =  proxy->getContactPoint().x;
          float y =  proxy->getContactPoint().y;
          float z =  proxy->getContactPoint().z;
        
          float fx = -app->tool1->m_lastComputedGlobalForce.x ;
          float fy = -app->tool1->m_lastComputedGlobalForce.y ;
          float fz = -app->tool1->m_lastComputedGlobalForce.z ;

          cGenericObject* cur_object = proxy->getContactObject();
          bool found = false;
          cODEMesh* mesh = 0;
          while (cur_object && !found)
          {
            mesh = dynamic_cast<cODEMesh*>(cur_object);
            if (mesh) found = true;
            cur_object = cur_object->getParent();
          }
          
          if ((found) && (mesh) && (mesh->m_objType == DYNAMIC_OBJECT)) 
            dBodyAddForceAtPos(mesh->m_odeBody,fx,fy,fz,x,y,z);
        }
      }

      if (app->tool2 && app->second_device_enabled) 
      {
        cProxyPointForceAlgo *proxy   = dynamic_cast<cProxyPointForceAlgo*>(app->tool2->getProxy());

        if (proxy && proxy->getContactObject() != NULL) 
        {
          float x =  proxy->getContactPoint().x;
          float y =  proxy->getContactPoint().y;
          float z =  proxy->getContactPoint().z;
        
          float fx = -app->tool2->m_lastComputedGlobalForce.x ;
          float fy = -app->tool2->m_lastComputedGlobalForce.y ;
          float fz = -app->tool2->m_lastComputedGlobalForce.z ;

          cGenericObject* cur_object = proxy->getContactObject();
          bool found = false;
          cODEMesh* mesh = 0;
          while (cur_object && !found)
          {
            mesh = dynamic_cast<cODEMesh*>(cur_object);
            if (mesh) found = true;
            cur_object = cur_object->getParent();
          }

          if ((found) && (mesh) && (mesh->m_objType == DYNAMIC_OBJECT)) 
            dBodyAddForceAtPos(mesh->m_odeBody,fx,fy,fz,x,y,z); 
        }
      }     
   
      dSpaceCollide (app->ode_space,app->ode_collision_callback_data,app->ode_collision_callback);
      dWorldStep(app->ode_world,app->ode_step);
      dJointGroupEmpty(app->ode_contact_group);

      app->object->updateDynamicPosition();
      app->object->m_historyValid = true;
      if (app->tool1 && app->first_device_enabled)
        app->tool1->computeGlobalPositions(1);

      if (app->tool2 && app->second_device_enabled)
        app->tool2->computeGlobalPositions(1);

      app->ode_clock->initialize();
      app->ready = true;
    }
  }
}
   

// This loop is used only in the threaded version of this
// application... all it does is call the main haptic
// iteration loop, which is called directly from a timer
// callback if USE_MM_TIMER_FOR_HAPTICS is defined
DWORD dynamic_ode_haptic_loop(void* param) {
  Cdynamic_odeApp* app = (Cdynamic_odeApp*)(param);
  while(app->haptics_enabled) {
    dynamic_ode_haptic_iteration(param);
  }
  app->haptics_thread_running = 0;
  return 0;
}


// The viewport must be reinitialized when stereo is toggled
void Cdynamic_odeApp::reinitialize_viewport(int stereo_enabled) {
  if (viewport) delete viewport;
  viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, stereo_enabled);
  _cprintf("Stereo rendering is %s\n",viewport->getStereoOn()?"enabled":"disabled");
}


/***
 
  Enable or disable haptics; called when the user clicks
  the enable/disable haptics button.  The "enable" parameter
  is one of :

  #define TOGGLE_HAPTICS_TOGGLE  -1
  #define TOGGLE_HAPTICS_DISABLE  0
  #define TOGGLE_HAPTICS_ENABLE   1

***/
void Cdynamic_odeApp::toggle_haptics(int enable) {

  if (enable == TOGGLE_HAPTICS_TOGGLE) {

    if (haptics_enabled) toggle_haptics(TOGGLE_HAPTICS_DISABLE);
    else toggle_haptics(TOGGLE_HAPTICS_ENABLE);

  }

  else if (enable == TOGGLE_HAPTICS_ENABLE) {
  
    if (haptics_enabled) return;

    tool1_ready = 0;
    haptics_enabled = 1;

    // create a phantom tool with its graphical representation
    //
    // Use device zero, and use either the gstEffect or direct 
    // i/o communication mode, depending on the USE_PHANTOM_DIRECT_IO
    // constant
    if (tool1 == 0) 
    {
      // create a haptic tool and set up its workspace
      tool1 = new cMeta3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);
      tool1->setWorkspace(3.0,3.0,3.0);
  
      world->addChild(tool1);        
      tool1->setRadius(0.05);
      tool1->computeGlobalCurrentObjectOnly(true);

      // Replace the proxy with our custom ODE proxy
      cProxyPointForceAlgo* old_proxy = (cProxyPointForceAlgo*)(tool1->m_pointForceAlgos[0]);
      tool1->m_pointForceAlgos[0] = new cODEProxy(old_proxy);
      delete old_proxy;

    }
  
    // set up the device
    tool1->initialize();

    // open communication to the device
    tool1->start();
    
    // update initial orientation and position of device
    tool1->updatePose();

    // tell the tool to show his coordinate frame so you
    // can see tool rotation
    tool1->visualizeFrames(true);

    // I need to call this so the tool can update its internal
    // transformations before performing collision detection, etc.
    tool1->computeGlobalPositions(true);
    
    tool1->setForcesON();
      
    ode_clock = new cPrecisionClock();
    ode_clock->setTimeoutPeriod(TIME_STEP * 1e+6);
    ode_clock->start(); 

#ifdef USE_MM_TIMER_FOR_HAPTICS

    // start the mm timer to run the haptic loop
    timer.set(0,object_loader_haptic_iteration,this);

#else

    // start haptic thread
    haptics_thread_running = 1;

    first_device_enabled = 1;
    Sleep(100);
    DWORD thread_id;
    ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(dynamic_ode_haptic_loop), this, 0, &thread_id);

    // Boost thread and process priority
    ::SetThreadPriority(&thread_id, THREAD_PRIORITY_ABOVE_NORMAL);
    //::SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);

#endif
       
  } // enabling

  else if (enable == TOGGLE_HAPTICS_DISABLE) {

    // Don't do anything if haptics are already off
    if (haptics_enabled == 0) return;

    // tell the haptic thread to quit
    haptics_enabled = 0;

    first_device_enabled = 0;

#ifdef USE_MM_TIMER_FOR_HAPTICS

    timer.stop();

#else

    // wait for the haptic thread to quit
    while(haptics_thread_running) Sleep(1);

#endif
    
    // Stop the haptic device...
    tool1->setForcesOFF();
    tool1->stop();

    // If a second device is also running, stop it too
    second_device_enabled = 0;
    if (tool2) {
      tool2->setForcesOFF();
      tool2->stop();
    }       

  } // disabling

} // toggle_haptics()


void Cdynamic_odeApp::toggle_second_device(int enable) {
  if (enable == TOGGLE_HAPTICS_TOGGLE && haptics_enabled) {

    if (second_device_enabled) toggle_second_device(TOGGLE_HAPTICS_DISABLE);
    else toggle_second_device(TOGGLE_HAPTICS_ENABLE);

  }

  else if (enable == TOGGLE_HAPTICS_ENABLE) {
  
    if (second_device_enabled) return;

    // temporarily disable the first device, because the scheduler will have to be restarted
    first_device_enabled = 0;

    // create a phantom tool with its graphical representation
    //
    // Use device 1 this time, and use either the gstEffect or direct 
    // i/o communication mode, depending on the USE_PHANTOM_DIRECT_IO
    // constant
    if (tool2 == 0) 
    { 
      
      // create a new tool and set its workspace
      tool2 = new cMeta3dofPointer(world, 1, USE_PHANTOM_DIRECT_IO);
      tool2->setWorkspace(3.0,3.0,3.0);

      // turn on ode proxy algorithm
      world->addChild(tool2);       
      tool2->setRadius(0.05);
      tool2->computeGlobalCurrentObjectOnly(true);

      // Replace the proxy with our custom ODE proxy
      cProxyPointForceAlgo* old_proxy = (cProxyPointForceAlgo*)(tool2->m_pointForceAlgos[0]);
      tool2->m_pointForceAlgos[0] = new cODEProxy(old_proxy);
      delete old_proxy;
    }

    // set up the device
    tool2->initialize();

    // open communication to the device
    tool2->start();

    // update initial orientation and position of device
    tool2->updatePose();

    // tell the tool to show his coordinate frame so you
    // can see tool rotation
    tool2->visualizeFrames(true);

    // I need to call this so the tool can update its internal
    // transformations before performing collision detection, etc.
    tool2->computeGlobalPositions();

    tool2->setForcesON(); 

    Sleep(100);

    // The devices are enabled, but are not "ready" for forces until 
    // the user has moved them into the relevant workspace
    tool1_ready = 0;
    tool2_ready = 0;

    first_device_enabled = 1;
    second_device_enabled = 1;
  
  } // enabling

  else if (enable == TOGGLE_HAPTICS_DISABLE) {

    // Don't do anything if haptics are already off
    if (second_device_enabled == 0) return;

    // tell the haptic thread to quit
    second_device_enabled = 0;
    
    // Stop the haptic device...
    tool2->setForcesOFF();
    tool2->stop();        
  } // disabling
}


// A global function for sticking a cube in the given mesh
// 
// Manually creates the 12 triangles (two per face) required to
// model a cube
void createCube(cMesh *mesh, float edge, int include_top) {

  // I define the cube's "radius" to be half the edge size
  float radius = edge / 2.0;
  int n;
  int cur_index = 0;
  int start_index = 0;

  // +x face
  mesh->newVertex( radius,  radius, -radius);
  mesh->newVertex( radius,  radius,  radius);
  mesh->newVertex( radius, -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex( radius, -radius, -radius);
  mesh->newVertex( radius,  radius,  radius);
  mesh->newVertex( radius, -radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().y + radius) / (2.0 * radius),
      (curVertex->getPos().z + radius) / (2.0 * radius)
      );
    curVertex->setNormal(1,0,0);
  }
  
  start_index += 6;

  // -x face
  mesh->newVertex(-radius,  radius,  radius);
  mesh->newVertex(-radius,  radius, -radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(-radius,  radius,  radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(-radius, -radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().y + radius) / (2.0 * radius),
      (curVertex->getPos().z + radius) / (2.0 * radius)
      );
    curVertex->setNormal(-1,0,0);
  }
  
  start_index += 6;

  // +y face
  mesh->newVertex(radius,  radius,  radius);
  mesh->newVertex(radius,  radius, -radius);
  mesh->newVertex(-radius, radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(radius,  radius,  radius);
  mesh->newVertex(-radius, radius, -radius);
  mesh->newVertex(-radius, radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().x + radius) / (2.0 * radius),
      (curVertex->getPos().z + radius) / (2.0 * radius)
      );
    curVertex->setNormal(0,1,0);
  }
  
  start_index += 6;

  // -y face
  mesh->newVertex(radius,  -radius,  radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(radius,  -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(radius,  -radius,  radius);
  mesh->newVertex(-radius, -radius,  radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().x + radius) / (2.0 * radius),
      (curVertex->getPos().z + radius) / (2.0 * radius)
      );
    curVertex->setNormal(0,-1,0);
  }
  
  start_index += 6;
  
  // -z face
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(radius,   radius, -radius);
  mesh->newVertex(radius,  -radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  mesh->newVertex( radius,  radius, -radius);
  mesh->newVertex(-radius, -radius, -radius);
  mesh->newVertex(-radius,  radius, -radius);
  mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
  cur_index+=3;

  for(n=start_index; n<cur_index; n++) {
    cVertex* curVertex = mesh->getVertex(n);
    curVertex->setTexCoord(
      (curVertex->getPos().x + radius) / (2.0 * radius),
      (curVertex->getPos().y + radius) / (2.0 * radius)
      );
    curVertex->setNormal(0,0,-1);
  }
  
  start_index += 6;

  if (include_top) {
  
    // +z face
    mesh->newVertex(-radius, -radius, radius);
    mesh->newVertex(radius,  -radius, radius);
    mesh->newVertex(radius,  radius,  radius);
    mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    mesh->newVertex(-radius, -radius, radius);
    mesh->newVertex( radius,  radius, radius);
    mesh->newVertex(-radius,  radius, radius);
    mesh->newTriangle(cur_index,cur_index+1,cur_index+2);
    cur_index+=3;

    for(n=start_index; n<cur_index; n++) {
      cVertex* curVertex = mesh->getVertex(n);
      curVertex->setTexCoord(
        (curVertex->getPos().x + radius) / (2.0 * radius),
        (curVertex->getPos().y + radius) / (2.0 * radius)
        );
      curVertex->setNormal(0,0,1);
  }
  
    start_index += 6;
  }

  // Give a color to each vertex
  for (unsigned int i=0; i<mesh->getNumVertices(); i++) {

    cVertex* nextVertex = mesh->getVertex(i);

    cColorb color;
    color.set(
      GLuint(0xff*(edge + nextVertex->getPos().x ) / (2.0 * edge)),
      GLuint(0xff*(edge + nextVertex->getPos().y ) / (2.0 * edge)),
      GLuint(0xff* nextVertex->getPos().z / 2*edge)
      );

    nextVertex->setColor(color);
  }

  // Set object settings.  The parameters tell the object
  // to apply this alpha level to his textures and to his
  // children (of course he has neither right now).
  // object->setTransparencyLevel(0.5, true, true);

  // Give him some material properties...
  cMaterial material;

  material.m_ambient.set( 0.6, 0.3, 0.3, 1.0 );
  material.m_diffuse.set( 0.8, 0.6, 0.6, 1.0 );
  material.m_specular.set( 0.9, 0.9, 0.9, 1.0 );
  material.setShininess(100);
  mesh->m_material = material;
  
}
