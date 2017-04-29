#pragma once

#include "Expression.h"
// CToolbox



class CToolbox : public CWnd
{
	DECLARE_DYNAMIC(CToolbox)
public:

	CToolbox *Subtoolbox;
	CToolbox *ContextMenu;
	CToolbox *Keyboard;
	CToolbox *Toolbar;

public:
	int m_IsSubtoolbox; //for submenu window
	int m_IsContextMenu; //for context menu window (right click)
	int m_IsKeyboard; //for virtual keyboard window
	int m_IsMain; //for main toolbox window
	int m_IsToolbar; //for the toolbar

public:
	int m_SelectedColor;
	int m_SelectedTextControl;
	int prevSelectedColor;
	int prevSelectedTextControl;
	int m_SelectedElement;
	int m_IsArrowSelected;
	int m_ContextMenuMember;
	int m_ContextMenuSubmember;
	int m_ContextMenuSelection;
	int m_FontModeElement;
	int m_FontModeSelection;
	int m_prevFontModeSelection;
	int m_KeyboardSmallCaps;
	int m_KeyboardElement;
	int m_ItemHeight;
	int m_LowHeightMode;

	int m_KeyboardX;
	int m_KeyboardY;
public:
	CToolbox(int IsSubtoolbox);
	virtual ~CToolbox();

protected:
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnPaint();
	void AdjustPosition(void);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//int AdjustToolboxParentheses(void);
	int AddSubmember(short Type,char data);
	int PaintToolboxElement(CDC * dc, int member , char IsArrowBlue);
	int PaintToolbar(CDC *dc);
	int ConfigureToolbar();
	int HideSubtoolbox(void);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	UINT KeyboardHit(UINT code,UINT Flags);
	int AddFontFormating(int FaceType, int IsBold, int IsItalic, int Modificator);
	void PaintToolboxHeader(CDC *dc);
	int DefineAcceleratorCode(short code);
	int AddKeyboardKey(int x, int y, char key);
	int AdjustKeyboardFont(void);
	void PaintKeyboardElement(CDC* dc, int element);
	void PaintColorbox(CDC* dc);
	void HideUnhideColorbox();
	void PaintTextcontrolbox(CDC* dc);
	void HideUnhideTextcontrolbox();
	int PopupCloses(int UserParam, int ExitCode);
	int AddAccelerator(short code);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnClose();
	int SaveSettings(char * filename);
	int LoadSettings(char * filename);
	afx_msg void OnSaveoptionsSaveasdefault();
	int AddDrawingSubmember(int form, int Cx, int Cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	int IsAcceleratorUsed(unsigned short Keycode);
	void ReformatKeyboardSelection(void);
	UINT GetUniformFormatting(void);
	int GetUniformFormattingColor(void);
	char IsUniformFormatting(void);
	UINT GetMixedFormatting(char key,char is_greek);
	CExpression *CheckForKeycodes(char *keystrokes,int *len);
	CExpression *ReturnKeycode(int keycode_order, char **Keycode);
	void ShowHelptext(char *text,char *command,char *accelerator,char *easycast, int language_code);
	void PickUpElementFromToolbox(int member, int submember);
	int InsertIntoToolbox(void);
	void ToolbarShowHelp(void);
	void UpdateToolbar(char force_redraw=0);
	int GetFormattingColor(void);
	void AutoResize(void);
	void ToolboxChangeIndividualKeyFont(void);
	UINT GetUppercaseFormatting(int key_code, UINT lowercase_formatting);
	//void MakeWheelChoice(void);

};


