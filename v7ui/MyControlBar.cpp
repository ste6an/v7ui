// MyControlBar.cpp: implementation of the CMyControlBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyControlBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMyControlBar::CMyControlBar()
{
	m_bOptimizedRedrawEnabled=TRUE;
	m_bAutoDelete=FALSE;
	m_pChild=new CWnd();
	m_pChild->SetParent(this);
}

CMyControlBar::~CMyControlBar()
{
	delete m_pChild;
}

int CMyControlBar::Create( CWnd* pWnd, const char* cCaption )
{
	SECControlBar::Create(pWnd, cCaption,WS_VISIBLE|CBRS_ALL ,1,7777,NULL);
//	m_pButton=new CButton();
//	m_pButton->Create("кнопачка",WS_CHILD|WS_VISIBLE,CRect(0,0,100,100),this,0);
	return 1;
}

void CMyControlBar::OnSize( UINT nType, int cx, int cy )
{
	//DoMsgLine("size");
}

void CMyControlBar::OnBarDock()
{
}

void CMyControlBar::OnBarFloat()
{
	OnBarDock();
}

void CMyControlBar::DoPaint( class CDC * pDC)
{
 	CRect ir;
 	GetInsideRect(ir);
 	CBrush br;
 	br.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
 	pDC->FillRect(ir,&br);
  	CRect rect;
  	GetClientRect(rect);
  	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
	SetGripperExpandButtonState(2);
	//AdjustInsideRectForGripper(rect,1);
// 	DrawGripperCloseButton(pDC,rect,1);
// 	DrawGripperExpandButton(pDC,rect,1);

}

CSize CMyControlBar::CalcDynamicLayout( int nLength, DWORD nMode )
{
	CSize sz = SECControlBar::CalcDynamicLayout(nLength, nMode);
	if(m_pChild)
	{
 		CRect ir(0,0,sz.cx,sz.cy);
		if(nMode & (LM_HORZDOCK | LM_VERTDOCK))
			AdjustInsideRectForGripper(ir,(nMode & LM_HORZ ? 1:0));
 		m_pChild->MoveWindow(ir,TRUE);
	}
	return sz;
}

CSize CMyControlBar::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
{
	CSize sz = SECControlBar::CalcFixedLayout(bStretch,bHorz);
	if(m_pChild)
	{
		CRect ir(0,0,sz.cx,sz.cy);
		if(!bStretch)
			AdjustInsideRectForGripper(ir,bHorz);
		m_pChild->MoveWindow(ir,TRUE);
	}
	return sz;
}

int CMyControlBar::CreateControl( LPCTSTR className )
{
	return m_pChild->CreateControl(className,NULL, WS_VISIBLE|WS_CHILD,CRect(0,0,0,0), this, 0, NULL,FALSE,NULL);	
}

BOOL CMyControlBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	//DoMsgLine("nID: %d\t nCode: %d",mmNone,nID,nCode);
	return TRUE;
}
