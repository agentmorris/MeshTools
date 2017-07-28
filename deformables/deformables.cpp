#include "stdafx.h"
#include "winmeshview_globals.h"
#include "deformables.h"
#include "deformablesDlg.h"
#include "cTeschnerMesh.h"
#include "cPrecisionClock.h"
#include "meshExporter.h"
#include "constraints.h"
#include "utils.h"
#include "RunningAverage.h"
#include <float.h>
#include <regex>

#ifdef _DEBUG
#define DEFAULT_TICKS_PER_RENDER_LOOP 1
#else
#define DEFAULT_TICKS_PER_RENDER_LOOP 20
#endif

#define DEFAULT_PROBLEM_FILENAME "default.defprob"
#define DEFAULT_DEFORMABLE_MODEL "test_models\\tet1.1.ele"
// #define DEFAULT_DEFORMABLE_MODEL "test_models\\cube.1.ele"

BEGIN_MESSAGE_MAP(Cteschner_deformableApp, CWinApp)
	//{{AFX_MSG_MAP(Cteschner_deformableApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

#ifdef COMPILING_TESCHNER_DEFORMABLE
Cteschner_deformableApp theApp;
#endif

using namespace std;

void Cteschner_deformableApp::problem_defaults() {

	cTetGenLoader::m_remove_small_tets_at_import = DEFAULT_REMOVE_SMALL_TETS_AT_IMPORT;
	cTetGenLoader::m_small_import_tet_volume = DEFAULT_SMALL_IMPORT_TET_VOLUME;

	m_periodically_outputting = false;
	m_output_interval = -1.0f;;
	m_begin_periodic_output_time = -1.0f;
	m_end_periodic_output_time = -1.0f;

	m_minimum_steadystate_simtime = 0.0;
	m_maximum_steadystate_simtime = FLT_MAX;
	m_reached_steady_state = false;
	m_steady_state_result = RESULT_NOT_DONE_YET;
	m_output_at_steady_state = false;
	m_monitor_steady_state = false;
	m_monitor_steady_state_after_reset = false;
	m_stop_at_steady_state = false;
	m_exit_at_steady_state = false;

	m_required_time_at_steady_state = 0.5f;
	m_max_acceleration_threshold = -1.0f;
	m_max_velocity_threshold = -1.0f;
	m_mean_acceleration_threshold = -1.0f;
	m_mean_velocity_threshold = -1.0f;
	m_required_time_at_steady_state = 0.0f;

	m_steady_state_start_time = -1.0f;
	m_last_output_time = 0.0;

	step_end_times.clear();
}

Cteschner_deformableApp::Cteschner_deformableApp() : m_exportHelper(this) {

	problem_defaults();

	m_reset_pending = false;
	m_loading_problem_file = false;
	m_loaded_problem_filename[0] = '\0';
	last_selected_triangle_highlight = 0;
	sim_thread_id = -1;
	m_simulation_running = 0;
	m_single_tick = 0;
	m_multithreaded_simulation = false;
	m_ticks_per_render_loop = DEFAULT_TICKS_PER_RENDER_LOOP;

	_cprintf("Created teschner_deformable application...\n");
}

// Load the model or problem file - if any - that was
// requested at the command line
void Cteschner_deformableApp::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast) {

	_cprintf("Parsing command line parameter: %s\n", pszParam);
	bool found_problem_file = false;
	if (m_current_param_index == 0) {
		found_problem_file = LoadProblemFile(pszParam);
	}
	if (found_problem_file == false) {
		CwinmeshviewApp::ParseParam(pszParam, bFlag, bLast);
	}
	else {
		m_current_param_index++;
	}
	_cprintf("Parsed command line parameter: %s\n", pszParam);
}


void Cteschner_deformableExportHelper::preExport(FILE* f, int filetype) {

	if (f == 0 || filetype != FILETYPE_ABAQUS_INP) return;

	// Nothing to write here...
	return;
}

