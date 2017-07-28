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
	\date       01/2006
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CBitmap.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  Constructor of cBitmap.

	  \fn       cBitmap::cBitmap()
*/
//===========================================================================
cBitmap::cBitmap()
{
	// initialize zoom factors
	m_zoomH = 1.0;
	m_zoomV = 1.0;

	// do not used transparency
	m_useTransparency = false;
}


//===========================================================================
/*!
	  Destructor of cBitmap.

	  \fn       cBitmap::~cBitmap()
*/
//===========================================================================
cBitmap::~cBitmap()
{
}


//===========================================================================
/*!
	  Render bitmap in OpenGL

	  \fn       void cBitmap::render(const int a_renderMode)
*/
//===========================================================================
void cBitmap::render(const int a_renderMode)
{
	glDisable(GL_LIGHTING);

	// transparency is used
	if (m_useTransparency)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
	}

	// transparency is not used
	else
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	if (m_image.getData() != NULL)
	{
		glRasterPos2i(1, 1);
		glPixelZoom(m_zoomH, m_zoomV);
		glDrawPixels(m_image.getWidth(),
			m_image.getHeight(),
			m_image.getFormat(),
			GL_UNSIGNED_BYTE,
			m_image.getData());
		glPixelZoom(1.0, 1.0);
	}

	// restore OpenGL state
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}


//===========================================================================
/*!
	  Set zoom factors for the horizontal and vertical directions

	  \fn       void cBitmap::setZoomHV(float a_zoomHorizontal, float a_zoomVertical);
	  \param    a_zoomHorizontal Zoom factor along the horizontal direction
	  \param    a_zoomVertical Zoom factor along the vertical direction
*/
//===========================================================================
void cBitmap::setZoomHV(float a_zoomHorizontal, float a_zoomVertical)
{
	m_zoomH = a_zoomHorizontal;
	m_zoomV = a_zoomVertical;
}

