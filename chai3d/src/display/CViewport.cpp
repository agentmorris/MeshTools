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
#include "CViewport.h"
#include "CWorld.h"

// Turn off annoying compiler warnings
#pragma warning(disable: 4838)

//---------------------------------------------------------------------------

cViewport* cViewport::lastActiveViewport = 0;

//===========================================================================
/*!
	Constructor of cViewport.

	\fn         cViewport::cViewport(HWND a_winHandle, cCamera *a_camera,
				const bool a_stereoEnabled, PIXELFORMATDESCRIPTOR* a_pixelFormat=0)
	\param      a_winHandle    Handle to the actual win32 window
	\param      a_camera       The camera through which this viewport should be rendered
	\param      a_stereoEnabled    If \b true, a stereo rendering context is created
	\param      a_pixelFormat  If non-zero, this custom pixel format is used to initialize the viewport
*/
//===========================================================================
cViewport::cViewport(HWND a_winHandle, cCamera *a_camera, const bool a_stereoEnabled, PIXELFORMATDESCRIPTOR* a_pixelFormat)
{

	// If no viewport has been created at all, creation is enough to make this
	// the active viewport
	if (lastActiveViewport == 0) lastActiveViewport = this;

	memset(m_glViewport, 0, sizeof(m_glViewport));

	// set the camera through which this viewport should be rendered
	setCamera(a_camera);

	// stereo status
	m_stereoEnabled = a_stereoEnabled;

	// update wincontrol
	m_winHandle = a_winHandle;

	// No post-render callback by default (see setPostRenderCallback() for details)
	m_postRenderCallback = 0;

	// ----------------------------
	// Initialize an OpenGL context
	// ----------------------------

	m_glDC = 0;

	// If the user requested a specific pixel format, use that as our
	// requested format for initializing the display context
	if (a_pixelFormat != 0)
	{
		m_pixelFormat = *a_pixelFormat;
	}

	// Otherwise use a default format descriptor...
	else
	{
		PIXELFORMATDESCRIPTOR pfd = {
		  sizeof(PIXELFORMATDESCRIPTOR),       // size of this pfd
		  1,                                   // version number
		  PFD_DRAW_TO_WINDOW |                 // support window
		  PFD_SUPPORT_OPENGL |                 // support OpenGL
		  (m_stereoEnabled ? PFD_STEREO : 0) | // optionally enable stereo
		  PFD_DOUBLEBUFFER,                    // double buffered
		  PFD_TYPE_RGBA,                       // RGBA type
		  32,                                  // 32-bit color depth
		  0, 0, 0, 0, 0, 0,                    // color bits ignored
		  0,                                   // no alpha buffer
		  0,                                   // shift bit ignored
		  0,                                   // no accumulation buffer
		  0, 0, 0, 0,                          // accum bits ignored
		  32,                                  // 32-bit z-buffer
		  0,                                   // no stencil buffer
		  0,                                   // no auxiliary buffer
		  PFD_MAIN_PLANE,                      // main layer
		  0,                                   // reserved
		  0, 0, 0                              // layer masks ignored
		};

		m_pixelFormat = pfd;
	}

	m_forceRenderArea.left = m_forceRenderArea.right =
		m_forceRenderArea.top = m_forceRenderArea.bottom = -1;

	if (m_winHandle != NULL)
	{
		// This actually creates the context...
		update();
	}
}


//===========================================================================
/*!
		Destructor of cViewport.

		\fn     cViewport::~cViewport()
*/
//===========================================================================
cViewport::~cViewport()
{
	cleanup();
}


