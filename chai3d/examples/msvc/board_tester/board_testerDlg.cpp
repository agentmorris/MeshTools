// board_testerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "board_tester.h"
#include "board_testerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMy06_board_testerDlg dialog

CMy06_board_testerDlg::CMy06_board_testerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMy06_board_testerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMy06_board_testerDlg)
	m_edtEnc0 = _T("");
	m_edtEnc1 = _T("");
	m_edtEnc2 = _T("");
	m_edtEnc3 = _T("");
	m_edtEnc4 = _T("");
	m_edtEnc5 = _T("");
	m_edtEnc6 = _T("");
	m_edtEnc7 = _T("");
	m_boardStatus = _T("");
	m_sldValue = 0;
	m_edtSlider = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy06_board_testerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMy06_board_testerDlg)
	DDX_Control(pDX, IDC_BUTTON12, m_Button12);
	DDX_Control(pDX, IDC_BUTTON11, m_Button11);
	DDX_Control(pDX, IDC_SLIDER1, m_sldControl);
	DDX_Control(pDX, IDC_BUTTON8, m_ButtonEnc5);
	DDX_Control(pDX, IDC_BUTTON7, m_ButtonEnc4);
	DDX_Control(pDX, IDC_BUTTON6, m_ButtonEnc3);
	DDX_Control(pDX, IDC_BUTTON5, m_ButtonEnc2);
	DDX_Control(pDX, IDC_BUTTON4, m_ButtonEnc1);
	DDX_Control(pDX, IDC_BUTTON1, m_ButtonEnc0);
	DDX_Control(pDX, IDC_BUTTON10, m_ButtonEnc7);
	DDX_Control(pDX, IDC_BUTTON9, m_ButtonEnc6);
	DDX_Control(pDX, IDC_BUTTON3, m_ButtonSensoray);
	DDX_Control(pDX, IDC_BUTTON2, m_ButtonServotogo);
	DDX_Text(pDX, IDC_EDIT1, m_edtEnc0);
	DDX_Text(pDX, IDC_EDIT2, m_edtEnc1);
	DDX_Text(pDX, IDC_EDIT3, m_edtEnc2);
	DDX_Text(pDX, IDC_EDIT4, m_edtEnc3);
	DDX_Text(pDX, IDC_EDIT5, m_edtEnc4);
	DDX_Text(pDX, IDC_EDIT6, m_edtEnc5);
	DDX_Text(pDX, IDC_EDIT7, m_edtEnc6);
	DDX_Text(pDX, IDC_EDIT8, m_edtEnc7);
	DDX_Text(pDX, IDC_EDIT9, m_boardStatus);
	DDX_Slider(pDX, IDC_SLIDER1, m_sldValue);
	DDX_Text(pDX, IDC_EDIT10, m_edtSlider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMy06_board_testerDlg, CDialog)
	//{{AFX_MSG_MAP(CMy06_board_testerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnEnc0)
	ON_BN_CLICKED(IDC_BUTTON2, OnStartServotogo)
	ON_BN_CLICKED(IDC_BUTTON3, OnStartSensoray626)
	ON_BN_CLICKED(IDC_BUTTON4, OnEnc1)
	ON_BN_CLICKED(IDC_BUTTON5, OnEnc2)
	ON_BN_CLICKED(IDC_BUTTON6, OnEnc3)
	ON_BN_CLICKED(IDC_BUTTON7, OnEnc4)
	ON_BN_CLICKED(IDC_BUTTON8, OnEnc5)
	ON_BN_CLICKED(IDC_BUTTON9, OnEnc6)
	ON_BN_CLICKED(IDC_BUTTON10, OnEnc7)
	ON_BN_CLICKED(IDC_BUTTON12, OnZeroDACS)
	ON_BN_CLICKED(IDC_BUTTON11, OnSliderDACs)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnReleasedcaptureSlider1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMy06_board_testerDlg message handlers

BOOL CMy06_board_testerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_sldControl.SetRange(-10, 10, true);
	
	m_ButtonEnc0.EnableWindow(FALSE);
	m_ButtonEnc1.EnableWindow(FALSE);
	m_ButtonEnc2.EnableWindow(FALSE);
	m_ButtonEnc3.EnableWindow(FALSE);
	m_ButtonEnc4.EnableWindow(FALSE);
	m_ButtonEnc5.EnableWindow(FALSE);
	m_ButtonEnc6.EnableWindow(FALSE);
	m_ButtonEnc7.EnableWindow(FALSE);
	m_Button11.EnableWindow(FALSE);
	m_Button12.EnableWindow(FALSE);

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMy06_board_testerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMy06_board_testerDlg::OnPaint() 
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
HCURSOR CMy06_board_testerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMy06_board_testerDlg::OnEnc0() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_0, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc0 = buffer;
  UpdateData(FALSE);
	
}

