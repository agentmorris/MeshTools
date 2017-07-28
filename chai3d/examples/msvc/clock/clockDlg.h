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

// clockDlg.h : header file
//

#if !defined(AFX_CLOCKDLG_H__785D63F5_30F9_4345_B211_AAA0E77CFCD8__INCLUDED_)
#define AFX_CLOCKDLG_H__785D63F5_30F9_4345_B211_AAA0E77CFCD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CPrecisionClock.h"

/////////////////////////////////////////////////////////////////////////////
// CClockDlg dialog

class CClockDlg : public CDialog
{
// Construction
public:
	CClockDlg(CWnd* pParent = NULL);	// standard constructor

	cPrecisionClock *time;

// Dialog Data
	//{{AFX_DATA(CClockDlg)
	enum { IDD = IDD_CLOCK_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClockDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CClockDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnStart();
	afx_msg void OnStop();
	afx_msg void OnInit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLOCKDLG_H__785D63F5_30F9_4345_B211_AAA0E77CFCD8__INCLUDED_)
