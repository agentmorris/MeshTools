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
#ifndef CMathsH
#define CMathsH
//---------------------------------------------------------------------------
#include "CMatrix3d.h"
#include <math.h>
//---------------------------------------------------------------------------
/*!
	\file   CMaths.h
	\brief
	Various mathematical utility functions
*/
//===========================================================================
/*!
	Check if \e value is equal or near zero.

	\param    a_value  Value to be checked.
	\return   Returns \b true if it's almost zero, otherwise \b false.
*/
//===========================================================================
inline bool cZero(const double& a_value)
{
	return ((a_value < CHAI_TINY) && (a_value > -CHAI_TINY));
}


//===========================================================================
/*!
	Check if \e value is strictly positive and less than \e maxBound in case
	\e maxBound is positive.

	\param    a_value  Value to be checked.
	\param    a_boundMax  Positive bound.
	\return   Return \b true if \e value is greater than 0 and either
			  \e maxBound is negative, or \e value is less than\e maxBound.
			  Otherwise return \b false.
*/
//===========================================================================
inline bool cPositiveBound(const double& a_value, const double& a_boundMax)
{
	return ((a_value > CHAI_TINY) && ((a_boundMax < 0) || (a_value < a_boundMax)));
}


//===========================================================================
/*!
	Compute absolute value.

	\param    a_value  Input value
	\return   Return |a_value|
*/
//===========================================================================
template<class T> inline T cAbs(const T a_value)
{
	return (a_value >= 0 ? a_value : -a_value);
}


//===========================================================================
/*!
	Compute maximum between two values

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\return   Return maximum of a_value1 and a_value2.

	Note that this function should _not_ be modified to take inputs by
	reference.
*/
//===========================================================================
template<class T> inline T cMax(const T a_value1, const T a_value2)
{
	return (a_value1 >= a_value2 ? a_value1 : a_value2);
}


//===========================================================================
/*!
	Compute minimum between two values

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\return   Return maximum of a_value1 and a_value2.

	Note that this function should _not_ be modified to take inputs by
	reference.
*/
//===========================================================================
template<class T> inline T cMin(const T a_value1, const T a_value2)
{
	return (a_value1 <= a_value2 ? a_value1 : a_value2);
}


//===========================================================================
/*!
	Compute maximum of 3 values.

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\param    a_value3  Third value.
	\return   Return maximum of a_value1, a_value2 and a_value3.
*/
//===========================================================================
template<class T> inline T cMax3(const T& a_value1, const T& a_value2, const T& a_value3)
{
	return (cMax(a_value1, cMax(a_value2, a_value3)));
}


//===========================================================================
/*!
	Return minimum of 3 values.

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\param    a_value3  Third value.
	\return   Return minimum of a_value1, a_value2 and a_value3.
*/
//===========================================================================
template<class T> inline T cMin3(const T& a_value1, const T& a_value2, const T& a_value3)
{
	return (cMin(a_value1, cMin(a_value2, a_value3)));
}


//===========================================================================
/*!
	Compute maximum of absolute values of 2 numbers.

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\return   Return max(abs(p), abs(q)).

	Note that this function should _not_ be modified to take inputs by
	reference.
*/
//===========================================================================
template<class T> inline T cMaxAbs(const T a_value1, const T a_value2)
{
	return (cAbs(a_value1) >= cAbs(a_value2) ? a_value1 : a_value2);
}


//===========================================================================
/*!
	Compute minimum of absolute values of 2 values.

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\return   Return min(abs(p), abs(q)).

	Note that this function should _not_ be modified to take inputs by
	reference.
*/
//===========================================================================
template<class T> inline T cMinAbs(const T a_value1, const T a_value2)
{
	return (cAbs(a_value1) <= cAbs(a_value2) ? a_value1 : a_value2);
}


//===========================================================================
/*!
	Compute maximum of absolute values of 3 values.

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\param    a_value3  Third value.
	\return   Return max(abs(p), abs(q), abs(r)).
*/
//===========================================================================
template<class T> inline T cMax3Abs(const T& a_value1, const T& a_value2, const T& a_value3)
{
	return cMaxAbs(a_value1, cMaxAbs(a_value2, a_value3));
}


