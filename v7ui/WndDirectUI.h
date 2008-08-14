#if !defined(AFX_WNDJOBS_H__6AE715EA_69E6_4C3A_953C_42319E05E780__INCLUDED_)
#define AFX_WNDJOBS_H__6AE715EA_69E6_4C3A_953C_42319E05E780__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WndDirectUI.h : Header-Datei
//

#ifdef USE_THEMES
#ifndef _UXTHEME_H_
#include <uxtheme.h>
#endif
#else
#define HTHEME int
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED                 0x031A
#endif

#define STATIC_DELTA_Y  5

class CDirectUIItem: public CObject
{
public:
	virtual HCURSOR GetCursor() {return m_hCursorHand;};
	virtual BOOL MouseItemLUp(CPoint point, CPoint ptOffset, CWnd* pParent);
	virtual BOOL MouseItemLDown(CPoint point, CPoint ptOffset);
	virtual BOOL MouseItemOut(CPoint point, CPoint ptOffset);
	virtual BOOL MouseItemIn(CPoint point, CPoint ptOffset);

	virtual BOOL    EnableItem(BOOL bEnable);
	virtual void    ParseToolbar(CToolBar *pToolbar);
	virtual BOOL    HitTest(CPoint point);
	CDirectUIItem(CString strName, UINT uiCommand, int nIconIndex = -1);
	virtual void    OnDraw(CDC* pDC, CRect rcItem, CImageList* pImageList, int nStyle, HTHEME hTheme, int nOffsetY, CWnd* pParent);
	virtual int     GetItemHeight(CDC* pDC, int nStyle, HTHEME hTheme);
	virtual CString& GetText() {return m_strName;};
	virtual void SetText(const CString& strText) {m_strName = strText; m_nHeight = -1;};

	UINT    m_uiItemState;
	CString m_strName;
	int     m_nIconIndex;
	UINT    m_uiCommand;
	CValue	m_ItemValue;

protected:
	static HCURSOR m_hCursorHand;
#ifdef USE_THEMES
	static CFont m_fntThemedText;
	static CFont* SetThemeFont(CDC* pDC, HTHEME hTheme);
#endif
	CRect   m_rcItem;
	int     m_nHeight;
};

class CDirectUIItemStatic: public CDirectUIItem
{
public:
	virtual HCURSOR GetCursor() {return m_hCursorArrow;};
	virtual BOOL MouseItemLUp(CPoint point, CPoint ptOffset)  {return FALSE;};
	virtual BOOL MouseItemLDown(CPoint point, CPoint ptOffset) {return FALSE;};
	virtual BOOL MouseItemOut(CPoint point, CPoint ptOffset) {return FALSE;};
	virtual BOOL MouseItemIn(CPoint point, CPoint ptOffset) {return FALSE;};

	virtual void    ParseToolbar(CToolBar *pToolbar) {;};
	CDirectUIItemStatic(CString strText) : CDirectUIItem(strText, -1, -1) 
	{	if (!m_hCursorArrow) m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);};
	virtual void    OnDraw(CDC* pDC, CRect rcItem, CImageList* pImageList, int nStyle, HTHEME hTheme, int nOffsetY, CWnd* pParent);
	virtual int     GetItemHeight(CDC* pDC, int nStyle, HTHEME hTheme);
protected:
	static HCURSOR m_hCursorArrow;
};

class CInPlaceEdit2;
class CDirectUIItemEdit: public CDirectUIItem
{
	friend class CInPlaceEdit2;
public:
	virtual HCURSOR GetCursor() 
	{
		return m_hCursorBeam;
	};
	virtual BOOL MouseItemLUp(CPoint point, CPoint ptOffset)  {return FALSE;};
	virtual BOOL MouseItemLDown(CPoint point, CPoint ptOffset)  {return FALSE;};
	virtual BOOL MouseItemOut(CPoint point, CPoint ptOffset);
	virtual BOOL MouseItemIn(CPoint point, CPoint ptOffset);

	virtual void    ParseToolbar(CToolBar *pToolbar) {;};
	CDirectUIItemEdit(CString strText, CWnd* pParent);
	~CDirectUIItemEdit();
	virtual void    OnDraw(CDC* pDC, CRect rcItem, CImageList* pImageList, int nStyle, HTHEME hTheme, int nOffsetY, CWnd* pParent);
	virtual int     GetItemHeight(CDC* pDC, int nStyle, HTHEME hTheme);
protected:
	static HCURSOR  m_hCursorBeam;
	CInPlaceEdit2* m_pEdit;
	CWnd* m_pParent;
};


class CDirectUIGroup: public CObject
{
public:
	static  COLORREF MakeXPColor(COLORREF cl, double factor = 0.85);
	CDirectUIGroup(CString strName, CImageList* pList, int* pnStyle);
	~CDirectUIGroup();
	void    OnDraw(CDC* pDC, CRect rcItem, HTHEME hTheme, int nOffsetY, CWnd* pParent);

public:
	void InternalCalcHeight(CDC* pDC, HTHEME hTheme);
	BOOL    EnableItem(UINT uiCommand, BOOL bEnable);
	void    ToggleExpand();
	BOOL    Expand(BOOL bExpand);
	BOOL    HitTestButton(CPoint point);
	CDirectUIItem* HitTest(CPoint point);
	void    ParseToolbar(CToolBar* pToolbar);
	void    CalcHeight();
	void    RemoveAllItems();
	int     AddItem(CDirectUIItem* pItem);

