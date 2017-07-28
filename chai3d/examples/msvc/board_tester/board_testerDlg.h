// 06_board_testerDlg.h : header file
//
#include "CDriverServotogo.h"
#include "CDriverSensoray626.h"
#include "CGenericDevice.h"

#if !defined(AFX_06_BOARD_TESTERDLG_H__F84466E5_E844_4762_B238_728A44412D45__INCLUDED_)
#define AFX_06_BOARD_TESTERDLG_H__F84466E5_E844_4762_B238_728A44412D45__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMy06_board_testerDlg dialog

class CMy06_board_testerDlg : public CDialog
{
// Construction
public:
	CMy06_board_testerDlg(CWnd* pParent = NULL);	// standard constructor
	
	cDriverServotogo * servotogoBoard;
    cDriverSensoray626 * sensorayBoard;
    cGenericDevice * board;

// Dialog Data
	//{{AFX_DATA(CMy06_board_testerDlg)
	enum { IDD = IDD_MY06_BOARD_TESTER_DIALOG };
	CButton	m_Button12;
	CButton	m_Button11;
	CSliderCtrl	m_sldControl;
	CButton	m_ButtonEnc5;
	CButton	m_ButtonEnc4;
	CButton	m_ButtonEnc3;
	CButton	m_ButtonEnc2;
	CButton	m_ButtonEnc1;
	CButton	m_ButtonEnc0;
	CButton	m_ButtonEnc7;
	CButton	m_ButtonEnc6;
	CButton	m_ButtonSensoray;
	CButton	m_ButtonServotogo;
	CString	m_edtEnc0;
	CString	m_edtEnc1;
	CString	m_edtEnc2;
	CString	m_edtEnc3;
	CString	m_edtEnc4;
	CString	m_edtEnc5;
	CString	m_edtEnc6;
	CString	m_edtEnc7;
	CString	m_boardStatus;
	int		m_sldValue;
	CString	m_edtSlider;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMy06_board_testerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMy06_board_testerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEnc0();
	afx_msg void OnStartServotogo();
	afx_msg void OnStartSensoray626();
	afx_msg void OnEnc1();
	afx_msg void OnEnc2();
	afx_msg void OnEnc3();
	afx_msg void OnEnc4();
	afx_msg void OnEnc5();
	afx_msg void OnEnc6();
	afx_msg void OnEnc7();
	afx_msg void OnZeroDACS();
	afx_msg void OnSliderDACs();
	afx_msg void OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_06_BOARD_TESTERDLG_H__F84466E5_E844_4762_B238_728A44412D45__INCLUDED_)
