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
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CColorH
#define CColorH
//---------------------------------------------------------------------------
#include "CMaths.h"
#ifdef _WIN32
#include "windows.h"
#endif

#include <GL/gl.h>

/*!
    \file CColor.h
*/

struct cColorb;
struct cColorf;

//---------------------------------------------------------------------------

//===========================================================================
/*!
      \struct   cColorf
      \brief    cColorf describes a color composed of 4 \e GLfloats.
*/
//===========================================================================
struct cColorf
{
  public:

    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------
    /*!
        Default constructor of cColorf (color defaults to opaque white)
    */
    //-----------------------------------------------------------------------
    cColorf()
    {
        // initialize color components R,G,B,A
        m_color[0] = 1.0;
        m_color[1] = 1.0;
        m_color[2] = 1.0;
        m_color[3] = 1.0;
    }


    //-----------------------------------------------------------------------
    /*!
        Constructor of cColorf. Define a color by passing its RGBA components
        as parameters.

        \param    a_red    Red component
        \param    a_green  Green component
        \param    a_blue   Blue component
        \param    a_alpha  Alpha component
    */
    //-----------------------------------------------------------------------
    cColorf(const GLfloat a_red, const GLfloat a_green, const GLfloat a_blue,
            const GLfloat a_alpha = 1.0)
    {
        m_color[0] = cClamp( a_red,   0.0f, 1.0f);
        m_color[1] = cClamp( a_green, 0.0f, 1.0f);
        m_color[2] = cClamp( a_blue,  0.0f, 1.0f);
        m_color[3] = cClamp( a_alpha, 0.0f, 1.0f);
    };


    //-----------------------------------------------------------------------
    /*!
        Destructor of cColorf.
    */
    //-----------------------------------------------------------------------
    ~cColorf() {};


    // METHODS:
    //-----------------------------------------------------------------------
    /*!
        Set a color by passing its RGBA components as parameters.

        \param    a_red    Red component
        \param    a_green  Green component
        \param    a_blue   Blue component
        \param    a_alpha  Alpha component
    */
    //-----------------------------------------------------------------------
    inline void set(const GLfloat a_red, const GLfloat a_green, const GLfloat a_blue,
                    const GLfloat a_alpha = 1.0)
    {
        m_color[0] = cClamp( a_red,   0.0f, 1.0f);
        m_color[1] = cClamp( a_green, 0.0f, 1.0f);
        m_color[2] = cClamp( a_blue,  0.0f, 1.0f);
        m_color[3] = cClamp( a_alpha, 0.0f, 1.0f);
    };


    //-----------------------------------------------------------------------
    /*!
        Set color by copying three \e GLfloats from an external array, each
        describing one of the RGB components. Alpha is set to \e 1.0

        \param    a_colorRGB  Pointer to an array of type \e float.
    */
    //-----------------------------------------------------------------------
    inline void setMem3(const GLfloat* a_colorRGB)
    {
        m_color[0] = a_colorRGB[0];
        m_color[1] = a_colorRGB[1];
        m_color[2] = a_colorRGB[2];
        m_color[3] = 1.0;
    }


