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
	\author:    Dan Morris
	\version    1.0
	\date       3/2005
*/
//===========================================================================

#include "CLabelPanel.h"

// Default values for the user-adjustable parameters
#define DEFAULT_TOP_INTERNAL_BORDER 15
#define DEFAULT_BOTTOM_INTERNAL_BORDER 15
#define DEFAULT_LEFT_INTERNAL_BORDER 10
#define DEFAULT_RIGHT_INTERNAL_BORDER 10
#define DEFAULT_KEY_LABEL_SPACE 10
#define DEFAULT_KEY_SQUARE_SIZE 10
#define DEFAULT_INTER_LABEL_Y_STEP 25
#define DEFAULT_PANEL_WIDTH 150
#define DEFAULT_PANEL_HEIGHT 200


//===========================================================================
/*!
	Constructor of cLabelPanel
	\fn         cLabelPanel::cLabelPanel(cWorld* a_world)
	\param      a_world                     The parent CHAI world
*/
//===========================================================================
cLabelPanel::cLabelPanel(cWorld* a_world) : cPanel(a_world)
{

	m_halignment = ALIGN_CENTER;
	m_valignment = VALIGN_CENTER;
	m_interLabelSpacing = DEFAULT_INTER_LABEL_Y_STEP;

	// We don't create our font until we render, since font information is
	// only available once a display context has been created
	// m_font = 0;

	m_font = cFont::createFont();
	m_backupFont = 0;

	m_showSquares = false;
	m_textColor = cColorf(1, 1, 1, 1);

	setSize(cVector3d(DEFAULT_PANEL_WIDTH, DEFAULT_PANEL_HEIGHT, 0));

	m_layoutPending = 0;
	m_useLighting = true;

	m_topBorder = DEFAULT_TOP_INTERNAL_BORDER;
	m_leftBorder = DEFAULT_LEFT_INTERNAL_BORDER;
	m_bottomBorder = DEFAULT_BOTTOM_INTERNAL_BORDER;
	m_rightBorder = DEFAULT_RIGHT_INTERNAL_BORDER;
}


//===========================================================================
/*!
	Destructor of cLabelPanel
	\fn         cLabelPanel::~cLabelPanel()
*/
//===========================================================================
cLabelPanel::~cLabelPanel()
{
	clearLabels();
	if (m_font) delete m_font;
	if (m_backupFont) delete m_backupFont;
}


//===========================================================================
/*!
	Clear all stored strings
	\fn         cLabelPanel::clearLabels()
*/
//===========================================================================
void cLabelPanel::clearLabels()
{
	std::vector<char*>::iterator iter;
	for (iter = m_labels.begin(); iter != m_labels.end(); iter++) {
		delete[](*iter);
	}
	m_labels.clear();
	m_materials.clear();
	layout();
}


//===========================================================================
/*!
	Add a new line of text to be rendered.
	\fn         void cLabelPanel::addLabel(const char* a_label, const cMaterial* a_mat)
	\param      a_label         The text to print
	\param      a_mat           The color to put in the iconic box for this string, if you
								have called setShowSquares(true) to render squares for each
								label.  Pass 0 (the default) to use a default material.
*/
//===========================================================================
void cLabelPanel::addLabel(const char* a_label, const cMaterial* a_mat)
{
	char* buf = new char[strlen(a_label) + 1];
	strcpy(buf, a_label);
	m_labels.push_back(buf);
	if (a_mat == 0) {
		// Default material
		cMaterial mat;
		m_materials.push_back(mat);
	}
	else {
		m_materials.push_back(*a_mat);
		m_showSquares = true;
	}
	layout();
}


//===========================================================================
/*!
	Add a new line of text to be rendered.
	\fn         void cLabelPanel::setLabel(const unsigned int& a_index, const char* a_label)
	\param      a_index         Which label are we changing?
	\param      a_label         The text to print
*/
//===========================================================================
void cLabelPanel::setLabel(const unsigned int& a_index, const char* a_label)
{
	if (a_index > m_labels.size()) return;

	char* oldstr = m_labels[a_index];
	char* buf = new char[strlen(a_label) + 1];
	strcpy(buf, a_label);
	m_labels[a_index] = buf;
	delete[] oldstr;
	layout();
}



