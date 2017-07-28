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
/*!
	\file CMacrosGL.h
*/
//---------------------------------------------------------------------------
#ifndef CMacrosGLH
#define CMacrosGLH
//---------------------------------------------------------------------------
#include "CVector3d.h"
#include "CMatrix3d.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
//---------------------------------------------------------------------------

#ifdef _MSVC
#include <conio.h>
#define CHAI_DEBUG_PRINT _cprintf
#else 
#define CHAI_DEBUG_PRINT printf
#endif

//! Align the current -z axis with a reference frame; a la gluLookAt
void cLookAt(const cVector3d& a_eye, const cVector3d& a_at, const cVector3d& a_up);

//===========================================================================
/*!
	  \struct   cMatrixGL
	  \brief    CHAI describes rotations using 3x3 rotation matrices (cMatrix3d)
				and 3D vectors (cVector3d) to express position or translation.
				On the OpenGL side 4x4 matrices are required to perform all
				geometrical transformations. cMatrixGL provides a structure
				which encapsulates all the necessary functionality to generate 4x4
				OpenGL transformation matrices from 3D position vectors and rotation
				matrices.

				cMatrixGL also provides OpenGL calls to push, multiply and pop
				matrices off the OpenGL stack.

				Note that OpenGL Matrices are COLUMN major, but CHAI matrices
				(and all other matrices in the universe) are ROW major.
*/
//===========================================================================
struct cMatrixGL
{
private:
	//! array of type \e double, defining the actual transformation
	double  m[4][4];

public:

	//-----------------------------------------------------------------------
	/*!
		  Default constructor
	*/
	//-----------------------------------------------------------------------
	cMatrixGL()
	{
		identity();
	}

	//-----------------------------------------------------------------------
	/*!
		  Returns a pointer to the matrix array in memory.

		  \return   Returns a pointer of type \e double.
	*/
	//-----------------------------------------------------------------------
	const double* pMatrix() const { return m[0]; }


	//-----------------------------------------------------------------------
	/*!
		  Creates OpenGL translation matrix from a position vector passed as
		  parameter.

		  \param    a_pos   Input vector.
	*/
	//-----------------------------------------------------------------------
	inline void set(const cVector3d& a_pos)
	{
		m[0][0] = 1.0;      m[0][1] = 0.0;       m[0][2] = 0.0;       m[0][3] = 0.0;
		m[1][0] = 0.0;      m[1][1] = 1.0;       m[1][2] = 0.0;       m[1][3] = 0.0;
		m[2][0] = 0.0;      m[2][1] = 0.0;       m[2][2] = 1.0;       m[2][3] = 0.0;
		m[3][0] = a_pos.x;  m[3][1] = a_pos.y;   m[3][2] = a_pos.z;   m[3][3] = 1.0;
	}


	//-----------------------------------------------------------------------
	/*!
		  Extract the translational component of this matrix
	*/
	//-----------------------------------------------------------------------
	inline cVector3d getPos() const
	{
		return cVector3d(m[3][0], m[3][1], m[3][2]);
	}


	//-----------------------------------------------------------------------
	/*!
		  Extract the rotational component of this matrix
	*/
	//-----------------------------------------------------------------------
	inline cMatrix3d getRot() const
	{
		cMatrix3d mat;
		mat.set(m[0][0], m[1][0], m[2][0],
			m[0][1], m[1][1], m[2][1],
			m[0][2], m[1][2], m[2][2]);
		return mat;
	}


