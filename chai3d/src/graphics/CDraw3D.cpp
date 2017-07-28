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
	\author:    Francois Conti
	\author:    Dan Morris
	\version    1.1
	\date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CDraw3D.h"
#include "CMaths.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  Draw an X-Y-Z Frame. The red arrow corresponds to the X-Axis,
	  green to the Y-Axis, and blue to the Z-Axis.

	  The scale parameter determines the size of the arrows.

	  \fn       void cDrawFrame(const double a_scale=1.0,const bool a_modifyMaterialState=true)
	  \param    a_scale  Length of each arrow
	  \param    a_modifyMaterialState If true, this function sets GL to the preferred material state
*/
//===========================================================================
void cDrawFrame(const double a_scale, const bool a_modifyMaterialState)
{
	cDrawFrame(a_scale, a_scale, a_modifyMaterialState);
}


void cDrawFrame(const double a_axisLengthScale, const double a_axisThicknessScale,
	const bool a_modifyMaterialState)
{

	// Triangle vertices:
	static int nTriangles = 8;

	static float triangle_vertices[72] = {
	  0.000000f,0.040000f,-0.800000f,0.028284f,0.028284f,-0.800000f,
		0.000000f,0.000000f,-1.000000f,0.028284f,0.028284f,-0.800000f,
		0.040000f,0.000000f,-0.800000f,0.000000f,0.000000f,-1.000000f,
		0.040000f,0.000000f,-0.800000f,0.028284f,-0.028284f,-0.800000f,
		0.000000f,0.000000f,-1.000000f,0.028284f,-0.028284f,-0.800000f,
		0.000000f,-0.040000f,-0.800000f,0.000000f,0.000000f,-1.000000f,
		0.000000f,-0.040000f,-0.800000f,-0.028284f,-0.028284f,-0.800000f,
		0.000000f,0.000000f,-1.000000f,-0.028284f,-0.028284f,-0.800000f,
		-0.040000f,0.000000f,-0.800000f,0.000000f,0.000000f,-1.000000f,
		-0.040000f,0.000000f,-0.800000f,-0.028284f,0.028284f,-0.800000f,
		0.000000f,0.000000f,-1.000000f,-0.028284f,0.028284f,-0.800000f,
		0.000000f,0.040000f,-0.800000f,0.000000f,0.000000f,-1.000000f
	};

	// Triangle normals:
	static float triangle_normals[72] = {
	  0.000000f,0.980581f,-0.196116f,0.693375f,0.693375f,-0.196116f,
		0.357407f,0.862856f,-0.357407f,0.693375f,0.693375f,-0.196116f,
		0.980581f,0.000000f,-0.196116f,0.862856f,0.357407f,-0.357407f,
		0.980581f,0.000000f,-0.196116f,0.693375f,-0.693375f,-0.196116f,
		0.862856f,-0.357407f,-0.357407f,0.693375f,-0.693375f,-0.196116f,
		0.000000f,-0.980581f,-0.196116f,0.357407f,-0.862856f,-0.357407f,
		0.000000f,-0.980581f,-0.196116f,-0.693375f,-0.693375f,-0.196116f,
		-0.357407f,-0.862856f,-0.357407f,-0.693375f,-0.693375f,-0.196116f,
		-0.980581f,0.000000f,-0.196116f,-0.862856f,-0.357407f,-0.357407f,
		-0.980581f,0.000000f,-0.196116f,-0.693375f,0.693375f,-0.196116f,
		-0.862856f,0.357407f,-0.357407f,-0.693375f,0.693375f,-0.196116f,
		0.000000f,0.980581f,-0.196116f,-0.357407f,0.862856f,-0.357407f
	};

	// Quad vertices:
	static int nQuads = 16;

	static float quad_vertices[192] = {
	  0.000000f,0.010000f,0.000000f,0.007000f,0.007000f,0.000000f,
		0.007000f,0.007000f,-0.800000f,0.000000f,0.010000f,-0.800000f,
		0.000000f,-0.010000f,0.000000f,-0.007000f,-0.007000f,0.000000f,
		-0.007000f,-0.007000f,-0.800000f,0.000000f,-0.010000f,-0.800000f,
		-0.007000f,-0.007000f,0.000000f,-0.010000f,0.000000f,0.000000f,
		-0.010000f,0.000000f,-0.800000f,-0.007000f,-0.007000f,-0.800000f,
		-0.010000f,0.000000f,0.000000f,-0.007000f,0.007000f,0.000000f,
		-0.007000f,0.007000f,-0.800000f,-0.010000f,0.000000f,-0.800000f,
		-0.007000f,0.007000f,0.000000f,0.000000f,0.010000f,0.000000f,
		0.000000f,0.010000f,-0.800000f,-0.007000f,0.007000f,-0.800000f,
		0.007000f,0.007000f,0.000000f,0.010000f,0.000000f,0.000000f,
		0.010000f,0.000000f,-0.800000f,0.007000f,0.007000f,-0.800000f,
		0.010000f,0.000000f,0.000000f,0.007000f,-0.007000f,0.000000f,
		0.007000f,-0.007000f,-0.800000f,0.010000f,0.000000f,-0.800000f,
		0.007000f,-0.007000f,0.000000f,0.000000f,-0.010000f,0.000000f,
		0.000000f,-0.010000f,-0.800000f,0.007000f,-0.007000f,-0.800000f,
		-0.007000f,0.007000f,-0.800000f,-0.028284f,0.028284f,-0.800000f,
		-0.040000f,0.000000f,-0.800000f,-0.010000f,0.000000f,-0.800000f,
		-0.010000f,0.000000f,-0.800000f,-0.040000f,0.000000f,-0.800000f,
		-0.028284f,-0.028284f,-0.800000f,-0.007000f,-0.007000f,-0.800000f,
		-0.007000f,-0.007000f,-0.800000f,-0.028284f,-0.028284f,-0.800000f,
		0.000000f,-0.040000f,-0.800000f,0.000000f,-0.010000f,-0.800000f,
		0.000000f,-0.010000f,-0.800000f,0.000000f,-0.040000f,-0.800000f,
		0.028284f,-0.028284f,-0.800000f,0.007000f,-0.007000f,-0.800000f,
		0.028284f,-0.028284f,-0.800000f,0.040000f,0.000000f,-0.800000f,
		0.010000f,0.000000f,-0.800000f,0.007000f,-0.007000f,-0.800000f,
		0.040000f,0.000000f,-0.800000f,0.028284f,0.028284f,-0.800000f,
		0.007000f,0.007000f,-0.800000f,0.010000f,0.000000f,-0.800000f,
		0.007000f,0.007000f,-0.800000f,0.028284f,0.028284f,-0.800000f,
		0.000000f,0.040000f,-0.800000f,0.000000f,0.010000f,-0.800000f,
		0.000000f,0.010000f,-0.800000f,0.000000f,0.040000f,-0.800000f,
		-0.028284f,0.028284f,-0.800000f,-0.007000f,0.007000f,-0.800000f
	};

	// Quad normals:
	static float quad_normals[192] = {
	  0.000000f,1.000000f,0.000000f,0.707107f,0.707107f,0.000000f,
		0.707107f,0.707107f,0.000000f,0.000000f,1.000000f,0.000000f,
		0.000000f,-1.000000f,0.000000f,-0.707107f,-0.707107f,0.000000f,
		-0.707107f,-0.707107f,0.000000f,0.000000f,-1.000000f,0.000000f,
		-0.707107f,-0.707107f,0.000000f,-1.000000f,0.000000f,0.000000f,
		-1.000000f,0.000000f,0.000000f,-0.707107f,-0.707107f,0.000000f,
		-1.000000f,0.000000f,0.000000f,-0.707107f,0.707107f,0.000000f,
		-0.707107f,0.707107f,0.000000f,-1.000000f,0.000000f,0.000000f,
		-0.707107f,0.707107f,0.000000f,0.000000f,1.000000f,0.000000f,
		0.000000f,1.000000f,0.000000f,-0.707107f,0.707107f,0.000000f,
		0.707107f,0.707107f,0.000000f,1.000000f,0.000000f,0.000000f,
		1.000000f,0.000000f,0.000000f,0.707107f,0.707107f,0.000000f,
		1.000000f,0.000000f,0.000000f,0.707107f,-0.707107f,0.000000f,
		0.707107f,-0.707107f,0.000000f,1.000000f,0.000000f,0.000000f,
		0.707107f,-0.707107f,0.000000f,0.000000f,-1.000000f,0.000000f,
		0.000000f,-1.000000f,0.000000f,0.707107f,-0.707107f,0.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f,
		0.000000f,0.000000f,1.000000f,0.000000f,0.000000f,1.000000f
	};

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	// Set up nice color-tracking
	if (a_modifyMaterialState)
	{
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	for (int k = 0; k < 3; k++) {

		glPushMatrix();

		// Rotate to the appropriate axis
		if (k == 0) {
			glRotatef(-90.0, 0, 1, 0);
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else if (k == 1) {
			glRotatef(90.0, 1, 0, 0);
			glColor3f(0.0f, 1.0f, 0.0f);
		}
		else {
			glRotatef(180.0, 1, 0, 0);
			glColor3f(0.0f, 0.0f, 1.0f);
		}

		glScaled(a_axisThicknessScale, a_axisThicknessScale, a_axisLengthScale);

		glVertexPointer(3, GL_FLOAT, 0, triangle_vertices);
		glNormalPointer(GL_FLOAT, 0, triangle_normals);
		glDrawArrays(GL_TRIANGLES, 0, nTriangles * 3);

		glVertexPointer(3, GL_FLOAT, 0, quad_vertices);
		glNormalPointer(GL_FLOAT, 0, quad_normals);
		glDrawArrays(GL_QUADS, 0, nQuads * 4);

		glPopMatrix();
	}

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

}


//===========================================================================
/*!
	  Draw a line-based box with sides parallel to the x-y-z axes

	  \fn       void cDrawWireBox(const double a_xMin, const double a_xMax,
				const double a_yMin, const double a_yMax,
				const double a_zMin, const double a_zMax)
	  \param    a_xMin  Box side x min position.
	  \param    a_xMax  Box side x max position.
	  \param    a_yMin  Box side y min position.
	  \param    a_yMax  Box side y max position.
	  \param    a_zMin  Box side z min position.
	  \param    a_zMax  Box side z max position.
*/
//===========================================================================
void cDrawWireBox(const double a_xMin, const double a_xMax,
	const double a_yMin, const double a_yMax,
	const double a_zMin, const double a_zMax)
{
	// render lines for each edge of the box
	glBegin(GL_LINES);
	glVertex3d(a_xMin, a_yMin, a_zMin);
	glVertex3d(a_xMax, a_yMin, a_zMin);
	glVertex3d(a_xMin, a_yMax, a_zMin);
	glVertex3d(a_xMax, a_yMax, a_zMin);
	glVertex3d(a_xMin, a_yMin, a_zMax);
	glVertex3d(a_xMax, a_yMin, a_zMax);
	glVertex3d(a_xMin, a_yMax, a_zMax);
	glVertex3d(a_xMax, a_yMax, a_zMax);

	glVertex3d(a_xMin, a_yMin, a_zMin);
	glVertex3d(a_xMin, a_yMax, a_zMin);
	glVertex3d(a_xMax, a_yMin, a_zMin);
	glVertex3d(a_xMax, a_yMax, a_zMin);
	glVertex3d(a_xMin, a_yMin, a_zMax);
	glVertex3d(a_xMin, a_yMax, a_zMax);
	glVertex3d(a_xMax, a_yMin, a_zMax);
	glVertex3d(a_xMax, a_yMax, a_zMax);

	glVertex3d(a_xMin, a_yMin, a_zMin);
	glVertex3d(a_xMin, a_yMin, a_zMax);
	glVertex3d(a_xMax, a_yMin, a_zMin);
	glVertex3d(a_xMax, a_yMin, a_zMax);
	glVertex3d(a_xMin, a_yMax, a_zMin);
	glVertex3d(a_xMin, a_yMax, a_zMax);
	glVertex3d(a_xMax, a_yMax, a_zMin);
	glVertex3d(a_xMax, a_yMax, a_zMax);
	glEnd();
}


//===========================================================================
/*!
	  Render a sphere given a radius.

	  \fn       void cDrawSphere(const double a_radius,
				const unsigned int a_numSlices=10, const unsigned int a_numStacks=10)
	  \param    a_radius  Radius of the sphere
	  \param    a_numSlices  Specifies the number of subdivisions around the
							 z axis (similar to lines of longitude)
	  \param    a_numStacks  Specifies the number of subdivisions along the
							 x/y axes (similar to lines of latitude)
*/
//===========================================================================
void cDrawSphere(const double a_radius,
	const unsigned int a_numSlices, const unsigned int a_numStacks)
{
	// allocate a new OpenGL quadric object for rendering a sphere
	GLUquadricObj *quadObj;
	quadObj = gluNewQuadric();

	// set rendering style
	gluQuadricDrawStyle(quadObj, GLU_FILL);

	// set normal-rendering mode
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	// render a sphere
	gluSphere(quadObj, a_radius, a_numSlices, a_numStacks);

	// delete our quadric object
	gluDeleteQuadric(quadObj);
}



//===========================================================================
/*!

  Draw a pretty arrow on the z-axis using a cone and a cylinder (using GLUT)

  \fn       void cDrawArrow(const cVector3d& a_arrowStart, const cVector3d& a_arrowTip, const double a_width);
  \param    a_arrowStart  The location of the back of the arrow
  \param    a_arrowTip    The location of the tip of the arrow
  \param    a_width       The width (in GL units) of the arrow shaft

*/
//===========================================================================
void cDrawArrow(const cVector3d& a_arrowStart, const cVector3d& a_arrowTip, const double a_width) {

	glPushMatrix();

	// We don't really care about the up vector, but it can't
	// be parallel to the arrow...
	cVector3d up = cVector3d(0, 1, 0);
	cVector3d arrow = a_arrowTip - a_arrowStart;
	arrow.normalize();
	double d = fabs(cDot(up, arrow));
	if (d > .9)
	{
		up = cVector3d(1, 0, 0);
	}

	cLookAt(a_arrowStart, a_arrowTip, up);

	// This flips the z axis around
	glRotatef(180, 1, 0, 0);

	GLUquadricObj *quadObj;
	quadObj = gluNewQuadric();

	double distance = cDistance(a_arrowTip, a_arrowStart);

#define ARROW_CYLINDER_PORTION 0.75
#define ARRROW_CONE_PORTION (1.0 - 0.75)

	// set rendering style
	gluQuadricDrawStyle(quadObj, GLU_FILL);

	// set normal-rendering mode
	gluQuadricNormals(quadObj, GLU_SMOOTH);

	// render a cylinder and a cone
	glRotatef(180, 1, 0, 0);
	gluDisk(quadObj, 0, a_width, 10, 10);
	glRotatef(180, 1, 0, 0);

	gluCylinder(quadObj, a_width, a_width, distance*ARROW_CYLINDER_PORTION, 10, 10);
	glTranslated(0, 0, ARROW_CYLINDER_PORTION*distance);

	glRotatef(180, 1, 0, 0);
	gluDisk(quadObj, 0, a_width*2.0, 10, 10);
	glRotatef(180, 1, 0, 0);

	gluCylinder(quadObj, a_width*2.0, 0.0, distance*ARRROW_CONE_PORTION, 10, 10);

	// delete our quadric object
	gluDeleteQuadric(quadObj);

	glPopMatrix();

}