//===========================================================================
/*!
	Compute minimum of absolute values of 3 values.

	\param    a_value1  First value.
	\param    a_value2  Second value.
	\param    a_value3  Third value.
	\return   Return min(abs(p), abs(q), abs(r)).
*/
//===========================================================================
template<class T> inline T cMin3Abs(const T& a_value1, const T& a_value2, const T& a_value3)
{
	return cMinAbs(a_value1, cMinAbs(a_value2, a_value3));
}


//===========================================================================
/*!
	Swap two elements.

	\param    a_value1  First value.
	\param    a_value2  Second value.
*/
//===========================================================================
template<class T> inline void cSwap(T &a_value1, T &a_value2)
{
	T value = a_value1;
	a_value1 = a_value2;
	a_value2 = value;
}


//===========================================================================
/*!
	Linear interpolation from \e value0 (when a=0) to \e value1 (when a=1).

	\param    a_level  Interpolation parameter.
	\param    a_value1  First value.
	\param    a_value2  Second value.
	\return   Return an interpolated result: (1-a_level) * a_value1 + a_level * a_value2
*/
//===========================================================================
template<class T> inline T cLerp(const double& a_level, const T& a_value1, const T& a_value2)
{
	return (a_value2 * a_level + a_value1 * (1 - a_level));
}


//===========================================================================
/*!
	Clamp the input to the specified range.

	\param    a_value  Input value.
	\param    a_low  Low boundary.
	\param    a_high  High boundary.
	\return   Return clamped value.

	Note that this function should _not_ be modified to take inputs by
	reference.
*/
//===========================================================================
template<class T> inline T cClamp(const T a_value, const T a_low, const T a_high)
{
	return (a_value < a_low ? a_low : a_value > a_high ? a_high : a_value);
}


//===========================================================================
/*!
	Clamp the input to the range 0 - infinity.

	\param    a_value  Input value.
	\return   Return clamped value.
*/
//===========================================================================
template<class T> inline T cClamp0(T& a_value)
{
	return cMax<T>(0, a_value);
}


//===========================================================================
/*!
	Clamp the input to the range [0,1].

	\param    a_value  Input value of type double.
	\return   Return clamped value.
*/
//===========================================================================
inline double cClamp01(double& a_value)
{
	return (cClamp(a_value, 0.0, 1.0));
}


//===========================================================================
/*!
	Check whether input is in the range of [low, high].

	\param    a_value  Input value.
	\param    a_low  Low boundary.
	\param    a_high  High boundary.
	\return   Return \b true if \e value is in the rage of [low, high]
				and \b false otherwise.
*/
//===========================================================================
template<class T, class V> inline bool cContains(const T& a_value, const V& a_low, const V& a_high)
{
	return ((a_value >= a_low) && (a_value <= a_high));
}


//===========================================================================
/*!
	Compute the square of a double.

	\param    a_value  Input value.
	\return   Return (/e value * /e value).
*/
//===========================================================================
inline double cSqr(const double& a_value)
{
	return(a_value*a_value);
}


//===========================================================================
/*!
	Compute the cosine of an angle defined in degrees.

	\param    a_angleDeg  Angle in degrees.
	\return   Return the cosine of angle.
*/
//===========================================================================
inline double cCosDeg(const double& a_angleDeg)
{
	return (cos(a_angleDeg * CHAI_DEG2RAD));
}


//===========================================================================
/*!
	Compute the sine of an angle defined in degrees.

	\param    a_angleDeg  Angle in degrees.
	\return   Return the sine of angle.
*/
//===========================================================================
inline double cSinDeg(const double& a_angleDeg)
{
	return (sin(a_angleDeg * CHAI_DEG2RAD));
}


//===========================================================================
/*!
	Compute the tangent of an angle defined in degrees.

	\param    a_angleDeg  Angle in degrees.
	\return   Return the tangent of angle.
*/
//===========================================================================
inline double cTanDeg(const double& a_angleDeg)
{
	return (tan(a_angleDeg * CHAI_DEG2RAD));
}


