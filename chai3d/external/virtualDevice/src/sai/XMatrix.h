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
//===========================================================================

//---------------------------------------------------------------------------
#ifndef xMatrixH
#define xMatrixH
//---------------------------------------------------------------------------

#include <stdlib.h>.
#include <vcl/dstring.h>
#include <math.h>
#include "gl/gl.h"
#include "CExceptions.h"

//===========================================================================
// - CONSTANTS -
//===========================================================================

#define XPI             3.1415926       // PI CONSTANT.
#define XDEG2RAD	0.01745329252   // CONVERSION DEGREES TO RADIANS.
#define XRAD2DEG	57.2957795131   // CONVERSION RADIANS TO DEGREES.
#define XMIN            1e-49           // SMALLEST VALUE NEAR ZERO.
#define XMAX            MaxInt          // BIGGEST INTEGER.


//===========================================================================
// - STRUCTURE DEFINITION -
// NAME:        xVector2d
/*!
     \struct    xVector2d
     \brief     Definition of a 2D Vector.
     \param     x       x component of vector.
     \param     y       y component of vector.
     \param     tag     value for custom usage.
*/
//===========================================================================
  struct xVector2d
  {
    //! component X of vector.
    double       x;
    //! component Y of vector.
    double       y;
    //! value for custom usage.
    int          tag;
  };


//===========================================================================
// - STRUCTURE DEFINITION -
// NAME:        xVector3d
/*!
     \struct    xVector3d
     \brief     Definition of a 3D Vector.
     \param     x       x component of vector.
     \param     y       y component of vector.
     \param     z       z component of vector.
     \param     tag     value for custom usage.
*/
//===========================================================================
  struct xVector3d
  {
    //! component X of vector.
    double       x;
    //! component Y of vector.
    double       y;
    //! component Z of vector.
    double       z;
    //! value for custom usage.
    int          tag;
  };


//===========================================================================
// - TYPE -
// NAME:        xMatrix22d
/*!
     \struct    xMatrix22d
     \brief     Definition of a 2x2 Matrix.
     \param     m       matrix array 2x2.
     \param     tag     value for custom usage.
*/
//===========================================================================
  struct xMatrix22d
  {
    //! matrice 2x2.
    double      m[2][2];
    //! value for custom usage.
    int         tag;
  };


//===========================================================================
// - TYPE -
// NAME:        xMatrix33d
/*!
     \struct    xMatrix33d
     \brief     Definition of a 3x3 Matrix.
     \param     m       matrix array 3x3.
     \param     tag     value for custom usage.
*/
//===========================================================================
  struct xMatrix33d
  {
    //! matrice 3x3.
    double       m[3][3];
    //! value for custom usage.
    int         tag;
  };


//===========================================================================
// - TYPE -
// NAME:        xDCS
//   M          array   [4]X[4] of double.
/*!
     \struct    xDCS
     \brief     Definition of a 4x4 Matrix for OpenGl matrix format.
     \param     m       matrix array 4x4.
     \param     tag     value for custom usage.
*/
//===========================================================================
  struct xDCS
  {
    //! matrice 4x4.
    double       m[4][4];
    //! value for custom usage.
    int         tag;
  };


//===========================================================================
// - FLOATING POINT OPERATIONS -
//===========================================================================
  //! Return in degrees the cosine of an angle.
  double xCosDeg(const double &iValue);
  //! Return in degrees the sine of an angle.
  double xSinDeg(const double &iValue);
  //! Return in degrees the tangent of an angle.
  double xTanDeg(const double &iValue);
  //! Return in radians the cosine of an angle.
  double xCosRad(const double &iValue);
  //! Return in radian the sine of an angle.
  double xSinRad(const double &iValue);
  //! Return in radian the tangent of an angle.
  double xTanRad(const double &iValue);
  //! Return the conversion of an angle in degrees to radians.
  double xDegToRad(const double &iValue);
  //! Return the conversion of an angle in radians to degrees.
  double xRadToDeg(const double &iValue);
  //! Convert a boolean into a string.
  String xStr(const bool &iValue);
  //! Convert a integer into a string.
  String xStr(const int &iValue);
  //! Convert a double into a string by also giving number of digits.
  String xStr(const double &iValue, const int &iNumberOfDigits);
  //! Convert a string into a double.
  bool xValue(const String &iString, double &iValue);

