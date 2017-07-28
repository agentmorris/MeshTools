//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       XColor.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "XColor.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//===========================================================================
// - FUNCTION -
/*!
      Set a color by giving its (R,G,B) components.

      \fn       xColor4b xSetColor4b(unsigned char iR, unsigned char iG,
                                     unsigned char iB)
      \param    iR     Red component.
      \param    iG     Green component.
      \param    iB     Blue component.
      \return   Return a color in xColor4b format
*/
//===========================================================================
xColor4b xSetColor4b(unsigned char iR, unsigned char iG, unsigned char iB)
{
  xColor4b Color;
  Color.r = iR;
  Color.g = iG;
  Color.b = iB;
  Color.a = 0xff;
  return(Color);
}


//===========================================================================
// - FUNCTION -
/*!
      Set a color by giving its (R,G,B) components.

      \fn       xColor4f xSetColor4f(float iR,float iG, float iB)
      \param    iR     Red component.
      \param    iG     Green component.
      \param    iB     Blue component.
      \return   Return a color in xColor4f format
*/
//===========================================================================
xColor4f xSetColor4f(float iR,float iG, float iB)
{
  xColor4f Color;
  Color.r = iR;
  Color.g = iG;
  Color.b = iB;
  Color.a = 1.0;
  return(Color);
}


//===========================================================================
// - FUNCTION -
/*!
      Set a color by giving its (R,G,B,A) components.

      \fn       xColor4b xSetColor4b(unsigned char iR, unsigned char iG,
                                     unsigned char iB, unsigned char iA)
      \param    iR     Red component.
      \param    iG     Green component.
      \param    iB     Blue component.
      \param    iA     Transparency component.
      \return   Return a color in xColor4b format
*/
//===========================================================================
xColor4b xSetColor4b(unsigned char iR, unsigned char iG, unsigned char iB,
                      unsigned char iA)
{
  xColor4b Color;
  Color.r = iR;
  Color.g = iG;
  Color.b = iB;
  Color.a = iA;
  return(Color);
}


//===========================================================================
// - FUNCTION -
/*!
      Set a color by giving its (R,G,B,A) components.

      \fn       xColor4f xSetColor4f(float iR, float iG, float iB, float iA)
      \param    iR     Red component.
      \param    iG     Green component.
      \param    iB     Blue component.
      \param    iA     Transparency component.
      \return   Return a color in xColor4f format
*/
//===========================================================================
xColor4f xSetColor4f(float iR, float iG, float iB, float iA)
{
  xColor4f Color;
  Color.r = iR;
  Color.g = iG;
  Color.b = iB;
  Color.a = iA;
  return(Color);
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a color from byte to float format.

      \fn       xColor4f xColorConvert(xColor4b iColor)
      \param    iColor    Byte color vector to be converted.
      \return   Return iColor in xColor4f format.
*/
//===========================================================================
xColor4f xColorConvert(xColor4b iColor)
{
  xColor4f Color;
  Color.r = (float)((float)iColor.r / 0xff);
  Color.g = (float)((float)iColor.g / 0xff);
  Color.b = (float)((float)iColor.b / 0xff);
  Color.a = (float)((float)iColor.a / 0xff);
  return(Color);
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a color from float to byte format.

      \fn       xColor4b xColorConvert(xColor4f iColor)
      \param    iColor    Float color vector to be converted.
      \return   Return iColor in xColor4b format.
*/
//===========================================================================
xColor4b xColorConvert(xColor4f iColor)
{
  xColor4b Color;
  Color.r = iColor.r * 0xff;
  Color.g = iColor.g * 0xff;
  Color.b = iColor.b * 0xff;
  Color.a = iColor.a * 0xff;
  return(Color);
}


//===========================================================================
// END OF FILE
//===========================================================================