//===========================================================================
/*!
	Re-compute the size of the box; actually just sets a flag to do this the next
	time we render.
	\fn         void cLabelPanel::layout()
*/
//===========================================================================
void cLabelPanel::layout()
{
	m_layoutPending = 1;
}


//===========================================================================
/*!
	Re-compute the size of the box.
	\fn         void cLabelPanel::layoutImmediately()
*/
//===========================================================================
void cLabelPanel::layoutImmediately()
{

	// Initialize our font if necessary
	if (m_font == 0) buildFont();

	m_layoutPending = 0;

	// The longest line of text in our panel
	int maxwidth = 0;

	unsigned int numLabels = m_labels.size();
	unsigned int i;

	// Find the longest line of text in our panel
	for (i = 0; i < numLabels; i++)
	{
		char* label = m_labels[i];
		char* ptr;
		int width = 0;
		for (ptr = label; *ptr; ptr++) width += m_font->getCharacterWidth(*ptr);
		if (width > maxwidth) maxwidth = width;
	}

	// What should the total width of our panel be?
	float width = (float)(maxwidth)+m_leftBorder + m_rightBorder;
	if (m_showSquares) width += DEFAULT_KEY_LABEL_SPACE + DEFAULT_KEY_SQUARE_SIZE;

	// What should the total height of our panel be?
	float height = (float)(
		m_interLabelSpacing * (numLabels - 1) +
		m_font->getPointSize() * numLabels +
		m_topBorder +
		m_bottomBorder
		);

	setSize(cVector3d(width, height, 0));

	m_yStep = (int)(m_interLabelSpacing + m_font->getPointSize());
}


//===========================================================================
/*!
	Allows us to re-create font information after a GL context change
	\fn         void cLabelPanel::onDisplayReset(const bool a_affectChildren)
	\param      a_affectChildren    Should we recursively affect our children?
*/
//===========================================================================
void cLabelPanel::onDisplayReset(const bool a_affectChildren) {

	if (m_backupFont) delete m_backupFont;
	m_backupFont = m_font;
	m_font = 0;
	m_layoutPending = true;

	// Use the superclass method to call the same function on the rest of the
	// scene graph...
	cPanel::onDisplayReset(a_affectChildren);
}


//===========================================================================
/*!
	Build a font object, optionally referring to our "backup" copy of an old font
	\fn         void cLabelPanel::buildFont()
*/
//===========================================================================
void cLabelPanel::buildFont()
{
	if (m_font) delete m_font;

	// Copy the font from an old version of the font (from a previous context)...
	if (m_backupFont)
	{
		m_font = cFont::createFont(m_backupFont);
		delete m_backupFont;
		m_backupFont = 0;
	}

	// Otherwise just build a new font...
	else
	{
		m_font = cFont::createFont();
	}
}


