#pragma once
#include "afxwin.h"


// CPasswordDlg dialog

class CPasswordDlg : public CDialog
{
	DECLARE_DYNAMIC(CPasswordDlg)

public:
	CPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPasswordDlg();
	virtual BOOL OnInitDialog();


// Dialog Data
	enum { IDD = IDD_DIALOG_PASSWORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit PasswordBox;
	afx_msg void OnBnClickedButton1();
	CEdit CommentBox;
	CEdit TimeLimitBox;
	afx_msg void OnBnClickedOk();
	CButton VisibilityButton;
	CButton OKButton;
};
