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
#include "CColor.h"
//---------------------------------------------------------------------------

cColorf CHAI_COLOR_RED(1.0f, 0.0f, 0.0f, 1.0f);
cColorf CHAI_COLOR_GREEN(0.0f, 1.0f, 0.0f, 1.0f);
cColorf CHAI_COLOR_BLUE(0.0f, 0.0f, 1.0f, 1.0f);
cColorf CHAI_COLOR_WHITE(1.0f, 1.0f, 1.0f, 1.0f);
cColorf CHAI_COLOR_BLACK(0.0f, 0.0f, 0.0f, 1.0f);
cColorf CHAI_COLOR_YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
cColorf CHAI_COLOR_AQUA(0.0f, 1.0f, 1.0f, 1.0f);
cColorf CHAI_COLOR_PURPLE(1.0f, 0.0f, 1.0f, 1.0f);

cColorf CHAI_BASIC_COLORS[N_CHAI_BASIC_COLORS] = {
  CHAI_COLOR_RED, CHAI_COLOR_GREEN, CHAI_COLOR_BLUE,
  CHAI_COLOR_WHITE, CHAI_COLOR_BLACK, CHAI_COLOR_YELLOW,
  CHAI_COLOR_AQUA, CHAI_COLOR_PURPLE
};

cColorf CHAI_COLOR_PASTEL_RED(1.0f, 0.5f, 0.5f, 1.0f);
cColorf CHAI_COLOR_PASTEL_GREEN(0.5f, 1.0f, 0.5f, 1.0f);
cColorf CHAI_COLOR_PASTEL_BLUE(0.5f, 0.5f, 1.0f, 1.0f);
cColorf CHAI_COLOR_PASTEL_AQUA(0.0f, 1.0f, 1.0f, 1.0f);
cColorf CHAI_COLOR_PASTEL_VIOLET(1.0f, 0.5f, 1.0f, 1.0f);
cColorf CHAI_COLOR_PASTEL_YELLOW(0.5F, 1.0f, 1.0f, 1.0f);
cColorf CHAI_COLOR_PASTEL_GRAY(0.5f, 0.5f, 0.5f, 1.0f);

cColorf CHAI_PASTEL_COLORS[N_CHAI_PASTEL_COLORS] = {
  CHAI_COLOR_PASTEL_RED, CHAI_COLOR_PASTEL_GREEN,
  CHAI_COLOR_PASTEL_BLUE, CHAI_COLOR_PASTEL_AQUA,
  CHAI_COLOR_PASTEL_VIOLET, CHAI_COLOR_PASTEL_YELLOW,
  CHAI_COLOR_PASTEL_GRAY
};
