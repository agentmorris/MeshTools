#include "StdAfx.h"
#include "teschner_deformable_matlab.h"
#include "teschner_deformable_globals.h"
#include "teschner_deformable.h"
#include "cTeschnerMesh.h"
#include "meshExporter.h"
#include "mex.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 50000

int g_console_allocated = 0;

/////////////////////////////////////////////////////////////////////////////
// Cleanup function to be called at exit
/////////////////////////////////////////////////////////////////////////////

void atexit() {

  Cteschner_deformableApp* app = (Cteschner_deformableApp*)(g_main_app);
  if (app) {
    app->enableGUI(false);
    delete app;    
  }
  g_main_app = 0;

  if (g_console_allocated) {
    // FreeConsole();
    g_console_allocated = 0;
  }
}


void mex_flush() {

  // This is so lame, but this is apparently the recommended way to force
  // matlab to flush I/O...
  mexEvalString("pause(.001);");

}


// Fills in the status array to return to the caller
void fill_status(int nlhs, mxArray* plhs[], int status) {
  if (nlhs < 1) return;
  plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
  double* out_data = (double *)mxGetPr(plhs[0]);
  out_data[0] = (double)status;
}

int g_nlhs;
mxArray** g_plhs;

void error_status(char* errorstr, int status=-1) {
  fill_status(g_nlhs,g_plhs,status);
  mexPrintf("%s\n",errorstr);
  mex_flush();
  return;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only mexFunction
/////////////////////////////////////////////////////////////////////////////

extern "C" void mexFunction(
                            int nlhs,              // Number of left hand side (output) arguments
                            mxArray *plhs[],       // Array of left hand side arguments
                            int nrhs,              // Number of right hand side (input) arguments
                            const mxArray *prhs[]) // Array of right hand side arguments
{

  AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

  g_nlhs = nlhs;
  g_plhs = plhs;

  Cteschner_deformableApp* app = (Cteschner_deformableApp*)(g_main_app);
  cTeschnerMesh* ctm = dynamic_cast<cTeschnerMesh*>(app->object);

  // assign the atexit function to close the library and deallocate everything if
  // the mex DLL is closed for any reason before the 'close' command is called
  mexAtExit(atexit);       

#ifdef _DEBUG
  if (g_console_allocated == 0) {
    _cprintf("Allocating console from mexfunction...\n");
    AllocConsole();
    g_console_allocated = 1;
  }
#endif

  // test for minimum number of arguments
  if (nrhs < 1) return error_status("At least one argument is required.");

  // get the command string and process the command
  char cmdstr[255];
  if (mxGetString(prhs[0],cmdstr,255))
    return error_status("The first parameter must be a valid command label...");

  if (strncmp(cmdstr,"open",4)==0) {
    return error_status("Teschner-deformable Matlab interface initialized...",0);    
  }

  else if (strncmp(cmdstr,"enable_gui",strlen("enable_gui"))==0) {
    if (nrhs < 2)
      return error_status("Need to supply one argument (1 or 0) for enable_gui...");
    
    if (mxIsDouble(prhs[1]) == 0)
      return error_status("Illegal parameter specification (not a double matrix)...");

    bool enable = (bool)(mxGetScalar(prhs[1]));
    app->enableGUI(enable);

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"enable_multithreading",strlen("enable_multithreading"))==0) {
    if (nrhs < 2)
      return error_status("Need to supply one argument (1 or 0) for enable_multithreading...");
      
    if (mxIsDouble(prhs[1]) == 0)
      return error_status("Illegal parameter specification (not a double matrix)...");

    bool enable = (bool)(mxGetScalar(prhs[1]));
    app->enableMultithreadedSimulation(enable);
    if (enable) app->m_render_on_gui_thread = false;
    else app->m_render_on_gui_thread = true;

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"enable_simulation",strlen("enable_simulation"))==0) {
    if (nrhs < 2)
      return error_status("Need to supply one argument (1 or 0) for enable_simulation...");
    
    if (mxIsDouble(prhs[1]) == 0) 
      return error_status("Illegal parameter specification (not a double matrix)...");

    bool start = (bool)(mxGetScalar(prhs[1]));
    app->toggle_simulation(start?SIMTOGGLE_START:SIMTOGGLE_STOP);
    
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"command",strlen("command"))==0) {
    if (nrhs < 2) 
      return error_status("Need to supply one argument (a command string) for command...");
    
    if (mxIsChar(prhs[1]) == 0) 
      return error_status("Illegal parameter specification (not a char array)...");
    
    char command[MAX_COMMAND_LENGTH];
    mxGetString(prhs[1],command,MAX_COMMAND_LENGTH);

    constraint_set* constraints = 0;
    external_material_properties material_props;
    if (ctm) constraints = ctm->m_constraints;
    app->ParseProblemFileCommand(command,-1,constraints,ctm?&(ctm->material_props):&(material_props));

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"renderloop",strlen("renderloop"))==0) {   
    app->render_loop();
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"simtick",strlen("simtick"))==0) {   
    app->simtick();
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"force_render_tet_mesh",strlen("force_render_tet_mesh"))==0) {
    if (nrhs < 2)
      return error_status("Need to supply one argument (1 or 0) for force_render_tet_mesh...");

    if (mxIsDouble(prhs[1]) == 0)
      return error_status("Illegal parameter specification (not a double matrix)...");

    bool force_render_tet_mesh = (bool)(mxGetScalar(prhs[1]));

    if (ctm) ctm->m_force_render_tet_mesh = force_render_tet_mesh;
    
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"disable_skinned_mesh",strlen("disable_skinned_mesh"))==0) {
    if (nrhs < 2)
      return error_status("Need to supply one argument (1 or 0) for disable_skinned_mesh...");

    if (mxIsDouble(prhs[1]) == 0)
      return error_status("Illegal parameter specification (not a double matrix)...");

    bool disable_skinned_mesh = (bool)(mxGetScalar(prhs[1]));

    if (ctm) {
      ctm->m_disable_skinned_mesh = disable_skinned_mesh;
      if (ctm->m_rendering_mesh) {
        ctm->m_rendering_mesh->setShow(disable_skinned_mesh?0:1);
      }
    }

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"render",strlen("render"))==0) {   
    app->render_without_simulating();
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"reset",strlen("reset"))==0) {   
    app->reset();
    _cprintf("Reset simulation...\n");
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"clear_constraints",strlen("clear_constraints"))==0) {   
    app->clear_constraints();
    _cprintf("Cleared constraints...\n");
    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"highlight_vertices",strlen("highlight_vertices"))==0) {   
    if (ctm == 0) return fill_status(nlhs,plhs,0);
    if (nrhs < 2) return error_status("Need to supply a vertex list...");

    double* vertices = mxGetPr(prhs[1]);
    unsigned int n = mxGetNumberOfElements(prhs[1]);
  
    double* colors = 0;
    if (nrhs >= 3) {
      colors = mxGetPr(prhs[2]);
      if (n != mxGetNumberOfElements(prhs[2])) 
        return error_status("Color list and vertex list need to be the same size...");
    }

    for(unsigned int i=0; i<n; i++) {
      int index = (int)(vertices[i]);
      if (index == -1) {
        ctm->m_highlighted_vertices.clear();
        ctm->m_highlight_colors.clear();
      }
      else {
        ctm->m_highlighted_vertices.push_back(index);
        if (colors) ctm->m_highlight_colors.push_back((int)(colors[i]));
      }
    }

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"solid_section",strlen("solid_section"))==0) {

    if (ctm == 0) return fill_status(nlhs,plhs,0);

    if (nrhs == 2) {
      if (mxIsChar(prhs[1]) == 0) 
        return error_status("Illegal solid section qualifier(not a char array)...");

      char qualifier[1000];
      mxGetString(prhs[1],qualifier,1000);

      if (strcmp(qualifier,"clear")==0) {
        _cprintf("Clearing solid sections...\n");

        ctm->clear_solid_sections();
        return fill_status(nlhs,plhs,0);
      }

    } // if we only have one additional argument

    if (nrhs < 4) return error_status("Need to supply an element list and material props...");

    solid_section* ssp = new solid_section;

    double youngs_modulus = mxGetScalar(prhs[1]);
    double poisson_coeff =  mxGetScalar(prhs[2]);

    ssp->props.youngs_modulus = youngs_modulus;
    ssp->props.poisson_coeff = poisson_coeff;

    double* elements = mxGetPr(prhs[3]);
    unsigned int n = mxGetNumberOfElements(prhs[3]);

    std::list<unsigned int> tets;

    for(unsigned int i=0; i<n; i++) {
      ssp->tets.push_back(elements[i]);
    }

    ctm->solid_sections.push_back(ssp);

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"load_problem_file",strlen("load_problem_file"))==0) {   
    if (nrhs < 2)
      return error_status("Need to supply a problem file name...");
    
    if (mxIsChar(prhs[1]) == 0) 
      return error_status("Illegal problem file name (not a char array)...");
    
    char fname[1000];
    mxGetString(prhs[1],fname,1000);
    int status = app->LoadProblemFile(fname)?0:-1;
    return fill_status(nlhs,plhs,status);
  }

  else if (strncmp(cmdstr,"export_mesh_file",strlen("export_mesh_file"))==0) {

    bool auto_export = true;
    char fname[1000];
    
    bool correct_for_transform = false;

    if (nrhs >= 2) {
      if (mxIsChar(prhs[1]) == 0) 
        return error_status("Illegal export file name (not a char array)...");

      auto_export = false;
      mxGetString(prhs[1],fname,1000);
      ::ExportModel(app->object,fname,
        correct_for_transform?&(app->current_mesh_transform):0
        ,app->m_loaded_mesh_filename);
    }

    else {
      app->AutoExport(fname,correct_for_transform);
    }

    fill_status(nlhs,plhs,0);

    if (auto_export && nlhs >= 2) {
      plhs[1] = mxCreateString(fname);
    }

    return;
  }

  else if (strncmp(cmdstr,"export_abaqus_file",strlen("export_abaqus_file"))==0) {

    bool auto_export = true;
    char fname[1000];

    bool correct_for_transform = false;

    if (nrhs >= 2) {
      if (mxIsChar(prhs[1]) == 0) 
        return error_status("Illegal export inp file name (not a char array)...");

      auto_export = false;
      mxGetString(prhs[1],fname,1000);
      ::ExportModel(app->object,fname,
        correct_for_transform?&(app->current_mesh_transform):0,
        app->m_loaded_mesh_filename,&(app->m_exportHelper));
    }

    else {
      app->AutoExport(FILETYPE_ABAQUS_INP,fname,false,correct_for_transform);
    }

    fill_status(nlhs,plhs,0);

    if (auto_export && nlhs >= 2) {
      plhs[1] = mxCreateString(fname);
    }

    return;
  }
  else if (strncmp(cmdstr,"run_to_steady_state",strlen("run_to_steady_state"))==0) {

    double maxtime = -1.0;
    if (nrhs >= 2 && mxIsDouble(prhs[1])) maxtime = mxGetScalar(prhs[1]);

    int drop_priority = 1;
    if (nrhs >= 3 && mxIsDouble(prhs[2])) drop_priority = (int)(mxGetScalar(prhs[2]));

    int old_priority;
    DWORD tid;

    if (drop_priority) {   
      _cprintf("Dropping thread priority...\n");
      HANDLE h = GetCurrentThread();
      old_priority = GetThreadPriority(h);
      tid = GetCurrentThreadId();
      ::SetThreadPriority(&tid, THREAD_PRIORITY_BELOW_NORMAL);
    }

    int result = app->run_to_steady_state(maxtime);

    if (nlhs >= 2) {
      plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
      *mxGetPr(plhs[1]) = (double)result;
    }

    if (drop_priority) {   
      ::SetThreadPriority(&tid, old_priority);
    }

    return fill_status(nlhs,plhs,0);

  }

  else if (strncmp(cmdstr,"prepare_heterogeneous_constant_rendering",strlen("prepare_heterogeneous_constant_rendering"))==0) {
    ctm->prepare_heterogeneous_constant_rendering();
  }

  else if (strncmp(cmdstr,"get_sim_time",strlen("get_sim_time"))==0) {

    if (nlhs >= 2) {
      plhs[1] = mxCreateDoubleMatrix(1, 1, mxREAL);
      double t = (ctm)?ctm->m_current_sim_time:0.0;
      *mxGetPr(plhs[1]) = t;
    }

    return fill_status(nlhs,plhs,0);
  }

  else if (strncmp(cmdstr,"get_current_mesh",strlen("get_current_mesh"))==0) {

    if (nlhs < 2) {
      return error_status("Need an output variable for mesh data...");
    }
    
    if (ctm == 0) {
      return error_status("No current mesh...");
    }
    // Find out how big our mesh is (only level-0 vertices)
    unsigned int nvertices = ctm->m_nVertices;

    // This is the number of _rendered_ triangles (only level-0 faces)
    unsigned int nfaces = ctm->getNumTriangles(false);
    unsigned int ntets = ctm->m_nTets;

    // Create the output matlab struct
    int dims[2] = {1, 1};
    const char *field_names[] = {"vertices", "faces", "tets"};

    plhs[1] = mxCreateStructArray(2, dims, 3, field_names);
    int vertices_field = mxGetFieldNumber(plhs[1],"vertices");
    int faces_field = mxGetFieldNumber(plhs[1],"faces");
    int tets_field = mxGetFieldNumber(plhs[1],"tets");

    // Create the arrays to hold the actual data
    mxArray *vertices_mx = mxCreateDoubleMatrix(nvertices,3,mxREAL);
    mxArray *faces_mx = mxCreateDoubleMatrix(nfaces,3,mxREAL);
    mxArray *tets_mx = mxCreateDoubleMatrix(ntets,4,mxREAL);

    // Get the output data pointers
    double* vertices_data = (double*)mxGetPr(vertices_mx);
    double* faces_data = (double*)mxGetPr(faces_mx);
    double* tets_data = (double*)mxGetPr(tets_mx);

    // Grab the actual data
    unsigned int i,k;

    for(i=0; i<nvertices; i++) {
      cDeformableVertex* v= ctm->m_deformableVertices+i;
      // Rotate to matlab indexing
      for(k=0; k<3; k++) vertices_data[k*nvertices+i] = v->m_pos[k];
    }

    for(i=0; i<ntets; i++) {
      unsigned int* v= ctm->m_tets+i*4;
      // Rotate to matlab indexing and offset content to matlab indexing
      for(k=0; k<4; k++) tets_data[k*ntets+i] = v[k]+1;
    }

    // Access only _rendered_ triangles
    for(i=0; i<nfaces; i++) {
      cTriangle* t = ctm->getTriangle(i,true);
      // Rotate and offset to matlab indexing
      for(k=0; k<3; k++) faces_data[k*nfaces+i] = t->getVertexIndex(k) + 1;
    }

    // Assign them to the output fields
    mxSetFieldByNumber(plhs[1],0,vertices_field,vertices_mx);
    mxSetFieldByNumber(plhs[1],0,faces_field,faces_mx);
    mxSetFieldByNumber(plhs[1],0,tets_field,tets_mx);

    return fill_status(nlhs,plhs,0);
  }

  else {
    mexErrMsgTxt("Unrecognized teschner_deformable command");
  }
  
}