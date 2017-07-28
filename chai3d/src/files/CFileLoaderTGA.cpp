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

	This TGA loader was written by Lev Povalahev and was
	downloaded from:

	http://www.levp.de/3d/index.html

	Used with permission.

	\author:    <http://www.chai3d.org>
	\author:    Lev Povalahev
	\author:    Dan Morris
	\version    1.0
	\date       03/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#include "CFileLoaderTGA.h"
#include <stdlib.h>
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// global functions
//---------------------------------------------------------------------------
static int TGAReadError = 0;

static void ReadData(std::ifstream &file, char* data, uint size)
{
	if (!file.is_open())
		return;
	uint a = (uint)file.tellg();
	a += size;
	file.read(data, size);

	uint g = (uint)file.tellg();

	if (a != g)
	{
		TGAReadError = 1;
	}
}

//---------------------------------------------------------------------------
cFileLoaderTGA::cFileLoaderTGA()
{
	m_loaded = false;
	m_width = 0;
	m_height = 0;
	m_pixelDepth = 0;
	m_alphaDepth = 0;
	m_type = itUndefined;
	m_pixels = 0;
}


//---------------------------------------------------------------------------
cFileLoaderTGA::cFileLoaderTGA(const std::string &filename)
{
	m_loaded = false;
	m_width = 0;
	m_height = 0;
	m_pixelDepth = 0;
	m_alphaDepth = 0;
	m_type = itUndefined;
	m_pixels = 0;
	LoadFromFile(filename);
}


//---------------------------------------------------------------------------
cFileLoaderTGA::~cFileLoaderTGA()
{
	Clear();
}


//---------------------------------------------------------------------------
bool cFileLoaderTGA::LoadFromFile(const std::string &filename)
{
	if (m_loaded)
		Clear();
	m_loaded = false;

	std::ifstream file;
	file.open(filename.c_str(), std::ios::binary);
	if (!file.is_open())
		return false;

	bool rle = false;
	bool truecolor = false;
	uint CurrentPixel = 0;
	byte ch_buf1, ch_buf2;
	byte buf1[1000];

	byte IDLength;
	byte IDColorMapType;
	byte IDImageType;

	ReadData(file, (char*)&IDLength, 1);
	ReadData(file, (char*)&IDColorMapType, 1);

	if (IDColorMapType == 1)
		return false;

	ReadData(file, (char*)&IDImageType, 1);

	switch (IDImageType)
	{
	case 2:
		truecolor = true;
		break;
	case 3:
		m_type = itGreyscale;
		break;
	case 10:
		rle = true;
		truecolor = true;
		break;
	case 11:
		rle = true;
		m_type = itGreyscale;
		break;
	default:
		return false;
	}

	file.seekg(5, std::ios::cur);

	file.seekg(4, std::ios::cur);
	ReadData(file, (char*)&m_width, 2);
	ReadData(file, (char*)&m_height, 2);
	ReadData(file, (char*)&m_pixelDepth, 1);

	if (!((m_pixelDepth == 8) || (m_pixelDepth == 24) ||
		(m_pixelDepth == 16) || (m_pixelDepth == 32)))
		return false;

	ReadData(file, (char*)&ch_buf1, 1);

	ch_buf2 = 15; //00001111;
	m_alphaDepth = ch_buf1 & ch_buf2;

	if (!((m_alphaDepth == 0) || (m_alphaDepth == 8)))
		return false;

	if (truecolor)
	{
		m_type = itRGB;
		if (m_pixelDepth == 32)
			m_type = itRGBA;
	}

	if (m_type == itUndefined)
		return false;

	file.seekg(IDLength, std::ios::cur);

	m_pixels = (byte*)malloc(m_width*m_height*(m_pixelDepth / 8));

	if (!rle)
		ReadData(file, (char*)m_pixels, m_width*m_height*(m_pixelDepth / 8));
	else
	{
		while (CurrentPixel < m_width*m_height - 1)
		{
			ReadData(file, (char*)&ch_buf1, 1);
			if ((ch_buf1 & 128) == 128)
			{   // this is an rle packet
				ch_buf2 = (byte)((ch_buf1 & 127) + 1);   // how many pixels are encoded using this packet
				ReadData(file, (char*)buf1, m_pixelDepth / 8);
				for (uint i = CurrentPixel; i < CurrentPixel + ch_buf2; i++)
					for (uint j = 0; j < m_pixelDepth / 8; j++)
						m_pixels[i*m_pixelDepth / 8 + j] = buf1[j];
				CurrentPixel += ch_buf2;
			}
			else
			{   // this is a raw packet
				ch_buf2 = (byte)((ch_buf1 & 127) + 1);
				ReadData(file, (char*)buf1, m_pixelDepth / 8 * ch_buf2);
				for (uint i = CurrentPixel; i < CurrentPixel + ch_buf2; i++)
					for (uint j = 0; j < m_pixelDepth / 8; j++)
						m_pixels[i*m_pixelDepth / 8 + j] = buf1[(i - CurrentPixel)*m_pixelDepth / 8 + j];
				CurrentPixel += ch_buf2;
			}
		}
	}

	if (TGAReadError != 0)
	{
		Clear();
		return false;
	}
	m_loaded = true;

	// swap BGR(A) to RGB(A)

	byte temp;
	if ((m_type == itRGB) || (m_type == itRGBA))
		if ((m_pixelDepth == 24) || (m_pixelDepth == 32))
			for (uint i = 0; i < m_width*m_height; i++)
			{
				temp = m_pixels[i*m_pixelDepth / 8];
				m_pixels[i*m_pixelDepth / 8] = m_pixels[i*m_pixelDepth / 8 + 2];
				m_pixels[i*m_pixelDepth / 8 + 2] = temp;
			}

	return true;
}


//---------------------------------------------------------------------------
void cFileLoaderTGA::Clear()
{
	if (m_pixels)
		free(m_pixels);
	m_pixels = 0;
	m_loaded = false;
	m_width = 0;
	m_height = 0;
	m_pixelDepth = 0;
	m_alphaDepth = 0;
	m_type = itUndefined;
}


//---------------------------------------------------------------------------
uint cFileLoaderTGA::GetAlphaDepth()
{
	return m_alphaDepth;
}


//---------------------------------------------------------------------------
uint cFileLoaderTGA::GetImageWidth()
{
	return m_width;
}


//---------------------------------------------------------------------------
uint cFileLoaderTGA::GetImageHeight()
{
	return m_height;
}


//---------------------------------------------------------------------------
uint cFileLoaderTGA::GetPixelDepth()
{
	return m_pixelDepth;
}


//---------------------------------------------------------------------------
byte* cFileLoaderTGA::GetPixels()
{
	return m_pixels;
}


//---------------------------------------------------------------------------
LImageType cFileLoaderTGA::GetImageType()
{
	return m_type;
}

