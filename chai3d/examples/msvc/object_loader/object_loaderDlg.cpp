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
#include "object_loader.h"
#include "object_loaderDlg.h"
#include "object_loader_globals.h"

#include <conio.h>
#include ".\object_loaderdlg.h"

// Turn off the annoying precision warning in msvc...
#ifdef _MSVC
#pragma warning(disable: 4244)
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Default haptic parameters for new objects
#define DEFAULT_STIFFNESS 50.0f
#define DEFAULT_STATIC_FRICTION 0.6f
#define DEFAULT_DYNAMIC_FRICTION 0.45f

#define DEFAULT_STEREO_SEPARATION 0.5f
#define DEFAULT_STEREO_FOCUS 4.0f

// This many slider scale units equals one friction unit
#define FRICTION_SLIDER_SCALE 100.0

#define MAXIMUM_FRICTION 2.0

#define SEPARATION_SLIDER_SCALE 50.0
#define FOCUS_SLIDER_SCALE 10.0

static int right_aligned_elements[] = {
  IDC_STEREOFOCUS_SLIDER, IDC_STATIC, IDC_CHECK_CULLING,
    IDC_CHECK_MATERIAL, IDC_CHECK_SHOWBOX, IDC_CHECK_TRANSPARENCY,
    IDC_CHECK_WIREFRAME, IDC_CHECK_SHOWNORMALS, IDC_LOAD_MODEL_BUTTON,
    IDC_LOAD_TEXTURE_BUTTON, IDC_STATIC, IDC_STEREOSEP_SLIDER,
    IDC_STEREOFOCUS_TEXT, IDC_STEREOSEP_SLIDER, IDC_STEREOSEP_TEXT,
    IDC_TOGGLE_STEREO_BUTTON, IDC_CAMZOOM_SLIDER, IDC_CHECK_SHOWFRAME,
    IDC_CHECK_USECOLORS, IDC_CHECK_USETEXTURE, IDC_STATIC3,
    IDC_CHECK_SHADERS, IDC_LOAD_SHADER_BUTTON
};
#define NUM_RIGHT_ALIGNED_ELEMENTS 23

static int bottom_aligned_elements[] = {
  IDC_TOGGLEHAPTICS_BUTTON, IDC_ANIMATION_BUTTON, IDC_DYNAMIC_FRICTION_SLIDER,
    IDC_STATICFRICTION_TEXT, IDC_STATICFRICTION_SLIDER, IDC_STIFFNESS_TEXT,
    IDC_STIFFNESS_SLIDER, IDC_STATIC4, IDC_STATIC5,
    IDC_STATIC6, IDC_DYNAMIC_FRICTION_RADIUS_TEXT
};
#define NUM_BOTTOM_ALIGNED_ELEMENTS 11

// Positions relative to the parent window left and _bottom_
CRect initial_ralign_positions[NUM_RIGHT_ALIGNED_ELEMENTS];
CRect initial_balign_positions[NUM_BOTTOM_ALIGNED_ELEMENTS];
CRect initial_dlg_rect;
CRect initial_dlg_window_rect;
CRect initial_gl_area_rect;
CRect initial_gl_wnd_rect;

Cobject_loaderDlg::Cobject_loaderDlg(CWnd* pParent /*=NULL*/)
: CDialog(Cobject_loaderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(Cobject_loaderDlg)
  initialized = 0;
	m_material_check = TRUE;
	m_showbox_check = FALSE;
	m_showframe_check = FALSE;
	m_shownormals_check = FALSE;
	m_usecolors_check = TRUE;
	m_usetexture_check = TRUE;
	m_usewireframe_check = FALSE;
  m_culling_check = TRUE;
  m_transparency_check = FALSE;
  m_shaders_check = TRUE;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  
}