	//-----------------------------------------------------------------------
	/*!
		  Create an OpenGL rotation matrix from a 3x3 rotation matrix passed
		  as a parameter.

		  \param    a_rot  The source rotation matrix
	*/
	//-----------------------------------------------------------------------
	void set(const cMatrix3d& a_rot)
	{
		m[0][0] = a_rot.m[0][0];  m[0][1] = a_rot.m[1][0];  m[0][2] = a_rot.m[2][0];  m[0][3] = 0.0;
		m[1][0] = a_rot.m[0][1];  m[1][1] = a_rot.m[1][1];  m[1][2] = a_rot.m[2][1];  m[1][3] = 0.0;
		m[2][0] = a_rot.m[0][2];  m[2][1] = a_rot.m[1][2];  m[2][2] = a_rot.m[2][2];  m[2][3] = 0.0;
		m[3][0] = 0.0;            m[3][1] = 0.0;            m[3][2] = 0.0;            m[3][3] = 1.0;
	}


	//-----------------------------------------------------------------------
	/*!
		  Create an OpenGL translation matrix from a 3-vector and a 3x3 matrix
		  passed as  a parameter.

		  \param    a_pos   Translational component of the transformation
		  \param    a_rot   Rotational component of the transformation
	*/
	//-----------------------------------------------------------------------
	void set(const cVector3d& a_pos, const cMatrix3d& a_rot)
	{
		m[0][0] = a_rot.m[0][0];  m[0][1] = a_rot.m[1][0];  m[0][2] = a_rot.m[2][0];  m[0][3] = 0.0;
		m[1][0] = a_rot.m[0][1];  m[1][1] = a_rot.m[1][1];  m[1][2] = a_rot.m[2][1];  m[1][3] = 0.0;
		m[2][0] = a_rot.m[0][2];  m[2][1] = a_rot.m[1][2];  m[2][2] = a_rot.m[2][2];  m[2][3] = 0.0;
		m[3][0] = a_pos.x;        m[3][1] = a_pos.y;        m[3][2] = a_pos.z;        m[3][3] = 1.0;
	}


	//-----------------------------------------------------------------------
	/*!
		Copy the current matrix to an external matrix passed as a parameter

		\param    a_destination  Destination matrix
	*/
	//-----------------------------------------------------------------------
	inline void copyto(cMatrixGL& a_destination) const
	{
		a_destination.m[0][0] = m[0][0];  a_destination.m[0][1] = m[0][1];
		a_destination.m[0][2] = m[0][2];  a_destination.m[0][3] = m[0][3];
		a_destination.m[1][0] = m[1][0];  a_destination.m[1][1] = m[1][1];
		a_destination.m[1][2] = m[1][2];  a_destination.m[1][3] = m[1][3];
		a_destination.m[2][0] = m[2][0];  a_destination.m[2][1] = m[2][1];
		a_destination.m[2][2] = m[2][2];  a_destination.m[2][3] = m[2][3];
		a_destination.m[3][0] = m[3][0];  a_destination.m[3][1] = m[3][1];
		a_destination.m[3][2] = m[3][2];  a_destination.m[3][3] = m[3][3];
	}


	//-----------------------------------------------------------------------
	/*!
		Copy values from an external matrix passed as parameter to this matrix

		\param    a_source  Source matrix
	*/
	//-----------------------------------------------------------------------
	inline void copyfrom(const cMatrixGL& a_source)
	{
		m[0][0] = a_source.m[0][0];  m[0][1] = a_source.m[0][1];
		m[0][2] = a_source.m[0][2];  m[0][3] = a_source.m[0][3];
		m[1][0] = a_source.m[1][0];  m[1][1] = a_source.m[1][1];
		m[1][2] = a_source.m[1][2];  m[1][3] = a_source.m[1][3];
		m[2][0] = a_source.m[2][0];  m[2][1] = a_source.m[2][1];
		m[2][2] = a_source.m[2][2];  m[2][3] = a_source.m[2][3];
		m[3][0] = a_source.m[3][0];  m[3][1] = a_source.m[3][1];
		m[3][2] = a_source.m[3][2];  m[3][3] = a_source.m[3][3];
	}


