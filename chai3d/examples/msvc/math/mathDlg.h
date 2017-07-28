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

#if !defined(AFX_mathDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
#define AFX_mathDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include <CVector3d.h>
#include <CMatrix3d.h>

class CmathDlg : public CDialog {

public:
	CmathDlg(CWnd* pParent = NULL);
  
  // vectors
  cVector3d v0;
  cVector3d v1;
  cVector3d vr;

  // matrices
  cMatrix3d m0;
  cMatrix3d m1;
  cMatrix3d mr;

  bool updating;

  double vValue;

// Dialog Data
	//{{AFX_DATA(CmathDlg)
	enum { IDD = IDD_math_DIALOG };
	CSliderCtrl	m_camera_zoom_slider;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CmathDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
  
  
protected:
	HICON m_hIcon;

	void updateAll();
  
  // Generated message map functions
	//{{AFX_MSG(CmathDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnChangeEdit();
	afx_msg void OnButton5();
	afx_msg void OnButton10();
	afx_msg void OnButton6();
	afx_msg void OnButton11();
	afx_msg void OnButton7();
	afx_msg void OnButton12();
	afx_msg void OnButton8();
	afx_msg void OnButton9();
	afx_msg void OnButton13();
	afx_msg void OnButton14();
	afx_msg void OnButton19();
	afx_msg void OnButton15();
	afx_msg void OnButton20();
	afx_msg void OnButton16();
	afx_msg void OnButton21();
	afx_msg void OnButton17();
	afx_msg void OnButton22();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_mathDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