void Cobject_loaderDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cobject_loaderDlg)
	DDX_Control(pDX, IDC_STEREOFOCUS_SLIDER, m_stereo_focus_slider);
	DDX_Control(pDX, IDC_STEREOSEP_SLIDER, m_stereo_separation_slider);
	DDX_Control(pDX, IDC_STEREOSEP_TEXT, m_stereo_separation_text);
	DDX_Control(pDX, IDC_STEREOFOCUS_TEXT, m_stereo_focus_text);
	DDX_Control(pDX, IDC_STIFFNESS_TEXT, m_stiffness_text);
	DDX_Control(pDX, IDC_STATICFRICTION_TEXT, m_static_friction_text);
	DDX_Control(pDX, IDC_DYNAMIC_FRICTION_RADIUS_TEXT, m_dynamic_friction_text);
	DDX_Control(pDX, IDC_STATICFRICTION_SLIDER, m_static_friction_slider);
	DDX_Control(pDX, IDC_STIFFNESS_SLIDER, m_stiffness_slider);
	DDX_Control(pDX, IDC_DYNAMIC_FRICTION_SLIDER, m_dynamic_friction_slider);
	DDX_Control(pDX, IDC_CAMZOOM_SLIDER, m_camera_zoom_slider);
	DDX_Check(pDX, IDC_CHECK_MATERIAL, m_material_check);
  DDX_Check(pDX, IDC_CHECK_CULLING, m_culling_check);
  DDX_Check(pDX, IDC_CHECK_TRANSPARENCY, m_transparency_check);
	DDX_Check(pDX, IDC_CHECK_SHOWBOX, m_showbox_check);
	DDX_Check(pDX, IDC_CHECK_SHOWFRAME, m_showframe_check);
	DDX_Check(pDX, IDC_CHECK_SHOWNORMALS, m_shownormals_check);
	DDX_Check(pDX, IDC_CHECK_USECOLORS, m_usecolors_check);
	DDX_Check(pDX, IDC_CHECK_USETEXTURE, m_usetexture_check);
	DDX_Check(pDX, IDC_CHECK_WIREFRAME, m_usewireframe_check);
  DDX_Check(pDX, IDC_CHECK_SHADERS, m_shaders_check);

	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(Cobject_loaderDlg, CDialog)
	//{{AFX_MSG_MAP(Cobject_loaderDlg)
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
  ON_BN_CLICKED(IDC_CHECK_MATERIAL, OnMaterialCheck)
  ON_BN_CLICKED(IDC_CHECK_CULLING, OnCheck)
  ON_BN_CLICKED(IDC_CHECK_TRANSPARENCY, OnTransparencyCheck)
	ON_BN_CLICKED(IDC_LOAD_MODEL_BUTTON, OnLoadModelButton)
	ON_BN_CLICKED(IDC_LOAD_TEXTURE_BUTTON, OnLoadTextureButton)
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
  ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
  ON_WM_MBUTTONDOWN()
  ON_WM_MBUTTONUP()
	ON_BN_CLICKED(IDC_TOGGLEHAPTICS_BUTTON, OnToggleHapticsButton)
	ON_BN_CLICKED(IDC_TOGGLE_STEREO_BUTTON, OnToggleStereoButton)
	ON_WM_PAINT()
  ON_BN_CLICKED(IDC_CHECK_SHOWBOX, OnCheck)
  ON_BN_CLICKED(IDC_CHECK_SHOWFRAME, OnCheck)
  ON_BN_CLICKED(IDC_CHECK_SHOWNORMALS, OnCheck)
  ON_BN_CLICKED(IDC_CHECK_USECOLORS, OnColorsCheck)
  ON_BN_CLICKED(IDC_CHECK_USETEXTURE, OnCheck)
  ON_BN_CLICKED(IDC_CHECK_WIREFRAME, OnCheck)
  ON_BN_CLICKED(IDC_CHECK_SHADERS, OnCheck)
	ON_BN_CLICKED(IDC_ANIMATION_BUTTON, OnAnimationButton)
  ON_BN_CLICKED(IDC_LOAD_SHADER_BUTTON, OnLoadShaderButton)
	//}}AFX_MSG_MAP
  ON_WM_SIZING()
  ON_WM_SIZE()  
END_MESSAGE_MAP()

