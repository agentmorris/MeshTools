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
#include "CFileLoaderBMP.h"
//---------------------------------------------------------------------------

//===========================================================================
/*!
	Constructor of cFileLoaderBMP

	\fn     cFileLoaderBMP::cFileLoaderBMP()
*/
//===========================================================================
cFileLoaderBMP::cFileLoaderBMP()
{
	// initialization
	reset();
}


//===========================================================================
/*!
	Constructor loads the bitmap when it is created

	\fn         cFileLoaderBMP::cFileLoaderBMP(char* a_fileName)
	\param      a_fileName  Filename of bitmap image.
*/
//===========================================================================
cFileLoaderBMP::cFileLoaderBMP(char* a_fileName)
{
	// initialization
	reset();

	// load BMP file
	loadBMP(a_fileName);
}


//===========================================================================
/*!
	  Destructor of cFileLoaderBMP

	  \fn         cFileLoaderBMP::~cFileLoaderBMP()
*/
//===========================================================================
cFileLoaderBMP::~cFileLoaderBMP()
{
	// release memory.
	if (m_colors != NULL)
	{
		delete[] m_colors;
	}

	if (m_pBitmap != NULL)
	{
		delete[] m_pBitmap;
	}
}


//===========================================================================
/*!
	  Load a bitmap from a file and represent it correctly in memory.

	  \fn         bool cFileLoaderBMP::loadBMP(char* a_fileName)
	  \param      a_fileName  Filename of image bitmap.
*/
//===========================================================================
bool cFileLoaderBMP::loadBMP(char* a_fileName)
{
	FILE *in;               // file stream for reading
	char *tempData;         // temp storage for image data
	int numColors;         // total available colours

	// bitmap is not loaded yet
	m_loaded = false;

	// make sure memory is not lost
	if (m_colors != 0)
	{
		delete[] m_colors;
	}

	if (m_pBitmap != NULL)
	{
		delete[] m_pBitmap;
	}

	// open the file for reading in binary mode
	in = fopen(a_fileName, "rb");

	// if the file does not exist return in error
	if (in == NULL)
	{
		m_errorMsg = "File does not exist.";
		// fclose(in);
		return false;
	}

	// read in the entire BITMAPFILEHEADER
	fread(&m_bmfh, sizeof(BITMAPFILEHEADER), 1, in);

	// check for the magic number that says this is a bitmap
	if (m_bmfh.bfType != BITMAP_MAGIC_NUMBER)
	{
		fclose(in);
		return false;
	}

	// read in the entire BITMAPINFOHEADER
	fread(&m_bmih, sizeof(BITMAPINFOHEADER), 1, in);

	// save the width, height and bits per pixel for external use
	m_width = m_bmih.biWidth;
	m_height = m_bmih.biHeight;
	m_bpp = m_bmih.biBitCount;

	// calculate the size of the image data with padding
	m_dataSize = (m_width * m_height * (unsigned int)(m_bmih.biBitCount / 8.0));

	// calculate the number of available colors
	numColors = 1 << m_bmih.biBitCount;

	// if the bitmap is not 8 bits per pixel or more return in error
	if (m_bpp < 8)
	{
		m_errorMsg = "File is not 8 or 24 bits per pixel.";
		fclose(in);
		return (false);
	}

	// load the palette for 8 bits per pixel
	if (m_bpp == 8)
	{
		m_colors = new RGBQUAD[numColors];
		fread(m_colors, sizeof(RGBQUAD), numColors, in);
	}

	// set up the temporary buffer for the image data
	tempData = new char[m_dataSize];

	// exit if there is not enough memory
	if (tempData == NULL)
	{
		m_errorMsg = "Not enough memory to allocate a temporary buffer.";
		fclose(in);
		return (false);
	}

	// read in the entire image
	fread(tempData, sizeof(char), m_dataSize, in);

	// close the file now that we have all the info
	fclose(in);

	// calculate the witdh of the final image in bytes
	m_byteWidth = m_padWidth = (int)((float)m_width * (float)m_bpp / 8.0);

	// adjust the width for padding as necessary
	while ((m_padWidth % 4) != 0)
	{
		m_padWidth++;
	}

	// change format from GBR to RGB
	if (m_bpp == 8)
	{
		m_loaded = convert8(tempData);
	}

	// change format from GBR to RGB
	else if (m_bpp == 24)
	{
		m_loaded = convert24(tempData);
	}

	// clean up memory
	delete[] tempData;

	// bitmap is now loaded
	m_errorMsg = "Bitmap loaded";

	// return result
	return (m_loaded);
}


