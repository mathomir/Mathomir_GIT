#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"

//HQZM (and HQZMp=HQZM/2) define zoom level when doing presentation-mode rendering
#define HQZM 256
#define HQZMp 128

#pragma warning(disable:4996)


class CMathomirApp : public CWinApp
{
public:
	CMathomirApp();

public:
	virtual BOOL InitInstance();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

//the TEACHER_VERSION switch enables certain options like digital exam
#define TEACHER_VERSION

extern HANDLE ProcessHeap;


//The main document strcture (the main document is stored as an array of these structures)
typedef struct THE_DOCUMENT
{
	CObject *Object;
	int absolute_X;
	int absolute_Y;
	int Checksum;  //calculated for Undo operation
	short Length;
	short Above;
	short Below;
	char MovingDotState;
	char Type; //1-expression, 2-drawing
} tDocumentStruct;



// *************************************
// initializes Undo memory
#define NUM_UNDO_LEVELS 5
typedef struct UNDO1
{
	void *data;
	int NumElements;
	char text[32];
} tUndoStruct;

typedef struct UNDO2
{
	CObject *pObject;
	CObject *pOriginal;
	short Type;
	int UsedInLevel;  //bitmask that tells us this object is used at what undo level
	int Checksum;
} tUndoObjectStruct;

extern tUndoStruct UndoStruct[];
extern int UndoNumLevels;
// ***********************************



#define NUM_COLORS 4
extern const unsigned int ColorTable[];

extern int NumDocumentElements;
extern int NumDocumentElementsReserved;
extern tDocumentStruct *TheDocument;
extern int ViewX;
extern int ViewY;
extern int ViewMaxX;
extern int ViewMaxY;
extern short ViewZoom;
extern CMathomirApp theApp;
extern int SmallCapsFactor;
extern int ToolboxSize;
extern int BaseToolboxSize;
extern int MouseMode;
extern CObject* KeyboardEntryObject;
extern tDocumentStruct* KeyboardEntryBaseObject;
extern int IsHighQualityRendering;
extern int IsHalftoneRendering;
extern int CenterParentheses_not_used; //not used any more
extern int DefaultParentheseType; //type of parenthese: 0-increasing in size, 1-aligned, 2-small
extern int FrameSelections;
extern int FixFontForNumbers;
extern int UseALTForExponents; //not used any more
extern int PaperWidth;
extern int PaperHeight;
extern RECT MainWindowRect;
extern int ImageSize;
extern int ForceHighQualityImage;
extern int ForceHalftoneImage;
extern int DefaultFontSize;
extern int MovingDotSize;
extern int MovingDotPermanent;
extern int IsDrawingMode; //if nonzero then we are drawing (1=rectangle,...)
extern int AccessLockedObjects; //can access locked object or not by mouse
extern int GRID;
//extern int SnapToGrid;
extern int IsShowGrid;
extern int IsSimpleVariableMode;
extern int IsMathDisabled;
extern char ImaginaryUnit;
extern int ShadowSelection;
extern int AutosaveOption;
extern int MoveCursorOnWheel;
extern int EnableMenuShortcuts;
extern int F1SetsZoom;
extern int PrintTextAsImage;
extern int UseCommaAsDecimal;
extern int UseWideCursor;
extern int UseCTRLForZoom;
extern int SnapToGuidlines;
extern char *LanguageStrings;
extern unsigned short *LanguagePointers;
extern int ViewOnlyMode;
extern int UseSpecialCapsLock;
extern int UseToolbar;
extern int ToolbarUseCross;
extern int ToolbarEditNodes;
extern int UseCapsLock;
extern int MouseWheelDirection;
extern int DefaultZoom;
extern int RightButtonTogglesWheel;
extern int WheelScrollingSpeed;
extern int PageNumeration;
extern int AutoResizeToolbox;
extern int UseComplexIndexes;
extern char NoImageAutogeneration;

//coloring definitions
#define BLUE_COLOR RGB(92,92,255)
#define GREEN_COLOR RGB(32,224,32)
#define SHADOW_BLUE_COLOR RGB(224,224,255)
#define SHADOW_BLUE_COLOR2 RGB(186,186,255)
#define SHADOW_BLUE_COLOR3 RGB(128,128,174)
#define DOCUMENT_AREA_BACKGROUND RGB(208,208,208)
#define PALE_RGB(x) ((((0xFF-((x>>16)&0xFF))/2+((x>>16)&0xFF))<<16)+(((0xFF-((x>>8)&0xFF))/2+((x>>8)&0xFF))<<8)+((0xFF-(x&0xFF))/2+(x&0xFF)))

#ifdef TEACHER_VERSION
typedef struct PUBLIC_KEY
{
	__int64 N;
	__int64 X;
} tPublicKey;
extern tPublicKey *PublicKey;
extern unsigned char TheTimeLimit;
extern unsigned char TheMathFlags;
extern DWORD TheExamStartTime;
extern unsigned char DisableEditing;
extern unsigned char WarningDisplayed;
#endif 

typedef struct PASSWORD_DLG_STRUCT
{
	char is_exam;
	char password[24];
	int time_limit;
	char disable_symbolic_math;
	char disable_math;
	char canceled;
} tPasswordDlgStruct;
extern tPasswordDlgStruct *PasswordDlgStruct;
extern char TheFileType;


HFONT GetFontFromPool(char Face, char Italic, char Bold, unsigned short Size);
HFONT GetFontFromPool(char combination, unsigned short Size);
void ClearFontPool();
HPEN GetPenFromPool(short width, char IsBlue,int color=0);
int PaintCheckedSign(CDC * DC, short x, short y, short size, char IsChecked);
void DisplayShortText(char* text, int x, int y,int LanguageID,int flags=0);
int AddDocumentObject(int type, int X, int Y);
int CopyTranslatedString(char *dest, const char* defstr, int id, int destlen);
void FatalErrorHandling();


//#define CRTDBG_MAP_ALLOC