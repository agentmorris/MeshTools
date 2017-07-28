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
#include "CFont.h"
#include "GL/gl.h"
#include <ctype.h>

// Different compilers like slightly different GLUT's 
#ifdef _MSVC
#include "../../external/OpenGL/msvc6/glut.h"
#else
#ifdef _POSIX
#include <GL/glut.h>
#else
#include "../../external/OpenGL/bbcp6/glut.h"
#endif
#endif

#ifdef _WIN32
#include "CViewport.h"
#endif

#include <string.h>

//===========================================================================
/*!
Default constructor for abstract class cFont
\fn         cFont::cFont()
*/
//===========================================================================
cFont::cFont()
{
	setFontFace(CHAI_DEFAULT_FONT_FACE);
	setPointSize(CHAI_DEFAULT_FONT_SIZE);
	memset(m_char_widths, 0, sizeof(m_char_widths));
}


//===========================================================================
/*!
	Use this to obtain an actual, non-virtual font object
	\fn         static cFont* cFont::createFont()
	\return     Returns a pointer to an actual cFont object that you can
				use for rendering.  The caller is responsible for deleting
				this object.  Returns 0 if no font can be constructed.
*/
//===========================================================================
cFont* cFont::createFont()
{
#if (defined(_WIN32) && !defined(_POSIX))
	// Only return a bitmap font if we have an active viewport,
	// implying that we can get a GL context
	if (cViewport::getLastActiveViewport())
		return new cWin32BitmapFont();
	else return new cGLUTBitmapFont();
#else
	return new cGLUTBitmapFont();
#endif
}


//===========================================================================
/*!
	Use this to copy data from an existing font object
	\fn         static cFont* cFont::createFont(const cFont* oldFont);
	\return     Returns a pointer to an actual cFont object that you can
	use for rendering.  The caller is responsible for deleting
	this object.  Returns 0 if no font can be constructed.
*/
//===========================================================================
cFont* cFont::createFont(const cFont* oldFont)
{
	// Create a generic font
	cFont* toReturn = createFont();

	// Make sure that went ok...
	if (toReturn == 0) return 0;
	else if (oldFont == 0) return toReturn;

	// Copy relevant data to the new font object...
	toReturn->m_pointSize = oldFont->m_pointSize;
	strcpy(toReturn->m_fontFace, oldFont->m_fontFace);
	return toReturn;
}


//===========================================================================
/*!
	Get the width of a particular character
	\fn         int cFont::getCharacterWidth(const unsigned char& a_char)
	\return     Returns the width of this character in pixels, or -1 for an error
	\param      a_char    The character (e.g. 'a') to look up.  Characters less than
						  32 ('A') will generate errors.
*/
//===========================================================================
int cFont::getCharacterWidth(const unsigned char& a_char)
{
	if (a_char < 32) return -1;
	int w = m_char_widths[a_char - 32];
	return w;
}


//===========================================================================
/*!
	Change the font face, possibly marking the font for re-initialization
	\fn         void cFont::setFontFace(const char* a_faceName)
	\param      a_faceName The new face name
*/
//===========================================================================
void cFont::setFontFace(const char* a_faceName)
{
	strncpy(m_fontFace, a_faceName, 255);
	int len = strlen(m_fontFace);

	// Convert to lowercase
	for (int i = 0; i < len; i++) m_fontFace[i] = tolower(m_fontFace[i]);
}


// Constants needed for GLUT fonts
void* glut_bitmap_fonts[7] = {
	GLUT_BITMAP_9_BY_15,
	GLUT_BITMAP_8_BY_13,
	GLUT_BITMAP_TIMES_ROMAN_10,
	GLUT_BITMAP_TIMES_ROMAN_24,
	GLUT_BITMAP_HELVETICA_10,
	GLUT_BITMAP_HELVETICA_12,
	GLUT_BITMAP_HELVETICA_18
};

const char* glut_bitmap_font_names[7] = {
	"GLUT_BITMAP_9_BY_15",
	"GLUT_BITMAP_8_BY_13",
	"GLUT_BITMAP_TIMES_ROMAN_10",
	"GLUT_BITMAP_TIMES_ROMAN_24",
	"GLUT_BITMAP_HELVETICA_10",
	"GLUT_BITMAP_HELVETICA_12",
	"GLUT_BITMAP_HELVETICA_18"
};

typedef enum {
	GLUT_FONT_FAMILY_COURIER = 0,
	GLUT_FONT_FAMILY_TIMES,
	GLUT_FONT_FAMILY_HELVETICA
} glut_font_families;

void* glut_stroke_fonts[2] = {
	GLUT_STROKE_ROMAN,
	GLUT_STROKE_MONO_ROMAN
};

const char* glut_stroke_font_names[2] = {
	"GLUT_STROKE_ROMAN",
	"GLUT_STROKE_MONO_ROMAN"
};

