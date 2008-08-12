// LayoutMgrContext.cpp: implementation of the CLayoutMgrContext class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LayoutMgrContext.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLayoutMgrContext::CLayoutMgrContext()
{
	m_pLMrg=new COXLayoutManager();
	m_pDoc=NULL;
}

CLayoutMgrContext::~CLayoutMgrContext()
{
	m_pLMrg->Detach();
	delete m_pLMrg;
}

BL_INIT_CONTEXT(CLayoutMgrContext);

CLayoutMgrContext::SetForm(CValue** ppParams)
{
	if (ppParams[0]->GetTypeCode() != AGREGATE_TYPE_1C)
		RuntimeError("Недопустимое значение параметра");
	
	CBLContext* pCtrlContext = ppParams[0]->GetContext();
	if (strcmp(pCtrlContext->GetRuntimeClass()->m_lpszClassName,"CFormAllCtrlsContext"))
		RuntimeError("Недопустимое значение параметра");
	
	m_pDoc = ((CFormAllCtrlsContext*)pCtrlContext)->m_GetDoc;
	if (m_pDoc->IsDocLayouted())
	{
		CWnd* pFormWindow = m_pDoc->GetForm();
		m_pLMrg->Attach(pFormWindow);
	}
	return TRUE;
}

CLayoutMgrContext::Attach(CValue** ppParams)
{
	if (!m_pDoc)
	{
		RuntimeError("Не установлена форма");
	}
	if (m_pDoc->IsDocLayouted())
	{
		CWnd* pFormWindow = m_pDoc->GetForm();
		m_pLMrg->Attach(pFormWindow);
	}else{
		RuntimeError("Форма не создана");
	}
	return TRUE;
}

CLayoutMgrContext::SetConstraint(CValue** ppParams)
{
	CString fn=ppParams[0]->GetString();
	char* FieldNames = fn.GetBuffer(fn.GetLength()+1);
	char seps[] = " ,\t\n";
	char *token;
	token = strtok( FieldNames, seps);
	while (token != NULL)
	{
		CGetField* pField = m_pDoc->GetField(token);
		int nSide = ppParams[1]->GetNumeric();
		int nType = ppParams[2]->GetNumeric();
		CString BaseWndName = ppParams[3]->GetString();
		BaseWndName.MakeUpper();
		int BaseWnd;
		if (BaseWndName == "ФОРМА")
		{
			BaseWnd=0;
		}else{
			BaseWnd=m_pDoc->GetField(BaseWndName)->GetCtrlID();
		}
		m_pLMrg->SetConstraint(pField->GetCtrlID(),nSide,nType,0,BaseWnd);

		token = strtok(NULL, seps);
	}

	return TRUE;
}

CLayoutMgrContext::RedrawLayout(CValue** ppParams)
{
	m_pLMrg->RedrawLayout();
	return TRUE;
}

CLayoutMgrContext::SetMinMax(CValue** ppParams)
{
	CGetField* pField = m_pDoc->GetField(ppParams[0]->GetString());
	m_pLMrg->SetMinMax(pField->GetCtrlID(),
		CSize(ppParams[1]->GetNumeric(), ppParams[2]->GetNumeric()),
		CSize(ppParams[3]->GetNumeric(), ppParams[4]->GetNumeric()));
	return TRUE;
}