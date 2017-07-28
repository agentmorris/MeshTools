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
    \author:    Chris Sewell
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#if !defined(AFX_math_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_math_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include "CWorld.h"
#include "CViewport.h"
#include "CGenericObject.h"
#include "CPhantom3dofPointer.h"
#include "CPrecisionTimer.h"


class CmathApp : public CWinApp {
public:
	CmathApp();
  
  // Call this in place of a destructor to clean up
  void uninitialize();

  

protected:

  
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CmathApp)
	public:
	virtual BOOL InitInstance();
  virtual int Run();
  //}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CmathApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_math_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