	//-----------------------------------------------------------------------
	/*!
		Set this matrix to be equal to the identity matrix.
	*/
	//-----------------------------------------------------------------------
	inline void identity()
	{
		m[0][0] = 1.0;  m[0][1] = 0.0;  m[0][2] = 0.0;  m[0][3] = 0.0;
		m[1][0] = 0.0;  m[1][1] = 1.0;  m[1][2] = 0.0;  m[1][3] = 0.0;
		m[2][0] = 0.0;  m[2][1] = 0.0;  m[2][2] = 1.0;  m[2][3] = 0.0;
		m[3][0] = 0.0;  m[3][1] = 0.0;  m[3][2] = 0.0;  m[3][3] = 1.0;
	}


	//-----------------------------------------------------------------------
	/*!
		Left-multiply the current matrix by an external matrix passed as
		a parameter.  That is, compute :

		this = a_matrix * this;

		Remember that all matrices are column-major.  That's why the following
		code looks like right-multiplication...

		\param    a_matrix  Matrix with which multiplication is performed.
	*/
	//-----------------------------------------------------------------------
	inline void mul(const cMatrixGL& a_matrix)
	{
		// compute multiplication between both matrices
		double m00 = m[0][0] * a_matrix.m[0][0] + m[0][1] * a_matrix.m[1][0] +
			m[0][2] * a_matrix.m[2][0] + m[0][3] * a_matrix.m[3][0];
		double m01 = m[0][0] * a_matrix.m[0][1] + m[0][1] * a_matrix.m[1][1] +
			m[0][2] * a_matrix.m[2][1] + m[0][3] * a_matrix.m[3][1];
		double m02 = m[0][0] * a_matrix.m[0][2] + m[0][1] * a_matrix.m[1][2] +
			m[0][2] * a_matrix.m[2][2] + m[0][3] * a_matrix.m[3][2];
		double m03 = m[0][0] * a_matrix.m[0][3] + m[0][1] * a_matrix.m[1][3] +
			m[0][2] * a_matrix.m[2][3] + m[0][3] * a_matrix.m[3][3];

		double m10 = m[1][0] * a_matrix.m[0][0] + m[1][1] * a_matrix.m[1][0] +
			m[1][2] * a_matrix.m[2][0] + m[1][3] * a_matrix.m[3][0];
		double m11 = m[1][0] * a_matrix.m[0][1] + m[1][1] * a_matrix.m[1][1] +
			m[1][2] * a_matrix.m[2][1] + m[1][3] * a_matrix.m[3][1];
		double m12 = m[1][0] * a_matrix.m[0][2] + m[1][1] * a_matrix.m[1][2] +
			m[1][2] * a_matrix.m[2][2] + m[1][3] * a_matrix.m[3][2];
		double m13 = m[1][0] * a_matrix.m[0][3] + m[1][1] * a_matrix.m[1][3] +
			m[1][2] * a_matrix.m[2][3] + m[1][3] * a_matrix.m[3][3];

		double m20 = m[2][0] * a_matrix.m[0][0] + m[2][1] * a_matrix.m[1][0] +
			m[2][2] * a_matrix.m[2][0] + m[2][3] * a_matrix.m[3][0];
		double m21 = m[2][0] * a_matrix.m[0][1] + m[2][1] * a_matrix.m[1][1] +
			m[2][2] * a_matrix.m[2][1] + m[2][3] * a_matrix.m[3][1];
		double m22 = m[2][0] * a_matrix.m[0][2] + m[2][1] * a_matrix.m[1][2] +
			m[2][2] * a_matrix.m[2][2] + m[2][3] * a_matrix.m[3][2];
		double m23 = m[2][0] * a_matrix.m[0][3] + m[2][1] * a_matrix.m[1][3] +
			m[2][2] * a_matrix.m[2][3] + m[2][3] * a_matrix.m[3][3];

		double m30 = m[3][0] * a_matrix.m[0][0] + m[3][1] * a_matrix.m[1][0] +
			m[3][2] * a_matrix.m[2][0] + m[3][3] * a_matrix.m[3][0];
		double m31 = m[3][0] * a_matrix.m[0][1] + m[3][1] * a_matrix.m[1][1] +
			m[3][2] * a_matrix.m[2][1] + m[3][3] * a_matrix.m[3][1];
		double m32 = m[3][0] * a_matrix.m[0][2] + m[3][1] * a_matrix.m[1][2] +
			m[3][2] * a_matrix.m[2][2] + m[3][3] * a_matrix.m[3][2];
		double m33 = m[3][0] * a_matrix.m[0][3] + m[3][1] * a_matrix.m[1][3] +
			m[3][2] * a_matrix.m[2][3] + m[3][3] * a_matrix.m[3][3];

		// return values to current matrix
		m[0][0] = m00;  m[0][1] = m01;  m[0][2] = m02;  m[0][3] = m03;
		m[1][0] = m10;  m[1][1] = m11;  m[1][2] = m12;  m[1][3] = m13;
		m[2][0] = m20;  m[2][1] = m21;  m[2][2] = m22;  m[2][3] = m23;
		m[3][0] = m30;  m[3][1] = m31;  m[3][2] = m32;  m[3][3] = m33;
	}


