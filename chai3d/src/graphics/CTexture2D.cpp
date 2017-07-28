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
#include "CTexture2D.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	A texture contains a 2D bitmap which can be projected onto the
	polygons of a 3D solid.

	\fn         cTexture2D::cTexture2D()
	\return     Return a pointer to new texture instance.
*/
//===========================================================================
cTexture2D::cTexture2D()
{
	// initialize internal variables
	reset();
}


//===========================================================================
/*!
	Destructor of cTexture2D.

	\fn         cTexture2D::~cTexture2D()
*/
//===========================================================================
cTexture2D::~cTexture2D()
{
	if (m_textureID != (unsigned int)-1)
	{
		glDeleteTextures(1, &m_textureID);
		m_textureID = (unsigned int)-1;
	}
}


//===========================================================================
/*!
	Reset internal variables. This function should be called only by constructors.

	\fn         void cTexture2D::reset()
*/
//===========================================================================
void cTexture2D::reset()
{
	// id number provided by OpenGL once texture is stored in graphics
	// card memory
	m_textureID = (unsigned int)-1;

	// texture has not yet been rendered
	m_updateTextureFlag = true;

	// Tile the texture in X. (GL_REPEAT or GL_CLAMP)
	m_wrapSmode = GL_REPEAT;

	// Tile the texture in Y. (GL_REPEAT or GL_CLAMP)
	m_wrapTmode = GL_REPEAT;

	// set the magnification function. (GL_NEAREST or GL_LINEAR)
	m_magnificationFunction = GL_NEAREST;

	// set the minifying function. (GL_NEAREST or GL_LINEAR)
	m_minifyingFunction = GL_NEAREST;

	// set environmental mode (GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE)
	m_environmentMode = GL_MODULATE;

	// set environmental color
	m_color.set(1.0, 1.0, 1.0, 0.0);

	// set spherical mode
	m_useSphericalMapping = false;

	// use mipmaps
	m_useMipmaps = true;
}


//===========================================================================
/*!
	  Enable texturing and set this texture as the current texture

	  \fn         void cTexture2D::render()
*/
//===========================================================================
void cTexture2D::render()
{
	if (m_image.initialized() == 0) return;

	// Only check residency in memory if we weren't going to
	// update the texture anyway...
	if (m_updateTextureFlag == 0)
	{
		GLboolean texture_is_resident;
		glAreTexturesResident(1, &m_textureID, &texture_is_resident);

		if (texture_is_resident == false)
		{
			m_updateTextureFlag = true;
		}
	}

	// is texture being rendered for the first time?
	if (m_updateTextureFlag)
	{
		update();
		m_updateTextureFlag = false;
	}

	// enable texturing
	glEnable(GL_TEXTURE_2D);

	// enable or disable spherical mapping
	if (m_useSphericalMapping)
	{
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	}
	else
	{
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}

	// Sets the wrap parameter for texture coordinate s to either
	// GL_CLAMP or GL_REPEAT.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapSmode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapTmode);

	// Set the texture magnification function to either GL_NEAREST or GL_LINEAR.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magnificationFunction);

	// Set the texture minifying function to either GL_NEAREST or GL_LINEAR.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minifyingFunction);

	// set the environment mode (GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_environmentMode);

	// make this the current texture
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	// set the environmental color
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &m_color.pColor()[0]);

}


//===========================================================================
/*!
	  Load an image file (CHAI currently supports 24-bit .bmp and
	  32-bit .tga files on all platforms and should support all formats
	  on Windows)

	  \fn         bool cTexture2D::loadFromFile(const char* a_fileName)
*/
//===========================================================================
bool cTexture2D::loadFromFile(const char* a_fileName)
{
	if (m_image.loadFromFile(a_fileName) < 0)
	{
		// Failure
		return false;
	}
	else
	{
		// Success
		return true;
	}
}