//===========================================================================
/*!
	Return the index of the current font in the table of font name
	\fn         int cGLUTBitmapFont::getBestFontMatch()
	\return     -1 for error or an index into glut_bitmap_fonts
*/
//===========================================================================
int cGLUTBitmapFont::getBestFontMatch()
{
	int toReturn = -1;

	int family = 0;

	// First, which font family are we?
	if (strstr(m_fontFace, "times"))          family = GLUT_FONT_FAMILY_TIMES;
	else if (strstr(m_fontFace, "roman"))     family = GLUT_FONT_FAMILY_TIMES;
	else if (strstr(m_fontFace, "helvetica")) family = GLUT_FONT_FAMILY_HELVETICA;
	else if (strstr(m_fontFace, "arial"))     family = GLUT_FONT_FAMILY_HELVETICA;
	else if (strstr(m_fontFace, "courier"))   family = GLUT_FONT_FAMILY_COURIER;
	else                                     family = GLUT_FONT_FAMILY_HELVETICA;

	// Now what size?
	switch (family)
	{
	case GLUT_FONT_FAMILY_HELVETICA:

		if (m_pointSize <= 10) toReturn = 4;
		else if (m_pointSize <= 15) toReturn = 5;
		else toReturn = 6;
		break;

	case GLUT_FONT_FAMILY_TIMES:

		if (m_pointSize <= 16) toReturn = 2;
		else toReturn = 3;
		break;

	case GLUT_FONT_FAMILY_COURIER:

		if (m_pointSize <= 14) toReturn = 0;
		else toReturn = 1;
		break;

	default:
		toReturn = 0;
		break;
	}

	return toReturn;
}


//===========================================================================
/*!
	Renders an actual string of text, initializing the font if necessary.
	\fn         int cGLUTBitmapFont::renderString(const char* a_str)
	\param      a_str The string to render; should not include newlines
	\return     0 for success, -1 for error
*/
//===========================================================================
int cGLUTBitmapFont::renderString(const char* a_str)
{
	int index = getBestFontMatch();
	while (*a_str)
	{
		glutBitmapCharacter(glut_bitmap_fonts[index], *a_str);
		a_str++;
	}
	glGetError();
	return 0;
}

//===========================================================================
/*!
  Get the width of a particular character
  \fn         int cGLUTBitmapFont::getCharacterWidth(const unsigned char& a_char)
  \return     Returns the width of this character in pixels, or -1 for an error
  \param      a_char    The character (e.g. 'a') to look up.  Characters less than
			  32 ('A') will generate errors.
*/
//===========================================================================
int cGLUTBitmapFont::getCharacterWidth(const unsigned char& a_char)
{
	int fontIndex = getBestFontMatch();
	return glutBitmapWidth(glut_bitmap_fonts[fontIndex], a_char);
}


#if (defined(_WIN32) & !defined(_POSIX))

//===========================================================================
/*!
	Constructor for cWin32BitmapFont
	\fn         cWin32BitmapFont::cWin32BitmapFont()
*/
//===========================================================================
cWin32BitmapFont::cWin32BitmapFont()
{
	m_logfont.lfHeight = (long)(-1.0*m_pointSize);
	m_logfont.lfWidth = 0;
	m_logfont.lfEscapement = 0;
	m_logfont.lfOrientation = 0;
	m_logfont.lfWeight = FW_NORMAL;
	m_logfont.lfItalic = FALSE;
	m_logfont.lfUnderline = FALSE;
	m_logfont.lfStrikeOut = FALSE;
	m_logfont.lfCharSet = ANSI_CHARSET;
	m_logfont.lfOutPrecision = OUT_TT_PRECIS;
	m_logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	m_logfont.lfQuality = ANTIALIASED_QUALITY;
	m_logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strncpy(m_logfont.lfFaceName, m_fontFace, 32);

	// Font is initially uninitialized
	m_bitmap_font_base = -1;

	m_solidFont = true;

	m_outlineFontDeviation = 0.0f;
	m_outlineFontExtrusion = 0.0f;
	m_usePolygonsForOutlineFonts = true;
}


//===========================================================================
/*!
  Destructor for cWin32BitmapFont
  \fn         cWin32BitmapFont::~cWin32BitmapFont()
*/
//===========================================================================
cWin32BitmapFont::~cWin32BitmapFont()
{
	uninitialize();
}


//===========================================================================
/*!
	Renders an actual string of text, initializing the font if necessary.
	\fn         int cWin32BitmapFont::renderString(const char* a_str)
	\param      a_str The string to render; should not include newlines
	\return     0 for success, -1 for error
*/
//===========================================================================
int cWin32BitmapFont::renderString(const char* a_str)
{
	// Make sure our font is initialized
	if (m_bitmap_font_base == -1) initialize();

	// Check for initialization errors
	if (m_bitmap_font_base == -1) return -1;

	// Back up the display list bits
	glPushAttrib(GL_LIST_BIT);

	// Set the base character to 32
	glListBase(m_bitmap_font_base - 32);

	// Render the text
	glCallLists(strlen(a_str), GL_UNSIGNED_BYTE, a_str);

	// Pop the display list bits
	glPopAttrib();

	return 0;
}


