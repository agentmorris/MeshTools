//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CWorld.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "CWorld.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
#include "gl/gl.h"
#include "gl/glu.h"
#include "XColor.h"
#include "CCamera.h"
#include "CShape.h"
//---------------------------------------------------------------------------

//==========================================================================
// - PUBLIC METHOD -
/*!
      Constructor of cWorld.

      \fn       cWorld::cWorld()
      \return   Return pointer to new cWorld instance.
*/
//===========================================================================
cWorld::cWorld()
{
  // CREATE LIGHTS:
  for (int i=0; i<8; i++)
  {
    lights[i] = new cLight(i);
  }
  // USE LIGHTINGS:
  useLighting = true;

  // SET LIGHT ID 0 ON:
  lights[0]->enabled = true;

  // SET PROPERTIES:
  backgroundColor = xSetColor4f(0.0f, 0.0f, 0.0f, 1.0f);

  // CREATE VIRTUAL FINGER:
  finger = new cFinger();

  // CREATE LIST OF SHAPES:
  shapes = new TList();
}


//===========================================================================
// - PUBLIC METHOD -
/*!
      Destructor of cWorld

      \fn       cWorld::~cWorld()
*/
//===========================================================================
cWorld::~cWorld()
{
  int i;
  int numItems;

  // DELETE LIGHTS:
  for (int i=0; i<8; i++)
  {
    delete lights[i];
  }

  // DELETE FINGER:
  delete finger;

  // DELETE SHAPES:
  numItems = shapes->Count;
  i = 0;
  while (i < numItems)
  {
    cShape* nextShape = (cShape*)shapes->Items[i];
    delete nextShape;
    i++;
  }
  delete shapes;
  
}


//===========================================================================
// - PUBLIC METHOD -
/*!
      Render the world in OpenGL. User should not call this function
      directly. To render an image, call viewport->render().

      \fn       void cWorld::render(cCamera *iCamera,
                     int iWindowWidth, int iWindowHeight)
      \param    iCamera           Camera used for rendering scene.
      \param    iWindowWidth      Width of Viewport Window.
      \param    iWindowHeight     Height of Viewport Window.
*/
//===========================================================================
void cWorld::render(cCamera *iCamera,
                     int iWindowWidth, int iWindowHeight)
{
  // DECLARATIONS:
  int i;

  // SET CLEAR COLOR:
  glClearColor((backgroundColor.r), (backgroundColor.g),
               (backgroundColor.b), (backgroundColor.a));

  // CLEAR BUFFERS OPEN GL:
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // DEPTH BUFFER IS WRITABLE:
  glDepthMask(GL_TRUE);

  // ENABLE DEPTH TEST:
  glEnable(GL_DEPTH_TEST);

  // PROJECTION MATRIX
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, iWindowWidth, 0.0, iWindowHeight);

  // SET CAMERA:
  glLoadIdentity();
  iCamera->render(iWindowWidth, iWindowHeight);

  // SETUP MODEL_VIEW:
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //------------------------------------------------------------------------
  // LIGHTINGS:
  //------------------------------------------------------------------------
  if (useLighting)
  {
    glEnable(GL_LIGHTING);        // Enable Lightings.
    for (int i=0; i<8; i++)       // Render the 8 Lights.
    {
      lights[i]->render();
    }
  }
  else
  {
    glDisable(GL_LIGHTING);       // Disable Lightings.
  }
  //------------------------------------------------------------------------
  // RENDER FINGER:
  //------------------------------------------------------------------------
  finger->render();

  //------------------------------------------------------------------------
  // RENDER SHAPES:
  //------------------------------------------------------------------------
  i = 0;
  while (i < shapes->Count)
  {
    cShape *nextShape = (cShape*)shapes->Items[i];
    nextShape->render();
    i++;
  }

  // RENDER WORKSPACE OF VIRTUAL DEVICE:
  xColor4f color = xSetColor4f(0.3, 0.0, 1.0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glColor4fv( (const float *)&color);

  // DRAW PLAN:
  double xMaxSpace;
  double xMinSpace;
  double yMaxSpace;
  double yMinSpace;
  double zMaxSpace;
  double zMinSpace;


  xMaxSpace = 0.3;
  xMinSpace = -0.1;
  yMaxSpace = 0.2;
  yMinSpace = -0.2;
  zMaxSpace = 0.2;
  zMinSpace = -0.2;

  xVector3d P01 = xSet(xMinSpace, yMinSpace, zMinSpace);
  xVector3d P02 = xSet(xMinSpace, yMaxSpace, zMinSpace);
  xVector3d P03 = xSet(xMinSpace, yMaxSpace, zMaxSpace);
  xVector3d P04 = xSet(xMinSpace, yMinSpace, zMaxSpace);

  xVector3d P11 = xSet(xMaxSpace, yMinSpace, zMinSpace);
  xVector3d P12 = xSet(xMaxSpace, yMaxSpace, zMinSpace);
  xVector3d P13 = xSet(xMaxSpace, yMaxSpace, zMaxSpace);
  xVector3d P14 = xSet(xMaxSpace, yMinSpace, zMaxSpace);

  glBegin(GL_LINE_STRIP);
    glVertex3dv((const double *)&P01);
    glVertex3dv((const double *)&P02);
    glVertex3dv((const double *)&P03);
    glVertex3dv((const double *)&P04);
    glVertex3dv((const double *)&P01);
  glEnd();
  glBegin(GL_LINE_STRIP);
    glVertex3dv((const double *)&P11);
    glVertex3dv((const double *)&P12);
    glVertex3dv((const double *)&P13);
    glVertex3dv((const double *)&P14);
    glVertex3dv((const double *)&P11);
  glEnd();
  glBegin(GL_LINES);
    glVertex3dv((const double *)&P01);
    glVertex3dv((const double *)&P11);
    glVertex3dv((const double *)&P02);
    glVertex3dv((const double *)&P12);
    glVertex3dv((const double *)&P03);
    glVertex3dv((const double *)&P13);
    glVertex3dv((const double *)&P04);
    glVertex3dv((const double *)&P14);
  glEnd();
}


//===========================================================================
// - PUBLIC METHOD -
/*!
      Compute forces and send values to Phantom haptic device.

      \fn       void cWorld::renderForces()

*/
//===========================================================================
void cWorld::renderForces()
{

}


//===========================================================================
// END OF FILE
//===========================================================================
