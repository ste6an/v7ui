// WndDirectUI.cpp: Implementierungsdatei
//

#include "stdafx.h"

#ifdef USE_THEMES
#include "VisualStylesXP.h"
#endif
#include "WndDirectUI.h"
#include "memdc.h"
#include "Inplaceedit2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CDirectUIGroup

/////////////////////////////////////////////////////////////////////////////
// 
// Credits...
// 
// Zebrex          : On fast MouseMove underlined item stays underlined; TODO
// John A. Johnson : DT_NOPREFIX was deleted in all the code
// Andreas Kapust  : ::OnMousemove bugfix was added to handle the scrollbar correctly
// Zebrex          : HitTest failed on collapsed group
// Ewan Ward       : How to convert the CString to TCHAR for DrawThemeText()
// 


#define ADVANCED_UI_CLASSNAME _T("WndDirectUI")

#ifdef USE_THEMES
CFont CDirectUIItem::m_fntThemedText;
#endif

HCURSOR CDirectUIItem::m_hCursorHand = 0;
HCURSOR CDirectUIItemStatic::m_hCursorArrow = 0;
HCURSOR CDirectUIItemEdit::m_hCursorBeam = 0;

CDirectUIGroup::CDirectUIGroup(CString strName, CImageList* pList, int* pnStyle)
{
	m_strName    = strName;
	m_nHeight    = 0;
	m_pImageList = pList;
	m_pnStyle    = pnStyle;
	m_uiItemState= 0;
	m_rcExpandCollapseBtn.SetRectEmpty();
	m_bIsCollapsed = FALSE;
	m_bCalcHeight  = TRUE;

//	CalcHeight(NULL);
}

CDirectUIGroup::~CDirectUIGroup()
{
	RemoveAllItems();
}

COLORREF CDirectUIGroup::MakeXPColor(COLORREF cl, double factor)
{
	if(factor>0.0&&factor<=1.0){
		BYTE red,green,blue,lightred,lightgreen,lightblue;
		red = GetRValue(cl);
		green = GetGValue(cl);
		blue = GetBValue(cl);
		lightred = (BYTE)((factor*(255-red)) + red);
		lightgreen = (BYTE)((factor*(255-green)) + green);
		lightblue = (BYTE)((factor*(255-blue)) + blue);
		cl = RGB(lightred,lightgreen,lightblue);
	}
	return(cl);
}

BOOL CDirectUIGroup::EnableItem(UINT uiCommand, BOOL bEnable)
{
	BOOL bRepaint;
	bRepaint = FALSE;

	for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
	{
		CDirectUIItem* pItem = m_lstItems.GetNext(pos);
		if (pItem->m_uiCommand == uiCommand)
			bRepaint |= pItem->EnableItem(bEnable);
	}

	return m_bIsCollapsed ? FALSE : bRepaint;
}

void CDirectUIGroup::OnDraw(CDC* pDC, CRect rcItem, HTHEME hTheme, int nOffsetY, CWnd* pParent)
{
	if (m_bCalcHeight)
	{
		InternalCalcHeight(pDC, hTheme);
	}

	const int nHeaderRadius = 5;

	switch (*m_pnStyle)
	{
	case CWndDirectUI::styleOffice:
		{
			CPen*  pnOld;
			CFont* fnOld;
			// Draw Header
			LOGFONT lf;
			CFont::FromHandle((HFONT) GetStockObject(ANSI_VAR_FONT))->GetLogFont(&lf);
			CFont fnHeader;
			lf.lfWeight = FW_BOLD;
			fnHeader.CreateFontIndirect(&lf);
			fnOld = pDC->SelectObject(&fnHeader);

			CRect rcHeader;
			rcHeader.SetRect(rcItem.left, rcItem.top, rcItem.right, rcItem.top + GetSystemMetrics(SM_CYCAPTION)); // Whole top area
			pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->DrawText(m_strName, rcHeader, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
			
			CPen pnSeparator;
			pnSeparator.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
			pnOld = pDC->SelectObject(&pnSeparator);
			pDC->MoveTo(rcHeader.left, rcHeader.bottom);
			pDC->LineTo(rcHeader.BottomRight());

			// Draw Items
			CRect rcItems;
			rcItems.SetRect(rcItem.left+1, rcHeader.bottom, rcItem.right-1, rcHeader.bottom);
			if (m_pImageList) m_pImageList->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHT));
			CPen pnUnderline;
			pnUnderline.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHT));
			pDC->SelectObject(&pnUnderline);

			// Draw Items
			if (!m_bIsCollapsed)
			{
				for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
				{
					rcItems.top     = rcItems.bottom;
					rcItems.bottom  = rcItems.top + m_lstItems.GetAt(pos)->GetItemHeight(pDC, *m_pnStyle, NULL);
					m_lstItems.GetNext(pos)->OnDraw(pDC, rcItems, m_pImageList, *m_pnStyle, NULL, nOffsetY, pParent);
				}
			}
			pDC->SelectObject(fnOld);
			pDC->SelectObject(pnOld);
		}
		break;
	case CWndDirectUI::styleXP:
		{
			CPen*   pnOld;
			CFont*  fnOld;
			CBrush* brOld;

			CPen   pnMarker;
			CPen   pnBorder;
			CPen   pnBtn;
			CBrush brHeader;
			CBrush brItemArea;
			COLORREF clBorder   = GetSysColor(COLOR_HIGHLIGHT);
			COLORREF clText     = GetSysColor(COLOR_HIGHLIGHTTEXT);
			COLORREF clItemArea = MakeXPColor(clBorder, 0.85); // Seems to be ok
			pnMarker.CreatePen(PS_SOLID, 1, clText);
			pnBtn   .CreatePen(PS_SOLID, 1, clItemArea);
			pnBorder.CreatePen(PS_SOLID, 1, clBorder);
			brHeader.CreateSolidBrush(clBorder);
			brItemArea.CreateSolidBrush(clItemArea);

			// Draw Header
			LOGFONT lf;
			CFont::FromHandle((HFONT) GetStockObject(ANSI_VAR_FONT))->GetLogFont(&lf);
			CFont fnHeader;
			lf.lfWeight = FW_BOLD;
			fnHeader.CreateFontIndirect(&lf);
			fnOld = pDC->SelectObject(&fnHeader);

			// Calculate all area sizes
			CRect rcHeader;
			CRect rcPieLeft;
			CRect rcPieRight;
			CRect rcHeaderTextArea;
			rcHeader.SetRect(rcItem.left, rcItem.top, rcItem.right, rcItem.top + GetSystemMetrics(SM_CYCAPTION)); // Whole top area
			rcPieLeft.SetRect(rcItem.left, rcItem.top, rcItem.left+nHeaderRadius+nHeaderRadius, rcItem.top+nHeaderRadius+nHeaderRadius); // Upper-left edge
			rcPieRight.SetRect(rcItem.right-nHeaderRadius-nHeaderRadius, rcItem.top, rcItem.right, rcItem.top+nHeaderRadius+nHeaderRadius);// Upper-right edge
			rcHeaderTextArea.SetRect(rcHeader.left+nHeaderRadius, rcHeader.top, rcHeader.right-nHeaderRadius, rcHeader.bottom); // area between
			
			// Draw the "title bar"
			pnOld = pDC->SelectObject(&pnBorder);
			brOld = pDC->SelectObject(&brHeader);
			pDC->Pie(rcPieLeft,  CPoint(rcPieLeft.left+nHeaderRadius, rcPieLeft.top), CPoint(rcPieLeft.left, rcPieLeft.top+nHeaderRadius));
			pDC->Pie(rcPieRight, CPoint(rcPieRight.left+nHeaderRadius, rcPieRight.top+nHeaderRadius), CPoint(rcPieRight.left+nHeaderRadius, rcPieRight.top));
			pDC->FillSolidRect(rcHeaderTextArea, clBorder);
			pDC->FillSolidRect(CRect(rcHeader.left, rcHeader.top+nHeaderRadius, rcHeader.left+nHeaderRadius, rcHeader.bottom), clBorder);
			pDC->FillSolidRect(CRect(rcHeader.right-nHeaderRadius, rcHeader.top+nHeaderRadius, rcHeader.right, rcHeader.bottom), clBorder);
			if (m_uiItemState & ODS_SELECTED) 
				pDC->SetTextColor(clItemArea);
			else
				pDC->SetTextColor(clText);
			pDC->SetBkColor(clBorder);

			// Make the text field a little smaller
			rcHeaderTextArea.left += nHeaderRadius+nHeaderRadius;
			pDC->DrawText(m_strName, rcHeaderTextArea, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);

			// Draw the Expand/Collapse button
			const int nBtnSpace = 4;
			int nBtnSize;
			nBtnSize = GetSystemMetrics(SM_CYCAPTION)-nBtnSpace-nBtnSpace;
			m_rcExpandCollapseBtn.SetRect(rcItem.right-nHeaderRadius-nBtnSize, rcItem.top +nBtnSpace, rcItem.right-nHeaderRadius, rcItem.top+nBtnSpace+nBtnSize);
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->SelectObject(&pnBtn);
			pDC->Ellipse(m_rcExpandCollapseBtn);
			if (m_uiItemState & ODS_SELECTED) 
				pDC->SelectObject(&pnBtn);
			else
				pDC->SelectObject(&pnMarker);
			CPoint ptCenter = m_rcExpandCollapseBtn.CenterPoint();
			CPoint ptLeft   = ptCenter +CPoint(-4, m_bIsCollapsed ? -4:4);
			CPoint ptRight  = ptCenter +CPoint( 4, m_bIsCollapsed ? -4:4);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptLeft);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptRight);
			ptCenter.Offset(0, m_bIsCollapsed ?3:-3);
			ptLeft.Offset(0, m_bIsCollapsed ?3:-3);
			ptRight.Offset(0, m_bIsCollapsed ?3:-3);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptLeft);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptRight);
			m_rcExpandCollapseBtn.left = rcHeader.left;

			// Now draw the lighter area for all items
			pDC->SelectObject(&pnBorder);
			pDC->SelectObject(&brItemArea);
			pDC->Rectangle(rcItem.left, rcHeaderTextArea.bottom-1, rcItem.right, rcItem.bottom);

			// Draw Items
			if (!m_bIsCollapsed)
			{
				CRect rcItems;
				rcItems.SetRect(rcItem.left+1, rcHeaderTextArea.bottom, rcItem.right-1, rcHeaderTextArea.bottom);
				if (m_pImageList) m_pImageList->SetBkColor(clItemArea);
				for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
				{
					rcItems.top     = rcItems.bottom;
					rcItems.bottom  = rcItems.top + m_lstItems.GetAt(pos)->GetItemHeight(pDC, *m_pnStyle, NULL);
					m_lstItems.GetNext(pos)->OnDraw(pDC, rcItems, m_pImageList, *m_pnStyle, NULL, nOffsetY, pParent);
				}
			}

			pDC->SelectObject(fnOld);
			pDC->SelectObject(pnOld);
			pDC->SelectObject(brOld);
		}
		break;
