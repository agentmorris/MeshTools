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

#include "stdafx.h"
#include "voxelizer.h"
#include "voxelizerDlg.h"
#include "voxelizer_globals.h"

#include <conio.h>
#include ".\voxelizerdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CRect initial_dlg_rect;
CRect initial_dlg_window_rect;
CRect initial_gl_area_rect;
CRect initial_gl_wnd_rect;

CvoxelizerDlg::CvoxelizerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CvoxelizerDlg::IDD, pParent), m_rendermesh_check(true)
	, m_distance_check(FALSE)
{
	//{{AFX_DATA_INIT(CvoxelizerDlg)
	m_material_check = TRUE;
	m_showbox_check = FALSE;
	m_showframe_check = FALSE;
	m_shownormals_check = FALSE;
	m_usecolors_check = FALSE;
	m_usetexture_check = TRUE;
	m_usewireframe_check = FALSE;
	m_renderpoints_check = FALSE;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

void CvoxelizerDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CvoxelizerDlg)
	DDX_Control(pDX, IDC_CAMZOOM_SLIDER, m_camera_zoom_slider);
	DDX_Check(pDX, IDC_CHECK_MATERIAL, m_material_check);
	DDX_Check(pDX, IDC_CHECK_SHOWBOX, m_showbox_check);
	DDX_Check(pDX, IDC_CHECK_SHOWFRAME, m_showframe_check);
	DDX_Check(pDX, IDC_CHECK_SHOWNORMALS, m_shownormals_check);
	DDX_Check(pDX, IDC_CHECK_USECOLORS, m_usecolors_check);
	DDX_Check(pDX, IDC_CHECK_USETEXTURE, m_usetexture_check);
	DDX_Check(pDX, IDC_CHECK_WIREFRAME, m_usewireframe_check);
	DDX_Check(pDX, IDC_RENDERPOINTS_CHECK, m_renderpoints_check);
	DDX_Check(pDX, IDC_CHECK_RENDER_MESH, m_rendermesh_check);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_VOXELRES_EDIT, m_voxelres_edit);
	DDX_Control(pDX, IDC_NORMALMULT_EDIT, m_normalmult_edit);
	DDX_Check(pDX, IDC_DISTANCEFIELD_CHECK, m_distance_check);
}

BEGIN_MESSAGE_MAP(CvoxelizerDlg, CDialog)
	//{{AFX_MSG_MAP(CvoxelizerDlg)
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_MATERIAL, OnCheck)
	ON_BN_CLICKED(IDC_LOAD_MODEL_BUTTON, OnLoadModelButton)
	ON_BN_CLICKED(IDC_LOAD_TEXTURE_BUTTON, OnLoadTextureButton)
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_BN_CLICKED(IDC_TOGGLEHAPTICS_BUTTON, OnToggleHapticsButton)
	ON_BN_CLICKED(IDC_VOXELIZE_BUTTON, OnVoxelizeButton)
	ON_BN_CLICKED(IDC_QUITVOXELIZING_BUTTON, OnQuitvoxelizingButton)
	ON_BN_CLICKED(IDC_LOADSUBTRACTOR_BUTTON, OnLoadSubtractorButton)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_SHOWBOX, OnCheck)
	ON_BN_CLICKED(IDC_CHECK_SHOWFRAME, OnCheck)
	ON_BN_CLICKED(IDC_CHECK_SHOWNORMALS, OnCheck)
	ON_BN_CLICKED(IDC_CHECK_USECOLORS, OnCheck)
	ON_BN_CLICKED(IDC_CHECK_USETEXTURE, OnCheck)
	ON_BN_CLICKED(IDC_CHECK_WIREFRAME, OnCheck)
	ON_BN_CLICKED(IDC_CHECK_RENDER_MESH, OnCheck)
	ON_BN_CLICKED(IDC_LOAD_MODIFIER_BUTTON, OnLoadModifierButton)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TETRAHEDRALIZE_BUTTON, OnTetrahedralizeButton)
	ON_BN_CLICKED(IDC_TEST_VOXELIZE_BUTTON, OnTestVoxelizeButton)
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_DISTANCEFIELD_CHECK, OnDistanceCheck)
END_MESSAGE_MAP()