void Cteschner_deformableExportHelper::postExport(FILE* f, int filetype) {

	if (f == 0 || filetype != FILETYPE_ABAQUS_INP) return;

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(myApp->object);

	if (ctm == 0) return;

	// Do we have explicitly-defined solid sections?
	bool multiple_sections = false;
	if (ctm->solid_sections.empty() == false) {
		multiple_sections = true;
	}


	if (multiple_sections == false) {

		// Define an element set that includes every tet
		fprintf(f, "*Elset, elset=WMV-ELSET, generate\n");
		fprintf(f, "1,%d,1\n", m_nTets - m_nBadTets);

		// Define a section to include that element set
		fprintf(f, "*Solid Section, elset=WMV-ELSET, material=WMV-MATERIAL\n");
		fprintf(f, "1.0,\n");

	}

	else {

		int section_index = 0;

		// Define an element set and a solid section for each section we've defined
		solid_section_list::iterator section_iter = ctm->solid_sections.begin();
		while (section_iter != ctm->solid_sections.end()) {

			fprintf(f, "*Elset, elset=WMV-ELSET%d, generate\n", section_index);
			solid_section* ssp = (*section_iter);

			int range_start = -1;
			int range_end = -1;

			// Loop over all tets
			std::vector<unsigned int>::iterator ssp_iter = ssp->tets.begin();
			while (1) {
				int tetnum = -1;
				bool finish_set = false;

				// Are we finishing a set because we hit the end of the list?
				if (ssp_iter == ssp->tets.end()) {
					finish_set = true;
				}

				// We're not at the end of the list
				else {
					tetnum = (*ssp_iter);

					// Are we finishing a set because there's a jump in the list?
					if ((range_start != -1) && (tetnum != (range_end + 1))) {
						finish_set = true;
					}

					else {
						// Keep counting...
						if (range_start == -1) range_start = tetnum;
						range_end = tetnum;
					}

				}

				if (finish_set) {
					// Abaqus likes 1-indexed element numbers
					_cprintf("Writing an element set from %d to %d\n", range_start + 1, range_end + 1);
					fprintf(f, "%d,%d,1\n", range_start + 1, range_end + 1);
					range_start = tetnum;
					range_end = tetnum;
				}

				if (ssp_iter == ssp->tets.end()) {
					break;
				}
				ssp_iter++;
			} // for each tet

			// Define a section to include that element set
			fprintf(f, "*Solid Section, elset=WMV-ELSET%d, material=WMV-MATERIAL%d\n", section_index, section_index);
			fprintf(f, "1.0,\n");

			section_index++;
			section_iter++;

		} // for each section

	} // if we need to define multiple sections

	// Close the part
	fprintf(f, "*End part\n");

	int nsteps = myApp->step_end_times.size();

	// Write our problem info...

	// Create an assembly
	fprintf(f, "*Assembly, name=WMV-Assembly\n");

	// Create one instance of our model
	fprintf(f, "*Instance, name=WMV-Instance, part=WINMESHVIEW_EXPORTED_PART\n");
	fprintf(f, "*End Instance\n");

	// Create node sets for every constraint
	constraint_set* cs = ctm->m_constraints;

	std::multiset<constraint*, lt_constraint_start>::iterator iter =
		cs->constraints_by_start_time.begin();

	int constraint_number = 0;

	while (iter != cs->constraints_by_start_time.end()) {

		constraint* c = (*iter);
		int nvertices = c->vertices.size();

		// Determine whether this node set can be written out as
		// a continuous range
		bool range = true;

		if (c->affects_all_vertices) range = true;
		else {
			for (int i = 1; i < nvertices; i++) {
				if (c->vertices[i] != c->vertices[i - 1] + 1) {
					range = false;
					break;
				}
			}
		}

		if (range) {
			fprintf(f, "*Nset, nset=constraintset%d, instance=WMV-Instance, generate\n", constraint_number);
			fprintf(f, "%d, %d, 1\n", c->vertices[0] + 1, c->vertices[nvertices - 1] + 1);
		}

		// We need to write out each vertex...
		else {
			fprintf(f, "*Nset, nset=constraintset%d, instance=WMV-Instance\n", constraint_number);
			int linepos = 0;
			for (int i = 0; i < nvertices; i++) {
				// Write out the vertex # with a possible comma
				fprintf(f, "%d%s ", c->vertices[i] + 1, (i == nvertices - 1) ? "" : ",");
				if ((++linepos == 16) && (i != nvertices - 1)) {
					fprintf(f, "\n");
					linepos = 0;
				}
			} // for each vertex
			fprintf(f, "\n");
		} // if we need to write out each vertex

		iter++;
		constraint_number++;
	}

	fprintf(f, "*End Assembly\n");

	if (multiple_sections == false) {

		// Create a default material
		fprintf(f, "*Material, name=WMV-MATERIAL\n");
		fprintf(f, "*Elastic\n");
		fprintf(f, "%lf, %lf\n", ctm->material_props.youngs_modulus, ctm->material_props.poisson_coeff);

	}

	else {

		// Create a material for each section we've defined
		int section_index = 0;
		solid_section_list::iterator section_iter = ctm->solid_sections.begin();

		while (section_iter != ctm->solid_sections.end()) {

			solid_section* ssp = (*section_iter);
			fprintf(f, "*Material, name=WMV-MATERIAL%d\n", section_index);
			fprintf(f, "*Elastic\n");
			fprintf(f, "%lf, %lf\n", ssp->props.youngs_modulus, ssp->props.poisson_coeff);
			section_iter++;
			section_index++;
		}

	}


	// See which position constraints are really displacement boundary
	// conditions
	iter = cs->constraints_by_start_time.begin();
	constraint_number = 0;

	while (iter != cs->constraints_by_start_time.end()) {

		constraint* c = (*iter);

		bool boundary_condition = false;
		if (nsteps > 0 &&
			c->type == CONSTRAINT_POSITION_CONSTANT &&
			c->start_time <= 0.0 &&
			c->end_time >= myApp->step_end_times[nsteps - 1]) boundary_condition = true;

		if (nsteps == 0 && c->start_time <= 0.0 && c->type == CONSTRAINT_POSITION_CONSTANT)
			boundary_condition = true;

		// If this is a position constraint that extends over our
		// whole analysis...
		if (boundary_condition) {

			fprintf(f, "*Boundary\n");

			// Constrain all six degrees of freedom
			//
			// TODO: may not really want rotational constraints
			/*
			for(int k=0; k<6; k++) {
			fprintf(f,"constraintset%d %d %d\n",constraint_number,k,k);
			}
			*/
			fprintf(f, "constraintset%d, %d, %d\n", constraint_number, 1, 6);
		}
		iter++;
		constraint_number++;
	}

	// Create steps
	for (int i = 0; i < nsteps; i++) {

		float start_time = (i == 0) ? 0.0f : myApp->step_end_times[i - 1];
		float end_time = myApp->step_end_times[i];
		fprintf(f, "*Step, name=WMV-STEP%02d, amplitude=STEP\n*Static\n", i + 1);
		float duration = end_time - start_time;
		fprintf(f, "%f, %f, %f, %f\n", duration, duration, duration*1e-05, duration);

		// Output information
		fprintf(f, "*Restart, write, frequency=0\n");
		fprintf(f, "*Output, field, variable=PRESELECT\n");
		fprintf(f, "*Output, history, variable=PRESELECT\n");

		// Output information (comments)
		fprintf(f, "\n** Abaqus/Explicit only...\n**FILEOUTPUT, NUMBERINTERVAL=1\n\n");

		// Results file output
		fprintf(f, "** Node output, displacement and loads\n");
		fprintf(f, "*NODE FILE\nU, CF, COORD\n");
		// Data file output
		fprintf(f, "*NODE PRINT\nU, CF, COORD\n\n");

		fprintf(f, "** Other node information (explicit only):\n");
		fprintf(f, "** V, A\n\n");

		// Results file element output
		fprintf(f, "** Element output, stresses and strains\n");
		fprintf(f, "*ELFILE, POSITION=NODES\n");
		fprintf(f, "S\n");
		fprintf(f, "E\n");
		fprintf(f, "\n");

		// Data file element output
		fprintf(f, "*ELPRINT, POSITION=NODES\n");
		fprintf(f, "S\n");
		fprintf(f, "E\n");
		fprintf(f, "\n");

		fprintf(f, "** Other stresses/strains:\n");
		fprintf(f, "** SP, E, EP, EE, EEP\n\n");

		// See which constraints should be represented as loads
		iter = cs->constraints_by_start_time.begin();
		constraint_number = 0;

		while (iter != cs->constraints_by_start_time.end()) {

			constraint* c = (*iter);

			if (c->type == CONSTRAINT_FORCE_CONSTANT) {

				// See if this force should be included in this step... our standard will
				// be that if _any_ part of the force overlaps with this step, it goes in this step.
				if (
					// If the constraint times straddle the step start time
					(c->start_time <= start_time && c->end_time >= start_time) ||
					// If the constraint times straddle the step end time
					(c->start_time <= end_time   && c->end_time >= end_time) ||
					// If the constraint is entirely within the step
					(c->start_time >= start_time && c->end_time <= end_time)
					) {
					fprintf(f, "*Cload\n");
					constraint_force_constant* fc = (constraint_force_constant*)(c);
					if (fc->force.x != 0.0) fprintf(f, "constraintset%d, 1, %lf\n", constraint_number, fc->force.x);
					if (fc->force.y != 0.0) fprintf(f, "constraintset%d, 2, %lf\n", constraint_number, fc->force.y);
					if (fc->force.z != 0.0) fprintf(f, "constraintset%d, 3, %lf\n", constraint_number, fc->force.z);
				}
			}
			iter++;
			constraint_number++;
		}

		fprintf(f, "*End Step\n");
	}

	// TODO: process transient position constraints
}


int Cteschner_deformableApp::ExitInstance() {
	_cprintf("Exiting teschner_deformable instance...\n");
	// _getch();
	return CwinmeshviewApp::ExitInstance();
}


void Cteschner_deformableApp::enableGUI(bool enable) {

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);

	if (enable) {

		_cprintf("Enabling GUI...\n");
		if (g_main_dlg == 0) {
			_cprintf("Creating dialog...\n");
			AfxEnableControlContainer();
			Cteschner_deformableDlg* dlg = new Cteschner_deformableDlg;
			g_main_dlg = dlg;
			m_pMainWnd = dlg;
			dlg->Create(IDD_teschner_deformable_DIALOG, NULL);

			// Place the window at the far right of the desktop
			RECT r;
			::GetWindowRect(m_pMainWnd->m_hWnd, &r);
			int winwidth = r.right - r.left;
			int winheight = r.bottom - r.top;

			int desktopwidth = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
			int desktopheight = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

			::SetWindowPos(m_pMainWnd->m_hWnd, HWND_TOP, desktopwidth - winwidth, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);

			// Position the console window so it looks nice...
			HWND con_wnd = GetConsoleWindow();

			if (con_wnd) {
				RECT r;
				::GetWindowRect(con_wnd, &r);
				int winwidth = r.right - r.left;
				int winheight = r.bottom - r.top;

				int primary_width = ::GetSystemMetrics(SM_CXSCREEN);

				int xloc = 0;
				if (desktopwidth > primary_width) xloc = primary_width;
				::SetWindowPos(con_wnd, HWND_TOP, xloc, 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
			}

			_cprintf("Creating viewport...\n");
			if (viewport) delete viewport;
			viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);
			viewport->setPostRenderCallback(this);
		}
		if (viewport == 0) {
			viewport = new cViewport(g_main_dlg->m_gl_area_hwnd, camera, false);
			viewport->setPostRenderCallback(this);
		}

		if (ctm) {
			ctm->m_compute_vertex_normals = true;
		}

		_cprintf("Enabled GUI\n");

	}

	else {
		_cprintf("Disabling GUI...\n");
		if (viewport) {
			_cprintf("Destroying viewport...\n");
			delete viewport;
			viewport = 0;
		}
		if (g_main_dlg) {
			_cprintf("Destroying window...\n");
			((CwinmeshviewDlg*)(g_main_dlg))->DestroyWindow();
			// delete g_main_dlg;
			g_main_dlg = 0;
		}

		// Don't need to compute vertex normals if there's no rendering
		// going on
		if (ctm) {
			ctm->m_compute_vertex_normals = false;
		}

		_cprintf("Disabled GUI...\n");
	}

	m_gui_enabled = enable;
}

