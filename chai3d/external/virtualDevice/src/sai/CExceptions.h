//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CExceptions.h
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CExceptionsH
#define CExceptionsH
//---------------------------------------------------------------------------
#include <vcl/dstring.h>
//---------------------------------------------------------------------------

//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exException
      \brief      Parent exception class.
*/
//===========================================================================
class exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - Error"); };
};


//===========================================================================
// - CLASS DEFINITION -
// NAME:        exDivisionByZero
// ABSTRACT:    Division by zero.
//===========================================================================
//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exDivisionByZero
      \brief      Division by zero error.
*/
//===========================================================================
class exDivisionByZero : public exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - Division by zero"); };
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exVectorZero
      \brief      Vector of length zero. Further computation could not be performed.
*/
//===========================================================================
class exVectorZero : public exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - Length of Vector is zero"); };
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exZeroDeterminant
      \brief      Determinant of matrix is equal to zero.
                  Further computation could not be performed.
*/
//===========================================================================
class exZeroDeterminant : public exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - Determinant of Matrix is zero"); };
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exAngleError
      \brief      Angle between two vectors could not be computed.
*/
//===========================================================================
class exAngleError : public exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - Angle could not be computed"); };

};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exListItemDoesNotExist
      \brief      The selected items does not exist in the list.
*/
//===========================================================================
class exListItemDoesNotExist : public exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - List item does not exist."); };
};


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      exViewportError
      \brief      Viewport could not be initialized.
*/
//===========================================================================
class exViewportError : public exException
{
  public:
    virtual String GetErrorMessage(void) { return("SAI - Viewport could not be created."); };
};
#endif

//===========================================================================
// END OF FILE
//===========================================================================