	//-----------------------------------------------------------------------
	/*!
		Left-multiply the current matrix by an external matrix passed as
		a parameter, storing the result externally.  That is, compute :

		a_result = a_matrix * this;

		Remember that all matrices are column-major.  That's why the following
		code looks like right-multiplication...

		\param    a_matrix  Matrix with which multiplication is performed.
		\param    a_result  Matrix where the result is stored.
	*/
	//-----------------------------------------------------------------------
	inline void mulr(const cMatrix3d& a_matrix, cMatrix3d& a_result) const
	{
		// compute multiplication between both matrices
		a_result.m[0][0] = m[0][0] * a_matrix.m[0][0] + m[0][1] * a_matrix.m[1][0] + m[0][2] * a_matrix.m[2][0];
		a_result.m[0][1] = m[0][0] * a_matrix.m[0][1] + m[0][1] * a_matrix.m[1][1] + m[0][2] * a_matrix.m[2][1];
		a_result.m[0][2] = m[0][0] * a_matrix.m[0][2] + m[0][1] * a_matrix.m[1][2] + m[0][2] * a_matrix.m[2][2];
		a_result.m[1][0] = m[1][0] * a_matrix.m[0][0] + m[1][1] * a_matrix.m[1][0] + m[1][2] * a_matrix.m[2][0];
		a_result.m[1][1] = m[1][0] * a_matrix.m[0][1] + m[1][1] * a_matrix.m[1][1] + m[1][2] * a_matrix.m[2][1];
		a_result.m[1][2] = m[1][0] * a_matrix.m[0][2] + m[1][1] * a_matrix.m[1][2] + m[1][2] * a_matrix.m[2][2];
		a_result.m[2][0] = m[2][0] * a_matrix.m[0][0] + m[2][1] * a_matrix.m[1][0] + m[2][2] * a_matrix.m[2][0];
		a_result.m[2][1] = m[2][0] * a_matrix.m[0][1] + m[2][1] * a_matrix.m[1][1] + m[2][2] * a_matrix.m[2][1];
		a_result.m[2][2] = m[2][0] * a_matrix.m[0][2] + m[2][1] * a_matrix.m[1][2] + m[2][2] * a_matrix.m[2][2];
	}


	//-----------------------------------------------------------------------
	/*!
		Transpose this matrix.
	*/
	//-----------------------------------------------------------------------
	inline void trans()
	{
		double t;

		t = m[0][1]; m[0][1] = m[1][0]; m[1][0] = t;
		t = m[0][2]; m[0][2] = m[2][0]; m[2][0] = t;
		t = m[0][3]; m[0][3] = m[3][0]; m[3][0] = t;
		t = m[1][2]; m[1][2] = m[2][1]; m[2][1] = t;
		t = m[1][3]; m[1][3] = m[3][1]; m[3][1] = t;
		t = m[2][3]; m[2][3] = m[3][2]; m[3][2] = t;
	}