//===========================================================================
/*!
	Return the cosine of an angle defined in radians.

	\param    a_angleRad  Angle in radians.
	\return   Return the cosine of angle.
*/
//===========================================================================
inline double cCosRad(const double& a_angleRad)
{
	return (cos(a_angleRad));
}


//===========================================================================
/*!
	Return the sine of an angle defined in radians.

	\param    a_value Angle in radians.
	\return   Return the sine of angle a_value.
*/
//===========================================================================
inline double cSinRad(const double& a_value)
{
	return (sin(a_value));
}


//===========================================================================
/*!
	Return the tangent of an angle defined in radians.

	\param    a_value  Angle in radians.
	\return   Return the tangent of angle a_value.
*/
//===========================================================================
inline double cTanRad(const double& a_value)
{
	return (tan(a_value));
}


//===========================================================================
/*!
	Convert an angle from degrees to radians.

	\param    a_angleDeg  Angle in degrees.
	\return   Return angle in radians.
*/
//===========================================================================
inline double cDegToRad(const double& a_angleDeg)
{
	return (a_angleDeg * CHAI_DEG2RAD);
}


//===========================================================================
/*!
	Convert an angle from radians to degrees

	\param    a_angleRad  Angle in radians.
	\return   Return angle in degrees.
*/
//===========================================================================
inline double cRadToDeg(const double& a_angleRad)
{
	return (a_angleRad * CHAI_RAD2DEG);
}


//===========================================================================
/*!
	Compute the addition between two vectors. Result = Vector1 + Vector2.

	\param    a_vector1  First vector.
	\param    a_vector2  Second vector.
	\return   Return the addition of both vectors.
*/
//===========================================================================
inline cVector3d cAdd(const cVector3d& a_vector1, const cVector3d& a_vector2)
{
	return cVector3d(
		a_vector1.x + a_vector2.x,
		a_vector1.y + a_vector2.y,
		a_vector1.z + a_vector2.z);
}


//===========================================================================
/*!
	Compute the addition between three vectors.
	Result = Vector1 + Vector2 + Vector3.

	\param    a_vector1  First vector.
	\param    a_vector2  Second vector.
	\param    a_vector3  Third vector.
	\return   Return the addition of all three vectors.
*/
//===========================================================================
inline cVector3d cAdd(const cVector3d& a_vector1, const cVector3d& a_vector2, const cVector3d& a_vector3)
{
	return cVector3d(
		a_vector1.x + a_vector2.x + a_vector3.x,
		a_vector1.y + a_vector2.y + a_vector3.y,
		a_vector1.z + a_vector2.z + a_vector3.z);
}


//===========================================================================
/*!
	Compute the subtraction between two vectors. Result = Vector1 - Vector2.

	\param    a_vector1  First vector.
	\param    a_vector2  Second vector.
	\return   Return result of subtraction.
*/
//===========================================================================
inline cVector3d cSub(const cVector3d& a_vector1, const cVector3d& a_vector2)
{
	return cVector3d(
		a_vector1.x - a_vector2.x,
		a_vector1.y - a_vector2.y,
		a_vector1.z - a_vector2.z);
}


//===========================================================================
/*!
	Compute the negated vector of a input vector.

	\param    a_vector  Input vector.
	\return   Return (-a_vector).
*/
//===========================================================================
inline cVector3d cNegate(const cVector3d& a_vector)
{
	return cVector3d(a_vector.x*-1.0, a_vector.y*-1.0, a_vector.z*-1.0);
}


//===========================================================================
/*!
	Multiply a vector by a scalar.

	\param    a_value   Scalar.
	\param    a_vector  Input vector.
	\return   Returns result of multiplication.
*/
//===========================================================================
inline cVector3d cMul(const double& a_value, const cVector3d& a_vector)
{
	return cVector3d(a_vector.x*a_value, a_vector.y*a_value, a_vector.z*a_value);
}


//===========================================================================
/*!
	Divide a vector by a scalar.

	\param    a_value   Scalar.
	\param    a_vector  Input vector.
	\return   Returns result of division.
*/
//===========================================================================
inline cVector3d cDiv(const double& a_value, const cVector3d& a_vector)
{
	return cVector3d(a_vector.x / a_value, a_vector.y / a_value, a_vector.z / a_value);
}


