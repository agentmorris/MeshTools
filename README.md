# Mesh manipulation tools

This repository contains two tools (WinMeshView and Voxelizer) for mesh manipulation that I developed in the course of my thesis work:

Haptics and Physical Simulation for Virtual Bone Surgery
[http://dmorris.net/publications/dmorris_thesis.pdf](http://dmorris.net/publications/dmorris_thesis.pdf)
Morris D
PhD thesis, Stanford University, August 2006

Though I don't have time to maintain these tools, folks have found WinMeshView and Voxelizer useful over the years, so I try to at least make sure they avoid "code rot" and compile in modern versions of Visual Studio.

The "deformables" project is not really a useful tool for anyone other than 2006-me, but it has the same dependencies as WinMeshView and Voxelizer, so I keep it in the same code base.

There are four projects in the "meshes" solution...


## chai3d

The core CHAI scenegraph library used by all the other projects.  Builds to chai3d\lib\msvc\chai3d_complete.lib.  

This is an old version of the library; the current version lives at:

[http://chai3d.org/](http://chai3d.org/)

Numerous CHAI examples build from chai3d\examples\msvc\msvc_chai_examples.sln to chai3d\bin.  They refer to resources files that live in chai3d\bin\resources.


## winmeshview

winmeshview is a simple (and free) viewer/converter for 3d meshes, including surface meshes in .ply (ascii or binary), .3ds, .face/.node, .smesh, and .obj format, and tetrahedral meshes in the format (.ele/.node) generated by TetGen (a popular package for generating tetrahedral meshes) and the Abaqus input file format. At the time I'm writing this .html file, TetView - the viewer developed to complement TetGen - is not available for Windows, so winmeshview seemed like a useful thing to make available.

It's also useful as a quick viewer for standard 3d surface meshes; I find that I often download a mesh (or generate a mesh with TetGen) and have to open up a giant slow edtior/viewer just to quickly see what the deal is with the mesh. winmeshview loads whatever mesh you put on the command line, so it's easy to tell Windows to open supported file types with winmeshview, and it generally loads pretty quickly.

winmeshview reads meshes in .ply, .3ds, .face, .obj, and .smesh format, and writes meshes in .smesh, .face/.node, Abaqus .inp, and .obj format. It can also export to a format referred to as ".anode", which is the equivalent of a TetGen node file with only a reference to an element file.

Includes chai3d_complete.lib via relative paths.

Also see:

[http://dmorris.net/projects/winmeshview/](http://dmorris.net/projects/winmeshview/)


## voxelizer

Voxelizer floodfills a surface mesh (which must be very nearly watertight) to produce a voxel array, optionally computing a distance field in the process (the distance from each voxel to the surface and the corresponding closest surface point). The program can also compute shortest distances for each voxel to a series of additional meshes (up to 5). It can load models in 3ds, obj, and ply formats, and the .node/.face format used by TetGen. The output format is a ".voxels" file, which is a binary format described in voxel_file_format.h . The program can also output tetrahedral meshes in TetGen format; the floodfilling process is identical to that used for voxelization, and each voxel is simply split into five tets (so the mesh is not conformal). Voxelization can be initiated interactively or from an .ini file. A Matlab script is included to read the resulting files into Matlab.

Also see:

[http://dmorris.net/projects/voxelizer/](http://dmorris.net/projects/voxelizer/)


## deformables

My thesis work focused on calibrating a “fast” deformable model, suitable for interactive simulation but not based on real physics and not super-accurate, to optimally match the behavior of a “slow” deformable model, too slow for interactive simulation but accurate and physically based. The physical model I used was a commercial FEM package (Abaqus). This page hosts the code I wrote to implement the non-physically-based model, based directly on the work of Teschner et. al:

Teschner, M., Heidelberger, B., Muller, M., & Gross, M. (2004, June). A versatile and robust model for geometrically complex deformable solids. In Computer Graphics International, 2004. Proceedings (pp. 312-319). IEEE.

A concise description of my implementation can be found in:

Morris, D., & Salisbury, K. (2008). Automatic preparation, calibration, and simulation of deformable objects. Computer Methods In Biomechanics And Biomedical Engineering, 11(3), 263-279. (pdf)

The code is a relatively standalone mesh class that plugs into the CHAI framework.

[http://dmorris.net/projects/deformables](http://dmorris.net/projects/deformables)