//===========================================================================
/*!
	  Generate texture from memory data, to prepare for rendering.

	  \fn         void cTexture2D::update()
*/
//===========================================================================
void cTexture2D::update()
{
	if (m_textureID != (unsigned int)-1)
	{
		// Deletion makes for all kinds of new hassles, particularly
		// when re-initializing a whole display context, since opengl
		// automatically starts re-assigning texture ID's.  Not worth it.
		// glDeleteTextures(1,&m_textureID);

		m_textureID = (unsigned int)-1;
	}

	// Generate a texture ID and bind to it
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	if (m_useMipmaps)
	{
		int components = (m_image.getFormat() == GL_RGB ? 3 : 4);

		gluBuild2DMipmaps(GL_TEXTURE_2D,
			components,
			m_image.getWidth(),
			m_image.getHeight(),
			m_image.getFormat(),
			GL_UNSIGNED_BYTE,
			m_image.getData()
		);
	}

	else
	{
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);

		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			m_image.getWidth(),
			m_image.getHeight(),
			0,
			m_image.getFormat(),
			GL_UNSIGNED_BYTE,
			m_image.getData()
		);
	}
}


//===========================================================================
/*!
	  Sets the wrap parameter for texture coordinate s to either GL_CLAMP or
	  GL_REPEAT. GL_CLAMP causes s coordinates to be clamped to the
	  range [0,1] and is useful for preventing wrapping artifacts when mapping
	  a single image onto an object. GL_REPEAT causes the integer part of the
	  s coordinate to be ignored; OpenGL uses only the fractional part, thereby
	  creating a repeating pattern. Border texture elements are accessed only
	  if wrapping is set to GL_CLAMP. Initially, GL_TEXTURE_WRAP_S is set
	  to GL_REPEAT.

	  \fn       void cTexture2D::setWrapMode(const GLint& a_wrapSmode, const GLint& a_wrapTmode)
	  \param    a_wrapSmode  value shall be either GL_REPEAT or GL_CLAMP
	  \param    a_wrapTmode  value shall be either GL_REPEAT or GL_CLAMP
*/
//===========================================================================
void cTexture2D::setWrapMode(const GLint& a_wrapSmode, const GLint& a_wrapTmode)
{
	m_wrapSmode = a_wrapSmode;
	m_wrapTmode = a_wrapTmode;
}


//===========================================================================
/*!
	The texture magnification function is used when the pixel being textured
	maps to an area less than or equal to one texture element.
	It sets the texture magnification function to either GL_NEAREST or GL_LINEAR.

	\fn       void cTexture2D::setMagnificationFunction(GLint a_magnificationFunction)
	\param    a_magnificationFunction  value shall be either GL_NEAREST or GL_LINEAR.
*/
//==========================================================================
void cTexture2D::setMagnificationFunction(GLint a_magnificationFunction)
{
	m_magnificationFunction = a_magnificationFunction;
}


//===========================================================================
/*!
	The texture minifying function is used whenever the pixel being textured
	maps to an area greater than one texture element. There are six defined
	minifying functions. Two of them use the nearest one or nearest four
	texture elements to compute the texture value. The other four use mipmaps.
	A mipmap is an ordered set of arrays representing the same image at
	progressively lower resolutions. If the texture has dimensions 2nx2m
	there are max(n, m) + 1 mipmaps. The first mipmap is the original texture,
	with dimensions 2nx2m. Each subsequent mipmap has dimensions 2k1x2l1 where 2
	kx2l are the dimensions of the previous mipmap, until either k = 0 or l = 0.
	At that point, subsequent mipmaps have dimension 1x2l1 or 2k1x1 until the
	final mipmap, which has dimension 1x1. Mipmaps are defined using
	glTexImage1D or glTexImage2D with the level-of-detail argument indicating
	the order of the mipmaps. Level 0 is the original texture; level bold
	max(n, m) is the final 1x1 mipmap.

	\fn       void cTexture2D::setMinifyingFunction(GLint a_minifyingFunction);
	\param    a_minifyingFunction  value shall be either GL_NEAREST or GL_LINEAR.
*/
//==========================================================================
void cTexture2D::setMinifyingFunction(GLint a_minifyingFunction)
{
	m_minifyingFunction = a_minifyingFunction;
}