BOOL Cobject_loaderDlg::OnInitDialog() {

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

  m_gl_wnd = 0;
  m_left_scrolling_gl_area = 0;
  m_right_scrolling_gl_area = 0;
  m_middle_scrolling_gl_area = 0;

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA);  
  
  RECT child;
  pWnd->GetClientRect(&child);
  
  // Create our opengl window
  m_gl_wnd = new CWnd();
  m_gl_wnd->Create(NULL,
						       NULL,
						       WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE,
						       child,
						       pWnd,
						       0);
  m_gl_area_hwnd = m_gl_wnd->m_hWnd;
  
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);
  initial_gl_area_rect = r;

  m_gl_wnd->GetWindowRect(&r);
  pWnd->ScreenToClient(&r);
  initial_gl_wnd_rect = r;

  // Map field of view from 0 to 180 chai units
  m_camera_zoom_slider.SetRange(0,180,1);
  m_camera_zoom_slider.SetPos(45);
  
  // Set up haptic property sliders
  m_static_friction_slider.SetRange(0,FRICTION_SLIDER_SCALE*MAXIMUM_FRICTION,1);
  m_static_friction = DEFAULT_STATIC_FRICTION;
  m_static_friction_slider.SetPos(m_static_friction*FRICTION_SLIDER_SCALE);

  m_dynamic_friction_slider.SetRange(0,FRICTION_SLIDER_SCALE*MAXIMUM_FRICTION,1);
  m_dynamic_friction = DEFAULT_DYNAMIC_FRICTION;
  m_dynamic_friction_slider.SetPos(m_dynamic_friction*FRICTION_SLIDER_SCALE);

  m_stiffness_slider.SetRange(0,100,1);
  m_stiffness = DEFAULT_STIFFNESS;
  m_stiffness_slider.SetPos(m_stiffness);

  m_stereo_separation_slider.SetRange(0,100,1);
  m_separation = DEFAULT_STEREO_SEPARATION;
  m_stereo_separation_slider.SetPos(m_separation*SEPARATION_SLIDER_SCALE);
  
  m_stereo_focus_slider.SetRange(0,100,1);
  m_focus = DEFAULT_STEREO_FOCUS;
  m_stereo_focus_slider.SetPos(m_focus*FOCUS_SLIDER_SCALE);

  update_slider_text();

  UpdateData(FALSE);
  
  initialized = 1;

  return TRUE;
}


void Cobject_loaderDlg::update_slider_text() {

  char buf[100];

  sprintf(buf,"Stiffness: %3.3f",m_stiffness);
  m_stiffness_text.SetWindowText(buf);
  
  sprintf(buf,"Dynamic friction radius: %3.3f",m_dynamic_friction);
  m_dynamic_friction_text.SetWindowText(buf);

  sprintf(buf,"Static friction radius: %3.3f",m_static_friction);
  m_static_friction_text.SetWindowText(buf);

  sprintf(buf,"Stereo separation: %3.3f",m_separation);
  m_stereo_separation_text.SetWindowText(buf);

  sprintf(buf,"Stereo focal length: %3.3f",m_focus);
  m_stereo_focus_text.SetWindowText(buf);

  UpdateData(FALSE);

}


HCURSOR Cobject_loaderDlg::OnQueryDragIcon() {
	return (HCURSOR) m_hIcon;
}


void Cobject_loaderDlg::OnCheck() {
  UpdateData(TRUE);
  g_main_app->update_options_from_gui();
}


void Cobject_loaderDlg::OnTransparencyCheck() {  
  CButton* b = (CButton*)(GetDlgItem(IDC_CHECK_TRANSPARENCY));
  int curstate = b->GetCheck();
  int newstate = (curstate == BST_UNCHECKED)?BST_CHECKED:BST_UNCHECKED;
  b->SetCheck(newstate);
  m_transparency_check = newstate;
  g_main_app->update_options_from_gui();
}


void Cobject_loaderDlg::OnMaterialCheck() {
  CButton* b = (CButton*)(GetDlgItem(IDC_CHECK_MATERIAL));
  int curstate = b->GetCheck();
  int newstate = (curstate == BST_UNCHECKED)?BST_CHECKED:BST_UNCHECKED;
  b->SetCheck(newstate);
  m_material_check = newstate;
  g_main_app->update_options_from_gui();
}


void Cobject_loaderDlg::OnColorsCheck() {
  CButton* b = (CButton*)(GetDlgItem(IDC_CHECK_USECOLORS));
  int curstate = b->GetCheck();
  int newstate = (curstate == BST_UNCHECKED)?BST_CHECKED:BST_UNCHECKED;
  b->SetCheck(newstate);
  m_usecolors_check = newstate;
  g_main_app->update_options_from_gui();
}


void Cobject_loaderDlg::OnClose() {
	
  g_main_app->uninitialize();

  FreeConsole();
  exit(0);

	CDialog::OnClose();
}


void Cobject_loaderDlg::OnLoadModelButton() {

  char filename[_MAX_PATH];

  int result = FileBrowse(filename, _MAX_PATH, 0, 0,
    "model files (*.obj, *.3ds)|*.obj;*.3ds|All Files (*.*)|*.*||",
    "Choose a model...");

  if (result < 0) {
    _cprintf("File browse canceled...\n");
    return;
  }

  g_main_app->LoadModel(filename);
}

