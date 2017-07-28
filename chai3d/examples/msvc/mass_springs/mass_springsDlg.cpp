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

#include "stdafx.h"
#include "mass_springs.h"
#include "mass_springsDlg.h"
#include "mass_springs_globals.h"

#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int right_aligned_elements[] = {
  IDC_TOGGLEHAPTICS_BUTTON, IDC_CAMZOOM_SLIDER,
    IDC_STATIC, IDC_ADDBALL_BUTTON, IDC_REMOVEBALL_BUTTON
};
#define NUM_RIGHT_ALIGNED_ELEMENTS 5

static int bottom_aligned_elements[] = {
 IDC_STATIC_1
};
#define NUM_BOTTOM_ALIGNED_ELEMENTS 1

// Positions relative to the parent window left and _bottom_
CRect initial_ralign_positions[NUM_RIGHT_ALIGNED_ELEMENTS];
CRect initial_balign_positions[NUM_BOTTOM_ALIGNED_ELEMENTS];
CRect initial_dlg_rect;
CRect initial_dlg_window_rect;
CRect initial_gl_area_rect;
CRect initial_gl_wnd_rect;


Cmass_springsDlg::Cmass_springsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cmass_springsDlg::IDD, pParent) {
	//{{AFX_DATA_INIT(Cmass_springsDlg)
	//}}AFX_DATA_INIT
  initialized = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  
}

void Cmass_springsDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cmass_springsDlg)
	DDX_Control(pDX, IDC_CAMZOOM_SLIDER, m_camera_zoom_slider);	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(Cmass_springsDlg, CDialog)
	//{{AFX_MSG_MAP(Cmass_springsDlg)
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
  ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
  ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
  ON_BN_CLICKED(IDC_TOGGLEHAPTICS_BUTTON, OnToggleHapticsButton)
	ON_BN_CLICKED(IDC_ADDBALL_BUTTON, OnAddBallButton)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_REMOVEBALL_BUTTON, OnRemoveBallButton)
	//}}AFX_MSG_MAP
  ON_WM_SIZING()
  ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL Cmass_springsDlg::OnInitDialog() {

  m_left_scrolling_gl_area = 0;
  m_right_scrolling_gl_area = 0;
  initialized = 0;

	CDialog::OnInitDialog();

  GetWindowRect(&initial_dlg_window_rect);
  GetClientRect(&initial_dlg_rect);

  int i;
  for(i=0; i<NUM_RIGHT_ALIGNED_ELEMENTS; i++) {
    CWnd* c = this->GetDlgItem(right_aligned_elements[i]);
    RECT r;
    c->GetWindowRect(&r);
    ScreenToClient(&r);
    initial_ralign_positions[i] = r;
  }

  for(i=0; i<NUM_BOTTOM_ALIGNED_ELEMENTS; i++) {
    CWnd* c = this->GetDlgItem(bottom_aligned_elements[i]);
    RECT r;
    c->GetWindowRect(&r);
    ScreenToClient(&r);
    initial_balign_positions[i] = r;
  }

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	
  CWnd* pWnd = GetDlgItem(IDC_GL_AREA);
  m_gl_area_hwnd = pWnd->m_hWnd;

  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);
  initial_gl_area_rect = r;

  // Map from 0 to 100 chai units
  m_camera_zoom_slider.SetRange(0,180,1);
  m_camera_zoom_slider.SetPos(45);
  
  initialized = 1;
  return TRUE;
}


HCURSOR Cmass_springsDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}


void Cmass_springsDlg::OnClose() {
	
  g_main_app->uninitialize();

  FreeConsole();
  exit(0);

	CDialog::OnClose();
}


void Cmass_springsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	
  if ((void*)(pScrollBar) == (void*)(&m_camera_zoom_slider)) {

    UpdateData(TRUE);
    int position = m_camera_zoom_slider.GetPos();
    g_main_app->zoom(position);
  }

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void Cmass_springsDlg::OnMouseMove(UINT nFlags, CPoint point) {

	CDialog::OnMouseMove(nFlags, point);

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA);
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);

  // convert to viewport coordinates
  point.x -= r.left;
  point.y -= r.top;  

  if (m_left_scrolling_gl_area) {
    CPoint delta = point - last_left_scroll_point;    
    g_main_app->scroll(delta,1);
    last_left_scroll_point = point;
  }

  if (m_right_scrolling_gl_area) {
    CPoint delta = point - last_right_scroll_point;
    g_main_app->scroll(delta,0);
    last_right_scroll_point = point;
  }
}


void Cmass_springsDlg::OnLButtonDown(UINT nFlags, CPoint point) {

  CDialog::OnLButtonDown(nFlags, point);

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA);
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);

  if (PtInRect(&r,point)) {

    // convert to viewport coordinates
    point.x -= r.left;
    point.y -= r.top;      
  
    m_left_scrolling_gl_area = 1;
    last_left_scroll_point = point;
    g_main_app->select(point);
  }	
  
}