void CMy06_board_testerDlg::OnStartServotogo() 
{
	//	start servotogo
	// create board        
	board = new cGenericDevice;
    servotogoBoard = new cDriverServotogo;
    board = servotogoBoard;

    // open board
    board->open();

    // check if board opened properly
    if (board->isSystemReady())
    {
		m_boardStatus = "Servotogo Board Ready";
		m_ButtonEnc0.EnableWindow(TRUE);
		m_ButtonEnc1.EnableWindow(TRUE);
		m_ButtonEnc2.EnableWindow(TRUE);
		m_ButtonEnc3.EnableWindow(TRUE);
		m_ButtonEnc4.EnableWindow(TRUE);
		m_ButtonEnc5.EnableWindow(TRUE);
		m_ButtonEnc6.EnableWindow(TRUE);
		m_ButtonEnc7.EnableWindow(TRUE);
		m_Button11.EnableWindow(TRUE);
		m_Button12.EnableWindow(TRUE);
		board->command(CHAI_CMD_RESET_ENCODER_0, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_1, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_2, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_3, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_4, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_5, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_6, NULL);
		board->command(CHAI_CMD_RESET_ENCODER_7, NULL);
	}
	else
	{
		m_boardStatus = "Servotogo Board NOT Ready";
	}

	m_ButtonSensoray.EnableWindow(FALSE);
	m_ButtonServotogo.EnableWindow(FALSE);

	UpdateData(FALSE);

}

void CMy06_board_testerDlg::OnStartSensoray626() 
{
        board = new cGenericDevice;
        sensorayBoard = new cDriverSensoray626;
        board = sensorayBoard;

        // open board
        board->open();

        // check if board opened properly
        if (board->isSystemReady())
        {
			m_boardStatus = "Sensoray626 Board Ready";
			m_ButtonEnc0.EnableWindow(TRUE);
			m_ButtonEnc1.EnableWindow(TRUE);
			m_ButtonEnc2.EnableWindow(TRUE);
			m_ButtonEnc3.EnableWindow(TRUE);
			m_ButtonEnc4.EnableWindow(TRUE);
			m_ButtonEnc5.EnableWindow(TRUE);
			m_Button11.EnableWindow(TRUE);
			m_Button12.EnableWindow(TRUE);
			
			board->command(CHAI_CMD_RESET_ENCODER_0, NULL);
			board->command(CHAI_CMD_RESET_ENCODER_1, NULL);
			board->command(CHAI_CMD_RESET_ENCODER_2, NULL);
			board->command(CHAI_CMD_RESET_ENCODER_3, NULL);
			board->command(CHAI_CMD_RESET_ENCODER_4, NULL);
			board->command(CHAI_CMD_RESET_ENCODER_5, NULL);
		
		}
		else
		{
			m_boardStatus = "Sensoray626 Board NOT Ready";
		}
		
		m_ButtonSensoray.EnableWindow(FALSE);
		m_ButtonServotogo.EnableWindow(FALSE);

		UpdateData(FALSE);
		
	
}

void CMy06_board_testerDlg::OnEnc1() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_1, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc1 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnEnc2() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_2, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc2 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnEnc3() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_3, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc3 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnEnc4() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_4, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc4 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnEnc5() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_5, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc5 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnEnc6() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_6, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc6 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnEnc7() 
{
  long encoder_value;
  board->command(CHAI_CMD_GET_ENCODER_7, &encoder_value);
  char buffer[100];
  sprintf(buffer, "%d", encoder_value);
  m_edtEnc7 = buffer;
  UpdateData(FALSE);
}

void CMy06_board_testerDlg::OnZeroDACS() 
{
        double DACvalue = 0;
        board->command(CHAI_CMD_SET_DAC_0,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_1,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_2,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_3,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_4,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_5,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_6,&DACvalue);
        board->command(CHAI_CMD_SET_DAC_7,&DACvalue);
	
}

void CMy06_board_testerDlg::OnSliderDACs() 
{
	double DACvalue = 0;
	UpdateData(TRUE);
	DACvalue = (double) m_sldValue;
	
	board->command(CHAI_CMD_SET_DAC_0,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_1,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_2,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_3,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_4,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_5,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_6,&DACvalue);
    board->command(CHAI_CMD_SET_DAC_7,&DACvalue);

}

void CMy06_board_testerDlg::OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
    UpdateData(TRUE);
    int position = m_sldValue;
	char buffer[100];
	sprintf(buffer, "%d", m_sldValue);
	m_edtSlider = buffer;
	UpdateData(FALSE);
	
	*pResult = 0;
}
