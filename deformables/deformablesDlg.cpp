#include "stdafx.h"
#include "deformables.h"
#include "cTeschnerMesh.h"
#include "deformablesDlg.h"
#include "meshExporter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

Cteschner_deformableDlg::Cteschner_deformableDlg(CWnd* pParent /*=NULL*/)
	: CwinmeshviewDlg(Cteschner_deformableDlg::IDD, pParent)
	, m_simtime_text(_T("Sim Time: 00.000"))
{
	initialize();
}

Cteschner_deformableDlg::Cteschner_deformableDlg(int idd, CWnd* pParent /*=NULL*/)
	: CwinmeshviewDlg(idd, pParent)
{
	initialize();
}

void Cteschner_deformableDlg::initialize() {
	//{{AFX_DATA_INIT(CwinmeshviewDlg)
	m_threadsim_check = false;
	//}}AFX_DATA_INIT
}

void Cteschner_deformableDlg::DoDataExchange(CDataExchange* pDX) {
	CwinmeshviewDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Cteschner_deformableDlg)
	DDX_Check(pDX, IDC_THREADSIM_CHECK, m_threadsim_check);
	//}}AFX_DATA_MAP  
	DDX_Text(pDX, IDC_SIMTIME_TEXT, m_simtime_text);
}

BEGIN_MESSAGE_MAP(Cteschner_deformableDlg, CwinmeshviewDlg)
	//{{AFX_MSG_MAP(Cteschner_deformableDlg)
	ON_BN_CLICKED(IDC_INITIALIZE_TMESH, OnInitializeMeshButton)
	ON_BN_CLICKED(IDC_TIMESTEP_BUTTON, OnSingleStepButton)
	ON_BN_CLICKED(IDC_STARTSTOPSIM_BUTTON, OnStartSimButton)
	ON_BN_CLICKED(IDC_THREADSIM_CHECK, OnThreadSimCheck)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LOAD_PROBLEMFILE_BUTTON, OnLoadProblemFileButton)
	ON_BN_CLICKED(IDC_EXPORT_DEFORMED_MODEL_BUTTON, OnBnClickedExportDeformedModel)
	ON_BN_CLICKED(IDC_AUTO_EXPORT_DEFORMED_MODEL_BUTTON, OnBnClickedAutoExportDeformedModel)
	ON_BN_CLICKED(IDC_RESET_MESH_BUTTON, OnBnClickedResetMesh)
	//ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()


void Cteschner_deformableDlg::OnInitializeMeshButton() {
	if (g_main_app->object == 0) return;

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(g_main_app->object);
	if (ctm) {
		_cprintf("Initializing deformable mesh...\n");
		ctm->initialize();
		_cprintf("Initialization complete...\n");
	}
	else _cprintf("Could not initialize deformable mesh...\n");

}


void Cteschner_deformableDlg::OnSingleStepButton() {
	if (((Cteschner_deformableApp*)(g_main_app->object)) == 0) return;

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(g_main_app->object);
	if (ctm == 0) {
		_cprintf("No mesh available...\n");
		return;
	}
	_cprintf("Ticking...\n");
	((Cteschner_deformableApp*)(g_main_app))->singletick();
	_cprintf("Preparing for rendering...\n");
	ctm->prepare_to_render();
	_cprintf("Timestep complete...\n");
}


void Cteschner_deformableDlg::OnRButtonDown(UINT nFlags, CPoint point) {

	if (m_left_scrolling_gl_area) {
		((Cteschner_deformableApp*)(g_main_app))->toggle_simulation(SIMTOGGLE_STOP);
	}

	else CwinmeshviewDlg::OnRButtonDown(nFlags, point);
}


void Cteschner_deformableDlg::OnStartSimButton() {
	((Cteschner_deformableApp*)(g_main_app))->toggle_simulation(SIMTOGGLE_TOGGLE);

	/*
	CButton* button = (CButton*)(GetDlgItem(IDC_STARTSTOPSIM_BUTTON));
	button->SetWindowText(
	  ((Cteschner_deformableApp*)(g_main_app))->m_simulation_running?"Stop Simulation":"Start Simulation"
	  );
	  */
}


void Cteschner_deformableDlg::OnThreadSimCheck() {
	UpdateData(TRUE);
	((Cteschner_deformableApp*)(g_main_app))->m_multithreaded_simulation =
		1 - ((Cteschner_deformableApp*)(g_main_app))->m_multithreaded_simulation;
}


void Cteschner_deformableDlg::OnLoadProblemFileButton() {

	bool bresult = ((Cteschner_deformableApp*)(g_main_app))->LoadProblemFile((char*)(0));
	if (bresult) g_main_app->resetCamera();

}

void Cteschner_deformableDlg::OnBnClickedExportDeformedModel() {
	if (g_main_app->object == 0) return;
	ExportModel(g_main_app->object, 0, &(g_main_app->current_mesh_transform), 0, &(((Cteschner_deformableApp*)(g_main_app))->m_exportHelper));
}

void Cteschner_deformableDlg::OnBnClickedAutoExportDeformedModel() {
	((Cteschner_deformableApp*)(g_main_app))->AutoExport();
}

void Cteschner_deformableDlg::OnBnClickedResetMesh() {

	if (g_main_app->object == 0) return;

	int result = 0;

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(g_main_app->object);
	if (ctm) {
		_cprintf("Resetting deformable mesh...\n");
		result = ctm->reset();
		_cprintf("Reset complete...\n");
	}
	if (ctm == 0 || result != 0) _cprintf("Could not initialize deformable mesh...\n");

}