    //-----------------------------------------------------------------------
    /*!
        Set color by copying four \e GLfloats from an external array, each
        describing one of the RGBA components.

        \param    a_colorRGBA  Pointer to an array of type \e float.
    */
    //-----------------------------------------------------------------------
    inline void setMem4(const GLfloat* a_colorRGBA)
    {
        m_color[0] = a_colorRGBA[0];
        m_color[1] = a_colorRGBA[1];
        m_color[2] = a_colorRGBA[2];
        m_color[3] = a_colorRGBA[3];
    }

    
    //-----------------------------------------------------------------------
    /*!
          Access the nth component of this color (we provide both const
          and non-const versions so you can use this operator as an l-value
          or an r-value)
    */
    //-----------------------------------------------------------------------
    inline GLfloat operator[](const unsigned int n) const
    {
        if (n<4) return m_color[n];
        else return 0.0f;
    }
    inline GLfloat& operator[](const unsigned int n)
    {
        if (n<4) return m_color[n];
        else return m_color[0];
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e red component.          
    */
    //-----------------------------------------------------------------------
    inline void setR(const GLfloat a_red)
    {
        m_color[0] = cClamp( a_red, 0.0f, 1.0f);
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e red component.
    */
    //-----------------------------------------------------------------------
    inline GLfloat getR() const
    {
        return(m_color[0]);
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e green component.          
    */
    //-----------------------------------------------------------------------
    inline void setG(const GLfloat a_green)
    {
        m_color[1] = cClamp( a_green, 0.0f, 1.0f);
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e green component.
    */
    //-----------------------------------------------------------------------
    inline GLfloat getG() const
    {
        return(m_color[1]);
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e blue component.

          \param    a_blue  Blue component.
    */
    //-----------------------------------------------------------------------
    inline void setB(const GLfloat a_blue)
    {
        m_color[2] = cClamp( a_blue, 0.0f, 1.0f);
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e blue component.
    */
    //-----------------------------------------------------------------------
    inline GLfloat getB() const
    {
        return(m_color[2]);
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e alpha component.          
    */
    //-----------------------------------------------------------------------
    inline void setA(const GLfloat a_alpha)
    {
        m_color[3] = cClamp( a_alpha, 0.0f, 1.0f);
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e alpha component.
    */
    //-----------------------------------------------------------------------
    inline GLfloat getA() const
    {
        return(m_color[3]);
    }


    //-----------------------------------------------------------------------
    /*!
          Render this color in OpenGL (sets it to be the currnent color)

          Does not confirm that GL color-tracking is enabled
    */
    //-----------------------------------------------------------------------
    inline void render() const
    {
        glColor4fv(&m_color[0]);
    };


    //-----------------------------------------------------------------------
    /*!
          Returns a pointer to the raw color array
    */
    //-----------------------------------------------------------------------
    inline const GLfloat* pColor() const
    {
        return (&m_color[0]);
    }


    //-----------------------------------------------------------------------
    /*!
          Returns this color, converted to \e cColorb format.

          This method is the only method defined outside the class
          definition, to avoid a recursive referencing problem with the
          cColorb class.
    */
    //-----------------------------------------------------------------------
    inline cColorb getColorb() const;   

  public:
    // MEMBERS:
    //! Color in \e GLfloat format [R,G,B,A].
    GLfloat m_color[4];
};


//===========================================================================
/*!
      \struct   cColorb
      \brief    cColorb describes a color composed of 4 \e bytes.
*/
//===========================================================================
struct cColorb
{
  public:

    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------
    /*!
        Constructor of cColorb.
    */
    //-----------------------------------------------------------------------
    cColorb()
    {
        // initialize color components R,G,B,A
        m_color[0] = 0xff;
        m_color[1] = 0xff;
        m_color[2] = 0xff;
        m_color[3] = 0xff;
    }


    //-----------------------------------------------------------------------
    /*!
        Constructor of cColorb. Define a color by passing its RGBA components.
        as parameters.

        \param    a_red    Red component.
        \param    a_green  Green component.
        \param    a_blue   Blue component.
        \param    a_alpha  Alpha component.
    */
    //-----------------------------------------------------------------------
    cColorb(const GLubyte a_red, const GLubyte a_green, const GLubyte a_blue,
            const GLubyte a_alpha = 0xff)
    {
        m_color[0] = a_red;
        m_color[1] = a_green;
        m_color[2] = a_blue;
        m_color[3] = a_alpha;
    };


    //-----------------------------------------------------------------------
    /*!
        Destructor of cColorf.
    */
    //-----------------------------------------------------------------------
    ~cColorb() {};


    // METHODS:
    //-----------------------------------------------------------------------
    /*!
        Set a color by passing its RGBA components as parameters.

        \param    a_red    Red component.
        \param    a_green  Green component.
        \param    a_blue   Blue component.
        \param    a_alpha  Alpha component.
    */
    //-----------------------------------------------------------------------
    inline void set(const GLubyte a_red, const GLubyte a_green, const GLubyte a_blue,
                    const GLubyte a_alpha = 0xff)
    {
        m_color[0] = a_red;
        m_color[1] = a_green;
        m_color[2] = a_blue;
        m_color[3] = a_alpha;
    };


    //-----------------------------------------------------------------------
    /*!
        Set color by copying three floats from an external array, each
        describing an RGB component. Alpha is set to \e 0xff.

        \param    a_colorRGB  Pointer to array of type \e GLubyte.
    */
    //-----------------------------------------------------------------------
    inline void setMem3(const GLubyte* a_colorRGB)
    {
        m_color[0] = a_colorRGB[0];
        m_color[1] = a_colorRGB[1];
        m_color[2] = a_colorRGB[2];
        m_color[3] = 0xff;
    }


    //-----------------------------------------------------------------------
    /*!
        Set color by copying four floats from an external array, each
        describing an RGBA component

        \param    a_colorRGBA  Pointer to an array of type \e GLubyte
    */
    //-----------------------------------------------------------------------
    inline void setMem4(const GLubyte* a_colorRGBA)
    {
        m_color[0] = a_colorRGBA[0];
        m_color[1] = a_colorRGBA[1];
        m_color[2] = a_colorRGBA[2];
        m_color[3] = a_colorRGBA[3];
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e red component.

          \param    a_red  Red component
    */
    //-----------------------------------------------------------------------
    inline void setR(const GLubyte a_red)
    {
        m_color[0] = a_red;
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e red component
    */
    //-----------------------------------------------------------------------
    inline GLfloat getR() const
    {
        return(m_color[0]);
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e green component

          \param    a_green  Green component
    */
    //-----------------------------------------------------------------------
    inline void setG(const GLubyte a_green)
    {
        m_color[1] = a_green;
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e green component
    */
    //-----------------------------------------------------------------------
    inline GLfloat getG() const
    {
        return(m_color[1]);
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e blue component

          \param    a_blue  Blue component
    */
    //-----------------------------------------------------------------------
    inline void setB(const GLubyte a_blue)
    {
        m_color[2] = a_blue;
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e blue component
    */
    //-----------------------------------------------------------------------
    inline GLfloat getB() const
    {
        return(m_color[2]);
    }


    //-----------------------------------------------------------------------
    /*!
          Set the \e alpha component

          \param    a_alpha Alpha component
    */
    //-----------------------------------------------------------------------
    inline void setA(const GLubyte a_alpha)
    {
        m_color[3] = a_alpha;
    }


    //-----------------------------------------------------------------------
    /*!
          Read the \e alpha component.
    */
    //-----------------------------------------------------------------------
    inline GLfloat getA() const
    {
        return(m_color[3]);
    }


    //-----------------------------------------------------------------------
    /*!
          Render this color in OpenGL (sets it to be the currnent color)

          Does not confirm that GL color-tracking is enabled
    */
    //-----------------------------------------------------------------------
    inline void render() const
    {
        glColor4bv((const signed char*)(&m_color[0]));
    };


    //-----------------------------------------------------------------------
    /*!
          Return a pointer to the raw color array

          \return   Return memory location of color array
    */
    //-----------------------------------------------------------------------
    inline const GLubyte* pColor() const
    {
        return (&m_color[0]);
    }



    //-----------------------------------------------------------------------
    /*!
          Return this color converted to \e cColorf format.

          This method is the only method defined outside the class
          definition, to avoid a recursive referencing problem with the
          cColorf class.
    */
    //-----------------------------------------------------------------------
    inline cColorf getColorf() const;

  public:
    // MEMBERS:
    //! Color in \e GLubyte format [R,G,B,A].
    GLubyte m_color[4];
};


// One function in each type was declared but not defined, to avoid
// recursive referencing problems...
cColorb cColorf::getColorb(void) const
{
    cColorb color(   (GLubyte)(m_color[0] * (GLfloat)0xff),
                     (GLubyte)(m_color[1] * (GLfloat)0xff),
                     (GLubyte)(m_color[2] * (GLfloat)0xff),
                     (GLubyte)(m_color[3] * (GLfloat)0xff) );
    return (color);
}

cColorf cColorb::getColorf(void) const
{
    cColorf color (   (GLfloat)m_color[0] / (GLfloat)0xff,
                      (GLfloat)m_color[1] / (GLfloat)0xff,
                      (GLfloat)m_color[2] / (GLfloat)0xff,
                      (GLfloat)m_color[3] / (GLfloat)0xff );
    return color;
}

extern cColorf CHAI_COLOR_RED;
extern cColorf CHAI_COLOR_GREEN;
extern cColorf CHAI_COLOR_BLUE;
extern cColorf CHAI_COLOR_WHITE;
extern cColorf CHAI_COLOR_BLACK;
extern cColorf CHAI_COLOR_YELLOW;
extern cColorf CHAI_COLOR_AQUA;
extern cColorf CHAI_COLOR_PURPLE;

extern cColorf CHAI_COLOR_PASTEL_RED;
extern cColorf CHAI_COLOR_PASTEL_GREEN;
extern cColorf CHAI_COLOR_PASTEL_BLUE;
extern cColorf CHAI_COLOR_PASTEL_AQUA;
extern cColorf CHAI_COLOR_PASTEL_VIOLET;
extern cColorf CHAI_COLOR_PASTEL_YELLOW;
extern cColorf CHAI_COLOR_PASTEL_GRAY;

#define N_CHAI_BASIC_COLORS 8
extern cColorf CHAI_BASIC_COLORS[N_CHAI_BASIC_COLORS];

#define N_CHAI_PASTEL_COLORS 7
extern cColorf CHAI_PASTEL_COLORS[N_CHAI_PASTEL_COLORS];

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

