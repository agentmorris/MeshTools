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
    \date       06/2004
*/
//===========================================================================

#if !defined(AFX_object_loaderDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
#define AFX_object_loaderDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

class Cobject_loaderDlg : public CDialog {

public:
	Cobject_loaderDlg(CWnd* pParent = NULL);

  CWnd* m_gl_wnd;
  HWND m_gl_area_hwnd;  

  // Floating-point values grabbed from the various slider
  // controls...
  float m_dynamic_friction, m_static_friction, m_stiffness, m_separation, m_focus;

// Dialog Data
	//{{AFX_DATA(Cobject_loaderDlg)
	enum { IDD = IDD_object_loader_DIALOG };
	CSliderCtrl	m_stereo_focus_slider;
	CSliderCtrl	m_stereo_separation_slider;
	CStatic	m_stereo_separation_text;
	CStatic	m_stereo_focus_text;
	CStatic	m_stiffness_text;
	CStatic	m_static_friction_text;
	CStatic	m_dynamic_friction_text;
	CSliderCtrl	m_static_friction_slider;
	CSliderCtrl	m_stiffness_slider;
	CSliderCtrl	m_dynamic_friction_slider;
	CSliderCtrl	m_camera_zoom_slider;
	BOOL	m_material_check;
	BOOL	m_showbox_check;
	BOOL	m_showframe_check;
	BOOL	m_shownormals_check;
	BOOL	m_usecolors_check;
	BOOL	m_usetexture_check;
	BOOL	m_usewireframe_check;
  BOOL  m_culling_check;
  BOOL  m_transparency_check;
  BOOL  m_shaders_check;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cobject_loaderDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
  
  
protected:
	HICON m_hIcon;

  // These variables will be -1 if we just started scrolling, 0
  // if we're not scrolling, 1 if we're really scrolling now
  //
  // This is necessary because setcapture apparently changes the
  // origin of mouse events in a bizarre way...
  int m_left_scrolling_gl_area;
  int m_middle_scrolling_gl_area;
  int m_right_scrolling_gl_area;
  int initialized;

  // The last point scrolled through by each mouse button,
  // in _viewport_ coordinates (i.e. (0,0) is the top-left
  // corner of the viewport area)
  CPoint last_left_scroll_point;
  CPoint last_right_scroll_point;
  CPoint last_middle_scroll_point;
  
  // Updates the text associated with the haptic property sliders
  void update_slider_text();

  // Generated message map functions
	//{{AFX_MSG(Cobject_loaderDlg)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnCheck();
  
  afx_msg void OnMaterialCheck();
  afx_msg void OnTransparencyCheck();
  afx_msg void OnColorsCheck();

	afx_msg void OnLoadModelButton();
	afx_msg void OnLoadTextureButton();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnToggleHapticsButton();
	afx_msg void OnToggleStereoButton();
	afx_msg void OnAnimationButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:  
  afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnLoadShaderButton();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_object_loaderDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