	CImageList* m_pImageList;
	CString     m_strName;
	int         m_nHeight;
	UINT        m_uiItemState;

protected:
	CTypedPtrList<CObList, CDirectUIItem*> m_lstItems;
	int*        m_pnStyle;
	CRect       m_rcExpandCollapseBtn;
	BOOL        m_bIsCollapsed;
	BOOL		m_bCalcHeight;
};

/////////////////////////////////////////////////////////////////////////////
// Fenster CWndDirectUI 

class CExpBarContext;
class CWndDirectUI;

class CWndDirectUI : public CWnd
{
	friend class CExpBarContext;
public:

	// Konstruktion
	CWndDirectUI();
	CWndDirectUI(CExpBarContext* cont, CBLContext* pUDC);
	enum Styles {
		styleXP        =       0x01,
		styleOffice    =       0x02,
		styleXPclassic =       0x03,
		styleThemed    =       0x04
		};

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CWndDirectUI)
	//}}AFX_VIRTUAL

// Implementierung
public:
	BOOL RegisterWindowClass();
	void     EnableItem(UINT uiCommand, BOOL bEnable=TRUE);
	BOOL     Create(const RECT& rect, CWnd* pParentWnd, UINT nID);
	int      GetStyle();
	BOOL     SetStyle(int nStyle);
	BOOL     AddItemCommand(int nGroup, CString strItem, UINT uiCommand);
	BOOL     AddItem(int nGroup, CDirectUIItem* pItem);
	BOOL     SetToolbarImages(UINT uiToolbar, UINT uiAlternateImages = -1, int nFlagColor=ILC_COLOR8, COLORREF clBackground = RGB(255,0,255), int nWidth=16);
	BOOL     InitFromMenu(UINT id);
	BOOL     ExpandGroup(int nGroup, BOOL bExpand=TRUE);
	BOOL     ChangeGroupName(int nGroup, CString strName);
	CString& GetGroupName(int nGroup);
	BOOL     RemoveGroup(int nGroup);
	CDirectUIGroup* AddGroup(CString strName);
	void     RemoveAll();
	virtual ~CWndDirectUI();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	int GetOffsetX();
	int GetOffsetY();
	int             m_nStyle;
	CImageList      m_lstImages;
	HCURSOR         m_hHand;
	CDirectUIItem*  m_pLastHitItem;
	CDirectUIGroup* m_pLastHitGroup;

	CDirectUIGroup* GetGroupByNumber(int nPos);
	void            InvalidateIfPossible();
	//{{AFX_MSG(CWndDirectUI)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
#ifdef USE_THEMES
	afx_msg LRESULT OnThemeChanged(WPARAM wParam, LPARAM lParam);
#endif
	DECLARE_MESSAGE_MAP()

	CTypedPtrList<CObList, CDirectUIGroup*> m_lstGroups;
protected:
	CExpBarContext* m_pEBContext;
};

class CEBGroupContext : public CContextImpl<CEBGroupContext,no_init_done>
{
public:
	CEBGroupContext();
	CEBGroupContext(CString sCaption,CWndDirectUI* pExpBar);
	virtual ~CEBGroupContext();

	BL_BEGIN_CONTEXT("CEBGroupContext","CEBGroupContext");
	BL_FUNC_WITH_DEFVAL(AddItem,"AddItem",3);
	BL_DEFVAL_FOR(AddItem)
	{
		if(nParam==2)
		{
			*pValue=0L;
			return TRUE;
		}
		return FALSE;
	}
	BL_END_CONTEXT();

protected:
	CDirectUIGroup* m_pGroup;
};

class CEBItemContext : public CContextImpl<CEBItemContext,no_init_done>
{
public:
	CEBItemContext();
	CEBItemContext(CString sCaption, CDirectUIGroup* pGroup, int nIconIndex, CValue& value);
	virtual ~CEBItemContext();

	BL_BEGIN_CONTEXT("CEBItemContext","CEBItemContext");
	BL_END_CONTEXT();
protected:
	CDirectUIItem* m_pItem;
};

class CExpBarContext : public CContextImpl<CExpBarContext>, public CV7Control, public CV7ControlEx
{
	friend class CControlBarContext;
public:
	CExpBarContext();
	virtual ~CExpBarContext();
	
    //CV7ControlEx
    virtual BOOL InitControlEx(CBLContext* pForm, CGetDoc7* pDoc, CGetField* pField, CBLContext* pUDC, CValue* pCreateParam);
    virtual BOOL CreateControlWndEx(CWnd* pParent);
	
    //CV7Control
    virtual BOOL CreateControlWnd(CWnd* pParent, CGetDoc7* pDoc, CGetField* pGetField, CBLContext* pUDC);
	
	BL_BEGIN_CONTEXT("ExplorerBar", "ExplorerBar");
	BL_FUNC(AddGroup,"AddGroup",1);
	BL_PROC(SetStyle,"SetStyle",1);
	BL_END_CONTEXT();

	void OnItemClick(CDirectUIItem* pItem);

protected:
	CWndDirectUI* m_pExpBar;
	CGetDoc7* m_pDoc;
	CGetField* m_pField;
	CBLContext* m_pUDC;
	CCtrlEventManager m_EventManager;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_WNDJOBS_H__6AE715EA_69E6_4C3A_953C_42319E05E780__INCLUDED_