//===========================================================================
/*!
	Render the panel and all strings to the screen
	\fn         void cLabelPanel::render(const int a_renderMode)
	\param      a_renderMode    The current rendering pass; see cGenericObject
*/
//===========================================================================
void cLabelPanel::render(const int a_renderMode)
{

	// Initialize our font if necessary
	if (m_font == 0) buildFont();

	// We don't do multipass for this pure-2D class, so only render on the opaque passes
	if (a_renderMode != CHAI_RENDER_MODE_RENDER_ALL &&
		a_renderMode != CHAI_RENDER_MODE_TRANSPARENT_FRONT_ONLY) return;

	// Lay myself out if necessary
	if (m_layoutPending) layoutImmediately();

	glPushMatrix();

	// Move my origin to reflect alignment
	if (m_halignment == ALIGN_LEFT) glTranslated(getSize().x / 2.0, 0, 0);
	else if (m_halignment == ALIGN_RIGHT) glTranslated(-1.0*getSize().x / 2.0, 0, 0);

	if (m_valignment == VALIGN_TOP) glTranslated(0, -1.0*getSize().y / 2.0, 0);
	else if (m_valignment == VALIGN_BOTTOM) glTranslated(0, getSize().y / 2.0, 0);

	// Render the background panel
	cPanel::render(a_renderMode);

	// Back up OpenGL normal state
	float old_normal[3];
	glGetFloatv(GL_CURRENT_NORMAL, old_normal);

	glDisableClientState(GL_NORMAL_ARRAY);
	glNormal3f(0, 0, 1.0f);

	if (m_useLighting) glEnable(GL_LIGHTING);
	else glDisable(GL_LIGHTING);

	cVector3d size = getSize();
	int starting_y = (int)(size.y / 2.0 - m_topBorder - m_font->getPointSize());
	int current_y = starting_y;
	int current_x = (int)(-1.0*size.x / 2.0) + m_leftBorder;

	int num_labels = m_labels.size();

	// Render each line of text
	int i;
	for (i = 0; i < num_labels; i++)
	{

		char* label = m_labels[i];
		cMaterial mat = m_materials[i];

		current_x = (int)(-1.0*size.x / 2.0) + m_leftBorder;

		// Render this line's colored square if necessary
		if (m_showSquares)
		{
			m_materials[i].render();
			glTranslatef((float)current_x, (float)(current_y - 1), 0);
			glBegin(GL_QUADS);
			glVertex3f(0, 0, 0);
			glVertex3f(DEFAULT_KEY_SQUARE_SIZE, 0, 0);
			glVertex3f(DEFAULT_KEY_SQUARE_SIZE, DEFAULT_KEY_SQUARE_SIZE, 0);
			glVertex3f(0, DEFAULT_KEY_SQUARE_SIZE, 0);
			glEnd();
			glTranslatef((float)(-current_x), (float)(-(current_y - 1)), 0);

			current_x += DEFAULT_KEY_SQUARE_SIZE;
			current_x += DEFAULT_KEY_LABEL_SPACE;
		}

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

		glColor4f(m_textColor[0], m_textColor[1], m_textColor[2], m_textColor[3]);

		// We set the z-position to 1 to push him up a bit
		glRasterPos3f((float)current_x, (float)current_y, 1);

		//glGetFloatv(GL_CURRENT_RASTER_POSITION,pos);
		//glGetFloatv(GL_CURRENT_RASTER_POSITION_VALID,&valid);

		m_font->renderString(label);

		current_y -= m_yStep;
	}

	// Restore opengl state
	glEnable(GL_LIGHTING);
	glNormal3f(old_normal[0], old_normal[1], old_normal[2]);

	glPopMatrix();
}



//===========================================================================
/*!
	Read out a particular label
	\fn         const char* cLabelPanel::getLabel(const unsigned int& a_index) const
	\param      a_index     Which label do you want to read?
	\return     The requested label, or 0 for an error
*/
//===========================================================================
const char* cLabelPanel::getLabel(const unsigned int& a_index) const
{
	if (a_index >= m_labels.size()) return 0;
	return m_labels[a_index];
}


//===========================================================================
/*!
	Should we show iconic colored squares next to each line of text?
	\fn         void cLabelPanel::setShowSquares(const bool& a_showSquares)
	\param      a_showSquares  True to show iconic squares
*/
//===========================================================================
void cLabelPanel::setShowSquares(const bool& a_showSquares)
{
	if (m_showSquares == a_showSquares) return;
	m_showSquares = a_showSquares;
	layout();
}


