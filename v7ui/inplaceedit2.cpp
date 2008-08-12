// InPlaceEdit2.cpp : implementation file
//

#include "stdafx.h"
#include "InPlaceEdit2.h"
#include "WndDirectUI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit

CInPlaceEdit2::CInPlaceEdit2(CString sInitText, CDirectUIItemEdit* pParent)
:m_sInitText( sInitText )
{
	m_bESC = FALSE;
	m_pParent = pParent;
}

CInPlaceEdit2::~CInPlaceEdit2()
{
}


BEGIN_MESSAGE_MAP(CInPlaceEdit2, CEdit)
//{{AFX_MSG_MAP(CInPlaceEdit2)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit message handlers

BOOL CInPlaceEdit2::PreTranslateMessage(MSG* pMsg)
{
	if( pMsg->message == WM_KEYDOWN )
	{
		if(pMsg->wParam == VK_RETURN
		|| pMsg->wParam == VK_DELETE
		|| pMsg->wParam == VK_ESCAPE
		|| GetKeyState( VK_CONTROL))
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;    // DO NOT process further
		}
	}
	return CEdit::PreTranslateMessage(pMsg);
}


void CInPlaceEdit2::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	DestroyWindow();
}

void CInPlaceEdit2::OnNcDestroy()
{
	CString str;
	GetWindowText(str);
	m_pParent->SetText(str);
	m_pParent->m_pEdit = NULL;

	CEdit::OnNcDestroy();

	delete this;
}


void CInPlaceEdit2::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Include the following lines if you want to close the control
	// by pressing ESC or ENTER

	/*
	if( nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if( nChar == VK_ESCAPE )
			m_bESC = TRUE;
		GetParent()->SetFocus();
		return;
	}
	*/

	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

int CInPlaceEdit2::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
	return -1;

	// Set the proper font
	SetFont(CFont::FromHandle((HFONT)GetStockObject(ANSI_VAR_FONT)));

	SetWindowText( m_sInitText );
	SetFocus();
	SetSel( 0, -1 );
	return 0;
}