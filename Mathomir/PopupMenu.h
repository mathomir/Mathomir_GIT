#pragma once
#include "Expression.h"
#include "Element.h"


// PopupMenu

class PopupMenu : public CWnd
{
	DECLARE_DYNAMIC(PopupMenu)


public:
	PopupMenu();
	virtual ~PopupMenu();
	CExpression *m_Expression;
	int m_IsFirstPass;
	int m_HaveCutDel;
	int m_MenuType;
	int m_NumOptions;
	int m_SizeX;
	int m_SizeY;
	int m_SelectedOption,m_prevSelectedOption,m_SelectedSuboption,m_prevSelectedSuboption;
	CWnd *m_Owner;
	int m_OwnerType;
	int m_UserParam;
	tElementStruct *m_theSelectedElement;
	int StartExtendedSelection;
	int EndExtendedSelection;
	int LevelExtendedSelection;
	int MovingMode;

int Popup_CursorX;
int Popup_CursorY;
unsigned char entry_box_first_call;
char PopupMenuSecondPassChoosing;
CEdit *ValueEntryBox;
int ValueEntryBoxData;
char ValueEntryBoxString[300];

	struct POPUPMENU_OPTION
	{
		char Text[40];
		short X;
		short Y;
		short Cx;
		short Cy;
		char IsEnabled;
		char IsChecked;  //can be checked / unchecked
		short IsButton;  //Button option
		CExpression *Graphics; //non-null if has graphics representation (an equation)
		char IsGraphicsSensitive; //if it can be pointed at some particular point of the graphics
		int Data; //any valuable data
		int DataArray[6];
		CExpression *Parent; //the original that will be replaced if menu item is chosen
	} Options[64];

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	int PaintThePopupMenu(void);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	int ShowPopupMenu(CExpression* expression,CWnd *owner, int OwnerType, int UserParam,int no_reposition=0);
	int PrepareFontMenu(int y);
	int PrepareSymbolMenu(int y);
	int PrepareParenthesesMenu(int y);
	int PrepareConditionListMenu();
	int HidePopupMenu(void);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	int AddMenuOption(int X, int Cx, char * text, int Data, int new_line);
	int AddMenuOptionButton(int X, char * text, int Data, int button_ndx, int new_line);
	int AddCheckedMenuOption(int X, int Cx, char * text, int is_checked, int Data, int new_line);
	int AddCheckedMenuOptionButton(int X, char * text, int is_checked, int Data, int button_ndx, int new_line);
	int UncheckOptions(int from, int to);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	int ExtractSelection(int StartPos,int EndPos,int * StartSel, int * EndSel);
	int SymbolicComputation(void);
	int AddMathMenuOption(CExpression * E1,CExpression *original=NULL);
	int PaintWorkIndicator(void);
};


