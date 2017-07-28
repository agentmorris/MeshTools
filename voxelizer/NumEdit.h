//--------------------------------------------------------------------------------------/
//	Header File

//	Class:			CNumEdit			
//	Base Class:		CEdit

//	File:			NumEdit.h		
//	Revision:		A						
//	Date:			17 August 2000
//  Designed:		Ian J Hart	
//--------------------------------------------------------------------------------------/

#ifndef __NUMEDIT_H__
#define __NUMEDIT_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//--------------------------------------------------------------------------------------/

class CNumEdit : public CEdit
{
	// 1 - Construction
public:

	CNumEdit();

	// 2 - Destruction
public:

	virtual ~CNumEdit();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNumEdit)
	//}}AFX_VIRTUAL

// 3 - Message Map Functions 
protected:

	//{{AFX_MSG(CNumEdit)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetfocus();
	afx_msg void OnKillFocus();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	//	4 - Class Member Variables 
private:
	CToolTipCtrl* m_pToolTip;

protected:

	double	m_fNumber;
	int		m_dPrecision;
	bool	m_bNegValues;
	CString m_strText;

	//	5 - Class Member Functions 
private:

	void Update();

protected:

	void	Get_EditCtrl_Text();
	void	Set_EditCtrl_Text(CString strText);
	bool	Validate_Char(UINT nChar);
	double	Text_To_Number(CString strText);
	CString Number_To_Text(double fNumber);

	//	6 - Class Creation/Initialisation

public:

	void Init(double	fNumber = 0.00,
		int		dPrecision = 2,
		bool		bNegValues = true);

	void Set_Data(double	fNumber,
		int		dPrecision,
		bool		bNegValues);


	// 7 - Interface 

public:


	CString Get_Number_Text();

	double	Get_Number();
	void	Set_Number(double fNumber);

};
#endif 
//--------------------------------------------------------------------------------------/
