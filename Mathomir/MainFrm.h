// MainFrm.h : interface of the CMainFrame class
//


#pragma once

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//afx_msg void OnSysKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
	afx_msg void OnSysCommand(UINT nID,LPARAM lParam);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnActivateApp(BOOL bActive,DWORD dwThreadID);
	afx_msg LRESULT OnMenuChar(UINT nChar,UINT nFlags,CMenu* pMenu);

	int StartMyPainting(CDC *DC,int width, int above, int below,int color=0);
	int MyPolyline(CDC * DC, LPPOINT points, int count, int LineWidth, char IsBlue);
	int EndMyPainting(CDC * DC, int X , int Y, int force_black=0,int flip_image=0);
	int MyMoveTo(CDC * DC, int X, int Y);
	int MyLineTo(CDC * DC, int X, int Y, char IsBlue);
	int MyArc(CDC * DC, int X1, int Y1, int X2, int Y2, int Xstart, int Ystart, int Xend, int Yend, char IsBlue);
	int MySetPixel(CDC * DC, int X, int Y, char IsBlue);
	int MyBitBlt(CDC * DC, int X, int Y, int width, int height, int Xsrc, int Ysrc, char IsXInvers);
	char * XML_search(char * text, char * file);
	int ClearDocument(void);
	char * XML_read_attribute(char * attribute, char * value, char * file,int value_buffer_size);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	int AdjustMenu(int adjust_undo_only=0);
	int MyCircle(CDC * DC, int X1, int Y1, int X2, int Y2, int PenWidth, char IsBlue);
	int UndoInit();
	int UndoRestore();
	int UndoSave(char *undo_text,int unique_ID=-1);
	int UndoCheckText(char *text,int unique_ID=-1);
	int UndoDisableSaving();
	int UndoEnableSaving();
	int ReleaseMyPainting(void);
	int UndoRelease(int exit_application=0);
	int RearangeObjects(int delta);
	void SetFontsToDefaults(void);
	void GetLogicalFont(int font_no,LOGFONT *lf,CDC *DC);
	void SetLogicalFont(int font_no,LOGFONT *lf,CDC *DC);

	afx_msg void OnUpdateEditImage(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditSaveequationimage(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditCopymathmlcode(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditCopylatexcode(CCmdUI *pCmdUI);
	afx_msg void CMainFrame::OnDisplayChange(WPARAM wp, LPARAM lp);
};


