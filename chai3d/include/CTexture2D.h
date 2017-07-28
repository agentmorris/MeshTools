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
#ifndef CTexture2DH
#define CTexture2DH
//---------------------------------------------------------------------------
#include "CImageLoader.h"
#include "CColor.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glu.h>
#include <GL/gl.h>
#include <string>
#include <stdio.h>
//---------------------------------------------------------------------------

//===========================================================================
/*!
      \file       CTexture2D.h
      \class      cTexture2D
      \brief      cTexture2D describes a 2D bitmap texture used for OpenGL
                  texture-mapping
*/
//===========================================================================
class cTexture2D
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cTexture2D.
    cTexture2D();

    //! Destructor of cTexture2D.
    ~cTexture2D();

    //! Load an image file (CHAI currently supports 24-bit .bmp and 32-bit .tga files)
    bool loadFromFile(const char* a_fileName);

    //! Enable texturing and set this texture as the current texture
    void render();

    //! Call this to force texture re-initialization
    void markForUpdate() { m_updateTextureFlag = true; }

    //! Set the environment mode (GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE, or -1 for "don't set")
    void setEnvironmentMode(const GLint& a_environmentMode) { m_environmentMode = a_environmentMode; }

    //! Get the environment mode status
    GLint getEnvironmentMode() { return (m_environmentMode); }

    //! Set the texture wrap mode
    void setWrapMode(const GLint& a_wrapSmode, const GLint& a_wrapTmode);

    //! Get the texture wrap mode of S
    GLint getWrapSmode() { return (m_wrapSmode); }

    //! Get the texture wrap mode of T
    GLint getWrapTmode() { return (m_wrapSmode); }

    //! set the magnification function
    void setMagnificationFunction(GLint a_magnificationFunction);

    //! get current magnification function
    GLint getMagnificationFunction() { return (m_magnificationFunction); }

    //! set the minification function
    void setMinifyingFunction(GLint a_minifyingFunction);

    //! get current magnification function
    GLint getMinifyingFunction() { return (m_minifyingFunction); }

    //! set spherical mapping mode ON or OFF
    void setSphericalMappingEnabled(bool a_enabled) { m_useSphericalMapping = a_enabled; }

    // get the status of the spherical mapping mode
    bool getSphericalMappingEnabled() { return (m_useSphericalMapping); }

    //! Image loader (use this to get data about the texture itself)
    cImageLoader m_image;

    //! Environmental color
    cColorf m_color;

  private:
    // METHODS:

    //! Reset internal variables. This function should be called only by constructors.
    void reset();

    //! Initialize GL texture
    void update();

    // MEMBERS:

    //! If \b true, texture bitmap has not yet been sent to video card.
    bool m_updateTextureFlag;

    //! OpenGL texture ID number.
    GLuint m_textureID;

    //! texture wrap parameter along S and T (GL_REPEAT or GL_CLAMP)
    GLint m_wrapSmode;
    GLint m_wrapTmode;

    //! texture magnification function. (GL_NEAREST or GL_LINEAR)
    GLint m_magnificationFunction;

    //! texture minifying function. (GL_NEAREST or GL_LINEAR)
    GLint m_minifyingFunction;

    //! If \b true, we use GLU to build mipmaps.
    bool m_useMipmaps;

    //! If \b true, we use spherical mapping.
    bool m_useSphericalMapping;

    //! OpenGL texture mode (GL_MODULATE, GL_DECAL, GL_BLEND, GL_REPLACE)
    GLint m_environmentMode;
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

