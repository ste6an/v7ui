// MyControlBar.h: interface for the CMyControlBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYCONTROLBAR_H__B5C2495B_38B0_4ADD_A0AF_26CE2755C577__INCLUDED_)
#define AFX_MYCONTROLBAR_H__B5C2495B_38B0_4ADD_A0AF_26CE2755C577__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMyControlBar : public SECControlBar  
{
public:
	CMyControlBar();
	virtual ~CMyControlBar();
	CWnd* m_pChild;
	int CreateControl(LPCTSTR className);

	virtual void  DoPaint(class CDC *);
	virtual int Create(CWnd* pWnd, const char* cCaption);
	virtual void OnSize(UINT nType, int cx, int cy) ;
	virtual void OnBarDock();
	virtual void OnBarFloat();
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode );
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz );
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	

};

#endif // !defined(AFX_MYCONTROLBAR_H__B5C2495B_38B0_4ADD_A0AF_26CE2755C577__INCLUDED_)