#ifdef USE_THEMES
	case CWndDirectUI::styleThemed:
		if (g_xpStyle.IsAppThemed() & (hTheme != 0))
		{
			CRect rcHeader, rcHeaderTextArea;
			rcHeader.SetRect(rcItem.left, rcItem.top, rcItem.right, rcItem.top + GetSystemMetrics(SM_CYCAPTION)); // Whole top area
			if ((*m_pnStyle == CWndDirectUI::styleThemed) & g_xpStyle.IsAppThemed()) 
				rcHeader.bottom = rcHeader.top + MulDiv(rcHeader.Height(), 9, 8);

			g_xpStyle.DrawThemeBackground(hTheme, pDC->m_hDC, EBP_NORMALGROUPHEAD, 0, &rcHeader, 0);
			g_xpStyle.GetThemeBackgroundContentRect(hTheme, pDC->m_hDC, EBP_NORMALGROUPHEAD, 0, rcHeader,  &rcHeaderTextArea);

			int nTextLen = strlen(m_strName); 
			int mlen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, NULL, 0); 
			WCHAR* output = new WCHAR[mlen];
			if(output)
			{
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, output, mlen);
				g_xpStyle.DrawThemeText(hTheme, pDC->m_hDC, EBP_NORMALGROUPHEAD, 0, output, -1, DT_LEFT|DT_VCENTER | DT_SINGLELINE|DT_END_ELLIPSIS, 0, &rcHeaderTextArea);
				delete output;
			}

			const int nBtnSpace = 2;
			int nBtnSize;
			nBtnSize = GetSystemMetrics(SM_CYCAPTION)-nBtnSpace-nBtnSpace;
			m_rcExpandCollapseBtn.SetRect(rcItem.right-nHeaderRadius-nBtnSize, rcItem.top +nBtnSpace, rcItem.right-nHeaderRadius, rcItem.top+nBtnSpace+nBtnSize);
			g_xpStyle.DrawThemeBackground(hTheme, pDC->m_hDC, m_bIsCollapsed ? EBP_NORMALGROUPEXPAND:EBP_NORMALGROUPCOLLAPSE, (m_uiItemState & ODS_SELECTED) ? EBNGC_HOT: EBNGC_NORMAL, &m_rcExpandCollapseBtn, 0);
			m_rcExpandCollapseBtn.left = rcHeader.left;
			if (!m_bIsCollapsed)
				g_xpStyle.DrawThemeBackground(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0, CRect(rcItem.left, rcHeader.bottom-1, rcItem.right, rcItem.bottom), 0);

			CPen*   pnOld;
			CPen   pnUnderline;
			pnUnderline.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHTTEXT));
			pnOld = pDC->SelectObject(&pnUnderline);

			if (!m_bIsCollapsed)
			{
				CRect rcItems;
				g_xpStyle.GetThemeBackgroundContentRect(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0, rcHeader,  &rcItems);
				rcItems.top = rcHeader.bottom;
				rcItems.bottom = rcHeader.bottom;
				for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
				{
					rcItems.top     = rcItems.bottom;
					rcItems.bottom  = rcItems.top + m_lstItems.GetAt(pos)->GetItemHeight(pDC, *m_pnStyle, NULL);
					m_lstItems.GetNext(pos)->OnDraw(pDC, rcItems, m_pImageList, *m_pnStyle, hTheme, nOffsetY, pParent);
				}
			}
			pDC->SelectObject(pnOld);

			break;
		}
