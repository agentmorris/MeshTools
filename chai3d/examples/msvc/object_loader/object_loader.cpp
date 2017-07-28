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

#include "stdafx.h"
#include "object_loader.h"
#include "object_loaderDlg.h"
#include <conio.h>
#include <process.h>
#include "CPrecisionClock.h"
#include "CVBOMesh.h"
#include "CImageLoader.h"

// If this is defined, we do our rendering with vertex buffers, instead
// of plain-old GL immediate mode.  This is accomplished by simply creating
// cVBOMesh's instead of cMesh's
#define USE_VERTEX_BUFFER_MESHES

#ifndef M_PI
#define M_PI 3.1415926535898
#endif

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
#include "object_loader_globals.h"
#undef ALLOCATE_SCOPED_GLOBALS

// Default initial position for the object that will appear in the main window
cVector3d g_initial_object_pos(1.5,0,-1.2);

// The translational and rotational velocities (gl units/s and radians/s)
// at which the object is animated when the 'toggle animation' button gets
// clicked
#define INITIAL_OBJECT_X_VELOCITY -0.5
#define OBJECT_R_VELOCITY (M_PI / 4.0)

// This is the point where the object will "turn around"
// when he's being animated...
#define MAXIMUM_ANIMATION_XVAL 1.0

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Turn off annoying compiler warnings
#pragma warning(disable: 4305)
#pragma warning(disable: 4800)

BEGIN_MESSAGE_MAP(Cobject_loaderApp, CWinApp)
	//{{AFX_MSG_MAP(Cobject_loaderApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// No one really knows why GetConsoleWindow() is not
// properly exported by the MS include files.
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow (); 

Cobject_loaderApp theApp;

Cobject_loaderApp::Cobject_loaderApp() {

  disable_haptics_temporarily = false;
  disable_haptics_temporarily_received = false;

  m_last_animation_time = -1.0;

  object = 0;

  shader = 0;

  moving_object = 0;

  selected_object = 0;

  haptics_enabled = 0;

  tool = 0;

  AllocConsole();

  HWND con_wnd = GetConsoleWindow();

  // We want the console to pop up over any non-chai windows, although
  // we'll put the main dialog window above it.
  ::SetForegroundWindow(con_wnd);
  
  SetWindowPos(con_wnd,HWND_TOP,0,0,0,0,SWP_NOSIZE);

  g_main_app = this;
}


void Cobject_loaderApp::uninitialize() {

  toggle_haptics(TOGGLE_HAPTICS_DISABLE);
  delete world;
  delete viewport;

}


BOOL Cobject_loaderApp::InitInstance() {

  AfxEnableControlContainer();

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  memset(keys_to_handle,0,sizeof(keys_to_handle));
	
	g_main_dlg = new Cobject_loaderDlg;
  m_pMainWnd = g_main_dlg;

  g_main_dlg->Create(IDD_object_loader_DIALOG,NULL);
    
  // Now we should have a display context to work with...

  world = new cWorld();

  // set background color
  world->setBackgroundColor(0.0f,0.0f,0.0f);

  // Create a camera
  camera = new cCamera(world);

  // This is not strictly necessary; it enables rendering the camera
  // (and children of the camera) as a visible object...
  world->addChild(camera);
  
  // set camera position and orientation
  // 
  // We choose to put it out on the positive z axis, so things appear
  // the way OpenGL users expect them to appear, with z going in and
  // out of the plane of the screen.
  int result = camera->set(
              cVector3d(0,0,4),           // position of camera
              cVector3d(0.0, 0.0, 0.0),   // camera looking at origin
              cVector3d(0.0, 1.0, 0.0)    // orientation of camera (standing up)
              );

  if (result == 0) {
    _cprintf("Could not create camera...\n");
  }


  // Create a display for graphic rendering
  viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);

  // Create a light source
  light = new cLight(world);
  
  // This would cause the light to follow the camera...
  // camera->addChild(light);

  // This is not strictly necessary; it enables rendering the light
  // (and children of the light) as a visible object
  world->addChild(light);
  light->setEnabled(true);

  // A purely directional light, pointing into the monitor
  // and a little to the right... note that to accomplish
  // that, I "place" the light off to the left and behind the
  // origin (positive z relative to the origin).
  light->setDirectionalLight(true);
  light->setPos(cVector3d(-1,0,1));

  // Create a mesh - we will build a pyramid manually, and later let the
  // user load 3d models

