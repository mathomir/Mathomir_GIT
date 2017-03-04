/*****************************************  The MIT License ***********************************************

Copyright  2017, Danijel Gorupec

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the
following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN 
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*********************************************************************************************************/

#include "stdafx.h"
#include "Mathomir.h"
#include "PasswordDlg.h"
#include ".\passworddlg.h"


// CPasswordDlg dialog

IMPLEMENT_DYNAMIC(CPasswordDlg, CDialog)
CPasswordDlg::CPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPasswordDlg::IDD, pParent)
{
}

CPasswordDlg::~CPasswordDlg()
{
}

void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, PasswordBox);
	DDX_Control(pDX, IDC_EDIT1, CommentBox);
	DDX_Control(pDX, IDC_EDIT3, TimeLimitBox);
	DDX_Control(pDX, IDC_BUTTON1, VisibilityButton);
	DDX_Control(pDX, IDOK, OKButton);
}


BEGIN_MESSAGE_MAP(CPasswordDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CPasswordDlg message handlers
#pragma optimize("s",on)
BOOL CPasswordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	HICON icon=::LoadIcon(theApp.m_hInstance,MAKEINTRESOURCE(IDI_ICON3));
	VisibilityButton.SetIcon(icon);

	if (PasswordDlgStruct)
	{
		PasswordDlgStruct->canceled=1;
		PasswordBox.SendMessage(EM_SETPASSWORDCHAR,(WPARAM)'*',0);

		char str[192];
		CopyTranslatedString(str,"Password entry",5500,80);this->SetWindowText(str);
		CopyTranslatedString(str,"Password:",5501,50);this->GetDlgItem(IDC_STATIC1)->SetWindowText(str);
		CopyTranslatedString(str,"Time limit:",5502,60);this->GetDlgItem(IDC_STATIC2)->SetWindowText(str);
		CopyTranslatedString(str,"Disable calculator",5503,50);this->GetDlgItem(IDC_CHECK1)->SetWindowText(str);
		CopyTranslatedString(str,"Disable symbolic computation",5504,50);this->GetDlgItem(IDC_CHECK3)->SetWindowText(str);


		if (PasswordDlgStruct->is_exam==0)
		{
			RECT wr,cr;
			this->GetWindowRect(&wr);
			this->GetClientRect(&cr);
			wr.bottom-=wr.top;
			this->SetWindowPos(NULL,0,0,wr.right-wr.left,wr.bottom-cr.bottom+50,SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
#ifdef TEACHER_VERSION
			CopyTranslatedString(str,"Exam parameters",5505,80);this->SetWindowText(str);
			CopyTranslatedString(str,"Password is used to open exam results",5506,190);CommentBox.SetWindowText(str);

			//CommentBox.SetWindowText("Password is used to open exam results.");
			char buf[16];
			itoa(PasswordDlgStruct->time_limit,buf,10);
			TimeLimitBox.SetWindowText(buf);
			this->CheckDlgButton(IDC_CHECK1,PasswordDlgStruct->disable_math);
			this->CheckDlgButton(IDC_CHECK3,PasswordDlgStruct->disable_symbolic_math);
#endif
		}
		PasswordBox.SetFocus();
	}


	return 1;
}

void CPasswordDlg::OnBnClickedButton1()
{
	PasswordBox.SendMessage(EM_SETPASSWORDCHAR,0,0);
	PasswordBox.RedrawWindow();
	OKButton.SetFocus();
	PasswordBox.SetFocus();
}

void CPasswordDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	if (PasswordDlgStruct)
	{
		PasswordDlgStruct->canceled=0;
		PasswordBox.GetWindowText(PasswordDlgStruct->password,23);
		char buf[15];
		TimeLimitBox.GetWindowText(buf,15);
		PasswordDlgStruct->time_limit=atoi(buf);
		PasswordDlgStruct->disable_math=(this->IsDlgButtonChecked(IDC_CHECK1))?1:0;
		PasswordDlgStruct->disable_symbolic_math=(this->IsDlgButtonChecked(IDC_CHECK3))?1:0;
	}
	OnOK();
}