#endif
	case CWndDirectUI::styleXPclassic:
	default:
		{
			CPen*   pnOld;
			CFont*  fnOld;
			CBrush* brOld;
			
			CPen   pnBorder;
			CPen   pnMarker;
			CBrush brHeader;
			CBrush brItemArea;
			COLORREF clBorder   = GetSysColor(COLOR_3DFACE);
			COLORREF clText     = GetSysColor(COLOR_BTNTEXT);
			COLORREF clItemArea = GetSysColor(COLOR_WINDOW);
			pnBorder.CreatePen(PS_SOLID, 1, clBorder);
			pnMarker.CreatePen(PS_SOLID, 1, clText);
			brHeader.CreateSolidBrush(clBorder);
			brItemArea.CreateSolidBrush(clItemArea);
			
			// Draw Header
			LOGFONT lf;
			CFont::FromHandle((HFONT) GetStockObject(ANSI_VAR_FONT))->GetLogFont(&lf);
			CFont fnHeader;
			lf.lfWeight = FW_BOLD;
			fnHeader.CreateFontIndirect(&lf);
			fnOld= pDC->SelectObject(&fnHeader);
			
			// Draw Header
			CRect rcHeader;
			CRect rcHeaderTextArea;
			rcHeader.SetRect(rcItem.left, rcItem.top, rcItem.right, rcItem.top + GetSystemMetrics(SM_CYCAPTION)); // Whole top area
			pDC->SetTextColor(clText);
			pDC->SetBkColor(clBorder);
			rcHeaderTextArea.SetRect(rcHeader.left+nHeaderRadius, rcHeader.top, rcHeader.right-nHeaderRadius, rcHeader.bottom); // area between
			brOld = pDC->SelectObject(&brHeader);
			pDC->FillSolidRect(rcHeader, clBorder);
			pDC->DrawText(m_strName, rcHeaderTextArea, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
			
			// Draw the Expand/Collapse button
			const int nBtnSpace = 2;
			int nBtnSize;
			nBtnSize = GetSystemMetrics(SM_CYCAPTION)-nBtnSpace-nBtnSpace;
			m_rcExpandCollapseBtn.SetRect(rcItem.right-nHeaderRadius-nBtnSize, rcItem.top +nBtnSpace, rcItem.right-nHeaderRadius, rcItem.top+nBtnSpace+nBtnSize);
			if (m_uiItemState & ODS_SELECTED) pDC->Draw3dRect(m_rcExpandCollapseBtn, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
			pnOld = pDC->SelectObject(&pnMarker);
			CPoint ptCenter = m_rcExpandCollapseBtn.CenterPoint();
			CPoint ptLeft   = ptCenter +CPoint(-4, m_bIsCollapsed ? -4:4);
			CPoint ptRight  = ptCenter +CPoint( 4, m_bIsCollapsed ? -4:4);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptLeft);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptRight);
			ptCenter.Offset(0, m_bIsCollapsed ?3:-3);
			ptLeft.Offset(0, m_bIsCollapsed ?3:-3);
			ptRight.Offset(0, m_bIsCollapsed ?3:-3);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptLeft);
			pDC->MoveTo(ptCenter);
			pDC->LineTo(ptRight);
			m_rcExpandCollapseBtn.left = rcHeader.left;
			
			// Now draw the lighter area for all items
			pDC->SelectObject(&pnBorder);
			pDC->SelectObject(&brItemArea);
			pDC->Rectangle(rcItem.left, rcHeaderTextArea.bottom-1, rcItem.right, rcItem.bottom);
			
			// Draw Items
			pDC->SelectObject(&pnMarker);
			if (!m_bIsCollapsed)
			{
				CRect rcItems;
				rcItems.SetRect(rcItem.left+1, rcHeaderTextArea.bottom, rcItem.right-1, rcHeaderTextArea.bottom);
				if (m_pImageList) m_pImageList->SetBkColor(clItemArea);
				for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
				{
					rcItems.top     = rcItems.bottom;
					rcItems.bottom  = rcItems.top + m_lstItems.GetAt(pos)->GetItemHeight(pDC, *m_pnStyle, NULL);
					m_lstItems.GetNext(pos)->OnDraw(pDC, rcItems, m_pImageList, *m_pnStyle, NULL, nOffsetY, pParent);
				}
			}
			pDC->SelectObject(fnOld);
			pDC->SelectObject(pnOld);
			pDC->SelectObject(brOld);
		}
	}
}

int CDirectUIGroup::AddItem(CDirectUIItem *pItem)
{
	m_lstItems.AddTail(pItem);
	HTHEME hTheme;
	hTheme = 0;
	CalcHeight();
	return m_lstItems.GetCount()-1;
}

void CDirectUIGroup::RemoveAllItems()
{
	while (m_lstItems.GetCount()) delete m_lstItems.RemoveHead();
	CalcHeight();
}

void CDirectUIGroup::CalcHeight()
{
	m_bCalcHeight = TRUE;
}

BOOL CDirectUIGroup::HitTestButton(CPoint point)
{
	if (*m_pnStyle == CWndDirectUI::styleOffice) return FALSE; // Not supported for Office style
	return (PtInRect(m_rcExpandCollapseBtn, point)) ? TRUE:FALSE;
}

CDirectUIItem* CDirectUIGroup::HitTest(CPoint point)
{
	if (m_bIsCollapsed) return FALSE;

	for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
	{
		CDirectUIItem* pItem = m_lstItems.GetNext(pos);
		if (pItem->HitTest(point)) return pItem;

	}

	return NULL;
}

void CDirectUIGroup::ParseToolbar(CToolBar *pToolbar)
{
	for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
	{
		CDirectUIItem* pItem = m_lstItems.GetNext(pos);
		pItem->ParseToolbar(pToolbar);
	}
}

BOOL CDirectUIGroup::Expand(BOOL bExpand)
{
	if (m_bIsCollapsed != bExpand) return FALSE;

	m_bIsCollapsed = !bExpand;
	CalcHeight();
	return TRUE;
}

void CDirectUIGroup::ToggleExpand()
{	
	m_bIsCollapsed = !m_bIsCollapsed;
	CalcHeight();
}

