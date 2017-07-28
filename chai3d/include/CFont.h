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
    \date       12/2005
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CFontH
#define CFontH

#include <stdlib.h>
#include <string.h>

#if (defined(_WIN32) && !defined(_POSIX))
#include <windows.h>
#endif

//---------------------------------------------------------------------------

#define CHAI_DEFAULT_FONT_FACE "Arial"
#define CHAI_DEFAULT_FONT_SIZE 12.0f

//===========================================================================
/*!
    \file CFont.h
    \class      cFont
    \brief      cFont is a generic and pure virtual font interface, to be
                subclassed by platform-specific implementations.  For the
                simplest, most portable approach, use this class and the static
                method createFont", which returns an actual font object.  You may
                also create subclass font types directly (see below).

                Specific implementations can be found later in this file.
*/
//===========================================================================
class cFont
{

public:

    //! Use this to obtain an actual, non-virtual font object
    static cFont* createFont();

    //! Use this to copy data from an existing font object
    static cFont* createFont(const cFont* oldFont);

    //! Renders a single-line string
    virtual int renderString(const char* a_str)=0;

    //! Change the font face; may require re-initializing the font
    virtual void setFontFace(const char* a_faceName);
    //! Get the current font face
    virtual void getFontFace(char* a_faceName) const { strcpy(a_faceName,m_fontFace); }

    //! Change the font size; may require re-initializing the font
    virtual void setPointSize(const float& a_pointSize) { m_pointSize = a_pointSize; }
    //! Get the current font size
    virtual float getPointSize() const { return m_pointSize; }

    //! Constructor
    cFont();

    //! Destructor
    virtual ~cFont() { }

    //! Get the width of a particular character
    virtual int getCharacterWidth(const unsigned char& a_char);

  protected:
    //! The point size of the font
    float m_pointSize;
    //! The font face name
    char m_fontFace[255]; 
    //! The width of each character in our font
    int m_char_widths[255];
};


class cGLUTBitmapFont : public cFont
{

public:

  //! Renders a string, should not contain any newlines
  //!
  //! Returns 0 for success, -1 for error
  virtual int renderString(const char* a_str);

  //! Constructor
  cGLUTBitmapFont() { }

  //! Destructor
  virtual ~cGLUTBitmapFont() { }

  //! Get the width of a particular character
  virtual int getCharacterWidth(const unsigned char& a_char);

protected:

  // Return the index of the current font in the table of font names
  int getBestFontMatch();
};


#if (defined(_WIN32) & !defined(_POSIX))

//===========================================================================
/*!
    \class      cWin32BitmapFont
    \brief      A 2D, texture-based, win32-specific implementation of cFont
*/
//===========================================================================
class cWin32BitmapFont : public cFont
{

public:

    //! Renders a string, optionally a string w/embedded printf specifiers
    //!
    //! Returns 0 for success, -1 for error
    virtual int renderString(const char* a_str);

    //! Change the font face; may require re-initializing the font
    virtual void setFontFace(const char* a_faceName);

    //! Change the font size; may require re-initializing the font
    virtual void setPointSize(const float& a_pointSize);

    //! Constructor
    cWin32BitmapFont();

    //! Destructor
    virtual ~cWin32BitmapFont();

    //! Used to access win32 font information directly; use with care and be aware
    //! that the font may need reinitialization if you modify options _after_ the
    //! font is used for rendering.
    virtual LOGFONT* getLogFont() { return &m_logfont; }

    //! If you want an outline font instead of a solid font, set this to
    //! false before using the font for rendering.
    bool m_solidFont;

    //! Get the width of a particular character
    virtual int getCharacterWidth(const unsigned char& a_char);

protected:
    
    // The base openGL display list used for our font, or -1 if we're uninitialized
    int m_bitmap_font_base;

    //! Information about the actual win32 font
    LOGFONT m_logfont;

    //! Should be called with an active rendering context; returns 0 for success, -1 for error
    int initialize();

    //! Clean up
    int uninitialize();

    //! Parameters relevant to outline fonts only
    float m_outlineFontDeviation, m_outlineFontExtrusion;
    bool m_usePolygonsForOutlineFonts;
};

#endif // _WIN32

#endif
