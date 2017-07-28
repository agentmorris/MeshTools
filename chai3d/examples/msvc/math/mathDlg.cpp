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
    \author:    Chris Sewell
    \version    1.0
    \date       06/2004
*/
//===========================================================================

#include "stdafx.h"
#include "math.h"
#include "mathDlg.h"
#include "math_globals.h"

#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CmathDlg::CmathDlg(CWnd* pParent /*=NULL*/)
  : CDialog(CmathDlg::IDD, pParent) {
  //{{AFX_DATA_INIT(CmathDlg)
  //}}AFX_DATA_INIT
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  
}

void CmathDlg::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CmathDlg)  
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CmathDlg, CDialog)
  //{{AFX_MSG_MAP(CmathDlg)
  ON_WM_CLOSE()
  ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit)
  ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
  ON_BN_CLICKED(IDC_BUTTON10, OnButton10)
  ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
  ON_BN_CLICKED(IDC_BUTTON11, OnButton11)
  ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
  ON_BN_CLICKED(IDC_BUTTON12, OnButton12)
  ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
  ON_BN_CLICKED(IDC_BUTTON9, OnButton9)
  ON_BN_CLICKED(IDC_BUTTON13, OnButton13)
  ON_BN_CLICKED(IDC_BUTTON14, OnButton14)
  ON_BN_CLICKED(IDC_BUTTON19, OnButton19)
  ON_BN_CLICKED(IDC_BUTTON15, OnButton15)
  ON_BN_CLICKED(IDC_BUTTON20, OnButton20)
  ON_BN_CLICKED(IDC_BUTTON16, OnButton16)
  ON_BN_CLICKED(IDC_BUTTON21, OnButton21)
  ON_BN_CLICKED(IDC_BUTTON17, OnButton17)
  ON_WM_QUERYDRAGICON()
  ON_WM_PAINT()
  ON_EN_CHANGE(IDC_EDIT2, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT3, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT4, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT5, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT6, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT7, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT8, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT9, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT10, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT11, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT12, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT13, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT14, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT15, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT16, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT17, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT18, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT19, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT20, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT21, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT22, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT23, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT24, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT25, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT26, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT27, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT28, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT29, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT30, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT31, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT32, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT33, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT34, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT35, OnChangeEdit)
  ON_EN_CHANGE(IDC_EDIT36, OnChangeEdit)
  ON_BN_CLICKED(IDC_BUTTON22, OnButton22)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CmathDlg::OnInitDialog() {

  CDialog::OnInitDialog();

  // initialise vector v0
  v0.set(1.0, 2.0, 3.0);

  // copy values of vector v0 to vector v1
  v1.copyfrom(v0);

  // copy values from vector v1 to vector vr
  v1.copyto(vr);

  // initialise matrix m0 with identity matrix
  m0.identity();

  // initilise matrix m2 with values
  m1.set(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);

  // copy values from m2 to mr
  m1.copyto(mr);

  vValue = 0.0;

  // update display of vector in edit boxes
  updating = true;
  updateAll();
  updating = false;
  
  return TRUE;
}


void CmathDlg::OnClose() {
  
  g_main_app->uninitialize();

  FreeConsole();
  exit(0);

  CDialog::OnClose();
}


