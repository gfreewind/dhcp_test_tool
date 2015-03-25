// dhcp_tool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Cdhcp_toolApp:
// See dhcp_tool.cpp for the implementation of this class
//

class Cdhcp_toolApp : public CWinApp
{
public:
	Cdhcp_toolApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern Cdhcp_toolApp theApp;