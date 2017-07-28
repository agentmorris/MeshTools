//===========================================================================
//
//  - UNIT -
//
//  Copyright (C) 2000. Francois Conti. All rights reserved.
//  Robotics Laboratory - Stanford University
//
//  /*!
//  \brief      Library for managing floats, vectors and matrices.
//  \author     conti@robotics.stanford.edu
//  \version    1.0
//  \date       01/2002
//  */
//
//===============================================F============================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "XMatrix.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//===========================================================================
// - FUNCTION -
/*!
      Calculates the cosine of an angle given in degrees.

      \fn       double xCosDeg(const double &iValue)
      \param    iValue  Angle in degrees.
      \return   return the cosine of the angle.
*/
//===========================================================================
double xCosDeg(const double &iValue)
{
  return (cos(iValue * XDEG2RAD));
}


//===========================================================================
// - FUNCTION -
/*!
      Calculates the sine of an angle given in degrees.

      \fn       double xSinDeg(const double &iValue)
      \param    iValue  Angle in degrees.
      \return   return the sine of the angle.
*/
//===========================================================================
double xSinDeg(const double &iValue)
{
  return (sin(iValue * XDEG2RAD));
}


//===========================================================================
// - FUNCTION -
/*!
      Calculates the tangent of an angle given in degrees.

      \fn       double xTanDeg(const double &iValue)
      \param    iValue  Angle in degrees.
      \return   return the tangent of the angle.
*/
//===========================================================================
double xTanDeg(const double &iValue)
{
  return (tan(iValue * XDEG2RAD));
}


//===========================================================================
// - FUNCTION -
/*!
      Calculates the cosine of an angle given in radians.

      \fn       double xCosRad(const double &iValue)
      \param    iValue  Angle in radians.
      \return   return the cosine of the angle.
*/
//===========================================================================
double xCosRad(const double &iValue)
{
  return (cos(iValue));
}


//===========================================================================
// - FUNCTION -
/*!
      Calculates the sine of an angle given in radians.

      \fn       double xSinRad(const double &iValue)
      \param    iValue  Angle in radians.
      \return   return the sine of the angle.
*/
//===========================================================================
double xSinRad(const double &iValue)
{
  return (sin(iValue));
}


//===========================================================================
// - FUNCTION -
/*!
      Calculates the tangent of an angle given in radians.

      \fn       double xTanRad(const double &iValue)
      \param    iValue  Angle in radians.
      \return   return the tangent of the angle.
*/
//===========================================================================
double xTanRad(const double &iValue)
{
  return (tan(iValue));
}


//===========================================================================
// - FUNCTION -
/*!
      Convert an angle from degrees to radians.

      \fn       double xDegToRad(const double &iValue)
      \param    iValue  Angle in degrees.
      \return   return the angle in radians.
*/
//===========================================================================
double xDegToRad(const double &iValue)
{
  return (iValue * XDEG2RAD);
}