//===========================================================================
// - VECTORS & MATRICES -
//===========================================================================
//---------------------------------------------------------------------------
// VECTORS:
//---------------------------------------------------------------------------
// INITIALIZATION:
  //! Set a 2D-Vector (iX,iY).
  xVector2d xSet(const double &iX, const double &iY);
  //! Set a 3D-Vector (iX,iY,iZ).
  xVector3d xSet(const double &iX, const double &iY, const double &iZ);

// ADDITION:
  //! Return the addition of two 2D-Vectors.
  xVector2d xAdd(const xVector2d &iVect0, const xVector2d &iVect1);
  //! Return the addition of two 3D-Vectors.
  xVector3d xAdd(const xVector3d &iVect0, const xVector3d &iVect1);

// SUBSTRACTION:
  //! Return the substraction of a 2D-Vector by a 2D-Vector.
  xVector2d xSub(const xVector2d &iVect0, const xVector2d &iVect1);
  //! Return the substraction of a 3D-Vector by a 3D-Vector.
  xVector3d xSub(const xVector3d &iVect0, const xVector3d &iVect1);

// MULTIPLICATION:
  //! Return the multiplication of a 2D-Vector by a scalar.
  xVector2d xMul(const double &iK, const xVector2d &iVect);
  //! Return the multiplication of a 3D-Vector by a scalar.
  xVector3d xMul(const double &iK, const xVector3d &iVect);

// CROSS PRODUCT:
  //! Return the cross product between two 2D-Vectors. (Returns a 3D-Vector)
  xVector3d xCross(const xVector2d &iVect0, const xVector2d &iVect1);
  //! Return the cross product between two 3D-Vectors.
  xVector3d xCross(const xVector3d &iVect0, const xVector3d &iVect1);

// DOT PRODUCT:
  //! Return the dot product between two 2D-Vectors.
  double xDot(const xVector2d &iVect0, const xVector2d &iVect1);
  //! Return the dot product between two 3D-Vectors.
  double xDot(const xVector3d &iVect0, const xVector3d &iVect1);

// NORMALIZATION (Length of Vector = 1):
  //! Return the normalized vector of a given 2D-Vector.
  xVector2d xNormalize(const xVector2d &iVect);
  //! Return the normalized vector of a given 3D-Vector.
  xVector3d xNormalize(const xVector3d &iVect);

// DISTANCE BETWEEN TWO POINTS:
  //! Return the distance between two points (vectors) in 2D-Space.
  double xDist(const xVector2d &iPoint0, const xVector2d &iPoint1);
  //! Return the distance between two points (vectors) in 3D-Space.
  double xDist(const xVector3d &iPoint0, const xVector3d &iPoint1);

// LENGTH OF A VECTOR:
  //! Return the length (norm) of a 2D-Vector.
  double xNorm(const xVector2d &iVect);
  //! Return the length (norm) of a 3D-Vector.
  double xNorm(const xVector3d &iVect);

// INVERSE VECTOR (Vector in opposite direction):
  //! Return the opposite 2D-Vector of a given vector.
  xVector2d xInv(const xVector2d &iVect);
  //! Return the opposite 3D-Vector of a given vector.
  xVector3d xInv(const xVector3d &iVect);

// COMPARING TWO VECTORS:
  //! Return TRUE if two 2D-Vectors are equal.
  bool xEqual(const xVector2d &iVect0, const xVector2d &iVect1);
  //! Return TRUE if two 3D-Vectors are equal.
  bool xEqual(const xVector3d &iVect0, const xVector3d &iVect1);

// STRING CONVERTION:
  //! Return a 2D-Vector converted into a string with a given number of digits.
  String xStr(const xVector2d &iVect, const int &iNumberOfDigits);
  //! Return a 3D-Vector converted into a string with a given number of digits.
  String xStr(const xVector3d &iVect, const int &iNumberOfDigits);


