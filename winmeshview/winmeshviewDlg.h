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

#include "afxwin.h"
#include "winmeshview.h"
#include "resource.h"
#include "winmeshview_globals.h"
#include "CPrecisionClock.h"

#if !defined(AFX_winmeshviewDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
#define AFX_winmeshviewDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#define MAX_GL_WINDOW_WIDTH  1280
#define MAX_GL_WINDOW_HEIGHT 1024

class CwinmeshviewDlg : public CDialog, public winmeshview_dialog_type {

public:

	int initialized;

	cPrecisionClock clock;

	virtual void CwinmeshviewDlg::update_rendering_options_from_gui(CwinmeshviewApp* app,
		int preserve_loaded_properties = 0);
	virtual void CwinmeshviewDlg::copy_rendering_options_to_gui(CwinmeshviewApp* app);

	bool m_trackingMouse;

	CwinmeshviewDlg(CWnd* pParent = NULL);
	CwinmeshviewDlg(int idd, CWnd* pParent = NULL);
	void initialize();
	void shutdown() {
		OnClose();
		OnDestroy();
	}

	CWnd* m_gl_wnd;

	// Floating-point values grabbed from the various slider
	// controls...
	float m_dynamic_friction, m_static_friction, m_stiffness, m_separation, m_focus, m_cutplane_position;

	// Dialog Data
		//{{AFX_DATA(CwinmeshviewDlg)
	enum { IDD = IDD_winmeshview_DIALOG };
	CSliderCtrl	m_stereo_focus_slider;
	CSliderCtrl m_cutplane_slider;
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
	BOOL  m_showEdgesCheck;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CwinmeshviewDlg)
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


protected:
	HICON m_hIcon;

	// Updates the text associated with the haptic property sliders
	void update_slider_text();

	// Generated message map functions
	  //{{AFX_MSG(CwinmeshviewDlg)
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
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	virtual afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
protected:
	afx_msg void OnToggleHapticsButton();
	afx_msg void OnToggleStereoButton();
	afx_msg void OnAnimationButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBuildAABBButton();
	CComboBox m_cutplane_combo;
	afx_msg void OnCutComboSelChange();
	afx_msg void OnReverseNormalsButton();
	afx_msg void OnExportModelButton();
	afx_msg void OnResetCameraButton();
	BOOL m_invert_clip_plane_check;
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_winmeshviewDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