#ifdef USE_VERTEX_BUFFER_MESHES
  // Create a mesh that uses vertex buffers
  object = new cVBOMesh(world);
#else
  // Create a mesh that uses plain-vanilla GL rendering
  object = new cMesh(world);
#endif

  world->addChild(object);

  double size = 1.0;

  // Create a nice little cube
  createCube(object,size);

  // Move the object over some so the Phantom will not initially be
  // inside the object.
  object->translate(g_initial_object_pos);

  object->computeGlobalPositions(true);
  object->computeBoundaryBox(true);

  // Note that I don't bother to build a fancy collision detector
  // for this cube, since the brute force collision detector -
  // which is the default - works fine for using the haptic proxy
  // on a simple shape with only a few triangles.
  //
  // I will create a fancy collision detector later when I load
  // a model from a file.

  // These two calls are useful for debugging, but not
  // really important...

  // Tells the object to compute a boundary box around
  // himself and all his children.  We do this here just so
  // he can render his bounding box for your viewing
  // pleasure.
  object->computeBoundaryBox(true);

  // Set the size of the coordinate frame that we're
  // going to gratuitously render
  object->setFrameSize(1.0, 1.0, false);

  // Compute normals for each triangle; generally you would want
  // to do this only if you think your model doesn't already
  // have normals in it...
  //object->computeAllNormals();
  
  world->computeGlobalPositions(false);

  // Create a text label
  label = new cLabelPanel(world);
  label->addLabel("This is the default object...");
  label->getFont()->setPointSize(14.0f);
  label->getFont()->setFontFace("arial");
  label->setBorders(15,15,-1,-1,-1);
  label->setDisplayRect(true);
  label->m_rectColor = cColorf(0.7,0.2,0.2);
  label->setDisplayEdges(true);
  label->m_edgeColor = cColorf(0.5,0.5,0.5);
  label->m_textColor = cColorf(1,1,1);    
  label->setPos(cVector3d(50,50,0));
  label->m_useLighting = false;
  label->setAlignment(ALIGN_LEFT,VALIGN_BOTTOM);

  camera->m_front_2Dscene.addChild(label);


  // This will set rendering and haptic options as they're defined in
  // the GUI...
  update_options_from_gui();

  return TRUE;
}


