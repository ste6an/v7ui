// LayoutMgrContext.h: interface for the CLayoutMgrContext class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYOUTMGRCONTEXT_H__87CA00F2_4014_44BF_82BB_2C98F3117149__INCLUDED_)
#define AFX_LAYOUTMGRCONTEXT_H__87CA00F2_4014_44BF_82BB_2C98F3117149__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LayoutManager.h"

class CLayoutMgrContext : public CContextImpl<CLayoutMgrContext> 
{
public:
	CLayoutMgrContext();
	virtual ~CLayoutMgrContext();

	BL_BEGIN_CONTEXT("LayoutManager", "LayoutManager");
	BL_PROC(SetForm,"SetForm",1);
	BL_PROC(Attach,"Attach",0);
	BL_PROC(SetConstraint,"Привязать",4);
	BL_PROC(RedrawLayout,"RedrawLayout",0);
	BL_PROC_WITH_DEFVAL(SetMinMax,"SetMinMax",5);
	BL_DEFVAL_FOR(SetMinMax)
	{
		*pValue=0L;
		return TRUE;
	}
	BL_END_CONTEXT();
private:
	COXLayoutManager* m_pLMrg;
	CGetDoc7* m_pDoc;
};

#endif // !defined(AFX_LAYOUTMGRCONTEXT_H__87CA00F2_4014_44BF_82BB_2C98F3117149__INCLUDED_)
