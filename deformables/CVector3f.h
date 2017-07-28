//---------------------------------------------------------------------------
#ifndef CVector3fH
#define CVector3fH
//---------------------------------------------------------------------------
#include "CString.h"
#include <math.h>
#include "CVector3d.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	  \struct   cVector3f
	  \brief    cVector3f represents a 3D vector in 3 dimensional
				space with components X, Y and Z.
*/
//===========================================================================
struct cVector3f
{
public:
	// MEMBERS:
	//! Component X of vector.
	float x;
	//! Component Y of vector.
	float y;
	//! Component Z of vector.
	float z;


	// CONSTRUCTOR & DESTRUCTOR:
	//-----------------------------------------------------------------------
	/*!
	  Constructor of cVector3f.
	*/
	//-----------------------------------------------------------------------
	cVector3f() {}
	cVector3f(cVector3d& copy) {
		x = (float)(copy.x); y = (float)(copy.y); z = (float)(copy.z);
	}
	cVector3f(const char*& a_initstr) { set(a_initstr); }
	cVector3f(const string& a_initstr) { set(a_initstr); }
	cVector3f(const float& a_x, const float& a_y, const float& a_z)
	{
		x = a_x;
		y = a_y;
		z = a_z;
	};


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

	inline void elementMul(const cVector3f& a_vector)
	{
		x *= a_vector.x;
		y *= a_vector.y;
		z *= a_vector.z;
	}


	//-----------------------------------------------------------------------
	/*!
		Return the i th component of the vector. a_component = 0 return x,
		a_component = 1 returns y, a_component = 2 returns z.

		\param  a_component  component number
	*/
	//-----------------------------------------------------------------------
	inline float get(const unsigned int& a_component) const
	{
		switch (a_component)
		{
		case 0:  return (x);
		case 1:  return (y);
		case 2:  return (z);
		}
		return (0);
	}
	inline float operator[] (const unsigned int& a_component) const
	{
		switch (a_component)
		{
		case 0:  return (x);
		case 1:  return (y);
		case 2:  return (z);
		}
		return (0);
	}

	/*!
		An overloaded /= operator for vector/scalar division
	*/
	inline void operator/= (const float& a_val)
	{
		x /= a_val;
		y /= a_val;
		z /= a_val;
	}

	/*!
		An overloaded *= operator for vector/scalar multiplication
	*/
	inline void operator*= (const float& a_val)
	{
		// for(register unsigned int i=0; i<3; i++) ((float*)(this))[i] *= a_val;
		x *= a_val;
		y *= a_val;
		z *= a_val;
	}

	/*!
		An overloaded += operator for vector/vector addition
	*/
	inline void operator+= (const cVector3f& a_input)
	{
		x += a_input.x;
		y += a_input.y;
		z += a_input.z;
	}

	/*!
		An overloaded -= operator for vector/vector subtraction
	*/
	inline void operator-= (const cVector3f& a_input)
	{
		x -= a_input.x;
		y -= a_input.y;
		z -= a_input.z;
	}