BOOL CvoxelizerDlg::OnInitDialog() {

	m_gl_wnd = 0;
	m_left_scrolling_gl_area = 0;
	m_right_scrolling_gl_area = 0;
	m_middle_scrolling_gl_area = 0;

	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_gl_wnd = GetDlgItem(IDC_GL_AREA);
	m_gl_area_hwnd = m_gl_wnd->m_hWnd;

	CWnd* pWnd = GetDlgItem(IDC_GL_AREA);
	RECT child;
	pWnd->GetClientRect(&child);

	RECT r;
	pWnd->GetWindowRect(&r);
	ScreenToClient(&r);
	initial_gl_area_rect = r;

	m_gl_wnd->GetWindowRect(&r);
	pWnd->ScreenToClient(&r);
	initial_gl_wnd_rect = r;

	// Map from 0 to 100 chai units
	m_camera_zoom_slider.SetRange(0, 180, 1);
	m_camera_zoom_slider.SetPos(45);

	m_voxelres_edit.Init(g_main_app->m_long_axis_resolution, 0, 0);
	m_normalmult_edit.Init(DEFAULT_NORMAL_MULTIPLIER, 2, true);

	return TRUE;
}


HCURSOR CvoxelizerDlg::OnQueryDragIcon() {
	return (HCURSOR)m_hIcon;
}


void CvoxelizerDlg::OnClose() {

	g_main_app->uninitialize();

	FreeConsole();
	exit(0);

	CDialog::OnClose();
}


void CvoxelizerDlg::OnCheck() {
	UpdateData(TRUE);
	g_main_app->update_options_from_gui();
}

void CvoxelizerDlg::OnLoadTextureButton() {

	char filename[_MAX_PATH];

	int result = FileBrowse(filename, _MAX_PATH, 0, 0,
		"image files (*.bmp, *.tga)|*.bmp;*.tga|All Files (*.*)|*.*||",
		"Choose an image file...");

	if (result < 0) {
		_cprintf("File browse canceled...\n");
		return;
	}

	g_main_app->LoadTexture(filename);

}


void CvoxelizerDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {

	if ((void*)(pScrollBar) == (void*)(&m_camera_zoom_slider)) {

		UpdateData(TRUE);
		int position = m_camera_zoom_slider.GetPos();
		g_main_app->zoom(position);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CvoxelizerDlg::OnMouseMove(UINT nFlags, CPoint point) {

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
		g_main_app->scroll(delta, MOUSE_BUTTON_LEFT);
		last_left_scroll_point = point;
	}

	if (m_right_scrolling_gl_area) {
		CPoint delta = point - last_right_scroll_point;
		g_main_app->scroll(delta, MOUSE_BUTTON_RIGHT);
		last_right_scroll_point = point;
	}

	if (m_middle_scrolling_gl_area) {
		CPoint delta = point - last_middle_scroll_point;
		g_main_app->scroll(delta, MOUSE_BUTTON_MIDDLE);
		last_middle_scroll_point = point;
	}

}


void CvoxelizerDlg::OnMButtonDown(UINT nFlags, CPoint point) {

	CDialog::OnMButtonDown(nFlags, point);

	::SetCapture(m_hWnd);

	CWnd* pWnd = m_gl_wnd; // GetDlgItem(IDC_GL_AREA);
	RECT r;
	pWnd->GetWindowRect(&r);
	ScreenToClient(&r);

	if (PtInRect(&r, point)) {

		// convert to viewport coordinates
		point.x -= r.left;
		point.y -= r.top;

		m_middle_scrolling_gl_area = 1;
		last_middle_scroll_point = point;

		// No need to select for the middle button...
		// g_main_app->select(point);
	}

}


void CvoxelizerDlg::OnMButtonUp(UINT nFlags, CPoint point) {

	::ReleaseCapture();

	CDialog::OnMButtonUp(nFlags, point);
	m_middle_scrolling_gl_area = 0;

}


void CvoxelizerDlg::OnLButtonDown(UINT nFlags, CPoint point) {

	CDialog::OnLButtonDown(nFlags, point);

	::SetCapture(m_hWnd);

	CWnd* pWnd = m_gl_wnd; // GetDlgItem(IDC_GL_AREA);
	RECT r;
	pWnd->GetWindowRect(&r);
	ScreenToClient(&r);

	if (PtInRect(&r, point)) {

		// convert to viewport coordinates
		point.x -= r.left;
		point.y -= r.top;

		m_left_scrolling_gl_area = 1;
		last_left_scroll_point = point;
		g_main_app->select(point);
	}

}

void CvoxelizerDlg::OnLButtonUp(UINT nFlags, CPoint point) {

	::ReleaseCapture();

	CDialog::OnLButtonUp(nFlags, point);
	m_left_scrolling_gl_area = 0;

}



void CvoxelizerDlg::OnRButtonDown(UINT nFlags, CPoint point) {

	CDialog::OnRButtonDown(nFlags, point);

	::SetCapture(m_hWnd);

	CWnd* pWnd = m_gl_wnd; // GetDlgItem(IDC_GL_AREA);
	RECT r;
	pWnd->GetWindowRect(&r);
	ScreenToClient(&r);

	if (PtInRect(&r, point)) {

		// convert to viewport coordinates
		point.x -= r.left;
		point.y -= r.top;

		m_right_scrolling_gl_area = 1;
		last_right_scroll_point = point;
		g_main_app->select(point);
	}



}


void CvoxelizerDlg::OnRButtonUp(UINT nFlags, CPoint point) {

	::ReleaseCapture();

	CDialog::OnRButtonUp(nFlags, point);
	m_right_scrolling_gl_area = 0;

}


void CvoxelizerDlg::OnToggleHapticsButton() {

	g_main_app->toggle_haptics();

	CButton* button = (CButton*)(GetDlgItem(IDC_TOGGLEHAPTICS_BUTTON));
	button->SetWindowText(g_main_app->haptics_enabled ? "Disable haptics" : "Enable haptics");

}


BOOL CvoxelizerDlg::PreTranslateMessage(MSG* pMsg) {

	//_cprintf("message\n");

	// Handle mouse messages explicitly...
	if (pMsg->message == WM_LBUTTONDOWN ||
		pMsg->message == WM_RBUTTONDOWN ||
		pMsg->message == WM_MBUTTONDOWN ||
		pMsg->message == WM_LBUTTONUP ||
		pMsg->message == WM_RBUTTONUP ||
		pMsg->message == WM_MBUTTONUP ||
		pMsg->message == WM_MOUSEMOVE) {

		//_cprintf("mouse msg\n");

		int x = LOWORD(pMsg->lParam);
		int y = HIWORD(pMsg->lParam);
		CPoint p(x, y);

		if (pMsg->hwnd == m_gl_area_hwnd) {

			if (pMsg->message == WM_LBUTTONDOWN)      OnLButtonDown(pMsg->wParam, p);
			else if (pMsg->message == WM_RBUTTONDOWN) OnRButtonDown(pMsg->wParam, p);
			else if (pMsg->message == WM_MBUTTONDOWN) OnMButtonDown(pMsg->wParam, p);
			else if (pMsg->message == WM_LBUTTONUP)   OnLButtonUp(pMsg->wParam, p);
			else if (pMsg->message == WM_RBUTTONUP)   OnRButtonUp(pMsg->wParam, p);
			else if (pMsg->message == WM_MBUTTONUP)   OnMButtonUp(pMsg->wParam, p);
			else if (pMsg->message == WM_MOUSEMOVE)   OnMouseMove(pMsg->wParam, p);

			return true;
		}

	}

	return CDialog::PreTranslateMessage(pMsg);
}


DWORD WINAPI voxelizer_thread(void* param) {

	int operation = (int)param;

	if (g_main_app->multithreaded_voxelizer == 0)
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	g_main_app->quit_voxelizing = 0;
	g_main_app->voxelize_current_object(operation);

	return 0;
}


void CvoxelizerDlg::OnQuitvoxelizingButton() {

	g_main_app->quit_voxelizing = 1;

}


void CvoxelizerDlg::OnLoadSubtractorButton() {

	char filename[_MAX_PATH];

	int result = FileBrowse(filename, _MAX_PATH, 0, 0,
		"model files (*.obj, *.3ds, *.ply, *.node, *.face, *.ele, *.smesh, *.msh)|*.obj;*.3ds;*.ply;*.node;*.face;*.ele;*.smesh;*.msh|All Files (*.*)|*.*||",
		"Choose a model to subtract...");

	if (result < 0) {
		_cprintf("File browse canceled...\n");
		return;
	}

	g_main_app->LoadObjectToSubtract(filename);

}


void CvoxelizerDlg::OnLoadModelButton() {

	char filename[_MAX_PATH];

	int result = FileBrowse(filename, _MAX_PATH, 0, 0,
		"model files (*.obj, *.3ds, *.ply, *.node, *.face, *.ele, *.smesh, *.msh)|*.obj;*.3ds;*.ply;*.node;*.face;*.ele;*.smesh;*.msh|All Files (*.*)|*.*||",
		"Choose a model to fill...");

	if (result < 0) {
		_cprintf("File browse canceled...\n");
		return;
	}

	g_main_app->LoadObjectToVoxelize(filename);
}


void CvoxelizerDlg::OnLoadModifierButton() {

	char filename[_MAX_PATH];

	int result = FileBrowse(filename, _MAX_PATH, 0, 0,
		"model files (*.obj, *.3ds, *.ply, *.node, *.face, *.ele, *.smesh, *.msh)|*.obj;*.3ds;*.ply;*.node;*.face;*.ele;*.smesh;*.msh|All Files (*.*)|*.*||",
		"Choose a model as a modifier...");

	if (result < 0) {
		_cprintf("File browse canceled...\n");
		return;
	}

	g_main_app->LoadModifierObject(filename);

}


void CvoxelizerDlg::OnVoxelizeButton() {

	UpdateData(TRUE);
	g_main_app->m_long_axis_resolution = m_voxelres_edit.Get_Number();
	g_main_app->m_normal_multiplier = m_normalmult_edit.Get_Number();
	g_main_app->render_point_cloud = m_renderpoints_check;

	short keystate = ::GetAsyncKeyState(VK_CONTROL);

	if (keystate & (1 << 15)) {
		_cprintf("Launching threaded voxelizer\n");
		g_main_app->multithreaded_voxelizer = 1;
		DWORD tid;
		::CreateThread(0, 0, voxelizer_thread, (void*)OPERATION_VOXELIZE, 0, &tid);
	}
	else {
		g_main_app->multithreaded_voxelizer = 0;
		voxelizer_thread((void*)OPERATION_VOXELIZE);
	}

}


void CvoxelizerDlg::OnTetrahedralizeButton() {

	UpdateData(TRUE);
	g_main_app->m_long_axis_resolution = m_voxelres_edit.Get_Number();
	g_main_app->m_normal_multiplier = m_normalmult_edit.Get_Number();
	g_main_app->render_point_cloud = m_renderpoints_check;

	short keystate = ::GetAsyncKeyState(VK_CONTROL);

	if (keystate & (1 << 15)) {
		_cprintf("Launching threaded voxelizer\n");
		g_main_app->multithreaded_voxelizer = 1;
		DWORD tid;
		::CreateThread(0, 0, voxelizer_thread, (void*)OPERATION_TETRAHEDRALIZE, 0, &tid);
	}
	else {
		g_main_app->multithreaded_voxelizer = 0;
		voxelizer_thread((void*)OPERATION_TETRAHEDRALIZE);
	}
}

void CvoxelizerDlg::OnTestVoxelizeButton() {

	UpdateData(TRUE);

	m_renderpoints_check = true;
	UpdateData(FALSE);

	g_main_app->m_long_axis_resolution = m_voxelres_edit.Get_Number();
	g_main_app->m_normal_multiplier = m_normalmult_edit.Get_Number();
	g_main_app->render_point_cloud = m_renderpoints_check;

	_cprintf("Launching threaded voxelizer\n");
	g_main_app->multithreaded_voxelizer = 1;
	DWORD tid;
	::CreateThread(0, 0, voxelizer_thread, (void*)OPERATION_TESTONLY, 0, &tid);
}

void CvoxelizerDlg::OnSize(UINT nType, int cx, int cy) {

	CDialog::OnSize(nType, cx, cy);

	RECT dlgr;
	GetClientRect(&dlgr);

	// Resize GL area
	RECT r = initial_gl_area_rect;
	r.bottom += dlgr.bottom - initial_dlg_rect.bottom;
	r.right += dlgr.right - initial_dlg_rect.right;

	CWnd* c = GetDlgItem(IDC_GL_AREA);
	if (!c) return;
	c->MoveWindow(r.left, r.top, r.right - r.left, r.bottom - r.top, false);

	r = initial_gl_wnd_rect;
	r.bottom += dlgr.bottom - initial_dlg_rect.bottom;
	r.right += dlgr.right - initial_dlg_rect.right;

	m_gl_wnd->MoveWindow(r.left, r.top, r.right - r.left, r.bottom - r.top, false);

	g_main_app->viewport->update(true);

	// Repaint
	Invalidate();
	UpdateWindow();
	RedrawWindow(0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

void CvoxelizerDlg::OnSizing(UINT fwSide, LPRECT pRect) {
}


void CvoxelizerDlg::OnDistanceCheck() {
	UpdateData(true);
	g_main_app->m_compute_distance_field = m_distance_check;
}
