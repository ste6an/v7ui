// ControlBarContext.h: interface for the CControlBarContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONTROLBARCONTEXT_H__908E9D08_1B66_4831_8D1A_C285B101DDA3__INCLUDED_)
#define AFX_CONTROLBARCONTEXT_H__908E9D08_1B66_4831_8D1A_C285B101DDA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyControlBar.h"

class CControlBarContext : public CContextImpl<CControlBarContext>  
{
	friend class CWndDirectUI;
public:
	CControlBarContext();
	virtual ~CControlBarContext();
	BL_BEGIN_CONTEXT("DockPanel","—тыкуема€ѕанель");
	BL_PROC(Dock,"ѕристыковать",1);
	BL_FUNC(CreateControl,"—оздатьЁлемент”правлени€",2);
	BL_FUNC(CreateExplorerBar,"—оздатьѕанельƒействий",1);
	BL_PROP_RW(Visible,"¬идимость");
	BL_PROP_WRITE(Visible);
	BL_PROP_RW(Caption,"«аголовок");
	BL_PROP_WRITE(Caption);
	BL_END_CONTEXT();
private:
	CMyControlBar* m_pCBar;
};

class CBLMyEx:public CBLExportContext
{
public:
	static long DispParamToValueEx(CValue** ppVal, DISPPARAMS* pDisp,unsigned int* pUI)
	{
		return static_cast<CBLMyEx*>(NULL)->DispParamToValue(ppVal, pDisp,pUI);
	}
	int  TryReturnByRefEx(CValue ** ppVal,DISPPARAMS* pDisp)
	{
		return TryReturnByRef(ppVal,pDisp);
	}
	static int  ValueToVariantEx(CValue& Val,tagVARIANT * pVariant){
		return static_cast<CBLMyEx*>(NULL)->ValueToVariant(Val,pVariant);
	}
};

#endif // !defined(AFX_CONTROLBARCONTEXT_H__908E9D08_1B66_4831_8D1A_C285B101DDA3__INCLUDED_)
