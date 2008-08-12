// v7ui.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static AFX_EXTENSION_MODULE V7uiDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("V7UI.DLL Initializing!\n");
		
		if (!AfxInitExtensionModule(V7uiDLL, hInstance))
			return 0;

		context_obj::CContextBase::InitAllContextClasses();
		Init1CGlobal(hInstance);
		new CDynLinkLibrary(V7uiDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("V7UI.DLL Terminating!\n");
		context_obj::CContextBase::DoneAllContextClasses();
		AfxTermExtensionModule(V7uiDLL);
	}
	return 1;
}
