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

//---------------------------------------------------------------------------
#ifndef CVector3dH
#define CVector3dH
//---------------------------------------------------------------------------
#include "CString.h"
#include "CConstants.h"
#include <math.h>
#include <ostream>
//---------------------------------------------------------------------------

#ifdef _MSVC
#include <conio.h>
#define CHAI_DEBUG_PRINT _cprintf
#else 
#define CHAI_DEBUG_PRINT printf
#endif


//===========================================================================
/*!
      \file     CVector3d.h
      \struct   cVector3d
      \brief    cVector3d represents a 3D vector with components X, Y and Z.
*/
//===========================================================================
struct cVector3d
{
    public:
    // MEMBERS:
    //! Component X of vector.
    double x;
    //! Component Y of vector.
    double y;
    //! Component Z of vector.
    double z;


    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------
    /*!
      Constructors of cVector3d.

      You can initialize a cVector3d from any of the following:

      char*
      string
      double,double,double
      cVector3d
      double*
      float*

      See the set(char*) function for a description of the acceptable
      string formats.
    */
    //-----------------------------------------------------------------------
    cVector3d() {}
    cVector3d(const char* a_initstr) { set(a_initstr); }
    cVector3d(const string& a_initstr) { set(a_initstr); }
    cVector3d(const double a_x, const double a_y,
        const double a_z) : x(a_x), y(a_y), z(a_z) { }
    cVector3d(const cVector3d& a_source) : x(a_source.x), y(a_source.y), z(a_source.z) { }
    cVector3d(const double* a_in) : x(a_in[0]), y(a_in[1]), z(a_in[2]) { }
    cVector3d(const float* a_in) : x(a_in[0]), y(a_in[1]), z(a_in[2]) { }
     
    // OVERLOADED CAST OPERATORS:
    //-----------------------------------------------------------------------
    /*!
      Cast to a double*

      This replaces the previous [] operators (without breaking existing 
      code).
    */
    //-----------------------------------------------------------------------
    // operator double* () { return &x; }
    // operator const double* () const { return (const double*)&x; } 
    
    double& operator[] (unsigned int index) { return (&x)[index]; }
    double operator[] (unsigned int index) const { return ((double*)(&x))[index]; }

    // METHODS:
    //-----------------------------------------------------------------------
    /*!
        Clear vector with zeros.
    */
    //-----------------------------------------------------------------------
    inline void zero()
    {
      x = 0.0;
      y = 0.0;
      z = 0.0;
    }


    //-----------------------------------------------------------------------
    /*!
        Return the i th component of the vector. a_component = 0 return x,
        a_component = 1 returns y, a_component = 2 returns z.

        \param  a_component  component number
    */
    //-----------------------------------------------------------------------
    inline double get(const unsigned int& a_component) const
    {
        return ((double*)(this))[a_component];
    }
    
    /*!
        An overloaded /= operator for vector/scalar division
    */
    inline void operator/= (const double& a_val)
    {
      x /= a_val;
      y /= a_val;
      z /= a_val;
    }

    /*!
        An overloaded *= operator for vector/scalar multiplication
    */
    inline void operator*= (const double& a_val)
    {
      x *= a_val;
      y *= a_val;
      z *= a_val;
    }

    /*!
        An overloaded += operator for vector/vector addition
    */
    inline void operator+= (const cVector3d& a_input)
    {
      x += a_input.x;
      y += a_input.y;
      z += a_input.z;      
    }

    /*!
        An overloaded -= operator for vector/vector subtraction
    */
    inline void operator-= (const cVector3d& a_input)
    {
      x -= a_input.x;
      y -= a_input.y;
      z -= a_input.z;      
    }
    
    //-----------------------------------------------------------------------
    /*!
        Initialize 3 dimensional vector with parameters \c a_X, \c a_Y
        and \c a_Z.

        \param  a_x  X component.
        \param  a_y  Y component.
        \param  a_z  Z component.
    */
    //-----------------------------------------------------------------------
    inline void set(const double& a_x, const double& a_y, const double& a_z)
    {
      x = a_x;
      y = a_y;
      z = a_z;
    }

