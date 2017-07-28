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
#ifndef CPhantomH
#define CPhantomH
//---------------------------------------------------------------------------

  struct TPhantomCom
  {
    double       forceX;   // Force component X.
    double       forceY;   // Force component Y.
    double       forceZ;   // Force component Z.
    double       torqueA;  // Torque alpha.
    double       torqueB;  // Torque beta.
    double       torqueG;  // Torque gamma.
    double       posX;     // Position X.
    double       posY;     // Position Y.
    double       posZ;     // Position Z.
    double       angleA;   // Angle alpha.
    double       angleB;   // Angle beta.
    double       angleG;   // Angle gamma.
    bool         button0;  // Button 0 status.
  };

  // MAXIMUM FORCE THAT CAN BE APPLIED TO DEVICE:
  const float MAX_FORCE_PHANTOM = 5.8;


//===========================================================================
//  - CLASS DEFINITION -
/*!
      \class      cPhantom
      \brief      cPhantom implements a connection to a Phantom device.
*/
//===========================================================================
class cPhantom
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cPhantom.
    cPhantom();
    //! Destructor of cPhantom.
    ~cPhantom();
    //! Return True is sytem is ready and can communicate with device.
    bool systemReady;

    // METHODS:
    //! Set force.
    void setForce(const double &iForceX, const double &iForceY, const double &iForceZ);
    //! Set torque.
    void setTorque(const double &iTorqueA, const double &iTorqueB, const double &iTorqueG);
    //! Get position of device.
    void getPosition(double &iPosX, double &iPosY, double &iPosZ);
    //! Get orientation of gimble.
    void getAngles(double &iAngleA, double &iAngleB, double &iAngleG);
    //! Get switch status (on/off).
    bool getSwitch();

  private:
    void displayError();

    HANDLE hMapFile;
    LPVOID lpMapAddress;
    TPhantomCom* pDevice;
};

#endif


//===========================================================================
//  END OF FILE
//===========================================================================
