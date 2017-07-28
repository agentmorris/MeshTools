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
    \date       03/2006
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Different compilers like slightly different GLUT's 
#ifdef _MSVC
#include "../../../external/OpenGL/msvc6/glut.h"
#else
  #ifdef _POSIX
    #include <GL/glut.h>
  #else
    #include "../../../external/OpenGL/bbcp6/glut.h"
  #endif
#endif

//---------------------------------------------------------------------------
#include "CCamera.h"
#include "CLight.h"
#include "CWorld.h"
#include "CMesh.h"
#include "CTriangle.h"
#include "CVertex.h"
#include "CMaterial.h"
#include "CTexture2D.h"
#include "CMatrix3d.h"
#include "CVector3d.h"
#include "CPrecisionTimer.h"
#include "CMeta3dofPointer.h"
#include "CBitmap.h"
//---------------------------------------------------------------------------

// the world in which we will create our environment
cWorld* world;

// the camera which is used view the environment in a window
cCamera* camera;

// a light source
cLight *light;

// our loaded object
cMesh* object;

// a little "chai3d" bitmap logo at the bottom of the screen
cBitmap* logo;

// a 3D tool which represents the haptic device
cMeta3dofPointer* tool;

// haptic timer callback
cPrecisionTimer timer;

// width and height of the current viewport display
int width   = 0;
int height  = 0;

// menu options
const int OPTION_FULLSCREEN     = 1;
const int OPTION_WINDOWDISPLAY  = 2;

// track mouse-button state (-1, glut_left_button, or glut_middle_button) 
int buttonDown = -1;
int lastX = 0 ,lastY = 0;

// a function to help us load and scale meshes
void loadMesh(char* filename);

//---------------------------------------------------------------------------

void draw(void)
{
    // set the background color of the world
    cColorf color = camera->getParentWorld()->getBackgroundColor();
    glClearColor(color.getR(), color.getG(), color.getB(), color.getA());

    // clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render world
    camera->renderView(width, height);

    // check for any OpenGL errors
    GLenum err;
    err = glGetError();
    if (err != GL_NO_ERROR) printf("Error:  %s\n", gluErrorString(err));

    // Swap buffers
    glutSwapBuffers();
}

//---------------------------------------------------------------------------

void key(unsigned char key, int x, int y)
{
    // 'ESC' for quit...
    if (key == 27)
    {
        // stop the simulation timer
        timer.stop();

        // stop the tool
        tool->stop();

#ifdef _WIN32
        // wait for the simulation timer to close
        Sleep(100);
#endif

        // exit application
        exit(0);
    }
}

//---------------------------------------------------------------------------

void resizeWindow(int w, int h)
{
    // update the size of the viewport
    width = w;
    height = h;
    glViewport(0, 0, width, height);

    // update the size of the "chai3d" logo
    float scale = (float) w / 1500.0;
    logo->setZoomHV(scale, scale);
}

//---------------------------------------------------------------------------

void mouseDown(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        lastX = x;
        lastY = y;
        buttonDown = button;
    }

    else
    {
        buttonDown = -1;
    }
}

//---------------------------------------------------------------------------

void mouseMove(int x, int y)
{
    if (buttonDown == -1) return;

    int dx = x - lastX;
    int dy = y - lastY;

    lastX = x;
    lastY = y;
   
    if (buttonDown == GLUT_LEFT_BUTTON)
    {
        // rotate the model

        // These vectors come from the (unusual) definition of the CHAI
        // camera's rotation matrix:
        //
        // column 0: look
        // column 2: up
        // column 1: look x up

        // Rotation around the horizontal camera axis
        cVector3d axis1(0,1,0);
        camera->getRot().mul(axis1);
        object->rotate(axis1,1.0*(float)dy / 50.0);

        // Rotation around the vertical camera axis
        cVector3d axis2(0,0,1);
        camera->getRot().mul(axis2);
        object->rotate(axis2,(float)dx / 50.0);

        object->computeGlobalPositions(true);
    }
    else
    {
        // move the model
        cVector3d translation_vector = 
          (-1.0*(float)dy / 100.0) * camera->getUpVector() +
          ( 1.0*(float)dx / 100.0) * camera->getRightVector();
        object->translate(translation_vector);

        object->computeGlobalPositions(true);
    }
}

//---------------------------------------------------------------------------

void updateDisplay(int val)
{
    // draw scene
    draw();

    // update the GLUT timer for the next rendering call
    glutTimerFunc(30, updateDisplay, 0);
}

//---------------------------------------------------------------------------

void menuCallback(int value)
{
    switch (value)
    {
        case OPTION_FULLSCREEN:
            glutFullScreen();
            break;

        case OPTION_WINDOWDISPLAY:
            glutReshapeWindow(512, 512);
            glutInitWindowPosition(0, 0);
            break;
    }
    
    glutPostRedisplay();
}