//===========================================================================
/*!
	Divide a scalar by components of a 3D vector and return vector

	\param    a_value   Scalar.
	\param    a_vector  Input vector.
	\return   Returns a vector. \e result = \e value / \e vector.
*/
//===========================================================================
inline cVector3d cDivVect(const double& a_value, const cVector3d& a_vector)
{
	return cVector3d(
		a_value / a_vector.x, a_value / a_vector.y, a_value / a_vector.z
	);
}


//===========================================================================
/*!
	Compute the cross product between two 3D vectors.

	\param    a_vector1  First vector.
	\param    a_vector2  Second vector.
	\return   Returns the cross product between both vectors.
*/
//===========================================================================
inline cVector3d cCross(const cVector3d& a_vector1, const cVector3d& a_vector2)
{
	cVector3d result;
	a_vector1.crossr(a_vector2, result);
	return (result);
}


//===========================================================================
/*!
	Compute the dot product between two vectors.

	\param    a_vector1  First vector.
	\param    a_vector2  Second vector.
	\return   Returns the dot product between between both vectors.
*/
//===========================================================================
inline double cDot(const cVector3d& a_vector1, const cVector3d& a_vector2)
{
	return(a_vector1.dot(a_vector2));
}


//===========================================================================
/*!
	Compute the normalized vector (length = 1) of an input vector.

	\param    a_vector  Input vector.
	\return   Returns the normalized vector.
*/
//===========================================================================
inline cVector3d cNormalize(const cVector3d& a_vector)
{
	cVector3d result;
	a_vector.normalizer(result);
	return (result);
}


//===========================================================================
/*!
	Compute the distance between two points.

	\param    a_point1  First point.
	\param    a_point2  Second point.
	\return   Return distance between points
*/
//===========================================================================
inline double cDistance(const cVector3d& a_point1, const cVector3d& a_point2)
{
	return (a_point1.distance(a_point2));
}


//===========================================================================
/*!
Compute the squared distance between two points.

\param    a_point1  First point.
\param    a_point2  Second point.
\return   Return squared distance between points
*/
//===========================================================================
inline double cDistanceSq(const cVector3d& a_point1, const cVector3d& a_point2)
{
	return (a_point1.distancesq(a_point2));
}


//===========================================================================
/*!
	 Determine whether two vectors represent the same point.

	 \fn       bool inline cEqualPoints(const cVector3d& v1, const cVector3d& v2,
						   const double epsilon=CHAI_SMALL)
	 \param    v1  First point.
	 \param    v2  Second point.
	 \param    epsilon  Two points will be considered equal if each
						component is within epsilon units.  Defaults
						to CHAI_SMALL.
	 \return   Return whether the two vectors represent the same point.
*/
//===========================================================================
bool inline cEqualPoints(const cVector3d& v1, const cVector3d& v2, const double epsilon = CHAI_SMALL)
{
	// Accelerated path for exact equality
	if (epsilon == 0.0) {
		if ((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z)) return true;
		else return false;
	}

	if ((fabs(v1.x - v2.x) < epsilon) &&
		(fabs(v1.y - v2.y) < epsilon) &&
		(fabs(v1.z - v2.z) < epsilon)) return true;
	else return false;
}


//===========================================================================
/*!
	Return the Identity Matrix

	\return   Return the identity matrix.
*/
//===========================================================================
inline cMatrix3d cIdentity3d(void)
{
	cMatrix3d result;
	result.identity();
	return (result);
}


//===========================================================================
/*!
	Compute the multiplication between two matrices.

	\param    a_matrix1  First matrix.
	\param    a_matrix2  Second matrix.
	\return   Returns multiplication of /e matrix1 * /e matrix2.
*/
//===========================================================================
inline cMatrix3d cMul(const cMatrix3d& a_matrix1, const cMatrix3d& a_matrix2)
{
	cMatrix3d result;
	a_matrix1.mulr(a_matrix2, result);
	return (result);
}