    //-----------------------------------------------------------------------
    /*!
        Initialize a vector from a string of the form (x,y,z), the
        same form produced by str().  Will actually accept any of the
        following forms:

        (4.3,23,54)
        4.3 54 2.1
        4.5,7.8,9.1      

        ...i.e., it expects three numbers, optionally preceded
        by '(' and whitespace, and separated by commas or whitespace.

        \param   a_initStr The string to convert
        \return  TRUE if conversion was successful.
    */
    //-----------------------------------------------------------------------
    inline bool set(const char* a_initStr)
    {
      if (a_initStr == 0) return false;

      double ax,ay,az;

      // Look for a valid-format string
      // Ignore leading whitespace and ('s
      const char* curpos = a_initStr;
      while(
        (*curpos != '\0') && 
        (*curpos == ' ' || *curpos == '\t' || *curpos == '(')
        )
          curpos++;      
        
      int result = sscanf(curpos,"%lf%*[ ,\t\n\r]%lf%*[ ,\t\n\r]%lf",&ax,&ay,&az);

      // Make sure the conversion worked
      if (result !=3) return false;

      // Copy the values we found
      x = ax; y = ay; z = az;
      return true;
    }

    //-----------------------------------------------------------------------
    /*!
        Initialize a vector from a string of the form (x,y,z) (the
        same form produced by str() )

        \param   a_initStr The string to convert
        \return  TRUE if conversion was successful.
    */
    //-----------------------------------------------------------------------
    inline bool set(const string& a_initStr)
    {
      return set(a_initStr.c_str());
    }

    //-----------------------------------------------------------------------
    /*!
        Copy current vector to external vector as parameter.

        \param  a_destination  Destination vector.
    */
    //-----------------------------------------------------------------------
    inline void copyto(cVector3d& a_destination) const
    {
      a_destination.x = x;
      a_destination.y = y;
      a_destination.z = z;
    }