	//-----------------------------------------------------------------------
	/*!
		Transpose this matrix and store the result in a_result

		\param      a_result  Result is stored here.
	*/
	//-----------------------------------------------------------------------
	inline void transr(cMatrixGL& a_result) const
	{
		a_result.m[0][0] = m[0][0];
		a_result.m[0][1] = m[1][0];
		a_result.m[0][2] = m[2][0];
		a_result.m[0][3] = m[3][0];

		a_result.m[1][0] = m[0][1];
		a_result.m[1][1] = m[1][1];
		a_result.m[1][2] = m[2][1];
		a_result.m[1][3] = m[3][1];

		a_result.m[2][0] = m[0][2];
		a_result.m[2][1] = m[1][2];
		a_result.m[2][2] = m[2][2];
		a_result.m[2][3] = m[3][2];

		a_result.m[3][0] = m[0][3];
		a_result.m[3][1] = m[1][3];
		a_result.m[3][2] = m[2][3];
		a_result.m[3][3] = m[3][3];
	}


	//-----------------------------------------------------------------------
	/*!
		Create a frustum matrix, as defined by the glFrustum function.
	*/
	//-----------------------------------------------------------------------
	inline void buildFrustumMatrix(double l, double r, double b, double t,
		double n, double f)
	{
		m[0][0] = (2.0*n) / (r - l);
		m[0][1] = 0.0;
		m[0][2] = 0.0;
		m[0][3] = 0.0;

		m[1][0] = 0.0;
		m[1][1] = (2.0*n) / (t - b);
		m[1][2] = 0.0;
		m[1][3] = 0.0;

		m[2][0] = (r + l) / (r - l);
		m[2][1] = (t + b) / (t - b);
		m[2][2] = -(f + n) / (f - n);
		m[2][3] = -1.0;

		m[3][0] = 0.0;
		m[3][1] = 0.0;
		m[3][2] = -(2.0*f*n) / (f - n);
		m[3][3] = 0.0;
	}


