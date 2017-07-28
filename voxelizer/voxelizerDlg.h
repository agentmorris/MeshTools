/******
 *
 * Written by Dan Morris
 * dmorris@cs.stanford.edu
 * http://cs.stanford.edu/~dmorris
 *
 * You can do anything you want with this file as long as this header
 * stays on it and I am credited when it's appropriate.
 *
 ******/

#if !defined(AFX_voxelizerDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
#define AFX_voxelizerDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include "OpenGL_dlgwin.h"
#include "afxwin.h"
#include "NumEdit.h"

#define MAX_GL_WINDOW_WIDTH  1280
#define MAX_GL_WINDOW_HEIGHT 1024

class CvoxelizerDlg : public CDialog {

public:
	CvoxelizerDlg(CWnd* pParent = NULL);

	CWnd* m_gl_wnd;
	HWND m_gl_area_hwnd;

	// Dialog Data
		//{{AFX_DATA(CvoxelizerDlg)
	enum { IDD = IDD_voxelizer_DIALOG };
	CSliderCtrl	m_camera_zoom_slider;
	BOOL	m_material_check;
	BOOL	m_showbox_check;
	BOOL	m_showframe_check;
	BOOL	m_shownormals_check;
	BOOL	m_usecolors_check;
	BOOL	m_usetexture_check;
	BOOL	m_usewireframe_check;
	BOOL	m_renderpoints_check;
	BOOL  m_rendermesh_check;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CvoxelizerDlg)
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


protected:
	HICON m_hIcon;
	int m_left_scrolling_gl_area;
	int m_middle_scrolling_gl_area;
	int m_right_scrolling_gl_area;

	// The last point scrolled through by each mouse button,
	// in _viewport_ coordinates (i.e. (0,0) is the top-left
	// corner of the viewport area)
	CPoint last_left_scroll_point;
	CPoint last_right_scroll_point;
	CPoint last_middle_scroll_point;

	// Generated message map functions
	  //{{AFX_MSG(CvoxelizerDlg)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnCheck();
	afx_msg void OnLoadModelButton();
	afx_msg void OnLoadTextureButton();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
protected:
	afx_msg void OnToggleHapticsButton();
	afx_msg void OnVoxelizeButton();
	afx_msg void OnQuitvoxelizingButton();
	afx_msg void OnLoadSubtractorButton();
	afx_msg void OnLoadModifierButton();
public:

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTetrahedralizeButton();
	afx_msg void OnTestVoxelizeButton();
	CNumEdit m_voxelres_edit;
	CNumEdit m_normalmult_edit;

	afx_msg void OnDistanceCheck();
	BOOL m_distance_check;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_voxelizerDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