void Cobject_loaderDlg::OnLoadTextureButton() {	

    char filename[_MAX_PATH];

  int result = FileBrowse(filename, _MAX_PATH, 0, 0,
    "image files (*.bmp, *.tga, *.jpg, *.jpeg, *.gif)|*.bmp;*.tga;*.jpg;*.jpeg;*.gif|All Files (*.*)|*.*||",
    "Choose an image file...");

  if (result < 0) {
    _cprintf("File browse canceled...\n");
    return;
  }

  g_main_app->LoadTexture(filename);

}


void Cobject_loaderDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	
  UpdateData(TRUE);
    
  if ((void*)(pScrollBar) == (void*)(&m_camera_zoom_slider)) {

    int position = m_camera_zoom_slider.GetPos();
    g_main_app->zoom(position);
    
  }
  
  // Update our local records of haptic properties and send them off
  // to the main application...
  m_stiffness = (float)(m_stiffness_slider.GetPos());
  m_static_friction = ((float)m_static_friction_slider.GetPos()) / FRICTION_SLIDER_SCALE;
  m_dynamic_friction = ((float)m_dynamic_friction_slider.GetPos()) / FRICTION_SLIDER_SCALE;
  m_focus = ((float)m_stereo_focus_slider.GetPos()) / FOCUS_SLIDER_SCALE;
  m_separation = ((float)m_stereo_separation_slider.GetPos()) / SEPARATION_SLIDER_SCALE;
   
  update_slider_text();

  g_main_app->update_options_from_gui();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void Cobject_loaderDlg::OnMouseMove(UINT nFlags, CPoint point) {

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
    // Look for the flag that says we _just_ started scrolling
    if (m_left_scrolling_gl_area > 0)
      g_main_app->scroll(delta,MOUSE_BUTTON_LEFT);
    last_left_scroll_point = point;
    m_left_scrolling_gl_area = 1;
  }

  if (m_right_scrolling_gl_area) {
    CPoint delta = point - last_right_scroll_point;
    // Look for the flag that says we _just_ started scrolling
    if (m_right_scrolling_gl_area > 0)
      g_main_app->scroll(delta,MOUSE_BUTTON_RIGHT);
    last_right_scroll_point = point;
    m_right_scrolling_gl_area = 1;
  }

  if (m_middle_scrolling_gl_area) {
    CPoint delta = point - last_middle_scroll_point;
    // Look for the flag that says we _just_ started scrolling
    if (m_middle_scrolling_gl_area > 0)
      g_main_app->scroll(delta,MOUSE_BUTTON_MIDDLE);
    last_middle_scroll_point = point;
    m_middle_scrolling_gl_area = 1;
  }

}


void Cobject_loaderDlg::OnMButtonDown(UINT nFlags, CPoint point) {

  CDialog::OnMButtonDown(nFlags,point);

  ::SetCapture(m_hWnd);

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA); // m_gl_wnd
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);

  if (PtInRect(&r,point)) {

    // convert to viewport coordinates
    point.x -= r.left;
    point.y -= r.top; 

    m_middle_scrolling_gl_area = -1;
    last_middle_scroll_point = point;

    // No need to select for the middle button...
    // g_main_app->select(point);
  }	

}


void Cobject_loaderDlg::OnMButtonUp(UINT nFlags, CPoint point) {

  ::ReleaseCapture();

  CDialog::OnMButtonUp(nFlags, point);  
  m_middle_scrolling_gl_area = 0;

}


void Cobject_loaderDlg::OnLButtonDown(UINT nFlags, CPoint point) {

  CDialog::OnLButtonDown(nFlags, point);

  ::SetCapture(m_hWnd);

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA); // m_gl_wnd
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);

  if (PtInRect(&r,point)) {

    // convert to viewport coordinates
    point.x -= r.left;
    point.y -= r.top; 
  
    m_left_scrolling_gl_area = -1;
    last_left_scroll_point = point;
    g_main_app->select(point);
  }	
  
}


void Cobject_loaderDlg::OnLButtonUp(UINT nFlags, CPoint point) {
	
  ::ReleaseCapture();

  CDialog::OnLButtonUp(nFlags, point);  
  m_left_scrolling_gl_area = 0;

}


void Cobject_loaderDlg::OnRButtonDown(UINT nFlags, CPoint point) {

  CDialog::OnRButtonDown(nFlags, point);

  ::SetCapture(m_hWnd);

  CWnd* pWnd = GetDlgItem(IDC_GL_AREA); // m_gl_wnd
  RECT r;
  pWnd->GetWindowRect(&r);
  ScreenToClient(&r);

  if (PtInRect(&r,point)) {

    // convert to viewport coordinates
    point.x -= r.left;
    point.y -= r.top; 

    m_right_scrolling_gl_area = -1;
    last_right_scroll_point = point;
    g_main_app->select(point);
  }

	
  
}