	//-----------------------------------------------------------------------
	/*!
		Return the i th component of the vector. a_component = 0 return x,
		a_component = 1 returns y, a_component = 2 returns z.

		This is the non-const version of this function, so its return value
		can be used as an l-value.

		\param  a_component  component number
	*/
	//-----------------------------------------------------------------------
	inline float& operator[] (const unsigned int& a_component)
	{
		switch (a_component)
		{
		case 0:  return (x);
		case 1:  return (y);
		default:  return (z);
		}
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
	inline void set(const float& a_x, const float& a_y, const float& a_z)
	{
		x = a_x;
		y = a_y;
		z = a_z;
	}

	inline void set(const cVector3d& v) {
		x = (float)v.x;
		y = (float)v.y;
		z = (float)v.z;
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

		float ax, ay, az;

		// Look for a valid-format string
		// Ignore leading whitespace and ('s
		const char* curpos = a_initStr;
		while (
			(*curpos != '\0') &&
			(*curpos == ' ' || *curpos == '\t' || *curpos == '(')
			)
			curpos++;

		int result = sscanf(curpos, "%f%*[ ,\t\n\r]%f%*[ ,\t\n\r]%f", &ax, &ay, &az);

		// Make sure the conversion worked
		if (result != 3) return false;

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
	inline void copyto(cVector3f& a_destination) const
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
	inline void copyfrom(const cVector3f &a_source)
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
	inline void add(const cVector3f& a_vector)
	{
		x += a_vector.x;
		y += a_vector.y;
		z += a_vector.z;
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
	inline void add(const float& a_x, const float& a_y, const float& a_z)
	{
		x += a_x;
		y += a_y;
		z += a_z;
	}


	//-----------------------------------------------------------------------
	/*!
	  Addition between current vector and external vector passed as
	  parameter.\n  Result is stored in external \e a_result vector.

	  \param  a_vector  Vector which is added to current one.
	  \param  a_result  Vector where result is stored.
	*/
	//-----------------------------------------------------------------------
	inline void addr(const cVector3f& a_vector, cVector3f& a_result) const
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
	inline void addr(const float& a_x, const float& a_y, const float& a_z, cVector3f& a_result) const
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
	inline void sub(const cVector3f& a_vector)
	{
		x -= a_vector.x;
		y -= a_vector.y;
		z -= a_vector.z;
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
	inline void sub(const float& a_x, const float& a_y, const float& a_z)
	{
		x -= a_x;
		y -= a_y;
		z -= a_z;
	}


	//-----------------------------------------------------------------------
	/*!
	  Subtraction between current vector and external vector passed as
	  parameter.\n  Result is stored in external \e a_result vector.

	  \param  a_vector  Vector which is subtracted from current one.
	  \param  a_result  Vector where result is stored.
	*/
	//-----------------------------------------------------------------------
	inline void subr(const cVector3f& a_vector, cVector3f& a_result) const
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
	inline void subr(const float& a_x, const float& a_y, const float& a_z,
		cVector3f &a_result) const
	{
		a_result.x = x - a_x;
		a_result.y = y - a_y;
		a_result.z = z - a_z;
	}


	//-----------------------------------------------------------------------
	/*!
	  Multiply current vector by a scalar. \n
	  Result is stored in current vector.

	  \param  a_scaler  Scalar value.
	*/
	//-----------------------------------------------------------------------
	inline void mul(const float &a_scalar)
	{
		x *= a_scalar;
		y *= a_scalar;
		z *= a_scalar;
	}


	//-----------------------------------------------------------------------
	/*!
	  Multiply current vector by a scalar. \n
	  Result is stored in \e a_result vector.

	  \param  a_scalar  Scalar value.
	  \param  a_result  Result vector.
	*/
	//-----------------------------------------------------------------------
	inline void mulr(const float& a_scalar, cVector3f& a_result) const
	{
		a_result.x = a_scalar * x;
		a_result.y = a_scalar * y;
		a_result.z = a_scalar * z;
	}


	//-----------------------------------------------------------------------
	/*!
	  Divide current vector by a scalar. \n
	  \b Warning: Do not passed the value zero (0) or a division
	  by zero error will occur. \n
	  Result is stored in current vector.

	  \param  a_scalar  Scalar value.
	*/
	//-----------------------------------------------------------------------
	inline void div(const float &iValue)
	{
		x /= iValue;
		y /= iValue;
		z /= iValue;
	}


	//-----------------------------------------------------------------------
	/*!
	  Divide current vector by a scalar. \n
	  Result is stored in \e a_result vector.

	  \param  a_scalar  Scalar value.
	  \param  a_result  Result vector.
	*/
	//-----------------------------------------------------------------------
	inline void divr(const float& a_scalar, cVector3f& a_result) const
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
		x *= -1.0f;
		y *= -1.0f;
		z *= -1.0f;
	}


	//-----------------------------------------------------------------------
	/*!
	  Negate current vector. \n
	  Result is stored in \e a_result vector.

	  \param  a_result  Result vector.
	*/
	//-----------------------------------------------------------------------
	inline void negater(cVector3f& a_result) const
	{
		a_result.x = -x;
		a_result.y = -y;
		a_result.z = -z;
	}


	//-----------------------------------------------------------------------
	/*!
	Compute the cross product between current vector and an external
	vector. \n Result is returned.

	\param  a_vector  Vector with which cross product is computed with.
	*/
	//-----------------------------------------------------------------------
	inline cVector3f cross_and_return(const cVector3f& a_vector) const
	{
		// compute cross product
		float a = (y * a_vector.z) - (z * a_vector.y);
		float b = -(x * a_vector.z) + (z * a_vector.x);
		float c = (x * a_vector.y) - (y * a_vector.x);

		return cVector3f(a, b, c);
	}


	//-----------------------------------------------------------------------
	/*!
	  Compute the cross product between current vector and an external
	  vector. \n Result is stored in current vector.

	  \param  a_vector  Vector with which cross product is computed with.
	*/
	//-----------------------------------------------------------------------
	inline void cross(const cVector3f& a_vector)
	{
		// compute cross product
		float a = (y * a_vector.z) - (z * a_vector.y);
		float b = -(x * a_vector.z) + (z * a_vector.x);
		float c = (x * a_vector.y) - (y * a_vector.x);

		// store result in current vector
		x = a;
		y = b;
		z = c;
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
	inline void crossr(const cVector3f& a_vector, cVector3f& a_result) const
	{
		a_result.x = (y * a_vector.z) - (z * a_vector.y);
		a_result.y = -(x * a_vector.z) + (z * a_vector.x);
		a_result.z = (x * a_vector.y) - (y * a_vector.x);
	}


	//-----------------------------------------------------------------------
	/*!
	  Compute the dot product between current vector and an external vector
	  passed as parameter.

	  \param  a_vector  Vector with which dot product is computed.
	  \return  Returns dot product computed between both vectors.
	*/
	//-----------------------------------------------------------------------
	inline float dot(const cVector3f& a_vector) const
	{
		return((x * a_vector.x) + (y * a_vector.y) + (z * a_vector.z));
	}


	//-----------------------------------------------------------------------
	/*!
	  Compute the length of current vector.

	  \return   Returns length of current vector.
	*/
	//-----------------------------------------------------------------------
	inline float length() const
	{
		return(sqrtf((x * x) + (y * y) + (z * z)));
	}


	//-----------------------------------------------------------------------
	/*!
	  Compute the square length of current vector.

	  \return   Returns square length of current vector.
	*/
	//-----------------------------------------------------------------------
	inline float lengthsq(void) const
	{
		return((x * x) + (y * y) + (z * z));
	}


	//-----------------------------------------------------------------------
	/*!
	  Normalize current vector to become a vector of length one.\n
	  \b Warning: Vector should not be of equal to (0,0,0) or a division
	  by zero error will occur. \n
	  Result is stored in current vector.
	*/
	//-----------------------------------------------------------------------
	inline void normalize()
	{

		const float sq = sqrtf((x * x) + (y * y) + (z * z));
		if (sq < CHAI_SMALL) return;
		const float one_over_length = 1.0f / sq;

		// TODO: is this more efficient than the previous implementation?
		/*
		const float one_over_length = 1.0f / sqrtf((x * x) + (y * y) + (z * z));
		*/

		// divide current vector by its length
		x *= one_over_length;
		y *= one_over_length;
		z *= one_over_length;
	}


	//-----------------------------------------------------------------------
	/*!
	  Normalize current vector to become a vector of length one. \n
	  \b WARNING: Vector should not be of equal to (0,0,0) or a division
	  by zero error will occur. \n
	  Result is stored in \e a_result vector.

	  \param  a_result  Vector where result is stored.
	*/
	//-----------------------------------------------------------------------
	inline void normalizer(cVector3f& a_result) const
	{
		// compute length of vector
		float length = sqrtf((x * x) + (y * y) + (z * z));

		// divide current vector by its length
		a_result.x = x / length;
		a_result.y = y / length;
		a_result.z = z / length;
	}


	//-----------------------------------------------------------------------
	/*!
	  Compute the distance between current point and an external point
	  passed as parameter.

	  \param  a_vector  Point with which distance is measured.
	  \return  Returns distance between both points.
	*/
	//-----------------------------------------------------------------------
	inline float distance(const cVector3f& a_vector) const
	{
		// compute distance between both points
		float dx = x - a_vector.x;
		float dy = y - a_vector.y;
		float dz = z - a_vector.z;

		// return result
		return(sqrtf(dx * dx + dy * dy + dz * dz));
	}


	//-----------------------------------------------------------------------
	/*!
	  Compute the square distance between current point and an external
	  point passed as parameter.

	  \param  a_vector  Point with which square distance is measured.
	  \return  Returns square distance between both points.
	*/
	//-----------------------------------------------------------------------
	inline float distancesq(const cVector3f& a_vector) const
	{
		// compute distance between both points
		float dx = x - a_vector.x;
		float dy = y - a_vector.y;
		float dz = z - a_vector.z;

		// return result
		return(dx * dx + dy * dy + dz * dz);
	}


	//-----------------------------------------------------------------------
	/*!
	  Compare if current vector and external vector passed as parameter
	  are equal.

	  \param    a_vector  Vector with which equality is checked.
	  \return   Returns \c true if both vectors are equal, otherwise
			returns \c false.
	*/
	//-----------------------------------------------------------------------
	inline bool equals(const cVector3f& a_vector) const
	{
		if ((x == a_vector.x) && (y == a_vector.y) && (z == a_vector.z))
		{
			return (true);
		}
		else
		{
			return (false);
		}
	}


	//-----------------------------------------------------------------------
	/*!
	  Convert current vector into a string.

	  \param    a_string   String where conversion is stored
	  \param    a_precision  Number of digits.
	*/
	//-----------------------------------------------------------------------
	inline void str(string& a_string, const unsigned int a_precision = 2) const
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
	inline string str(const unsigned int a_precision = 2) const
	{
		string a_string;
		str(a_string, a_precision);
		return a_string;
	}
};

/*!
An overloaded * operator for vector/scalar multiplication
*/
inline cVector3f operator*(const cVector3f& v, const float a_input)
{
	return cVector3f(v.x*a_input, v.y*a_input, v.z*a_input);
}


/*!
An overloaded * operator for scalar/vector multiplication
*/
inline cVector3f operator*(const float a_input, const cVector3f& v)
{
	return cVector3f(v.x*a_input, v.y*a_input, v.z*a_input);
}


/*!
An overloaded / operator for vector/scalar division
*/
inline cVector3f operator/(const cVector3f& v, const float a_input)
{
	return cVector3f(v.x / a_input, v.y / a_input, v.z / a_input);
}


/*!
An overloaded + operator for vector/vector addition
*/
inline cVector3f operator+(const cVector3f& v1, const cVector3f& v2)
{
	return cVector3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}


/*!
An overloaded - operator for vector/vector subtraction
*/
inline cVector3f operator-(const cVector3f& v1, const cVector3f& v2)
{
	return cVector3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}


/*!
An overloaded * operator for vector/vector dotting
*/
inline float operator*(const cVector3f& v1, const cVector3f& v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}


//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