#ifdef COMPILING_DLL
#pragma message("Compiling TD DLL...")
#endif

#ifdef COMPILING_APP
#pragma message("Compiling TD app...")
#endif


BOOL Cteschner_deformableApp::InitInstance() {

	_cprintf("Initializing teschner_deformable instance...\n");

#ifdef COMPILING_DLL
	m_gui_enabled = false;
#else
	m_gui_enabled = true;
#endif

	// Let the parent class create the world
	CwinmeshviewApp::InitInstance();

#ifdef COMPILING_TESCHNER_DEFORMABLE
	enableGUI(m_gui_enabled);
#endif

	if (m_factory_mesh) delete m_factory_mesh;
	m_factory_mesh = new cTeschnerMesh(world);

	// Create a transparent floor
	floor = new cMesh(world);
	floor->newVertex(-10, 0, -10);
	floor->newVertex(-10, 0, 10);
	floor->newVertex(10, 0, 10);
	floor->newVertex(10, 0, -10);
	floor->newTriangle(0, 1, 2);
	floor->newTriangle(0, 2, 3);
	floor->enableTransparency(true, true);
	floor->useMaterial(true, true);
	floor->setPos(0, DEFAULT_FLOOR_POSITION, 0);
	cMaterial m;
	m.m_diffuse.set(0.5, 0.5, 0.8, 0.5);
	m.m_ambient.set(0.5, 0.5, 0.8, 0.5);
	floor->setMaterial(m, true);
	floor->deleteCollisionDetector(true);
	world->addChild(floor);
	//world->setShowFrame(true);

#ifdef COMPILING_TESCHNER_DEFORMABLE

	loadCommandLineModel();
	if (object == 0) createDefaultModel();
	if (object) {
		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
		if (ctm) {
			ctm->initialize();
			_cprintf("Model initialized...\n");
			// toggle_simulation(SIMTOGGLE_START);      
		}
	}
	else {
		_cprintf("Could not load command line or default object...\n");
	}

#endif

	_cprintf("Initialized teschner_deformable instance...\n");

	return true;

}


bool Cteschner_deformableApp::createTet() {

	cTeschnerMesh* ctm = new cTeschnerMesh(world);
	ctm->m_nTets = 1;
	ctm->m_tets = new unsigned int[4];
	for (int i = 0; i < 4; i++) ctm->m_tets[i] = i;
	ctm->newVertex(0, 0, 0);
	ctm->newVertex(0, 1, 0);
	ctm->newVertex(0, 0, 1);
	ctm->newVertex(1, 1, 1);
	for (int i = 0; i < 4; i++) {
		ctm->newTriangle(tet_triangle_faces[i][0], tet_triangle_faces[i][1],
			tet_triangle_faces[i][2]);
	}
	if (object) {
		world->removeChild(object);
		delete object;
	}
	object = ctm;
	world->addChild(object);

	return true;
}

bool Cteschner_deformableApp::createDefaultModel() {

	// the superclass init method may have already loaded a command-line-
	// specified file
	if (object == 0) {
		if (LoadProblemFile(DEFAULT_PROBLEM_FILENAME)) {
			_cprintf("Loaded default problem\n");
		}
		else {
			bool result = LoadModel(DEFAULT_DEFORMABLE_MODEL);
			if (result) _cprintf("Loaded default model\n");
			else _cprintf("Warning: couldn't load default model or default problem\n");
		}
	}

	if (object == 0) {
		createTet();
	}

	world->computeGlobalPositions(false);

	// This will set rendering and haptic options as they're defined in
	// the GUI...
	if (g_main_dlg) g_main_dlg->update_rendering_options_from_gui(this);

	return true;
}


void Cteschner_deformableApp::select(CPoint p) {
	CwinmeshviewApp::select(p);

	if (selected_triangle)
		_cprintf("Selected triangle %d (nodes %d,%d,%d)\n",
			selected_triangle->getIndex(),
			selected_triangle->getVertexIndex(0),
			selected_triangle->getVertexIndex(1),
			selected_triangle->getVertexIndex(2));

}
void Cteschner_deformableApp::scroll(CPoint p, int button) {

	int shift_pressed = GetAsyncKeyState(VK_SHIFT) & (1 << 15);

	if (button != MOUSE_BUTTON_LEFT || shift_pressed == 0) {
		CwinmeshviewApp::scroll(p, button);
		return;
	}

	/*
	if (last_selected_triangle_highlight == 0) {
	  last_selected_triangle_highlight = new cMesh(world);
	  last_selected_triangle_highlight->newTriangle(
		cVector3d(0,0,0),cVector3d(0,0,0),cVector3d(0,0,0));
	}

	object->setWireMode(true,true);

	if (object->containsChild(last_selected_triangle_highlight,false)==false) {
	  object->addChild(last_selected_triangle_highlight);
	}
	for(int i=0; i<3; i++) {
	  last_selected_triangle_highlight->getVertex(i)->
		setPos(selected_triangle->getVertex(i)->getPos());
	}
	last_selected_triangle_highlight->setWireMode(false,true);
	_cprintf("Activated triangle highlight\n");
	*/

}


