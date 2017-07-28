//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include <float.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
    ready = false;

    _control87(MCW_EM,MCW_EM);
    
    // SET DEVICE PARAMETERS:
    xMaxSpace = 0.2;
    xMinSpace = -0.2;
    yMaxSpace = 0.2;
    yMinSpace = -0.2;
    zMaxSpace = 0.2;
    zMinSpace = -0.2;
    forceMax  = 25.0;

    // CREATE FILE TO BE MAPPED:
    hMapFile = CreateFileMapping(
       (HANDLE)0xFFFFFFFF,                  // Current file handle.
        NULL,                               // Default security.
        PAGE_READWRITE | SEC_COMMIT,        // Read/write permission.
        0,                                  // Max. object size.
        1024,                               // Size of hFile.
        "dhdVirtual");                      // Name of mapping object.

    if (hMapFile == NULL) {
      displayError();
	  exit(-1);
	}

    if (GetLastError() != 0)
    {
      displayError();
      exit(-1);
    }

    // GET ADDRESS:
    lpMapAddress = MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0);

    if (lpMapAddress == NULL) {
      displayError();
	  exit(-1);
	}

     // SET MEMORY:
    pDevice = (TVirtualDelta*)lpMapAddress;


 // CREATE A WORLD:
  world = new cWorld();

  // SET BACKGROUND COLOR OF WORLD:
  world->backgroundColor = xSetColor4f(0.0,0.0,0.0);  // black

  // SET LIGHTING:
  world->useLighting = true;
  world->lights[0]->enabled = true;
  world->lights[0]->pos = xSet(3,3,3);
  world->lights[0]->dir = xSet(-1,-1,-1);

  // CREATE AND POSITION CAMERA:
  camera  = new cCamera();

  camera->set( xSet(0.8, 0.0, 0.30),   // Position of camera.
               xSet(0.0, 0.0, -0.1),   // Camera looking at origin.
               xSet(0.0, 0.0, 1.0));  // Orientation of camera. (standing up.)

  // CREATE A VIEWPORT TO DISPLAY THE 3D IMAGE:
  viewport = new cViewport(panelView);   // Pass the Panel name (panelView) as
                                         // a parameter to inform OpenGL where
                                         // to render the scene.

  // CONNECT VIEWPORT TO THE WORLD:
  viewport->world = world;

  // SELECT A VIRTUAL CAMERA INSIDE THE WORLD:
  viewport->camera = camera;

  // DISABLE THE VIRTUAL FINGER:
  world->finger->enable = true;
  world->finger->radius = 0.01;
  world->finger->pos = xSet(0, 0, 0);

  ready = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
  if (ready)
  {
    render();
  }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::panelViewMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  padActivated = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::panelViewMouseUp(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  padActivated = false;
  LastPos = xSet(X,Y);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::panelViewMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  // UPDATE POSITION:
  double K = 0.001;
  xVector2d NextPos = xSet(X,Y);
  xVector2d DeltaPos = xSub(NextPos,LastPos);
  LastPos = NextPos;
  if (padActivated)
  {
    if (Shift.Contains(ssShift) || Shift.Contains(ssRight))
    {
      xVector3d delta = xSet(K*DeltaPos.y, K*DeltaPos.x, 0);
      world->finger->pos = xAdd(world->finger->pos, delta);
    }
    else
    {
      xVector3d delta = xSet(0,K*DeltaPos.x, -K*DeltaPos.y);
      world->finger->pos = xAdd(world->finger->pos, delta);
    }
  }

  // LIMIT SIDES:
  xVector3d pos = world->finger->pos;
  if (pos.x > xMaxSpace) { pos.x = xMaxSpace; }
  if (pos.y > yMaxSpace) { pos.y = yMaxSpace; }
  if (pos.z > zMaxSpace) { pos.z = zMaxSpace; }
  if (pos.x < xMinSpace) { pos.x = xMinSpace; }
  if (pos.y < yMinSpace) { pos.y = yMinSpace; }
  if (pos.z < zMinSpace) { pos.z = zMinSpace; }
  world->finger->pos = pos;

  // RENDER:
  render();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::upDownXClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == btNext)
  {
    xVector3d pos = world->finger->pos;
    pos.x = pos.x + 0.01;
    world->finger->pos = pos;
  }
  else if (Button == btPrev)
  {
    xVector3d pos = world->finger->pos;
    pos.x = pos.x - 0.01;
    world->finger->pos = pos;
  }
  // RENDER:
  render();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::upDownYClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == btNext)
  {
    xVector3d pos = world->finger->pos;
    pos.y = pos.y + 0.01;
    world->finger->pos = pos;
  }
  else if (Button == btPrev)
  {
    xVector3d pos = world->finger->pos;
    pos.y = pos.y - 0.01;
    world->finger->pos = pos;
  }
  // RENDER:
  render();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::upDownZClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == btNext)
  {
    xVector3d pos = world->finger->pos;
    pos.z = pos.z + 0.01;
    world->finger->pos = pos;
  }
  else if (Button == btPrev)
  {
    xVector3d pos = world->finger->pos;
    pos.z = pos.z - 0.01;
    world->finger->pos = pos;
  }
  // RENDER:
  render();
}

