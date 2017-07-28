//--------------------------------------------------------------------------------------/
//	Implementation File

//	Class:			CNumEdit			
//	Base Class:		CEdit

//	File:			NumEdit.cpp		
//	Revision:		A						
//	Date:			17 August 2000
//  Designed:		Ian J Hart	
//--------------------------------------------------------------------------------------/

#include "stdafx.h"
#include "NumEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C_C/

CNumEdit::CNumEdit()
{
	m_fNumber = 0.00;
	m_dPrecision = 2;
	m_bNegValues = true;
	m_strText = Number_To_Text(m_fNumber);
	m_pToolTip = NULL;

}
//D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D_D/

CNumEdit::~CNumEdit()
{

}
//M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M_M/

BEGIN_MESSAGE_MAP(CNumEdit, CEdit)
	//{{AFX_MSG_MAP(CNumEdit)
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillFocus)
	ON_CONTROL_REFLECT(EN_SETFOCUS, OnSetfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//--------------------------------------------------------------------------------------/

void CNumEdit::Init(double	fNumber,
	int		dPrecision,
	bool	bNegValues)
{
	CEdit::OnKillFocus(AfxGetMainWnd());
	// 1 - Initialise the controls data

	Set_Data(fNumber, dPrecision, bNegValues);

	// 2 - Call SetFocus to force the control to update

	SetFocus();
}

//--------------------------------------------------------------------------------------/

void CNumEdit::Set_Data(double	fNumber,
	int		dPrecision,
	bool	bNegValues)
{
	m_fNumber = fNumber;
	m_dPrecision = dPrecision;
	m_bNegValues = bNegValues;
	m_strText = Number_To_Text(m_fNumber);
}
//--------------------------------------------------------------------------------------/

void CNumEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	// 1 - Check validity of character typed
	if (Validate_Char(nChar) != true)
	{
		//MessageBeep(0);
		return;
	}

	// 2 - Standard OnChar processeing 
	CEdit::OnChar(nChar, nRepCnt, nFlags);

	// DSM 
	Get_EditCtrl_Text();
}
//--------------------------------------------------------------------------------------/

void CNumEdit::OnSetfocus()
{
	Set_EditCtrl_Text(m_strText);
}
//--------------------------------------------------------------------------------------/

void CNumEdit::OnKillFocus()
{
	Get_EditCtrl_Text();
	Update();
	CEdit::OnKillFocus(AfxGetMainWnd());

}
//--------------------------------------------------------------------------------------/

void CNumEdit::Update()
{
	m_strText = Number_To_Text(m_fNumber);
	Set_EditCtrl_Text(m_strText);
}
//--------------------------------------------------------------------------------------/

bool CNumEdit::Validate_Char(UINT nChar)
{

	bool bValidChar = false;

	// 2 - Check if a valid character has been typed.
	//	   Valid characters '1','2','3'...'9' '-' '.' etc
	//	   Note refer WINUSER.H for virtual key definition eg VK_SPACE = 0x20

	if ((nChar < VK_SPACE) ||
		(nChar >= '0'   &&  nChar <= '9'))
	{
		bValidChar = true;
	}

	if (m_bNegValues == true && nChar == '-')
	{
		bValidChar = true;
	}

	if (m_dPrecision > 0 && nChar == '.')
	{
		bValidChar = true;
	}

	// 3 - If its not valid character return false	
	if (bValidChar != true)
	{
		return false;
	}

	CString   strText;
	TCHAR     chText;
	int		  nPosStart, nPosEnd;

	//  5 -	Get the current text in the edit control and 
	//		the current position of the cursor

	GetWindowText(strText);
	GetSel(nPosStart, nPosEnd);

	//  6 - Test the validity of character typed in context with
	//      existing edit control characters 

	for (int i = 0; i < strText.GetLength(); i++)
	{
		chText = strText[i];

		// 6.1 - Prevent more then one period being typed 
		if ((nChar == '.') && (chText == '.'))
		{
			return false;
		}
		// 6.2 - Prevent period being first character
		if ((nChar == '.') && (nPosStart == 0))
		{
			return false;
		}
		// 6.3 - Prevent more then one negative sign being typed
		if ((nChar == '-') && (chText == '-'))
		{
			return false;
		}
		// 6.4 - Ensure negative sign is the first character typed
		if ((nChar == '-') && (nPosStart != 0))
		{
			return false;
		}

		// 6.5 - Ensure precision enforced. 
		/*if(nChar!=VK_BACK)
		{
			if(	strText.Find('.')!=-1)
			{
				if(nPosStart > (strText.Find('.') + m_dPrecision))
				{
					return false;
				}

				if(nPosStart > strText.Find('.')  &&
				   nPosStart < strText.GetLength() &&
				   strText.GetLength() - strText.Find('.') -1 >= m_dPrecision)

				{
					return false;
				}

			}
		}*/
	}

	return true;
}
//--------------------------------------------------------------------------------------/

void   CNumEdit::Set_EditCtrl_Text(CString strText)
{

	SetWindowText(strText);

}
//--------------------------------------------------------------------------------------/

void CNumEdit::Get_EditCtrl_Text()
{
	// 1 - Get the edit control text and number value
	//     The number value is then converted back to text this process 
	//     performs additional validation  preventing
	//	 a) .
	//   b) 0.
	//   c).022
	//	 d) 666.
	//	 e) 0004444

	CString		strText;
	GetWindowText(strText);
	m_fNumber = Text_To_Number(strText);
	strText = Number_To_Text(m_fNumber);

	m_strText = strText;
}
//--------------------------------------------------------------------------------------/

double  CNumEdit::Text_To_Number(CString strText)
{
	return atof(strText);
}
//--------------------------------------------------------------------------------------/

CString CNumEdit::Number_To_Text(double fNumber)
{
	// 1 - Prepare the format string 
	//	   eg format string %.5f formats a double with the precision set 
	//	   to five (5)
	CString strFormat;

	// DSM
	  /*strFormat.Format("%d",m_dPrecision);
	  strFormat="%."+strFormat+"f";*/
	strFormat = "%g";

	// 2 - Format the number to text
	CString strText;
	strText.Format(strFormat, fNumber);

	return strText;
}
//--------------------------------------------------------------------------------------/

CString CNumEdit::Get_Number_Text()
{
	return m_strText;
}
//--------------------------------------------------------------------------------------/

void CNumEdit::Set_Number(double fNumber)
{
	m_fNumber = fNumber;
	Update();
}
//--------------------------------------------------------------------------------------/

double CNumEdit::Get_Number()
{
	return m_fNumber;
}
//--------------------------------------------------------------------------------------/
void CNumEdit::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	//1 - Override to prevent popup menu preventing pasting	
	return;
}
//--------------------------------------------------------------------------------------/