void Cmass_springsDlg::OnLButtonUp(UINT nFlags, CPoint point) {
	
  CDialog::OnLButtonUp(nFlags, point);  
  m_left_scrolling_gl_area = 0;

}


void Cmass_springsDlg::OnRButtonDown(UINT nFlags, CPoint point) {

  CDialog::OnRButtonDown(nFlags, point);

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA);
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);
  
  if (PtInRect(&r,point)) {

    // convert to viewport coordinates
    point.x -= r.left;
    point.y -= r.top;    

    m_right_scrolling_gl_area = 1;
    last_right_scroll_point = point;
    g_main_app->select(point);
  }	
  
}


void Cmass_springsDlg::OnRButtonUp(UINT nFlags, CPoint point) {
	
  CDialog::OnRButtonUp(nFlags, point);
  m_right_scrolling_gl_area = 0;

}


void Cmass_springsDlg::OnToggleHapticsButton() {
	
  g_main_app->toggle_haptics();

  CButton* button = (CButton*)(GetDlgItem(IDC_TOGGLEHAPTICS_BUTTON));
  button->SetWindowText(g_main_app->haptics_enabled?"Disable haptics":"Enable haptics");
	
}

void Cmass_springsDlg::OnAddBallButton() {
	
	g_main_app->addball_pending = 1;
}

void Cmass_springsDlg::OnRemoveBallButton() {
	
  g_main_app->removeball_pending = 1;

}


void Cmass_springsDlg::OnSizing(UINT fwSide, LPRECT pRect)
{

  int neww = pRect->right - pRect->left;
  int initw = initial_dlg_window_rect.right - initial_dlg_window_rect.left;

  if (neww < initw) {
    if (fwSide == WMSZ_RIGHT || fwSide == WMSZ_TOPRIGHT || fwSide == WMSZ_BOTTOMRIGHT)
      pRect->right += initw - neww;
    else
      pRect->left -= initw - neww;
  }

  int newh = pRect->bottom - pRect->top;
  int inith = initial_dlg_window_rect.bottom - initial_dlg_window_rect.top;

  if (newh < inith) {
    if (fwSide == WMSZ_BOTTOM || fwSide == WMSZ_BOTTOMLEFT || fwSide == WMSZ_BOTTOMRIGHT)
      pRect->bottom += inith - newh;
    else
      pRect->top -= inith - newh;
  }

  // Repaint
  Invalidate();
  UpdateWindow();
}


void Cmass_springsDlg::OnSize(UINT nType, int cx, int cy)
{
  CDialog::OnSize(nType, cx, cy);

  if (initialized==0) return;

  RECT dlgr;
  GetClientRect(&dlgr);

  int i;

  for(i=0; i<NUM_RIGHT_ALIGNED_ELEMENTS; i++) {
    CWnd* c = GetDlgItem(right_aligned_elements[i]);
    int roffset = initial_dlg_rect.right - 
      initial_ralign_positions[i].left;

    RECT r;
    r.left = dlgr.right - roffset;
    r.top = initial_ralign_positions[i].top;
    // width and height
    r.right = initial_ralign_positions[i].right -
      initial_ralign_positions[i].left;
    r.bottom =       initial_ralign_positions[i].bottom -
      initial_ralign_positions[i].top;
    c->MoveWindow(r.left,r.top,r.right,r.bottom);
  }

  for(i=0; i<NUM_BOTTOM_ALIGNED_ELEMENTS; i++) {
    CWnd* c = GetDlgItem(bottom_aligned_elements[i]);
    int boffset = initial_dlg_rect.bottom - 
      initial_balign_positions[i].top;

    RECT r;
    r.left = initial_balign_positions[i].left;
    r.top = dlgr.bottom - boffset;

    // width and height
    r.right = initial_balign_positions[i].right -
      initial_balign_positions[i].left;
    r.bottom =       initial_balign_positions[i].bottom -
      initial_balign_positions[i].top;
    c->MoveWindow(r.left,r.top,r.right,r.bottom);
  }


  // Resize GL area
  RECT r = initial_gl_area_rect;
  r.bottom += dlgr.bottom - initial_dlg_rect.bottom;
  r.right += dlgr.right - initial_dlg_rect.right;

  CWnd* c = GetDlgItem(IDC_GL_AREA);
  c->MoveWindow(r.left,r.top,r.right - r.left, r.bottom - r.top, false);

  r = initial_gl_wnd_rect;
  r.bottom += dlgr.bottom - initial_dlg_rect.bottom;
  r.right += dlgr.right - initial_dlg_rect.right;

  //m_gl_wnd->MoveWindow(r.left,r.top,r.right - r.left, r.bottom - r.top, false);

  g_main_app->viewport->update(true);

  // Repaint
  Invalidate();
  UpdateWindow();
  RedrawWindow(0,0,RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}