int Cobject_loaderApp::Run() {

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
int Cobject_loaderApp::render_loop() {

  // Perform animation if the haptics thread isn't running but
  // we're supposed to be moving the object
  if (haptics_thread_running == 0 && moving_object) {

    animate();

  }

  // If the viewport is rendering in stereo, it renders a full pair here...
  viewport->render();

  // If you wanted to control the stereo rendering, for example if you
  // couldn't suspend your computation for two rendering passes, you
  // could do:

  /*
  if (viewport->getStereoOn() == 0) {
  
    // Just draw the scene...
    viewport->render();

  }

  else {

    // Draw a stereo pair... by convention, we render left first,
    // and the buffers are swapped when we render the _right_ image
    // (by CHAI convention)
    viewport->render(CHAI_STEREO_LEFT);

    // Now I have time to do my business...

    viewport->render(CHAI_STEREO_RIGHT);    

  }
  */


  return 0;

}


void Cobject_loaderApp::copy_rendering_options_to_gui() {

  g_main_dlg->m_usecolors_check = (object->getColorsEnabled()?BST_INDETERMINATE:BST_UNCHECKED);
  g_main_dlg->m_material_check = (object->getMaterialEnabled()?BST_INDETERMINATE:BST_UNCHECKED);
  g_main_dlg->m_transparency_check = (object->getTransparencyEnabled()?BST_INDETERMINATE:BST_UNCHECKED);
  g_main_dlg->UpdateData(FALSE);
  
}

void Cobject_loaderApp::update_options_from_gui(int preserve_loaded_properties) {

  // The second parameter in each case transfers options to
  // any children the object has...
  object->setWireMode(g_main_dlg->m_usewireframe_check, true);
  object->setShowFrame(g_main_dlg->m_showframe_check, true);
  object->showNormals(g_main_dlg->m_shownormals_check, true);
  object->setShowBox(g_main_dlg->m_showbox_check, true);
  object->useCulling(g_main_dlg->m_culling_check, true);
  object->useTexture(g_main_dlg->m_usetexture_check, true);
  
  if (shader) shader->setShadingEnabled(g_main_dlg->m_shaders_check);

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


// A function used to pop up a dialog box and ask the user to select
// a file (for selecting game files) (last five parameters are optional).
int FileBrowse(char* buffer, int length, int save, char* forceExtension,
               char* extension_string, char* title) {
  
  CFileDialog cfd(1-save,0,0,0,extension_string,g_main_dlg);

  if (forceExtension != 0) {
    cfd.m_ofn.lpstrDefExt = forceExtension;
  }

  if (title != 0) {
    cfd.m_ofn.lpstrTitle = title;
  }

  if (save) {
    cfd.m_ofn.Flags |= OFN_OVERWRITEPROMPT;
  }

  // We don't want to change the working path
  cfd.m_ofn.Flags |= OFN_NOCHANGEDIR;

  if (cfd.DoModal() != IDOK) return -1;

  CString result = cfd.GetPathName();

  // This should never happen, since I always pass in _MAX_PATH
  // for the length.
  if (result.GetLength() > length) {
    _cprintf("Warning: truncating filename to %d characters\n",length);
  }

  strncpy(buffer,(LPCSTR)(result),length);

  // strncpy needs help with the nul termination
  buffer[length-1] = '\0';

  return 0;
}


// Called by the GUI when the user clicks the "load shader"
// button.  Opens the specified file and applies the shader to
// the current object.
int Cobject_loaderApp::LoadShader(const char* filename) {

  // Give the base filename .vert and .frag extensions
  char vertex_filename[_MAX_PATH];
  char fragment_filename[_MAX_PATH];

  replace_extension(vertex_filename,filename,"vert");
  replace_extension(fragment_filename,filename,"frag");

  bool had_shader = (shader!=0);

  if (had_shader == false) shader = new cGLSLShader;

  shader->loadFragmentShaderFromFile(fragment_filename);
  shader->loadVertexShaderFromFile(vertex_filename);
  
  // Stick the shader in the scenegraph between the world and the object
  if (had_shader == false) {
    object->removeFromGraph();
    shader->addChild(object);
    world->addChild(shader);
  }

  shader->setShadingEnabled(g_main_dlg->m_shaders_check);

  return 0;
}



// Called by the GUI when the user clicks the "load model"
// button.  Opens the specified file and displays the model (graphically
// and haptically).
//
// To demonstrate some extra mesh processing, this function also scales
// the mesh's vertices to fit nicely in the display and centers the model
// in the viewing area.
int Cobject_loaderApp::LoadModel(char* filename) {

  // create a new mesh

#ifdef USE_VERTEX_BUFFER_MESHES
  // Create a mesh that uses vertex buffers
  cMesh* new_object = new cVBOMesh(world);
#else
  // Create a mesh that uses plain-vanilla GL rendering
  cMesh* new_object = new cMesh(world);
#endif   
  
  _cprintf("Loading mesh file %s\n",filename);

  // load 3d object file
  int result = new_object->loadFromFile(filename);

  if (result == 0) {

    _cprintf("Could not load model %s\n",filename);
    delete new_object;
    return -1;

  }

  // I'm going to scale the object so his maximum axis has a
  // size of MESH_SCALE_SIZE. This will make him fit nicely in
  // our viewing area.
#define MESH_SCALE_SIZE 2.0

  // Tell him to compute a bounding box...
  new_object->computeBoundaryBox(true);

  cVector3d min = new_object->getBoundaryMin();
  cVector3d max = new_object->getBoundaryMax();

  // This is the "size" of the object
  cVector3d span = max;
  span.sub(min);

  // Find his maximum dimension
  float max_size = span.x;
  if (span.y > max_size) max_size = span.y;
  if (span.z > max_size) max_size = span.z;

  // We'll center all vertices, then multiply by this amount,
  // to scale to the desired size.
  float scale_factor = MESH_SCALE_SIZE / max_size;

  // To center vertices, we add this amount (-1 times the
  // center of the object's bounding box)
  cVector3d offset = max;
  offset.add(min);
  offset.div(2.0);
  offset.negate();

  // Now we need to actually scale all the vertices.  However, the
  // vertices might not actually be in this object; they might
  // be in children or grand-children of this mesh (depending on how the 
  // model was defined in the file).
  // 
  // So we find all the sub-meshes we loaded from this file, by descending
  // through all available children.

  // This will hold all the meshes we need to operate on... we'll fill
  // it up as we find more children.
  std::list<cMesh*> meshes_to_scale;

  // This will hold all the parents we're still searching...
  std::list<cMesh*> meshes_to_descend;
  meshes_to_descend.push_front(new_object);

  // Keep track of how many meshes we've found, just to print
  // it out for the user
  int total_meshes = 0;

  // While there are still parent meshes to process
  while(meshes_to_descend.empty() == 0) {

    total_meshes++;

    // Grab the next parent
    cMesh* cur_mesh = meshes_to_descend.front();
    meshes_to_descend.pop_front();
    meshes_to_scale.push_back(cur_mesh);

    // Put all his children on the list of parents to process
    for(unsigned int i=0; i<cur_mesh->getNumChildren(); i++) {

      cGenericObject* cur_object = cur_mesh->getChild(i);

      // Only process cMesh children
      cMesh* cur_mesh = dynamic_cast<cMesh*>(cur_object);
      if (cur_mesh) meshes_to_descend.push_back(cur_mesh);
    }
  }

  _cprintf("Loaded %d vertices in %d meshes from model %s\n",
    new_object->getNumVertices(true),total_meshes,filename);
  
  std::list<cMesh*>::iterator mesh_iter;

  // Now loop over _all_ the meshes we found...
  for(mesh_iter = meshes_to_scale.begin(); mesh_iter != meshes_to_scale.end(); mesh_iter++) {

    cMesh* cur_mesh = *mesh_iter;
    vector<cVertex>* vertices = cur_mesh->pVertices();
    int num_vertices = cur_mesh->getNumVertices(false);
    cVertex* cur_vertex = (cVertex*)(vertices);

    // Move and scale each vertex in this mesh...
    for(int i=0; i<num_vertices; i++) {
      cur_vertex = cur_mesh->getVertex(i);
      cVector3d pos = cur_vertex->getPos();      
      pos.add(offset);
      pos.mul(scale_factor);
      cur_vertex->setPos(pos);
      cur_vertex++;
    }
  }

  int size = new_object->pTriangles()->size();

  // Re-compute a bounding box
  new_object->computeBoundaryBox(true);
  
  // Build a nice collision-detector for this object, so
  // the proxy will work nicely when haptics are enabled.
  _cprintf("Building collision detector...\n");

  // new_object->createSphereTreeCollisionDetector(true,true);
  new_object->createAABBCollisionDetector(true,true);
  
  _cprintf("Finished building collision detector...\n");

  new_object->computeGlobalPositions();
  
  int haptics_was_enabled = haptics_enabled;
  if (haptics_was_enabled) {
    disable_haptics_temporarily_received = false;
    disable_haptics_temporarily = true;
    unsigned int i=0;
    #define MAX_WAIT_LOOPS 15
    while(disable_haptics_temporarily_received == false && i < MAX_WAIT_LOOPS) {
      Sleep(5);
      i++;
    }
    if (i >= MAX_WAIT_LOOPS ) {
      _cprintf("Warning: could not turn forces off to load a new model\n");
    }
  }

  // Replace the old object we're displaying with the new one  
  object->removeFromGraph();
  delete object;
  object = new_object;

  // Put the object under the shader in our scenegraph if we have a shader
  if (shader) shader->addChild(object);
  else world->addChild(object);
  

  if (haptics_was_enabled) {
    disable_haptics_temporarily = false;
  }

  /*
  // Optional things that help make some models look good...

  // Fix normals if they're broken in the file...
  object->computeAllNormals();
  object->reverseAllNormals();

  // Set a transparency level for each vertex (for vertex-color mode) and
  // for the current material (for material-color mode)
  object->setTransparencyLevel(0.5);

  // Enable multi-pass transparency
  object->setTransparencyRenderMode(true,true);
  object->enableTransparency(true,true);
  camera->enableMultipassTransparency(true);
  
  // Add a nice material if the default material for this object doesn't
  // look good...
  cMaterial m = object->m_material;
  m.m_ambient.set(1.0,0.8,0.8,0.5);
  m.m_diffuse.set(1.0,0.8,0.8,0.5);
  object->setMaterial(m,true);
  */

  // Copy relevant rendering variables back to the GUI
  copy_rendering_options_to_gui();

  // This will set up default rendering options and haptic properties
  //
  // Don't over-write things we loaded from the file...
  update_options_from_gui(1);

  // Change the text label displayed to the user so it reflects the new filename...
  label->clearLabels();
  char short_filename[_MAX_PATH];
  find_filename(short_filename,filename,true);
  label->addLabel(short_filename);

  return 0;

}


// Applies a texture - loaded from the specified file -
// to the current model, if texture coordinates are
// defined.
int Cobject_loaderApp::LoadTexture(char* filename) {

  cTexture2D *newTexture = world->newTexture();
  int result = newTexture->loadFromFile(filename);

  if (result == 0) {
    _cprintf("Could not load texture file %s\n",filename);
    delete newTexture;
    return -1;
  }

  object->setTexture(newTexture,1);
  object->useTexture(1,1);

  return 0;
}


// Called when the camera zoom slider changes
void Cobject_loaderApp::zoom(int zoom_level) {

  //camera->setPos(p.x,((float)(zoom_level))/100.0*10.0,p.z);
  camera->setFieldViewAngle(((float)(zoom_level)));
  
}


// Called when the user moves the mouse in the main window
// while a button is held down
void Cobject_loaderApp::scroll(CPoint p, int button) {

  int shift_pressed = GetKeyState(VK_SHIFT) & (1<<15);

  // Get the current camera vectors
  cVector3d up = camera->getUpVector();
  cVector3d right = camera->getRightVector();
  cVector3d look = camera->getLookVector();

  if (button == MOUSE_BUTTON_RIGHT) {

    cVector3d motion;

    if (shift_pressed) {
      // In/out motion based on vertical mouse movement
      motion = ((double)p.y)/50.0 * look +
        ((double)p.x)/-50.0 * right;        
    }
    else {
      // Up/down motion based on vertical mouse movement
      motion = ((double)p.y)/50.0 * up +
        ((double)p.x)/-50.0 * right;        
    }
    camera->translate(motion);
    world->computeGlobalPositions(true);
    return;
  }

  // Middle button
  if (button == MOUSE_BUTTON_MIDDLE) {
    
    cMatrix3d rot;
    rot.identity();

    // Map horizontal mouse motion to motion around the up vector
    rot.rotate(up,(float)p.x / 300.0);

    // Map vertical mouse motion to motion around the right vector
    rot.rotate(right,(float)p.y / 300.0);    

    // Build a new rotation matrix for the camera
    cVector3d new_look = rot * look;
    cVector3d new_up = rot * up;
    new_look.normalize();
    new_up.normalize();
    cVector3d new_right = cCross(new_up,new_look);

    rot.setCol0(new_look);
    rot.setCol1(new_right);
    rot.setCol2(new_up);

    camera->setRot(rot);
    world->computeGlobalPositions(true);
    return;
  }

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

  if (button == MOUSE_BUTTON_LEFT) {

    if (shift_pressed) {
      cVector3d translation_vector = 
        (-1.0*(float)p.y / 100.0) * camera->getUpVector() +
        ( 1.0*(float)p.x / 100.0) * camera->getRightVector();
      object_to_move->translate(translation_vector);      
    }

    else {    

      // These vectors come from the (unusual) definition of the CHAI
      // camera's rotation matrix:
      //
      // column 0: look
      // column 2: up
      // column 1: look x up

      // Rotation around the horizontal camera axis
      cVector3d axis1(0,1,0);
      camera->getRot().mul(axis1);
      object_to_move->rotate(axis1,1.0*(float)p.y / 50.0);
  
      // Rotation around the vertical camera axis
      cVector3d axis2(0,0,1);
      camera->getRot().mul(axis2);
      object_to_move->rotate(axis2,(float)p.x / 50.0);
    }

    // The object has moved/rotated, so update his global position/rotation
    object->computeGlobalPositions(true);
    //object->computeBoundaryBox(true);    
  }  
}


// Called when the user clicks his mouse in the main window
//
// Lets CHAI figure out which object was clicked on.
void Cobject_loaderApp::select(CPoint p) {

  if (viewport->select(p.x, p.y, true)) {
    selected_object = viewport->getLastSelectedObject();         
  }
  else {    
    selected_object = 0;
  }

}


// Our haptic loop... just computes forces on the 
// phantom every iteration, until haptics are disabled
// in the supplied Cobject_loaderApp

// A single iteration through the loop...
void object_loader_haptic_iteration(void* param) {

  Cobject_loaderApp* app = (Cobject_loaderApp*)(param);

  app->animate();

  if (app->disable_haptics_temporarily_received == false && app->disable_haptics_temporarily) {
    _cprintf("Temporarily disabling haptic forces to load a new model...\n");
    app->disable_haptics_temporarily_received = true;
  }

  app->tool->updatePose();
  
  if (app->disable_haptics_temporarily == false) app->tool->computeForces();
  else app->tool->m_lastComputedGlobalForce.zero();

  app->tool->applyForces();

}



// If we're using a device-supported callback mechanism, this is how
// we'll be called...
void Cobject_loaderApp::callback() {
  object_loader_haptic_iteration(this);
}


// We use this in our "animation" routine...
inline double sgn(const double& a) {
  if (a>=0) return 1.0;
  return -1.0;  
}       


void Cobject_loaderApp::animate() {

  if (object == 0) return;
       
  // The A and D keys can be used to move the current object around
  if (keys_to_handle['A']) {
    object->translate(-0.02,0,0);
    object->computeGlobalPositions(1);
    keys_to_handle['A'] = 0;
  }
  if (keys_to_handle['D']) {
    object->translate( 0.02,0,0);
    object->computeGlobalPositions(1);
    keys_to_handle['D'] = 0;
  }
          

  cPrecisionClock clock;
  double curtime = clock.getCPUtime();
    
  if (moving_object && m_last_animation_time >= 0) {

    double elapsed = curtime - m_last_animation_time;

    // Move the object...
    cVector3d delta = cMul(elapsed,m_animation_velocity);
    object->translate(delta);
    
    double r_delta = elapsed * OBJECT_R_VELOCITY;
    object->rotate(cVector3d(0,1,0),r_delta);

    // This is necessary to allow collision detection to work
    // after object movement
    object->computeGlobalPositions(1);    
    // object->computeBoundaryBox(true);

    // Turn our velocity around if we reach the end of the animation
    // space...
    if (
        ( fabs(object->getPos().x) > MAXIMUM_ANIMATION_XVAL )
        &&
        ( sgn(m_animation_velocity.x) == sgn(object->getPos().x) )
       )
       {

        m_animation_velocity[0] = m_animation_velocity[0] * -1.0;

    } // if we need to turn around
        
  } // if we're animating our object

  m_last_animation_time = curtime;

}


void Cobject_loaderApp::toggle_animation() {

  if (object == 0) return;

  // Disable animation
  if (moving_object) {
    moving_object = 0;
    object->setPos(g_initial_object_pos);
    object->computeGlobalPositions(true);

    // Disable the "dynamic proxy", which will handle moving objects
    cProxyPointForceAlgo* proxy = tool->getProxy();
    proxy->enableDynamicProxy(false);
    proxy->setMovingObject(0);
  }

  // Enable animation
  else {
    moving_object = 1;
    m_last_animation_time = -1.0;

    // Enable the "dynamic proxy", which will handle moving objects
    cProxyPointForceAlgo* proxy = tool->getProxy();
    proxy->enableDynamicProxy(true);
    proxy->setMovingObject(object);

    // Put the object at the zero position...
    object->setPos(g_initial_object_pos);

    // And reset the animation velocity to its initial
    // default...
    m_animation_velocity = cVector3d(INITIAL_OBJECT_X_VELOCITY,0,0);
  }

}


// This loop is used only in the threaded version of this
// application... all it does is call the main haptic
// iteration loop, which is called directly from a timer
// callback if USE_MM_TIMER_FOR_HAPTICS is defined
DWORD object_loader_haptic_loop(void* param) {

  Cobject_loaderApp* app = (Cobject_loaderApp*)(param);

  while(app->haptics_enabled) {
    object_loader_haptic_iteration(param);
  }

  app->haptics_thread_running = 0;

  return 0;
}


void Cobject_loaderApp::reinitialize_viewport(int stereo_enabled) {

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
void Cobject_loaderApp::toggle_haptics(int enable) {

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
 
      tool = new cMeta3dofPointer(world, 0, USE_PHANTOM_DIRECT_IO);
      
      // The tool is rendered as a child of the camera.
      //
      // This has the nice property of always aligning the Phantom's
      // axes with the camera's axes.
      camera->addChild(tool);
      tool->setPos(-4.0, 0.0, 0.0);

      // This is what we would do if we _didn't_ want the tool to
      // move around as a child of the camera
      //world->addChild(tool);
      // Rotate the tool so its axes align with our opengl-like axes
      //tool->rotate(cVector3d(0,0,1),-90.0*M_PI/180.0);
      //tool->rotate(cVector3d(1,0,0),-90.0*M_PI/180.0);
      
      // set up a nice-looking workspace for the phantom so 
      // it fits nicely with our models
      tool->setWorkspace(2.0,2.0,2.0);         
      tool->setRadius(0.05);      
         
    }
    
    // set up the device
    tool->initialize();

    // open communication to the device
    tool->start();

    // tell the tool to show his coordinate frame so you
    // can see tool rotation
    tool->visualizeFrames(true);

    // I need to call this so the tool can update its internal
    // transformations before performing collision detection, etc.
    tool->computeGlobalPositions();
    tool->setForcesON();

    // Make sure the haptic device knows where he is in the camera frame
    world->computeGlobalPositions(true);

    // If our device supports callbacks, use them to run our haptic loop...
    bool result = tool->getDevice()->setCallback(this);

    if (result) {
      _cprintf("Using native haptic device callback...\n");
    }

    else {
    
#ifdef USE_MM_TIMER_FOR_HAPTICS

      // start the mm timer to run the haptic loop
      timer.set(0,object_loader_haptic_iteration,this);

#else

      // start haptic thread
      haptics_thread_running = 1;

      DWORD thread_id;
      ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(object_loader_haptic_loop), this, 0, &thread_id);

      // Boost thread and process priority
      ::SetThreadPriority(&thread_id, THREAD_PRIORITY_ABOVE_NORMAL);
      //::SetPriorityClass(GetCurrentProcess(),ABOVE_NORMAL_PRIORITY_CLASS);

#endif

    }

  } // enabling

  else if (enable == TOGGLE_HAPTICS_DISABLE) {

    // Don't do anything if haptics are already off
    if (haptics_enabled == 0) return;

    // tell the haptic thread to quit
    haptics_enabled = 0;
    tool->getDevice()->setCallback(0); // unregister the callback
#ifdef USE_MM_TIMER_FOR_HAPTICS

    timer.stop();

#else

    // wait for the haptic thread to quit
    while(haptics_thread_running) Sleep(1);

#endif
    
    // Stop the haptic device...
    tool->setForcesOFF();
    tool->stop();
    
    // SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);    

  } // disabling

} // toggle_haptics()


// A global function for sticking a cube in the given mesh
// 
// Manually creates the 12 triangles (two per face) required to
// model a cube
void createCube(cMesh *mesh, float edge) {

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

  // +y face
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
