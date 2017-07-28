#if !defined(AFX_teschner_deformable_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
#define AFX_teschner_deformable_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "winmeshview.h"
#include "constraints.h"
#include "meshExporter.h"
#include "cTeschnerMesh.h"

typedef enum {
  SIMTOGGLE_TOGGLE=0, SIMTOGGLE_START, SIMTOGGLE_STOP
} simtoggle_action;

typedef enum {
  RESULT_NOT_DONE_YET=0,RESULT_STEADY_STATE,RESULT_NO_STEADY_STATE,
  RESULT_ERROR,RESULT_MANUAL_TERMINATION,RESULT_MANUAL_PAUSE
} steady_state_result;

class Cteschner_deformableApp;

class Cteschner_deformableExportHelper : public ExportHelper {
public:

  virtual void preExport(FILE* f, int filetype);
  virtual void postExport(FILE* f, int filetype);

  Cteschner_deformableExportHelper(Cteschner_deformableApp* app) {
    myApp = app;
  }

  Cteschner_deformableApp* myApp;
};

class Cteschner_deformableApp : public CwinmeshviewApp {
public:

	Cteschner_deformableApp();

  void problem_defaults();

  Cteschner_deformableExportHelper m_exportHelper;

  // Look for a file defining our default model, or create one
  // if we can't find the file.  Only called if no model was
  // specified at the command line.
  virtual bool createDefaultModel();

  // Parse command-line parameters, invoked by the superclass at
  // startup
  virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

  // Handles mouse-scroll events (moves or rotates the selected object)
  //
  // In general, I'll leave most of the work to my parent class here.
  virtual void scroll(CPoint p, int button = MOUSE_BUTTON_LEFT);

  // Handles mouse clicks (marks the front-most clicked object as 'selected')
  virtual void select(CPoint p);

  bool m_gui_enabled;
  bool m_reset_pending;

  char m_loaded_problem_filename[_MAX_PATH];

  cMesh* last_selected_triangle_highlight;
  cMesh* floor;

  std::vector<float> step_end_times;

  // Explicitly build a default model (a single tet)
  virtual bool createTet();

  bool m_simulation_running;
  bool m_single_tick;
  bool m_multithreaded_simulation;
  DWORD sim_thread_id;
  int m_ticks_per_render_loop;
  
  // Periodic output variables
  bool  m_periodically_outputting;
  float m_output_interval;
  float m_begin_periodic_output_time;
  float m_end_periodic_output_time;
  float m_last_output_time;

  bool  m_monitor_steady_state;
  bool  m_monitor_steady_state_after_reset;
  bool  m_output_at_steady_state;
  bool  m_stop_at_steady_state;
  bool  m_exit_at_steady_state;

  float m_minimum_steadystate_simtime;
  float m_maximum_steadystate_simtime;

  float m_required_time_at_steady_state;
  float m_max_acceleration_threshold;
  float m_max_velocity_threshold;
  float m_mean_acceleration_threshold;
  float m_mean_velocity_threshold;
  
  float m_steady_state_start_time;
  bool  m_reached_steady_state;
  int   m_steady_state_result;

  // Override this to prevent finalization and deleting
  virtual bool LoadModel(const char* filename,
    bool build_collision_detector=false,
    bool finalize=false, bool delete_old_model=false);

  // Override this to prevent finalization and deleting
  virtual bool LoadRenderModel(const char* filename);

  // Load a deformation problem file
  bool m_loading_problem_file;
  
  // Features accessible from Matlab
  virtual void enableGUI(bool enable);
  virtual bool LoadProblemFile(const char* in_filename);
  virtual bool ParseProblemFileCommand(const char* line, int cur_line_number, constraint_set* constraints, external_material_properties* material_props);
  virtual void toggle_simulation(simtoggle_action action);
  virtual void enableMultithreadedSimulation(bool enable) { m_multithreaded_simulation = enable; }
  virtual void simtick();
  virtual void singletick();
  virtual int  render_loop(); 
  virtual void reset_immediately();
  virtual void reset();
  virtual void clear_constraints();
  virtual void render_without_simulating();
  virtual int run_to_steady_state(double maxtime = -1.0);
  virtual bool AutoExport(char* out_filename=0, bool correct_for_transform=true);
  virtual bool AutoExport(int filetype, char* out_filename=0, bool include_simtime=true, bool correct_for_transform=true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cteschner_deformableApp)
	public:
	virtual BOOL InitInstance();
  virtual int ExitInstance();
  virtual int Run();
  //}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(Cteschner_deformableApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_teschner_deformable_H__0F50D983_2916_4586_800E_8C934B7EEE55__INCLUDED_)