//===========================================================================
/*!
	Compute the multiplication of a matrix and a vector.

	\param    a_matrix  Input matrix.
	\param    a_vector  Input vector.
	\return   Returns the multiplication of the matrix and vector.
*/
//===========================================================================
inline cVector3d cMul(const cMatrix3d& a_matrix, const cVector3d& a_vector)
{
	cVector3d result;
	a_matrix.mulr(a_vector, result);
	return(result);
}


//===========================================================================
/*!
	Compute the transpose of a matrix

	\param    a_matrix  Input matrix.
	\return   Returns the transpose of the input matrix.
*/
//===========================================================================
inline cMatrix3d cTrans(const cMatrix3d& a_matrix)
{
	cMatrix3d result;
	a_matrix.transr(result);
	return(result);
}


//===========================================================================
/*!
	Compute the inverse of a matrix.

	\param    a_matrix  Input matrix.
	\return   Returns the inverse of the input matrix.
*/
//===========================================================================
inline cMatrix3d cInv(const cMatrix3d& a_matrix)
{
	cMatrix3d result;
	a_matrix.invertr(result);
	return(result);
}


//===========================================================================
/*!
	Compute the angle between two vectors (in radians).

	\param    a_vector0  Input vector.
	\param    a_vector1  Input vector.
	\return   Returns the angle (in radians) between the input vectors.
*/
//===========================================================================
inline double cAngle(const cVector3d& a_vector0, const cVector3d& a_vector1)
{

	double n0 = a_vector0.length();
	double n1 = a_vector1.length();
	double val = a_vector0.dot(a_vector1) / (n0*n1);

	if (val > 1.0) { val = 1.0; }
	else if (val < -1.0) { val = -1.0; }

	return(acos(val));
}


//===========================================================================
/*!
	Compute cosine of the angle between two vectors.

	\param    a_vector0  Input vector.
	\param    a_vector1  Input vector.
	\return   Returns the cosine of the angle between the input vectors.
*/
//===========================================================================
inline double cCosAngle(const cVector3d& a_vector0, const cVector3d& a_vector1)
{
	double n0 = a_vector0.length();
	double n1 = a_vector1.length();

	return(a_vector0.dot(a_vector1) / (n0*n1));
}


//===========================================================================
/*!
	Compute a rotation matrix given a rotation \e axis and an \e angle.

	\param    a_axis  Axis of rotation.
	\param    a_angleRad  Rotation angle in Radian.
	\return   Returns a rotation matrix.
*/
//===========================================================================
inline cMatrix3d cRotMatrix(const cVector3d& a_axis, const double& a_angleRad)
{
	cMatrix3d result;
	result.set(a_axis, a_angleRad);
	return (result);
}


//===========================================================================
/*!
	Compute the rotation of a matrix around an \e axis and an \e angle.

	\param    a_matrix  Input matrix.
	\param    a_axis  Axis of rotation.
	\param    a_angleRad  Rotation angle in Radian.
	\return   Returns input matrix after rotation.
*/
//===========================================================================
inline cMatrix3d cRotate(const cMatrix3d& a_matrix, const cVector3d& a_axis,
	const double& a_angleRad)
{
	cMatrix3d result;
	a_matrix.rotater(a_axis, a_angleRad, result);
	return (result);
}


//===========================================================================
/*!
Compute the projection of a point on a plane. the plane is expressed
by a point and a surface normal.

\param    a_point  Point to project on plane.
\param    a_planePoint   Point on plane.
\param    n              Plane normal.
\return   Returns the projection of \e a_point on plane.
*/
//===========================================================================
inline cVector3d cProjectPointOnPlane(const cVector3d& a_point,
	const cVector3d& a_planePoint, const cVector3d& n)
{
	// compute a projection matrix
	cMatrix3d projectionMatrix;

	projectionMatrix.set(
		(n.y * n.y) + (n.z * n.z), -(n.x * n.y), -(n.x * n.z),
		-(n.y * n.x), (n.x * n.x) + (n.z * n.z), -(n.y * n.z),
		-(n.z * n.x), -(n.z * n.y), (n.x * n.x) + (n.y * n.y));

	// project point on plane and return projected point.
	cVector3d point;
	a_point.subr(a_planePoint, point);
	projectionMatrix.mul(point);
	point.add(a_planePoint);

	// return result
	return (point);
}


