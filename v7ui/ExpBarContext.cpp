// ExpBarContext.cpp: implementation of the CExpBarContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WndDirectUI.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

enum ExpBarEvents
{
    ebOnItemClick,
	ebLastEvent
};

stCtrlEvents CEBCtrlEvents[]  =  
{
    {"OnItemClick", "OnItemClick", 1}
};

BL_INIT_CONTEXT(CExpBarContext);

CExpBarContext::CExpBarContext()
{
	m_pDoc=NULL;
	m_pExpBar=NULL;
	m_pField=NULL;
	m_pUDC=NULL;
}

CExpBarContext::~CExpBarContext()
{

}

BOOL CExpBarContext::InitControlEx(CBLContext* pForm, CGetDoc7* pDoc, CGetField* pField, CBLContext* pUDC, CValue* pCreateParam)
{
	m_pDoc = pDoc;
	m_pField = pField;
	m_pUDC = pUDC;
	return TRUE;
}

BOOL CExpBarContext::CreateControlWndEx(CWnd* pParent)
{
	if (!m_pDoc)
	{
		BOOL bSuccess = FALSE;
		m_pExpBar = new CWndDirectUI(this,m_pUDC);
		if(m_pExpBar)
		{
			bSuccess=m_pExpBar->Create(CRect(0,0,50,100), pParent, 0);
			if (bSuccess)
			{
				m_EventManager.SetEvents(CEBCtrlEvents, ebLastEvent, "", CBLModule::GetExecutedModule(), this, m_pUDC);
			}
		}
		return bSuccess;
	}else{
		CControlID* pControlID = m_pField->GetCtrlInfo();
		CWnd* pControlWnd = m_pDoc->GetFieldWnd(m_pField);
		CRect rect;
		pControlWnd->GetWindowRect(&rect);
		pParent->ScreenToClient(rect);
		
		BOOL bSuccess = FALSE;
		m_pExpBar = new CWndDirectUI(this,m_pUDC);
		if(m_pExpBar)
		{
			bSuccess=m_pExpBar->Create(rect, pParent, m_pField->GetCtrlID());
			if (bSuccess)
			{
				pControlWnd->DestroyWindow();
				m_EventManager.SetEvents(CEBCtrlEvents, ebLastEvent, pControlID, m_pDoc->m_pBLModule, this, m_pUDC);
			}
		}
		
		return bSuccess;
	}
}

BOOL CExpBarContext::CreateControlWnd(CWnd* pParent, CGetDoc7* pDoc, CGetField* pGetField, CBLContext* pUDC)
{
	
	m_pDoc = pDoc;
	m_pField = pGetField;
	m_pUDC = pUDC;
	
	return CreateControlWndEx(pParent);
}

BOOL CExpBarContext::AddGroup(CValue& retVal, CValue** ppParams)
{
	retVal.AssignContext(new CEBGroupContext(ppParams[0]->GetString(),m_pExpBar));
	return TRUE;
}


void CExpBarContext::OnItemClick( CDirectUIItem* pItem )
{
	if (m_EventManager.IsAppointed(ebOnItemClick))
	{
		CValue val=pItem->m_ItemValue;
		CExecBatchHelper ExecBatchHelper(m_pDoc, TRUE);
		m_EventManager.Invoke1(ebOnItemClick, val);
	}
}

BOOL CExpBarContext::SetStyle(CValue** ppParams)
{
	m_pExpBar->SetStyle(ppParams[0]->GetNumeric());
	return TRUE;
}