void CDirectUIGroup::InternalCalcHeight(CDC *pDC, HTHEME hTheme)
{
	if (!m_bCalcHeight) return;
	m_nHeight = GetSystemMetrics(SM_CYCAPTION);

#ifdef USE_THEMES
	// Make the title bar a little larger
	if ((*m_pnStyle == CWndDirectUI::styleThemed) & g_xpStyle.IsAppThemed()) 
		m_nHeight = MulDiv(m_nHeight, 9, 8);
#endif

	if (!m_bIsCollapsed)
		for (POSITION pos = m_lstItems.GetHeadPosition(); pos;)
		{
#ifdef USE_THEMES
			if ((*m_pnStyle == CWndDirectUI::styleThemed) & g_xpStyle.IsAppThemed()) 
				m_nHeight += m_lstItems.GetNext(pos)->GetItemHeight(pDC, *m_pnStyle, hTheme);
			else
#endif
				m_nHeight += m_lstItems.GetNext(pos)->GetItemHeight(pDC, *m_pnStyle, NULL);
		}
		m_bCalcHeight = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CDirectUIItem

int     CDirectUIItem::GetItemHeight(CDC* pDC, int nStyle, HTHEME hTheme) 
{
	if (m_nHeight == -1)
	{
		m_nHeight = MulDiv(GetSystemMetrics(SM_CYMENU), 9, 8);
	}
	return m_nHeight;
};

CDirectUIItem::CDirectUIItem(CString strName, UINT uiCommand, int nIconIndex)
{
	m_rcItem.SetRectEmpty();
	m_strName    = strName;
	m_nIconIndex = nIconIndex;
	m_uiCommand  = uiCommand;
	m_uiItemState= 0;
	m_nHeight    = -1;
	if (!m_hCursorHand) m_hCursorHand = AfxGetApp()->LoadCursor(IDC_MYHAND);
}

BOOL CDirectUIItem::EnableItem(BOOL bEnable)
{
	BOOL bIsEnabled;
	bIsEnabled = ! (m_uiItemState & ODS_DISABLED);
	if (bEnable == bIsEnabled) return FALSE;

	if (!bEnable) m_uiItemState |= ODS_DISABLED;
	if (bEnable)  m_uiItemState &= !ODS_DISABLED;
	return TRUE;
}

void CDirectUIItem::OnDraw(CDC* pDC, CRect rcItem, CImageList* pImageList, int nStyle, HTHEME hTheme, int nOffsetY, CWnd* pParent)
{
	const nBorderX     = 10;
	const nBorderIconX = 4;
	const nIconSize    = 16;

	switch (nStyle)
	{
	case CWndDirectUI::styleXP:
		{
			CGdiObject*  fnOld;
			
			rcItem.left  += nBorderX;
			rcItem.right -= nBorderX;
			
			// When there's an icon place the text more to the right
			if (m_nIconIndex != -1)
				rcItem.left += nIconSize+nBorderIconX;

			m_rcItem = rcItem;
			
			// Item selected? Then draw it brighter
			if (m_uiItemState & ODS_SELECTED)
				pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHT));
			else
				pDC->SetTextColor(CDirectUIGroup::MakeXPColor(GetSysColor(COLOR_HIGHLIGHT), 0.15));
			if (m_uiItemState & ODS_DISABLED) pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
			
			fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
			pDC->DrawText(m_strName, m_rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_CALCRECT);
			
			// Item selected? Then underline it (doing it this way I don't need to create an underlined font)
			if (m_uiItemState & ODS_SELECTED)
			{
				pDC->MoveTo(m_rcItem.left, m_rcItem.bottom);
				pDC->LineTo(m_rcItem.right, m_rcItem.bottom);
			}
			
			if ((m_nIconIndex != -1) && (pImageList))
			{
				// Finally draw the icon
				m_rcItem.left -= (nBorderIconX+nIconSize);
				if (m_uiItemState & ODS_SELECTED)
					pImageList->Draw(pDC, m_nIconIndex, CPoint(m_rcItem.left, rcItem.top+((rcItem.Height()-nIconSize)/2)), ILD_SELECTED);
				else
					pImageList->Draw(pDC, m_nIconIndex, CPoint(m_rcItem.left, rcItem.top+((rcItem.Height()-nIconSize)/2)), ILD_NORMAL);
			}
			
			pDC->SelectObject(fnOld);
		}
		break;
#ifdef USE_THEMES
	case CWndDirectUI::styleThemed:
		if (g_xpStyle.IsAppThemed() & (hTheme != 0))
		{
			// When there's an icon place the text more to the right
			if (m_nIconIndex != -1)
				rcItem.left += nIconSize+nBorderIconX;
			
			m_rcItem = rcItem;

			int nTextLen = strlen(m_strName); 
			int mlen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, NULL, 0); 
			WCHAR* output = new WCHAR[mlen];
			if(output)
			{
				CRect rc;
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, output, mlen);
				if (m_uiItemState & ODS_DISABLED)
				{
					CFont* fntOld = SetThemeFont(pDC, hTheme);
					pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
					pDC->SetBkMode(TRANSPARENT);
					pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
					g_xpStyle.GetThemeTextExtent(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0, output, -1, DT_LEFT|DT_VCENTER | DT_SINGLELINE|DT_END_ELLIPSIS, rcItem, m_rcItem);
					pDC->SelectObject(fntOld);
				}
				else
				{
					g_xpStyle.DrawThemeText(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0 , output, -1, DT_LEFT|DT_VCENTER | DT_SINGLELINE|DT_END_ELLIPSIS, DTT_GRAYED, &rcItem);
					g_xpStyle.GetThemeTextExtent(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0, output, -1, DT_LEFT|DT_VCENTER | DT_SINGLELINE|DT_END_ELLIPSIS, rcItem, m_rcItem);
				}
				delete output;
			}

			// Item selected? Then underline it (doing it this way I don't need to create an underlined font)
			if (m_uiItemState & ODS_SELECTED)
			{
				pDC->MoveTo(m_rcItem.left, m_rcItem.bottom);
				pDC->LineTo(m_rcItem.right, m_rcItem.bottom);
			}

			if ((m_nIconIndex != -1) && (pImageList))
			{
				// Finally draw the icon
				m_rcItem.left -= (nBorderIconX+nIconSize);
				if (m_uiItemState & ODS_SELECTED)
					pImageList->Draw(pDC, m_nIconIndex, CPoint(m_rcItem.left, rcItem.top+((rcItem.Height()-nIconSize)/2)), ILD_SELECTED|ILD_TRANSPARENT);
				else
					pImageList->Draw(pDC, m_nIconIndex, CPoint(m_rcItem.left, rcItem.top+((rcItem.Height()-nIconSize)/2)), ILD_NORMAL|ILD_TRANSPARENT);
			}

			break;
		}
#endif
	case CWndDirectUI::styleOffice:
		{
			CGdiObject*  fnOld;
			
			m_rcItem = rcItem;
			
			rcItem.left += nIconSize+nBorderIconX;
			fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetTextColor((m_uiItemState & ODS_DISABLED) ? GetSysColor(COLOR_GRAYTEXT) : CDirectUIGroup::MakeXPColor(GetSysColor(COLOR_HIGHLIGHT), 0.15));
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
			pDC->DrawText(m_strName, m_rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_CALCRECT);
			m_rcItem.OffsetRect(nIconSize+nBorderIconX, 0);
			
			if ((m_nIconIndex != -1) && (pImageList))
				pImageList->Draw(pDC, m_nIconIndex, CPoint(m_rcItem.left-nIconSize-nBorderIconX, rcItem.top+((rcItem.Height()-nIconSize)/2)), ILD_NORMAL);
			
			if ((m_uiItemState & ODS_SELECTED) & !(m_uiItemState & ODS_DISABLED))
			{
				pDC->MoveTo(m_rcItem.left, m_rcItem.bottom);
				pDC->LineTo(m_rcItem.right, m_rcItem.bottom);
			}
			
			pDC->SelectObject(fnOld);
		}
		break;
	case CWndDirectUI::styleXPclassic:
	default:
		{
			CGdiObject*  fnOld;
			
			
			rcItem.left  += nBorderX;
			rcItem.right -= nBorderX;
			if (m_nIconIndex != -1)
				rcItem.left += nIconSize+nBorderIconX;

			m_rcItem = rcItem;
			fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetTextColor((m_uiItemState & ODS_DISABLED) ? GetSysColor(COLOR_GRAYTEXT) : CDirectUIGroup::MakeXPColor(GetSysColor(COLOR_HIGHLIGHT), 0.15));
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
			pDC->DrawText(m_strName, m_rcItem, DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_CALCRECT);
			//m_rcItem.OffsetRect(nIconSize+nBorderIconX, 0);
			
			if ((m_nIconIndex != -1) && (pImageList))
				pImageList->Draw(pDC, m_nIconIndex, CPoint(m_rcItem.left-nIconSize-nBorderIconX, rcItem.top+((rcItem.Height()-nIconSize)/2)), ILD_NORMAL);
			
			if ((m_uiItemState & ODS_SELECTED) & !(m_uiItemState & ODS_DISABLED))
			{
				pDC->MoveTo(m_rcItem.left, m_rcItem.bottom);
				pDC->LineTo(m_rcItem.right, m_rcItem.bottom);
			}
			
			pDC->SelectObject(fnOld);
		} 
	}
}

