//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "CCamera.h"
#include "CViewport.h"
#include "CWorld.h"
#include "XMatrix.h"
#include "CShape.h"
//#include "CSharedMap.h"
#include <Menus.hpp>
//---------------------------------------------------------------------------

// DEFINITIONS:
struct TVirtualDelta
{
  double       ForceX;   // Force component X.
  double       ForceY;   // Force component Y.
  double       ForceZ;   // Force component Z.
  double       TorqueA;  // Torque alpha.
  double       TorqueB;  // Torque beta.
  double       TorqueG;  // Torque gamma.
  double       PosX;     // Position X.
  double       PosY;     // Position Y.
  double       PosZ;     // Position Z.
  double       AngleA;   // Angle alpha.
  double       AngleB;   // Angle beta.
  double       AngleG;   // Angle gamma.
  bool        Button0;  // Button 0 status.
  bool         AckMsg;   // Acknowledge Message
  bool         CmdReset; // Command Reset
};

class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TEdit *edtPosX;
        TEdit *edtPosY;
        TEdit *edtPosZ;
        TUpDown *upDownZ;
        TUpDown *upDownX;
        TUpDown *upDownY;
        TLabel *Label1;
        TLabel *Label2;
        TEdit *edtForceZ;
        TEdit *edtForceY;
        TEdit *edtForceX;
        TPanel *panelView;
        TProgressBar *barForce;
        TTimer *Timer1;
        TUpDown *upDownXs;
        TUpDown *upDownYs;
        TUpDown *upDownZs;
        TLabel *Label3;
        TCheckBox *ckbButton;
        TLabel *Label4;
        TLabel *Label5;
        TLabel *Label6;
        TLabel *Label7;
        TLabel *Label8;
        TLabel *Label9;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        TLabel *Label13;
        TLabel *Label14;
        TLabel *Label15;
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall panelViewMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall panelViewMouseUp(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
        void __fastcall panelViewMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
        void __fastcall upDownXClick(TObject *Sender, TUDBtnType Button);
        void __fastcall upDownYClick(TObject *Sender, TUDBtnType Button);
        void __fastcall upDownZClick(TObject *Sender, TUDBtnType Button);
        void __fastcall upDownXsClick(TObject *Sender, TUDBtnType Button);
        void __fastcall upDownYsClick(TObject *Sender, TUDBtnType Button);
        void __fastcall upDownZsClick(TObject *Sender, TUDBtnType Button);
private:	// User declarations
        // VARIABLE DECLARATIONS
        cWorld *world;          // world
        cViewport *viewport;    // viewport (display)
        cCamera *camera;        // virtual camera.
        cShapePlan *plan;       // planar shape.
        bool padActivated;
        xVector2d LastPos;
        void render();
        HANDLE hMapFile;
        LPVOID lpMapAddress;
        TVirtualDelta* pDevice;
        void displayError();
        bool ready;

        // WORKSPACE:
        double xMaxSpace;
        double xMinSpace;
        double yMaxSpace;
        double yMinSpace;
        double zMaxSpace;
        double zMinSpace;
        double forceMax;

public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