bool Cteschner_deformableApp::ParseProblemFileCommand(const char* line, int cur_line_number,
	constraint_set* constraints, external_material_properties* material_props) {

	const char* buf = line;

	cmatch matches;
	bool result;

	// Look for comments (whitespace # stuff)
	regex comment_exp("^\\s*(#|(\\s*$))", regex::icase);
	result = regex_search(buf, matches, comment_exp);
	if (result) {
		return false;
	}

	// What type of entry do we have here?
	//
	// The first match is the entry type, the second match
	// is everything else.
	regex type_exp("^\\s*(\\S+)\\s+(\\S.*)$", regex::icase);
	result = regex_search(buf, matches, type_exp);
	if (result == false || matches.size() < 3) {
		_cprintf("Could not parse problem entry %s\n", buf);
		return false;
	}

	string entry_type(matches[1].first, matches[1].second);
	string entry_data(matches[2].first, matches[2].second);

	// Is this a constraint specification?
	if (entry_type == "constraint") {
		_cprintf("Parsing and adding a constraint...\n");
		if (constraints) constraints->add_constraint(buf);
		_cprintf("Parsed and added a constraint...\n");
	}

	// Is this a model specification?
	else if (entry_type == "model") {
		_cprintf("Loading model %s\n", entry_data.c_str());
		bool bresult = LoadModel(entry_data.c_str());
		if (bresult == false) {
			_cprintf("Warning: could not load model file specified on line %d\n", cur_line_number);
		}
	}

	// Is this a rendermodel specification?
	else if (entry_type == "rendermodel") {
		_cprintf("Loading render model %s\n", entry_data.c_str());
		bool bresult = LoadRenderModel(entry_data.c_str());
		if (bresult == false) {
			_cprintf("Warning: could not load render model file specified on line %d\n", cur_line_number);
		}
		else {
			_cprintf("Loaded render model %s\n", entry_data.c_str());
		}
	}

	else if (entry_type == "ticks_per_render_loop") {
		const char* data = entry_data.c_str();
		if (data == 0) {
			_cprintf("Could not process ticks_per_render_loop %s\n", buf);
			return false;
		}
		int result = sscanf(data, "%d", &m_ticks_per_render_loop);
		if (result != 1) {
			_cprintf("Could not convert ticks_per_render_loop %s\n", buf);
			return false;
		}
		_cprintf("Using %d ticks per render loop\n", m_ticks_per_render_loop);
	}

	// Is this a request to enable or disable heterogeneous constants?
	else if (entry_type == "enable_nonhomogeneous_constants") {

		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
		if (ctm == 0) {
			_cprintf("No object to enable heterogeneity for...\n");
			return false;
		}

		regex constant_exp("^\\s*(\\S+)", regex::icase);
		result = regex_search(entry_data.c_str(), matches, constant_exp);
		if (result == false || matches.size() < 1) {
			_cprintf("Could not parse homogeneous property enabling specification %s\n", buf);
			return false;
		}
		int value;
		string valuestr(matches[1].first, matches[1].second);
		result = sscanf(valuestr.c_str(), "%d", &value);
		if (result == 0) {
			_cprintf("Could not parse homogeneous property enabling value %s\n", valuestr.c_str());
			return false;
		}

		if (ctm->m_heterogeneous_constants == false && (value)) {
			ctm->initialize_nonhomogeneity();
		}
		ctm->m_heterogeneous_constants = (bool)value;
	}

	// Is this a specification of external material properties?
	else if (entry_type == "abaqus_material_prop") {

		regex constant_exp("^\\s*(\\S+)\\s+(\\S+)", regex::icase);
		result = regex_search(entry_data.c_str(), matches, constant_exp);
		if (result == false || matches.size() < 3) {
			_cprintf("Could not parse material property specification %s\n", buf);
			return false;
		}
		double value;
		string valuestr(matches[2].first, matches[2].second);
		result = sscanf(valuestr.c_str(), "%lf", &value);
		if (result == 0) {
			_cprintf("Could not parse material property value %s\n", valuestr.c_str());
			return false;
		}

		string namestr(matches[1].first, matches[1].second);

		_cprintf("Parsed material property %s, value %lf\n", namestr.c_str(), value);

		if (namestr == "youngs_modulus") material_props->youngs_modulus = value;
		else if (namestr == "poisson_coeff") material_props->poisson_coeff = value;
		else _cprintf("Unrecognized material property [%s]\n", namestr.c_str());
		return false;
	}

	// Is this a solid section spec?
	else if (entry_type == "solid_section") {

		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
		if (ctm == 0) {
			_cprintf("No object to apply a solid section to...\n");
			return false;
		}

		regex constant_exp("^\\s*(\\S+)(\\s+)?(\\S+)?(\\s+)?(\\S+)?", regex::icase);

		result = regex_search(entry_data.c_str(), matches, constant_exp);
		int nmatches = matches.size();
		if (result == false || nmatches < 2) {
			_cprintf("Could not parse constant specification %s (%d matches)\n", buf, nmatches);
			return false;
		}

		string match1(matches[1].first, matches[1].second);
		if (match1 == "clear") {
			_cprintf("Clearing solid sections...\n");
			ctm->clear_solid_sections();
			return true;
		}

		if (nmatches < 6) {
			_cprintf("Unrecognized solid_section specifier %s\n", buf);
			return false;
		}

		string str_value1(matches[1].first, matches[1].second);
		string str_value2(matches[3].first, matches[3].second);
		string str_element_list(matches[5].first, matches[5].second);

		/*
		_cprintf("Value 1: %s\n",str_value1.c_str());
		_cprintf("Value 2: %s\n",str_value2.c_str());
		_cprintf("Element list: %s\n",str_element_list.c_str());
		*/

		float fvalue1, fvalue2;
		result = sscanf(str_value1.c_str(), "%f", &fvalue1);
		if (result == 0) {
			_cprintf("Could not parse constant value %s\n", str_value1.c_str());
			return false;
		}
		result = sscanf(str_value2.c_str(), "%f", &fvalue2);
		if (result == 0) {
			_cprintf("Could not parse constant value %s\n", str_value2.c_str());
			return false;
		}

		solid_section* ssp = new solid_section;
		ssp->props.youngs_modulus = fvalue1;
		ssp->props.poisson_coeff = fvalue2;

		// Grab tokens from the vertex list...
		regex r("\\s*\\(?([^\\)]*)\\)?\\s*$");
		str_element_list = regex_replace(str_element_list,
			r,
			"$1");

		regex delimiter("[ ,]");
		sregex_token_iterator i(str_element_list.begin(), str_element_list.end(), delimiter, -1);

		// Defaults to "end"
		sregex_token_iterator j;

		// For each token in the element list
		while (i != j) {
			int val;
			string s(*i);

			// Is this an x-y formatted string?
			regex r("^\\s*(\\d+)\\s*-\\s*(\\d+)[\r\n]?$", regex::icase);

			result = regex_match(s.c_str(), matches, r);
			if (result == true) {
				string start(matches[1].first, matches[1].second);
				string end(matches[2].first, matches[2].second);

				int istart, iend;
				result = sscanf(start.c_str(), "%d", &istart);
				if (result == 0) {
					_cprintf("Error converting range %s\n", s.c_str());
					++i; continue;
				}

				result = sscanf(end.c_str(), "%d", &iend);
				if (result == 0) {
					_cprintf("Error converting range %s\n", s.c_str());
					++i; continue;
				}

				if (end < start) {
					_cprintf("Illegal range %s\n", s.c_str());
				}

				_cprintf("Converted a range: %d - %d\n", istart, iend);

				for (int k = istart; k <= iend; k++)
					ssp->tets.push_back(k);

			}

			// Should be a single element
			else {
				result = sscanf(s.c_str(), "%d", &val);
				if (result == 0) {
					_cprintf("Error converting vertex %s\n", s.c_str());
					++i; continue;
				}
				ssp->tets.push_back(val);
			}
			++i;
		} // for each token in the element list      

		/*
		std::vector<unsigned int>::iterator iter = ssp->tets.begin();
		_cprintf("Elements read:\n");
		while(iter != ssp->tets.end()) {
		  _cprintf("%d,",(*iter));
		  iter++;
		}
		_cprintf("\n");
		*/

		ctm->solid_sections.push_back(ssp);

		_cprintf("Loaded a solid section (%d) with %d elements (%d to %d)\n",
			ctm->solid_sections.size(), ssp->tets.size(), ssp->tets.front(), ssp->tets.back());

	} // solid_section


	// Is this a constant specification?
	else if (entry_type == "constant") {

		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
		if (ctm == 0) {
			_cprintf("No object to apply a constant to...\n");
			return false;
		}

		regex constant_exp("^\\s*(\\S+)\\s+(\\S+)(\\s+)?(\\S+)?", regex::icase);
		result = regex_search(entry_data.c_str(), matches, constant_exp);
		if (result == false || matches.size() < 5) {
			_cprintf("Could not parse constant specification %s\n", buf);
			return false;
		}

		float value;
		string valuestr(matches[2].first, matches[2].second);
		result = sscanf(valuestr.c_str(), "%f", &value);
		if (result == 0) {
			_cprintf("Could not parse constant value %s\n", valuestr.c_str());
			return false;
		}

		string namestr(matches[1].first, matches[1].second);

		// Is this a heterogeneous constant spec?
		if (matches[4].length() > 0) {

			// _cprintf("Appears to specify heterogeneous constant values...\n");
			string str_vertices(matches[4].first, matches[4].second);

			std::list<unsigned int> vertices;

			// Grab tokens from the vertex list...
			regex r("\\s*\\(?([^\\)]*)\\)?\\s*$");
			str_vertices = regex_replace(str_vertices,
				r,
				"$1");

			regex delimiter("[ ,]");
			sregex_token_iterator i(str_vertices.begin(), str_vertices.end(), delimiter, -1);

			// Defaults to "end"
			sregex_token_iterator j;

			// For each token in the vertex list
			while (i != j) {
				int val;
				string s(*i);

				// Is this an x-y formatted string?
				regex r("^\\s*(\\d+)\\s*-\\s*(\\d+)[\r\n]?$", regex::icase);

				result = regex_match(s.c_str(), matches, r);
				if (result == true) {
					string start(matches[1].first, matches[1].second);
					string end(matches[2].first, matches[2].second);

					int istart, iend;
					result = sscanf(start.c_str(), "%d", &istart);
					if (result == 0) {
						_cprintf("Error converting range %s\n", s.c_str());
						++i; continue;
					}

					result = sscanf(end.c_str(), "%d", &iend);
					if (result == 0) {
						_cprintf("Error converting range %s\n", s.c_str());
						++i; continue;
					}

					if (end < start) {
						_cprintf("Illegal range %s\n", s.c_str());
					}

					_cprintf("Converted a range: %d - %d\n", istart, iend);

					for (int k = istart; k <= iend; k++)
						vertices.push_back(k);

				}

				// Should be a single vertex
				else {
					result = sscanf(s.c_str(), "%d", &val);
					if (result == 0) {
						_cprintf("Error converting vertex %s\n", s.c_str());
						++i; continue;
					}
					vertices.push_back(val);
				}
				++i;
			} // for each token in the vertex list      

			// Now we have a vertex list
			pending_constant_assignment pca;
			pca.value = value;
			if (namestr == "kv") pca.constant_name = KVOLUME;
			else if (namestr == "ka") pca.constant_name = KAREA;
			else if (namestr == "kd") pca.constant_name = KDISTANCE;
			else if (namestr == "kd_damp") pca.constant_name = KDAMPING;
			else {
				_cprintf("Unrecognized heterogeneous constant name %s...\n", namestr.c_str());
				return false;
			}
			pca.vertices = vertices;
			// _cprintf("Pending assignment of value %f to constant %s for %d vertices\n",
			//   value,namestr.c_str(),vertices.size());

			if (ctm) {
				ctm->m_pending_constant_assignments.push_back(pca);
			}

			// Actually put these values in action if necessary
			if (ctm && ctm->m_initialized) {
				// _cprintf("Applying immediately...\n");
				ctm->process_pending_constant_assignments();
			}
		}

		// Not a heterogeneous constant spec...
		else {
			_cprintf("Parsed constant %s, value %f\n", namestr.c_str(), value);

			if (namestr == "timestep") ctm->m_timestep = value;
			else if (namestr == "kv") {
				ctm->m_kVolumePreservation = value;
				ctm->m_heterogeneous_constant_flags &= (~(1 << KVOLUME));
			}
			else if (namestr == "ka") {
				ctm->m_kAreaPreservation = value;
				ctm->m_heterogeneous_constant_flags &= (~(1 << KAREA));
			}
			else if (namestr == "kd") {
				ctm->m_kDistancePreservation = value;
				ctm->m_heterogeneous_constant_flags &= (~(1 << KDISTANCE));
			}
			else if (namestr == "kd_damp") {
				ctm->m_kDistanceDamping = value;
				ctm->m_heterogeneous_constant_flags &= (~(1 << KDAMPING));
			}

			else if (namestr == "kfloor") ctm->m_kFloor = value;
			else if (namestr == "external_force_scale") ctm->m_external_force_scale = value;
			else if (namestr == "time_force_exponent") ctm->m_time_force_exponent = value;

			else if (namestr == "gravity_force") ctm->m_gravity_force = value;
			else if (namestr == "floor_position") ctm->m_floor_position = value;

			else if (namestr == "render_vertex_constraints")
				ctm->m_render_vertex_constraints = (value == 0.0) ? false : true;
			else if (namestr == "gravity_enabled")
				ctm->m_gravity_enabled = (bool)value;
			else if (namestr == "floor_enabled")
				ctm->m_floor_force_enabled = (bool)value;

			else if (namestr == "mass_assignment_strategy")
				ctm->m_mass_assignment_strategy = (int)value;
			else if (namestr == "mass_assignment_constant")
				ctm->m_mass_assignment_constant = value;

			else {
				_cprintf("Unrecognized constant [%s]\n", namestr.c_str());
				return false;
			}
			_cprintf("Read value %f for constant %s\n", (float)value, namestr.c_str());
		}
	}

	else if (entry_type == "remove_small_tets_at_import") {
		int value;
		sscanf(entry_data.c_str(), "%d", &value);
		cTetGenLoader::m_remove_small_tets_at_import = (value != 0);
		_cprintf("Read value %d for entry %s\n", value, entry_type.c_str());
	}
	else if (entry_type == "small_import_tet_volume") {
		int value;
		sscanf(entry_data.c_str(), "%d", &value);
		cTetGenLoader::m_small_import_tet_volume = value;
		_cprintf("Read value %d for entry %s\n", value, entry_type.c_str());
	}

	// Is this a periodic output specification?
	else if (entry_type == "periodic_output") {
		_cprintf("Processing a periodic output directive %s\n", buf);
		float begin, end, period;
		int result = sscanf(entry_data.c_str(), "%f %f %f", &begin, &end, &period);
		if (result != 3) {
			_cprintf("Error processing periodic output directive %s\n", entry_data.c_str());
			return false;
		}
		else {
			m_begin_periodic_output_time = begin;
			m_end_periodic_output_time = end;
			m_periodically_outputting = true;
			m_output_interval = period;
		}
	}

	// Is this a steady-state output specification?
	else if (entry_type == "steady_state_output") {

		_cprintf("Processing a steady state output directive %s\n", buf);
		float mintime, maxtime;
		int exit_at_steady_state;
		int result = sscanf(entry_data.c_str(), "%f %f %d", &mintime, &maxtime, &exit_at_steady_state);
		if (result != 3) {
			_cprintf("Error processing steady-state output directive %s\n", entry_data.c_str());
			return false;
		}
		else {
			m_minimum_steadystate_simtime = mintime;
			m_maximum_steadystate_simtime = maxtime;
			m_exit_at_steady_state = exit_at_steady_state;
			m_output_at_steady_state = true;
			m_stop_at_steady_state = true;
			m_monitor_steady_state = true;
			m_monitor_steady_state_after_reset = true;
		}
	}

	// Is this a steady-state output parameter specification?
	else if (entry_type == "steady_state_parameters") {

		_cprintf("Processing a steady state parameter directive %s\n", buf);
		float maxvel, meanvel, maxaccel, meanaccel, required_time;

		int result = sscanf(entry_data.c_str(), "%f %f %f %f %f",
			&maxvel, &meanvel, &maxaccel, &meanaccel, &required_time);
		if (result != 5) {
			_cprintf("Error processing steady-state parameter directive %s\n", entry_data.c_str());
			return false;
		}
		else {
			m_max_velocity_threshold = maxvel;
			m_mean_velocity_threshold = meanvel;
			m_max_acceleration_threshold = maxaccel;
			m_mean_acceleration_threshold = meanaccel;
			m_required_time_at_steady_state = required_time;
		}

		m_stop_at_steady_state = true;
		m_monitor_steady_state = true;
		m_monitor_steady_state_after_reset = true;

	}

	// Is this an Abaqus step block directive?
	else if (entry_type == "steps") {

		regex delimiter(" ");
		sregex_token_iterator i(entry_data.begin(), entry_data.end(), delimiter, -1);

		// Defaults to "end"
		sregex_token_iterator j;

		while (i != j) {
			float val;
			string s(*i);

			int result = sscanf(s.c_str(), "%f", &val);
			if (result != 1) {
				_cprintf("Error processing step list (%s)\n", entry_data.c_str());
				return false;
			}

			step_end_times.push_back(val);
			++i;
		}

		_cprintf("Processed output steps: ");
		for (unsigned int i = 0; i < step_end_times.size(); i++) {
			_cprintf("%f ", step_end_times[i]);
		}
		_cprintf("\n");

	}

	else {
		_cprintf("Unrecognized problem file entry: %s\n", buf);
		return false;
	}

	return true;
}