void CmathDlg::updateAll() {
  char buff[50];
  updating = true;

  CEdit* edit = (CEdit*)(GetDlgItem(IDC_EDIT1));
  sprintf(buff, "%.3lf", m0.m[0][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT2));
  sprintf(buff, "%.3lf", m0.m[0][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT3));
  sprintf(buff, "%.3lf", m0.m[0][2]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT4));
  sprintf(buff, "%.3lf", m0.m[1][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT5));
  sprintf(buff, "%.3lf", m0.m[1][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT6));
  sprintf(buff, "%.3lf", m0.m[1][2]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT7));
  sprintf(buff, "%.3lf", m0.m[2][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT8));
  sprintf(buff, "%.3lf", m0.m[2][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT9));
  sprintf(buff, "%.3lf", m0.m[2][2]);
  edit->SetWindowText(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT10));
  sprintf(buff, "%.3lf", m1.m[0][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT11));
  sprintf(buff, "%.3lf", m1.m[0][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT12));
  sprintf(buff, "%.3lf", m1.m[0][2]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT13));
  sprintf(buff, "%.3lf", m1.m[1][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT14));
  sprintf(buff, "%.3lf", m1.m[1][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT15));
  sprintf(buff, "%.3lf", m1.m[1][2]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT16));
  sprintf(buff, "%.3lf", m1.m[2][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT17));
  sprintf(buff, "%.3lf", m1.m[2][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT18));
  sprintf(buff, "%.3lf", m1.m[2][2]);
  edit->SetWindowText(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT19));
  sprintf(buff, "%.3lf", mr.m[0][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT20));
  sprintf(buff, "%.3lf", mr.m[0][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT21));
  sprintf(buff, "%.3lf", mr.m[0][2]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT22));
  sprintf(buff, "%.3lf", mr.m[1][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT23));
  sprintf(buff, "%.3lf", mr.m[1][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT24));
  sprintf(buff, "%.3lf", mr.m[1][2]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT25));
  sprintf(buff, "%.3lf", mr.m[2][0]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT26));
  sprintf(buff, "%.3lf", mr.m[2][1]);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT27));
  sprintf(buff, "%.3lf", mr.m[2][2]);
  edit->SetWindowText(buff);
  
  edit = (CEdit*)(GetDlgItem(IDC_EDIT28));
  sprintf(buff, "%.3lf", v0.x);
  edit->SetWindowText(buff);
    edit = (CEdit*)(GetDlgItem(IDC_EDIT29));
  sprintf(buff, "%.3lf", v0.y);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT30));
  sprintf(buff, "%.3lf", v0.z);
  edit->SetWindowText(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT31));
  sprintf(buff, "%.3lf", v1.x);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT32));
  sprintf(buff, "%.3lf", v1.y);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT33));
  sprintf(buff, "%.3lf", v1.z);
  edit->SetWindowText(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT34));
  sprintf(buff, "%.3lf", vr.x);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT35));
  sprintf(buff, "%.3lf", vr.y);
  edit->SetWindowText(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT36));
  sprintf(buff, "%.3lf", vr.z);
  edit->SetWindowText(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT37));
  sprintf(buff, "%.3lf", vValue);
  edit->SetWindowText(buff);

  updating = false;
}


