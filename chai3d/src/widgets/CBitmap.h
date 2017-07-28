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
	\version    1.1
	\date       1/2006
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CBitmapH
#define CBitmapH
//---------------------------------------------------------------------------
#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include "CGenericObject.h"
#include "CImageLoader.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \file       CBitmap.h
	  \class      cBitmap
	  \brief      This class provides functionalities to display a bitmap
				  image
*/
//===========================================================================
class cBitmap : public cGenericObject
{
public:

	// CONSTRUCTOR & DESTRUCTOR:

	//! Constructor of cLabel
	cBitmap();

	//! Destructor of cFont
	virtual ~cBitmap();

	//! Get a pointer to the actual image data... use with care...
	inline unsigned char* getData() { return m_image.getData(); }

	//! Get width of image.
	inline unsigned int getBitmapWidth() { return m_image.getWidth(); }

	//! Get height of image.
	inline unsigned int getBitmapHeight() { return m_image.getHeight(); }

	//! Get the format (GL_RGB or GL_RGBA) of the image
	inline unsigned int getBitmapFormat() { return m_image.getFormat(); }

	//! Set zoom factors
	void setZoomHV(float a_zoomHorizontal, float a_zoomVertical);

	//! Get zoom factor along horizontal axis
	float getZoomH() { return (m_zoomH); }

	//! Get zoom factor along vertical axis
	float getZoomV() { return (m_zoomV); }

	//! enable or disable the use of transparency
	void enableTransparency(bool a_enableTransparency) { m_useTransparency = a_enableTransparency; }

	//! Image loader
	cImageLoader m_image;

private:
	//! zoom factors
	float m_zoomH, m_zoomV;

	//! transparency status
	bool m_useTransparency;

	//! Render texture in OpenGL
	virtual void render(const int a_renderMode = CHAI_RENDER_MODE_RENDER_ALL);
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