BOOL CDirectUIItem::HitTest(CPoint point)
{
	if (m_uiItemState & ODS_DISABLED) return FALSE;
	return (PtInRect(m_rcItem, point)) ? TRUE:FALSE;
}

void CDirectUIItem::ParseToolbar(CToolBar *pToolbar)
{
	ASSERT(pToolbar);
	int nBitmap = pToolbar->SendMessage(TB_GETBITMAP, m_uiCommand, 0);
	if (nBitmap > 0) 
		m_nIconIndex = nBitmap; 
	else 
		m_nIconIndex = -1;
}


BOOL CDirectUIItem::MouseItemIn(CPoint point, CPoint ptOffset)
{
	m_uiItemState |= ODS_SELECTED;
	return TRUE;
}

BOOL CDirectUIItem::MouseItemOut(CPoint point, CPoint ptOffset)
{
	m_uiItemState &= ~ODS_SELECTED;
	return TRUE;
}

BOOL CDirectUIItem::MouseItemLDown(CPoint point, CPoint ptOffset)
{
	return FALSE;
}

BOOL CDirectUIItem::MouseItemLUp(CPoint point, CPoint ptOffset)
{
	//AfxGetMainWnd()->PostMessage(WM_COMMAND, m_uiCommand, 0);
	return FALSE;
}



/////////////////////////////////////////////////////////////////////////////
// CWndDirectUI

BEGIN_MESSAGE_MAP(CWndDirectUI, CWnd)
	//{{AFX_MSG_MAP(CWndDirectUI)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
#ifdef USE_THEMES
	ON_MESSAGE(WM_THEMECHANGED, OnThemeChanged)
#endif
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CWndDirectUI 

CWndDirectUI::CWndDirectUI() 
{
	m_pLastHitItem  = NULL;
	m_pLastHitGroup = NULL;
	m_hHand         = AfxGetApp()->LoadCursor(IDC_MYHAND);
	VERIFY(m_hHand);  // If asserts, add this cursor to your resource
	m_nStyle        = styleThemed;
	m_lstImages.Create(1, 1, ILC_COLOR,1, 1);
	RegisterWindowClass();
}

CWndDirectUI::~CWndDirectUI() 
{
	RemoveAll();
}


int CWndDirectUI::GetOffsetY()
{
	if (::GetWindowLong(m_hWnd, GWL_STYLE) & WS_VSCROLL) 
	{
		return GetScrollPos(SB_VERT);
	}
	else
		return 0;
}

int CWndDirectUI::GetOffsetX()
{
	return 0;
}