//---------------------------------------------------------------------------
// MATRICES:
//---------------------------------------------------------------------------
// INITIALIZATION:
  //! Set a 2D-Matrix from a set of doubles.
  xMatrix22d xSetMat(const double &iM00, const double &iM01,
                      const double &iM10, const double &iM11);

  //! Set a 3D-Matrix from a set of doubles.
  xMatrix33d xSetMat(const double &iM00, const double &iM01, const double &iM02,
                      const double &iM10, const double &iM11, const double &iM12,
                      const double &iM20, const double &iM21, const double &iM22);

  //! Set a 2D-Matrix from a set of column 2D-Vectors.
  xMatrix22d xSetMat(const xVector2d &iVectCol0, const xVector2d &iVectCol1);

  //! Set a 3D-Matrix from a set of column 3D-Vectors.
  xMatrix33d xSetMat(const xVector3d &iVectCol0, const xVector3d &iVectCol1,
                      const xVector3d &iVectCol2);

// IDENTITY MATRICES:
  //! Return a 2D Identity Matrix.
  xMatrix22d xIdentity22d(void);
  //! Return a 3D Identity Matrix.
  xMatrix33d xIdentity33d(void);

// MULTIPLICATIONS:
  //! Return the multiplication of a 2D-Matrix by a second 2D-Matrix.
  xMatrix22d xMul(const xMatrix22d &iMat0, const xMatrix22d &iMat1);
  //! Return the multiplication of a 3D-Matrix by a second 3D-Matrix.
  xMatrix33d xMul(const xMatrix33d &iMat0, const xMatrix33d &iMat1);
  //! Return the multiplication of a 2D-Matrix with a 2D-Vector.
  xVector2d xMul(const xMatrix22d &iMat, const xVector2d &iVect);
  //! Return the multiplication of a 3D-Matrix with a 3D-Vector.
  xVector3d xMul(const xMatrix33d &iMat, const xVector3d &iVect);

// DETERMINANT OF A MATRIX:
  //! Return the determinant of a 2D-Matrix.
  double xDet(const xMatrix22d &iMat);
  //! Return the determinant of a 3D-Matrix.
  double xDet(const xMatrix33d &iMat);

// TRANSPOSE A MATRIX:
  //! Return the transpose of a 2D-Matrix.
  xMatrix22d xTrans(const xMatrix22d &iMat);
  //! Return the transpose of a 3D-Matrix.
  xMatrix33d xTrans(const xMatrix33d &iMat);

// INVERSE A MATRIX:
  //! Return the inverse of a 2D-Matrix.
  xMatrix22d xInv(const xMatrix22d &iMat);
  //! Return the inverse of a 3D-Matrix.
  xMatrix33d xInv(const xMatrix33d &iMat);

// ROTATION MATRICES:
  //! Return a rotation matrix given a rotation axis and an angle.
  xMatrix33d xRotMatrix(const xVector3d &iAxis, const double &iAngleRad);
  //! Return a matrix after rotation arround an axis and an angle.
  xMatrix33d xRotate(const xMatrix33d &iMat,
                     const xVector3d &iAxis, const double &iAngleRad);

// CONVERT A MATRIX INTO A STRING:
  //! Convert a 2D-Matrix into a string by giving the number of digits for each scalar.
  String xStr(const xMatrix22d &iMat, const int &iNumberOfDigits);
  //! Convert a 3D-Matrix into a string by giving the number of digits for each scalar.
  String xStr(const xMatrix33d &iMat, const int &iNumberOfDigits);


//===========================================================================
// - OPEN GL COMMANDS -
//===========================================================================

  // DCS MATRICES (Dynamic Coordinate System):
  //! Set a DCS translation matrix for OpenGl. Do not use directly.
  xDCS xSetDCS(const xVector3d &iTrans);
  //! Set a DCS rotation matrix for OpenGl. Do not use directly.
  xDCS xSetDCS(const xVector3d &iRot);

  // OPEN GL MATRIX COMMANDS:
  //! Multiply current openGl matrix by a translation matrix.
  void xGLMulMatrixPos(const xVector3d &iTrans);
  //! Multiply current openGl matrix by a rotation matrix.
  void xGLMulMatrixRot(const xMatrix33d &iRot);
  //! Push current openGl matrix and apply a translation.
  void xGLPushMatrixPos(const xVector3d &iTrans);
  //! Push current openGl matrix and apply a rotation.
  void xGLPushMatrixRot(const xMatrix33d &iRot);
  //! Pop current OpenGl matrix.
  void xGLPopMatrix();

#endif

//===========================================================================
// END OF FILE
//===========================================================================
