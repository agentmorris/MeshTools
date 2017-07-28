#include "afxwin.h"
#include "winmeshviewDlg.h"
#include "resource.h"

#if !defined(AFX_teschner_deformableDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
#define AFX_teschner_deformableDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

class Cteschner_deformableDlg : public CwinmeshviewDlg {

public:
  Cteschner_deformableDlg(CWnd* pParent = NULL);
  Cteschner_deformableDlg(int idd, CWnd* pParent = NULL);
  void initialize();
  	
// Dialog Data
  //{{AFX_DATA(CwinmeshviewDlg)
  enum { IDD = IDD_teschner_deformable_DIALOG };
  BOOL m_threadsim_check;
  //}}AFX_DATA

// ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(Cteschner_deformableDlg)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
  virtual afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

  //}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()
public:      
  afx_msg void OnThreadSimCheck();
  afx_msg void OnInitializeMeshButton();
  afx_msg void OnSingleStepButton();
  afx_msg void OnStartSimButton();  
  afx_msg void OnLoadProblemFileButton();
  CString m_simtime_text;
  afx_msg void OnBnClickedExportDeformedModel();
  afx_msg void OnBnClickedAutoExportDeformedModel();
  afx_msg void OnBnClickedResetMesh();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_teschner_deformableDLG_H__DD14893C_4DE3_469B_BE2A_FB588BF46B3B__INCLUDED_)