void CWndDirectUI::OnPaint() 
{
	const int nSpaceX = 10;
	const int nSpaceY = 10;

	CPaintDC dc(this); // device context for painting

	CRect rc;
	CRect rcItem;
	GetWindowRect(&rc);
	rc -= rc.TopLeft();

	BOOL bHasScrollbar;
	BOOL bNeedScrollbar;
	int  nOffsetY;

	bHasScrollbar  = ::GetWindowLong(m_hWnd, GWL_STYLE) & WS_VSCROLL ? TRUE:FALSE;
	if (bHasScrollbar) 
	{
		rc.right -= GetSystemMetrics(SM_CXHTHUMB);
		nOffsetY = GetOffsetY();
	}
	else
		nOffsetY = 0;

	rc.bottom +=nOffsetY; 
#if TRUE
	CMemDC MemDC(&dc, &rc);
	MemDC.SaveDC();
#else
	#define MemDC dc
#endif


	dc.SetWindowOrg(0, nOffsetY);

	HTHEME hTheme;
	hTheme = 0;

	switch (m_nStyle)
	{
	case styleXP:
		MemDC.FillSolidRect(rc, CDirectUIGroup::MakeXPColor(GetSysColor(COLOR_HIGHLIGHT), 0.4));
		break;
#ifdef USE_THEMES
	case styleThemed:
		if (g_xpStyle.IsAppThemed())
		{
			hTheme = g_xpStyle.OpenThemeData(GetSafeHwnd(), L"EXPLORERBAR");
			g_xpStyle.DrawThemeBackground(hTheme, MemDC.m_hDC, EBP_HEADERBACKGROUND, 0, &rc, 0);
			break;
		}
#endif
	case styleXPclassic:
	case styleOffice:
	default:
		MemDC.FillSolidRect(rc, GetSysColor(COLOR_WINDOW));
	}

	rcItem.SetRect(nSpaceX, 0, rc.right - nSpaceX, 0);
	for (POSITION pos = m_lstGroups.GetHeadPosition(); pos;)
	{
		CDirectUIGroup* pGroup = m_lstGroups.GetNext(pos);
		pGroup->InternalCalcHeight(&MemDC, hTheme);
		rcItem.top    = rcItem.bottom + nSpaceY;
		rcItem.bottom = rcItem.top + pGroup->m_nHeight;
		pGroup->OnDraw(&MemDC, rcItem, hTheme, nOffsetY, this);
	}

#ifdef USE_THEMES
	if (hTheme)
	{
		g_xpStyle.CloseThemeData(hTheme);
	}
#endif

	MemDC.RestoreDC(-1);

	// Show Scrollbar if needed
	bNeedScrollbar = ((rc.Height()-nOffsetY) < (rcItem.bottom+nSpaceY)) ? TRUE:FALSE;
	if (bNeedScrollbar != bHasScrollbar)
	{
		if (bNeedScrollbar) 
			ShowScrollBar(SB_VERT, TRUE);
		else
			ShowScrollBar(SB_VERT, FALSE);
		Invalidate();
	}
	
	if (bNeedScrollbar)
	{
		// Set Scrollbar range
		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask  = SIF_PAGE|SIF_RANGE;
		si.nMin   = 0;
		si.nMax   = rcItem.bottom+nSpaceY;
		si.nPage  = rc.Height()-nOffsetY;
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
}

BOOL CWndDirectUI::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CWndDirectUI::RemoveAll()
{
	m_pLastHitItem = NULL;
	m_pLastHitGroup = NULL;
	while (m_lstGroups.GetCount()) delete m_lstGroups.RemoveHead();
}

int CWndDirectUI::AddGroup(CString strName)
{
	CDirectUIGroup* pGroup = new CDirectUIGroup(strName, &m_lstImages, &m_nStyle);
	m_lstGroups.AddTail(pGroup);

	InvalidateIfPossible();
	
	return m_lstGroups.GetCount()-1;
}

BOOL CWndDirectUI::RemoveGroup(int nGroup)
{
	CDirectUIGroup* pGroup = GetGroupByNumber(nGroup);
	if (!pGroup) return FALSE;

	POSITION pos = m_lstGroups.Find(pGroup);
	ASSERT(pos);
	if (!pos) return FALSE;

	m_lstGroups.RemoveAt(pos);
	delete pGroup;

	InvalidateIfPossible();
	return TRUE;
}

CString& CWndDirectUI::GetGroupName(int nGroup)
{
	static CString strNull;
	CDirectUIGroup* pGroup = GetGroupByNumber(nGroup);
	if (!pGroup) return strNull;

	return pGroup->m_strName;
}

BOOL CWndDirectUI::ChangeGroupName(int nGroup, CString strName)
{
	CDirectUIGroup* pGroup = GetGroupByNumber(nGroup);
	if (!pGroup) return FALSE;

	pGroup->m_strName = strName;
	InvalidateIfPossible();

	return TRUE;
}

BOOL CWndDirectUI::ExpandGroup(int nGroup, BOOL bExpand)
{
	CDirectUIGroup* pGroup = GetGroupByNumber(nGroup);
	if (!pGroup) return FALSE;

	if (pGroup->Expand(bExpand)) InvalidateIfPossible();

	return TRUE;
}

BOOL CWndDirectUI::InitFromMenu(UINT id)
{
	// Main funtion! Use a 2D Menu
	RemoveAll();
	CMenu mnuGroups;
	MENUITEMINFO info;

	if (!mnuGroups.LoadMenu(id)) return FALSE;
	
	int i = 0;
	int j = 0;
	while (mnuGroups.GetSubMenu(i))
	{
		CMenu* pMenu = mnuGroups.GetSubMenu(i);
		CString strName;
		mnuGroups.GetMenuString(i, strName, MF_BYPOSITION);
		CDirectUIGroup* pGroup = new CDirectUIGroup(strName, &m_lstImages, &m_nStyle);

		info.cbSize = sizeof (MENUITEMINFO);
		info.fMask = MIIM_ID;

		j = 0;
		while (pMenu->GetMenuString(j, strName, MF_BYPOSITION))
		{
			pMenu->GetMenuItemInfo(j, &info, TRUE);
			pGroup->AddItem(new CDirectUIItem(strName, info.wID));
			j++;
		}

		m_lstGroups.AddTail(pGroup);
		i++;
	}

	InvalidateIfPossible();
	return TRUE;
}

BOOL CWndDirectUI::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_pLastHitItem != NULL)
	{
		SetCursor(m_pLastHitItem->GetCursor());
		return TRUE;
	}
	else
	if (((m_pLastHitGroup != NULL)) & (nHitTest == HTCLIENT))
	{
		SetCursor(m_hHand);
		return TRUE;
	}
	else
		return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CWndDirectUI::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//CWnd::OnLButtonDown(nFlags, point);
	if (m_pLastHitGroup)
	{
		m_pLastHitGroup->ToggleExpand();
		Invalidate();
	}

	if (m_pLastHitItem)
		if (m_pLastHitItem->MouseItemLDown(point, CPoint(GetOffsetX(), GetOffsetY()))) InvalidateIfPossible();
}

void CWndDirectUI::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Do the hoover thing
	CDirectUIItem*  pItem;
	CDirectUIGroup* pCurrentGroup;
	CDirectUIGroup* pGroup;
	pItem  = NULL;
	pGroup = NULL;

	if (::GetWindowLong(m_hWnd, GWL_STYLE) & WS_VSCROLL)
		point.y += GetScrollPos(SB_VERT);

	for (POSITION pos = m_lstGroups.GetHeadPosition(); pos;)
	{
		pCurrentGroup = m_lstGroups.GetNext(pos);
		if (pCurrentGroup->HitTestButton(point))
		{
			pGroup = pCurrentGroup;
			break;
		}
		pItem   = pCurrentGroup->HitTest(point);
		if (pItem != NULL) break;
	}

	if (pGroup != m_pLastHitGroup)
	{
		if (m_pLastHitGroup) m_pLastHitGroup->m_uiItemState &= ~ODS_SELECTED;
		m_pLastHitGroup = pGroup;
		if (m_pLastHitGroup) m_pLastHitGroup->m_uiItemState |= ODS_SELECTED;
		Invalidate();
	}
	else if (pItem  != m_pLastHitItem)	
	{
		// Draw Item
		BOOL bRedraw = FALSE;
		if (m_pLastHitItem) bRedraw |= m_pLastHitItem->MouseItemOut(point, CPoint(GetOffsetX(), GetOffsetY()));
		m_pLastHitItem = pItem;
		if (m_pLastHitItem) bRedraw |= m_pLastHitItem->MouseItemIn(point, CPoint(GetOffsetX(), GetOffsetY()));
		if (bRedraw) 
		{
			InvalidateIfPossible();
		}
	}

	//CWnd::OnMouseMove(nFlags, point);
}

void CWndDirectUI::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_pLastHitItem)
	{
		if (m_pLastHitItem->MouseItemLUp(point, CPoint(GetOffsetX(), GetOffsetY()))) InvalidateIfPossible();
		//GetParent()->PostMessage(WM_COMMAND, m_pLastHitItem->m_uiCommand, 0);
	}
	//CWnd::OnLButtonUp(nFlags, point);
}

CDirectUIGroup* CWndDirectUI::GetGroupByNumber(int nPos)
{
	if (nPos < 0) return NULL;

	CDirectUIGroup* pGroup;
	for (POSITION pos = m_lstGroups.GetHeadPosition(); pos;)
	{
		pGroup = m_lstGroups.GetNext(pos);
		if (nPos == 0) return pGroup;
		nPos--;
	}
	return NULL;
}

void CWndDirectUI::InvalidateIfPossible()
{
	if (IsWindow(m_hWnd)) Invalidate();
}