bool Cteschner_deformableApp::LoadProblemFile(const char* in_filename) {

	char filename[_MAX_PATH];

	if (in_filename) {
		strcpy(filename, in_filename);
	}

	else {
		int result = FileBrowse(filename, _MAX_PATH, 0, 0,
			"problem files (*.defprob)|*.defprob|All Files (*.*)|*.*||",
			"Choose a problem file...");

		if (result < 0) {
			_cprintf("File browse canceled...\n");
			return false;
		}
	}

	char* extension = find_extension(filename);
	if (strcmp(extension, "defprob")) {
		_cprintf("%s does not have a problem file extension...\n", filename);
		return false;
	}

	_cprintf("Loading problem file %s\n", filename);

	FILE* f = fopen(filename, "r");
	if (f == 0) {
		_cprintf("Could not open problem file %s\n", filename);
		return false;
	}

	strcpy(m_loaded_problem_filename, filename);

#define MAX_LINE_LENGTH 2000
	char buf[MAX_LINE_LENGTH];

	m_loading_problem_file = true;

	problem_defaults();

	// Stop our simulation
	bool simulation_was_running = m_simulation_running;
	toggle_simulation(SIMTOGGLE_STOP);

	/*
	// Clear the existing deformable model
	if (object) {
	  object->removeFromGraph();
	  delete object;
	  object = 0;
	}
	*/

	constraint_set* constraints = new constraint_set;
	external_material_properties material_props;

	int cur_line_number = 0;

	while (1) {

		char* cresult = fgets(buf, MAX_LINE_LENGTH, f);
		if (cresult == 0) break;

		cur_line_number++;

		// Eliminate trailing newline
		if (buf[0] && (buf[strlen(buf) - 1] == '\n')) buf[strlen(buf) - 1] = '\0';

		ParseProblemFileCommand(buf, cur_line_number, constraints, &material_props);

	}

	fclose(f);

	// Need to reset new constraint sets
	constraints->reset();

	_cprintf("Found %d constraints\n", constraints->constraints_by_start_time.size());
	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm) {

		if (ctm->m_constraints) {
			delete ctm->m_constraints;
			ctm->m_constraints = 0;
		}

		ctm->m_constraints = constraints;
		ctm->material_props = material_props;
		floor->setShow(ctm->m_floor_force_enabled);
		floor->setPos(0, ctm->m_floor_position, 0);
	}
	else {
		delete constraints;
	}

	if (simulation_was_running) {
		if (ctm) {
			ctm->initialize();
			toggle_simulation(SIMTOGGLE_START);
		}
	}

	m_loading_problem_file = false;

	_cprintf("Loaded problem file...\n");

	return true;
}


