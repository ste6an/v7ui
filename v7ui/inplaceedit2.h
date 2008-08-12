// InPlaceEdit2.h : header file
//
// Original class made by Zafir Anjum

/////////////////////////////////////////////////////////////////////////////
// CInPlaceEdit2 window

class CDirectUIItemEdit;
class CInPlaceEdit2 : public CEdit
{
// Construction
public:
	CInPlaceEdit2(CString sInitText, CDirectUIItemEdit* pParent);

// Attributes
public:

// Operations
public:

// Overrides
// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceEdit2)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInPlaceEdit2();

// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceEdit2)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	private:
	CString m_sInitText;
	BOOL    m_bESC; // To indicate whether ESC key was pressed
	CDirectUIItemEdit* m_pParent;
};