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

#ifndef _winmeshview_GLOBALS_H_
#define _winmeshview_GLOBALS_H_

#define MESSAGE_UPDATE WM_USER + 0

class CwinmeshviewApp;

class winmeshview_dialog_type {
public:

	int m_left_scrolling_gl_area;
	int m_middle_scrolling_gl_area;
	int m_right_scrolling_gl_area;

	// The last point scrolled through by each mouse button,
	// in _viewport_ coordinates (i.e. (0,0) is the top-left
	// corner of the viewport area)
	CPoint last_left_scroll_point;
	CPoint last_right_scroll_point;
	CPoint last_middle_scroll_point;

	CPoint last_left_click_point;
	CPoint last_right_click_point;
	CPoint last_middle_click_point;

	double last_left_click_time;
	double last_right_click_time;
	double last_middle_click_time;

	HWND m_gl_area_hwnd;
	virtual void update_rendering_options_from_gui(CwinmeshviewApp* app,
		int preserve_loaded_properties = 0) = 0;
	virtual void copy_rendering_options_to_gui(CwinmeshviewApp* app) = 0;
};

// This makes sure that only the one file who defines
// ALLOCATE_SCOPED_GLOBALS actually creates space for
// all the global variables in the binary; everyone else
// externs them...
#ifdef ALLOCATE_SCOPED_GLOBALS
#define SCOPE
#else 
#define SCOPE extern
#endif

SCOPE winmeshview_dialog_type* g_main_dlg;
SCOPE CwinmeshviewApp* g_main_app;
#endif

