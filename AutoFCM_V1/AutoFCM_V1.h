
// AutoFCM_V1.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAutoFCMApp:
// See AutoFCM_V1.cpp for the implementation of this class
//

class CAutoFCMApp : public CWinApp
{
public:
	CAutoFCMApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAutoFCMApp theApp;