//===========================================================================
/*!
	  Enable or disable stereo rendering on this viewport

	  Note that it is not possible to change the pixel format of a window
	  in Windows, so if you create a viewport that doesn't have stereo support,
	  you can't enable stereo rendering without creating a new window/viewport.

	  \fn     cViewport::setStereoOn(bool a_stereoEnabled)
*/
//===========================================================================
void cViewport::setStereoOn(bool a_stereoEnabled)
{
	// check if new mode is not already active
	if (a_stereoEnabled == m_stereoEnabled) { return; }

	// update stereo rendering state
	m_stereoEnabled = a_stereoEnabled;

	// See whether stereo is _really_ enabled
	PIXELFORMATDESCRIPTOR pfd;
	int formatIndex = GetPixelFormat(m_glDC);
	DescribePixelFormat(m_glDC, formatIndex, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	// if stereo was enabled but can not be displayed, switch over to mono.
	if (((pfd.dwFlags & PFD_STEREO) == 0) && m_stereoEnabled)
	{
		m_stereoEnabled = false;
	}
}


//===========================================================================
/*!
		Clean up the current rendering context

		\fn     bool cViewport::cleanup()
*/
//===========================================================================
bool cViewport::cleanup()
{
	bool status = true;

	// delete display context
	int result = ReleaseDC(m_winHandle, m_glDC);
	if (result == 0) status = false;

	result = wglDeleteContext(m_glContext);
	if (result == 0) status = false;

	m_glContext = 0;
	m_glDC = 0;
	m_glReady = false;
	return status;
}


//===========================================================================
/*!
		If the window has been modified, or just created, call this function
		to update the OpenGL display context.

		\fn         bool cViewport::update(bool resizeOnly)
		\param      resizeOnly  If false (default), reinitializes the GL context.
		\return     Return true if operation succeeded.
*/
//===========================================================================
bool cViewport::update(bool resizeOnly)
{

	// Clean up the old rendering context if necessary
	if ((resizeOnly == false) && m_glDC) cleanup();

	// declare variables
	int formatIndex;

	// viewport is not yet enabled
	m_enabled = false;

	// gl display not yet ready
	m_glReady = false;

	// check display handle
	if (m_winHandle == NULL) { return (false); }

	// Find out the rectangle to which we should be rendering

	// If we're using the entire window...
	if (m_forceRenderArea.left == -1)
	{
		if (GetWindowRect(m_winHandle, &m_activeRenderingArea) == 0) { return (false); }

		// Convert from screen to window coordinates
		m_activeRenderingArea.right -= m_activeRenderingArea.left;
		m_activeRenderingArea.left = 0;

		m_activeRenderingArea.bottom -= m_activeRenderingArea.top;
		m_activeRenderingArea.top = 0;

		// Convert from y-axis-down to y-axis-up, since that's how we store
		// our rendering area.
		int height = m_activeRenderingArea.bottom;
		m_activeRenderingArea.top = height - m_activeRenderingArea.top;
		m_activeRenderingArea.bottom = height - m_activeRenderingArea.bottom;

	}

	// Otherwise use whatever rectangle the user wants us to use...
	else
	{
		m_activeRenderingArea = m_forceRenderArea;
	}

	// retrieve handle of the display device context
	m_glDC = ::GetDC(m_winHandle);

	if (m_glDC == 0)
	{
		return(false);
	}

	if (resizeOnly == false)
	{
		// find pixel format supported by the device context. If error return false.
		formatIndex = ChoosePixelFormat(m_glDC, &m_pixelFormat);
		if (formatIndex == 0)
		{
			return(false);
		}

		// sets the specified device context's pixel format. If error return false
		if (!SetPixelFormat(m_glDC, formatIndex, &m_pixelFormat))
		{
			return(false);
		}

		formatIndex = GetPixelFormat(m_glDC);
		DescribePixelFormat(m_glDC, formatIndex, sizeof(PIXELFORMATDESCRIPTOR), &m_pixelFormat);

		// if stereo was enabled but can not be displayed, switch over to mono.
		if (((m_pixelFormat.dwFlags & PFD_STEREO) == 0) && m_stereoEnabled)
		{
			m_stereoEnabled = false;
		}

		// create display context
		m_glContext = wglCreateContext(m_glDC);
		if (m_glContext == 0)
		{
			return(false);
		}

		wglMakeCurrent(m_glDC, m_glContext);

	}

	// OpenGL is now ready for rendering
	m_glReady = true;

	lastActiveViewport = this;

	// enable viewport
	m_enabled = true;

	if (resizeOnly == false) onDisplayReset();

	// return success
	return(true);
}


//===========================================================================
/*!
	Call this method to render the OpenGL world inside the viewport.

	The default rendering option (CHAI_STEREO_DEFAULT) tells the
	viewport to decide whether it's rendering in stereo, and - if so - to
	render a full stereo pair.  The other rendering options let you specify a
	specific image index (mono, left, or right).

	The actual rendering is done in the renderView() function, once this
	function decides which frame to render.

	Usually you want to use CHAI_STEREO_DEFAULT.  The best reasons not to are:

	(1) I have a stereo context, but sometimes I want to render in mono for
		a while.  Alternatively, I could just disable stereo rendering
		temporarily.

	(2) I have a stereo context, but I have a lot of computation to do and I
		want to get control back between the left and right frames.

	\fn         bool cViewport::render(int imageIndex)
	\param      imageIndex Either CHAI_STEREO_DEFAULT, CHAI_MONO, CHAI_STEREO_LEFT, or CHAI_STEREO_RIGHT
	\return     Return \b true if operation succeeded.
*/
//===========================================================================
bool cViewport::render(int imageIndex)
{
	bool result;

	lastActiveViewport = this;

	// The default rendering option tells the viewport to decide
	// whether it's rendering in stereo, and - if so - to render
	// a full stereo pair.
	if (imageIndex == CHAI_STEREO_DEFAULT)
	{
		// render mono mode
		if (m_stereoEnabled)
		{
			result = renderView(CHAI_STEREO_LEFT);
			if (!result) return (false);

			result = renderView(CHAI_STEREO_RIGHT);
			return (result);
		}
		// render stereo mode
		else
		{
			result = renderView(CHAI_MONO);
			return (result);
		}
	}

	else
	{
		result = renderView(imageIndex);
		return (result);
	}

}


//===========================================================================
/*!
	Renders the OpenGL scene in the buffer specified by a_imageIndex

	\fn         void cViewport::renderView(const int a_imageIndex)
	\param      a_imageIndex  CHAI_MONO, CHAI_STEREO_LEFT or CHAI_STEREO_RIGHT
	\return     Return \b true if operation succeeded.
*/
//===========================================================================
bool cViewport::renderView(const int a_imageIndex)
{
	// Make sure the viewport is really ready for rendering
	if ((m_glReady == 0) || (m_enabled == 0) || (m_camera == NULL)) return false;


	// Find out whether we need to update the size of our viewport...

	// If we're using the whole window, see whether the window has
	// changed size...
	if (m_forceRenderArea.left == -1) {

		RECT sizeWin;
		if (GetWindowRect(m_winHandle, &sizeWin) == 0) { return (false); }

		unsigned int width = sizeWin.right - sizeWin.left;
		unsigned int height = sizeWin.bottom - sizeWin.top;

		if (
			(m_activeRenderingArea.left != 0) ||
			(m_activeRenderingArea.bottom != 0) ||
			(m_activeRenderingArea.right != width) ||
			(m_activeRenderingArea.top != height)
			)
		{
			update();
		}
	}

	// Otherwise the user is telling us to use a particular rectangle; see
	// whether that rectangle has changed...
	else
	{
		if ((m_activeRenderingArea.left != m_forceRenderArea.left) ||
			(m_activeRenderingArea.right != m_forceRenderArea.right) ||
			(m_activeRenderingArea.top != m_forceRenderArea.top) ||
			(m_activeRenderingArea.bottom != m_forceRenderArea.bottom))
		{
			update();
		}
	}

	// Activate display context
	//
	// Note that in the general case, this is not strictly necessary,
	// but if a user is using multiple viewports, we don't want him
	// to worry about the current rendering context, so we incur a bit
	// of overhead here.
	if (!wglMakeCurrent(m_glDC, m_glContext))
	{

		// Try once to re-initialize the context...
		if (!(update()))
			// And return an error if this doesn't work out...
			return(false);
	}

	// Set up rendering to the appropriate buffer
	if (a_imageIndex == CHAI_STEREO_RIGHT)
	{
		glDrawBuffer(GL_BACK_RIGHT);
	}
	else if (a_imageIndex == CHAI_STEREO_LEFT)
	{
		glDrawBuffer(GL_BACK_LEFT);
	}
	else
	{
		glDrawBuffer(GL_BACK);
	}

	// set viewport size
	int width = m_activeRenderingArea.right - m_activeRenderingArea.left;
	int height = m_activeRenderingArea.top - m_activeRenderingArea.bottom;
	glViewport(m_activeRenderingArea.left, m_activeRenderingArea.bottom,
		width, height);

	glGetIntegerv(GL_VIEWPORT, m_glViewport);

	// set background color
	cColorf color = m_camera->getParentWorld()->getBackgroundColor();
	glClearColor(color.getR(), color.getG(), color.getB(), color.getA());

	// clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render world
	m_camera->renderView(width, height, a_imageIndex);

	if (m_postRenderCallback) m_postRenderCallback->renderSceneGraph();

	// Swap buffers
	// If stereo is enabled, we only swap after the _right_ image is drawn
	if (m_stereoEnabled == 0 || a_imageIndex == CHAI_STEREO_RIGHT)
	{
		SwapBuffers(m_glDC);
	}

	// deactivate display context (not necessary)
	// wglMakeCurrent(m_glDC, 0);

	// operation succeeded
	return (true);
}


//===========================================================================
/*!
	 Select an object on displayed in the viewport. This method casts a
	 virtual ray through the viewport and asks the world for the first
	 object hit by that ray.

	 It's most useful if you want to allow the user to use the mouse to
	 click on objects in your virtual scene.

	 Use getLastSelectedObject(), getLastSelectedTriangle(), and
	 getLastSelectedPoint() to extract information about the results of
	 this operation.

	 \fn        bool cViewport::select(const unsigned int a_windowPosX,
				const unsigned int a_windowPosY, const bool a_selectVisibleObjectsOnly)
	 \param     a_windowPosX  X coordinate position of mouse click.
	 \param     a_windowPosY  Y coordinate position of mouse click.
	 \param     a_selectVisibleObjectsOnly  Should we ignore invisible objects?
	 \return    Return \b true if an object has been hit.
*/
//===========================================================================
bool cViewport::select(const unsigned int a_windowPosX, const unsigned int a_windowPosY,
	const bool a_selectVisibleObjectsOnly)
{

	if (m_camera == 0) return false;

	int width = m_activeRenderingArea.right - m_activeRenderingArea.left;
	int height = m_activeRenderingArea.top - m_activeRenderingArea.bottom;

	// search for intersection between ray and objects in world
	bool result = m_camera->select(a_windowPosX,
		a_windowPosY,
		width,
		height,
		m_lastSelectedObject,
		m_lastSelectedTriangle,
		m_lastSelectedPoint,
		m_lastSelectedDistance,
		a_selectVisibleObjectsOnly
	);

	// return result. True if and object has been hit, else false.
	return(result);
}


//===========================================================================
/*!
	Set camera. The viewport will now display the image filmed by this
	virtual camera.

	\fn     void cViewport::setCamera(cCamera *a_camera)
	\param  a_camera  Virtual camera in world.
*/
//===========================================================================
void cViewport::setCamera(cCamera *a_camera)
{
	// set camera
	m_camera = a_camera;
}



//===========================================================================
/*!
	You can use this to specify a specific rectangle to which you want this
	viewport to render within the window.  Supply -1 for each coordinate
	to return to the default behavior (rendering to the whole window).
	The _positive_ y axis goes _up_.

	\fn     void cViewport::setRenderArea(RECT& r)
	\param  r  The rendering area within the GL context
*/
//===========================================================================
void cViewport::setRenderArea(RECT& r)
{
	m_forceRenderArea = r;
}



//===========================================================================
/*!
	Clients should call this when the scene associated with
	this viewport may need re-initialization, e.g. after a
	switch to or from fullscreen.  Automatically called from update()
	when the viewport creates a new GL context.

	\fn     void cViewport::onDisplayReset()
*/
//===========================================================================
void cViewport::onDisplayReset()
{
	if (m_camera) m_camera->onDisplayReset(true);
}


//===========================================================================
/*!
	Project a world-space point from 3D to 2D, using my viewport xform, my
	camera's projection matrix, and his world's modelview matrix

	\fn     void cViewport::projectPoint(cVector3d& a_point)
	\param  a_point     The point to transform
	\return             The transformed point in window space
*/
//===========================================================================
cVector3d cViewport::projectPoint(cVector3d& a_point)
{
	cVector3d toReturn;

	int* viewport = m_glViewport;
	double* projection = m_camera->m_projectionMatrix;
	double* modelview = m_camera->getParentWorld()->m_worldModelView;

	int success = ::gluProject(a_point.x, a_point.y, a_point.z, modelview, projection,
		viewport, &toReturn.x, &toReturn.y, &toReturn.z);

	return toReturn;
}