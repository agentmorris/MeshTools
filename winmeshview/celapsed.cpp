#include "StdAfx.h"
#include "celapsed.h"

bool CElapsed::m_timeIsHijacked = false;
double CElapsed::m_hijackedTimeValue = 0.0;
double CElapsed::m_hijackedBeginValue = 0.0;