BOOL CWndDirectUI::SetToolbarImages(UINT uiToolbar, UINT uiAlternateImages, int nFlagColor, COLORREF clBackground, int nWidth)
{
	// Use this function after inserting all items and groups

	// Create an invisible toolbar and let it do all the work
	CToolBar tbToolbar;
	if (!tbToolbar.Create(this, WS_CHILD, 42)) return FALSE;
	if (!tbToolbar.LoadToolBar(uiToolbar)) return FALSE;

	m_lstImages.DeleteImageList();
	m_lstImages.Detach();

	if (uiAlternateImages == -1)
	{
		if (!m_lstImages.Create(CImageList::FromHandle((HIMAGELIST)tbToolbar.SendMessage(TB_GETIMAGELIST, 0, 0)))) return FALSE;
	}
	else
	{
		// Credits to Dany Cantin's "Very simple true color toolbar (It's true!)" where I
		// got the code for loading the hi-color toolbars

		CBitmap		cBitmap;
		BITMAP		bmBitmap;
		if (!cBitmap.Attach(LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(uiAlternateImages),
									  IMAGE_BITMAP, 0, 0,
									  LR_DEFAULTSIZE|LR_CREATEDIBSECTION)) ||
			!cBitmap.GetBitmap(&bmBitmap))
			return FALSE;
		CSize		cSize(bmBitmap.bmWidth, bmBitmap.bmHeight); 

		if (!m_lstImages.Create(nWidth, cSize.cy, nFlagColor|ILC_MASK, cSize.cx/nWidth, 0))
			return FALSE;

		if (m_lstImages.Add(&cBitmap, clBackground) == -1)
			return FALSE;
	}
	
	for (POSITION pos = m_lstGroups.GetHeadPosition(); pos;)
		m_lstGroups.GetNext(pos)->ParseToolbar(&tbToolbar);

	InvalidateIfPossible();
	return TRUE;
}

BOOL CWndDirectUI::AddItem(int nGroup, CDirectUIItem* pItem)
{
	CDirectUIGroup* pGroup = GetGroupByNumber(nGroup);
	if (!pGroup) return FALSE;
	pGroup->AddItem(pItem);
	InvalidateIfPossible();
	return TRUE;
}

BOOL CWndDirectUI::AddItemCommand(int nGroup, CString strItem, UINT uiCommand)
{
	CDirectUIItem* pItem = new CDirectUIItem(strItem, uiCommand);
	if (AddItem(nGroup, pItem)) return TRUE;
	else
	{
		delete pItem;
		return FALSE;
	}
}

BOOL CWndDirectUI::SetStyle(int nStyle)
{
	m_nStyle = nStyle;
	
	for (POSITION pos = m_lstGroups.GetHeadPosition(); pos;)
	{
		CDirectUIGroup* pGroup;
		pGroup = m_lstGroups.GetNext(pos);
		if (nStyle == styleOffice) 
			pGroup->Expand(TRUE); // Includes the CalcHeight(), so we don't have to do it again.
		else
			pGroup->CalcHeight();
	}

	InvalidateIfPossible();
	return TRUE;
}

int CWndDirectUI::GetStyle()
{
	return m_nStyle;
}


void CWndDirectUI::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	switch (nSBCode)
	{
	case SB_BOTTOM:
		nPos = GetScrollLimit(SB_VERT);
		break;
	case SB_ENDSCROLL:   
		return;
	case SB_LINEDOWN:
		nPos = GetScrollPos(SB_VERT) +1;
		break;
	case SB_LINEUP:
		nPos = GetScrollPos(SB_VERT) -1;
		break;
	case SB_PAGEDOWN:
		nPos = GetScrollPos(SB_VERT) + MulDiv(GetSystemMetrics(SM_CYMENU), 9, 8);
		break;
	case SB_PAGEUP:
		nPos = GetScrollPos(SB_VERT) - MulDiv(GetSystemMetrics(SM_CYMENU), 9, 8);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		break;
	case SB_TOP:
		nPos = 0;
		break;
	}
	Invalidate();
	SetScrollPos(SB_VERT, nPos, TRUE);
}

BOOL CWndDirectUI::Create(const RECT &rect, CWnd *pParentWnd, UINT nID)
{
	return CWnd::Create(NULL, ADVANCED_UI_CLASSNAME, WS_CHILD|WS_VISIBLE, rect, pParentWnd, nID);
}

#ifdef USE_THEMES
LRESULT CWndDirectUI::OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
	InvalidateIfPossible();
	return 0;
}
#endif

void CWndDirectUI::EnableItem(UINT uiCommand, BOOL bEnable)
{
	BOOL bRepaint;
	bRepaint = FALSE;
	for (POSITION pos = m_lstGroups.GetHeadPosition(); pos;)
	{
		bRepaint |= (m_lstGroups.GetNext(pos))->EnableItem(uiCommand, bEnable);
	}
	if (bRepaint) InvalidateIfPossible();
}


int CWndDirectUI::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

BOOL CWndDirectUI::RegisterWindowClass()
{
    WNDCLASS wndcls;
    HINSTANCE hInst = AfxGetInstanceHandle();

    if (!(::GetClassInfo(hInst, ADVANCED_UI_CLASSNAME, &wndcls)))
    {
        // otherwise we need to register a new class
        wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wndcls.lpfnWndProc      = ::DefWindowProc;
        wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
        wndcls.hInstance        = hInst;
        wndcls.hIcon            = NULL;
        wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
        wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wndcls.lpszMenuName     = NULL;
        wndcls.lpszClassName    = ADVANCED_UI_CLASSNAME;

        if (!AfxRegisterClass(&wndcls))
        {
            AfxThrowResourceException();
            return FALSE;
        }
    }

    return TRUE;
}

#ifdef USE_THEMES
CFont* CDirectUIItem::SetThemeFont(CDC *pDC, HTHEME hTheme)
{
	if (m_fntThemedText.m_hObject == 0)
	{
		struct _FONT_STRUCT
		{
			LOGFONT lfFont;
			BYTE    bufer[255];
		} fontbuffer;

		ZeroMemory(&fontbuffer, sizeof(fontbuffer));
		if (S_OK != g_xpStyle.GetThemeFont(hTheme, pDC->m_hDC,EBP_NORMALGROUPBACKGROUND,0,TMT_FONT, &fontbuffer.lfFont))
		{
			CFont::FromHandle((HFONT) GetStockObject(ANSI_VAR_FONT))->GetLogFont(&fontbuffer.lfFont);
		}
		m_fntThemedText.CreateFontIndirect(&fontbuffer.lfFont);
	}
	return pDC->SelectObject(&m_fntThemedText);
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CDirectUIItemStatic

void CDirectUIItemStatic::OnDraw(CDC* pDC, CRect rcItem, CImageList* pImageList, int nStyle, HTHEME hTheme, int nOffsetY, CWnd* pParent)
{
	const nBorderX     = 10;
	const nBorderIconX = 4;
	const nIconSize    = 16;

	rcItem.top += STATIC_DELTA_Y;

	switch (nStyle)
	{
	case CWndDirectUI::styleXP:
		{
			CGdiObject*  fnOld;
			
			rcItem.left  += nBorderX;
			rcItem.right -= nBorderX;
			
			m_rcItem = rcItem;
			
			pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHT));
			
			fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_END_ELLIPSIS);
			pDC->DrawText(m_strName, m_rcItem, DT_LEFT|DT_END_ELLIPSIS|DT_CALCRECT);
			
			pDC->SelectObject(fnOld);
		}
		break;
#ifdef USE_THEMES
	case CWndDirectUI::styleThemed:
		if (g_xpStyle.IsAppThemed() & (hTheme != 0))
		{
			// When there's an icon place the text more to the right
			m_rcItem = rcItem;

			int nTextLen = strlen(m_strName); 
			int mlen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, NULL, 0); 
			WCHAR* output = new WCHAR[mlen];
			if(output)
			{
				CRect rc;
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, output, mlen);
				{
					g_xpStyle.DrawThemeText(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0 , output, -1, DT_LEFT|DT_END_ELLIPSIS, 0, &rcItem);
					g_xpStyle.GetThemeTextExtent(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0, output, -1, DT_LEFT|DT_END_ELLIPSIS, rcItem, m_rcItem);
				}
				delete output;
			}
			break;
		}
