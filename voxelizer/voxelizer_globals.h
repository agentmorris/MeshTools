#ifndef _voxelizer_GLOBALS_H_
#define _voxelizer_GLOBALS_H_

class CvoxelizerDlg;
class CvoxelizerApp;

// This makes sure that only the one file who defines
// ALLOCATE_SCOPED_GLOBALS actually creates space for
// all the global variables in the binary; everyone else
// externs them...
#ifdef ALLOCATE_SCOPED_GLOBALS
#define SCOPE
#else 
#define SCOPE extern
#endif

SCOPE CvoxelizerDlg* g_main_dlg;
SCOPE CvoxelizerApp* g_main_app;

// The resolution along each axis in the anisotropic case, or the resolution
// along the longest axis in the isotropic case
#define DEFAULT_VOXEL_RESOLUTION 20 // 100

// The amount to multiply that triangle's normal by to find the seed point
//
// Should generally be negative, unless you know that your model has funny-facing
// triangles.
#define DEFAULT_NORMAL_MULTIPLIER -.001

#endif

