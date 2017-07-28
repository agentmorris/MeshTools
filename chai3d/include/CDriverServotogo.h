//===========================================================================
/*
    This file is part of the CHAI 3D visualization and haptics libraries.
    Copyright (C) 2003-2004 by CHAI 3D. All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License("GPL") version 2
    as published by the Free Software Foundation.

    For using the CHAI 3D libraries with software that can not be combined
    with the GNU GPL, and for taking advantage of the additional benefits
    of our support services, please contact CHAI 3D about acquiring a
    Professional Edition License.

    \author:    <http://www.chai3d.org>
    \author:    Federico Barbagli
    \version    1.2
    \date       01/2004
*/
//===========================================================================

//---------------------------------------------------------------------------
#ifndef CDriverServotogoH
#define CDriverServotogoH
//---------------------------------------------------------------------------
#include "DLPORTIO.H"
#include "CGenericDevice.h"
//---------------------------------------------------------------------------

typedef union
{
    unsigned long                       all;
    struct {unsigned char  A, B, C, D;} port;
} IO32;

typedef union
{
    long           Long;
    unsigned char  Byte[4];
} LONGBYTE;

typedef union
{
    unsigned short                          Word;
    struct   {unsigned char    high, low;}  Byte;
} WORDBYTE;


//===========================================================================
/*!
      \file       CDriverServotogo.h
      \class      cDriverServotogo
      \brief      cDriverServotogo offers an interface to the Servo2Go boards.
*/
//===========================================================================
class cDriverServotogo : public cGenericDevice
{
  public:
    // CONSTRUCTOR & DESTRUCTOR:
    //! Constructor of cDriverServotogo.
    cDriverServotogo();
    //! Destructor of cVirtualTool.
    ~cDriverServotogo();

    // METHODS:
    //! Open connection to Sensoray626 board
    virtual int open();

    //! Close connection to Sensoray626 board
    virtual int close();

    //! Initialize Sensoray626 board
    virtual int initialize(const bool a_resetEncoders=false);

    //! Send a command to the Servo2Go board
    //! possible commands are:
    //! CHAI_CMD_GET_DEVICE_STATE: returns an int (1 board is ready, 0 board is NOT ready)

    //! CHAI_CMD_GET_ENCODER_0: reads encoder 0, returns counts value in a long
    //! CHAI_CMD_GET_ENCODER_1: reads encoder 1, returns counts value in a long
    //! CHAI_CMD_GET_ENCODER_2: reads encoder 2, returns counts value in a long
    //! CHAI_CMD_GET_ENCODER_3: reads encoder 3, returns counts value in a long
    //! CHAI_CMD_GET_ENCODER_4: reads encoder 4, returns counts value in a long, and a value of 0 if the encoder does not exist
    //! CHAI_CMD_GET_ENCODER_5: reads encoder 5, returns counts value in a long, and a value of 0 if the encoder does not exist
    //! CHAI_CMD_GET_ENCODER_6: reads encoder 6, returns counts value in a long, and a value of 0 if the encoder does not exist
    //! CHAI_CMD_GET_ENCODER_7: reads encoder 7, returns counts value in a long, and a value of 0 if the encoder does not exist

    //! CHAI_CMD_SET_DAC_0: writes a voltage to DAC 0 a value between +10 and -10 volts, which is a double
    //! CHAI_CMD_SET_DAC_1: writes a voltage to DAC 1 a value between +10 and -10 volts, which is a double
    //! CHAI_CMD_SET_DAC_2: writes a voltage to DAC 2 a value between +10 and -10 volts, which is a double
    //! CHAI_CMD_SET_DAC_3: writes a voltage to DAC 3 a value between +10 and -10 volts, which is a double
    //! CHAI_CMD_SET_DAC_4: writes a voltage to DAC 4 a value between +10 and -10 volts, which is a double. If this axis is not supported no action is taken
    //! CHAI_CMD_SET_DAC_5: writes a voltage to DAC 5 a value between +10 and -10 volts, which is a double. If this axis is not supported no action is taken
    //! CHAI_CMD_SET_DAC_6: writes a voltage to DAC 6 a value between +10 and -10 volts, which is a double. If this axis is not supported no action is taken
    //! CHAI_CMD_SET_DAC_7: writes a voltage to DAC 7 a value between +10 and -10 volts, which is a double. If this axis is not supported no action is taken
    virtual int command(int iCommand, void* iData);


    private:

    //! MEMBERS:
    //! Initial values of the encoder to reset them
    long m_homeposition[20];

    //! Board base address
    unsigned short m_wBaseAddress;

    //! Interrupt request used by board
    unsigned short m_wIrq;

    //! Board model: defined by BrdtstOK
    unsigned short m_wModel;

    //! Tells if the board is present. set by stg_init()
    unsigned short m_wNoBoardFlag;

    //! Number of encoders used. set by stgInit() through encoderInit()
    unsigned short m_wAxesInSys;

    //! Directions for DIO. Set by DIODirections(). don't care now
    unsigned short m_wSaveDirs;

    //! Set by stg_init()
    unsigned char  m_byIndexPollAxis;

    //! Polarity of signals.
    unsigned char  m_byIndexPulsePolarity;

    //! METHODS
    //! Set value to dac
    void setDac(int a_num, double a_volts);

    //! Read encoder values. Returns the number of values read
    int getEncoder(int a_num, long *a_value);

    //! Checks if the board is present
    unsigned short brdtstOK(unsigned short a_baseAddress);

    //! Initializes the board. This should be called by the constructor
    void stg_Init(unsigned short a_wAdd);

    //! Initializes the encoders
    void encoderInit();

    //! Latches the encoders.
    void encoderLatch();

    //! Automatically finds the base address of the board
    unsigned short findBaseAddress();

    //! Latches and reads all the encoders of the board
    void encReadAll(LONGBYTE * a_lbEnc);
    void rawDAC(unsigned short a_nAxis, long a_lCounts);

    //! Returns the base address for the board
    int getBaseAddress();
};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