//===========================================================================
/*!
	  This function initializes all variables in class.

	  \fn        void cFileLoaderBMP::reset()
*/
//===========================================================================
void cFileLoaderBMP::reset()
{
	m_loaded = false;
	m_colors = 0;
	m_pBitmap = NULL;
	m_errorMsg = "";
}


//===========================================================================
/*!
	  Convert format from GBR to RGB - 24bits images

	  \fn        bool cFileLoaderBMP::convert24(char* tempData)
*/
//===========================================================================
bool cFileLoaderBMP::convert24(char* tempData)
{
	int offset, diff;
	diff = m_width * m_height * RGB_BYTE_SIZE;

	//allocate the buffer for the final image data
	m_pBitmap = new unsigned char[diff];

	//exit if there is not enough memory
	if (m_pBitmap == NULL)
	{
		m_errorMsg = "Not enough memory to allocate an image buffer";
		delete[] m_pBitmap;
		return (false);
	}

	if (m_height > 0)
	{
		offset = m_padWidth - m_byteWidth;

		//count backwards so you start at the front of the image
		for (unsigned int i = 0; i < m_dataSize; i += 3)
		{
			//jump over the padding at the start of a new line
			if (((i + 1) % m_padWidth) == 0)
			{
				i += offset;
			}

			//transfer the data
			*(m_pBitmap + i + 2) = *(tempData + i);
			*(m_pBitmap + i + 1) = *(tempData + i + 1);
			*(m_pBitmap + i) = *(tempData + i + 2);
		}
	}

	//image parser for a forward image
	else
	{
		offset = m_padWidth - m_byteWidth;
		int j = m_dataSize - 3;

		//count backwards so you start at the front of the image
		//here you can start from the back of the file or the front,
		//after the header  The only problem is that some programs
		//will pad not only the data, but also the file size to
		//be divisible by 4 bytes.

		for (unsigned int i = 0; i < m_dataSize; i += 3)
		{
			//jump over the padding at the start of a new line
			if (((i + 1) % m_padWidth) == 0)
			{
				i += offset;
			}

			//transfer the data
			*(m_pBitmap + j + 2) = *(tempData + i);
			*(m_pBitmap + j + 1) = *(tempData + i + 1);
			*(m_pBitmap + j) = *(tempData + i + 2);
			j -= 3;
		}
	}

	// return success
	return (true);
}


//===========================================================================
/*!
	  Convert format from GBR to RGB - 8bits images

	  \fn        bool cFileLoaderBMP::convert8(char* tempData)
*/
//===========================================================================
bool cFileLoaderBMP::convert8(char* tempData)
{
	int offset, diff;
	diff = m_width * m_height * RGB_BYTE_SIZE;

	//allocate the buffer for the final image data
	m_pBitmap = new unsigned char[diff];

	//exit if there is not enough memory
	if (m_pBitmap == NULL)
	{
		m_errorMsg = "Not enough memory to allocate an image buffer";
		delete[] m_pBitmap;
		return (false);
	}

	if (m_height > 0)
	{
		offset = m_padWidth - m_byteWidth;
		int j = 0;

		//count backwards so you start at the front of the image
		for (unsigned int i = 0; i < m_dataSize*RGB_BYTE_SIZE; i += 3)
		{
			//jump over the padding at the start of a new line
			if (((i + 1) % m_padWidth) == 0)
			{
				i += offset;
			}

			//transfer the data
			*(m_pBitmap + i) = m_colors[*(tempData + j)].rgbRed;
			*(m_pBitmap + i + 1) = m_colors[*(tempData + j)].rgbGreen;
			*(m_pBitmap + i + 2) = m_colors[*(tempData + j)].rgbBlue;
			j++;
		}
	}

	//image parser for a forward image
	else
	{
		offset = m_padWidth - m_byteWidth;
		int j = m_dataSize - 1;

		//count backwards so you start at the front of the image
		for (unsigned int i = 0; i < m_dataSize*RGB_BYTE_SIZE; i += 3)
		{
			//jump over the padding at the start of a new line
			if (((i + 1) % m_padWidth) == 0)
			{
				i += offset;
			}
			//transfer the data
			*(m_pBitmap + i) = m_colors[*(tempData + j)].rgbRed;
			*(m_pBitmap + i + 1) = m_colors[*(tempData + j)].rgbGreen;
			*(m_pBitmap + i + 2) = m_colors[*(tempData + j)].rgbBlue;
			j--;
		}
	}

	// return success
	return (true);
}
