//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       XColor.h
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef XColorH
#define XColorH
//---------------------------------------------------------------------------

//===========================================================================
// - STRUCTURE DEFINITION -
/*!
     \struct    xColor4b
     \brief     Definition of a byte color vector using RGBA components.
                Each color component requires 1 byte memory storage.
     \param     r     Red component.
     \param     g     Green component.
     \param     b     Blue component.
     \param     a     Transparency component.
*/
//===========================================================================
  struct xColor4b
  {
    unsigned char   r;
    unsigned char   g;
    unsigned char   b;
    unsigned char   a;
  };


//===========================================================================
// - STRUCTURE DEFINITION -
/*!
     \struct    xColor4f
     \brief     Definition of a float color vector in RGBA components.
                Each color component requires 1 float memory storage.
     \param     r     Red component.
     \param     g     Green component.
     \param     b     Blue component.
     \param     a     Transparency component.
*/
//===========================================================================
  struct xColor4f
  {
    float   r;
    float   g;
    float   b;
    float   a;
  };


//===========================================================================
// - COLOR OPERATIONS -
//===========================================================================
  // INITIALIZATION:
  //! Set a color given its RGB components.
  xColor4b xSetColor4b(unsigned char iR, unsigned char iG, unsigned char iB);
  //! Set a color given its RGB components.
  xColor4f xSetColor4f(float iR,float iG, float iB);
  //! Set a color given its RGBA components.
  xColor4b xSetColor4b(unsigned char iR, unsigned char iG, unsigned char iB,
                       unsigned char iA);
  //! Set a color given its RGBA components.
  xColor4f xSetColor4f(float iR, float iG, float iB, float iA);

  // FORMAT CONVERSION:
  //! Convert a byte color vector into a float color vector.
  xColor4f xColorConvert(xColor4b iColor);
  //! Convert a float color vector into a byte color vector.
  xColor4b xColorConvert(xColor4f iColor);

#endif

//===========================================================================
// END OF FILE
//===========================================================================