#endif
	case CWndDirectUI::styleOffice:
	case CWndDirectUI::styleXPclassic:
	default:
		{
			CGdiObject*  fnOld;
			
			m_rcItem = rcItem;
			
			fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetTextColor((m_uiItemState & ODS_DISABLED) ? GetSysColor(COLOR_GRAYTEXT) : CDirectUIGroup::MakeXPColor(GetSysColor(COLOR_HIGHLIGHT), 0.15));
			pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_END_ELLIPSIS);
			pDC->DrawText(m_strName, m_rcItem, DT_LEFT|DT_END_ELLIPSIS|DT_CALCRECT);
			m_rcItem.OffsetRect(nIconSize+nBorderIconX, 0);
			
			pDC->SelectObject(fnOld);
		} 
	}
}

int     CDirectUIItemStatic::GetItemHeight(CDC* pDC, int nStyle, HTHEME hTheme)
{
	ASSERT(pDC);

	if (m_nHeight == -1)
	{
		switch (nStyle)
		{
#ifdef USE_THEMES
		case CWndDirectUI::styleThemed:
			if (g_xpStyle.IsAppThemed() & (hTheme != 0))
			{
				int nTextLen = strlen(m_strName); 
				int mlen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, NULL, 0); 
				WCHAR* output = new WCHAR[mlen];
				if(output)
				{
					CRect rc;
					CRect rc2;
					rc.SetRect(0, 0, 1000, 1000);
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, output, mlen);
					g_xpStyle.GetThemeTextExtent(hTheme, pDC->m_hDC, EBP_NORMALGROUPBACKGROUND, 0, output, -1, DT_LEFT|DT_END_ELLIPSIS, rc, rc2);
					m_nHeight = rc2.Height()+STATIC_DELTA_Y+STATIC_DELTA_Y;
				}
				delete output;
				break;
			}
#endif
		default:
			{
				m_nHeight = MulDiv(GetSystemMetrics(SM_CYMENU), 9, 8);

				CGdiObject*  fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
				CRect rc;
				rc.SetRect(0, 0, 1000, 1000);
				pDC->DrawText(m_strName, rc, DT_LEFT|DT_END_ELLIPSIS|DT_CALCRECT);
				pDC->SelectObject(fnOld);
				m_nHeight = rc.Height()+STATIC_DELTA_Y+STATIC_DELTA_Y;
			}
		}
	}
	return m_nHeight;
}



/////////////////////////////////////////////////////////////////////////////
// CDirectUIItemEdit
void    CDirectUIItemEdit::OnDraw(CDC* pDC, CRect rcItem, CImageList* pImageList, int nStyle, HTHEME hTheme, int nOffsetY, CWnd* pParent)
{
	const nBorderX     = 10;
	// Draw like the edit control is displayed...
	rcItem.left+= nBorderX;
	rcItem.right-= nBorderX;
	m_rcItem = rcItem;

	switch (nStyle)
	{
	case CWndDirectUI::styleXP:
		{
			pDC->Draw3dRect(rcItem, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));
			rcItem.DeflateRect(1, 1, 1, 1);
			pDC->FillSolidRect(rcItem, CDirectUIGroup::MakeXPColor(GetSysColor(COLOR_3DFACE), 0.5));
			rcItem.DeflateRect(1, 1, 1, 1);
			pDC->SetTextColor(GetSysColor(COLOR_BTNTEXT));
			CGdiObject* fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(m_strName, rcItem, DT_LEFT|DT_SINGLELINE);
			pDC->SelectObject(fnOld);
			rcItem.InflateRect(2, 2, 2, 2);
		}
		break;
#ifdef USE_THEMES
	case CWndDirectUI::styleThemed:
		if (g_xpStyle.IsAppThemed())
		{
			HTHEME hTheme2 = g_xpStyle.OpenThemeData(pParent->GetSafeHwnd(), L"EDIT");
			if (hTheme2)
			{
				int nTextLen = strlen(m_strName); 
				int mlen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, NULL, 0); 
				WCHAR* output = new WCHAR[mlen];
				if(output)
				{
					CRect rc;
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, m_strName, nTextLen + 1, output, mlen);
					{
						g_xpStyle.DrawThemeBackground(hTheme2, pDC->m_hDC, EP_EDITTEXT, 0 , &rcItem, &rcItem);
						rcItem.DeflateRect(1, 1, 1, 1);
						g_xpStyle.DrawThemeText(hTheme2, pDC->m_hDC, EP_EDITTEXT, 0 , output, -1, DT_LEFT|DT_END_ELLIPSIS, 0, &rcItem);
					}
					delete output;
				}
				g_xpStyle.CloseThemeData(hTheme2);
				break;
			}
		}
#endif
	case CWndDirectUI::styleOffice:
	case CWndDirectUI::styleXPclassic:
	default:
		{
			pDC->Draw3dRect(m_rcItem, GetSysColor(COLOR_WINDOWFRAME), GetSysColor(COLOR_WINDOWFRAME));
			m_rcItem.DeflateRect(1, 1, 1, 1);
			pDC->FillSolidRect(m_rcItem, GetSysColor(COLOR_WINDOW));
			m_rcItem.DeflateRect(1, 1, 1, 1);
			pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			CGdiObject* fnOld = pDC->SelectStockObject(ANSI_VAR_FONT);
			pDC->SetBkMode(TRANSPARENT);
			pDC->DrawText(m_strName, m_rcItem, DT_LEFT|DT_SINGLELINE);
			pDC->SelectObject(fnOld);
		} 
		break;
	}
	m_rcItem = rcItem;
}

int     CDirectUIItemEdit::GetItemHeight(CDC* pDC, int nStyle, HTHEME hTheme)
{
	if (m_nHeight == -1)
	{
		m_nHeight = MulDiv(GetSystemMetrics(SM_CYMENU), 9, 8);
	}
	return m_nHeight;
}

BOOL CDirectUIItemEdit::MouseItemIn(CPoint point, CPoint ptOffset)
{
	if (!m_pEdit)
	{
		m_pEdit =  new CInPlaceEdit2(GetText(), this);
		CRect rc(m_rcItem);
		rc.OffsetRect(-ptOffset.x, -ptOffset.y);
		m_pEdit->Create(WS_BORDER|WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, rc, m_pParent, 4001);
	}
	return FALSE;
}

BOOL CDirectUIItemEdit::MouseItemOut(CPoint point, CPoint ptOffset)
{
	if (m_pEdit)
		m_pEdit->DestroyWindow();

	return FALSE;
}

CDirectUIItemEdit::CDirectUIItemEdit(CString strText, CWnd* pParent) : CDirectUIItem(strText, -1, -1) 
{	
	if (!m_hCursorBeam) 
		m_hCursorBeam = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);

	m_pParent = pParent;
	m_pEdit = NULL;

};

CDirectUIItemEdit::~CDirectUIItemEdit()
{
	if (m_pEdit)
		m_pEdit->DestroyWindow();
}
