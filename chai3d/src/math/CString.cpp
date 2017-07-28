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
#include "CString.h"
#include "CMacrosGL.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Compute the length of a string up to 255 characters.

	\fn			int ccStrLength(char& a_string)
	\param		a_string  Input string. Pointer to a char.
	\return		Return the length of the string
*/
//===========================================================================
int cStringLength(const char* a_string)
{
	int counter = 0;
	while (counter < 256)
	{
		if (a_string[counter] == 0) { return (counter); }
		counter++;
	}
	return (-1);
}


//===========================================================================
/*!
	Convert a \e boolean into a \e string.

	\fn     void cStr(string& a_string, const bool& a_value)
	\param    a_string  Input string.
	\param    a_value  Input value of type \e boolean.
*/
//===========================================================================
void cStr(string& a_string, const bool& a_value)
{
	if (a_value) a_string.append("true");
	else a_string.append("false");
}


//===========================================================================
/*!
	Convert an \e integer into a \e string.

	\fn     void cStr(string& a_string, const int& a_value)
	\param    a_string  Input string.
	\param    a_value  Input value of type \e integer.
*/
//===========================================================================
void cStr(string& a_string, const int& a_value)
{
	char buffer[255];
	sprintf(buffer, "%d", a_value);
	a_string.append(buffer);
}


//===========================================================================
/*!
	Convert a \e float into a \e string.

	\fn       void cStr(string& a_string, const float& a_value, const unsigned int a_precision=2)
	\param    a_string  Input string.
	\param    a_value  Input value of type \e integer.
	\param    a_precision  Number of digits displayed after the decimal pt.
*/
//===========================================================================
void cStr(string& a_string, const float& a_value, const unsigned int a_precision)
{
	// make sure number of digits ranges between 0 and 20
	//   unsigned int numDigits = a_precision;
	int numDigits = a_precision;
	if (numDigits > 20)
	{
		numDigits = 20;
	}

	// if number of digits is zero, remove '.'
	if (numDigits == 0)
	{
		numDigits = -1;
	}

	char buffer[255];
	sprintf(buffer, "%.20f", a_value);
	buffer[(cStringLength(buffer) - 20 + numDigits)] = NULL;
	double chopped_value = atof(buffer);
	double round_diff = a_value - chopped_value;
	double round_threshold = 0.5f*pow(0.1f, (int)a_precision);
	if (fabs(round_diff) >= round_threshold)
	{
		double rounded_value;
		if (a_value >= 0.0) rounded_value = a_value + round_threshold;
		else rounded_value = a_value - round_threshold;
		sprintf(buffer, "%.20f", rounded_value);
		buffer[(cStringLength(buffer) - 20 + numDigits)] = NULL;
	}
	a_string.append(buffer);

}


//===========================================================================
/*!
	Convert a \e double into a \e string.

	\fn     void cStr(string& a_string, const double& a_value,
				  const unsigned int a_precision=2);
	\param    a_string  Input string.
	\param    a_value  Input value of type \e integer.
	\param    a_precision  Number of digits displayed after the decimal pt.
*/
//===========================================================================
void cStr(string& a_string, const double& a_value, const unsigned int a_precision)
{
	// make sure number of digits ranges between 0 and 20
	//    unsigned int numDigits = a_precision;
	int numDigits = a_precision;
	if (numDigits > 20)
	{
		numDigits = 20;
	}

	// if number of digits is zero, remove '.'
	if (numDigits == 0)
	{
		numDigits = -1;
	}

	char buffer[255];
	sprintf(buffer, "%.20f", a_value);
	buffer[(cStringLength(buffer) - 20 + numDigits)] = NULL;
	double chopped_value = atof(buffer);
	double round_diff = a_value - chopped_value;
	double round_threshold = 0.5*pow(0.1, (int)a_precision);
	if (fabs(round_diff) >= round_threshold)
	{
		double rounded_value;
		if (a_value >= 0.0) rounded_value = a_value + round_threshold;
		else rounded_value = a_value - round_threshold;
		sprintf(buffer, "%.20f", rounded_value);
		buffer[(cStringLength(buffer) - 20 + numDigits)] = NULL;
	}
	a_string.append(buffer);
}
