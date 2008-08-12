// ControlBarContext.cpp: implementation of the CControlBarContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ControlBarContext.h"
#include "resource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void DispatchToValue(IUnknown** ppUnk, CValue* pVal)
{
	pVal->Reset();
	
	IDispatchPtr pDisp = ppUnk[0];
	
	if(pDisp != NULL)
	{
		VARIANT var;
		
		var.vt		 = VT_DISPATCH;
		var.pdispVal = pDisp;
		
		DISPPARAMS disp = {&var, NULL, 1, 0};
		
		UINT ui;
		static_cast<CBLMyEx*>(NULL)->DispParamToValueEx(&pVal, &disp, &ui);
		
		pDisp->Release();
	}
}

BL_INIT_CONTEXT(CControlBarContext);

CControlBarContext::CControlBarContext()
{
	m_pCBar = new CMyControlBar();
	m_pCBar->Create(pMainFrame,_T("!!!"));
	m_pCBar->EnableDocking(CBRS_ALIGN_ANY);
	pMainFrame->FloatControlBar(m_pCBar, CPoint(100,100),0);
}

CControlBarContext::~CControlBarContext()
{
//	m_pCBar->DestroyWindow();
	delete m_pCBar;
}

BOOL CControlBarContext::Dock(CValue** ppParams)
{
	UINT dockSide;
	switch(ppParams[0]->GetNumeric())
	{
	case 0:	dockSide=AFX_IDW_DOCKBAR_TOP; break;
	case 1:	dockSide=AFX_IDW_DOCKBAR_BOTTOM; break;
	case 2:	dockSide=AFX_IDW_DOCKBAR_LEFT; break;
	case 3:	dockSide=AFX_IDW_DOCKBAR_RIGHT; break;
	case 4:	dockSide=AFX_IDW_DOCKBAR_FLOAT; break;
	}
	//m_pCBar->SetBarStyle(WS_CHILD|WS_VISIBLE|dockSide);
	pMainFrame->DockControlBarEx(m_pCBar, dockSide, 0, 0, (float)0.75, 100);
	return TRUE;
}

BOOL CControlBarContext::CreateControl(CValue& rValue, CValue** ppValues)
{
	m_pCBar->CreateControl(ppValues[0]->GetString());
 	LPUNKNOWN iUnkControl=m_pCBar->m_pChild->GetControlUnknown();
 	iUnkControl->AddRef();
 	DispatchToValue(reinterpret_cast<IUnknown**>(&iUnkControl), &rValue);
	return TRUE;
}

BOOL CControlBarContext::CreateExplorerBar(CValue& rValue, CValue** ppValues)
{
	CWndDirectUI* pW = new CWndDirectUI();
	//pW->SetStyle(CWndDirectUI::styleXP);
	pW->Create(CRect(0,0,300,0), m_pCBar, 1000);
	pW->InitFromMenu(151);
	int nGroup=pW->AddGroup("√руппа1");
	pW->AddItem(nGroup, new CDirectUIItemStatic(_T("—сылка1")));
	m_pCBar->m_pChild = pW;
	return TRUE;
	return TRUE;
}

BOOL CControlBarContext::Get_Visible(CValue& value) const
{
	value=m_pCBar->IsWindowVisible();
	return TRUE;
}

BOOL CControlBarContext::Set_Visible(CValue const& value)
{
	int v = value.GetNumeric();
	m_pCBar->ShowWindow(v? SW_SHOW : SW_HIDE);
	pMainFrame->RecalcLayout(1);
	return TRUE;
}

BOOL CControlBarContext::Get_Caption(CValue& value) const
{
	return TRUE;
}

BOOL CControlBarContext::Set_Caption(CValue const& value)
{
	return TRUE;
}