void TForm1::render()
{
  if ((bool)(*pDevice).CmdReset == true)
  {
    world->finger->pos = xSet(0,0,0);
    (bool)(*pDevice).CmdReset = false;
  }

  // LIMIT SIDES:
  xVector3d pos = world->finger->pos;
  if (pos.x > xMaxSpace) { pos.x = xMaxSpace; }
  if (pos.y > yMaxSpace) { pos.y = yMaxSpace; }
  if (pos.z > zMaxSpace) { pos.z = zMaxSpace; }
  if (pos.x < xMinSpace) { pos.x = xMinSpace; }
  if (pos.y < yMinSpace) { pos.y = yMinSpace; }
  if (pos.z < zMinSpace) { pos.z = zMinSpace; }
  world->finger->pos = pos;

  pos = world->finger->pos;
  (double)(*pDevice).PosX = pos.x;
  (double)(*pDevice).PosY = pos.y;
  (double)(*pDevice).PosZ = pos.z;

  xVector3d force;
  force.x = (double)(*pDevice).ForceX;
  force.y = (double)(*pDevice).ForceY;
  force.z = (double)(*pDevice).ForceZ;

  if (xNorm(force) > forceMax)
  {
    xVector3d tempForce = xMul( (forceMax/xNorm(force)), force);
    force = tempForce;
  }

  (bool)(*pDevice).Button0 = ckbButton->Checked;

  // RENDER:
  viewport->render();
  edtPosX->Text = xStr(world->finger->pos.x, 3);
  edtPosY->Text = xStr(world->finger->pos.y, 3);
  edtPosZ->Text = xStr(world->finger->pos.z, 3);
  edtForceX->Text = xStr(force.x, 3);
  edtForceY->Text = xStr(force.y, 3);
  edtForceZ->Text = xStr(force.z, 3);
  barForce->Position = (100.0f/forceMax)*xNorm(world->finger->force);

  world->finger->force = xMul(0.15,force);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::upDownXsClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == btNext)
  {
    xVector3d pos = world->finger->pos;
    pos.x = pos.x + 0.001;
    world->finger->pos = pos;
  }
  else if (Button == btPrev)
  {
    xVector3d pos = world->finger->pos;
    pos.x = pos.x - 0.001;
    world->finger->pos = pos;
  }
  // RENDER:
  render();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::upDownYsClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == btNext)
  {
    xVector3d pos = world->finger->pos;
    pos.y = pos.y + 0.001;
    world->finger->pos = pos;
  }
  else if (Button == btPrev)
  {
    xVector3d pos = world->finger->pos;
    pos.y = pos.y - 0.001;
    world->finger->pos = pos;
  }
  // RENDER:
  render();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::upDownZsClick(TObject *Sender, TUDBtnType Button)
{
  if (Button == btNext)
  {
    xVector3d pos = world->finger->pos;
    pos.z = pos.z + 0.001;
    world->finger->pos = pos;
  }
  else if (Button == btPrev)
  {
    xVector3d pos = world->finger->pos;
    pos.z = pos.z - 0.001;
    world->finger->pos = pos;
  }
  // RENDER:
  render();
}
//---------------------------------------------------------------------------

void TForm1::displayError()
{
    LPVOID lpMsgBuf;

    // SET THE FORMAT MESSAGE:
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );

    // DISPPLAY THE ERROR MESSAGE STRING.
   // MessageBox( NULL, (const char*)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );
    MessageBox( NULL, "A new Virtual Device could not be started.", "DHD Error", MB_OK|MB_ICONINFORMATION );

    // FREE THE BUFFER.
    LocalFree( lpMsgBuf );
}