bool Cteschner_deformableApp::AutoExport(char* out_filename, bool correct_for_transform) {
	return AutoExport(FILETYPE_ANODE, out_filename, true, correct_for_transform);
}


bool Cteschner_deformableApp::AutoExport(int filetype, char* out_filename, bool include_simtime, bool correct_for_transform) {

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm == 0) return false;

	char export_filename[_MAX_PATH];
	char default_filename[_MAX_PATH];

	// Find the extension for the selected file type
	const char* selected_extension = "anode";
	if (filetype >= 0 && filetype < FILETYPE_INVALID) selected_extension = mesh_export_extensions[filetype];

	// Find the root filename, from the problem or mesh file
	if (m_loaded_problem_filename[0] != '\0') {
		find_filename(default_filename, m_loaded_problem_filename, true);
	}
	else if (m_loaded_mesh_filename[0] != '\0') {
		find_filename(default_filename, m_loaded_mesh_filename, true);
	}
	else strcpy(default_filename, "deformation");

	// Append time and date
	makeTimeDateStr(export_filename, default_filename, &(ctm->m_sim_start_date_and_time));

	// Append tick information (if requested) and extension
	char tick_str[_MAX_PATH];
	if (include_simtime)
		sprintf(tick_str, "tick.%06.3f", ctm->m_current_sim_time);
	else
		tick_str[0] = '\0';

	char* extension = find_extension(export_filename);
	sprintf(extension, "%s.%s", tick_str, selected_extension);

	if (out_filename) {
		strcpy(out_filename, export_filename);
	}

	return ExportModel(ctm, export_filename,
		correct_for_transform ? &(g_main_app->current_mesh_transform) : 0,
		m_loaded_mesh_filename, &(this->m_exportHelper));

}


bool Cteschner_deformableApp::LoadModel(const char* filename,
	bool build_collision_detector, bool finalize, bool delete_old_model) {

	bool simulation_was_running = m_simulation_running;

	toggle_simulation(SIMTOGGLE_STOP);
	cTeschnerMesh* old_ctm = dynamic_cast<cTeschnerMesh*>(object);
	bool result = CwinmeshviewApp::LoadModel(filename, build_collision_detector, false, false);
	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);

	// Initialize the new model
	if (result && ctm) {
		_cprintf("Initializing model on load...\n");
		ctm->initialize(old_ctm);
	}

	if (simulation_was_running) toggle_simulation(SIMTOGGLE_START);

	// We're not really running the same problem any more
	if (result && m_loading_problem_file == false) m_loaded_problem_filename[0] = '\0';
	return result;
}

bool Cteschner_deformableApp::LoadRenderModel(const char* filename) {

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm == 0 || false == ctm->m_initialized) {
		_cprintf("Can't load render model without an initialized deformable model...\n");
	}

	cMesh* new_object = 0;
	mesh_xform_information unity_transform;
	unity_transform.model_offset.set(0, 0, 0);
	unity_transform.model_scale_factor = 1.0;

	cMesh render_factory_mesh(world);

	bool result = importModel(filename, new_object, world,
		// false,false,m_factory_mesh,0,XFORMOP_NONE,&unity_transform);
		false, false, &render_factory_mesh, 0, XFORMOP_NONE, &unity_transform);

	if (result == false) {
		_cprintf("Couldn't load render model %s\n", filename);
		return false;
	}

	// The transformations that were applied in the file
	cVector3d c;
	cVector3d m;

	c = new_object->getBoundaryCenter();
	m = ctm->getBoundaryCenter();

	_cprintf("\nCTM  COB is %s\n", c.str(2).c_str());
	_cprintf("Mesh COB is %s\n", m.str(2).c_str());

	_cprintf("Pre-transforming loaded mesh by %s %f\n", ctm->mesh_preoffset.str(2).c_str(),
		ctm->mesh_prescale);

	new_object->offsetVertices(ctm->mesh_preoffset, true);
	new_object->scale(ctm->mesh_prescale, true);

	_cprintf("Transforming loaded mesh by %s %f\n", current_mesh_transform.model_offset.str(2).c_str(),
		current_mesh_transform.model_scale_factor);

	//new_object->offsetVertices(current_mesh_transform.model_offset);
	//new_object->scale(current_mesh_transform.model_scale_factor);

	c = new_object->getBoundaryCenter();

	_cprintf("CTM  COB is %s\n", c.str(2).c_str());
	_cprintf("Mesh COB is %s\n", m.str(2).c_str());
	_cprintf("Difference is %s\n\n", (c - m).str(2).c_str());

	ctm->m_rendering_mesh = new_object;
	ctm->build_rendering_weights();

	return true;
}