	//-----------------------------------------------------------------------
	/*!
		Invert this matrix.

		\return     Return \b true if operation succeeds. Otherwise \b false.
	*/
	//-----------------------------------------------------------------------
	bool inline invert()
	{

		// Macros used during inversion
#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

		double *mat = m[0];

		GLdouble wtmp[4][8];
		GLdouble m0, m1, m2, m3, s;
		GLdouble *r0, *r1, *r2, *r3;

		r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

		r0[0] = MAT(mat, 0, 0), r0[1] = MAT(mat, 0, 1),
			r0[2] = MAT(mat, 0, 2), r0[3] = MAT(mat, 0, 3),
			r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

			r1[0] = MAT(mat, 1, 0), r1[1] = MAT(mat, 1, 1),
			r1[2] = MAT(mat, 1, 2), r1[3] = MAT(mat, 1, 3),
			r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

			r2[0] = MAT(mat, 2, 0), r2[1] = MAT(mat, 2, 1),
			r2[2] = MAT(mat, 2, 2), r2[3] = MAT(mat, 2, 3),
			r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

			r3[0] = MAT(mat, 3, 0), r3[1] = MAT(mat, 3, 1),
			r3[2] = MAT(mat, 3, 2), r3[3] = MAT(mat, 3, 3),
			r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

		// choose pivot
		if (fabs(r3[0]) > fabs(r2[0])) SWAP_ROWS(r3, r2);
		if (fabs(r2[0]) > fabs(r1[0])) SWAP_ROWS(r2, r1);
		if (fabs(r1[0]) > fabs(r0[0])) SWAP_ROWS(r1, r0);
		if (0.0 == r0[0])
		{
			return false;
		}

		// eliminate first variable
		m1 = r1[0] / r0[0]; m2 = r2[0] / r0[0]; m3 = r3[0] / r0[0];
		s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
		s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
		s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
		s = r0[4];
		if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r0[5];
		if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r0[6];
		if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r0[7];
		if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

		// choose pivot
		if (fabs(r3[1]) > fabs(r2[1])) SWAP_ROWS(r3, r2);
		if (fabs(r2[1]) > fabs(r1[1])) SWAP_ROWS(r2, r1);
		if (0.0 == r1[1])
		{
			return false;
		}

		// eliminate second variable
		m2 = r2[1] / r1[1]; m3 = r3[1] / r1[1];
		r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
		r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
		s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
		s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
		s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
		s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

		// choose pivot
		if (fabs(r3[2]) > fabs(r2[2])) SWAP_ROWS(r3, r2);
		if (0.0 == r2[2])
		{
			return false;
		}

		// eliminate third variable
		m3 = r3[2] / r2[2];
		r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
			r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
			r3[7] -= m3 * r2[7];

		// last check
		if (0.0 == r3[3])
		{
			return false;
		}

		s = 1.0 / r3[3];
		r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

		m2 = r2[3];
		s = 1.0 / r2[2];
		r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
			r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
		m1 = r1[3];
		r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
			r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
		m0 = r0[3];
		r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
			r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

		m1 = r1[2];
		s = 1.0 / r1[1];
		r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
			r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
		m0 = r0[2];
		r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
			r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

		m0 = r0[1];
		s = 1.0 / r0[0];
		r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
			r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

		MAT(mat, 0, 0) = r0[4]; MAT(mat, 0, 1) = r0[5],
			MAT(mat, 0, 2) = r0[6]; MAT(mat, 0, 3) = r0[7],
			MAT(mat, 1, 0) = r1[4]; MAT(mat, 1, 1) = r1[5],
			MAT(mat, 1, 2) = r1[6]; MAT(mat, 1, 3) = r1[7],
			MAT(mat, 2, 0) = r2[4]; MAT(mat, 2, 1) = r2[5],
			MAT(mat, 2, 2) = r2[6]; MAT(mat, 2, 3) = r2[7],
			MAT(mat, 3, 0) = r3[4]; MAT(mat, 3, 1) = r3[5],
			MAT(mat, 3, 2) = r3[6]; MAT(mat, 3, 3) = r3[7];

		return true;

		// Macros used during inversion
#undef MAT
#undef SWAP_ROWS
	}


	//-----------------------------------------------------------------------
	/*!
		Build a perspective matrix, according to the gluPerspective function
	*/
	//-----------------------------------------------------------------------
	inline void buildPerspectiveMatrix(double  fovy, double aspect,
		double zNear, double zFar)
	{
		double xMin, xMax, yMin, yMax;

		yMax = zNear * tan(fovy * CHAI_PI / 360.0);
		yMin = -yMax;

		xMin = yMin * aspect;
		xMax = yMax * aspect;

		buildFrustumMatrix(xMin, xMax, yMin, yMax, zNear, zFar);
	}


