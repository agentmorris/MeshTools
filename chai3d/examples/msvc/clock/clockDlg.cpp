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
    \date       06/2004
*/
//===========================================================================

// clockDlg.cpp : implementation file
//

#include "stdafx.h"
#include "clock.h"
#include "clockDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClockDlg dialog

CClockDlg::CClockDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClockDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClockDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClockDlg::OnClose()
{

    exit(0);

	CDialog::OnClose();

}

void CClockDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClockDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClockDlg, CDialog)
	//{{AFX_MSG_MAP(CClockDlg)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, OnStart)
	ON_BN_CLICKED(IDCANCEL, OnStop)
	ON_BN_CLICKED(IDC_BUTTON2, OnInit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClockDlg message handlers

// The number of microseconds after which a timeout
// message will appear in the dialog window
#define DEFAULT_TIMEOUT_PERIOD 5000000

BOOL CClockDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	time = new cPrecisionClock();
	time->setTimeoutPeriod(DEFAULT_TIMEOUT_PERIOD);

	char buff[50];
	CEdit* edit = (CEdit*)(GetDlgItem(IDC_EDIT1));
	sprintf(buff, "%d", time->getCurrentTime());
    edit->SetWindowText(buff);

    edit = (CEdit*)(GetDlgItem(IDC_EDIT2));
	sprintf(buff, "%d", time->getTimeoutPeriod());
    edit->SetWindowText(buff);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClockDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClockDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


DWORD clock_loop(void* param) 
{
    CClockDlg* app = (CClockDlg*)(param);

    while(app->time->on()) {
		char buff[50];
	    CEdit* edit = (CEdit*)(app->GetDlgItem(IDC_EDIT1));
		sprintf(buff, "%ld", (app->time->getCurrentTime()));
		edit->SetWindowText(buff);    

		edit = (CEdit*)(app->GetDlgItem(IDC_EDIT3));
		if (app->time->timeoutOccurred())
			edit->SetWindowText("Timeout Occurred!");
		else
			edit->SetWindowText("");
	}
	return 0;
}


void CClockDlg::OnStart() 
{	
    time->start();	

    DWORD thread_id;
    ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(clock_loop), this, 0, &thread_id);

    // Boost thread and process priority
    ::SetThreadPriority(&thread_id, THREAD_PRIORITY_ABOVE_NORMAL);
}

void CClockDlg::OnStop() 
{
	time->stop();	
}

void CClockDlg::OnInit() 
{	
	time->initialize();

	char buff[50];
	CEdit* edit = (CEdit*)(GetDlgItem(IDC_EDIT1));
	sprintf(buff, "%d", time->getCurrentTime());
    edit->SetWindowText(buff);

	edit = (CEdit*)(GetDlgItem(IDC_EDIT2));
	sprintf(buff, "%d", time->getTimeoutPeriod());
    edit->SetWindowText(buff);	
}
