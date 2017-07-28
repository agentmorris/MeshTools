// 06_board_tester.h : main header file for the 06_BOARD_TESTER application
//

#if !defined(AFX_06_BOARD_TESTER_H__EA460884_D8FD_4554_B57F_9E27F67B7C03__INCLUDED_)
#define AFX_06_BOARD_TESTER_H__EA460884_D8FD_4554_B57F_9E27F67B7C03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMy06_board_testerApp:
// See 06_board_tester.cpp for the implementation of this class
//

class CMy06_board_testerApp : public CWinApp
{
public:
	CMy06_board_testerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy06_board_testerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMy06_board_testerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_06_BOARD_TESTER_H__EA460884_D8FD_4554_B57F_9E27F67B7C03__INCLUDED_)