    //-----------------------------------------------------------------------
    /*!
        Copy external vector as parameter to current vector.

        \param  a_source  Source vector.
    */
    //-----------------------------------------------------------------------
    inline void copyfrom(const cVector3d &a_source)
    {
      x = a_source.x;
      y = a_source.y;
      z = a_source.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Addition between current vector and  external vector passed as
        parameter. \n
        Result is stored in current vector.

        \param  a_vector  This vector is added to current one.
    */
    //-----------------------------------------------------------------------
    inline void add(const cVector3d& a_vector)
    {
      x = x + a_vector.x;
      y = y + a_vector.y;
      z = z + a_vector.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Addition between current vector and external vector passed as
        parameter. \n
        Result is stored in current vector.

        \param  a_x  X component.
        \param  a_y  Y component.
        \param  a_z  Z component.
    */
    //-----------------------------------------------------------------------
    inline void add(const double& a_x, const double& a_y, const double& a_z)
    {
      x = x + a_x;
      y = y + a_y;
      z = z + a_z;
    }


    //-----------------------------------------------------------------------
    /*!
        Addition between current vector and external vector passed as
        parameter.\n  Result is stored in external \e a_result vector.

        \param  a_vector  Vector which is added to current one.
        \param  a_result  Vector where result is stored.
    */
    //-----------------------------------------------------------------------
    inline void addr(const cVector3d& a_vector, cVector3d& a_result) const
    {
      a_result.x = x + a_vector.x;
      a_result.y = y + a_vector.y;
      a_result.z = z + a_vector.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Addition between current vector and vector passed by parameter.\n
        Result is stored in \e a_result vector.

        \param  a_x  X component.
        \param  a_y  Y component.
        \param  a_z  Z component.
        \param  a_result  Vector where result is stored.
    */
    //-----------------------------------------------------------------------
    inline void addr(const double& a_x, const double& a_y, const double& a_z, cVector3d& a_result) const
    {
      a_result.x = x + a_x;
      a_result.y = y + a_y;
      a_result.z = z + a_z;
    }


    //-----------------------------------------------------------------------
    /*!
        Subtraction between current vector and an external vector
        passed as parameter.\n
        Result is stored in current vector.

        \param  a_vector  Vector which is subtracted from current one.
    */
    //-----------------------------------------------------------------------
    inline void sub(const cVector3d& a_vector)
    {
      x = x - a_vector.x;
      y = y - a_vector.y;
      z = z - a_vector.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Subtract an external vector passed as parameter from current
        vector. \n Result is stored in current vector.

        \param  a_x  X component.
        \param  a_y  Y component.
        \param  a_z  Z component.
    */
    //-----------------------------------------------------------------------
    inline void sub(const double& a_x, const double& a_y, const double& a_z)
    {
      x = x - a_x;
      y = y - a_y;
      z = z - a_z;
    }


    //-----------------------------------------------------------------------
    /*!
        Subtraction between current vector and external vector passed as
        parameter.\n  Result is stored in external \e a_result vector.

        \param  a_vector  Vector which is subtracted from current one.
        \param  a_result  Vector where result is stored.
    */
    //-----------------------------------------------------------------------
    inline void subr(const cVector3d& a_vector, cVector3d& a_result) const
    {
      a_result.x = x - a_vector.x;
      a_result.y = y - a_vector.y;
      a_result.z = z - a_vector.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Subtract current vector from vector passed by parameter.\n
        Result is stored in \e a_result vector.

        \param  a_x  X component.
        \param  a_y  Y component.
        \param  a_z  Z component.
        \param  a_result  Vector where result is stored.
    */
    //-----------------------------------------------------------------------
    inline void subr(const double& a_x, const double& a_y, const double& a_z,
                  cVector3d &a_result) const
    {
      a_result.x = x - a_x;
      a_result.y = y - a_y;
      a_result.z = z - a_z;
    }


    //-----------------------------------------------------------------------
    /*!
        Multiply current vector by a scalar. \n
        Result is stored in current vector.

        \param  a_scalar  Scalar value.
    */
    //-----------------------------------------------------------------------
    inline void mul(const double &a_scalar)
    {
      x = a_scalar * x;
      y = a_scalar * y;
      z = a_scalar * z;
    }


    //-----------------------------------------------------------------------
    /*!
        Multiply current vector by a scalar. \n
        Result is stored in \e a_result vector.

        \param  a_scalar  Scalar value.
        \param  a_result  Result vector.
    */
    //-----------------------------------------------------------------------
    inline void mulr(const double& a_scalar, cVector3d& a_result) const
    {
      a_result.x = a_scalar * x;
      a_result.y = a_scalar * y;
      a_result.z = a_scalar * z;
    }


    //-----------------------------------------------------------------------
    /*!
        Divide current vector by a scalar. No check for divide-by-zero
        is performed.

        Result is stored in current vector.

        \param  a_scalar  Scalar value.
    */
    //-----------------------------------------------------------------------
    inline void div(const double &a_scalar)
    {
      x = x / a_scalar;
      y = y / a_scalar;
      z = z / a_scalar;
    }


    //-----------------------------------------------------------------------
    /*!
        Divide current vector by a scalar. \n
        Result is stored in \e a_result vector.

        \param  a_scalar  Scalar value.
        \param  a_result  Result vector.
    */
    //-----------------------------------------------------------------------
    inline void divr(const double& a_scalar, cVector3d& a_result) const
    {
      a_result.x = x / a_scalar;
      a_result.y = y / a_scalar;
      a_result.z = z / a_scalar;
    }


    //-----------------------------------------------------------------------
    /*!
        Negate current vector. \n
        Result is stored in current vector.
    */
    //-----------------------------------------------------------------------
    inline void negate()
    {
      x = -x;
      y = -y;
      z = -z;
    }


    //-----------------------------------------------------------------------
    /*!
        Negate current vector. \n
        Result is stored in \e a_result vector.

        \param  a_result  Result vector.
    */
    //-----------------------------------------------------------------------
    inline void negater(cVector3d& a_result) const
    {
      a_result.x = -x;
      a_result.y = -y;
      a_result.z = -z;
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the cross product between current vector and an external
        vector. \n Result is stored in current vector.

        \param  a_vector  Vector with which cross product is computed with.
    */
    //-----------------------------------------------------------------------
    inline void cross(const cVector3d& a_vector)
    {
      // compute cross product
      double a =  (y * a_vector.z) - (z * a_vector.y);
      double b = -(x * a_vector.z) + (z * a_vector.x);
      double c =  (x * a_vector.y) - (y * a_vector.x);

      // store result in current vector
      x = a;
      y = b;
      z = c;
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the cross product between current vector and an
        external vector passed as parameter. \n
        
        Result is returned.  Performance-wise, cross() and crossr() are usually
        preferred, since this version creates a new stack variable.

        \param  a_vector  Vector with which cross product is computed.
        \return Resulting cross product.
    */
    //-----------------------------------------------------------------------
    inline cVector3d crossAndReturn(const cVector3d& a_vector) const
    {
      cVector3d r;
      crossr(a_vector,r);
      return r;
    }
      
      
    //-----------------------------------------------------------------------
    /*!
        Compute the cross product between current vector and an
        external vector passed as parameter. \n
        Result is stored in \e a_result vector.

        \param  a_vector  Vector with which cross product is computed.
        \param  a_result  Vector where result is stored.
    */
    //-----------------------------------------------------------------------
    inline void crossr(const cVector3d& a_vector, cVector3d& a_result) const
    {
      a_result.x =  (y * a_vector.z) - (z * a_vector.y);
      a_result.y = -(x * a_vector.z) + (z * a_vector.x);
      a_result.z =  (x * a_vector.y) - (y * a_vector.x);
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the dot product between current vector and an external vector
        passed as parameter.

        \param  a_vector  Vector with which dot product is computed.
        \return  Returns dot product computed between both vectors.
    */
    //-----------------------------------------------------------------------
    inline double dot(const cVector3d& a_vector) const
    {
      return((x * a_vector.x) + (y * a_vector.y) + (z * a_vector.z));
    }

    //-----------------------------------------------------------------------
    /*!
        Compute the element-by-element product between current vector and an external
        vector and store in the current vector.
        
        \param  a_vector  Vector with which product is computed.
    */
    //-----------------------------------------------------------------------
    inline void elementMul(const cVector3d& a_vector)
    {
      x*=a_vector.x;
      y*=a_vector.y;
      z*=a_vector.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the element-by-element product between current vector and an external
        vector and store in the supplied output vector.

        \param  a_vector  Vector with which product is computed.
        \param  a_result  Resulting vector.
    */
    //-----------------------------------------------------------------------
    inline void elementMulr(const cVector3d& a_vector, cVector3d& a_result) const
    {
      a_result.x = x*a_vector.x;
      a_result.y = y*a_vector.y;
      a_result.z = z*a_vector.z;
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the length of current vector.

        \return   Returns length of current vector.
    */
    //-----------------------------------------------------------------------
    inline double length() const
    {
      return(sqrt((x * x) + (y * y) + (z * z)));
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the square length of current vector.

        \return   Returns square length of current vector.
    */
    //-----------------------------------------------------------------------
    inline double lengthsq(void) const
    {
      return((x * x) + (y * y) + (z * z));
    }


    //-----------------------------------------------------------------------
    /*!
        Normalize current vector to become a vector of length one.\n
        \b Warning: Vector should not be equal to (0,0,0) or a division
        by zero error will occur. \n
        Result is stored in current vector.
    */
    //-----------------------------------------------------------------------
    inline void normalize()
    {
      // compute length of vector
      double length = sqrt((x * x) + (y * y) + (z * z));

      // divide current vector by its length
      x = x / length;
      y = y / length;
      z = z / length;
    }


    //-----------------------------------------------------------------------
    /*!
        Normalize current vector to become a vector of length one. \n
        \b WARNING: Vector should not be equal to (0,0,0) or a division
        by zero error will occur. \n
        Result is stored in \e a_result vector.

        \param  a_result  Vector where result is stored.
    */
    //-----------------------------------------------------------------------
    inline void normalizer(cVector3d& a_result) const
    {
      // compute length of vector
      double length = sqrt((x * x) + (y * y) + (z * z));

      // divide current vector by its length
      a_result.x = x / length;
      a_result.y = y / length;
      a_result.z = z / length;
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the distance between current point and an external point
        passed as parameter.

        \param  a_vector  Point to which the distance is measured
        \return  Returns distance between the points
    */
    //-----------------------------------------------------------------------
    inline double distance(const cVector3d& a_vector) const
    {
      // compute distance between both points
      double dx = x - a_vector.x;
      double dy = y - a_vector.y;
      double dz = z - a_vector.z;

      // return result
      return(sqrt( dx * dx + dy * dy + dz * dz ));
    }


    //-----------------------------------------------------------------------
    /*!
        Compute the square distance between the current point and an external
        point.

        \param  a_vector  Point to which squared distance is measured
        \return  Returns the squared distance between the points
    */
    //-----------------------------------------------------------------------
    inline double distancesq(const cVector3d& a_vector) const
    {
      // compute distance between both points
      double dx = x - a_vector.x;
      double dy = y - a_vector.y;
      double dz = z - a_vector.z;

      // return result
      return( dx * dx + dy * dy + dz * dz );
    }


    //-----------------------------------------------------------------------
    /*!
        Test whether the current vector and an external vector are equal.

        \param    a_vector  Vector with which equality is checked.
        \param    epsilon  Two vectors will be considered equal if each
                  component is within epsilon units.  Defaults
                  to zero.
        \return   Returns \c true if both vectors are equal, otherwise
                  returns \c false.
    */
    //-----------------------------------------------------------------------
    inline bool equals(const cVector3d& a_vector, const double epsilon=0.0) const
    {

      // Accelerated path for exact equality
      if (epsilon == 0.0) {      
          if ( (x == a_vector.x) && (y == a_vector.y) && (z == a_vector.z) ) return true;
          else return false;
      }

      if ((fabs(a_vector.x-x) < epsilon) &&
          (fabs(a_vector.y-y) < epsilon) &&
          (fabs(a_vector.z-z) < epsilon)) return true;
      else return false;      
    }


    //-----------------------------------------------------------------------
    /*!
        Convert current vector into a string.

        \param    a_string   String where conversion is stored
        \param    a_precision  Number of digits.
    */
    //-----------------------------------------------------------------------
    inline void str(string& a_string, const unsigned int a_precision=2) const
    {
      a_string.append("( ");
      cStr(a_string, x, a_precision);
      a_string.append(", ");
      cStr(a_string, y, a_precision);
      a_string.append(", ");
      cStr(a_string, z, a_precision);
      a_string.append(" )");
    }

    //-----------------------------------------------------------------------
    /*!
        Convert current vector into a string, which is returned on the stack.
        \param    a_precision  Number of digits.
    */
    //-----------------------------------------------------------------------
    inline string str(const unsigned int a_precision=2) const
    {
      string a_string;
      str(a_string,a_precision);
      return a_string;
    }

    //-----------------------------------------------------------------------
    /*!
        Print the current vector using the CHAI_DEBUG_PRINT macro.

        \param    a_precision  Number of digits.
    */
    //-----------------------------------------------------------------------    
    inline void print(const unsigned int a_precision=2) const
    {
      string s;
      str(s,a_precision);
      CHAI_DEBUG_PRINT("%s\n",s.c_str());
    }   

    //-----------------------------------------------------------------------
    /*!
        Decompose me into components parallel and perpendicular to the input
        vector.

        \param    a_input         Reference vector.
        \param    a_parallel      Parallel component
        \param    a_perpendicular Perpendicular component
    */
    //-----------------------------------------------------------------------    
    inline void decompose(const cVector3d& a_input, cVector3d& a_parallel, cVector3d& a_perpendicular)
    {
      double scale = (this->dot(a_input) / (a_input.dot(a_input)));
      a_parallel = a_input;
      a_parallel.mul(scale);
      this->subr(a_parallel,a_perpendicular);
    }
    //-----------------------------------------------------------------------
    /*!
        Spherically linearly interpolate between two vectors and store in this vector. 
        Vectors should have the same length

        \param    a_level         Fraction of distance to a_vector2 (0 is fully at a_vector1, 1.0 is fully at a_vector2)
        \param    a_vector1       First vector to interpolate from
        \param    a_vector2       Second vector to interpolate from
    */
    //-----------------------------------------------------------------------
    inline void slerp(double a_level, cVector3d const& a_vector1, cVector3d a_vector2)
    {
        // a_vector2 is passed in by value so that we may scale it
        double a_vec1lensq = a_vector1.lengthsq();
        double cosomega = a_vector1.dot(a_vector2)/(a_vec1lensq);
        if ((cosomega-1.0) > -1e-4 && (cosomega-1.0) < 1e-4) {
            // vectors are (almost) parallel
            // linearly interpolate
            *this = a_vector1;
            this->mul(1.0-a_level);
            a_vector2.mul(a_level);
            this->operator +=(a_vector2);
            this->mul(sqrt(a_vec1lensq/this->lengthsq()));
        } else {
            if (cosomega < 0.0) {
                cosomega = -cosomega;
                a_vector2.negate();
            }
            double ratio1, ratio2;
            if ((cosomega+1.0) > -1e-4 && (cosomega+1.0) < 1e-4) {
                // vectors are 180 degrees apart
                // there is no unique path between them
                if ((a_vector1.x < a_vector1.y) && (a_vector1.x < a_vector1.z)){
                    // x component is the smallest
                    a_vector2.x = 0;
                    a_vector2.y = -a_vector1.z;
                    a_vector2.z = a_vector1.y;
                } else if (a_vector1.y < a_vector1.z) {
                    // y component is the smallest
                    a_vector2.x = -a_vector1.z;
                    a_vector2.y = 0;
                    a_vector2.z = a_vector1.x;
                } else {
                    // z component is the smallest
                    a_vector2.x = -a_vector1.y;
                    a_vector2.y = a_vector1.x;
                    a_vector2.z = 0;
                }
                // scale it so it is the same length as before
                a_vector2.mul(sqrt(a_vec1lensq/a_vector2.lengthsq()));

                ratio1 = sin(CHAI_PI*(0.5-a_level));
                ratio2 = sin(CHAI_PI*a_level);
            } else {
                double omega = acos(cosomega);
                double sinomega = sin(omega);
                ratio1 = sin(omega*(1.0-a_level))/sinomega;
                ratio2 = sin(omega*a_level)/sinomega;
            }
            *this = a_vector1;
            this->mul(ratio1);
            a_vector2.mul(ratio2);
            this->add(a_vector2);
        }
    }
};

/*!
An overloaded * operator for vector/scalar multiplication
*/
inline cVector3d operator*(const cVector3d& v, const double a_input)
{
  return cVector3d(v.x*a_input,v.y*a_input,v.z*a_input);
}


/*!
An overloaded / operator for vector/scalar division
*/
inline cVector3d operator/(const cVector3d& v, const double a_input)
{
  return cVector3d(v.x/a_input,v.y/a_input,v.z/a_input);
}


/*!
An overloaded * operator for scalar/vector multiplication
*/
inline cVector3d operator*(const double a_input, const cVector3d& v)
{
    return cVector3d(v.x*a_input,v.y*a_input,v.z*a_input);
}


/*!
An overloaded + operator for vector/vector addition
*/
inline cVector3d operator+(const cVector3d& v1, const cVector3d& v2)
{
    return cVector3d(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z);
}


/*!
An overloaded - operator for vector/vector subtraction
*/
inline cVector3d operator-(const cVector3d& v1, const cVector3d& v2)
{
    return cVector3d(v1.x-v2.x,v1.y-v2.y,v1.z-v2.z);
}


/*!
An overloaded * operator for vector/vector dotting
*/
inline double operator*(const cVector3d& v1, const cVector3d& v2)
{
    return v1.x*v2.x+v1.y*v2.y+v1.z*v2.z;
}

/*!
    ostream operator
    Outputs the vector's components seperated by commas
*/
static inline std::ostream &operator << (std::ostream &a_os, cVector3d const& a_vec)
{
    a_os << a_vec.x << ", " << a_vec.y << ", " << a_vec.z;
    return a_os;
}

//===========================================================================
/*!
    \struct   cRay3D
    \brief    cRay3D represents a 3D vector with an origin.
*/
//===========================================================================
struct cRay3D
{
    //! Vector representing ray origin
    cVector3d m_origin;  

    //! Unit vector representing ray direction
    cVector3d m_direction;

    cRay3D();

    //! This constructor assumes that a_direction is normalized already
    cRay3D(const cVector3d& a_origin, const cVector3d& a_direction) :
        m_origin(a_origin), m_direction(a_direction)
    {  }
};

//===========================================================================
/*!
    \struct   cSegment3D
    \brief    cSegment3D represents a line segment with a start and an end
*/
//===========================================================================
struct cSegment3D
{

    cVector3d m_start;
    cVector3d m_end;

    cSegment3D(const cVector3d& a_start, const cVector3d& a_end) :
        m_start(a_start), m_end(a_end)
    { }

    //! Returns the squared distance from this segment to a_point and the
    //! position along the segment (from 0.0 to 1.0) of the closest point
    double distanceSquaredToPoint(const cVector3d& a_point, double& a_t, cVector3d* a_closestPoint=0);
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