//===========================================================================
/*!
	Set the border size and spacing for labels within the panel
	\fn         void cLabelPanel::setBorders(const int& a_topBorder, const int& a_bottomBorder,
											const int& a_leftBorder, const int& a_rightBorder,
											const int& a_interLabelSpacing)
	\param      a_topBorder     Sets the width, in pixels, of a particular border.
	\param      a_bottomBorder  Sets the width, in pixels, of a particular border.
	\param      a_leftBorder    Sets the width, in pixels, of a particular border.
	\param      a_rightBorder   Sets the width, in pixels, of a particular border.
	\param      a_interLabelSpacing   Sets the spacing, in pixels, between labels.
*/
//===========================================================================
void cLabelPanel::setBorders(const int& a_topBorder, const int& a_bottomBorder, const int& a_leftBorder,
	const int& a_rightBorder, const int& a_interLabelSpacing)
{
	bool newValue = false;

	if ((a_topBorder >= 0) && (a_topBorder != m_topBorder)) newValue = true;
	if ((a_leftBorder >= 0) && (a_leftBorder != m_leftBorder)) newValue = true;
	if ((a_rightBorder >= 0) && (m_rightBorder != m_rightBorder)) newValue = true;
	if ((a_bottomBorder >= 0) && (a_bottomBorder != m_bottomBorder)) newValue = true;
	if ((a_interLabelSpacing >= 0) && (a_interLabelSpacing != m_interLabelSpacing)) newValue = true;

	if (!newValue) return;

	if (a_topBorder != -1) m_topBorder = a_topBorder;
	if (a_rightBorder != -1) m_rightBorder = a_rightBorder;
	if (a_leftBorder != -1) m_leftBorder = a_leftBorder;
	if (a_bottomBorder != -1) m_bottomBorder = a_bottomBorder;
	if (a_interLabelSpacing != -1) m_interLabelSpacing = a_interLabelSpacing;

	layout();
}


//===========================================================================
/*!
	Get the border size and spacing for labels within the panel
	\fn         void cLabelPanel::getBorders(int& a_topBorder, int& a_bottomBorder, int& a_leftBorder,
											int& a_rightBorder, int& a_interLabelSpacing) const
	\param      a_topBorder     Returns the width, in pixels, of a particular border.
	\param      a_bottomBorder  Returns the width, in pixels, of a particular border.
	\param      a_leftBorder    Returns the width, in pixels, of a particular border.
	\param      a_rightBorder   Returns the width, in pixels, of a particular border.
	\param      a_interLabelSpacing     Returns the spacing, in pixels, between labels.
*/
//===========================================================================
void cLabelPanel::getBorders(int& a_topBorder, int& a_bottomBorder, int& a_leftBorder,
	int& a_rightBorder, int& a_interLabelSpacing) const
{
	a_topBorder = m_topBorder;
	a_bottomBorder = m_bottomBorder;
	a_leftBorder = m_leftBorder;
	a_rightBorder = m_rightBorder;
	a_interLabelSpacing = m_interLabelSpacing;
}


//===========================================================================
/*!
	Set the alignment of the _panel_ relative to its position (_not_ the text alignment).
	Use the horizontal_label_panel_alignments and vertical_label_panel_alignments
	enumerations found in cLabelPanel.h .  For example, a right-aligned panel extends to the
	left of its current position and vice-versa.

	\fn         void cLabelPanel::setAlignment(const int& a_horizontalAlignment, const int& a_verticalAlignment)
	\param      a_horizontalAlignment  Should be ALIGN_CENTER, ALIGN_LEFT, or ALIGN_RIGHT
	\param      a_verticalAlignment    Should be VALIGN_CENTER, VALIGN_TOP, or VALIGN_BOTTOM
*/
//===========================================================================
void cLabelPanel::setAlignment(const int& a_horizontalAlignment, const int& a_verticalAlignment)
{
	bool newValue = false;

	if (a_horizontalAlignment >= 0 && a_horizontalAlignment != m_halignment) newValue = true;
	if (a_verticalAlignment >= 0 && a_verticalAlignment != m_valignment) newValue = true;

	if (!newValue) return;

	if (a_horizontalAlignment != -1) m_halignment = a_horizontalAlignment;
	if (a_verticalAlignment != -1) m_valignment = a_verticalAlignment;

	layout();
}


void cLabelPanel::getAlignment(int& a_horizontalAlignment, int& a_verticalAlignment) const
{
	a_horizontalAlignment = m_halignment;
	a_verticalAlignment = m_valignment;
}
