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
#ifndef CViewportH
#define CViewportH
//---------------------------------------------------------------------------
#include <windows.h>
#include <GL/gl.h>
#include "CWorld.h"
#include "CCamera.h"
#include "CMatrix3d.h"
#include "CVector3d.h"

//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \file       CViewport.h
	  \class      cViewport
	  \brief

	  cViewport describes a two-dimensional window for rendering an OpenGL scene.
	  Basically this class encapsulates an OpenGL rendering context.  Creating
	  a window is left to the application programmer, since that will depend
	  on the development environment that you're using.  Once you have a window
	  handle, use this class to bind it to an OpenGL context.

	  Typically a cViewport is connected to a cCamera for rendering, and a cCamera
	  is typically connected to a cWorld, where objects actually live.

*/
//===========================================================================
class cViewport
{
public:

	// CONSTRUCTOR & DESTRUCTOR:

	//! Constructor of cViewport
	cViewport(HWND a_winHandle, cCamera *a_camera, const bool a_stereoEnabled = false, PIXELFORMATDESCRIPTOR* a_pixelFormat = 0);
	//! Destructor of cViewport.
	~cViewport();

	// METHODS:

	//! Get height of active viewport area.
	unsigned int getHeight() const { return (m_activeRenderingArea.top - m_activeRenderingArea.bottom); }
	//! Get width of active viewport area.
	unsigned int getWidth() const { return (m_activeRenderingArea.right - m_activeRenderingArea.left); }

	//! Set the camera through which this viewport will be rendered
	void setCamera(cCamera *a_camera);
	//! Get the camera through which this viewport is being rendered
	cCamera* getCamera() const { return (m_camera); }

	//! Enable or disable rendering of this viewport
	void setEnabled(const bool& a_enabled) { m_enabled = a_enabled; }
	//! Get the rendering status of this viewport
	bool getEnabled() const { return (m_enabled); }

	//! Set post-render callback... the object you supply here will be rendered _after_ all other rendering
	void setPostRenderCallback(cGenericObject* a_postRenderCallback)
	{
		m_postRenderCallback = a_postRenderCallback;
	}

	//! Get post-render callback
	cGenericObject* getPostRenderCallback() const { return (m_postRenderCallback); }

	//! Enable or disable stereo rendering
	void setStereoOn(bool a_stereoEnabled);
	//! Is stereo rendering enabled?
	bool getStereoOn() const { return (m_stereoEnabled); }

	//! Render the scene in OpenGL
	bool render(const int imageIndex = CHAI_STEREO_DEFAULT);

	//! Returns the pixel format used by this viewport
	const PIXELFORMATDESCRIPTOR* getPixelFormat() { return (&m_pixelFormat); }

	//! Tell the viewport to figure out whether the (x,y) viewport coordinate is within a visible object
	bool select(const unsigned int a_windowPosX, const unsigned int a_windowPosY,
		const bool a_selectVisibleObjectsOnly);
	//! Get last selected mesh
	cGenericObject* getLastSelectedObject() { return (m_lastSelectedObject); }
	//! Get last selected triangle.
	cTriangle* getLastSelectedTriangle() { return (m_lastSelectedTriangle); }
	//! Get last selected point position.
	cVector3d getLastSelectedPoint(void) { return(m_lastSelectedPoint); }
	//! Get distance to last selected object.
	double getLastSelectedDistance(void) { return (m_lastSelectedDistance); }

	//! Clients should call this when the scene associated with
	//! this viewport may need re-initialization, e.g. after a 
	//! switch to or from fullscreen.
	virtual void onDisplayReset();

	//! Return a direct handle to the OpenGL viewing context
	HDC getGLDC() { return m_glDC; }

	//! You can use this to specify a specific rectangle to which you want this
	//! viewport to render within the window.  Supply -1 for each coordinate
	//! to return to the default behavior (rendering to the whole window).
	//! The _positive_ y axis goes _up_.
	void setRenderArea(RECT& r);
	void getRenderArea(RECT& r) { r = this->m_forceRenderArea; }

	static cViewport* getLastActiveViewport() { return lastActiveViewport; }

	//! Reconfigures the display context.
	bool update(bool resizeOnly = false);

	// Project a world-space point from 3D to 2D, using my viewport xform, my
	// camera's projection matrix, and his world's modelview matrix
	cVector3d projectPoint(cVector3d& a_point);

	//! It's useful to store the last viewport transformation, for gluProject'ing things
	int m_glViewport[4];

protected:

	// PROPERTIES:

	//!  Virtual camera connected to this viewport.
	cCamera* m_camera;
	//! Status of viewport.
	bool m_enabled;
	//! Stereo status.
	bool m_stereoEnabled;
	//! Last selected mesh.
	cGenericObject* m_lastSelectedObject;
	//! Last selected triangle.
	cTriangle* m_lastSelectedTriangle;
	//! Last selected point.
	cVector3d m_lastSelectedPoint;
	//! Distance between camera and last selected point.
	double m_lastSelectedDistance;
	//! Handle to window display.
	HWND m_winHandle;
	//! OpenGL display context
	HGLRC m_glContext;
	//! display context
	HDC m_glDC;
	//! GL Status
	bool m_glReady;

	//! The rectangle to which we're rendering within the GL window, equal to
	//! the window size by default.  The _positive_ y axis goes _up_.
	RECT m_activeRenderingArea;

	//! If we're forcing rendering to a particular rectangle within the viewport,
	//! this rectangle contains those coordinates.  Otherwise all coordinates are
	//! -1, which tells cViewport to use the whole window.  The _positive_ y axis
	//! goes _up_.
	RECT m_forceRenderArea;

	//! Descriptor of the context opened for OpenGL rendering
	PIXELFORMATDESCRIPTOR m_pixelFormat;

	//! If non-zero, this object will get rendered immediately 
	//! before the GL buffer is swapped out, after all other
	cGenericObject* m_postRenderCallback;

	//! The most recent viewport to initiate rendering; useful
	//! for finding global opengl state information
	static cViewport* lastActiveViewport;

	// METHODS:

	//! Clean up the current rendering context
	bool cleanup();
	//! Render the scene in OpenGL.  Nukes the contents of the GL buffers.
	virtual bool renderView(const int a_imageIndex);

};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