//===========================================================================
// - FUNCTION -
/*!
      Convert an angle from radians to degrees.

      \fn       double xRadToDeg(const double &iValue)
      \param    iValue  Angle in radians.
      \return   return the angle in degrees.
*/
//===========================================================================
double xRadToDeg(const double &iValue)
{
  return (iValue * XRAD2DEG);
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a boolean value into a string: "true" or "false"

      \fn       String xStr(const Boolean &iValue)
      \param    iValue  Boolean value to be converted into a string.
      \return   Return string "true" or "false"
*/
//===========================================================================
String xStr(const Boolean &iValue)
{
  if (iValue){ return ("true"); }
  else { return ("false"); }
}


//===========================================================================
// - FUNCTION -
/*!
      Convert an integer into an string.

      \fn       String xStr(const int &iValue)
      \param    iValue  Integer value to be converted into a string.
      \return   Return string of iValue.
*/
//===========================================================================
String xStr(const int &iValue)
{
  return(IntToStr(iValue));
}


//===========================================================================
// - FUNCTION -
//  NAME:               xStr()
/*!
      Convert a double into an string.

      \fn       String xStr(const double &iValue, const int &iNumberOfDigits)
      \param    iValue            Number to be converted into a string.
      \param    iNumberOfDigits   Number of digits.
      \return   Return string of iValue.
*/
//===========================================================================
String xStr(const double &iValue, const int &iNumberOfDigits)
{
  return( FloatToStrF((float)iValue,ffFixed,8,iNumberOfDigits));
}

//===========================================================================
// - FUNCTION -
/*!
      Convert a string into a double.

      \fn       bool xValue(const String &iString, double &iValue)
      \param    iString   String to be converted.
      \param    iValue    Value returned after string conversion
      \return   Return TRUE if conversion succeded. else return FALSE.
*/
//===========================================================================
bool xValue(const String &iString, double &iValue)
{
  bool ErrorCode;
  try {
   iValue = StrToFloat(iString);
   ErrorCode = TRUE;
  }
  catch(EConvertError& theException)
  {
    iValue = 0;
    ErrorCode = FALSE;
  }
  return(ErrorCode);
}


//===========================================================================
// - VECTORS & MATRICES -
//===========================================================================

//===========================================================================
// - FUNCTION -
/*!
      Initialize a 2D vector.

      \fn       xVector2d xSet(const double &iX, const double &iY)
      \param    iX      X component.
      \param    iY      Y component.
      \return   return 2D vector.
*/
//===========================================================================
xVector2d xSet(const double &iX, const double &iY)
{
  xVector2d Vect;
  Vect.x = iX;
  Vect.y = iY;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Initialize a 3D vector.

      \fn       xVector3d xSet(const double &iX, const double &iY, const double &iZ)
      \param    iX      X component.
      \param    iY      Y component.
      \param    iZ      Z component.
      \return   return 3D vector.
*/
//===========================================================================
xVector3d xSet(const double &iX, const double &iY, const double &iZ)
{
  xVector3d Vect;
  Vect.x = iX;
  Vect.y = iY;
  Vect.z = iZ;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Addition of two 2D-Vectors.

      \fn       xVector2d xAdd(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0    Vector 0.
      \param    iVect1    Vector 1.
      \return   Return the addition of both vectors. Result = iVect0 + iVect1.
*/
//===========================================================================
// ADDITION:
xVector2d xAdd(const xVector2d &iVect0, const xVector2d &iVect1)
{
  xVector2d Vect;
  Vect.x = iVect0.x + iVect1.x;
  Vect.y = iVect0.y + iVect1.y;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Addition of two 3D-Vectors.

      \fn       xVector3d xAdd(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0    Vector 0.
      \param    iVect1    Vector 1.
      \return   Return the addition of both vectors. Result = iVect0 + iVect1.
*/
//===========================================================================
xVector3d xAdd(const xVector3d &iVect0, const xVector3d &iVect1)
{
  xVector3d Vect;
  Vect.x = iVect0.x + iVect1.x;
  Vect.y = iVect0.y + iVect1.y;
  Vect.z = iVect0.z + iVect1.z;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Subtract Vector1 from Vector0. (2D-Vectors)

      \fn       xVector2d xSub(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0    Vector 0.
      \param    iVect1    Vector 1.
      \return   Return the following subtraction: Result = iVect0 - iVect1.
*/
//===========================================================================
xVector2d xSub(const xVector2d &iVect0, const xVector2d &iVect1)
{
  xVector2d Vect;
  Vect.x = iVect0.x - iVect1.x;
  Vect.y = iVect0.y - iVect1.y;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Subtract Vector1 from Vector0. (3D-Vectors)

      \fn       xVector3d xSub(const xVector3d &iVect0, const xVector3d &iVect1)
      \param    iVect0    Vector 0.
      \param    iVect1    Vector 1.
      \return   Return the following subtraction: Result = iVect0 - iVect1.
*/
//===========================================================================
xVector3d xSub(const xVector3d &iVect0, const xVector3d &iVect1)
{
  xVector3d Vect;
  Vect.x = iVect0.x - iVect1.x;
  Vect.y = iVect0.y - iVect1.y;
  Vect.z = iVect0.z - iVect1.z;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Multiply a 2D vector by a scalar.

      \fn       xVector2d xMul(const double &iK, const xVector2d &iVect)
      \param    iK    Scalar.
      ]param    iVect Vector.
      \return   Returns a vector. Result = iK * iVect.
*/
//===========================================================================
xVector2d xMul(const double &iK, const xVector2d &iVect)
{
  xVector2d Vect;
  Vect.x = iK * iVect.x;
  Vect.y = iK * iVect.y;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Multiply a 3D vector by a scalar.

      \fn       xVector3d xMul(const double &iK, const xVector3d &iVect)
      \param    iK    Scalar.
      ]param    iVect Vector.
      \return   Returns a vector. Result = iK * iVect.
*/
//===========================================================================
xVector3d xMul(const double &iK, const xVector3d &iVect)
{
  xVector3d Vect;
  Vect.x = iK * iVect.x;
  Vect.y = iK * iVect.y;
  Vect.z = iK * iVect.z;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the cross product between two 2D vectors.

      \fn       xVector3d xCross(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0  Vector 0.
      \param    iVect1  Vector 1.
      \return   Returns the cross product between iVect0 and iVect1.
                A 3D vector is returned.
*/
//===========================================================================
xVector3d xCross(const xVector2d &iVect0, const xVector2d &iVect1)
{
  xVector3d Vect;
  Vect.x = 0;
  Vect.y = 0;
  Vect.z = (iVect0.x * iVect1.y) - (iVect0.y * iVect1.x);
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the cross product between two 3D vectors.

      \fn       xVector3d xCross(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0  Vector 0.
      \param    iVect1  Vector 1.
      \return   Returns the cross product between iVect0 and iVect1.
*/
//===========================================================================
xVector3d xCross(const xVector3d &iVect0, const xVector3d &iVect1)
{
  xVector3d Vect;
  Vect.x = (iVect0.y * iVect1.z) - (iVect0.z * iVect1.y);
  Vect.y =-(iVect0.x * iVect1.z) + (iVect0.z * iVect1.x);
  Vect.z = (iVect0.x * iVect1.y) - (iVect0.y * iVect1.x);
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the dot product between two 2D vectors.

      \fn       double xDot(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0  Vector 0.
      \param    iVect1  Vector 1.
      \return   Returns the dot product between iVect0 and iVect1.
*/
//===========================================================================
double xDot(const xVector2d &iVect0, const xVector2d &iVect1)
{
  return( (iVect0.x * iVect1.x) + (iVect0.y * iVect1.y) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the dot product between two 3D vectors.

      \fn       double xDot(const xVector3d &iVect0, const xVector3d &iVect1)
      \param    iVect0  Vector 0.
      \param    iVect1  Vector 1.
      \return   Returns the dot product between iVect0 and iVect1.
*/
//===========================================================================
double xDot(const xVector3d &iVect0, const xVector3d &iVect1)
{
  return( (iVect0.x * iVect1.x) + (iVect0.y * iVect1.y) +
          (iVect0.z * iVect1.z) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the normalized vector (length = 1) from a given 2D vector.

      \fn       xVector2d xNormalize(const xVector2d &iVect)
      \param    iVect   Vector
      \return   Returns the normalized vector of iVect.
                Length of returned vector = 1.
*/
//===========================================================================
xVector2d xNormalize(const xVector2d &iVect)
{
  return( xMul( 1/xNorm(iVect), iVect) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the normalized vector (length = 1) from a given 3D vector.

      \fn       xVector3d xNormalize(const xVector3d &iVect)
      \param    iVect   Vector
      \return   Returns the normalized vector of iVect.
                Length of returned vector = 1.
*/
//===========================================================================
xVector3d xNormalize(const xVector3d &iVect)
{
  return( xMul( 1/xNorm(iVect), iVect) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the distance between two 2D points.

      \fn       double xDist(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0  Point0.
      \param    iVect1  Point1.
      \return   Return distance between iVect0 and iVect1.
*/
//===========================================================================
double xDist(const xVector2d &iVect0, const xVector2d &iVect1)
{
  return( sqrt( ((iVect0.x - iVect1.x) * (iVect0.x - iVect1.x)) +
                ((iVect0.y - iVect1.y) * (iVect0.y - iVect1.y)) ) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the distance between two 3D points.

      \fn       double xDist(const xVector3d &iVect0, const xVector3d &iVect1)
      \param    iVect0  Point0.
      \param    iVect1  Point1.
      \return   Return distance between iVect0 and iVect1.
*/
//===========================================================================
double xDist(const xVector3d &iVect0, const xVector3d &iVect1)
{
  return( sqrt( ((iVect0.x - iVect1.x) * (iVect0.x - iVect1.x)) +
                ((iVect0.y - iVect1.y) * (iVect0.y - iVect1.y)) +
                ((iVect0.z - iVect1.z) * (iVect0.z - iVect1.z)) ) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the length of a given 2D vector.

      \fn       double xNorm(const xVector2d &iVect)
      \param    iVect   Vector.
      \return   Returns the value corresponding to the length of iVect.
*/
//===========================================================================
double xNorm(const xVector2d &iVect)
{
  return( sqrt( (iVect.x * iVect.x) + (iVect.y * iVect.y) ) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the length of a given 3D vector.

      \fn       double xNorm(const xVector3d &iVect)
      \param    iVect   Vector.
      \return   Returns the value corresponding to the length of iVect.
*/
//===========================================================================
double xNorm(const xVector3d &iVect)
{
  return( sqrt( (iVect.x * iVect.x) + (iVect.y * iVect.y) + (iVect.z * iVect.z) ) );
}


//===========================================================================
// - FUNCTION -
/*!
      Compare two 2D vectors for equality.

      \fn       bool xEqual(const xVector2d &iVect0, const xVector2d &iVect1)
      \param    iVect0  Vector 0.
      \param    iVect1  Vector 1.
      \return   Return True if two vectors are equal (iVect0 == iVect1).
                Otherwize return False.
*/
//===========================================================================
bool xEqual(const xVector2d &iVect0, const xVector2d &iVect1)
{
  if ( (iVect0.x == iVect1.x) && (iVect0.y == iVect1.y) ) { return( True); }
  else { return (False); }
}


//===========================================================================
// - FUNCTION -
/*!
      Compare two 3D vectors for equality.

      \fn       bool xEqual(const xVector3d &iVect0, const xVector3d &iVect1)
      \param    iVect0  Vector 0.
      \param    iVect1  Vector 1.
      \return   Return True if two vectors are equal (iVect0 == iVect1).
                Otherwize return False.
*/
//===========================================================================
bool xEqual(const xVector3d &iVect0, const xVector3d &iVect1)
{
  if ( (iVect0.x == iVect1.x) && (iVect0.y == iVect1.y) &&
       (iVect0.z == iVect1.z) ) { return( True); }
  else { return (False); }
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a 2D vector into an string.

      \fn       String xStr(const xVector2d &iVect, const int &iNumberOfDigits)
      \param    iVect             Vector.
      \param    iNumberOfDigits   Number of digits for each vector component (X,Y).
      \return   Returns a string of the given 2D vector.
                Example of string retured: "(1.1, 2.5)"
*/
//===========================================================================
String xStr(const xVector2d &iVect, const int &iNumberOfDigits)
{
  return("(" + xStr(iVect.x, iNumberOfDigits) + ", "
             + xStr(iVect.y, iNumberOfDigits) + ")");
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a 3D vector into an string.

      \fn       String xStr(const xVector3d &iVect, const int &iNumberOfDigits)
      \param    iVect             Vector.
      \param    iNumberOfDigits   Number of digits for each vector component (X,Y,Z).
      \return   Returns a string of the given 3D vector.
                Example of string retured: "(6.0, 9.8, 7.8)"
*/
//===========================================================================
String xStr(const xVector3d &iVect, const int &iNumberOfDigits)
{
  return("(" + xStr(iVect.x, iNumberOfDigits) + ", "
             + xStr(iVect.y, iNumberOfDigits) + ", "
             + xStr(iVect.z, iNumberOfDigits) + ")");
}


//===========================================================================
// - FUNCTION -
/*!
      Initialize a 2x2 matrix.

      \fn       xMatrix22d xSetMat(const double &iM00, const double &iM01,
                                   const double &iM10, const double &iM11)
      \param    iM00    Matrix Component [0,0]
      \param    iM01    Matrix Component [0,1]
      \param    iM10    Matrix Component [1,0]
      \param    iM11    Matrix Component [1,1]
      \return   Returns a 2x2 matrix containg the defined iMxx parameters.
*/
//===========================================================================

xMatrix22d xSetMat(const double &iM00, const double &iM01,
                    const double &iM10, const double &iM11)
{
  xMatrix22d Mat;
  Mat.m[0][0]=iM00;  Mat.m[0][1]=iM01;
  Mat.m[1][0]=iM10;  Mat.m[1][1]=iM11;
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Initialize a 3x3 matrix.

      \fn       xMatrix33d xSetMat(const double &iM00, const double &iM01, const double &iM02,
                    const double &iM10, const double &iM11, const double &iM12,
                    const double &iM20, const double &iM21, const double &iM22)
      \param    iM00    Matrix Component [0,0]
      \param    iM01    Matrix Component [0,1]
      \param    iM02    Matrix Component [0,2]
      \param    iM10    Matrix Component [1,0]
      \param    iM11    Matrix Component [1,1]
      \param    iM12    Matrix Component [1,2]
      \param    iM20    Matrix Component [2,0]
      \param    iM21    Matrix Component [2,1]
      \param    iM22    Matrix Component [2,2]
      \return   Returns a 3x3 matrix containg the defined iMxx parameters.
*/
//===========================================================================
xMatrix33d xSetMat(const double &iM00, const double &iM01, const double &iM02,
                    const double &iM10, const double &iM11, const double &iM12,
                    const double &iM20, const double &iM21, const double &iM22)
{
  xMatrix33d Mat;
  Mat.m[0][0]=iM00;  Mat.m[0][1]=iM01;  Mat.m[0][2]=iM02;
  Mat.m[1][0]=iM10;  Mat.m[1][1]=iM11;  Mat.m[1][2]=iM12;
  Mat.m[2][0]=iM20;  Mat.m[2][1]=iM21;  Mat.m[2][2]=iM22;
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Initialize a 2x2 matrix from 2D column vectors.

      \fn       xMatrix22d xSetMat(const xVector2d &iVectCol0, const xVector2d &iVectCol1)
      \param    iVectCol0       Vector Column 0.
      \param    iVectCol1       Vector Column 1.
      \return   Return a 2x2 matrix containing defined column vectors.
*/
//===========================================================================
xMatrix22d xSetMat(const xVector2d &iVectCol0, const xVector2d &iVectCol1)
{
  xMatrix22d Mat;
  Mat.m[0][0]=iVectCol0.x;  Mat.m[0][1]=iVectCol1.x;
  Mat.m[1][0]=iVectCol0.y;  Mat.m[1][1]=iVectCol1.y;
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Initialize a 3x3 matrix from 3D column vectors.

      \fn       xMatrix33d xSetMat(const xVector3d &iVectCol0, const xVector3d &iVectCol1,
                                   const xVector3d &iVectCol2)
      \param    iVectCol0       Vector Column 0.
      \param    iVectCol1       Vector Column 1.
      \param    iVectCol2       Vector Column 2.
      \return   Return a 3x3 matrix containing defined column vectors.
*/
//===========================================================================
xMatrix33d xSetMat(const xVector3d &iVectCol0, const xVector3d &iVectCol1,
                   const xVector3d &iVectCol2)
{
  xMatrix33d Mat;
  Mat.m[0][0]=iVectCol0.x;  Mat.m[0][1]=iVectCol1.x;  Mat.m[0][2]=iVectCol2.x;
  Mat.m[1][0]=iVectCol0.y;  Mat.m[1][1]=iVectCol1.y;  Mat.m[1][2]=iVectCol2.y;
  Mat.m[2][0]=iVectCol0.z;  Mat.m[2][1]=iVectCol1.z;  Mat.m[2][2]=iVectCol2.z;
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Return the 2x2 identity matrix.

      \fn       xMatrix22d xIdentity22d(void)
      \return   Return the 2x2 identity matrix.
*/
//===========================================================================
xMatrix22d xIdentity22d(void)
{
  xMatrix22d Mat;
  Mat.m[0][0]=1;  Mat.m[0][1]=0;
  Mat.m[1][0]=0;  Mat.m[1][1]=1;
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Return the 3x3 identity matrix.

      \fn       xMatrix33d xIdentity33d(void)
      \return   Return the 3x3 identity matrix.
*/
//===========================================================================
xMatrix33d xIdentity33d(void)
{
  xMatrix33d Mat;
  Mat.m[0][0]=1;  Mat.m[0][1]=0;  Mat.m[0][2]=0;
  Mat.m[1][0]=0;  Mat.m[1][1]=1;  Mat.m[1][2]=0;
  Mat.m[2][0]=0;  Mat.m[2][1]=0;  Mat.m[2][2]=1;
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Multiply two 2x2 matrices.

      \fn       xMatrix22d xMul(const xMatrix22d &iMat0, const xMatrix22d &iMat1)
      \param    iMat0   Matrix 0.
      \param    iMat1   Matrix 1.
      \return   Return a the multiplication of both matrices. Result = iMat0 * iMat1.
*/
//===========================================================================
xMatrix22d xMul(const xMatrix22d &iMat0, const xMatrix22d &iMat1)
{
  xMatrix22d Mat;
  for (int i=0; i<2; i++){
    for (int j=0; j<2; j++){
      Mat.m[i][j] = iMat0.m[i][0] * iMat1.m[0][j] +
                    iMat0.m[i][1] * iMat1.m[1][j];
      }}
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Multiply two 3x3 matrices.

      \fn       xMatrix33d xMul(const xMatrix33d &iMat0, const xMatrix33d &iMat1)
      \param    iMat0   Matrix 0.
      \param    iMat1   Matrix 1.
      \return   Return a the multiplication of both matrices. Result = iMat0 * iMat1.
*/
//===========================================================================
xMatrix33d xMul(const xMatrix33d &iMat0, const xMatrix33d &iMat1)
{
  xMatrix33d Mat;
  for (int i=0; i<3; i++){
    for (int j=0; j<3; j++){
      Mat.m[i][j] = iMat0.m[i][0] * iMat1.m[0][j] +
                    iMat0.m[i][1] * iMat1.m[1][j] +
                    iMat0.m[i][2] * iMat1.m[2][j];
      }}
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      2D vector, 2x2 matrix multiplication.

      \fn       xVector2d xMul(const xMatrix22d &iMat, const xVector2d &iVect)
      \param    iMat    Matrix
      \param    iVect   Vector
      \return   Return a vector. Result = iMat * iVect.
*/
//===========================================================================
xVector2d xMul(const xMatrix22d &iMat, const xVector2d &iVect)
{
  xVector2d Vect;
  Vect.x = iMat.m[0][0]*iVect.x + iMat.m[0][1]*iVect.y;
  Vect.y = iMat.m[1][0]*iVect.x + iMat.m[1][1]*iVect.y;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      3D vector, 3x3 matrix multiplication.

      \fn       xVector3d xMul(const xMatrix33d &iMat, const xVector3d &iVect)
      \param    iMat    Matrix
      \param    iVect   Vector
      \return   Return a vector. Result = iMat * iVect.
*/
//===========================================================================
xVector3d xMul(const xMatrix33d &iMat, const xVector3d &iVect)
{
  xVector3d Vect;
  Vect.x = iMat.m[0][0]*iVect.x + iMat.m[0][1]*iVect.y + iMat.m[0][2]*iVect.z;
  Vect.y = iMat.m[1][0]*iVect.x + iMat.m[1][1]*iVect.y + iMat.m[1][2]*iVect.z;
  Vect.z = iMat.m[2][0]*iVect.x + iMat.m[2][1]*iVect.y + iMat.m[2][2]*iVect.z;
  return(Vect);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the determinant of a 2x2 matrix.

      \fn       double xDet(const xMatrix22d &iMat)
      \param    iMat    Matrix.
      \return   Return the determinant of matrix iMat.
*/
//===========================================================================
double xDet(const xMatrix22d &iMat)
{
  return (+ iMat.m[0][0] * iMat.m[1][1]
          - iMat.m[1][0] * iMat.m[0][1]);

}


//===========================================================================
// - FUNCTION -
/*!
      Compute the determinant of a 3x3 matrix.

      \fn       double xDet(const xMatrix33d &iMat)
      \param    iMat    Matrix.
      \return   Return the determinant of matrix iMat.
*/
//===========================================================================
double xDet(const xMatrix33d &iMat)
{
  return (+ iMat.m[0][0] * iMat.m[1][1] * iMat.m[2][2]
          + iMat.m[0][1] * iMat.m[1][2] * iMat.m[2][0]
          + iMat.m[0][2] * iMat.m[1][0] * iMat.m[2][1]

          - iMat.m[2][0] * iMat.m[1][1] * iMat.m[0][2]
          - iMat.m[2][1] * iMat.m[1][2] * iMat.m[0][0]
          - iMat.m[2][2] * iMat.m[1][0] * iMat.m[0][1]);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the transpose of a 2x2 matrix.

      \fn       xMatrix22d xTrans(const xMatrix22d &iMat)
      \param    iMat    Matrix.
      \return   Return the transpose of matrix iMat.
*/
//===========================================================================
xMatrix22d xTrans(const xMatrix22d &iMat)
{
  xMatrix22d Mat;
  Mat.m[0][0]=iMat.m[0][0];  Mat.m[0][1]=iMat.m[1][0];
  Mat.m[1][0]=iMat.m[0][1];  Mat.m[1][1]=iMat.m[1][1];
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the transpose of a 3x3 matrix.

      \fn       xMatrix33d xTrans(const xMatrix33d &iMat)
      \param    iMat    Matrix.
      \return   Return the transpose of matrix iMat.
*/
//===========================================================================
xMatrix33d xTrans(const xMatrix33d &iMat)
{
  xMatrix33d Mat;
  Mat.m[0][0]=iMat.m[0][0];  Mat.m[0][1]=iMat.m[1][0];
  Mat.m[0][2]=iMat.m[2][0];

  Mat.m[1][0]=iMat.m[0][1];  Mat.m[1][1]=iMat.m[1][1];
  Mat.m[1][2]=iMat.m[2][1];

  Mat.m[2][0]=iMat.m[0][2];  Mat.m[2][1]=iMat.m[1][2];
  Mat.m[2][2]=iMat.m[2][2];

  Mat.m[3][0]=iMat.m[0][3];  Mat.m[3][1]=iMat.m[1][3];
  Mat.m[3][2]=iMat.m[2][3];
  return(Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the inverse of a 2x2 matrix.

      \fn       xMatrix22d xInv(const xMatrix22d &iMat)
      \param    iMat    Matrix.
      \return   Return the inverse of matrix iMat.
*/
//===========================================================================
xMatrix22d xInv(const xMatrix22d &iMat)
{
  xMatrix22d Mat;
  double Det = xDet(iMat);
  if ((Det < XMIN) && (Det > -XMIN))
  {
    throw exZeroDeterminant();
  }
  else
  {
    Mat.m[0][0] =  (iMat.m[1][1])/Det;
    Mat.m[0][1] = -(iMat.m[0][1])/Det;
    Mat.m[1][0] = -(iMat.m[1][0])/Det;
    Mat.m[1][1] =  (iMat.m[0][0])/Det;
  }
  return (Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Compute the inverse of a 3x3 matrix.

      \fn       xMatrix33d xInv(const xMatrix33d &iMat)
      \param    iMat    Matrix.
      \return   Return the inverse of matrix iMat.
*/
//===========================================================================
xMatrix33d xInv(const xMatrix33d &iMat)
{
  xMatrix33d Mat;
  double Det = xDet(iMat);
  if ((Det < XMIN) && (Det > -XMIN))
  {
    throw exZeroDeterminant();
  }
  else
  {
    Mat.m[0][0] =  (iMat.m[1][1]*iMat.m[2][2] - iMat.m[2][1]*iMat.m[1][2])/Det;
    Mat.m[0][1] = -(iMat.m[0][1]*iMat.m[2][2] - iMat.m[2][1]*iMat.m[0][2])/Det;
    Mat.m[0][2] =  (iMat.m[0][1]*iMat.m[1][2] - iMat.m[1][1]*iMat.m[0][2])/Det;

    Mat.m[1][0] = -(iMat.m[1][0]*iMat.m[2][2] - iMat.m[2][0]*iMat.m[1][2])/Det;
    Mat.m[1][1] =  (iMat.m[0][0]*iMat.m[2][2] - iMat.m[2][0]*iMat.m[0][2])/Det;
    Mat.m[1][2] = -(iMat.m[0][0]*iMat.m[1][2] - iMat.m[1][0]*iMat.m[0][2])/Det;

    Mat.m[2][0] =  (iMat.m[1][0]*iMat.m[2][1] - iMat.m[2][0]*iMat.m[1][1])/Det;
    Mat.m[2][1] = -(iMat.m[0][0]*iMat.m[2][1] - iMat.m[2][0]*iMat.m[0][1])/Det;
    Mat.m[2][2] =  (iMat.m[0][0]*iMat.m[1][1] - iMat.m[1][0]*iMat.m[0][1])/Det;

  }
  return (Mat);
}


//===========================================================================
// - FUNCTION -
/*!
      Generate a 3x3 rotation matrix defined by a rotation axis and rotation
      angle in radian.

      \fn       xMatrix33d xRotMatrix(const xVector3d &iAxis, const double &iAngleRad)
      \param    iAxis           Axis of rotation.
      \param    iAngleRad       Rotation angle in Radian.
      \return   Return a matrix corresponding to the defined axis/angle rotation.
*/
//===========================================================================
xMatrix33d xRotMatrix(const xVector3d &iAxis, const double &iAngleRad)
{
  double c = cos(iAngleRad);
  double s = sin(iAngleRad);
  double v = 1-c;

  xVector3d Axis = xNormalize(iAxis);
  double x = Axis.x;
  double y = Axis.y;
  double z = Axis.z;

  xMatrix33d  Rot;

  Rot.m[0][0] = x*x*v+c;     Rot.m[0][1] = x*y*v-z*s;  Rot.m[0][2] = x*z*v+y*s;
  Rot.m[1][0] = x*y*v+z*s;   Rot.m[1][1] = y*y*v+c;    Rot.m[1][2] = y*z*v-x*s;
  Rot.m[2][0] = x*z*v-y*s;   Rot.m[2][1] = y*z*v+x*s;  Rot.m[2][2] = z*z*v+c;

  return(Rot);
}


//===========================================================================
// - FUNCTION -
/*!
      Rotate a 3x3 rotation matrix by giving the axis of rotation and angle
      in radian of rotation.

      \fn       xMatrix33d xRotate(const xMatrix33d &iMat,
                                   const xVector3d &iAxis, const double &iAngleRad)
      \param    Initial rotation matrix.
      \param    Axis of rotation. Length of Axis must equal 1.
      \param    Rotation angle in Radian.
      \return   Return resulting rotation matrix after transformation.
*/
//===========================================================================
xMatrix33d xRotate(const xMatrix33d &iMat,
                    const xVector3d &iAxis, const double &iAngleRad)
{
  xMatrix33d Rot;
  Rot = xMul(xRotMatrix(iAxis, iAngleRad),iMat);
  return(Rot);
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a 2x2 matrix into an string.

      \fn       String xStr(const xMatrix22d &iMat, const int &iNumberOfDigits)
      \param    iMat              Matrix.
      \param    iNumberOfDigits   Number of digits for each matrix component.
      \return   Return a string of the 2x2 matrix.
*/
//===========================================================================
String xStr(const xMatrix22d &iMat, const int &iNumberOfDigits)
{
  return("( ( "
    + xStr(iMat.m[0][0], iNumberOfDigits) + " , " + xStr(iMat.m[0][1], iNumberOfDigits) + " ) ( "
    + xStr(iMat.m[1][0], iNumberOfDigits) + " , " + xStr(iMat.m[1][1], iNumberOfDigits) + " ) ) " );
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a 3x3 matrix into an string.

      \fn       String xStr(const xMatrix33d &iMat, const int &iNumberOfDigits)
      \param    iMat              Matrix.
      \param    iNumberOfDigits   Number of digits for each matrix component.
      \return   Return a string of the 3x3 matrix.
*/
//===========================================================================
String xStr(const xMatrix33d &iMat, const int &iNumberOfDigits)
{
  return("( ( "
    + xStr(iMat.m[0][0], iNumberOfDigits) + " , " + xStr(iMat.m[0][1], iNumberOfDigits) + " , " + xStr(iMat.m[0][2], iNumberOfDigits) + " ) ( "
    + xStr(iMat.m[1][0], iNumberOfDigits) + " , " + xStr(iMat.m[1][1], iNumberOfDigits) + " , " + xStr(iMat.m[1][2], iNumberOfDigits) + " ) ( "
    + xStr(iMat.m[2][0], iNumberOfDigits) + " , " + xStr(iMat.m[2][1], iNumberOfDigits) + " , " + xStr(iMat.m[2][2], iNumberOfDigits) + " ) ) " );
}



//===========================================================================
// - FUNCTION -
/*!
      Convert a translation vector into a 4x4 matrix for OpenGL.

      \fn       xDCS xSetDCS(xVector3d iPos)
      \param    iTranslation    Translation vector
      \return   Return a 4x4 matrix corresponding to the translation.
*/
//===========================================================================
xDCS xSetDCS(const xVector3d &iTrans)
{
  xDCS DCS;

  DCS.m[0][0] = 1;  DCS.m[0][1] = 0;  DCS.m[0][2] = 0;  DCS.m[0][3] = 0;
  DCS.m[1][0] = 0;  DCS.m[1][1] = 1;  DCS.m[1][2] = 0;  DCS.m[1][3] = 0;
  DCS.m[2][0] = 0;  DCS.m[2][1] = 0;  DCS.m[2][2] = 1;  DCS.m[2][3] = 0;
  DCS.m[3][0] = iTrans.x;  DCS.m[3][1] = iTrans.y; DCS.m[3][2] = iTrans.z;  DCS.m[3][3] = 1;

  return(DCS);
}


//===========================================================================
// - FUNCTION -
/*!
      Convert a rotation matrix into a 4x4 matrix for OpenGL.

      \fn       xDCS xSetDCS(xMatrix33d iRot)
      \param    iRot    3x3 Rotation matrix
      \return   Return a 4x4 matrix corresponding to the rotation.
*/
//===========================================================================
xDCS xSetDCS(const xMatrix33d &iRot)
{
  xDCS DCS;

  DCS.m[0][0] = iRot.m[0][0];  DCS.m[0][1] = iRot.m[1][0];  DCS.m[0][2] = iRot.m[2][0];  DCS.m[0][3] = 0.0f;
  DCS.m[1][0] = iRot.m[0][1];  DCS.m[1][1] = iRot.m[1][1];  DCS.m[1][2] = iRot.m[2][1];  DCS.m[1][3] = 0.0f;
  DCS.m[2][0] = iRot.m[0][2];  DCS.m[2][1] = iRot.m[1][2];  DCS.m[2][2] = iRot.m[2][2];  DCS.m[2][3] = 0.0f;
  DCS.m[3][0] = 0.0f;  DCS.m[3][1] = 0.0f;  DCS.m[3][2] = 0.0f;  DCS.m[3][3] = 1.0f;

  return(DCS);
}


//===========================================================================
// - FUNCTION -
/*!
      Multiply current OpenGL Matrix with a translation matrix.

      \fn       void xGLMulMatrixPos(const xVector3d &iTrans)
      \param    iTrans  3D vector describing a translation.
*/
//===========================================================================
void xGLMulMatrixPos(const xVector3d &iTrans)
{
  xDCS DCS = xSetDCS(iTrans);
  glMultMatrixd( (const double *)&DCS );
}


//===========================================================================
// - FUNCTION -
/*!
      Multiply current OpenGL Matrix with a rotation matrix.

      \fn       void xGLMulMatrixRot(const xMatrix33d &iRot)
      \param    iRot  3x3 matrix describing a rotation.
*/
//===========================================================================
void xGLMulMatrixRot(const xMatrix33d &iRot)
{
  xDCS DCS = xSetDCS(iRot);
  glMultMatrixd( (const double *)&DCS );
}


//===========================================================================
// - FUNCTION -
/*!
      First push OpenGL matrix stack then multiply OpenGL Matrix with
      a translation matrix.

      \fn       void xGLPushMatrixPos(const xVector3d &iPos)
      \param    iTrans  3D vector describing a translation.
*/
//===========================================================================
void xGLPushMatrixPos(const xVector3d &iTrans)
{
  glPushMatrix();
  xDCS DCS = xSetDCS(iTrans);
  glMultMatrixd( (const double *)&DCS );
}


//===========================================================================
// - FUNCTION -
/*!
      First push OpenGL matrix stack then multiply OpenGL Matrix with
      a rotation matrix.

      \fn       void xGLPushMatrixRot(const xMatrix33d &iRot)
      \param    iRot  3x3 matrix describing a rotation.
*/
//===========================================================================
void xGLPushMatrixRot(const xMatrix33d &iRot)
{
  glPushMatrix();
  xDCS DCS = xSetDCS(iRot);
  glMultMatrixd( (const double *)&DCS );
}


//===========================================================================
// - FUNCTION -
/*!
      Pop OpenGl top matrix from stack.
      \fn       void xGLPopMatrix()
*/
//===========================================================================
void xGLPopMatrix()
{
  glPopMatrix();
}


//===========================================================================
// END OF FILE
//===========================================================================