void CmathDlg::OnChangeEdit() 
{
  if (updating)
    return;
  char buff[50];
  CEdit* edit = (CEdit*)(GetDlgItem(IDC_EDIT1));
  edit->GetWindowText(buff, 50);
  m0.m[0][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT2));
  edit->GetWindowText(buff, 50);
  m0.m[0][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT3));
  edit->GetWindowText(buff, 50);
  m0.m[0][2] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT4));
  edit->GetWindowText(buff, 50);
  m0.m[1][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT5));
  edit->GetWindowText(buff, 50);
  m0.m[1][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT6));
  edit->GetWindowText(buff, 50);
  m0.m[1][2] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT7));
  edit->GetWindowText(buff, 50);
  m0.m[2][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT8));
  edit->GetWindowText(buff, 50);
  m0.m[2][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT9));
  edit->GetWindowText(buff, 50);
  m0.m[2][2] = atof(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT10));
  edit->GetWindowText(buff, 50);
  m1.m[0][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT11));
  edit->GetWindowText(buff, 50);
  m1.m[0][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT12));
  edit->GetWindowText(buff, 50);
  m1.m[0][2] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT13));
  edit->GetWindowText(buff, 50);
  m1.m[1][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT14));
  edit->GetWindowText(buff, 50);
  m1.m[1][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT15));
  edit->GetWindowText(buff, 50);
  m1.m[1][2] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT16));
  edit->GetWindowText(buff, 50);
  m1.m[2][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT17));
  edit->GetWindowText(buff, 50);
  m1.m[2][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT18));
  edit->GetWindowText(buff, 50);
  m1.m[2][2] = atof(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT19));
    edit->GetWindowText(buff, 50);
  mr.m[0][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT20));
  edit->GetWindowText(buff, 50);
  mr.m[0][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT21));
  edit->GetWindowText(buff, 50);
  mr.m[0][2] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT22));
  edit->GetWindowText(buff, 50);
  mr.m[1][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT23));
  edit->GetWindowText(buff, 50);
  mr.m[1][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT24));
  edit->GetWindowText(buff, 50);
  mr.m[1][2] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT25));
  edit->GetWindowText(buff, 50);
  mr.m[2][0] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT26));
  edit->GetWindowText(buff, 50);
  mr.m[2][1] = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT27));
  edit->GetWindowText(buff, 50);
  mr.m[2][2] = atof(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT28));
  edit->GetWindowText(buff, 50);
  v0.x = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT29));
  edit->GetWindowText(buff, 50);
  v0.y = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT30));
  edit->GetWindowText(buff, 50);
  v0.z = atof(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT31));
  edit->GetWindowText(buff, 50);
  v1.x = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT32));
  edit->GetWindowText(buff, 50);
  v1.y = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT33));
  edit->GetWindowText(buff, 50);
  v1.z = atof(buff);

  edit = (CEdit*)(GetDlgItem(IDC_EDIT34));
  edit->GetWindowText(buff, 50);
  vr.x = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT35));
  edit->GetWindowText(buff, 50);
  vr.y = atof(buff);
  edit = (CEdit*)(GetDlgItem(IDC_EDIT36));
  edit->GetWindowText(buff, 50);
  vr.z = atof(buff);
  
}

void CmathDlg::OnButton5() 
{
  OnChangeEdit();
  vr.copyfrom(v0);
  vr.add(v1);
  updateAll();
}

void CmathDlg::OnButton10() 
{
  OnChangeEdit();
  vr.copyfrom(v0);
  vr.sub(v1);
  updateAll();      
}

void CmathDlg::OnButton6() 
{
  OnChangeEdit();
  vr.mul(2.0);
  updateAll();  
}

void CmathDlg::OnButton11() 
{
  OnChangeEdit();
  vr.div(2.0);
  updateAll();  
}

void CmathDlg::OnButton7() 
{
  OnChangeEdit();
  v0.crossr(v1, vr);
  updateAll();  
}

void CmathDlg::OnButton12() 
{
  OnChangeEdit();
  vr.cross(v1);
  updateAll();  
}

void CmathDlg::OnButton8() 
{
  OnChangeEdit();
  vr.normalize();
  updateAll();  
}

void CmathDlg::OnButton9() 
{
  OnChangeEdit();
  vValue = vr.length();
  updateAll();  
}

void CmathDlg::OnButton13() 
{
  OnChangeEdit();
  vr.copyfrom(v0);
  vValue = vr.dot(v1);
  updateAll();  
}

void CmathDlg::OnButton14() 
{
  OnChangeEdit();
  m0.identity();
  m1.identity();
  mr.identity();
  updateAll();
}

void CmathDlg::OnButton19() 
{
  OnChangeEdit();
  m0.mulr(m1, mr);
  updateAll();  
}

void CmathDlg::OnButton15() 
{
  OnChangeEdit();
  m0.invert();
  updateAll();  
}

void CmathDlg::OnButton20() 
{
  OnChangeEdit();
  m1.copyfrom(m0);
  updateAll();  
}

void CmathDlg::OnButton16() 
{
  OnChangeEdit();
  mr.trans();
  updateAll();  
}

void CmathDlg::OnButton21() 
{
  OnChangeEdit();
  m1.transr(mr);
  updateAll();  
}

void CmathDlg::OnButton17() 
{
  OnChangeEdit();
  m0.mulr(v0, vr);
  updateAll();  
}

void CmathDlg::OnButton22() 
{
  OnChangeEdit();
  mr.set(0.0);
  updateAll();  
}