void Cobject_loaderDlg::OnRButtonUp(UINT nFlags, CPoint point) {
	
  ::ReleaseCapture();

  CDialog::OnRButtonUp(nFlags, point);
  m_right_scrolling_gl_area = 0;

}


void Cobject_loaderDlg::OnToggleHapticsButton() {
	
  g_main_app->toggle_haptics();

  CButton* button = (CButton*)(GetDlgItem(IDC_TOGGLEHAPTICS_BUTTON));
  button->SetWindowText(g_main_app->haptics_enabled?"Disable haptics":"Enable haptics");
	
}

void Cobject_loaderDlg::OnToggleStereoButton() {
	
  if (m_gl_wnd) {
    m_gl_wnd->DestroyWindow();
    delete m_gl_wnd;
  }
  
  RECT child;
  CWnd* pWnd = GetDlgItem(IDC_GL_AREA);  
  pWnd->GetClientRect(&child);
  
  m_gl_wnd = new CWnd();
  m_gl_wnd->Create(NULL,
						       NULL,
						       WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_VISIBLE,
						       child,
						       pWnd,
						       0);
  m_gl_area_hwnd = m_gl_wnd->m_hWnd;
  
  int currently_enabled = g_main_app->viewport->getStereoOn();
  g_main_app->reinitialize_viewport(1 - currently_enabled);
	
}


BOOL Cobject_loaderDlg::PreTranslateMessage(MSG* pMsg) {
	
  //_cprintf("message\n");

  // Handle mouse messages explicitly...
  if (pMsg->message == WM_LBUTTONDOWN ||
      pMsg->message == WM_RBUTTONDOWN ||
      pMsg->message == WM_MBUTTONDOWN ||
      pMsg->message == WM_LBUTTONUP   ||
      pMsg->message == WM_RBUTTONUP   ||
      pMsg->message == WM_MBUTTONUP   ||
      pMsg->message == WM_MOUSEMOVE) {

    //_cprintf("mouse msg\n");

    int x = LOWORD(pMsg->lParam);
    int y = HIWORD(pMsg->lParam);
    CPoint p(x,y);

    if (pMsg->hwnd == m_gl_area_hwnd) {

      if (pMsg->message == WM_LBUTTONDOWN)      OnLButtonDown(pMsg->wParam,p);
      else if (pMsg->message == WM_RBUTTONDOWN) OnRButtonDown(pMsg->wParam,p);
      else if (pMsg->message == WM_MBUTTONDOWN) OnMButtonDown(pMsg->wParam,p);
      else if (pMsg->message == WM_LBUTTONUP)   OnLButtonUp(pMsg->wParam,p);
      else if (pMsg->message == WM_RBUTTONUP)   OnRButtonUp(pMsg->wParam,p);
      else if (pMsg->message == WM_MBUTTONUP)   OnMButtonUp(pMsg->wParam,p);
      else if (pMsg->message == WM_MOUSEMOVE)   OnMouseMove(pMsg->wParam,p); 

      return true;
		}
    
  }

	return CDialog::PreTranslateMessage(pMsg);
}

void Cobject_loaderDlg::OnAnimationButton() {

  g_main_app->toggle_animation();

  CButton* button = (CButton*)(GetDlgItem(IDC_ANIMATION_BUTTON));
  button->SetWindowText(g_main_app->moving_object?"Stop Animation":"Animate Object");

}


void Cobject_loaderDlg::OnSizing(UINT fwSide, LPRECT pRect)
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


void Cobject_loaderDlg::OnSize(UINT nType, int cx, int cy)
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

  m_gl_wnd->MoveWindow(r.left,r.top,r.right - r.left, r.bottom - r.top, false);

  g_main_app->viewport->update(true);

  // Repaint
  Invalidate();
  UpdateWindow();
  RedrawWindow(0,0,RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

void Cobject_loaderDlg::OnLoadShaderButton()
{
  char filename[_MAX_PATH];

  int result = FileBrowse(filename, _MAX_PATH, 0, 0,
    "shader files (*.vert, *.frag)|*.vert;*.frag|All Files (*.*)|*.*||",
    "Choose a GLSL shader file...");

  if (result < 0) {
    _cprintf("File browse canceled...\n");
    return;
  }

  g_main_app->LoadShader(filename);
}
