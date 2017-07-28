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
    \date       05/2006
*/
//===========================================================================

#if !defined(AFX_mesh_mesh_collisionsDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
#define AFX_mesh_mesh_collisionsDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

class Cmesh_mesh_collisionsDlg : public CDialog {

public:
	Cmesh_mesh_collisionsDlg(CWnd* pParent = NULL);
  HWND m_gl_area_hwnd; 
  int initialized;

// Dialog Data
	//{{AFX_DATA(Cmesh_mesh_collisionsDlg)
	enum { IDD = IDD_mesh_mesh_collisions_DIALOG };
	CSliderCtrl	m_camera_zoom_slider;
	CButton m_showAll;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cmesh_mesh_collisionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
  
  
protected:
	HICON m_hIcon;

  // Booleans indicating whether we're currently scrolling with
  // the left and/or right mouse buttons
  int m_left_scrolling_gl_area;
  int m_right_scrolling_gl_area;

  // The last point scrolled through by each mouse button,
  // in _viewport_ coordinates (i.e. (0,0) is the top-left
  // corner of the viewport area)
  CPoint last_left_scroll_point;
  CPoint last_right_scroll_point;
  
  // Generated message map functions
	//{{AFX_MSG(Cmesh_mesh_collisionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnToggleHapticsButton();
	afx_msg void OnCheck1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
  afx_msg void OnSize(UINT nType, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_mesh_mesh_collisionsDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