//===========================================================================
/*!
	Compute the projection of a point on a plane. the plane is expressed
	by a set of three points.

	\param    a_point  Point to project on plane.
	\param    a_planePoint0  Point 0 on plane.
	\param    a_planePoint1  Point 1 on plane.
	\param    a_planePoint2  Point 2 on plane.
	\return   Returns the projection of \e a_point on plane.
*/
//===========================================================================
inline cVector3d cProjectPointOnPlane(const cVector3d& a_point,
	const cVector3d& a_planePoint0, const cVector3d& a_planePoint1,
	const cVector3d& a_planePoint2)
{
	// create two vectors from the three input points lying in the projection
	// plane.
	cVector3d v01, v02;
	a_planePoint1.subr(a_planePoint0, v01);
	a_planePoint2.subr(a_planePoint0, v02);

	// compute the normal vector of the plane
	cVector3d n;
	v01.crossr(v02, n);
	n.normalize();

	return cProjectPointOnPlane(a_point, a_planePoint0, n);
}


//===========================================================================
/*!
	Compute the projection of a point on a line. the line is expressed
	by a point located on the line and a direction vector.

	\param    a_point  Point to project on the line.
	\param    a_pointOnLine  Point located on the line
	\param    a_directionOfLine  Vector expressing the direction of the line
	\return   Returns the projection of \e a_point on the line.
*/
//===========================================================================
inline cVector3d cProjectPointOnLine(const cVector3d& a_point,
	const cVector3d& a_pointOnLine, const cVector3d& a_directionOfLine)
{
	cVector3d point, result;

	double lengthDirSq = a_directionOfLine.lengthsq();
	a_point.subr(a_pointOnLine, point);

	a_directionOfLine.mulr((point.dot(a_directionOfLine) / (lengthDirSq)),
		result);

	result.add(a_pointOnLine);

	return (result);
}


//===========================================================================
/*!
	Project a vector V0 onto a second vector V1.

	\param    a_vector0  Vector 0.
	\param    a_vector1  Vector 1.
	\return   Returns the projection of \e a_point on the line.
*/
//===========================================================================
inline cVector3d cProject(const cVector3d& a_vector0, const cVector3d& a_vector1)
{
	cVector3d result;

	double lengthSq = a_vector1.lengthsq();

	a_vector1.mulr((a_vector0.dot(a_vector1) / (lengthSq)), result);

	return (result);
}


//===========================================================================
/*!
	Compute the normal of a surface defined by three point passed as
	parameters.

	\param    a_surfacePoint0  Point 0 of surface.
	\param    a_surfacePoint1  Point 1 of surface.
	\param    a_surfacePoint2  Point 2 of surface.
	\return     Return surface normal.
*/
//===========================================================================
inline cVector3d cComputeSurfaceNormal(const cVector3d& a_surfacePoint0,
	const cVector3d& a_surfacePoint1, const cVector3d& a_surfacePoint2)
{
	cVector3d v01, v02, result;

	// compute surface normal
	a_surfacePoint1.subr(a_surfacePoint0, v01);
	a_surfacePoint2.subr(a_surfacePoint0, v02);
	v01.normalize();
	v02.normalize();
	v01.crossr(v02, result);
	result.normalize();

	// return result
	return (result);
}

//===========================================================================
/*!
	Returns true if p is contained in the bounding box defined by min and max

	\param    p Test point
	\param    box_min Minimum coordinate in the boundary box
	\param    box_max Maximum coordinate in the boundary box
	\return   Returns true if p is in [box_min,box_max], inclusive
*/
//===========================================================================
inline bool cBoxContains(const cVector3d& p, const cVector3d& box_min, const cVector3d& box_max)
{
	if ((p.x >= box_min.x) && (p.x <= box_max.x) &&
		(p.y >= box_min.y) && (p.y <= box_max.y) &&
		(p.z >= box_min.z) && (p.z <= box_max.z))
		return true;
	return false;
}

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------