int Cteschner_deformableApp::Run() {

	return CwinmeshviewApp::Run();

} // Run()



DWORD teschner_sim_thread(void* param) {

	_cprintf("Simulation thread running...\n");

	Cteschner_deformableApp* app = (Cteschner_deformableApp*)(param);

	while (app->m_simulation_running) {
		if (app->m_reset_pending) {
			app->reset_immediately();
		}

		app->simtick();

		if (app->m_render_on_gui_thread == false) {
			app->render_without_simulating();
		}
	}

	_cprintf("Simulation thread quitting...\n");

	app->sim_thread_id = -1;

	return 0;
}


void Cteschner_deformableApp::toggle_simulation(simtoggle_action action) {

	CDialog* dlg = dynamic_cast<CDialog*>(g_main_dlg);

	if (action == SIMTOGGLE_START && m_simulation_running == 1) return;
	if (action == SIMTOGGLE_STOP  && m_simulation_running == 0) return;

	if (action == SIMTOGGLE_TOGGLE) m_simulation_running = 1 - m_simulation_running;
	if (action == SIMTOGGLE_START) m_simulation_running = 1;
	if (action == SIMTOGGLE_STOP) m_simulation_running = 0;

	_cprintf("%s simulation\n", m_simulation_running ? "Starting" : "Stopping");

	if (m_multithreaded_simulation && (m_simulation_running == 1) && (sim_thread_id == -1)) {

		_cprintf("Creating simulation thread...\n");
		::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(teschner_sim_thread), this, 0, &sim_thread_id);

		// Boost thread priority over gui thread
		::SetThreadPriority(&sim_thread_id, THREAD_PRIORITY_ABOVE_NORMAL);
	}

	if (dlg) {
		CButton* button = (CButton*)(dlg->GetDlgItem(IDC_STARTSTOPSIM_BUTTON));
		button->SetWindowText(
			(m_simulation_running ? "Stop Simulation" : "Start Simulation")
		);
	}
}

float g_last_simtime_window_update = 0.0;
#define SIMTIME_UPDATE_INTERVAL 0.01

float g_last_dot_time = 0.0;
int g_dot_column = 0;
#define DOT_INTERVAL 0.05

void Cteschner_deformableApp::simtick() {

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm == 0) {
		_cprintf("No mesh available...\n");
		return;
	}

	float curtime = ctm->m_current_sim_time;
	if ((curtime < g_last_simtime_window_update)
		||
		((curtime - g_last_simtime_window_update) >
			SIMTIME_UPDATE_INTERVAL)) {
		g_last_simtime_window_update = curtime;
		char buf[100];
		sprintf(buf, "Sim Time: %06.3f", curtime);
#ifndef COMPILING_DLL
		if (g_main_dlg) {
			((Cteschner_deformableDlg*)(g_main_dlg))->m_simtime_text.SetString(buf);
			::PostMessage(((Cteschner_deformableDlg*)(g_main_dlg))->m_hWnd,
				MESSAGE_UPDATE, 0, 0);
		}
#endif
	}

	if ((curtime < g_last_dot_time)
		||
		((curtime - g_last_dot_time) > DOT_INTERVAL)) {
		g_last_dot_time = curtime;
		_cprintf(".");
		if (++g_dot_column >= 60) {
			_cprintf("\n");
			g_dot_column = 0;
		}
	}

	for (int i = 0; i < m_ticks_per_render_loop; i++) {

		// Apply forces...
		/*
		ctm->applyForce(0,cVector3f(0,-1.0,0));
		ctm->applyForce(1,cVector3f(0,-1.0,0));
		ctm->applyForce(2,cVector3f(0,-1.0,0));
		ctm->applyForce(3,cVector3f(0,-1.0,0));
		*/

		curtime = ctm->m_current_sim_time;

		if (m_periodically_outputting) {
			if (ctm->m_current_sim_time >= m_begin_periodic_output_time &&
				ctm->m_current_sim_time <= m_end_periodic_output_time) {
				if (curtime - m_last_output_time >= m_output_interval || m_last_output_time == 0.0) {
					AutoExport();

					if (m_last_output_time == 0.0) {
						_cprintf("Started periodic output...\n");
					}
					m_last_output_time = curtime;

				}
			}
			else if (ctm->m_current_sim_time > m_end_periodic_output_time) {
				m_periodically_outputting = false;
				_cprintf("Finished periodic output...\n");
			}
		}

		int control_pressed = GetAsyncKeyState(VK_CONTROL) & (1 << 15);
		int alt_pressed = GetAsyncKeyState(VK_MENU) & (1 << 15);
		int q_pressed = GetAsyncKeyState('Q') & (1 << 15);
		int p_pressed = GetAsyncKeyState('P') & (1 << 15);

		if (control_pressed && alt_pressed && p_pressed) {
			toggle_simulation(SIMTOGGLE_STOP);
			m_monitor_steady_state = false;
			m_reached_steady_state = false;
			m_steady_state_result = RESULT_MANUAL_PAUSE;
			_cprintf("Early termination (pause) requested...\n");
		}

		if (control_pressed && alt_pressed && q_pressed) {
			toggle_simulation(SIMTOGGLE_STOP);
			m_monitor_steady_state = false;
			m_reached_steady_state = false;
			m_steady_state_result = RESULT_MANUAL_TERMINATION;
			_cprintf("Early termination requested...\n");
		}

		else if (control_pressed) {
			_cprintf("Sim time: %f\n", ctm->m_current_sim_time);
		}

		float maxvel = ctm->m_maximum_vertex_velocity;
		float meanvel = fabs(ctm->m_mean_vertex_velocity);
		float maxaccel = ctm->m_maximum_vertex_acceleration;
		float meanaccel = fabs(ctm->m_mean_vertex_acceleration);

#define VERY_BIG 1e10
#define MAX_VERY_BIG_ITERATIONS 20
		static int very_big_iterations = 0;

		// How long have we been at ss?
		float time_at_ss =
			(m_steady_state_start_time < 0) ? 0.0f : (curtime - m_steady_state_start_time);

		bool steadystate = false;

		bool way_too_big = false;

		if (maxvel > VERY_BIG || meanvel > VERY_BIG || maxaccel > VERY_BIG || meanaccel > VERY_BIG) {
			very_big_iterations++;
			_cprintf("Very big: v: %f,%f a: %f,%f (max,mean) %d\n",
				maxvel, meanvel, maxaccel, meanaccel, very_big_iterations);
		}
		else {
			very_big_iterations = 0;
		}

		if (very_big_iterations > MAX_VERY_BIG_ITERATIONS) {
			way_too_big = true;
		}

		if (m_monitor_steady_state &&
			(
			(ctm->m_current_sim_time >= m_minimum_steadystate_simtime)
				||
				way_too_big
				)
			) {

			steadystate = (
				((m_max_acceleration_threshold < 0.0) || (maxaccel < m_max_acceleration_threshold))
				&&
				((m_max_velocity_threshold < 0.0) || (maxvel < m_max_velocity_threshold))
				&&
				((m_mean_acceleration_threshold < 0.0) || (meanaccel < m_mean_acceleration_threshold))
				&&
				((m_mean_velocity_threshold < 0.0) || (meanvel < m_mean_velocity_threshold))
				);

			bool quit_early = false;

			if (_isnan(maxvel) || _isnan(meanvel) || _isnan(maxaccel) || _isnan(meanaccel)) {
				_cprintf("Warning: nan in simulation computation...\n\n");
				quit_early = true;
				m_steady_state_result = RESULT_ERROR;
			}
			if (steadystate == false && ctm->m_current_sim_time > m_maximum_steadystate_simtime) {
				_cprintf("Warning: did not reach steady state by time %f...\n\n",
					m_maximum_steadystate_simtime);
				quit_early = true;
				m_steady_state_result = RESULT_NO_STEADY_STATE;
			}
			if (way_too_big) {
				_cprintf("Warning: simulation appears to have exploded...\n");
				quit_early = true;
				m_steady_state_result = RESULT_NO_STEADY_STATE;
			}

			// Early termination of our simulation
			if (quit_early) {

				if (m_stop_at_steady_state) {
					toggle_simulation(SIMTOGGLE_STOP);
				}

				if (m_exit_at_steady_state) {
					// TODO
				}

				if (m_output_at_steady_state) {
					// TODO
				}

				m_monitor_steady_state = false;
				m_reached_steady_state = false;

			} // if we failed to reach steady state...

			if (steadystate) {

				// This means we weren't previously at steady-state
				if (m_steady_state_start_time < 0) {
					m_steady_state_start_time = curtime;
				}
				else if (curtime - m_steady_state_start_time >= m_required_time_at_steady_state) {
					_cprintf("Reached steady state at time %f\n", ctm->m_current_sim_time);
					if (m_stop_at_steady_state) toggle_simulation(SIMTOGGLE_STOP);

					if (m_exit_at_steady_state) {
						// TODO
					}

					if (m_output_at_steady_state) {
						// TODO
					}

					m_monitor_steady_state = false;
					m_reached_steady_state = true;
					m_steady_state_result = RESULT_STEADY_STATE;
				}
			}
			else {
				m_steady_state_start_time = -1.0f;
			}

		} // if we're interested in reaching steady-state...

		// Print out steady-state data if necessary
		if (m_monitor_steady_state) {
			if (alt_pressed) {
				_cprintf("Values: v: %f,%f a: %f,%f (max,mean) %3.3f %d %3.3f\n",
					maxvel, meanvel, maxaccel, meanaccel,
					ctm->m_current_sim_time, (int)steadystate, time_at_ss);
				_cprintf("Thresholds: v: %f,%f a: %f,%f (max,mean)\n",
					m_max_velocity_threshold, m_mean_velocity_threshold,
					m_max_acceleration_threshold, m_mean_acceleration_threshold);
			}
		}

		if (m_simulation_running || m_single_tick) ctm->tick();
		if (m_single_tick || (!m_simulation_running)) break;

	} // for each tick in our loop

} // simtick()


