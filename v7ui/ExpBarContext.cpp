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
	m_pExpBar = new CWndDirectUI(this);
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
		BOOL bSuccess = m_pExpBar->Create(CRect(0,0,50,100), pParent, 0);
		if (bSuccess)
		{
			m_EventManager.SetEvents(CEBCtrlEvents, ebLastEvent, "", CBLModule::GetExecutedModule(), this, m_pUDC);
		}
		return bSuccess;
	}else{
		CControlID* pControlID = m_pField->GetCtrlInfo();
		CWnd* pControlWnd = m_pDoc->GetFieldWnd(m_pField);
		CRect rect;
		pControlWnd->GetWindowRect(&rect);
		pParent->ScreenToClient(rect);
		
		BOOL bSuccess = m_pExpBar->Create(rect, pParent, m_pField->GetCtrlID());
		if (bSuccess)
		{
			pControlWnd->DestroyWindow();
			m_EventManager.SetEvents(CEBCtrlEvents, ebLastEvent, pControlID, m_pDoc->m_pBLModule, this, m_pUDC);
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

BOOL CExpBarContext::SetImageList(CValue& retVal, CValue** ppParams)
{
	if (ppParams[0]->GetTypeCode() != AGREGATE_TYPE_1C || strcmp(ppParams[0]->GetContext()->GetRuntimeClass()->m_lpszClassName,"CPictureContext"))
	{
		RuntimeError("Ќедопустимое значение первого параметра.", 0);
		return FALSE;
	}
	CPictureHolder7& PictureHolder7 = static_cast<CPictureContext*>(ppParams[0]->GetContext())->m_Picture;
	
	BOOL bSuccess = FALSE;
	CDC dc;
	CDC DesktopDC;
	DesktopDC.Attach(::GetDC(NULL));
	if (dc.CreateCompatibleDC(&DesktopDC))
	{
		CSize size = PictureHolder7.GetSize();
		if (ppParams[1]->GetTypeCode() != UNDEFINE_TYPE_1C)
			size.cy = ppParams[1]->GetNumeric();
		CBitmap bitmap;
		if (bitmap.CreateCompatibleBitmap(&DesktopDC, size.cx, size.cy))
		{
			CBitmap* pOldbitmap = dc.SelectObject(&bitmap);
			CRect rect(CPoint(0,0), PictureHolder7.GetSize());
			dc.FillSolidRect(0,0, size.cx, size.cy, RGB(0x00, 0x80, 0x80));
			PictureHolder7.Render(dc, rect, 1, NULL);
			dc.SelectObject(pOldbitmap);
			if(m_pExpBar->m_lstImages.GetSafeHandle())
				m_pExpBar->m_lstImages.DeleteImageList();
			if (m_pExpBar->m_lstImages.Create(size.cy, size.cy, ILC_MASK|ILC_COLOR24, 2, 0))
			{
				bSuccess = m_pExpBar->m_lstImages.Add(&bitmap, RGB(0x00, 0x80, 0x80)) == 0;
				int i = m_pExpBar->m_lstImages.GetImageCount();
			}
			
		}
	}
	return bSuccess;
}