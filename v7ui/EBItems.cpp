// EBItems.cpp: implementation of the CEBItems class.
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

BL_INIT_CONTEXT(CEBGroupContext);
BL_INIT_CONTEXT(CEBItemContext);


CEBGroupContext::CEBGroupContext()
{
}

CEBGroupContext::CEBGroupContext(CString sCaption, CWndDirectUI* pExpBar)
{
	m_pGroup=pExpBar->AddGroup(sCaption);
}

CEBGroupContext::~CEBGroupContext()
{
}

CEBGroupContext::AddItem(CValue& retVal, CValue** ppParams)
{
	retVal.AssignContext(new CEBItemContext(ppParams[0]->GetString(),m_pGroup,
		ppParams[2]->GetNumeric(), *ppParams[1]));
	return TRUE;
}

CEBItemContext::CEBItemContext()
{	
}

CEBItemContext::CEBItemContext(CString sCaption, CDirectUIGroup* pGroup, int nIconIndex, CValue& value)
{
	m_pItem=new CDirectUIItem(sCaption,0,nIconIndex);
	m_pItem->m_ItemValue=value;
	pGroup->AddItem(m_pItem);
}

CEBItemContext::~CEBItemContext()
{
	
}