	//-----------------------------------------------------------------------
	/*!
		Build a 4x4 matrix transform, according to the gluLookAt function
	*/
	//-----------------------------------------------------------------------
	inline void buildLookAtMatrix(double eyex, double eyey, double eyez,
		double centerx, double centery, double centerz,
		double upx, double upy, double upz)
	{
		double x[3], y[3], z[3];
		double mag;

		// create rotation matrix

		// Z vector
		z[0] = eyex - centerx;
		z[1] = eyey - centery;
		z[2] = eyez - centerz;
		mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
		if (mag) {  /* mpichler, 19950515 */
			z[0] /= mag;
			z[1] /= mag;
			z[2] /= mag;
		}

		// Y vector
		y[0] = upx;
		y[1] = upy;
		y[2] = upz;

		// X vector = Y cross Z
		x[0] = y[1] * z[2] - y[2] * z[1];
		x[1] = -y[0] * z[2] + y[2] * z[0];
		x[2] = y[0] * z[1] - y[1] * z[0];

		// Recompute Y = Z cross X
		y[0] = z[1] * x[2] - z[2] * x[1];
		y[1] = -z[0] * x[2] + z[2] * x[0];
		y[2] = z[0] * x[1] - z[1] * x[0];


		// Normalize
		mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
		if (mag) {
			x[0] /= mag;
			x[1] /= mag;
			x[2] /= mag;
		}

		mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
		if (mag) {
			y[0] /= mag;
			y[1] /= mag;
			y[2] /= mag;
		}

		m[0][0] = x[0];  m[1][0] = x[1];  m[2][0] = x[2];  m[3][0] = -x[0] * eyex + -x[1] * eyey + -x[2] * eyez;
		m[0][1] = y[0];  m[1][1] = y[1];  m[2][1] = y[2];  m[3][1] = -y[0] * eyex + -y[1] * eyey + -y[2] * eyez;
		m[0][2] = z[0];  m[1][2] = z[1];  m[2][2] = z[2];  m[3][2] = -z[0] * eyex + -z[1] * eyey + -z[2] * eyez;
		m[0][3] = 0.0;   m[1][3] = 0.0;   m[2][3] = 0.0;   m[3][3] = 1.0;
	}


	//-----------------------------------------------------------------------
	/*!
		Build a 4x4 matrix transform, according to the gluLookAt function
	*/
	//-----------------------------------------------------------------------
	inline void buildLookAtMatrix(cVector3d& a_eye, cVector3d&  a_lookAt, cVector3d a_up)
	{
		buildLookAtMatrix(a_eye.x, a_eye.y, a_eye.z,
			a_lookAt.x, a_lookAt.y, a_lookAt.z,
			a_up.x, a_up.y, a_up.z);
	}


	//-----------------------------------------------------------------------
	/*!
		Push the current OpenGL matrix stack
	*/
	//-----------------------------------------------------------------------
	inline void glMatrixPush()
	{
		glPushMatrix();
	}


	//-----------------------------------------------------------------------
	/*!
		Load the current OpenGL matrix with this cMatrixGL matrix
	*/
	//-----------------------------------------------------------------------
	inline void glMatrixLoad()
	{
		glLoadMatrixd((const double *)pMatrix());
	}


	//-----------------------------------------------------------------------
	/*!
		Multiply the current OpenGL matrix with this cMatrixGL matrix
	*/
	//-----------------------------------------------------------------------
	inline void glMatrixMultiply()
	{
		glMultMatrixd((const double *)pMatrix());
	}


	//-----------------------------------------------------------------------
	/*!
		Push the current OpenGL matrix stack and multiply with this cMatrixGL
		matrix.
	*/
	//-----------------------------------------------------------------------
	inline void glMatrixPushMultiply()
	{
		glPushMatrix();
		glMultMatrixd((const double *)pMatrix());
	}


	//-----------------------------------------------------------------------
	/*!
		Pop current OpenGL matrix off the stack.
	*/
	//-----------------------------------------------------------------------
	inline void glMatrixPop()
	{
		glPopMatrix();
	}


	//-----------------------------------------------------------------------
	/*!
		Convert the current matrix into an std::string

		\param    a_string     String where conversion is stored
		\param    a_precision  Number of digits
	*/
	//-----------------------------------------------------------------------
	inline void str(std::string& a_string, int a_precision)
	{
		a_string.append("[ ");
		for (int i = 0; i < 4; i++)
		{
			a_string.append("( ");
			for (int j = 0; j < 4; j++)
			{
				cStr(a_string, m[j][i], a_precision);
				if (j < 3)
				{
					a_string.append(", ");
				}
			}
			a_string.append(" ) ");
		}
		a_string.append("]");
	}
};


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
