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
	m_pChild=NULL;
}

CMyControlBar::~CMyControlBar()
{
	if(m_pChild) delete m_pChild;
}

int CMyControlBar::Create( CWnd* pWnd, const char* cCaption )
{
	SECControlBar::Create(pWnd, cCaption,WS_VISIBLE|CBRS_ALL ,1,0,NULL);
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
	CRect rect;
	GetInsideRect(ir);
	GetClientRect(rect);
  	DrawBorders(pDC, rect);
	CBrush br;
	br.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
	pDC->FillRect(ir,&br);
	DrawGripper(pDC, rect);
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
	if(!m_pChild)
		m_pChild=new CWnd();
	return m_pChild->CreateControl(className,NULL, WS_VISIBLE|WS_CHILD,CRect(0,0,0,0), this, 0, NULL,FALSE,NULL);	
}

BOOL CMyControlBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	return TRUE;
}
