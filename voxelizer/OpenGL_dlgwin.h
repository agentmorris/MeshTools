#if !defined(AFX_OPENGL_H__34EC9E69_B426_4DC4_8F61_7DEEC99D79D1__INCLUDED_)
#define AFX_OPENGL_H__34EC9E69_B426_4DC4_8F61_7DEEC99D79D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COpenGL : public CWnd {

public:
	COpenGL();

	HDC hdc;
	HGLRC hglrc;

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(COpenGL)
		//}}AFX_VIRTUAL

	// Implementation
	virtual ~COpenGL();

	// Generated message map functions
protected:
	//{{AFX_MSG(COpenGL)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENGL_H__34EC9E69_B426_4DC4_8F61_7DEEC99D79D1__INCLUDED_)
