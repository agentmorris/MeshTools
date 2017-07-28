//===========================================================================
//  - UNIT -
//
//    Copyright (C) 2002. Stanford University - Robotics Laboratory
/*!
      \author     conti@robotics.stanford.edu
      \file       CPhantom.cpp
      \version    1.0
      \date       01/2002
*/
//===========================================================================

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "CPhantom.h"
#include "math.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//===========================================================================
//  - PUBLIC METHOD -
/*!
      The class communicates to the Phantom Server program using shared
      memory. If the server is not already activated, an error message is
      displayed.

      \fn         cPhantom::cPhantom()
      \return     Return a pointer to new cPhantom instance.
*/
//===========================================================================
cPhantom::cPhantom()
{
  // CONNECTION IS READY YET:
  systemReady = false;                                   

  // OPEN MAPPED FILE
  hMapFile = OpenFileMapping(
      FILE_MAP_ALL_ACCESS,
      FALSE,
      "Phantom");

  if (hMapFile == NULL) {
    displayError();
    return;
  }

  // GET MEMORY ADDRESS:
  lpMapAddress = MapViewOfFile(
    hMapFile,
    FILE_MAP_ALL_ACCESS,
    0,
    0,
    0);

  if (lpMapAddress == NULL) {
    displayError();
    return;
  }

  // SET MEMORY:
  pDevice = (TPhantomCom*)lpMapAddress;

  // SYSTEM IS READY:
  systemReady = true;
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Destructor of cPhantom.

      \fn         cPhantom::~cPhantom()
*/
//===========================================================================
cPhantom::~cPhantom()
{
  // SET ZERO FORCE:
  setForce(0,0,0);
  // UNMAP FILE:
  UnmapViewOfFile(lpMapAddress);
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Set a Force vector to the Phantom device

      \fn         void cPhantom::setForce(const double &iForceX,
                  const double &iForceY, const double &iForceZ)

      \param      iForceX     X Component of force vector.
      \param      iForceY     Y Component of force vector.
      \param      iForceZ     Z Component of force vector.

*/
//===========================================================================
void cPhantom::setForce(const double &iForceX, const double &iForceY, const double &iForceZ)
{
  if (systemReady) {

    // CHECK FORCE MAGNITUDE:
    double forceMagnitude = sqrt(iForceX*iForceX + iForceY*iForceY + iForceZ*iForceZ);
    if (forceMagnitude > MAX_FORCE_PHANTOM)
    {
      // SCALE FORCE:
      double scale = (MAX_FORCE_PHANTOM / forceMagnitude);
      double scaleForceX = scale * iForceX;
      double scaleForceY = scale * iForceY;
      double scaleForceZ = scale * iForceZ;
      // APPLY FORCE:
      (double)(*pDevice).forceX = scaleForceY;
      (double)(*pDevice).forceY = scaleForceZ;
      (double)(*pDevice).forceZ = scaleForceX;
    }
    else
    {
      // APPLY FORCE:
      (double)(*pDevice).forceX = iForceY;
      (double)(*pDevice).forceY = iForceZ;
      (double)(*pDevice).forceZ = iForceX;
    }
  }
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Set a Torque to the Phantom device

      \fn         void cPhantom::setTorque(const double &iTorqueA,
                       const double &iTorqueB, const double &iTorqueG)

      \param      iTorqueA     Alpha torque.
      \param      iTorqueB     Beta torque.
      \param      iTorqueG     Gamma torque.

*/
//===========================================================================
void cPhantom::setTorque(const double &iTorqueA, const double &iTorqueB, const double &iTorqueG)
{
  if (systemReady) {
    (double)(*pDevice).torqueA = iTorqueA;
    (double)(*pDevice).torqueB = iTorqueB;
    (double)(*pDevice).torqueG = iTorqueG;
  }
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      return the Position [X,Y,Z] of the Phantom device

      \fn         void cPhantom::getPosition(double &iPosX,
                       double &iPosY, double &iPosZ)

      \param      iPosX     X component of position vector.
      \param      iPosY     Y component of position vector.
      \param      iPosZ     Z component of position vector.

*/
//===========================================================================
void cPhantom::getPosition(double &iPosX, double &iPosY, double &iPosZ)
{
  if (systemReady) {
    iPosX = (double)(*pDevice).posZ / 1000;
    iPosY = (double)(*pDevice).posX / 1000;
    iPosZ = (double)(*pDevice).posY / 1000;
  }
  else {
    iPosX = 0;
    iPosY = 0;
    iPosZ = 0;
  }
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      return the Angles [A,B,G] of the Phantom device.

      \fn         void cPhantom::getAngles(double &iAngleA,
                       double &iAngleB, double &iAngleG)

      \param      iAngleA     Alpha angle.
      \param      iAngleB     Beta angle.
      \param      iAngleG     Gamma angle.

*/
//===========================================================================
void cPhantom::getAngles(double &iAngleA, double &iAngleB, double &iAngleG)
{
  if (systemReady) {
    iAngleA = (double)(*pDevice).angleA;
    iAngleB = (double)(*pDevice).angleB;
    iAngleG = (double)(*pDevice).angleG;
  }
  else {
    iAngleA = 0;
    iAngleB = 0;
    iAngleG = 0;
  }
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Return the state of the button switch of the Phantom device

      \fn         cPhantom::getSwitch()
      \return     Return true if button pressed, else false.
*/
//===========================================================================
bool cPhantom::getSwitch()
{
  if (systemReady) {
    return( (bool)(*pDevice).button0);
  }
  else {
    return (false);
  }
}


//===========================================================================
//  - PUBLIC METHOD -
/*!
      Display an error message.

      \fn         cPhantom::displayError()
*/
//===========================================================================
void cPhantom::displayError()
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
    MessageBox( NULL, (const char*)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

    // FREE THE BUFFER.
    LocalFree( lpMsgBuf );
}

//===========================================================================
//  END OF FILE
//===========================================================================