void Cteschner_deformableApp::singletick() {
	m_single_tick = true;
	simtick();
	m_single_tick = false;
}

void Cteschner_deformableApp::reset_immediately() {
	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm == 0) return;
	if (m_monitor_steady_state_after_reset) {
		m_monitor_steady_state = true;
		m_reached_steady_state = false;
		m_steady_state_start_time = -1.0;
	}
	ctm->reset();
}


void Cteschner_deformableApp::reset() {

	if (sim_thread_id != -1) {
		m_reset_pending = true;
		return;
	}
	else reset_immediately();

}


void Cteschner_deformableApp::clear_constraints() {

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm == 0) return;
	else ctm->clear_constraints();

}

void Cteschner_deformableApp::render_without_simulating() {

	if (m_gui_enabled == false) {
		_cprintf("No gui for rendering...\n");
		return;
	}

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
	if (ctm == 0) {
		_cprintf("No model for rendering...\n");
		return;
	}

	ctm->prepare_to_render();
	CwinmeshviewApp::render_loop();

}

external_force mouse_force;
int g_first_force = 1;
cTriangle* forcetri = 0;

// Our main drawing / simulation loop...
int Cteschner_deformableApp::render_loop() {

	if (keys_to_handle['G']) {
		keys_to_handle['G'] = 0;
		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
		ctm->m_gravity_enabled = 1 - ctm->m_gravity_enabled;
		_cprintf("Gravity is %s\n", ctm->m_gravity_enabled ? "enabled" : "disabled");
	}

	if (m_simulation_running) {

		cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
		if (ctm == 0) {
			_cprintf("No mesh available...\n");
		}
		else {
			if (m_multithreaded_simulation == 0) {
				simtick();
			}
			if (m_gui_enabled) ctm->prepare_to_render();
		}

		// If we have a gui
		if (g_main_dlg && m_gui_enabled) {

			if (g_main_dlg->m_left_scrolling_gl_area == 0) {
				if (ctm) ctm->external_forces.clear();
			}

			else if (ctm && selected_triangle) {

				CPoint p;
				p.x = g_main_dlg->last_left_scroll_point.x - g_main_dlg->last_left_click_point.x;
				p.y = g_main_dlg->last_left_scroll_point.y - g_main_dlg->last_left_click_point.y;

				cVector3d force_vector =
					cVector3d(
					(-1.0*(float)p.y / 100.0) * camera->getUpVector() +
						(1.0*(float)p.x / 100.0) * camera->getRightVector()
					);
				force_vector *= ctm->m_external_force_scale;

				cPrecisionClock clock;
				double elapsed = clock.getCPUtime() - g_main_dlg->last_left_click_time;

				if (elapsed > 1.0) {
					float time_weight = elapsed;
					// if (time_weight > 10.0) time_weight = 10.0;
					time_weight = pow(time_weight, ctm->m_time_force_exponent);
					force_vector *= time_weight;
				}

				mouse_force.tag = 0;
				//if (g_first_force) {
				forcetri = selected_triangle;
				g_first_force = 0;
				//}

				  /*
				_cprintf("Applying force %lf to triangle %d, v's %d,%d,%d\n",
				  force_vector.length(),
				  forcetri->getIndex(),
				  forcetri->getVertexIndex(0),
				  forcetri->getVertexIndex(1),
				  forcetri->getVertexIndex(2)
				  );
				  */

				mouse_force.vertices.clear();
				mouse_force.vertices.push_back(forcetri->getVertexIndex(0));
				mouse_force.vertices.push_back(forcetri->getVertexIndex(1));
				mouse_force.vertices.push_back(forcetri->getVertexIndex(2));
				mouse_force.force = force_vector;

				cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);
				if (ctm && (ctm->external_forces.size() == 0)) {
					// ctm->external_forces[0] = (&mouse_force);
					ctm->external_forces.push_back(&mouse_force);
				}

			} // if a triangle has been selected for force application

		} // if we have a gui

	} // if we're running our simulation

	// Do the actual rendering
	if (m_gui_enabled) CwinmeshviewApp::render_loop();

	return 0;

}


int Cteschner_deformableApp::run_to_steady_state(double maxtime) {

	if (m_output_at_steady_state == false) {
		_cprintf("Warning: no steady state information received...\n");
		return false;
	}

	cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(object);

	if (ctm == 0) {
		_cprintf("No mesh to simulate...\n");
		return false;
	}

	ctm->prepare_heterogeneous_constant_rendering();

	m_reached_steady_state = false;
	toggle_simulation(SIMTOGGLE_START);
	_cprintf("Starting simulation until steady-state...\n");
	while (m_simulation_running) {
		if (maxtime >= 0.0 && ctm->m_current_sim_time >= maxtime) break;
		// simtick();
		render_loop();
	}
	_cprintf("Finished simulation (result %d)...\n", m_steady_state_result);
	return m_steady_state_result;
}
