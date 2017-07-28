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

#ifndef CLabelPanelH
#define CLabelPanelH

#include "CPanel.h"
#include "CFont.h"
#include <list>

//! An numeration used to align text horizontally in cLabelPanels
typedef enum {
	ALIGN_CENTER = 0, ALIGN_LEFT, ALIGN_RIGHT
} horizontal_label_panel_alignments;

//! An enumeration used to align text vertically in cLabelPanels
typedef enum {
	VALIGN_CENTER = 0, VALIGN_TOP, VALIGN_BOTTOM
} vertical_label_panel_alignments;


//===========================================================================
/*!
\file       CLabelPanel.h
\class      cLabelPanel
\brief      This class represents a 2D panel with at least one piece of text
			on it, optionally including several pieces of text with colored
			markers next to them (like the key on a graph).

			The panel can be auto-sized to fit its contents.

			This class is intended to be used for 2D rendering.
*/
//===========================================================================
class cLabelPanel : public cPanel {

public:

	//! Constructor for cLabelPanel
	cLabelPanel(cWorld* a_world);
	//! Destructor for cLabelPanel
	virtual ~cLabelPanel();

	//! Add a label (an additional line of text) to the panel
	void addLabel(const char* a_label, const cMaterial* a_mat = 0);
	//! Set an existing label to a new line of text
	void setLabel(const unsigned int& a_index, const char* a_label);
	//! How many labels are currently displayed?
	unsigned int getNumLabels() const;
	//! Read out a particular label
	const char* getLabel(const unsigned int& a_index) const;
	//! Clear all labels from the panel
	void clearLabels();

	//! The panel's drawing function
	virtual void render(const int a_renderMode = 0);

	//! Allows us to re-create font information after a GL context change
	virtual void onDisplayReset(const bool a_affectChildren = true);

	// Layout options...

	//! Should we show iconic squares along with each label?
	void setShowSquares(const bool& a_showSquares);
	//! Are we showing iconic squares along with each label?
	bool getShowSquares() const { return m_showSquares; }

	//! Set the border size and spacing for labels within the panel
	void setBorders(const int& a_topBorder, const int& a_bottomBorder,
		const int& a_leftBorder, const int&a_rightBorder,
		const int& a_interLabelSpacing);
	//! Get the border size and spacing for labels within the panel
	void getBorders(int& a_topBorder, int& a_bottomBorder,
		int& a_leftBorder, int& a_rightBorder, int& a_interLabelSpacing) const;

	//! Set the alignment of the _panel_ relative to its position (_not_ the text alignment)
	void setAlignment(const int& a_horizontalAlignment, const int& a_verticalAlignment);
	//! Get the alignment of the _panel_ relative to its position (_not_ the text alignment)
	void getAlignment(int& a_horizontalAlignment, int& a_verticalAlignment) const;

	//! Get the font object used for text rendering
	cFont* getFont() const { return m_font; }

	//! What color should be used for text?
	cColorf m_textColor;

	//! Tell the panel to re-arrange its labels the next time it renders
	//!
	//! It's not usually necessary to call this directly.
	void layout();

	//! Tell the panel to re-arrange its labels _right now_
	//!
	//! It's not usually necessary to call this directly.
	void layoutImmediately();

protected:

	//! Should we show iconic squares for the different supplied materials?
	bool m_showSquares;

	//! The sizes, in pixels, of the borders (padding) inside the panel
	int m_topBorder, m_leftBorder, m_bottomBorder, m_rightBorder;

	//! The spacing - in pixels - between the lines in the panel
	int m_interLabelSpacing;

	//! The horizontal alignment of the panel relative to its position
	int m_halignment;
	//! The vertical alignment of the panel relative to its position
	int m_valignment;

	//! Do we need to re-layout the next time we render?
	int m_layoutPending;

	//! The actual label strings
	std::vector<char*> m_labels;

	//! The materials used for the iconic squares
	std::vector<cMaterial> m_materials;

	//! The total step from one label to the next
	int m_yStep;

	//! The font used for text rendering
	cFont* m_font;

	//! If the display context is nuked, we keep a copy of our font around so we
	//! can re-create the same font
	cFont* m_backupFont;

	//! Build a font object, optionally referring to our "backup" copy of an old font
	void buildFont();

};

#endif