//---------------------------------------------------------------------------

void hapticsLoop(void* a_pUserData)
{
    // read position from haptic device
    tool->updatePose();

    // compute forces
    tool->computeForces();

    // send forces to haptic device
    tool->applyForces();
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{

    // display pretty message
    printf ("\n");
    printf ("  ===================================\n");
    printf ("  CHAI 3D\n");
    printf ("  Viewmesh Demo\n");
    printf ("  Copyright 2006\n");
    printf ("\n  Use the left mouse button to rotate the model\n");
    printf ("\n  Use the middle mouse button to move the model\n");
    printf ("\n  Use the right mouse button to switch to fullscreen\n");
    printf ("  ===================================\n");
    printf ("\n");

    // make sure the user specified a mesh
    if (argc < 2)
    {
      printf("Usage: %s [mesh_filename]\n\n",argv[0]);
      return -1;
    }

    // create a new world
    world = new cWorld();

    loadMesh(argv[1]);

    if (object == 0) {
        printf("Could not load model %s\n\n",argv[1]);
        return -1;
    }

    // set background color
    world->setBackgroundColor(0.0f,0.0f,0.0f);

    // create a camera
    camera = new cCamera(world);
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

    // load a little chai bitmap logo which will located at the bottom of the screen
    logo = new cBitmap();
    logo->m_image.loadFromFile("./resources/images/chai3d.bmp");
    logo->setPos(10,10,0);
    camera->m_front_2Dscene.addChild(logo);

    // we replace the background color of the logo (black) with a transparent color.
    // we also enable transparency
    logo->m_image.replace(cColorb(0,0,0), cColorb(0,0,0,0));
    logo->enableTransparency(true);

    // Create a light source and attach it to the camera
    light = new cLight(world);
    light->setEnabled(true);
    light->setPos(cVector3d(2,0.5,1));
    light->setDir(cVector3d(-2,0.5,1));
    camera->addChild(light);

    // create a tool and add it to the world.
    tool = new cMeta3dofPointer(world, false);

    // This is what we would do if we _didn't_ want the tool to
    // move around as a child of the camera
    //   world->addChild(tool);
    // Rotate the tool so its axes align with our opengl-like axes
    //   tool->rotate(cVector3d(0,0,1),-90.0*M_PI/180.0);
    //   tool->rotate(cVector3d(1,0,0),-90.0*M_PI/180.0);

    // set up a nice-looking workspace for the phantom so 
    // it fits nicely with our models
    tool->setPos(-4.0, 0.0, 0.0);
    tool->setWorkspace(2.0,2.0,2.0);         
    tool->setRadius(0.05);      
    camera->addChild(tool);

    // initialize the GLUT windows
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(draw);
    glutKeyboardFunc(key);
    glutReshapeFunc(resizeWindow);
    glutSetWindowTitle("CHAI 3D");

    // create a mouse menu
    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Full Screen", OPTION_FULLSCREEN);
    glutAddMenuEntry("Window Display", OPTION_WINDOWDISPLAY);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMotionFunc(mouseMove);
    glutMouseFunc(mouseDown);

    // Make sure the haptic device knows where he is in the camera frame
    world->computeGlobalPositions(true);

    // set up the device
    tool->initialize();

    // open communication to the device
    tool->start();

    // start haptic timer callback
    timer.set(0, hapticsLoop, NULL);

    // update display
    glutTimerFunc(30, updateDisplay, 0);

    // start main graphic rendering loop
    glutMainLoop();
    return 0;
}

//---------------------------------------------------------------------------

void loadMesh(char* filename)
{

  // load the mesh specified on the command line
  object = new cMesh(world);

  // load 3d object file
  int result = object->loadFromFile(filename);

  if (result == 0)
  {
    delete object;
    object = 0;
    return;
  }

    // I'm going to scale the object so his maximum axis has a
    // size of MESH_SCALE_SIZE. This will make him fit nicely in
    // our viewing area.
#define MESH_SCALE_SIZE 2.0

    // Tell him to compute a bounding box...
    object->computeBoundaryBox(true);

    cVector3d min = object->getBoundaryMin();
    cVector3d max = object->getBoundaryMax();

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
    meshes_to_descend.push_front(object);

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

    printf("Loaded %d vertices in %d meshes from model %s\n",
      object->getNumVertices(true),total_meshes,filename);

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

    int size = object->pTriangles()->size();

    // Re-compute a bounding box
    object->computeBoundaryBox(true);

    // Build a nice collision-detector for this object, so
    // the proxy will work nicely when haptics are enabled.
    printf("Building collision detector...\n");

    // object->createSphereTreeCollisionDetector(true,true);
    object->createAABBCollisionDetector(true,true);

    printf("Finished building collision detector...\n");

    object->computeGlobalPositions();

    world->addChild(object);

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

}