//===========================================================================
/*!
	Change the font face, possibly marking the font for re-initialization
	\fn         void cWin32BitmapFont::setFontFace(const char* a_faceName)
	\param      a_faceName The new face name
*/
//===========================================================================
void cWin32BitmapFont::setFontFace(const char* a_faceName)
{
	// Do we need to re-initialize our font?
	if (m_bitmap_font_base != -1) uninitialize();

	strncpy(m_logfont.lfFaceName, a_faceName, 32);
	cFont::setFontFace(a_faceName);
}


//===========================================================================
/*!
	Change the font size, possibly marking the font for re-initialization
	\fn         void cWin32BitmapFont::setPointSize(const float& a_pointSize)
	\param      a_pointSize The new font size
*/
//===========================================================================
void cWin32BitmapFont::setPointSize(const float& a_pointSize)
{
	// Do we need to re-initialize our font?
	if (m_bitmap_font_base != -1) uninitialize();
	cFont::setPointSize(a_pointSize);
}


//===========================================================================
/*!
	Initialize the internal font representation
	\fn         int cWin32BitmapFont::initialize()
	\return     0 for success, -1 for error
*/
//===========================================================================
int cWin32BitmapFont::initialize()
{
	// If there is a current gl error, the font won't build correctly,
	// so we clear the error here.
	glGetError();

	// Necessary to correctly built textured fonts
	glDisable(GL_TEXTURE_2D);
	glLineWidth(1.0);

	if (m_bitmap_font_base != -1) uninitialize();

	HDC hdc;

	cViewport* viewport = cViewport::getLastActiveViewport();
	hdc = viewport->getGLDC();

	// Convert from point size to "log height"
	int logheight = -MulDiv((int)m_pointSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	m_logfont.lfHeight = logheight;

	HFONT	font;
	HFONT	oldfont;

	// Generate characters for our font
	m_bitmap_font_base = glGenLists(96);

	// Create a windows font
	font = CreateFontIndirect(&m_logfont);

	if (font == NULL) return -1;

	// Select the font we want
	oldfont = (HFONT)SelectObject(hdc, font);

	// Build 96 characters starting at character 32 ('A')
	bool result;

	if (m_solidFont)
	{
		result = (bool)(wglUseFontBitmaps(hdc, 32, 96, m_bitmap_font_base));
	}

	else
	{
		result = (bool)(wglUseFontOutlines(hdc, 32, 96, m_bitmap_font_base,
			m_outlineFontDeviation, m_outlineFontExtrusion,
			m_usePolygonsForOutlineFonts ? WGL_FONT_POLYGONS : WGL_FONT_LINES, 0));
	}

	if (result == false)
	{
		/*
		// Useful for debugging font problems...
		DWORD err = GetLastError();
		char buf[1000];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,err,0,buf,1000,0);
		if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
		if (buf[strlen(buf)-1] == '\r') buf[strlen(buf)-1] = '\0';
		_cprintf("Error %d (%s) creating GL font bitmaps...\n",err,buf);
		*/
		return -1;
	}

	// Try first as a non-truettype font...
	if (!GetCharWidth32(hdc, 32, 96 + 32, m_char_widths))
	{

		// Try a as truetype font...
		ABC abcs[98];

		if (GetCharABCWidths(hdc, 32, 96 + 32 + 1, abcs))
		{
			unsigned int i;
			for (i = 32; i <= 97; i++)
			{
				m_char_widths[i - 32] = abcs[i].abcA + abcs[i].abcB + abcs[i].abcC;
			}
		}
	}

	// Restore the global font state
	SelectObject(hdc, oldfont);
	DeleteObject(font);

	return 0;
}


//===========================================================================
/*!
	Clean up a win32 bitmapped font
	\fn         int cWin32BitmapFont::uninitialize()
	\return     0 for success, -1 for error
*/
//===========================================================================
int cWin32BitmapFont::uninitialize()
{
	if (m_bitmap_font_base != -1) {
		glDeleteLists(m_bitmap_font_base, 96);
		m_bitmap_font_base = -1;
	}
	memset(m_char_widths, 0, sizeof(m_char_widths));
	return 0;
}

//===========================================================================
/*!
	Get the width of a particular character
	\fn         int cWin32BitmapFont::getCharacterWidth(const unsigned char& a_char)
	\return     Returns the width of this character in pixels, or -1 for an error
	\param      a_char    The character (e.g. 'a') to look up.  Characters less than
						  32 ('A') will generate errors.
*/
//===========================================================================
int cWin32BitmapFont::getCharacterWidth(const unsigned char& a_char)
{
	if (m_bitmap_font_base == -1) initialize();
	if (m_bitmap_font_base == -1) return -1;
	if (a_char < 32) return -1;
	int w = m_char_widths[a_char - 32];
	return w;
}

#endif // _WIN32
