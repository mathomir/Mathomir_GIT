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

#include "MathomirDoc.h"
#include "MathomirView.h"
#include ".\MathomirView.h"
#include "Expression.h"
#include "toolbox.h"
#include "popupmenu.h"
#include "mainfrm.h"
#include "math.h"
#include "atlimage.h"
#include "gdiplusimaging.h" 
#include "htmlhelp.h"
#include "drawing.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
int MouseWheelAnimation; //scrolling in Y direction
int MouseWheelAnimationX; //scrolling in X direction

int prevCursorX;
int prevCursorY;
int prevCursorSize;
CBitmap *CursorBitmap;

int prevCrossX,prevCrossY;
CBitmap *CrossBitmapX;
CBitmap *CrossBitmapY;
char prevCrossShown; 
char PreopenDrawingToolboxLock;

char ReenableMenu=0;
char IsSelectionFresh=0;

int SteadyCursorTimer;
char QuickTypeUsed;
char MouseTouchingSelection;
extern int SelectedLineColor;
extern int SelectedLineWidth;
char DisableMultitouch=0;
char TouchMouseMode=0;
char dont_empty_clipboard=0;
extern int ToolboxAutoopen;
extern CExpression* ClipboardExpression;
extern CExpression* prevClipboardExpression;
extern CDrawing* ClipboardDrawing;
extern CToolbox *Toolbox;
extern PopupMenu *Popup;
extern int PlusLevel;
extern int MulLevel;
extern int EqLevel;
extern int LastDrawingCreated;
extern unsigned char OperatorLevelTable[256];
#define GetOperatorLevel(x) ((int)OperatorLevelTable[(unsigned char)(x)])
int PrintRendering=0;
char MouseOverScrollbar=0;
RECT TheClientRect; 
CMathomirView *pMainView;
CView *pMainViewBase;
int QuickSelectActive=0;
int RootObjectCopy;
int RootObjectFontSize;
extern char TemporaryShowColoredSelection;
extern int EasycastListStart;
char MouseIsTouchingExpression;
char MouseHighVelocity;
char MouseSteady;
int PreopenDrawingToolboxX;
int PreopenDrawingToolboxY;
unsigned int PreopenDrawingToolboxTime;
extern CElement *SelectedTab;
extern int SelectedTabX;
PopupMenu *SearchPopup;
PopupMenu *ListPopup;
tDocumentStruct *LastQuickTypeObject;

int IsMenuAccessKey(UINT nChar);
int LastTakenChecksum;
char SpacebarPressedRecently;
int SpacebarPressedPos;

int RullerType=0; //0=horizontal ruller, 1=vertical ruller
int ShowRullerCounter=4;
int RullerGuidelineSelected=-1;
int RullerPositionPreselected=-1;
int SelectedGuidelineType=0;
int SingleSelectedTextObject=-1;
extern CEdit *StaticMessageWindow;
int PresentationModeActiveTimer;
tDocumentStruct *IsInsertionPointTouched;
int NewlineAddObject=-1;
int LeftClickTimer;
int prevNewlineAddObject=-1;
// menues
DWORD AutosaveTime;
DWORD AutosavePoints;
int AutosaveFreetime;
// CMathomirView

extern CExpression *AutocompleteSource;
//extern int DisableAutocomplete;
extern int AutocompleteTriggered;
extern CExpression *BackspaceStorage;
extern CExpression *LastCursorPositionExpression;

int ExecuteLink(char *command); 
unsigned int LongClickStartTime;
CPoint LongClickPosition;
CExpression *LongClickObject;
tDocumentStruct *prevTouchedObject=NULL;
char IsWindowOutOfFocus;

#define NUM_SPECIAL_KEYS 21
const UINT keystring[]={'0','1','2','3','4','5','6','7','8','9',VK_OEM_PLUS,VK_OEM_MINUS,VK_OEM_COMMA,VK_OEM_PERIOD,VK_OEM_1,VK_OEM_2,VK_OEM_3,VK_OEM_4,VK_OEM_5,VK_OEM_6,VK_OEM_7,0};
//UINT keystring2[NUM_SPECIAL_KEYS+1];
//UINT keystring3[NUM_SPECIAL_KEYS+1];
//char keystringtimer=6;
//char keystringmode=0;
//char keystringcnt=0;

struct
{
	int Show; //are tillens shown or not
	int StartX,StartY; //X and Y coordinates of the upper left corner of the first tillen
	int EndY; //Y coordinate of the bottom of the last tillen
	int Height,Width; //dimensions of a signle tillen (there is 5 pixels between-tillen distance in Y direction and 40 pixels distance in X direction)
	int Num, NumX, NumY; //overall number of tillens; number of tillens in X direction and Y direction. 
	int Zoom; //zoom level
	int Enlarge; //number of tillen that needs to be enlarged (otherse -1)
} Tillens;





CExpression *TheKeyboardClipboard;

IMPLEMENT_DYNCREATE(CMathomirView, CView)

BEGIN_MESSAGE_MAP(CMathomirView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_ZOOM_40, OnZoom40)
	ON_COMMAND(ID_ZOOM_60, OnZoom60)
	ON_COMMAND(ID_ZOOM_80, OnZoom80)
	ON_COMMAND(ID_ZOOM_100, OnZoom100)
	ON_COMMAND(ID_ZOOM_300, OnZoom300)
	ON_COMMAND(ID_ZOOM_400, OnZoom400)
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
	ON_WM_KEYUP()
	ON_WM_SYSKEYDOWN()
	ON_COMMAND(ID_HQ_REND, OnHqRend)
	ON_WM_SYSKEYUP()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONUP()
	ON_WM_NCRBUTTONDOWN()
	ON_COMMAND(ID_SELECTIONS_INTELLIGENTFRAMING, OnSelectionsIntelligentframing)
	ON_COMMAND(ID_KEYBOARD_FIXFONTFORNUMBERS, OnKeyboardFixfontfornumbers)
	ON_COMMAND(ID_VIEW_HALFTONERENDERING, OnViewHalftonerendering)
	ON_COMMAND(ID_PAGE_A4PORTRAIT, OnPageA4portrait)
	ON_COMMAND(ID_PAGE_A4LANDSCAPE, OnPageA4landscape)
	ON_COMMAND(ID_TOOLBOX_LARGE, OnToolboxLarge)
	ON_COMMAND(ID_TOOLBOX_MEDIUM, OnToolboxMedium)
	ON_COMMAND(ID_TOOLBOX_SMALL, OnToolboxSmall)
	ON_COMMAND(ID_SAVEOPTIONS_SAVEASDEFAULT, OnSaveoptionsSaveasdefault)
	ON_COMMAND(ID_SAVEOPTIONS_SAVEAS, OnSaveoptionsSaveas)
	ON_COMMAND(ID_SAVEOPTIONS_LOAD, OnSaveoptionsLoad)
	ON_COMMAND(ID_TEXTEND, OnTextend)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_IMAGE, OnEditCopyImage)
	ON_COMMAND(ID_OUTPUTIMAGE_SIZE200, OnOutputimageSize200)
	ON_COMMAND(ID_OUTPUTIMAGE_SIZE150, OnOutputimageSize150)
	ON_COMMAND(ID_OUTPUTIMAGE_SIZE100, OnOutputimageSize100)
	ON_COMMAND(ID_OUTPUTIMAGE_SIZE80, OnOutputimageSize80)
	ON_COMMAND(ID_OUTPUTIMAGE_FORCEHIGHQUALITY, OnOutputimageForcehighquality)
	ON_COMMAND(ID_OUTPUTIMAGE_FORCEHALFTONE, OnOutputimageForcehalftone)
	ON_COMMAND(ID_FONTSIZE_VERYLARGE, OnFontsizeVerylarge)
	ON_COMMAND(ID_FONTSIZE_LARGE, OnFontsizeLarge)
	ON_COMMAND(ID_FONTSIZE_NORMAL, OnFontsizeNormal)
	ON_COMMAND(ID_FONTSIZE_SMALL, OnFontsizeSmall)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_MOVINGDOT_LARGE, OnMovingdotLarge)
	ON_COMMAND(ID_MOVINGDOT_MEDIUM, OnMovingdotMedium)
	ON_COMMAND(ID_MOVINGDOT_SMALL, OnMovingdotSmall)
	ON_COMMAND(ID_MOVINGDOT_PERMANENT, OnMovingdotPermanent)
	ON_COMMAND(ID_HELP_QUICKGUIDE, OnHelpQuickguide)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_EDIT_ACCESSLOCKEDOBJECTS, OnEditAccesslockedobjects)
	ON_COMMAND(ID_GRID_FINE, OnGridFine)
	ON_COMMAND(ID_GRID_MEDIUM, OnGridMedium)
	ON_COMMAND(ID_GRID_COARSE, OnGridCoarse)
//	ON_COMMAND(ID_GRID_SNAPTOGRID, OnGridSnaptogrid)
	ON_COMMAND(ID_VIEW_SHOWGRID, OnViewShowgrid)
	ON_WM_CREATE()
	ON_COMMAND(ID_KEYBOARD_VERYSIMPLEVARIABLEMODE, OnKeyboardVerysimplevariablemode)
	ON_COMMAND(ID_SYMBOLICCALCULATOR_ENABLE, OnSymboliccalculatorEnable)
	ON_COMMAND(ID_IMAGINARYUNIT_I, OnImaginaryunitI)
	ON_COMMAND(ID_IMAGINARYUNIT_J, OnImaginaryunitJ)
	ON_COMMAND(ID_EDIT_SAVEEQUATIONIMAGE, OnEditSaveequationimage)
	ON_COMMAND(ID_SELECTIONS_SHADOWSELECTIONS, OnSelectionsShadowselections)
	ON_COMMAND(ID_AUTOSAVE_NEVER, OnAutosaveNever)
	ON_COMMAND(ID_AUTOSAVE_LOW, OnAutosaveLow)
	ON_COMMAND(ID_AUTOSAVE_MEDIUM, OnAutosaveMedium)
	ON_COMMAND(ID_AUTOSAVE_HIGH, OnAutosaveHigh)
	ON_COMMAND(ID_AUTOSAVE_LOADTHEAUTOSAVE, OnAutosaveLoadtheautosave)
	ON_COMMAND(ID_ZOOM_WHEELZOOMADJUSTSPOINTER, OnZoomWheelzoomadjustspointer)
	ON_COMMAND(ID_KEYBOARD_GENERALVARIABLEMODE, OnKeyboardGeneralvariablemode)
	ON_COMMAND(ID_KEYBOARD_ALTMENUSHORTCUTS, OnKeyboardAltmenushortcuts)
	ON_COMMAND(ID_VIEW_ZOOMIN, OnViewZoomin)
	ON_COMMAND(ID_VIEW_ZOOMOUT, OnViewZoomout)
	ON_COMMAND(ID_FONTFACES_FONT1, OnFontfacesFont1)
	ON_COMMAND(ID_FONTFACES_FONT2, OnFontfacesFont2)
	ON_COMMAND(ID_FONTFACES_FONT3, OnFontfacesFont3)
	ON_COMMAND(ID_FONTFACES_FONT4, OnFontfacesFont4)
	ON_COMMAND(ID_FONTFACES_SETFONTSTODEFAULLTS, OnFontfacesSetfontstodefaullts)
	ON_COMMAND(ID_EDIT_COPYLATEXCODE, OnEditCopylatexcode)
	ON_COMMAND(ID_KEYBOARD_F1SETSZOOMLEVELTO100, OnKeyboardF1setszoomlevelto100)
	ON_COMMAND(ID_VIEW_ZOOMTO1, OnViewZoomto1)
	ON_COMMAND(ID_OUTPUTIMAGE_PRINTFONTSASIMAGES, OnOutputimagePrintfontsasimages)
	ON_WM_SETCURSOR()
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_KEYBOARD_ALLOWCOMMAASDECIMALSEPARATOR, OnKeyboardAllowcommaasdecimalseparator)
	ON_COMMAND(ID_GRIDANDGUIDELINES_SNAPTOGUIDELINES, OnGridandguidelinesSnaptoguidelines)
	ON_COMMAND(ID_ZOOM_USECTRLFORWHEELZOOM, OnZoomUsectrlforwheelzoom)
	ON_COMMAND(ID_Menu32880, &CMathomirView::OnMenu32880)
	ON_COMMAND(ID_TOOLBOXANDCONTEXTMENU_SHOWTOOLBAR, &CMathomirView::OnToolboxandcontextmenuShowtoolbar)
	ON_COMMAND(ID_PAGE_LETTERPORTRAIT, &CMathomirView::OnPageLetterportrait)
	ON_COMMAND(ID_PAGE_LETTERLANDSCAPE, &CMathomirView::OnPageLetterlandscape)
	ON_COMMAND(ID_KEYBOARD_USECAPSLOCKTOTOGGLETYPINGMODE, &CMathomirView::OnKeyboardUsecapslocktotoggletypingmode)
	ON_COMMAND(ID_TOOLBOXANDCONTEXTMENU_GIGANTIC, &CMathomirView::OnToolboxandcontextmenuGigantic)
	ON_COMMAND(ID_MOUSE_REVERSEMOUSEWHEELSCROLLINGDIRECTION, &CMathomirView::OnMouseReversemousewheelscrollingdirection)
	ON_COMMAND(ID_FONTSIZEANDDEFAULTZOOM_DEFAULTZOOMIS150, &CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis150)
	ON_COMMAND(ID_FONTSIZEANDDEFAULTZOOM_DEFAULTZOOMIS120, &CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis120)
	ON_COMMAND(ID_FONTSIZEANDDEFAULTZOOM_DEFAULTZOOMIS100, &CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis100)
	ON_COMMAND(ID_FONTSIZEANDDEFAULTZOOM_DEFAULTZOOMIS80, &CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis80)
	ON_COMMAND(ID_MOUSE_MOUSE, &CMathomirView::OnMouseMouse)
	ON_COMMAND(ID_MOUSE_RIGHTMOUSEBUTTONTOTOGGLEMOUSE, &CMathomirView::OnMouseRightmousebuttontotogglemouse)
	ON_COMMAND(ID_MOUSE_SLOW, &CMathomirView::OnMouseSlow)
	ON_COMMAND(ID_PAGENUMERATION_NONE, &CMathomirView::OnPagenumerationNone)
	ON_COMMAND(ID_PAGENUMERATION_, &CMathomirView::OnPagenumeration)
	ON_COMMAND(ID_PAGENUMERATION_32899, &CMathomirView::OnPagenumeration32899)
	ON_COMMAND(ID_PAGENUMERATION_PAGE1OF10, &CMathomirView::OnPagenumerationPage1of10)
	ON_COMMAND(ID_PAGENUMERATION_BOTTOM, &CMathomirView::OnPagenumerationBottom)
	ON_COMMAND(ID_PAGENUMERATION_RIGHT, &CMathomirView::OnPagenumerationRight)
	ON_COMMAND(ID_PAGENUMERATION_EXCLUDEFIRSTPAGE, &CMathomirView::OnPagenumerationExcludefirstpage)
	ON_COMMAND(ID_TOOLBOXANDCONTEXTMENU_AUTO, &CMathomirView::OnToolboxandcontextmenuAuto)
	ON_COMMAND(ID_Menu32905, &CMathomirView::OnMenu32905)
	ON_COMMAND(ID_KEYBOARD_USECOMPLEXINDEXES, &CMathomirView::OnKeyboardUsecomplexindexes)
	END_MESSAGE_MAP()

// CMathomirView construction/destruction

CMathomirView::CMathomirView()
{
	// TODO: add construction code here
	m_PopupMenuObject=NULL;
	pMainView=this;
	pMainViewBase=(CView*)this;
}


int RepaintViewTimer=0;

struct
{
	CBitmap *bmp;
	CDC *DC;
	int X;
	int Y;
	int Cx;
	int Cy;
	int Restored;
	int ZoomLevel;
} ClipboardBackground;


void DetermineTillensData(int mouseY)
{
	if (mouseY==0x7FFFFFFF) {Tillens.Show=0;return;}
	//int new_file_loaded=0;
	if (mouseY<0) {mouseY=TheClientRect.bottom;}
	Tillens.Num=max(1,(ViewMaxY+PaperHeight/32)/PaperHeight);
	if (Tillens.Num>99) Tillens.Num=99;
	Tillens.NumX=1;

	if ((TheClientRect.bottom<600) || (TheClientRect.right<800) || (Tillens.Num>45))
		Tillens.Zoom=100*160/max(PaperHeight,PaperWidth);
	else if ((TheClientRect.bottom<700) || (TheClientRect.right<1000) || (Tillens.Num>32))
		Tillens.Zoom=100*180/max(PaperHeight,PaperWidth);
	else if ((TheClientRect.bottom<750) || (TheClientRect.right<1200) || (Tillens.Num>28))
		Tillens.Zoom=100*200/max(PaperHeight,PaperWidth);
	else
		Tillens.Zoom=100*220/max(PaperHeight,PaperWidth);
	Tillens.Height=Tillens.Zoom*PaperHeight/100;

	//calculates the number of tillens in Y direction
	int NumY=(TheClientRect.bottom+16)/(Tillens.Height+5);
	if (NumY>Tillens.Num) NumY=Tillens.Num;


	if (NumY<1) NumY=1;


	Tillens.NumY=NumY;
	Tillens.NumX=(Tillens.Num+NumY-1)/NumY;

	Tillens.Width=Tillens.Zoom*PaperWidth/100;

	int windowheight=Tillens.NumY*(Tillens.Height+5);

	Tillens.StartX=TheClientRect.right-(Tillens.Width+20)*Tillens.NumX;
	Tillens.StartY=mouseY-windowheight/2;
	if (Tillens.StartY<5) Tillens.StartY=5;
	if (Tillens.StartY+windowheight>TheClientRect.bottom) 
	{
		Tillens.StartY=TheClientRect.bottom-windowheight;
		if (Tillens.StartY<5) Tillens.StartY+=3;
	}

	Tillens.EndY=Tillens.StartY+windowheight;
	Tillens.Enlarge=-1;

}

CMathomirView::~CMathomirView()
{
	if (ClipboardBackground.DC)
	{
		ClipboardBackground.DC->DeleteDC();
		delete ClipboardBackground.DC;
	}
	if (ClipboardBackground.bmp)
	{
		ClipboardBackground.bmp->DeleteObject();
		delete ClipboardBackground.bmp;
	}

	if (TheKeyboardClipboard)
	{
		delete TheKeyboardClipboard;
	}

}

int CMathomirView::InitSpecific(void)
{
	UndoInit();
	return 0;
}

int CMathomirView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(1,10,NULL); //this is for cursor and copy-paste
	return 0;
}

BOOL CMathomirView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style=WS_BORDER|WS_VISIBLE|WS_CHILD|WS_VSCROLL|WS_HSCROLL;
	//cs.dwExStyle=WS_EX_WINDOWEDGE;
	return CView::PreCreateWindow(cs);
}

void GeneratePageNumerationString(char *pagestr,int number,int maxpage)
{
	int numtype=PageNumeration&0x0F;
	if (numtype==1) sprintf(pagestr,"- %d -",number);
	else if (numtype==2) sprintf(pagestr,"- %d/%d -",number,maxpage);
	else 
	{
		char num_template[64];
		CopyTranslatedString(num_template, "Page 1 of 10", 32900, 62);
		int l=(int)strlen(num_template);
		int i;
		int n_pos=0;
		int N_pos=0;
		for (i=0;i<l;i++)
		{
			if ((num_template[i]=='1') && (num_template[i+1]!='0'))
			{
				//raplaces '1' with '%d'
				l++;
				memmove(num_template+i+1,num_template+i,63-i);
				num_template[i]='%';
				num_template[i+1]='d';
				n_pos=i;
			}
			if ((num_template[i]=='1') && (num_template[i+1]=='0'))
			{
				//replaces '10' with %d
				num_template[i]='%';
				num_template[i+1]='d';
				N_pos=i;
			}
		}

		if (N_pos>n_pos)
			sprintf(pagestr,num_template,number,maxpage);
		else
			sprintf(pagestr,num_template,maxpage,number);
	}


}

bool CheckForGuidlines(int i,int AbsoluteX, int AbsoluteY, int show_range);
void PaintGuidlines(CDC *DC,  int AbsoluteY);
void PaintNewlineAddMark(CDC *DC);

// CMathomirView drawing
int MakeQuickDrawing;
int QuickDrawingDisable;
int QuickDrawingWidth;
int PagePrinted,MaxPagePrinted;
tDocumentStruct *SpecialDrawingHover; 
tDocumentStruct *prevSpecialDrawingHover;
int MultipleX=-1;
int MultipleY=-1;
int MultipleStartX=-1;
int MultipleStartY=-1;
CDrawing *tmpDrawing=NULL;
CDrawing *tmpDrawing2=NULL;
DWORD moving_start_timer; //timetag when we started moving an drawing
int prevDrawingMode=-1;
int MovingStartX=-1;
int MovingStartY=-1;
int MovingStartX2=-1;
int MovingStartY2=-1;
RECT SelectionRect;
RECT SelectionRectOrig;
int SelectedHandle=0;
int NumSelectedObjects;  //used for drawing box and displaying selection 
int NumSelectedDrawings;
int prevNumSelectedObjects=-1;
int GuidlineElement=-1;
int prevGuidlineElement=-1;
int prevGuidlineElement2=-1;
int prevGuidlineX=0x7FFFFFFF,prevGuidlineY;
int GuidlineClearTimer;
CBitmap bitmap;
CDC bitmapDC;
int MainViewCX,MainViewCY;
int RullerGuidelines[12]; //index of object in TheDocument struct
int NumRullerGuidelines=0;


void CMathomirView::OnDraw(CDC* pDC)
{	
	tDocumentStruct *ds=TheDocument;
	int SelectedElement;
	NumRullerGuidelines=0;
	if ((MouseMode!=9) && (MouseMode!=11)) SingleSelectedTextObject=-1;


	if (pDC->IsPrinting())
	{
		
		RECT ClientArea=TheClientRect;
		int prevViewX,prevViewY,prevViewZoom;

		prevViewX=ViewX;
		prevViewY=ViewY;
		prevViewZoom=ViewZoom;

		ClientArea.right=pDC->GetDeviceCaps(PHYSICALWIDTH);
		ClientArea.bottom=pDC->GetDeviceCaps(PHYSICALHEIGHT);
		ViewZoom=(short)(100*ClientArea.bottom/PaperHeight);
		ViewY=(PagePrinted-1)*PaperHeight+5+pDC->GetDeviceCaps(PHYSICALOFFSETY)*100/ViewZoom;
		ViewX=15+pDC->GetDeviceCaps(PHYSICALOFFSETX)*100/ViewZoom;
		int ViewYBottom=ViewY+ClientArea.bottom*100/ViewZoom;

		//Page numbering
		if (PageNumeration&0x0F)
		{
			int sz=20*ViewZoom/100;
			HDC hh=pDC->m_hAttribDC;
			pDC->m_hAttribDC=pDC->m_hDC;
			pDC->SelectObject(GetFontFromPool(4,0,0,sz));
			pDC->m_hAttribDC=hh;
			pDC->SetBkColor(RGB(255,255,255));
			pDC->SetBkMode(OPAQUE);
			pDC->SetTextColor(0);
			pDC->SetTextAlign(TA_CENTER);
			char pagestr[64];

			int number=PagePrinted;
			int maxpage=MaxPagePrinted;
			if (PageNumeration&0x40) {number--; maxpage--;}//exclude first page from numbering

			GeneratePageNumerationString(pagestr,number,maxpage);

			int x_pos=ClientArea.right/2;
			int y_pos=sz+40*ViewZoom/100;
			if (PageNumeration&0x20) 
			{
				//right aligned page number
				pDC->SetTextAlign(TA_RIGHT);
				x_pos=ClientArea.right-65*ViewZoom/100;
			}
			if (PageNumeration&0x10) 
			{
				//bottom postion for page number
				y_pos=ClientArea.bottom-75*ViewZoom/100;
			}
			if (number)
				pDC->TextOut(x_pos,y_pos,pagestr,(int)strlen(pagestr));
		}



		for (int i=0;i<NumDocumentElements;i++,ds++)
		{
			if ((ds->absolute_Y>=ViewY-5) && (ds->absolute_Y<ViewYBottom-5))
			{
				int RelativeX=(ds->absolute_X-ViewX)*ViewZoom/100;
				int RelativeY=(ds->absolute_Y-ViewY)*ViewZoom/100;

				CBitmap tmpbmp;
				CDC tmpDC;
				tmpDC.CreateCompatibleDC(pDC);

				short l,a,b;
				if (ds->Type==1)
				{
					((CExpression*)(ds->Object))->CalculateSize(&tmpDC,ViewZoom,&l,&a,&b);
					((CExpression*)(ds->Object))->DeselectExpression();
				} 
				else if (ds->Type==2)
				{
					a=0;
					((CDrawing*)(ds->Object))->CalculateSize(&tmpDC,ViewZoom,&l,&b);
					((CDrawing*)(ds->Object))->SelectDrawing(0);
				}
				tmpbmp.CreateCompatibleBitmap(pDC,l,a+b);
				tmpDC.SelectObject(tmpbmp);
				tmpDC.FillSolidRect(0,0,ClientArea.right,ClientArea.bottom,RGB(255,255,255));
				PrintRendering=1;
				if (PrintTextAsImage)
				{
					if (ds->Type==1)
						((CExpression*)(ds->Object))->PaintExpression(&tmpDC,ViewZoom,0,a);
					else if (ds->Type==2)
						((CDrawing*)(ds->Object))->PaintDrawing(&tmpDC,ViewZoom,0,a,-1,-1);
					pDC->TransparentBlt(RelativeX,RelativeY-a,l,a+b,&tmpDC,0,0,l,a+b,RGB(255,255,255)/*SRCCOPY*/);
				}
				else
				{
					HDC hh=pDC->m_hAttribDC;
					pDC->m_hAttribDC=pDC->m_hDC;
					if (ds->Type==1)
						((CExpression*)(ds->Object))->PaintExpression(pDC,ViewZoom,RelativeX,RelativeY);
					else if (ds->Type==2)
						((CDrawing*)(ds->Object))->PaintDrawing(pDC,ViewZoom,RelativeX,RelativeY,-1,-1);
					pDC->m_hAttribDC=hh;
				}
				PrintRendering=0;
				tmpbmp.DeleteObject();
				if (ds->Type==1)
					((CExpression*)(ds->Object))->CalculateSize(&tmpDC,prevViewZoom,&l,&a,&b);
				else if (ds->Type==2)
					((CDrawing*)(ds->Object))->CalculateSize(&tmpDC,prevViewZoom,&l,&b);
			}
		}

		ViewX=prevViewX;
		ViewY=prevViewY;
		ViewZoom=prevViewZoom;

	}
	else
	{
		if (!ClipboardBackground.Restored) RestoreClipboardBackground();

		int ViewXRight=ViewX+TheClientRect.right*100/ViewZoom;
		int ViewYBottom=ViewY+TheClientRect.bottom*100/ViewZoom;

		//create the main view bitmap into wich is drawn
		if ((TheClientRect.right>MainViewCX) || (TheClientRect.bottom>MainViewCY))
		{
			if (MainViewCX<TheClientRect.right) MainViewCX=TheClientRect.right;
			if (MainViewCY<TheClientRect.bottom) MainViewCY=TheClientRect.bottom;
			bitmap.DeleteObject();
			bitmapDC.DeleteDC();
			bitmap.CreateCompatibleBitmap(pDC,MainViewCX,MainViewCY);
			bitmapDC.CreateCompatibleDC(pDC);
			bitmapDC.SelectObject(bitmap);
		}
		if (!bitmapDC) return;

		//draw paper sheet area
		if (!ViewOnlyMode)
		{
			int RelativeLeft=(15-ViewX)*ViewZoom/100;
			int RelativeRight=(15+PaperWidth-ViewX)*ViewZoom/100;
			int RelativeHeight=(PaperHeight)*ViewZoom/100;
			int RelativeStart=-(ViewY%PaperHeight)*ViewZoom/100;

			bitmapDC.FillSolidRect(RelativeLeft,0,RelativeRight-RelativeLeft,TheClientRect.bottom,RGB(255,255,255));
			if (RelativeLeft>0) bitmapDC.FillSolidRect(0,0,RelativeLeft,TheClientRect.bottom,DOCUMENT_AREA_BACKGROUND);
			if (RelativeRight<TheClientRect.right) bitmapDC.FillSolidRect(RelativeRight,0,TheClientRect.right-RelativeRight,TheClientRect.bottom,DOCUMENT_AREA_BACKGROUND);
			int curpage=ViewY/PaperHeight+1;
			while (RelativeStart<TheClientRect.bottom)
			{
				bitmapDC.FillSolidRect(0,RelativeStart-5*ViewZoom/100,TheClientRect.right,10*ViewZoom/100,DOCUMENT_AREA_BACKGROUND);


				//Page numbering
				if (PageNumeration&0x0F)
				{
					int sz=20*ViewZoom/100;
					bitmapDC.SelectObject(GetFontFromPool(4,0,0,sz));
					bitmapDC.SetBkColor(RGB(255,255,255));
					bitmapDC.SetBkMode(OPAQUE);
					bitmapDC.SetTextColor(RGB(128,128,128));
					bitmapDC.SetTextAlign(TA_CENTER);
					char pagestr[64];

					int number=curpage++;
					int maxpage=max(1,(ViewMaxY+PaperHeight/32)/PaperHeight);
					if (PageNumeration&0x40) {number--;maxpage--;} //exclude first page from numbering
					
					GeneratePageNumerationString(pagestr,number,maxpage);
				
					int x_pos=(RelativeLeft+RelativeRight)/2;
					int y_pos=RelativeStart+sz+40*ViewZoom/100;
					if (PageNumeration&0x20) 
					{
						//right aligned page number
						bitmapDC.SetTextAlign(TA_RIGHT);
						x_pos=RelativeRight-65*ViewZoom/100;
					}
					if (PageNumeration&0x10) 
					{
						//bottom postion for page number
						y_pos=RelativeStart+RelativeHeight-75*ViewZoom/100;
					}
					if (number)
						bitmapDC.TextOut(x_pos,y_pos,pagestr,(int)strlen(pagestr));
				}
				RelativeStart+=RelativeHeight;
			}

			//painting grid
			if (IsShowGrid)
			{
				int sx=TheClientRect.right*100/ViewZoom;
				int sy=TheClientRect.bottom*100/ViewZoom;
				int step=(ViewZoom<50)?2*GRID:GRID;
				short *yarray=(short*)new short[sy/step*2+8];
				int ylen=0;
				for (int jj=(ViewY/GRID)*GRID-ViewY;jj<sy;jj+=step)
				{
					*(yarray+ylen)=jj*ViewZoom/100;
					ylen++;
				}
				for (int ii=(ViewX/GRID)*GRID-ViewX;ii<sx;ii+=step)
				{
					int ax=ii*ViewZoom/100;
					short *pnt=yarray;
					for (int jj=ylen;jj>0;jj--,pnt++)
					{
						bitmapDC.SetPixelV(ax,*pnt,RGB(0,192,0));
						if (ViewZoom&0xFF00) bitmapDC.SetPixelV(ax,*pnt+1,RGB(0,192,0));
					}				
				}
				delete yarray;
			}
		}
		else
			bitmapDC.FillSolidRect(0,0,TheClientRect.right,TheClientRect.bottom,RGB(255,255,255));



		NumSelectedObjects=0;
		NumSelectedDrawings=0;
		if ((MouseMode!=9) && (MouseMode!=11))
		{
			SelectionRect.top=SelectionRect.left=0x7FFFFFFF;
			SelectionRect.bottom=SelectionRect.right=-0x7FFFFFFF;
		}


		for (int i=NumDocumentElements-1;i>=0;i--,ds++)
		{
			if ((ds->absolute_Y<-1000) && (ds->absolute_Y>-1200))
			{
				if (NumRullerGuidelines<13) RullerGuidelines[NumRullerGuidelines++]=NumDocumentElements-i-1;
				continue;
			}

			if (ds->MovingDotState==3)
			{
				NumSelectedObjects++;
				if (ds->Type==2) NumSelectedDrawings++;
				SelectedElement=NumDocumentElements-i-1;
				if ((MouseMode!=9) && (MouseMode!=11))
				{
					if (ds->absolute_X<SelectionRect.left) SelectionRect.left=ds->absolute_X;
					if (ds->absolute_X+ds->Length>SelectionRect.right) SelectionRect.right=ds->absolute_X+ds->Length;
					if (ds->absolute_Y-ds->Above<SelectionRect.top) SelectionRect.top=ds->absolute_Y-ds->Above;
					if (ds->absolute_Y+ds->Below>SelectionRect.bottom) SelectionRect.bottom=ds->absolute_Y+ds->Below;
				}
			}

			//only painting elements that are visible
			if ((ds->absolute_Y+ds->Below>ViewY) && (ds->absolute_Y-ds->Above<ViewYBottom) &&
				 (ds->absolute_X+ds->Length>ViewX) && (ds->absolute_X<ViewXRight))
			{				
				int RelativeX=(ds->absolute_X-ViewX)*ViewZoom/100;
				int RelativeY=(ds->absolute_Y-ViewY)*ViewZoom/100;
				if ((ds->MovingDotState==3) && (((MouseMode!=9) && (MouseMode!=11)) || (SelectedHandle!=5)))
				{
					GentlyPaintObject(ds,&bitmapDC);
				}
				else if (ds->MovingDotState!=6) //prepared for moving
				{
					if (ds->Type==1)
						((CExpression*)(ds->Object))->PaintExpression(&bitmapDC,ViewZoom,RelativeX,RelativeY,&TheClientRect);
					else if ((ds->Type==2) && (ds!=SpecialDrawingHover))
						((CDrawing*)(ds->Object))->PaintDrawing(&bitmapDC,ViewZoom,RelativeX,RelativeY,ds->absolute_X,ds->absolute_Y,&TheClientRect);
				}

				if ((ds->Type==1) && ((ds->MovingDotState==1) || (ds->MovingDotState==2) || (MovingDotPermanent)))
				{
					int mds=max(MovingDotSize,MovingDotSize*(ViewZoom+420)/512);
					int border=1+ViewZoom/100;

					if (ds->MovingDotState==2)
					{
						bitmapDC.FillSolidRect(RelativeX,RelativeY+(ds->Below)*ViewZoom/100-mds+border,mds,mds,RGB(0,0,0));
					}
					else 
					{
						int xx=RelativeX;
						int yy=RelativeY+(ds->Below)*ViewZoom/100-mds+border;
						bitmapDC.FillSolidRect(xx+1,yy+1,mds-2,mds-2,RGB(176,176,176));
						bitmapDC.FillSolidRect(xx+2,yy+2,mds-4,mds-4,RGB(240,240,240));
					}
				}
			}
		}

		if ((prevNumSelectedObjects!=NumSelectedObjects) && ((prevNumSelectedObjects<3) || (NumSelectedObjects<3)))
		{
			if ((UseToolbar) && (Toolbox) &&(Toolbox->Toolbar)) {Toolbox->Toolbar->ConfigureToolbar();}
		}
		prevNumSelectedObjects=NumSelectedObjects;

		if ((MouseMode==6) && (tmpDrawing))
		{
			tmpDrawing->PaintDrawing(&bitmapDC,ViewZoom,(MovingStartX/10-ViewX)*ViewZoom/100,(MovingStartY/10-ViewY)*ViewZoom/100,-1,-1);
		}
		if (SpecialDrawingHover)
		{
			int RelativeX=(SpecialDrawingHover->absolute_X-ViewX)*ViewZoom/100;
			int RelativeY=(SpecialDrawingHover->absolute_Y-ViewY)*ViewZoom/100;
			((CDrawing*)(SpecialDrawingHover->Object))->PaintDrawing(&bitmapDC,ViewZoom,RelativeX,RelativeY,SpecialDrawingHover->absolute_X,SpecialDrawingHover->absolute_Y);
		}

		//painting multiple selection box
		if (MultipleStartX!=-1)
		{
			int X1,Y1,X2,Y2;
			X1=(MultipleStartX-ViewX)*ViewZoom/100;
			X2=(MultipleX-ViewX)*ViewZoom/100;
			Y1=(MultipleStartY-ViewY)*ViewZoom/100;
			Y2=(MultipleY-ViewY)*ViewZoom/100;

			bitmapDC.SelectObject(GetStockObject(BLACK_PEN));
			bitmapDC.MoveTo(X1,Y1);
			bitmapDC.LineTo(X1,Y2);bitmapDC.LineTo(X2,Y2);bitmapDC.LineTo(X2,Y1);bitmapDC.LineTo(X1,Y1);
			MakeQuickDrawing=0;
			if (NumSelectedObjects) QuickDrawingDisable=5;
			if ((!NumSelectedObjects) && (IsDrawingMode==0) && (QuickDrawingDisable==0) && (!ViewOnlyMode))
			{
				int do_draw=MouseSteady;
				//quick painting of curly brackets and section lines
				int len=X2-X1;
				int width=Y2-Y1;
				char vertical=0;
				if (abs(X1-X2)<abs(Y1-Y2)) {len=Y2-Y1;width=X2-X1;vertical=1;}
				if (abs(len)>12)
				{
					int w=((abs(len*32/ViewZoom)+230))/64+4;
					QuickDrawingWidth=(width&0xFFFF)|(w<<16);
					bitmapDC.SelectObject(GetPenFromPool(max(w*ViewZoom/400,2),0,(do_draw)?RGB(128,128,128):RGB(224,224,224)));

					if (abs(width)<abs(len/48)+5)
					{
						if (vertical) 
						{
							bitmapDC.MoveTo(X1,Y1);
							bitmapDC.LineTo(X1,Y2);
							MakeQuickDrawing=3;
						}
						else
						{
							bitmapDC.MoveTo(X1,Y1);
							bitmapDC.LineTo(X2,Y1);
							MakeQuickDrawing=1;
						}
					}
					else if (abs(len*100/ViewZoom/12)+10>abs(width*100/ViewZoom))
					{
						int c=10; 
						if (abs(len*100/ViewZoom)>150) c=14;
						if (abs(len*100/ViewZoom)>300) c=18;
						if (abs(len*100/ViewZoom)>600) c=24;
						int cc=c/2;

						if (width>abs(len)/16+5*ViewZoom/100) width=abs(len)/16+5*ViewZoom/100;
						if (width<-abs(len)/16-5*ViewZoom/100) width=-abs(len)/16-5*ViewZoom/100;
						bitmapDC.MoveTo(X1,Y1);
						if (vertical)
						{
							bitmapDC.LineTo(X1+width/2,Y1+len/c);
							bitmapDC.LineTo(X1+width/2,Y1+(cc-1)*len/c);
							bitmapDC.LineTo(X1+width,Y1+cc*len/c);
							bitmapDC.LineTo(X1+width/2,Y1+(cc+1)*len/c);
							bitmapDC.LineTo(X1+width/2,Y1+(c-1)*len/c);
							bitmapDC.LineTo(X1,Y1+c*len/c);
							MakeQuickDrawing=4;
						}
						else
						{
							bitmapDC.LineTo(X1+len/c,Y1+width/2);
							bitmapDC.LineTo(X1+(cc-1)*len/c,Y1+width/2);
							bitmapDC.LineTo(X1+cc*len/c,Y1+width);
							bitmapDC.LineTo(X1+(cc+1)*len/c,Y1+width/2);
							bitmapDC.LineTo(X1+(c-1)*len/c,Y1+width/2);
							bitmapDC.LineTo(X1+c*len/c,Y1);
							MakeQuickDrawing=2;
						}
					}
					if (do_draw==0) MakeQuickDrawing=0;
				}
			}
		}
		else if (((NumSelectedObjects) || (((MouseMode==9) || (MouseMode==11)) && (SelectedHandle==5))) && (MouseMode!=2) && (MouseMode!=102))
		{
			//paint streching handles around selection

			/* //some blue hue
			int x1=(SelectionRect.left-ViewX)*ViewZoom/100;
			int y1=(SelectionRect.top-ViewY)*ViewZoom/100;
			int x2=(SelectionRect.right-ViewX)*ViewZoom/100;
			int y2=(SelectionRect.bottom-ViewY)*ViewZoom/100;
			HBRUSH br=CreateSolidBrush(RGB(240,240,255));

			bitmapDC.SelectObject(br);
			bitmapDC.SetROP2(R2_MASKPEN);
			bitmapDC.Rectangle(x1,y1,x2,y2);
			DeleteObject(br);
			bitmapDC.SetROP2(R2_COPYPEN);*/

			if (NumSelectedObjects==1) 
			{
				if ((TheDocument[SelectedElement].Type==1) && (((CExpression*)TheDocument[SelectedElement].Object)->IsTextContained(-1)))
				{
					if (((CExpression*)TheDocument[SelectedElement].Object)->m_IsVertical==0)
						SingleSelectedTextObject=SelectedElement;
				}
			}
			int gg=MovingDotSize*3/2;

			if (((MouseMode!=9) && (MouseMode!=11)) || (SelectionRect.left!=-1234))
			{
				bitmapDC.FillSolidRect((SelectionRect.left-ViewX)*ViewZoom/100-gg,(SelectionRect.top-ViewY)*ViewZoom/100-gg,gg,gg,RGB((SelectedHandle==1)?255:160,0,0));
				bitmapDC.FillSolidRect((SelectionRect.right-ViewX)*ViewZoom/100,(SelectionRect.top-ViewY)*ViewZoom/100-gg,gg,gg,RGB((SelectedHandle==2)?255:160,0,0));
				bitmapDC.FillSolidRect((SelectionRect.right-ViewX)*ViewZoom/100,(SelectionRect.bottom-ViewY)*ViewZoom/100,gg,gg,RGB((SelectedHandle==3)?255:160,0,0));
				bitmapDC.FillSolidRect((SelectionRect.left-ViewX)*ViewZoom/100-gg,(SelectionRect.bottom-ViewY)*ViewZoom/100,gg,gg,RGB((SelectedHandle==4)?255:160,0,0));
				if (SingleSelectedTextObject!=-1)
					bitmapDC.FillSolidRect((SelectionRect.right-ViewX)*ViewZoom/100+gg/2,(SelectionRect.top-ViewY)*ViewZoom/100+gg/2,gg/2, (SelectionRect.bottom-SelectionRect.top)*ViewZoom/100-gg,RGB(0,(SelectedHandle==5)?255:160,0));
			}
			else
			{
				//paint rotation handles
				CPen mpen(PS_SOLID,1,RGB(255,0,0));
				int cx=((SelectionRectOrig.right+SelectionRectOrig.left)/2-ViewX)*ViewZoom/100;
				int cy=((SelectionRectOrig.bottom+SelectionRectOrig.top)/2-ViewY)*ViewZoom/100;
				HPEN p=(HPEN)bitmapDC.SelectObject(mpen);
				bitmapDC.Ellipse(cx-2,cy-2,cx+2,cy+2);
				bitmapDC.SelectObject(p);
			}
		}

		/*{
			//paint the ruller
			//bitmapDC.FillSolidRect(10,0,1,TheClientRect.bottom,RGB(228,228,228));
			int d=(ViewZoom>=100)?3:2;if (ViewZoom>250) d=4;

			int sx=((ViewY+GRID/2)/GRID)*GRID;
			int lng=TheClientRect.bottom;
			
			for (int i=sx;i<sx+lng*100/ViewZoom;i+=GRID)
			{
				int tmp=i/GRID;
				
				if ((tmp%5)==0) 
					bitmapDC.FillSolidRect(2,(i-ViewY)*ViewZoom/100-1,3,3,RGB(228,228,228));
			
					bitmapDC.FillSolidRect(5,(i-ViewY)*ViewZoom/100,4,1,RGB(228,228,228));
				
			}
		}*/

		//paint the ruller guidelines
		for (int i=0;i<NumRullerGuidelines;i++)
		{
			int idx=RullerGuidelines[i];
			if ((idx>=0) && (idx<NumDocumentElements))
			{
				int SelectedGuidelineType=0;
				tDocumentStruct *dsx=TheDocument+idx;
				if (dsx->absolute_Y>-1100) SelectedGuidelineType=1;
				bitmapDC.SetROP2(R2_MASKPEN);
				CPen p(PS_SOLID,1,(SelectedGuidelineType)?RGB(208,248,208):RGB(240,240,192));
				bitmapDC.SelectObject(p);
				int X=(dsx->absolute_X-ViewX+3)*ViewZoom/100;
				bitmapDC.MoveTo(X,0);
				bitmapDC.LineTo(X,TheClientRect.bottom);
				bitmapDC.SetROP2(R2_COPYPEN);
			}
		}
		if ((GuidlineElement>=0) && (GuidlineElement<NumDocumentElements))
		{		
			PaintGuidlines(&bitmapDC,-1);
		}
		if (NewlineAddObject>=0)
		{
			prevNewlineAddObject=-1;
			PaintNewlineAddMark(&bitmapDC);
		}
		if (ShowRullerCounter==0)
		{
			//paint the ruller
			int d=(ViewZoom>=100)?3:2;if (ViewZoom>250) d=4;

			if (RullerType==0)
			{
				bitmapDC.FillSolidRect(0,0,TheClientRect.right,12,RGB(255,255,192));
				if (RullerPositionPreselected>=0)
					bitmapDC.FillSolidRect((RullerPositionPreselected-ViewX+3)*ViewZoom/100-d,0,2*d+1,12,RGB(0,0,255));
			}
			else
				bitmapDC.FillSolidRect(0,0,12,TheClientRect.bottom,RGB(255,255,192));

			
			int sx=((ViewX+GRID/2)/GRID)*GRID;
			int lng=TheClientRect.right;
			if (RullerType==1)
			{
				sx=((ViewY+GRID/2)/GRID)*GRID;
				lng=TheClientRect.bottom;
			}
			for (int i=sx;i<sx+lng*100/ViewZoom;i+=GRID)
			{
				int tmp=i/GRID;
				if (RullerType==0)
				{
					int found=0;
					if (i!=RullerPositionPreselected+3)
					for (int k=0;k<NumRullerGuidelines;k++)
					{
						int idx=RullerGuidelines[k];
						if ((idx>=0) && (idx<NumDocumentElements))
						{
							tDocumentStruct *dsx=TheDocument+idx;
							if ((dsx->absolute_Y<-1000) && (dsx->absolute_X+3>=i) && (dsx->absolute_X+3<i+GRID))
								found=1;
						}
					}
					if (found)
						bitmapDC.FillSolidRect((i-ViewX)*ViewZoom/100-d,0,2*d+1,12,RGB(255,128,0));

					if ((tmp%5)==0) bitmapDC.FillSolidRect((i-ViewX)*ViewZoom/100-1,2,3,3,0);
					bitmapDC.FillSolidRect((i-ViewX)*ViewZoom/100,5,1,7,0);
				}
				else
				{
					if ((tmp%5)==0) bitmapDC.FillSolidRect(2,(i-ViewY)*ViewZoom/100-1,3,3,0);
					bitmapDC.FillSolidRect(5,(i-ViewY)*ViewZoom/100,7,1,0);
				}
			}
		}




		//painting small tillens
		if (Tillens.Show)
		{
			RECT viewport;
			viewport.left=ViewX;
			viewport.right=ViewX+TheClientRect.right*100/ViewZoom;
			viewport.top=ViewY;
			viewport.bottom=ViewY+TheClientRect.bottom*100/ViewZoom;


			int Lx=5+Tillens.NumX*(Tillens.Width+20);
			int Ly=5+Tillens.NumY*(Tillens.Height+5);
			bitmapDC.FillSolidRect(
				Tillens.StartX-5,
				Tillens.StartY-5,
				Lx,Ly,SHADOW_BLUE_COLOR2);
			bitmapDC.FillSolidRect(Tillens.StartX-5,Tillens.StartY-5,Lx,1,BLUE_COLOR);
			bitmapDC.FillSolidRect(Tillens.StartX-5,Tillens.StartY-5,1,Ly,BLUE_COLOR);
			bitmapDC.FillSolidRect(Tillens.StartX-5,Tillens.EndY,Lx,1,BLUE_COLOR);
			int tillen=0;
			for (int i=0;i<Tillens.NumX;i++)
				for (int j=0;j<Tillens.NumY;j++)
				{
					if (tillen>=Tillens.Num) break;
					int Tsx=Tillens.StartX+i*(Tillens.Width+20);
					int Tsy=Tillens.StartY+j*(Tillens.Height+5);
					bitmapDC.FillSolidRect(
						Tsx,
						Tsy,
						Tillens.Width,
						Tillens.Height,
						RGB(255,255,255));
					bitmapDC.FillSolidRect(Tsx-1,Tsy-1,Tillens.Width+2,1,BLUE_COLOR);
					bitmapDC.FillSolidRect(Tsx-1,Tsy-1,1,Tillens.Height+2,BLUE_COLOR);
					bitmapDC.FillSolidRect(Tsx-1,Tsy+Tillens.Height,Tillens.Width+2,1,BLUE_COLOR);
					bitmapDC.FillSolidRect(Tsx+Tillens.Width,Tsy-1,1,Tillens.Height+2,BLUE_COLOR);

					bitmapDC.FillSolidRect(Tsx+Tillens.Width+2,Tsy+50,9,9,SHADOW_BLUE_COLOR);
					bitmapDC.FillSolidRect(Tsx+Tillens.Width+6,Tsy+52,1,5,0);
					bitmapDC.FillSolidRect(Tsx+Tillens.Width+4,Tsy+54,5,1,0);

					int paperstart=tillen*PaperHeight;
					int paperend=paperstart+PaperHeight;
					int ytop=-1;
					int ybottom=-1;
					if ((paperstart<=viewport.top) && (paperend>=viewport.top)) ytop=(viewport.top-paperstart)*Tillens.Height/PaperHeight;
					if ((viewport.top<paperstart) && (viewport.bottom>=paperstart)) ytop=0;

					if ((paperend>=viewport.bottom) && (paperstart<=viewport.bottom)) ybottom=(viewport.bottom-paperstart)*Tillens.Height/PaperHeight;
					if ((viewport.bottom>paperend) && (viewport.top<paperend)) ybottom=Tillens.Height;

					if (ytop>=0)
					{
						int xleft=0;
						int xright=0;
						xleft=viewport.left*Tillens.Height/PaperHeight;
						xright=viewport.right*Tillens.Height/PaperHeight;
						if (xright>Tillens.Width) xright=Tillens.Width;
						bitmapDC.FillSolidRect(Tsx+xleft,Tsy+ytop,xright-xleft,ybottom-ytop,RGB(255,220,220));
						bitmapDC.FillSolidRect(Tsx+xleft+2,Tsy+ytop+2,max(0,xright-xleft-4),max(0,ybottom-ytop-4),RGB(255,240,240));
					}


					bitmapDC.SelectObject(GetFontFromPool(4,0,0,14));
					tillen++;
					char buff[16];
					itoa(tillen,buff,10);
					bitmapDC.SetTextColor(RGB(0,0,255));
					bitmapDC.SetBkMode(TRANSPARENT);
					bitmapDC.SetTextAlign(TA_BASELINE);
					bitmapDC.TextOut(Tsx+Tillens.Width+2,Tsy+Tillens.Height,buff,(int)strlen(buff));
				}

			ds=TheDocument;

			CRgn *myrgn=NULL;
			int lasttillen=-1;
			for (int kk=0;kk<2;kk++,ds=TheDocument)
			for (int i=NumDocumentElements-1;i>=0;i--,ds++)
			{
				int Tillen=(ds->absolute_Y/PaperHeight);
				int vX,vY;
				vX=0;
				vY=Tillen*PaperHeight;
				int TillenRelativeX=Tillens.StartX+(Tillen/Tillens.NumY)*(Tillens.Width+20);
				int TillenRelativeY=Tillens.StartY+(Tillen%Tillens.NumY)*(Tillens.Height+5);
				if (Tillen!=lasttillen)
				{
					lasttillen=tillen;
					bitmapDC.SelectClipRgn(NULL);

					if (myrgn) delete myrgn;
					myrgn=new CRgn();
					myrgn->CreateRectRgn(TillenRelativeX,TillenRelativeY,TillenRelativeX+Tillens.Width,TillenRelativeY+Tillens.Height);
					bitmapDC.SelectClipRgn(myrgn);
				}

				int RelativeX=TillenRelativeX+(ds->absolute_X-vX)*Tillens.Zoom/100;
				int RelativeY=TillenRelativeY+(ds->absolute_Y-vY)*Tillens.Zoom/100;

				short l,a,b;
				int is_title=0;
				if (ds->Type==1)
				if ((((CExpression*)ds->Object)->m_FontSize>190) || 
					(((CExpression*)ds->Object)->m_IsHeadline)) is_title=1;

				if (kk==1)
				{
					if ((ds->Type==1) && (is_title))
					{
						int zoom=7000/((CExpression*)ds->Object)->m_FontSize;
						if (((CExpression*)ds->Object)->m_FontSize<150) zoom=zoom*110/128;
						((CExpression*)(ds->Object))->CalculateSize(&bitmapDC,zoom,&l,&a,&b,0,0);
						if ((is_title) && (RelativeX<TillenRelativeX+Tillens.Width/5))
							RelativeX=TillenRelativeX-1; 
						if (RelativeX+l>TillenRelativeX+Tillens.Width)
						{
							int correction=RelativeX-(TillenRelativeX+Tillens.Width-l);
							if (correction>Tillens.Width/5) correction=Tillens.Width/5;
							RelativeX-=correction;
							//RelativeX=TillenRelativeX+Tillens.Width-l;
							
							if (RelativeX<TillenRelativeX-1) RelativeX=TillenRelativeX-1;
						}
						//bitmapDC.FillSolidRect(RelativeX,RelativeY-a,l,a+b,RGB(255,255,255));
						((CExpression*)(ds->Object))->PaintExpression(&bitmapDC,zoom,RelativeX,RelativeY,NULL,(((CExpression*)ds->Object)->m_IsHeadline)?RGB(0,0,255):0);
						((CExpression*)(ds->Object))->CalculateSize(&bitmapDC,ViewZoom,&l,&a,&b);
					}
					continue;
				}
				if (ds->Type==1)
				{
					int fs=((CExpression*)ds->Object)->m_FontSize*Tillens.Zoom;

					if (is_title)
					{
						//do nothing ,we paint titles in the second pass
					}
					else if ((fs<1200) && (ds->Above+ds->Below<=((CExpression*)ds->Object)->m_FontSize/4))
					{
						//we do not print very small text
						bitmapDC.FillSolidRect(
							RelativeX,
							RelativeY-ds->Above*Tillens.Zoom/100,
							max(ds->Length*Tillens.Zoom/100,1),
							max((ds->Below+ds->Above)*Tillens.Zoom/100,1),RGB(160,160,160));
					}
					else
					{
						((CExpression*)(ds->Object))->CalculateSize(&bitmapDC,Tillens.Zoom,&l,&a,&b,0,0);
						((CExpression*)(ds->Object))->PaintExpression(&bitmapDC,Tillens.Zoom,RelativeX,RelativeY);
						((CExpression*)(ds->Object))->CalculateSize(&bitmapDC,ViewZoom,&l,&a,&b);
					}
				}
				else if ((ds->Type==2) && (ds!=SpecialDrawingHover))
				{
					short w,h;
					((CDrawing*)(ds->Object))->CalculateSize(&bitmapDC,Tillens.Zoom,&w,&h);
					((CDrawing*)(ds->Object))->PaintDrawing(&bitmapDC,Tillens.Zoom,RelativeX,RelativeY,ds->absolute_X,ds->absolute_Y);
					((CDrawing*)(ds->Object))->CalculateSize(&bitmapDC,ViewZoom,&w,&h);
				}
			}

			if (myrgn) 
			{
				bitmapDC.SelectClipRgn(NULL);
				delete myrgn;
			}

			if (Tillens.Enlarge>=0)
			{
				int targettillen=Tillens.Enlarge;
				int Zoom=24;
				if ((TheClientRect.bottom>500) && (TheClientRect.right>670)) Zoom=32;
				if ((TheClientRect.bottom>600) && (TheClientRect.right>970)) Zoom=40;
				int RegionX=20;
				int RegionY=50;
				int RegionW=PaperWidth*Zoom/100;
				int RegionH=PaperHeight*Zoom/100;

				int tx=targettillen/Tillens.NumY;
				int ty=targettillen%Tillens.NumY;


				int tillenx=Tillens.StartX+tx*(Tillens.Width+20);
				int tilleny=Tillens.StartY+ty*(Tillens.Height+5);
				RegionX=tillenx+Tillens.Width-RegionW-3;
				RegionY=tilleny-(RegionH-Tillens.Height)/2;
				if (RegionX<2) RegionX=2;
				if (RegionY<2) RegionY=2;
				if (RegionY+RegionH>TheClientRect.bottom-2) RegionY=TheClientRect.bottom-RegionH-2;

				bitmapDC.FillSolidRect(RegionX-3,RegionY-3,RegionW+6,RegionH+6,RGB(0,0,255));
				bitmapDC.FillSolidRect(RegionX,RegionY,RegionW,RegionH,RGB(255,255,255));
				CRgn myrgn;
				myrgn.CreateRectRgn(RegionX,RegionY,RegionX+RegionW,RegionY+RegionH);
				bitmapDC.SelectClipRgn(&myrgn);


				ds=TheDocument;
				for (int i=NumDocumentElements-1;i>=0;i--,ds++)
				{
					int Tillen=ds->absolute_Y/PaperHeight;
					if (Tillen==targettillen)
					{
						int TillenStartY=Tillen*PaperHeight;
						int RelativeX=ds->absolute_X*Zoom/100;
						int RelativeY=(ds->absolute_Y-TillenStartY)*Zoom/100;
						RelativeX+=RegionX;
						RelativeY+=RegionY;
						if (ds->Type==1)
						{
							short l,a,b;
							int tmpzoom=Zoom;
							if ((((CExpression*)ds->Object)->m_FontSize>110) &&
								(((CExpression*)ds->Object)->m_FontSize<300))
								tmpzoom=45-(((CExpression*)ds->Object)->m_FontSize-110)/8;
							((CExpression*)(ds->Object))->CalculateSize(&bitmapDC,tmpzoom,&l,&a,&b,0,0);
							((CExpression*)(ds->Object))->PaintExpression(&bitmapDC,tmpzoom,RelativeX,RelativeY);
							((CExpression*)(ds->Object))->CalculateSize(&bitmapDC,ViewZoom,&l,&a,&b);

						}
						else if((ds->Type==2) && (ds!=SpecialDrawingHover))
						{
							short w,h;
							((CDrawing*)(ds->Object))->CalculateSize(&bitmapDC,Zoom,&w,&h);
							((CDrawing*)(ds->Object))->PaintDrawing(&bitmapDC,Zoom,RelativeX,RelativeY,ds->absolute_X,ds->absolute_Y);
							((CDrawing*)(ds->Object))->CalculateSize(&bitmapDC,ViewZoom,&w,&h);
						}
					}
				}
				bitmapDC.SelectClipRgn(NULL);
			}


		}

		POINT cursor;
		GetCursorPos(&cursor);
		ScreenToClient(&cursor);

		if (PreopenDrawingToolboxX) 
		{
			//paints a stylized popup drawing toolbox as an indcation that the real popup drawing toolbox will appear after the right mouse button gets released 
			bitmapDC.SetROP2(R2_MASKPEN);
			int xx=PreopenDrawingToolboxX-ToolboxSize;
			int yy=PreopenDrawingToolboxY-ToolboxSize;
			CPen pen1;
			CBrush brush1;
			brush1.CreateSolidBrush(RGB(240,240,255));
			pen1.CreatePen(PS_SOLID,1,RGB(224,224,224));
			bitmapDC.SelectObject(pen1);
			bitmapDC.SelectObject(brush1);
			bitmapDC.Rectangle(xx,yy,xx+ToolboxSize+5,yy+ToolboxSize+5);	
			bitmapDC.MoveTo(xx+ToolboxSize/3+1,yy);bitmapDC.LineTo(xx+ToolboxSize/3+1,yy+ToolboxSize+5);
			bitmapDC.MoveTo(xx+2*ToolboxSize/3+3,yy);bitmapDC.LineTo(xx+2*ToolboxSize/3+3,yy+ToolboxSize+5);
			bitmapDC.MoveTo(xx,yy+ToolboxSize/3+1);bitmapDC.LineTo(xx+ToolboxSize+5,yy+ToolboxSize/3+1);
			bitmapDC.MoveTo(xx,yy+2*ToolboxSize/3+3);bitmapDC.LineTo(xx+ToolboxSize+5,yy+2*ToolboxSize/3+3);
			bitmapDC.SetROP2(R2_COPYPEN);
		}
		PaintDrawingHotspot(1);
		pDC->BitBlt(0,0,TheClientRect.right,TheClientRect.bottom,&bitmapDC,0,0,SRCCOPY);

		if ((cursor.x>=0) && (cursor.y>=0))	PaintClipboard(cursor.x,cursor.y);

		if ((UseToolbar) && (Toolbox) && (Toolbox->Toolbar)) Toolbox->Toolbar->UpdateToolbar();
		PaintDrawingHotspot();
	}


	
}


// CMathomirView diagnostics

#ifdef _DEBUG
void CMathomirView::AssertValid() const
{
	CView::AssertValid();
}

void CMathomirView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMathomirDoc* CMathomirView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMathomirDoc)));
	return (CMathomirDoc*)m_pDocument;
}
#endif //_DEBUG


// CMathomirView message handlers

int StartAbsoluteX;
int StartAbsoluteY;
int StartViewX;
int StartViewY;
int PrevMouseX;
int PrevMouseY;
int MinMouseX,MinMouseY;
int ScrollModeWasAccepted=0;

tDocumentStruct *SelectedDocumentObject=NULL;
tDocumentStruct *SelectedDocumentObject2=NULL;

int prevSpacingY=-1;
int startSpacingY=-1;
CBitmap *SpacingBitmap=NULL;
CBitmap *GuidlineBitmap=NULL;
CBitmap *NewlineBitmap=NULL;


int MultipleModeWasAccepted=0;
int MovingFirstMove=0;
int ResizingFirstPass;
int LockDrawingBox=0;
int root_object_flag;

#pragma optimize("s",on)
void CMathomirView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SteadyCursorTimer=0;
	SpacebarPressedRecently=0;
	LeftClickTimer=0;
	LastQuickTypeObject=NULL;
	
	
	try{

	LastCursorPositionExpression=NULL; //delete previous cursor position information

		//if we are out of focus, the first (activating) click will not be interpreted
		
	AutosaveFreetime=0;
	
	if ((ListPopup) && (ListPopup->IsWindowVisible()))
	{
		ListPopup->ShowWindow(SW_HIDE);
	}

	if (SelectedTab)
	{
		MouseMode=10;
		return;
	}

	//clicking at tilens in document-preview mode
	if ((Tillens.Show) && (point.x>Tillens.StartX-5) && (point.y>Tillens.StartY-5) && (point.y<Tillens.EndY))
	{
		int x=(point.x-Tillens.StartX)%(Tillens.Width+20);
		int y=(point.y-Tillens.StartY)%(Tillens.Height+5);
		int TillenX=(point.x-Tillens.StartX)/(Tillens.Width+20);
		int TillenY=(point.y-Tillens.StartY)/(Tillens.Height+5);
		
		int Tillen=TillenX*Tillens.NumY+TillenY;
		if (Tillen>=Tillens.Num) return;


		if ((x>=Tillens.Width+2) && (x<=Tillens.Width+11) &&
			(y>=50) && (y<=59))
		{
			Tillens.Enlarge=Tillen;
			RepaintTheView();
			return;
		}
		if ((x>Tillens.Width) || (y>Tillens.Height)) return;


		ViewY=max(Tillen*PaperHeight+y*100/Tillens.Zoom-TheClientRect.bottom/2*100/ViewZoom,0);
		ViewX=max(x*100/Tillens.Zoom-TheClientRect.right/2*100/ViewZoom,0);
		Tillens.Show=0;
		RepaintTheView();

		return;
	}
	else if (Tillens.Show)
	{
		Tillens.Show=0;
		RepaintTheView();
	}

	
#ifdef TEACHER_VERSION
	if ((TheFileType=='r') && (DisableEditing)) return;
#endif

	theApp.m_pMainWnd->SetActiveWindow();//to activate window when clicked on it (for keyboard entry)
	Toolbox->HideSubtoolbox();
	Popup->HidePopupMenu();
	
	CDC *DC=this->GetDC();

	tDocumentStruct *ds;
	int AbsoluteX=ViewX+(int)point.x*100/(int)ViewZoom;
	int AbsoluteY=ViewY+(int)point.y*100/(int)ViewZoom;

	//if clicked over a special drawing - forward the click event
	if (!ViewOnlyMode)
	if (SpecialDrawingHover)
	{
		int X=(SpecialDrawingHover->absolute_X-AbsoluteX);
		int Y=(SpecialDrawingHover->absolute_Y-AbsoluteY);
		if (((CDrawing*)(SpecialDrawingHover->Object))->MouseClick(X,Y))
			goto on_lbuttondown_end;
	}
	



	//if drawing mode is on - start drawing
	if (ShowRullerCounter) //rullers not displayed
	if (!ViewOnlyMode)
	if ((IsDrawingMode) && (!MouseTouchingSelection) && (!(GetKeyState(VK_CONTROL)&0xFFFE)) && 
		(ClipboardDrawing==NULL) && (ClipboardExpression==NULL))
	{
		if (NumSelectedObjects)
		{
			for (int kk=0;kk<NumDocumentElements;kk++)
				if (TheDocument[kk].MovingDotState==3) TheDocument[kk].MovingDotState=0;
			NumSelectedObjects=0;
		}
		LockDrawingBox=1;
		if (tmpDrawing) {delete tmpDrawing;tmpDrawing=NULL;}
		MouseMode=6;
		if ((IsDrawingMode==6) || (IsDrawingMode==7)) UndoSave("erase drawing",20400);
		tmpDrawing=new CDrawing();
		tmpDrawing->StartCreatingItem(IsDrawingMode);
		MovingStartX=ViewX*10+(int)point.x*1000/(int)ViewZoom;;
		MovingStartY=ViewY*10+(int)point.y*1000/(int)ViewZoom;
		//if (GetKeyState(VK_MENU)&0xFFFE)
		{
			if ((SpecialDrawingHover)&& 
				(((CDrawing*)(SpecialDrawingHover->Object))->IsSpecialDrawing==50) &&
				(IsDrawingMode!=6) && (IsDrawingMode!=7) && (IsDrawingMode!=25) && (IsDrawingMode!=26))
			{
				if (GetKeyState(VK_MENU)&0xFFFE)
				{
					//special handling - when ALT key is used, use "snap to point" function
					int ppx=MovingStartX/10;
					int ppy=MovingStartY/10;
					if (tmpDrawing->FindNerbyPoint(&ppx,&ppy,NULL,0,0,0,0))
					{
						MovingStartX=ppx*10;
						MovingStartY=ppy*10;
					}
				}
			}
			else if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
			{
				MovingStartX/=10;MovingStartY/=10;
				MovingStartX=((MovingStartX+GRID/2)/GRID)*GRID;
				MovingStartY=((MovingStartY+GRID/2)/GRID)*GRID;
				MovingStartX*=10;MovingStartY*=10;
			}
		}


		goto on_lbuttondown_end;
	}
	
	if (MouseMode==102)
	{
		//placing down pasted objects (??)
		MouseMode=0;
		tDocumentStruct *ds=TheDocument;
		for (int i=0;i<NumDocumentElements;i++,ds++)
		{
			if (ds->MovingDotState==3) ds->MovingDotState=0;
			if (ds->Type==1) ((CExpression*)ds->Object)->DeselectExpression();
			if (ds->Type==2) ((CDrawing*)ds->Object)->SelectDrawing(0);

		}
		RepaintTheView();

		goto on_lbuttondown_end;
	}

	//clicked on streching handle - start stretching
	if ((NumSelectedObjects) && (SelectedHandle) && (!ViewOnlyMode))
	{
		SelectionRectOrig=SelectionRect;
		if (SelectedHandle==1) {MultipleX=SelectionRect.left;MultipleY=SelectionRect.top;}
		if (SelectedHandle==2) {MultipleX=SelectionRect.right;MultipleY=SelectionRect.top;}
		if (SelectedHandle==3) {MultipleX=SelectionRect.right;MultipleY=SelectionRect.bottom;}
		if (SelectedHandle==4) {MultipleX=SelectionRect.left;MultipleY=SelectionRect.bottom;}
		if (SelectedHandle==5) {MultipleX=SelectionRect.right;}

		MovingStartX=AbsoluteX-MultipleX;
		MovingStartY=AbsoluteY-MultipleY;
		MouseMode=9;
		ResizingFirstPass=1;
		goto on_lbuttondown_end;
	}
	
	if ((ShowRullerCounter==0) && (RullerPositionPreselected>=0) && (point.y<=12) && (!ViewOnlyMode) && (RullerType==0))
	{
		//click at the ruller
		//check if we clicked at already defined position
		int i;
		int deleted=0;
		int NumRullerGuidelines=0;
		for (i=0;i<NumDocumentElements;i++)
		{
			tDocumentStruct *dsx=TheDocument+i;
			if (dsx->absolute_Y<-1000)
			{
				if ((dsx->Type==1) && (((CExpression*)dsx->Object)->m_NumElements==1))
				{
					NumRullerGuidelines++;
					//this is a valid guideline object
					if ((dsx->absolute_X>=RullerPositionPreselected) && (dsx->absolute_X<RullerPositionPreselected+GRID))
					{
						deleted=1;
						DeleteDocumentObject(dsx);
					}
				}
			}
		}

		if (!deleted)
		{
			if (NumRullerGuidelines<12)
			{
				CExpression *e=new CExpression(NULL,NULL,100);
				AddDocumentObject(1,RullerPositionPreselected,-1101);
				TheDocument[NumDocumentElements-1].MovingDotState=0;
				TheDocument[NumDocumentElements-1].Object=(CObject*)e;
			}
		}
		InvalidateRect(NULL,0);
		UpdateWindow();
		return;
	}

	//********************  the main on-mouse-click loop - clicking on objects ************
	int five_pixels=5*100/ViewZoom;
	int is_near_object=0;
	int anything_clicked=0;
	CDrawing *pcd=ClipboardDrawing;

	
	InvalidateRect(NULL,0); //repaint the window
	UpdateWindow();//waits until repaint finishes
	
	//check if clicked on any object or empty area
	for (int i=NumDocumentElements-1;i>=0;i--)
	{
		ds=TheDocument+i;
		if ((ds->MovingDotState==5) && (!AccessLockedObjects)) continue; //ignore locked objects

		int is_expression_found=0;		

		if ((AbsoluteX>=ds->absolute_X-five_pixels) && (AbsoluteX<=ds->absolute_X+ds->Length+five_pixels) &&
			(AbsoluteY>=ds->absolute_Y-ds->Above-five_pixels) && (AbsoluteY<=ds->absolute_Y+ds->Below+five_pixels))
		{
			if ((ds->Type==1) && (ds->Object))
			{
				//check if we clicked very near an object

				CExpression *ob=(CExpression*)ds->Object;
				if ((ob->m_MaxNumColumns>1) || (ob->m_MaxNumRows>1)) 
					is_near_object=1;
				else
				{
					int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
					int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;

					CExpression *e=new CExpression(NULL,NULL,((CExpression*)ds->Object)->m_FontSize);
					e->CopyExpression((CExpression*)ds->Object,0,1,0);
					short l,a,b;
					e->CalculateSize(DC,ViewZoom,&l,&a,&b);
					short IsExpression;
					char IsParenthese=0;
					CObject *ret;


					ret=NULL;IsExpression=0;
					ret=e->SelectObjectAtPoint(DC,ViewZoom,X-5,Y,&IsExpression,&IsParenthese,1);
					if ((ret==NULL) && (IsExpression==0)) ret=e->SelectObjectAtPoint(DC,ViewZoom,X+5,Y,&IsExpression,&IsParenthese,1);
					if ((ret==NULL) && (IsExpression==0)) ret=e->SelectObjectAtPoint(DC,ViewZoom,X,Y-5,&IsExpression,&IsParenthese,1);
					if ((ret==NULL) && (IsExpression==0)) ret=e->SelectObjectAtPoint(DC,ViewZoom,X,Y+5,&IsExpression,&IsParenthese,1);
					if ((ret) || (IsExpression))
					{
						is_near_object=1; 
					}
					delete e;
				}
			}
			if ((ds->MovingDotState==3) && (!ViewOnlyMode))//moving selected objects (on moving dot)
			{
				MovingStartX=AbsoluteX;
				MovingStartY=AbsoluteY;
				MovingFirstMove=1;
				int minX,minY;
				minX=minY=0x7FFFFFFF;
				SelectedDocumentObject=SelectedDocumentObject2=ds;
				tDocumentStruct *ds2=TheDocument;
				for (int ii=0;ii<NumDocumentElements;ii++,ds2++)
					if (ds2->MovingDotState==3) 
					{
						int actX=0,actY=0;
						if ((ds2->Type==2) && (ds2->Object))
							((CDrawing*)(ds2->Object))->FindRealCorner(&actX,&actY);
						if (ds2->absolute_X+actX<minX) {minX=ds2->absolute_X+actX;SelectedDocumentObject=ds2;}
						if (ds2->absolute_Y-ds2->Above+actY<minY) {minY=ds2->absolute_Y-ds2->Above+actY;SelectedDocumentObject2=ds2;}
					}
				MouseMode=2;
				if (nFlags&MK_SHIFT) {ds->MovingDotState=0;MouseMode=0;}
				is_expression_found=1;
				anything_clicked=1;
				break;
			}
			
		}
		if ((ds->Type==1) && (ds->Object))
		{
			//for matrices, if row/column insertion is touched then we also set the 'IsInsertionPointTouched'
			//to force the mouse click action
			CExpression *exp=(CExpression*)ds->Object;
			if (((exp->m_IsColumnInsertion) || (exp->m_IsRowInsertion)) && ((exp->m_MaxNumColumns>1) || (exp->m_MaxNumRows>1)))
			{
				IsInsertionPointTouched=ds;
			}
		}

		//if clicked at object
		if ((((AbsoluteX>=ds->absolute_X) && (AbsoluteX<=ds->absolute_X+ds->Length) &&
			(AbsoluteY>=ds->absolute_Y-ds->Above) && (AbsoluteY<=ds->absolute_Y+ds->Below+2))) || (IsInsertionPointTouched==ds))
		{
			//clicked on an object 
			if ((ds->MovingDotState==2) && (ds->Type==1) && (ClipboardDrawing==NULL) && (!ViewOnlyMode))  //moving an object, holding it on moving dot
			{
				if (nFlags&MK_SHIFT)
					ds->MovingDotState=3;
				else
				{
					MovingStartX=AbsoluteX;
					MovingStartY=AbsoluteY;
					MovingFirstMove=1;
					SelectedDocumentObject=ds;
					MouseMode=2;
					is_expression_found=1;
					anything_clicked=1;
					//LongClickStartTime=GetTickCount();  //we cannot use long-click because users often long-click at the moving dot for minute position adjustments
					//LongClickPosition=point;
					//LongClickObject=(CExpression*)ds->Object;
				}
			}
			/*else if ((ClipboardExpression) && (ds->Type==1) && (ds->MovingDotState!=5) && (!ViewOnlyMode)) 
			{
				//if clipboard is non-empty
				if ((ClipboardExpression->m_NumElements>1) || (ClipboardExpression->m_pElementList->Type))
				{
					UndoSave("insert into",20303);
					int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
					int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;
					CExpression *dst=((CExpression*)(ds->Object))->CopyAtPoint(DC,ViewZoom,X,Y,ClipboardExpression);
					short l,a,b;
					((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
					char RepaintAll=0;
					l=(short)((int)l*100/(int)ViewZoom);
					a=(short)((int)a*100/(int)ViewZoom);
					b=(short)((int)b*100/(int)ViewZoom);

					if ((ds->Length>l) || (ds->Above>a) || (ds->Below>b)) RepaintAll=1;
					ds->Length=l;
					ds->Above=a;
					ds->Below=b;
					if (RepaintAll) 
						{InvalidateRect(NULL,0);UpdateWindow();} 
					else 
						GentlyPaintObject(ds, DC);

					//if we were inserting a empty text box /T/, we start keyboard entry.
					if ((ClipboardExpression->m_NumElements==1) && (ClipboardExpression->m_pElementList->Type==5))
					{
						if (((CExpression*)(ClipboardExpression->m_pElementList->pElementObject->Expression1))->m_StartAsText)
						{
							for (int kkk=0;kkk<dst->m_NumElements;kkk++)
								if (dst->CompareElement(ClipboardExpression->m_pElementList,(dst->m_pElementList+kkk)))
								{
									dst=((CExpression*)((dst->m_pElementList+kkk)->pElementObject->Expression1));
									break;
								}

							if (!ViewOnlyMode)
							if (dst->m_StartAsText)
							{
								dst->m_Selection=1;
								dst->KeyboardStart(DC,ViewZoom);
								dst->m_Alignment=1;
								KeyboardEntryObject=(CObject*)dst;
								KeyboardEntryBaseObject=ds;
							}
						}
					}
				}
				if (ClipboardExpression) {delete ClipboardExpression;ClipboardExpression=NULL;}
				SetCursor(::LoadCursor(NULL,IDC_ARROW));
				is_expression_found=1;
				anything_clicked=1;
			}*/
			else if ((ds->Type==2) && (ClipboardExpression==NULL) && (pcd==NULL) && (ds->MovingDotState!=5)) 
			{
				//clicked on drawing, we will copy the drawing into the clipbord
				if ((ds->Object) && (((CDrawing*)(ds->Object))->IsSelected))
				{

					CDrawing *drw=(CDrawing*)(ds->Object);
					if ((drw->AnyNodeSelected()) && (!ViewOnlyMode))
					{
						//handling if we are editing nodes
						if (ClipboardDrawing) {delete ClipboardDrawing;ClipboardDrawing=NULL;}
						MovingStartX=ds->absolute_X*10;
						MovingStartY=ds->absolute_Y*10;
						tmpDrawing2=drw;
						MouseMode=8;
						SelectedDocumentObject=ds;
						goto on_lbuttondown_end;
					}
					//int X=point.x*100/ViewZoom-(ds->absolute_X-ViewX);
					//int Y=point.y*100/ViewZoom-(ds->absolute_Y-ViewY);



					if ((KeyboardEntryObject==NULL) || (SpecialDrawingHover==NULL)) //note: when clicked inside drawing box while command line is displayed, we put the coordinates into the command line instead!
					if (ClipboardDrawing==NULL)
					{
						int x1,y1;
						ClipboardDrawing=(CDrawing*)ComposeDrawing(&x1,&y1,0,1);
						if (ClipboardDrawing)
						{
							MovingStartX2=ViewX+point.x*100/ViewZoom;
							MovingStartY2=ViewY+point.y*100/ViewZoom;
							if (GetKeyState(VK_MENU)&0xFFFE)
							if ((SpecialDrawingHover) &&
								(((CDrawing*)(SpecialDrawingHover->Object))->IsSpecialDrawing==50))
							{
								//we are just making "Snap to point" when the ALT key is held
								int ppx=MovingStartX2;
								int ppy=MovingStartY2;
								if ((tmpDrawing) && (tmpDrawing->FindNerbyPoint(&ppx,&ppy,NULL,0,0,0,0)))
								{
									MovingStartX2=ppx;
									MovingStartY2=ppy;
								}
							}
							ClipboardDrawing->CopyToWindowsClipboard();
							MovingStartX=MovingStartX2-x1;
							MovingStartY=MovingStartY2-y1;
							anything_clicked=1;
							moving_start_timer=GetTickCount();							
							SetCursor(theApp.LoadCursor(IDC_POINTER_COPY));
						}
					}
				}
			}
			else if ((ds->Type==1) && (ClipboardDrawing==NULL)) 
			{
				//nothing in the clipboard, then copy into the clipboard
				root_object_flag=0;
				RootObjectCopy=0;
				CExpression *selection;
				selection=((CExpression*)(ds->Object))->AdjustSelection();
				if (selection==(CExpression*)(ds->Object)) 
				{
					//the whole expression is selected, test if there is more than one
					//object selected - if yes then we are working with groups
					anything_clicked=1;
					int x1,y1;
					CDrawing *tmp=(CDrawing*)ComposeDrawing(&x1,&y1,0,0);
					if (tmp)
					{
						if (tmp->NumItems>1)
						{
							delete tmp;
							ClipboardDrawing=(CDrawing*)ComposeDrawing(&x1,&y1,0,1);
							MovingStartX2=ViewX+point.x*100/ViewZoom;
							MovingStartY2=ViewY+point.y*100/ViewZoom;
							MovingStartX=MovingStartX2-x1;
							MovingStartY=MovingStartY2-y1;
							anything_clicked=1;
							moving_start_timer=GetTickCount();							
							SetCursor(theApp.LoadCursor(IDC_POINTER_COPY));
						}						
						else
						{
							delete tmp;
							root_object_flag=1;
						}
					}
				}
				if (!ClipboardDrawing)
				{
					//not doing with groups
					if (selection)
					{
						if (selection->DecodeInternalInsertionPoint())
						{
							TouchMouseMode=1; //baba2
							MovingStartX=ViewX+point.x*100/ViewZoom;
							MovingStartY=ViewY+point.y*100/ViewZoom;
							anything_clicked=1;
							return;
						}
					}
					if (selection)
					{
						LongClickStartTime=GetTickCount();
						LongClickPosition=point;
						LongClickObject=selection;
						TouchMouseMode=1; //baba2
						MovingStartX=ViewX+point.x*100/ViewZoom;
						MovingStartY=ViewY+point.y*100/ViewZoom;
						is_expression_found=1;
						anything_clicked=1;
					}
					else if (!ViewOnlyMode)
					{
						int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
						int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;

						CExpression *e=(CExpression*)(ds->Object);
						short IsExpression=0;
						char IsParenthese=0;
						CObject *ret=e->SelectObjectAtPoint(DC,ViewZoom,X,Y,&IsExpression,&IsParenthese,1);

						if ((ret==NULL) && (IsExpression==0))
						if (((CObject*)e==KeyboardEntryObject) && (e->m_IsKeyboardEntry)) //check if clicked at keyboard cursor itself
						{
							int cX,cY;
							if (e->GetKeyboardCursorPos(&cX,&cY))
							{
								int ActualSize=e->GetActualFontSize(ViewZoom);
								if ((X>=cX-ActualSize/5) && (X<cX+ActualSize/6) && (Y>cY-ActualSize/3) && (Y<cY+ActualSize/3))
								{
									
									tElementStruct *ts=e->m_pElementList+e->m_IsKeyboardEntry-1;
									if ((ts->Type==1) && (e->m_KeyboardCursorPos==0) && (ts->pElementObject->Data1[0]==0))
									{
										IsExpression=1;
									}
								}
							}
						}

						//clicked at insertion point
						
						if ((ret) || (IsExpression))
						{
							if (IsWindowOutOfFocus) IsWindowOutOfFocus=3; //to surpress typing mode changing when clicked at blinking cursor
							TouchMouseMode=1; //baba2
							MovingStartX=ViewX+point.x*100/ViewZoom;
							MovingStartY=ViewY+point.y*100/ViewZoom;
							anything_clicked=1;
							is_expression_found=1;
						}						
					}
				}
			}
			if (is_expression_found) 
			{
				//expressions have precedance
				if (ClipboardDrawing) {delete ClipboardDrawing;ClipboardDrawing=NULL;}
				break;
			}
		}
	}
 
	//if (!ViewOnlyMode)
	//if ((!anything_clicked) && (AbsoluteX<15) && (ClipboardExpression==NULL) && (ClipboardDrawing==NULL)) //clicked on left edge, and there is nothing in the clipboard
	if ((ShowRullerCounter==0) && (point.x<=12) && (!ViewOnlyMode) && (RullerType==1))
	{
		//we are entering the spacing mode
		MouseMode=4;
		startSpacingY=AbsoluteY;
		is_near_object=1; //to suppress the following
		prevSpacingY=-1;
	}

	
	//disable multiple selection mode if special drawing is behind
	int tmp_dis=0;
	try
	{
		if ((SpecialDrawingHover) && (SpecialDrawingHover->Object))
		{
		
			if (((CDrawing*)(SpecialDrawingHover->Object))->IsSpecialDrawing==50) 
			{
				//special handling for drawing box - we normally allow multiple selection
				CDrawing *drw=(CDrawing*)SpecialDrawingHover->Object;
				if (((CDrawingBox*)(drw->SpecialData))->TheState)
					tmp_dis=1;
			}
			else
				tmp_dis=1;
		}
	}
	catch(...)
	{

	}

	if (!IsWindowOutOfFocus)
 	if (!tmp_dis)
	if ((!anything_clicked) && /*(AbsoluteX>=15) &&*/ (ClipboardExpression==NULL) && (ClipboardDrawing==NULL) && (MouseMode==0)) //multiple selection started
	{
		//we are entering multiple selection mode
		if (KeyboardEntryObject)
		{
			CExpression *expr=(CExpression*)KeyboardEntryObject;
			if (expr->m_pPaternalExpression==NULL)
			if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
			{
				expr->KeyboardStop();
				KeyboardEntryObject=NULL;
				DeleteDocumentObject(KeyboardEntryBaseObject);
				KeyboardEntryBaseObject=NULL;
				RepaintTheView();
			}
		}

		MouseMode=5;
		MultipleX=MultipleStartX=AbsoluteX;
		MultipleY=MultipleStartY=AbsoluteY;
		MultipleModeWasAccepted=1;
		if ((is_near_object) && (NewlineAddObject<0)) MultipleModeWasAccepted=0;
		is_near_object=1; //to suppress the following
	}

	if (!ViewOnlyMode)
	if ((!anything_clicked) && (!is_near_object) && (ClipboardExpression))
	{
		//clicked on empty screen, we are adding or from clipboard


		if ((NewlineAddObject>=0) && ((NewlineAddObject&0x3FFFFFFF)<NumDocumentElements) && (TheDocument[NewlineAddObject&0x3FFFFFFF].Type==1))
		{
			//clicked just below a multiline or text object - we will append a new line to that object
			UndoSave("insert new",20401);
			ClipboardExpression->AddToStackClipboard(0);

			CExpression *e=(CExpression*)TheDocument[NewlineAddObject&0x3FFFFFFF].Object;
			int last_obj=e->m_NumElements-1;
			if (((e->m_pElementList+last_obj)->Type!=2) || ((e->m_pElementList+last_obj)->pElementObject->Data1[0]!=(char)0xFF))
			{
				//we will add line wrap to this object
				e->InsertEmptyElement(e->m_NumElements,2,(char)0xFF);
			}
			e->m_Alignment=1;
			for (int i=0;i<ClipboardExpression->m_NumElements;i++)
			{
				tElementStruct *ts=(tElementStruct*)ClipboardExpression->m_pElementList+i;
				e->InsertElement(ts,e->m_NumElements);
			}
			delete ClipboardExpression;
			ClipboardExpression=NULL;
			short l,a,b;
			e->CalculateSize(DC,ViewZoom,&l,&a,&b);
			tDocumentStruct *ds=&TheDocument[NewlineAddObject&0x3FFFFFFF];
			ds->Above=(int)a*100/ViewZoom;
			ds->Below=(int)b*100/ViewZoom;
			ds->Length=(int)l*100/ViewZoom;
			this->GentlyPaintObject(ds,DC);

			
			goto on_lbuttondown_end;
		}



		int make_text=0;
		if (GuidlineElement>=0)
		{
			AbsoluteX=prevGuidlineX;
			make_text=SelectedGuidelineType;
		}
		else if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
		{
			AbsoluteX=((AbsoluteX+GRID/2)/GRID)*GRID-3;
			AbsoluteY=((AbsoluteY+GRID/2)/GRID)*GRID;
		}


		//if clipboard holds an empty expression then simply start the keyboard entry mode
		if (((ClipboardExpression->m_NumElements==1) && (ClipboardExpression->m_pElementList->Type==0)) || (ClipboardExpression->m_NumElements==0))
		{
			StartKeyboardEntryAt(AbsoluteX,AbsoluteY,make_text);
			goto on_lbuttondown_end;
		}
		if ((ClipboardExpression->m_NumElements==1) && (ClipboardExpression->m_pElementList->Type==5))
		{
			CExpression *expr=(CExpression*)(ClipboardExpression->m_pElementList->pElementObject->Expression1);
			if (expr->m_StartAsText)
			{
				StartKeyboardEntryAt(AbsoluteX,AbsoluteY,1);
				goto on_lbuttondown_end;
			}
		}


		UndoSave("insert new",20401);
		if (ClipboardExpression) ClipboardExpression->AddToStackClipboard(0);

		//NumDocumentElements++;
		//CheckDocumentMemoryReservations();
		AddDocumentObject(1,AbsoluteX,AbsoluteY);

		ds=TheDocument+NumDocumentElements-1;

		//ds->absolute_X=AbsoluteX; 
		//ds->absolute_Y=AbsoluteY;
		//ds->Type=1; //expression
		ds->Object=(CObject*)new CExpression(NULL,NULL,DefaultFontSize);
		if (RootObjectCopy==ClipboardExpression->CalcChecksum())
		{
			((CExpression*)(ds->Object))->m_FontSize=RootObjectFontSize;
			//((CExpression*)(ds->Object))->m_FontSizeHQ=RootObjectFontSizeHQ;
			RootObjectCopy=0;
		}
		if (ClipboardExpression)
		{
			((CExpression*)(ds->Object))->CopyExpression(ClipboardExpression,0);
			delete ClipboardExpression;
			ClipboardExpression=NULL;
			SetCursor(::LoadCursor(NULL,IDC_ARROW));
		}
		short l,a,b;
		((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
		ds->Length=(short)((int)l*100/(int)ViewZoom);
		ds->Above=(short)((int)a*100/(int)ViewZoom);
		ds->Below=(short)((int)b*100/(int)ViewZoom);
		ds->MovingDotState=0;

		if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
		if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;

		GentlyPaintObject(ds,DC);
	}

	if (!ViewOnlyMode)
	if ((!anything_clicked) && (ClipboardDrawing))
	{
		//clicked on empty screen, we are adding the drawing or from clipboard

		UndoSave("insert new",20401);
		PasteDrawing(DC,point.x,point.y);

		delete ClipboardDrawing;
		ClipboardDrawing=NULL;
		SetCursor(::LoadCursor(NULL,IDC_ARROW));
	}


on_lbuttondown_end:
	this->ReleaseDC(DC);
	CView::OnLButtonDown(nFlags, point); 

	
	}
	catch(...)
	{
		FatalErrorHandling();
	}
	
}
#pragma optimize("",on)


void PaintNewlineAddMark(CDC *DC)
{
	if (!NewlineBitmap)
	{
		NewlineBitmap=new CBitmap();
		NewlineBitmap->CreateCompatibleBitmap(DC,20,100);
	}

	int nlo=NewlineAddObject&0x0FFFFFFF;

	//check if we need to delete the mark
	if ((prevNewlineAddObject>=0) && (prevNewlineAddObject<NumDocumentElements) && (nlo!=prevNewlineAddObject))
	{
	
		tDocumentStruct *ds=TheDocument+prevNewlineAddObject;
		if ((ds->Type==1) && (ds->Object))
		{
			int X=(ds->absolute_X-ViewX+3)*ViewZoom/100-1;
			int Y=(ds->absolute_Y+ds->Below-ViewY+2)*ViewZoom/100;
			if (ViewZoom<150) Y-=1;
			int Ly=((CExpression*)ds->Object)->GetActualFontSize(ViewZoom);
			Ly=Ly*3/4;
			int Lx=Ly/10;
			if (Ly<3) Ly=3;
			if (Lx<2) Lx=2;
			CDC mdc;
			mdc.CreateCompatibleDC(DC);
			mdc.SelectObject(NewlineBitmap);
			DC->BitBlt(X,Y,Lx,Ly,&mdc,0,0,SRCCOPY);
		}
	}

	//check if we need to paint new mark
	
	if ((nlo>=0) && (nlo<NumDocumentElements) && (nlo!=prevNewlineAddObject))
	{
		tDocumentStruct *ds=TheDocument+nlo;
		if ((ds->Type==1) && (ds->Object))
		{
			int X=(ds->absolute_X-ViewX+3)*ViewZoom/100-1;
			int Y=(ds->absolute_Y+ds->Below-ViewY+2)*ViewZoom/100;
			if (ViewZoom<150) Y-=1;
			int Ly=((CExpression*)ds->Object)->GetActualFontSize(ViewZoom);
			Ly=Ly*3/4;
			int Lx=Ly/10;
			if (Ly<3) Ly=3;
			if (Lx<2) Lx=2;
			CDC mdc;
			mdc.CreateCompatibleDC(DC);
			mdc.SelectObject(NewlineBitmap);
			mdc.BitBlt(0,0,Lx,Ly,DC,X,Y,SRCCOPY);
			DC->FillSolidRect(X,Y,Lx,Ly,(NewlineAddObject&0x40000000)?GREEN_COLOR:BLUE_COLOR);
		}
	}
	prevNewlineAddObject=nlo;
}

void CMathomirView::AdjustPosition(void)
{
	RECT cr;
	if (theApp.m_pMainWnd==NULL)
	{
		GetWindowRect(&cr);
		SetWindowPos(NULL,ToolboxSize,((UseToolbar)?1*ToolboxSize/2:0),cr.right-ToolboxSize,cr.bottom-((UseToolbar)?1*ToolboxSize/2:0),SWP_NOZORDER | SWP_NOACTIVATE);
		return;
	}
	theApp.m_pMainWnd->GetClientRect(&cr);
	SetWindowPos(NULL,ToolboxSize,((UseToolbar)?1*ToolboxSize/2:0),cr.right-ToolboxSize,cr.bottom-((UseToolbar)?1*ToolboxSize/2:0),SWP_NOZORDER | SWP_NOACTIVATE);
	GetClientRect(&TheClientRect);
}

void CMathomirView::OnSize(UINT nType, int cx, int cy)
{
	
	if (Tillens.Show) Tillens.Show=0;

	//quick fix: we are going to disable reception of double-click messages
	DWORD style=GetClassLong(this->m_hWnd,GCL_STYLE);
	style&=(~CS_DBLCLKS);
	SetClassLong(this->m_hWnd,GCL_STYLE,style);

	if (theApp.m_pMainWnd) AdjustPosition();
	CView::OnSize(nType, cx, cy);
	if (theApp.m_pMainWnd) RepaintTheView();
}

void ChangeZOrderOfSelectedObjects(int newpos)
{
	tDocumentStruct tmp;
	
		/*	if (prevTouchedObject==object) prevTouchedObject=NULL;
			if (prevTouchedObject>object) prevTouchedObject--;
			if (SelectedDocumentObject==object) SelectedDocumentObject=NULL;
			if (SelectedDocumentObject>object) SelectedDocumentObject--;
			if (SelectedDocumentObject2==object) SelectedDocumentObject2=NULL;
			if (SelectedDocumentObject2>object) SelectedDocumentObject2--;
			if (SpecialDrawingHover==object) SpecialDrawingHover=NULL;
			if (SpecialDrawingHover>object) SpecialDrawingHover--;
			if (prevSpecialDrawingHover==object) prevSpecialDrawingHover=NULL;
			if (prevSpecialDrawingHover>object) prevSpecialDrawingHover--;
			if (m_PopupMenuObject==object) m_PopupMenuObject=NULL;
			if (m_PopupMenuObject>object) m_PopupMenuObject--;
			LongClickObject=NULL;*/

	for (int i=0;i<NumDocumentElements;i++)
		if (TheDocument[i].MovingDotState==3)
		{
			if (newpos<i)
			{
				tmp=TheDocument[i];
				for (int j=i-1;j>=newpos;j--)
					TheDocument[j+1]=TheDocument[j];
				TheDocument[newpos]=tmp;
				LongClickObject=NULL;
				pMainView->m_PopupMenuObject=prevSpecialDrawingHover=SpecialDrawingHover=SelectedDocumentObject=SelectedDocumentObject2=prevTouchedObject=NULL;
			}
			else if (newpos>i)
			{
				tmp=TheDocument[i];
				for (int j=i;j<newpos;j++)
					TheDocument[j]=TheDocument[j+1];
				TheDocument[newpos]=tmp;
				LongClickObject=NULL;
				pMainView->m_PopupMenuObject=prevSpecialDrawingHover=SpecialDrawingHover=SelectedDocumentObject=SelectedDocumentObject2=prevTouchedObject=NULL;

			}
			newpos++;
		}
}

void CMathomirView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	PaintDrawingHotspot(1);
	SelectedTab=0;
	if (Tillens.Show) {Tillens.Show=0;InvalidateRect(NULL,1);}

	if ((StaticMessageWindow) && (StaticMessageWindow->IsWindowVisible()))
	{
		PresentationModeActiveTimer=0;
		StaticMessageWindow->ShowWindow(SW_HIDE);
		InvalidateRect(NULL,1);
	}
	if (ShowRullerCounter==0) {ShowRullerCounter=5;InvalidateRect(NULL,0);UpdateWindow();}
	ShowRullerCounter=5;

	int prevX=ViewX;
	if (GuidlineElement>=0)
	{
		GuidlineElement=-1;
		CDC *DC=GetDC();
		PaintGuidlines(DC,-1);
		ReleaseDC(DC);
	}
	if (NewlineAddObject>=0)
	{
		NewlineAddObject=-1;
		CDC *DC=GetDC();
		PaintNewlineAddMark(DC);
		ReleaseDC(DC);
	}

	if (nSBCode==SB_LINELEFT)
	{
		ViewX-=2400/ViewZoom;
		if (ViewX<0) ViewX=0;
	}
	if (nSBCode==SB_PAGELEFT)
	{
		RECT cl=TheClientRect;
		cl.right/=24;
		cl.right*=24;
		cl.right=cl.right*100/ViewZoom;
		ViewX-=cl.right;
		if (ViewX<0) ViewX=0;
	}
	if (nSBCode==SB_LINERIGHT)
	{
		ViewX+=2400/ViewZoom;
		if (ViewX>ViewMaxX) ViewX=ViewMaxX;
	}
	if (nSBCode==SB_PAGERIGHT)
	{
		RECT cl=TheClientRect;
		cl.right/=24;
		cl.right*=24;
		cl.right=cl.right*100/ViewZoom;
		ViewX+=cl.right;
		if (ViewX>ViewMaxX) ViewX=ViewMaxX;
	}
	if (nSBCode==SB_THUMBPOSITION)
	{
		ViewX=nPos;
		if (ViewX<0) ViewX=0;
		InvalidateRect(NULL,0);
		SetScrollPos(SB_HORZ,ViewX,1);
	}
	if (nSBCode==SB_THUMBTRACK)
	{
		ViewX=nPos;
		InvalidateRect(NULL,0);
	}
	if ((nSBCode!=SB_THUMBPOSITION) && (nSBCode!=SB_THUMBTRACK))
	{
		ScrollWindow((prevX-ViewX)*ViewZoom/100,0);
		SetScrollPos(SB_HORZ,ViewX,1);
	}
	PaintDrawingHotspot();
	CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMathomirView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO ScrollInfo;
	if (this->GetScrollInfo(SB_VERT,&ScrollInfo,SIF_ALL)) //this code is needed when nPos>32768 (about 20 pages)
	{
		nPos=ScrollInfo.nPos;
		if ((nSBCode==SB_THUMBTRACK) || (nSBCode==SB_THUMBPOSITION)) nPos=ScrollInfo.nTrackPos;
	}

	PaintDrawingHotspot(1);
	SelectedTab=0;
	if (Tillens.Show) {Tillens.Show=0;InvalidateRect(NULL,1);}

	if ((StaticMessageWindow) && (StaticMessageWindow->IsWindowVisible()))
	{
		PresentationModeActiveTimer=0;
		StaticMessageWindow->ShowWindow(SW_HIDE);
		InvalidateRect(NULL,1);
	}

	if (ShowRullerCounter==0) {ShowRullerCounter=5;InvalidateRect(NULL,0);UpdateWindow();}
	ShowRullerCounter=5;
	int prevY=ViewY;
	if (GuidlineElement>=0)
	{
		GuidlineElement=-1;
		CDC *DC=GetDC();
		PaintGuidlines(DC,-1);
		ReleaseDC(DC);
	}
	if (NewlineAddObject>=0)
	{
		NewlineAddObject=-1;
		CDC *DC=GetDC();
		PaintNewlineAddMark(DC);
		ReleaseDC(DC);
	}
	if (nSBCode==SB_LINEUP)
	{
		ViewY-=2400/ViewZoom;
		if (ViewY<0) ViewY=0;
	}
	if (nSBCode==SB_PAGEUP)
	{
		RECT cl=TheClientRect;
		cl.bottom/=24;
		cl.bottom*=24;
		cl.bottom=cl.bottom*100/ViewZoom;
		ViewY-=cl.bottom;
		if (ViewY<0) ViewY=0;
	}
	if (nSBCode==SB_LINEDOWN)
	{
		ViewY+=2400/ViewZoom;
		if (ViewY>ViewMaxY) ViewY=ViewMaxY;
	}
	if (nSBCode==SB_PAGEDOWN)
	{
		RECT cl=TheClientRect;
		cl.bottom/=24;
		cl.bottom*=24;
		cl.bottom=cl.bottom*100/ViewZoom;
		ViewY+=cl.bottom;
		if (ViewY>ViewMaxY) ViewY=ViewMaxY;
	}
	if (nSBCode==SB_THUMBPOSITION)
	{
		ViewY=nPos;
		if (ViewY<0) ViewY=0;
		InvalidateRect(NULL,0);
		SetScrollPos(SB_VERT,ViewY,1);	
	}
	if (nSBCode==SB_THUMBTRACK)
	{
		ViewY=nPos;
		InvalidateRect(NULL,0);
	}

	if ((nSBCode!=SB_THUMBPOSITION) && (nSBCode!=SB_THUMBTRACK))
	{
		ScrollWindow(0,(prevY-ViewY)*ViewZoom/100);
		SetScrollPos(SB_VERT,ViewY,1);
	}
	PaintDrawingHotspot();
	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

int GuidlineElement2=-1;
int NerbyListForGuidlines[32];
int SizeNerbyList;
void CheckForExtendedGuidlines(int AbsoluteY)
{
	//this functions checks if there is one another object aligned with the object
	//that has the temporary guideline - if yes the guideline will be extended to both of them

	GuidlineElement2=-1;
	if (GuidlineElement<0) return;
	int X=TheDocument[GuidlineElement].absolute_X;
	int Y=TheDocument[GuidlineElement].absolute_Y;
	if (Y<-1000) return; //ruler guidelines have priority

	int find_below;
	int dist=0x7FFFFFFF;
	
	if (AbsoluteY>Y) {find_below=1;Y+=TheDocument[GuidlineElement].Below;} else {find_below=0;Y-=TheDocument[GuidlineElement].Above;}
	for (int i=0;i<SizeNerbyList;i++)
	{
		int pos=NerbyListForGuidlines[i];
		tDocumentStruct *ds=TheDocument+pos;

		if ((i>=0) && (i<NumDocumentElements))
		if (ds->absolute_X==X)
		{
			if ((find_below) && (ds->absolute_Y-ds->Above>Y) && (ds->absolute_Y-ds->Above-Y<dist))
			{
				dist=ds->absolute_Y-ds->Above-Y;
				GuidlineElement2=pos;
			}
			if ((!find_below) && (ds->absolute_Y+ds->Below<Y) && (Y-ds->absolute_Y-ds->Below<dist))
			{
				dist=Y-ds->absolute_Y-ds->Below;
				GuidlineElement2=pos;
			}
		}
	}

	SizeNerbyList=0;
}


void PaintGuidlines(CDC *DC, int AbsoluteY)
{
	if ((prevGuidlineX==0x7FFFFFFF) && (GuidlineElement<0)) return;
	if (prevCrossShown) pMainView->PaintDrawingHotspot(1); 


	int store_background=1;
	SelectedGuidelineType=0;

	if (GuidlineBitmap==NULL)
	{
		GuidlineBitmap=new CBitmap();
		GuidlineBitmap->CreateCompatibleBitmap(DC,1,2048);
	}

	CDC mdc;
	mdc.CreateCompatibleDC(DC);
	mdc.SelectObject(GuidlineBitmap);
	if ((GuidlineElement2>=0) && (GuidlineElement2<NumDocumentElements))
		AbsoluteY=TheDocument[GuidlineElement2].absolute_Y;
	if (AbsoluteY>0) prevGuidlineY=AbsoluteY;
	
	int ymin=0,ydelta=0;
	tDocumentStruct *ds=NULL;
	if ((GuidlineElement>=0) && (GuidlineElement<NumDocumentElements))
	{
		ds=TheDocument+GuidlineElement;
		ymin=(min(prevGuidlineY,ds->absolute_Y)-5-ViewY)*ViewZoom/100;
		ydelta=(abs(prevGuidlineY-ds->absolute_Y)+10)*ViewZoom/100;
		
		//following lines are here to support for ruller guidelines
		if (ymin<0) {ydelta+=ymin;ymin=0;}
		if (ds->absolute_Y<-1000) 
		{
			ydelta=TheClientRect.bottom;
			if ((ds->Object) && (ds->Type==1))
			{
				CExpression* e=(CExpression*)ds->Object;
				if (ds->absolute_Y>-1100) SelectedGuidelineType=1;
			}
		}
	}

	if ((prevGuidlineX!=0x7FFFFFFF) && (GuidlineBitmap))
	{
		//we restore background (either fully or partially)

		int X=(prevGuidlineX-ViewX+3)*ViewZoom/100;
		if ((!ds) || (TheDocument[GuidlineElement].absolute_X!=prevGuidlineX) || (NewlineAddObject!=-1))
		{
			//full restore
			DC->BitBlt(X,0,1,2048,&mdc,0,0,SRCCOPY);
			prevGuidlineX=0x7FFFFFFF;
		}
		else
		{
			//partial restore (supresses blinking of the guide line)
			store_background=0;
			DC->BitBlt(X,0,1,ymin,&mdc,0,0,SRCCOPY);
			DC->BitBlt(X,ymin+ydelta,1,2048,&mdc,0,ymin+ydelta,SRCCOPY);			
		}
	}

	if (NewlineAddObject==-1)
	if (ds)
	{
		//we need to draw guidline
		prevGuidlineX=ds->absolute_X;
		int X=(prevGuidlineX-ViewX+3)*ViewZoom/100;

		if (store_background)
			mdc.BitBlt(0,0,1,2048,DC,X,0,SRCCOPY);

		DC->SetROP2(R2_MASKPEN);
		CPen p(PS_SOLID,1,(SelectedGuidelineType)?RGB(128,225,128):RGB(224,224,0));
		DC->SelectObject(p);
		DC->MoveTo(X,ymin);
		DC->LineTo(X,ymin+ydelta);
		DC->SetROP2(R2_COPYPEN);
	}
}


int GuidlinesMaxDistance;
int GuidlineLocked;
int GuidlinesFirstPass;

bool AreGuidlinesEnabled()
{
	if (ViewOnlyMode) return 0;
	if ((GuidlineElement==-2) || (IsDrawingMode) || (ShowRullerCounter==0)) return 0;
	if ((GetKeyState(VK_CONTROL)&0xFFFE))
	{
		if (SnapToGuidlines) return 0;
	}
	else
	{
		if (!SnapToGuidlines) return 0;
	}
	return 1;
}

bool CheckForGuidlines(int i,int AbsoluteX, int AbsoluteY, int show_range)
{
	//this checks if the given document element 'i' is suitable as guideline base element
	//if yes, it marks 'GuidlineElement' variable
	//This function is called for every document element (of type 'expression')

	tDocumentStruct *ds=TheDocument+i;
	if (GuidlinesFirstPass) {GuidlinesMaxDistance=400;GuidlineLocked=0;GuidlinesFirstPass=0;}

	if ((ds->Type==1) && (ds->MovingDotState==0))
	{
		if (show_range==0) show_range=1;

		int upperY=ds->absolute_Y-ds->Above;
		int lowerY=ds->absolute_Y+ds->Below;
		if (((AbsoluteY<upperY) || (AbsoluteY>lowerY)) &&
			(((AbsoluteY>upperY-600) && (AbsoluteY<lowerY+600)) || (ds->absolute_Y<-1050)))   //special 'guidline object' are at absoluteY<-1000
		{
			if ((show_range<5) && ((i==prevGuidlineElement) || (i==prevGuidlineElement2))) show_range++; 

			if ((AbsoluteX>=ds->absolute_X-show_range) && (AbsoluteX<ds->absolute_X+show_range))
			{
				if (SizeNerbyList<32) NerbyListForGuidlines[SizeNerbyList++]=i;
				if ((i==prevGuidlineElement) || (i==prevGuidlineElement2)) {GuidlineLocked=1;return 1;}
			}
			if (GuidlineLocked) return 0;

			if ((AbsoluteX>=ds->absolute_X-9) && (AbsoluteX<ds->absolute_X+9))
			{
				//if there are objects that are near altough not in scope, we will reject the more distant one
				int t=min(abs(upperY-AbsoluteY),abs(lowerY-AbsoluteY));
				if (t<GuidlinesMaxDistance) {GuidlinesMaxDistance=t;GuidlineElement=-1;}
			}

			if ((AbsoluteX>=ds->absolute_X-show_range) && (AbsoluteX<ds->absolute_X+show_range))
			{	
				if (((AbsoluteY>=upperY-GuidlinesMaxDistance) && (AbsoluteY<=lowerY+GuidlinesMaxDistance) &&	(AbsoluteY>upperY-80) && (AbsoluteY<lowerY+350))
					|| (ds->absolute_Y<-1050))
				{
					if (ds->absolute_Y<-1050) GuidlinesMaxDistance=0;
					if (GuidlineElement<0) 
					{
						return 1;
					}
					else
					{
						tDocumentStruct *ds2=TheDocument+GuidlineElement;

						int ydist1=(AbsoluteY>ds->absolute_Y)?(AbsoluteY-lowerY):(upperY-AbsoluteY)*2;
						int ydist2=(AbsoluteY>ds2->absolute_Y)?(AbsoluteY-ds2->absolute_Y-ds2->Below):(ds2->absolute_Y-ds2->Above-AbsoluteY)*2;

						int dist1=abs(ds->absolute_X-AbsoluteX)*8+ydist1;
						int dist2=abs(ds2->absolute_X-AbsoluteX)*8+ydist2;
						
						//support for ruller guidlines (have precedance)
						if (ds2->absolute_Y<-1050) dist2=-1;
						if (ds->absolute_Y<-1050) dist1=-1;

						if (dist1<dist2) return 1;						
					}
				}
			}
		}
	}

	return 0;
}

void CMathomirView::PaintDrawingHotspot(char erase_only)
{
	char is_ctrl=(GetKeyState(VK_CONTROL)&0xfffe)?1:0;
	if ((prevCursorSize==0) && (prevCrossShown==0) && (ToolbarUseCross==0) && (is_ctrl==0) && (IsDrawingMode==0)) return;
	
	
	POINT point;
	GetCursorPos(&point);
	this->ScreenToClient(&point);
	CDC *dc=GetDC();
	CDC mdc;
	mdc.CreateCompatibleDC(dc);

	if ((prevCursorSize) && (CursorBitmap))
	{
		//restoring the cursor background
		mdc.SelectObject(CursorBitmap);
		dc->BitBlt(prevCursorX,prevCursorY,prevCursorSize,prevCursorSize,&mdc,0,0,SRCCOPY);
		prevCursorSize=0;
	}

	if ((prevCrossShown) && (CrossBitmapX))
	{
		//restoring the cross background
		if (prevCrossShown&0x01)
		{
			mdc.SelectObject(CrossBitmapX);
			dc->BitBlt(0,prevCrossY,prevCrossX-50,1,&mdc,0,0,SRCCOPY);
			dc->BitBlt(prevCrossX+50,prevCrossY,TheClientRect.right-prevCrossX-50,1,&mdc,prevCrossX+50,0,SRCCOPY);
		}
		if (prevCrossShown&0x02)
		{	
			mdc.SelectObject(CrossBitmapY);
			dc->BitBlt(prevCrossX,0,1,prevCrossY-50,&mdc,0,0,SRCCOPY);
			dc->BitBlt(prevCrossX,prevCrossY+50,1,TheClientRect.bottom-prevCrossY-50,&mdc,0,prevCrossY+50,SRCCOPY);
		}
		prevCrossShown=0;
	}


	if ((ToolbarUseCross) || (is_ctrl))
	{
		//paints the cursor cross
		if (CrossBitmapX==NULL) {CrossBitmapX=new CBitmap();CrossBitmapX->CreateCompatibleBitmap(dc,4096,1);}
		if (CrossBitmapY==NULL) {CrossBitmapY=new CBitmap();CrossBitmapY->CreateCompatibleBitmap(dc,1,4096);}

		if (!erase_only)
		if ((ToolbarUseCross) || (GetKeyState(VK_CONTROL)&0xfffe))
		if (KeyboardEntryObject==NULL)
		if (ShowRullerCounter)
		if ((MouseMode!=1) && (MouseMode!=5) && (MouseMode!=9) && (MouseMode!=11))
		if ((point.x>=0) && (point.y>=0) && (point.x<TheClientRect.right) && (point.y<TheClientRect.bottom))
		{
			prevCrossX=point.x;
			prevCrossY=point.y;
			mdc.SelectObject(CrossBitmapX);
			mdc.BitBlt(0,0,TheClientRect.right,1,dc,0,prevCrossY,SRCCOPY);
			mdc.SelectObject(CrossBitmapY);
			mdc.BitBlt(0,0,1,TheClientRect.bottom,dc,prevCrossX,0,SRCCOPY);

			dc->SetROP2(R2_MASKPEN);
			CPen pen(PS_SOLID,1,RGB(224,224,224));
			dc->SelectObject(pen);

			if (1)
			{
				dc->MoveTo(0,prevCrossY); dc->LineTo(prevCrossX-50,prevCrossY);
				dc->MoveTo(prevCrossX+50,prevCrossY); dc->LineTo(TheClientRect.right,prevCrossY);
				prevCrossShown|=0x01;
			}


			if (GuidlineElement<0)
			{
				dc->MoveTo(prevCrossX,0); dc->LineTo(prevCrossX,prevCrossY-50);
				dc->MoveTo(prevCrossX,prevCrossY+50); dc->LineTo(prevCrossX,TheClientRect.bottom);
				prevCrossShown|=0x02;
			}

			dc->SetROP2(R2_COPYPEN);
		}

	}

	if ((IsDrawingMode) && (ShowRullerCounter))
	{
		
		if (CursorBitmap==NULL)
		{
			CursorBitmap=new CBitmap();
			CursorBitmap->CreateCompatibleBitmap(dc,64,64);
		}
		mdc.SelectObject(CursorBitmap);
		if (!erase_only)
		if ((IsDrawingMode) && (!MouseTouchingSelection) &&
			((GetKeyState(VK_LBUTTON)&0xFFFE)==0) && 
			((GetKeyState(VK_CONTROL)&0xFFFE)==0) && 
			((GetKeyState(VK_UP)&0xFFFE)==0) && 
			((GetKeyState(VK_DOWN)&0xFFFE)==0) && 
			((GetKeyState(VK_LEFT)&0xFFFE)==0) && 
			((GetKeyState(VK_RIGHT)&0xFFFE)==0) && 
			((GetKeyState(VK_RBUTTON)&0xFFFE)==0))
		if ((IsDrawingMode<50) && (IsDrawingMode!=6) && (IsDrawingMode!=7)) //not for special drawing nor erasers
		if ((IsDrawingMode<10) || (IsDrawingMode>13)) //not for rasters (linear and logaritmic)
		if ((IsDrawingMode<18) || (IsDrawingMode>26)) //not for section line, Okay, ?, !, X and diagonals
		{
			unsigned int clr=ColorTable[SelectedLineColor&0x07];
			
			int w=SelectedLineWidth*ViewZoom/100;
			int dot_shape=1;
			int delta=0;
			if (w<DRWZOOM) {prevCursorSize=1;dot_shape=1;}//hair
			else if (w<3*DRWZOOM/2) {prevCursorSize=2;dot_shape=2;delta=1;} //thin
			else if (w<2*DRWZOOM) {prevCursorSize=2;dot_shape=3;delta=1;} // 3/2
			else if (w<4*DRWZOOM) {prevCursorSize=(w+DRWZOOM/5)/DRWZOOM;dot_shape=4;delta=1;}
			else {prevCursorSize=(w+DRWZOOM/5)/DRWZOOM;dot_shape=5;delta=1;}

			//prevCursorSize=max((SelectedLineWidth+400)*ViewZoom/50000,1);
								
			prevCursorX=point.x-(prevCursorSize)/2-1+delta;
			prevCursorY=point.y-(prevCursorSize)/2-1+delta;
			mdc.BitBlt(0,0,prevCursorSize,prevCursorSize,dc,prevCursorX,prevCursorY,SRCCOPY);

			
			int r=(clr>>16)&0xFF;
			int g=(clr>>8)&0xFF;
			int b=(clr)&0xFF;
			if (r+b+g<190)
			{
				r=r*5/4;
				g=g*5/4;
				b=b*5/4;
				r=min(r,255);
				g=min(g,255);
				b=min(b,255);
				clr=r*256*256+g*256+b;
			}
			
			if (dot_shape==1) //hair
			{
				dc->SetPixelV(prevCursorX,prevCursorY,PALE_RGB(clr));
				//dc->FillSolidRect(prevCursorX,prevCursorY,prevCursorSize,prevCursorSize,PALE_RGB(clr);
			}
			else if (dot_shape==2) // thin
			{
				//dc->FillSolidRect(prevCursorX,prevCursorY,prevCursorSize,prevCursorSize,PALE_RGB(PALE_RGB(clr)));
				
				dc->SetPixelV(prevCursorX,prevCursorY,(clr));
				dc->SetPixelV(prevCursorX+1,prevCursorY,(clr));
				//dc->SetPixelV(prevCursorX,prevCursorY+1,PALE_RGB(clr));
				//dc->SetPixelV(prevCursorX+1,prevCursorY,PALE_RGB(clr));
			}
			else if (dot_shape==3) //3/2
			{
				dc->FillSolidRect(prevCursorX,prevCursorY,prevCursorSize,prevCursorSize,PALE_RGB(clr));
				
			}
			else if (dot_shape==4) //medium and thick
			{
				dc->FillSolidRect(prevCursorX,prevCursorY,prevCursorSize,prevCursorSize,clr);
			}
			else
			{
				CPen p;
				p.CreatePen(PS_SOLID,1,clr);
				CBrush b;
				b.CreateSolidBrush(clr);
				dc->SelectObject(p);
				dc->SelectObject(b);
				dc->Ellipse(prevCursorX,prevCursorY,prevCursorX+prevCursorSize,prevCursorY+prevCursorSize);
			}
			
		}
		
	}
	ReleaseDC(dc);
	return;
}

int prevDrawingMode2;

int prevSelectionChecksum=0;
int prev_any_drawing_touched=0;
int prevXX,prevYY;
int prevSelectedHandle=0;
char IsMouseUsed=0;
int GuidlinesTemporaryDisabled=0;
extern int AlternateVirtualKeyboard;

POINT prevMousePos;
unsigned int prevMouseTime;
int SelectedTabObjectIndex;


void CMathomirView::OnMouseMove(UINT nFlags, CPoint point)
{

	try
	{
		if (SpacebarPressedRecently)
		{
			//if spacebar was hit in order to quick-type some plain text, but cursor traveled far away (3 pixels) then 
			//we cancel the text-typing mode
			POINT cp;
			GetCursorPos(&cp);
			int ppx=SpacebarPressedPos&0x7FFF;
			int ppy=SpacebarPressedPos/32768;
			if ((abs(ppx-cp.x)>3) || (abs(ppy-cp.y)>3)) 
			{
				SpacebarPressedRecently=0;
			}
		}

		IsInsertionPointTouched=NULL;

	PaintDrawingHotspot();

	if (Popup->IsWindowVisible()) return; //(if not then the selection clears when shift key is depressed)


	if ((IsDrawingMode) && ((GetKeyState(VK_CONTROL)&0xFFFE)==0))
	{
		//when switched to drawing mode, we cannot carry anything
		if (ClipboardExpression) {delete ClipboardExpression;ClipboardExpression=NULL;}
		if (ClipboardDrawing) {delete ClipboardDrawing;ClipboardDrawing=NULL;}
	}

	if (AlternateVirtualKeyboard)
	{
		POINT cursor;
		GetCursorPos(&cursor);
		if ((Toolbox) && (Toolbox->Keyboard) && (::WindowFromPoint(cursor)!=Toolbox->Keyboard->m_hWnd)) 
		{
			AlternateVirtualKeyboard=0;
			Toolbox->KeyboardHit(0,0);
			Toolbox->Keyboard->InvalidateRect(NULL,0);
			Toolbox->Keyboard->UpdateWindow();
		}
	}

	if ((Tillens.Show) && (Tillens.Enlarge!=-1) && ((nFlags&MK_LBUTTON)==0))
	{
		Tillens.Enlarge=-1;
		RepaintTheView();
	}
	if ((Tillens.Show) && (point.x>Tillens.StartX) && (point.y>Tillens.StartY) && (point.y<Tillens.EndY))
	{
		SetMousePointer();
		return;
	}
	

	//determinig mouse velocity
	int mdist=abs(prevMousePos.x-point.x)+abs(prevMousePos.y-point.y);
	int mnow=GetTickCount();
	int mtspan=mnow-prevMouseTime;
	int mspeed=1000000;
	if (mtspan) mspeed=10*mdist/mtspan;
	prevMouseTime=mnow;
	if (mspeed>4) 
		MouseHighVelocity=1;
	else 
		MouseHighVelocity=0;
	prevMousePos.x=point.x;
	prevMousePos.y=point.y;


	CDC *DC=this->GetDC();



	if (IsDrawingMode) prevDrawingMode2=IsDrawingMode;
	if ((ClipboardExpression) || (ClipboardDrawing))
	{
		//we cannot have both clipboards active at the same time, expressions have precedance
		if ((ClipboardExpression) && (ClipboardDrawing)){delete ClipboardDrawing;ClipboardDrawing=NULL;}
		PaintClipboard(point.x,point.y);
	}
	
	//capturing cursor (if outside of client area, or atop other window, release)
	int NoNewSelections=0;
	if ((!(nFlags&MK_LBUTTON)) && (!(nFlags&MK_RBUTTON)))
	{
		if ((point.x<TheClientRect.left) || (point.x>TheClientRect.right) || (point.y<TheClientRect.top) || (point.y>TheClientRect.bottom))
		{
			RestoreClipboardBackground();
			ReleaseCapture();
			PaintDrawingHotspot();
			NoNewSelections=1; //mouse will select nothing more 
		}
		else
		{
			if ((point.x>10) && (point.y>10) && (point.x<TheClientRect.right-10) && (point.y<TheClientRect.bottom-10))
				IsMouseUsed=3;

			POINT cursor;
			GetCursorPos(&cursor);
			if ((::WindowFromPoint(cursor))!=this->m_hWnd)
			{
				ReleaseCapture();
				PaintDrawingHotspot();
				NoNewSelections=1;
			}
			else
				SetCapture();
		}
	}
	else
	{
		SetCapture();
	}

	//calculate absolute positions of mouse arrow
	int AbsoluteX=ViewX+point.x*100/ViewZoom;
	int AbsoluteY=ViewY+point.y*100/ViewZoom;

	SetMousePointer();

	int isCTRL=GetKeyState(VK_CONTROL)&0xFFFE;


	if (MouseMode==10) //adjusting size of spacers (tabs)
	{
		if ((SelectedTab) && (nFlags&MK_LBUTTON))
		{
			try
			{
				CExpression *parent=(CExpression*)SelectedTab->m_pPaternalExpression;
				while (parent->m_pPaternalExpression) parent=(CExpression*)parent->m_pPaternalExpression;
				if ((SelectedTabObjectIndex<0) || (SelectedTabObjectIndex>=NumDocumentElements) || (TheDocument[SelectedTabObjectIndex].Object!=(CObject*)parent))
				{
					SelectedTabObjectIndex=-1;
					tDocumentStruct *ds=TheDocument;
					for (int i=0;i<NumDocumentElements;i++,ds++)
						if ((ds->Object==(CObject*)parent) && (ds->Type==1)) {SelectedTabObjectIndex=i;break;}
				}
				if (SelectedTabObjectIndex<0)
				{
					//some error
					SelectedTab=NULL;
					MouseMode=0;
					InvalidateRect(NULL,0);
					UpdateWindow();
					return;
				}
				
				tDocumentStruct *ds2=TheDocument+SelectedTabObjectIndex;
				int ActualSize=((CExpression*)SelectedTab->m_pPaternalExpression)->GetActualFontSize(ViewZoom);

				SelectedTab->Data1[3]=(unsigned char)max(min((point.x-SelectedTabX)*10/ActualSize,255),0) ;

				
				short l,a,b;
				CDC *DC=this->GetDC();
				parent->CalculateSize(DC,ViewZoom,&l,&a,&b);
				int prevlen=ds2->Length;
				ds2->Length=l*100/ViewZoom;
				ds2->Above=a*100/ViewZoom;
				ds2->Below=b*100/ViewZoom;
				if (prevlen-ds2->Length>4)
				{
					this->ReleaseDC(DC);
					InvalidateRect(NULL,1);
					UpdateWindow();
				}
				else
				{
					GentlyPaintObject(ds2,DC);
					this->ReleaseDC(DC);
				}
				return;
			}
			catch(...)
			{
				MouseMode=0;
				SelectedTab=0;
			}
		}
		else
		{
			MouseMode=0;
			InvalidateRect(NULL,0);
			UpdateWindow();
		}
	}

	if ((IsDrawingMode) && (!isCTRL))
	{
		if (prevDrawingMode!=IsDrawingMode)
		{
			if (tmpDrawing) {delete tmpDrawing;tmpDrawing=NULL;}
			tDocumentStruct *ds=TheDocument;
			for (int ii=NumDocumentElements-1;ii>=0;ii--,ds++)
				if (ds->Object)
				{
					if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(0);
					if (ds->Type==1) ((CExpression*)(ds->Object))->DeselectExpression();
				}

			RepaintTheView();
			prevDrawingMode=IsDrawingMode;
		}

		if ((IsDrawingMode==6) || (IsDrawingMode==7)) //special handling for eraser
		{
			int AbsoluteX=ViewX*10+point.x*1000/ViewZoom;
			int AbsoluteY=ViewY*10+point.y*1000/ViewZoom;
			if (tmpDrawing==NULL)
			{
				tmpDrawing=new CDrawing();
				tmpDrawing->StartCreatingItem(IsDrawingMode);
				MovingStartX=AbsoluteX;
				MovingStartY=AbsoluteY;
			}
			tmpDrawing->UpdateCreatingItem(AbsoluteX-MovingStartX,AbsoluteY-MovingStartY,0,0);
			DC->BitBlt(0,0,MainViewCX,MainViewCY,&bitmapDC,0,0,SRCCOPY);
			tmpDrawing->PaintDrawing(DC,ViewZoom,(MovingStartX/10-ViewX)*ViewZoom/100,(MovingStartY/10-ViewY)*ViewZoom/100,-1,-1);
		}
	}
	

	if (!ViewOnlyMode)
	if ((nFlags&MK_LBUTTON) && (ClipboardDrawing) && (moving_start_timer))
	{
		//moving of drawings
		if (GetTickCount()-moving_start_timer>100)
		{
			//we are going to delete every drawing that is selected
			UndoSave("move",20402);
			tDocumentStruct *ds=TheDocument;
			for (int i=NumDocumentElements-1;i>=0;i--,ds++)
				if ((ds->Object) && (ds->MovingDotState==4))
					ds->MovingDotState=6; //prepared for moving
			InvalidateRect(NULL);
			moving_start_timer=0;
			MouseMode=7;
		}
	}

	if (ShowRullerCounter==0) //when the ruller is shown
	{
		if (RullerType==0)
		{
			int prevrpp=RullerPositionPreselected;
			RullerPositionPreselected=AbsoluteX;
			RullerPositionPreselected=((RullerPositionPreselected+GRID/2)/GRID)*GRID-3;
			if (prevrpp!=RullerPositionPreselected) {InvalidateRect(NULL,0);UpdateWindow();}
			this->ReleaseDC(DC);
			return;
		}
	}

	if (TouchMouseMode) //BABA2
	{
		if (TouchMouseMode==3)
		{
			if ((abs(MovingStartX-AbsoluteX)>5) || (abs(MovingStartY-AbsoluteY)>5))
			{
				TouchMouseMode=0;
				OnMouseMove(0,point); //deselect;
				InvalidateRect(NULL,0);
				UpdateWindow();
			}
		}
		else if ((nFlags&MK_LBUTTON)==0)
		{
			TouchMouseMode=0;
		}
		else if (TouchMouseMode==1)
		{
			if ((abs(MovingStartX-AbsoluteX)>2) || (abs(MovingStartY-AbsoluteY)>2))
			{
				TouchMouseMode=2;
			}
		}
	}
	

	if (MouseMode==8)  //NodeEditing
	{
		if (((nFlags&MK_LBUTTON)==0) || (tmpDrawing2==NULL))
		{
			MouseMode=0;
			int x,y,w,h;
			if (tmpDrawing2) tmpDrawing2->AdjustCoordinates(&x,&y,&w,&h);
			if ((SelectedDocumentObject) && (SelectedDocumentObject->Object==(CObject*)tmpDrawing2))
			{
				SelectedDocumentObject->absolute_X+=x;
				SelectedDocumentObject->absolute_Y+=y;
				//UndoSave("NodeEdit"); - for some reason, it doesn't work
			}	
			if (tmpDrawing2) tmpDrawing2=NULL;
			RepaintTheView();
		}
		else if (tmpDrawing2)
		{
			int AbsoluteX=ViewX*10+point.x*1000/ViewZoom;
			int AbsoluteY=ViewY*10+point.y*1000/ViewZoom;
			tmpDrawing2->MoveNodeCoordinate(AbsoluteX-MovingStartX,AbsoluteY-MovingStartY);
			RepaintTheView();
		}	
	}

	if (MouseMode==6)  //drawing
	{
		//something is being drawn
		if (((nFlags&MK_LBUTTON)==0) || (tmpDrawing==NULL))
		{
			MouseMode=0;
			if (tmpDrawing) {delete tmpDrawing;tmpDrawing=NULL;}
			RepaintTheView();
		}
		else if (tmpDrawing)
		{
			int AbsoluteX=ViewX*10+point.x*1000/ViewZoom;
			int AbsoluteY=ViewY*10+point.y*1000/ViewZoom;

			if ((!SpecialDrawingHover) || (((CDrawing*)SpecialDrawingHover->Object)->IsSpecialDrawing!=50))  //we never do snap to grid inside drawing box
			if ((IsDrawingMode!=7) && (IsDrawingMode!=6) && 
				(IsDrawingMode!=26) && (IsDrawingMode!=25) &&
				(IsDrawingMode!=3) && (IsDrawingMode!=14))			
			if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
			{
				AbsoluteX/=10;AbsoluteY/=10;
				AbsoluteX=((AbsoluteX+GRID/2)/GRID)*GRID;
				AbsoluteY=((AbsoluteY+GRID/2)/GRID)*GRID;
				AbsoluteX*=10;AbsoluteY*=10;
			}
			if ((prevXX!=AbsoluteX) || (prevYY!=AbsoluteY))
			{
				PaintDrawingHotspot(1);
				tmpDrawing->UpdateCreatingItem(AbsoluteX-MovingStartX,AbsoluteY-MovingStartY,AbsoluteX,AbsoluteY);
				InvalidateRect(NULL,0);
				UpdateWindow();
				prevXX=AbsoluteX;
				prevYY=AbsoluteY;
			}
		}	
	}



	if ((MouseMode==9) || (MouseMode==11)) //streching selection  or rotating selection (holding it on streching handles with left or right button)
	{
		if (tmpDrawing2) {delete tmpDrawing2;tmpDrawing2=NULL;}

		if (nFlags&MK_LBUTTON) //stretching
		{
			if (SelectedHandle==1) {SelectionRect.left=AbsoluteX-MovingStartX;SelectionRect.top=AbsoluteY-MovingStartY;}
			if (SelectedHandle==2) {SelectionRect.right=AbsoluteX-MovingStartX;SelectionRect.top=AbsoluteY-MovingStartY;}
			if (SelectedHandle==3) {SelectionRect.right=AbsoluteX-MovingStartX;SelectionRect.bottom=AbsoluteY-MovingStartY;}
			if (SelectedHandle==4) {SelectionRect.left=AbsoluteX-MovingStartX;SelectionRect.bottom=AbsoluteY-MovingStartY;}
			if (SelectedHandle==5) {SelectionRect.right=AbsoluteX-MovingStartX;}

			if (ResizingFirstPass)
			if ((SelectionRect.left!=SelectionRectOrig.left) || (SelectionRect.right!=SelectionRectOrig.right) ||
				(SelectionRect.top!=SelectionRectOrig.top) || (SelectionRect.bottom!=SelectionRectOrig.bottom))
			{
				UndoSave("resizing",20116);
				ResizingFirstPass=0;
			}

			float fx=(float)(SelectionRect.right-SelectionRect.left)/(float)(SelectionRectOrig.right-SelectionRectOrig.left);
			float fy=(float)(SelectionRect.bottom-SelectionRect.top)/(float)(SelectionRectOrig.bottom-SelectionRectOrig.top);

			if ((nFlags&MK_SHIFT) || (GetKeyState(VK_RBUTTON)&0xFFFE))
			{
				fx=max(fx,fy);
				fy=fx;
				if (SelectedHandle==1) {SelectionRect.left=(long)(-(SelectionRectOrig.right-SelectionRectOrig.left)*fx+SelectionRectOrig.right);SelectionRect.top=(long)(-(SelectionRectOrig.bottom-SelectionRectOrig.top)*fy+SelectionRectOrig.bottom);}
				if (SelectedHandle==2) {SelectionRect.right=(long)((SelectionRectOrig.right-SelectionRectOrig.left)*fx+SelectionRectOrig.left);SelectionRect.top=(long)(-(SelectionRectOrig.bottom-SelectionRectOrig.top)*fy+SelectionRectOrig.bottom);}
				if (SelectedHandle==3) {SelectionRect.right=(long)((SelectionRectOrig.right-SelectionRectOrig.left)*fx+SelectionRectOrig.left);SelectionRect.bottom=(long)((SelectionRectOrig.bottom-SelectionRectOrig.top)*fy+SelectionRectOrig.top);}
				if (SelectedHandle==4) {SelectionRect.left=(long)(-(SelectionRectOrig.right-SelectionRectOrig.left)*fx+SelectionRectOrig.right);SelectionRect.bottom=(long)((SelectionRectOrig.bottom-SelectionRectOrig.top)*fy+SelectionRectOrig.top);}
			}
			
	
			RepaintTheView(); 


			if (SelectedHandle==5)
			{
				if ((SingleSelectedTextObject>=0) && (SingleSelectedTextObject<NumDocumentElements))
				{
					tDocumentStruct *ds=TheDocument+SingleSelectedTextObject;
					CExpression *e=(CExpression*)ds->Object;
					e->AutowrapText(DC,(AbsoluteX-ds->absolute_X)*ViewZoom/100,1);
					short l,a,b;
					e->CalculateSize(DC,ViewZoom,&l,&a,&b);
					ds->Length=l*100/ViewZoom;
					ds->Above=a*100/ViewZoom;
					ds->Below=b*100/ViewZoom;
					SelectionRect.right=SelectionRect.left+(ds->Length);
					SelectionRect.bottom=SelectionRect.top+(ds->Above+ds->Below);
				}

			}
			else
			{
				int sx,sy;
				tmpDrawing2=(CDrawing*)ComposeDrawing(&sx,&sy,1,0);
				int x,y,w,h;
				short ww,hh;
				tmpDrawing2->AdjustCoordinates(&x,&y,&w,&h);
				tmpDrawing2->ScaleForFactor(fx,fy);
				tmpDrawing2->AdjustCoordinates(&x,&y,&w,&h);
				tmpDrawing2->CalculateSize(DC,ViewZoom,&ww,&hh);
				int minx=SelectionRect.left,maxx=SelectionRect.right;
				if (maxx<minx) {minx=SelectionRect.right;maxx=SelectionRect.left;}
				int miny=SelectionRect.top,maxy=SelectionRect.bottom;
				if (maxy<miny) {miny=SelectionRect.bottom;maxy=SelectionRect.top;}
				if (SelectedHandle==1) tmpDrawing2->PaintDrawing(DC,ViewZoom,(short)(maxx-w-ViewX)*ViewZoom/100,(short)(maxy-h-ViewY)*ViewZoom/100,-1,-1);
				if (SelectedHandle==2) tmpDrawing2->PaintDrawing(DC,ViewZoom,(short)(minx-ViewX)*ViewZoom/100,(short)(maxy-h-ViewY)*ViewZoom/100,-1,-1);
				if (SelectedHandle==3) tmpDrawing2->PaintDrawing(DC,ViewZoom,(short)(minx-ViewX)*ViewZoom/100,(short)(miny-ViewY)*ViewZoom/100,-1,-1);
				if (SelectedHandle==4) tmpDrawing2->PaintDrawing(DC,ViewZoom,(short)(maxx-w-ViewX)*ViewZoom/100,(short)(miny-ViewY)*ViewZoom/100,-1,-1);
			}
		}
		else if (nFlags&MK_RBUTTON) //rotating
		{
			SelectionRect.top=SelectionRect.bottom=SelectionRect.left=SelectionRect.right=-1234;

			if (ResizingFirstPass)
			if ((SelectionRect.left!=SelectionRectOrig.left) || (SelectionRect.right!=SelectionRectOrig.right) ||
				(SelectionRect.top!=SelectionRectOrig.top) || (SelectionRect.bottom!=SelectionRectOrig.bottom))
			{
				UndoSave("rotating",20114);
				ResizingFirstPass=0;
			}

			int cx=(SelectionRectOrig.right+SelectionRectOrig.left)/2;
			int cy=(SelectionRectOrig.top+SelectionRectOrig.bottom)/2;
			double arc1=atan2((double)(cy-MovingStartY),(double)(cx-MovingStartX));
			double arc2=atan2((double)(cy-AbsoluteY),(double)(cx-AbsoluteX));		
			
			RepaintTheView();

			int sx,sy;
			tmpDrawing2=(CDrawing*)ComposeDrawing(&sx,&sy,1,0);
			int x,y,w,h;
			short ww,hh;
			tmpDrawing2->AdjustCoordinates(&x,&y,&w,&h);
			int ow=w/2;
			int oh=h/2;
			tmpDrawing2->RotateForAngle((float)(arc1-arc2),ow,oh,&x,&y,&w,&h);
			tmpDrawing2->CalculateSize(DC,ViewZoom,&ww,&hh);

			MultipleX=SelectionRectOrig.left+x+ow;
			MultipleY=SelectionRectOrig.top+y+oh;
			tmpDrawing2->PaintDrawing(DC,ViewZoom,(MultipleX-ViewX)*ViewZoom/100,(MultipleY-ViewY)*ViewZoom/100,-1,-1);
		}
		else 
		{
			//abort operation
			RepaintTheView();
			MouseMode=0;
		}
	}


	
	if (MouseMode==0) MouseTouchingSelection=0;

	if ((MouseMode==0) && (NumSelectedObjects)) //free moving - selection touched
	{
		int rl=(SelectionRect.left-ViewX)*ViewZoom/100;
		int rr=(SelectionRect.right-ViewX)*ViewZoom/100;
		int rt=(SelectionRect.top-ViewY)*ViewZoom/100;
		int rb=(SelectionRect.bottom-ViewY)*ViewZoom/100;
		int gg=MovingDotSize*3/2;
		SelectedHandle=0;
		if ((point.x>=rl-gg) && (point.x<=rl) && (point.y>=rt-gg) && (point.y<=rt)) SelectedHandle=1;
		if ((point.x>=rr) && (point.x<=rr+gg) && (point.y>=rt-gg) && (point.y<=rt)) SelectedHandle=2;
		if ((point.x>=rr) && (point.x<=rr+gg) && (point.y>=rb) && (point.y<=rb+gg)) SelectedHandle=3;
		if ((point.x>=rl-gg) && (point.x<=rl) && (point.y>=rb) && (point.y<=rb+gg)) SelectedHandle=4;
		if (SingleSelectedTextObject!=-1)
			if ((point.x>=rr+gg/3) && (point.x<=rr+gg) && (point.y>rt+gg/2) && (point.y<=rb-gg/2)) SelectedHandle=5;


		if (SelectedHandle) {NoNewSelections=1;MouseTouchingSelection=1;SetMousePointer();}
		if (SelectedHandle!=prevSelectedHandle)
		{
			InvalidateRect(NULL);
			UpdateWindow();
			prevSelectedHandle=SelectedHandle;
		}
	}

	if (GuidlinesTemporaryDisabled) GuidlinesTemporaryDisabled--;

	//checking for guidelines
	int GuidlineRange=MovingDotSize*66/ViewZoom;
	char GuidlinesAreEnabled=AreGuidlinesEnabled();
	prevGuidlineElement=GuidlineElement;
	prevGuidlineElement2=GuidlineElement2;
	GuidlinesFirstPass=1;
	GuidlineElement=-1;SizeNerbyList=0;
	
	if (SpecialDrawingHover==NULL)
	if (((MouseMode==5) && (MultipleModeWasAccepted!=2)) ||
		((MouseMode==2) && (SelectedDocumentObject) && (SelectedDocumentObject->MovingDotState==2)))
	if (GuidlinesAreEnabled)
	{
		//when not in free-moving mode (multiple-selection mode or object-moving)
		tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
		for (int i=NumDocumentElements-1;i>=0;i--,ds--)
		{
			if (ds->Type==1)
				if (CheckForGuidlines(i,AbsoluteX-3,AbsoluteY,GuidlineRange)) GuidlineElement=i;
		}
	}

	if (MouseMode!=5) NewlineAddObject=-1;
	if ((MouseMode==0) && (((IsDrawingMode!=6) && (IsDrawingMode!=7)) || (isCTRL))) //free moving
	{
		if (LockDrawingBox==0) SpecialDrawingHover=0;
		if ((IsDrawingMode==0) || (isCTRL))
		{
			prevDrawingMode=-1;
			SpecialDrawingHover=0;
			LockDrawingBox=0;
		}

		CObject *TouchedSubelement=NULL;
		tDocumentStruct *TouchedObject=NULL;
		tDocumentStruct *TouchedExpression=NULL;
		char IsTouchedExpressionFullyInside=0;
		short IsExpression=-1;
		char IsParenthese=-1;
		int ScheduleFullRepaint=0;
		int i;
		int any_drawing_touched=0;
		int any_expression_touched=0;
		int SelectionChecksum=0;
		MouseIsTouchingExpression=0;
		
		tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
		int ccc=1+100/ViewZoom;
		
		for (i=NumDocumentElements-1;i>=0;i--,ds--)
		{
			if ((ds->Type==2) && (ds->MovingDotState!=3) && (IsDrawingMode) && 
				(!isCTRL) && (((CDrawing*)ds->Object)->IsSpecialDrawing!=50)) continue;

			if ((ds->MovingDotState!=3) && (ds->MovingDotState!=5)) //not selected nor locked
				if ((ds->MovingDotState!=4) || (ClipboardDrawing==NULL))
					ds->MovingDotState=0;

			int counted_already=0;
			if (((nFlags&MK_SHIFT)==0) && (TouchMouseMode==0))
			{
				if (ds->Type==1) (KeyboardEntryObject)?(((CExpression*)(ds->Object))->DeselectExpressionExceptKeyboardSelection()):(((CExpression*)(ds->Object))->DeselectExpression());
				else if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(0);
			}
			else if(ds->MovingDotState!=3)
			{
				if (ds->Type==1)
				{
					if (((CExpression*)(ds->Object))->AdjustSelection()) 
					{
						counted_already=1;
						any_drawing_touched++;
						TouchedExpression=ds;
					}
				}
				else if ((ds->Type==2) && (((CDrawing*)(ds->Object))->IsSelected)) 
				{
					counted_already=1;
					any_drawing_touched++;
				}
			}

			if ((!MouseHighVelocity) || (prevGuidlineElement>=0))
			if (GuidlinesTemporaryDisabled==0)
			if (GuidlinesAreEnabled)
			if (ds->Type==1)
				if (CheckForGuidlines(i,AbsoluteX-3,AbsoluteY,GuidlineRange)) GuidlineElement=i;

			

			int extends=0;
			if (ds->Type==1) 
			{
				CExpression *e=(CExpression*)ds->Object;
				if (e)
				{
 					extends=3*e->m_MarginX/4;
					if (e->m_MarginX>=6) extends=4+(e->m_MarginX-6)/2;
					extends=extends*100/ViewZoom;
				}
			}

			//handling 'add new line' feature - pointing mouse pointer just below a mutli-line object
			if ((ds->Type==1) && (!IsDrawingMode))
			{
				int fntsz=(((CExpression*)ds->Object)->m_FontSize)/8;
				if ((AbsoluteY>ds->absolute_Y+ds->Below) && (AbsoluteY<ds->absolute_Y+ds->Below+fntsz) &&
					(AbsoluteX>=ds->absolute_X+3-fntsz/6) && (AbsoluteX<=ds->absolute_X+3+fntsz/3) && (ds->MovingDotState==0))
				{
					int is_multiline=0;
					int is_text=0;
					CExpression *e=(CExpression*)ds->Object;
					for (int ii=0;ii<e->m_NumElements;ii++)
						if (((e->m_pElementList+ii)->Type==2) && ((e->m_pElementList+ii)->pElementObject->Data1[0]==(char)0xFF))
						{
							is_multiline=1;
							break;
						}

					if (((e->m_NumElements>1) || ((e->m_pElementList->Type==1) &&
						(strlen(e->m_pElementList->pElementObject->Data1)>2))) && (e->m_IsHeadline==0) && (e->IsTextContained(0)))
						is_text=1;

					if ((is_multiline) && (e->m_Alignment!=1)) {is_multiline=0;is_text=0;}
					if ((is_multiline) || (is_text))
					{ 
						NewlineAddObject=i;
						if (is_text) NewlineAddObject|=0x40000000; //mark that this is a text object (must add text insertion point)
					}
				}
			}


			if ((AbsoluteY>=ds->absolute_Y-ds->Above) && (AbsoluteY<=ds->absolute_Y+ds->Below+ccc) &&
				(AbsoluteX>=ds->absolute_X-extends) && (AbsoluteX<=ds->absolute_X+ds->Length+extends) &&
				((ds->MovingDotState!=5) || (AccessLockedObjects)))
			{
				char is_fully_inside=0;
				if ((AbsoluteX>=ds->absolute_X) && (AbsoluteX<=ds->absolute_X+ds->Length)) is_fully_inside=1;

				if (ds->MovingDotState==3) 
				{
					MouseTouchingSelection=1;
					SetMousePointer();
					continue;
				}
				if (ds->Type==1) MouseIsTouchingExpression=1;
				int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
				int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;
				if ( (ds->Type==2) && (ds->Object) && (((CDrawing*)(ds->Object))->IsSpecialDrawing))
				{
					int NodeEdit;
					TouchedSubelement=((CDrawing*)(ds->Object))->SelectObjectAtPoint(DC,ViewZoom,X,Y,&NodeEdit);
					if (NodeEdit<0)
					{
						SpecialDrawingHover=ds;
						GuidlineElement=-2;
					}
					else
						((CDrawing*)(ds->Object))->SelectDrawing(0);
				}
				if (!any_expression_touched)
				{
					//mouse pointing to an object
					if ((IsDrawingMode==0) || (isCTRL))
					if ((ds->MovingDotState!=3) && (ds->MovingDotState!=5) && (ds->MovingDotState!=4) && (ds->Type!=2))
					{
						int mds=max(MovingDotSize,MovingDotSize*(ViewZoom+420)/512);
						int border=100+ViewZoom;
						int PreciseAbsX=(int)ViewX*ViewZoom+point.x*100L;
						int PreciseAbsY=(int)ViewY*ViewZoom+point.y*100L;
						int FontSz=0;
						if ((ds->Type==1) && (ds->Object)) FontSz=((CExpression*)ds->Object)->GetActualFontSize(ViewZoom)*100;
						if (((PreciseAbsY>=(ds->absolute_Y+ds->Below)*ViewZoom-mds*150+border) && (PreciseAbsY<=(ds->absolute_Y+ds->Below)*ViewZoom+border)) ||
							((PreciseAbsX<ds->absolute_X*ViewZoom+mds*ViewZoom/2+mds*60) && (PreciseAbsY>ds->absolute_Y*ViewZoom+FontSz/3)))
						{
							
							ds->MovingDotState=1;
					

							if ((GetKeyState(VK_SHIFT)&0xFFFE)==0)
							if (TouchMouseMode==0)
							if ((PreciseAbsX>=ds->absolute_X*ViewZoom) && 
								(PreciseAbsX<=ds->absolute_X*ViewZoom+mds*100) && 
								(PreciseAbsY>=(ds->absolute_Y+ds->Below)*ViewZoom-mds*100+border))
							{
								//if (!isCTRL)
								if (!NoNewSelections) ((CExpression*)ds->Object)->SelectExpression(1);
								ds->MovingDotState=2;
								GuidlineElement=-2;
								PaintGuidlines(DC,-1); //otherwise the moving dot is incorectly displayed (erased by guideline)
								NewlineAddObject=-1;
								PaintNewlineAddMark(DC);
							}
						}
					} 

					if (NoNewSelections)
						TouchedSubelement=0;
					else
					{
						if ((ds->Type==1) && (ClipboardDrawing==NULL) && ((is_fully_inside) || ((!IsTouchedExpressionFullyInside) && (any_drawing_touched==0))))
						{
							IsExpression=-1;
							
							TouchedSubelement=((CExpression*)(ds->Object))->SelectObjectAtPoint(DC,ViewZoom,X,Y,&IsExpression,&IsParenthese);
						
							if ((TouchedSubelement==NULL) && (IsExpression==-1) && (ds->MovingDotState==0)) continue;
							if ((ds->MovingDotState==2) && (TouchedSubelement)) 
							{
								((CExpression*)ds->Object)->SelectExpression(1);
							}
							if (ds->MovingDotState>=2)  ((CExpression*)ds->Object)->m_IsColumnInsertion=((CExpression*)ds->Object)->m_IsRowInsertion=0;

							if ((IsDrawingMode) && (!isCTRL))
							{
								if (((CExpression*)ds->Object)->m_InternalInsertionPoint==0)
								if ((((CExpression*)ds->Object)->m_Selection<=0) || (((CExpression*)ds->Object)->m_Selection>16383))
								{
									((CExpression*)(ds->Object))->DeselectExpression();
									continue;
								}
							}

							if ((nFlags&MK_SHIFT) || (TouchMouseMode))
							{
								((CExpression*)(ds->Object))->AdjustSelection();
							}
							else 
							{
								GuidlineElement=-2;
								any_expression_touched=1;
								SelectionChecksum=0;
								if (any_drawing_touched)
								{
									tDocumentStruct *ds3=TheDocument+NumDocumentElements-1;
									for (int iii=NumDocumentElements-1;iii>=0;iii--,ds3--)
										if (iii!=i)
										{
											
											if (ds3->Type==1) ((CExpression*)ds3->Object)->DeselectExpression();
											else if (ds3->Type==2) ((CDrawing*)ds3->Object)->SelectDrawing(0);
										}
									counted_already=0;
									any_drawing_touched=0;
								
								}
							}
							TouchedObject=ds;
							TouchedExpression=ds;
							if ((((CExpression*)ds->Object)->m_Selection>0) && (((CExpression*)ds->Object)->m_Selection<16383))
								IsInsertionPointTouched=ds;

							if (is_fully_inside) IsTouchedExpressionFullyInside=1;
							if (!counted_already) any_drawing_touched++;
							SelectionChecksum+=((CExpression*)(ds->Object))->CalcChecksum();
							SelectionChecksum+=IsExpression*16+IsParenthese*128+*(int*)&TouchedSubelement;
							SelectionChecksum+=ds->absolute_X+11*ds->absolute_Y;
							SelectionChecksum+=((CExpression*)(ds->Object))->m_IsColumnInsertion;
						}
						else if ((ds->Type==2) && (((ClipboardExpression==NULL) && (ClipboardDrawing==NULL)) || (((CDrawing*)(ds->Object))->IsSpecialDrawing)))
						{
							int NodeEdit;
							TouchedSubelement=((CDrawing*)(ds->Object))->SelectObjectAtPoint(DC,ViewZoom,X,Y,&NodeEdit);
							if (NodeEdit<0)
								SpecialDrawingHover=ds;

							IsExpression=IsParenthese=0;
							if (TouchedSubelement)
							{
								MouseIsTouchingExpression=1;
								GuidlineElement=-2;
								TouchedObject=ds;
								if (!counted_already) any_drawing_touched++;
								SelectionChecksum+=((CDrawing*)(ds->Object))->CalcChecksum();
								SelectionChecksum+=*(int*)&TouchedSubelement;
								SelectionChecksum+=NodeEdit;
								SelectionChecksum+=ds->absolute_X+11*ds->absolute_Y;
							}
						}
					}
					SelectionChecksum+=ds->MovingDotState;
				}
			}
			else
				if (ds->MovingDotState==3) continue;
		}


		
		//if (IsDrawingMode) ScheduleFullRepaint=1;
			
		if ((any_drawing_touched>1) && (TouchedExpression))
		{
			tDocumentStruct *ds2=TheDocument;
			for (int iii=0;iii<NumDocumentElements;iii++,ds2++)
				if ((ds2->Type==1) && (((CExpression*)(ds2->Object))->AdjustSelection()))
					((CExpression*)(ds2->Object))->SelectExpression(1);
			ScheduleFullRepaint=1;
		}
		
		if ((prevSelectionChecksum!=SelectionChecksum) || (ScheduleFullRepaint))
		{
				
			RepaintViewTimer=0;

			if (ScheduleFullRepaint)
				InvalidateRect(NULL,0);
			else if ((TouchedObject==prevTouchedObject) && (any_drawing_touched<2) && (prev_any_drawing_touched<2))
			{
				if (TouchedObject) ((!Tillens.Show)?GentlyPaintObject(TouchedObject,DC):RepaintTheView());
			}
			else
			{
				if ((any_drawing_touched<2) && (prev_any_drawing_touched<2))
				{
					try
					{
						if (prevTouchedObject)
							((!Tillens.Show)?GentlyPaintObject(prevTouchedObject,DC):RepaintTheView());
						if (TouchedObject)
							((!Tillens.Show)?GentlyPaintObject(TouchedObject,DC):RepaintTheView());
						else
							RepaintViewTimer=2;
					}
					catch (...)
					{
					}
				}
				else
					InvalidateRect(NULL,0);
			}
			prevTouchedObject=TouchedObject;
			prevSelectionChecksum=SelectionChecksum;
			prev_any_drawing_touched=any_drawing_touched;
		}		
		
	}

	CheckForExtendedGuidlines(AbsoluteY);
	if (NewlineAddObject==-1) PaintNewlineAddMark(DC); //this will delete the mark so that guidline can be painted
	PaintGuidlines(DC,AbsoluteY);
	PaintNewlineAddMark(DC);

	if (MouseMode==4) //spacing mode (adds space between equations - dragging on the vertical ruler)
	{		
		if (SpacingBitmap==NULL)
		{
			SpacingBitmap=new CBitmap();
			SpacingBitmap->CreateCompatibleBitmap(DC,2048,1);
		}

		int absy=AbsoluteY;
		if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
		{
			absy=((absy+GRID/2)/GRID)*GRID;
		}

		if (absy!=prevSpacingY)
		{
			UpdateWindow();
			CDC tmpDC;
			tmpDC.CreateCompatibleDC(DC);
			tmpDC.SelectObject(SpacingBitmap);
			if (prevSpacingY!=-1) DC->BitBlt(0,(prevSpacingY-ViewY)*ViewZoom/100,2048,1,&tmpDC,0,0,SRCCOPY);
			prevSpacingY=absy;

			tmpDC.BitBlt(0,0,2048,1,DC,0,(absy-ViewY)*ViewZoom/100,SRCCOPY);
			DC->FillSolidRect(0,(absy-ViewY)*ViewZoom/100,2048,1,RGB(0,0,0));
		}
	}


	if (MouseMode==5) //multiple selection (dragging a selection frame over objects)
	{
		if ((nFlags&MK_LBUTTON)==0) 
		{
			MouseMode=0;
			MultipleStartX=MultipleStartY=-1;
			RepaintTheView();
		}
		else 
		{
			if (MultipleModeWasAccepted!=2)
			{
				if ((MultipleStartX<14) && (AbsoluteX<16) && (abs(AbsoluteY-MultipleStartY)>4) && ((AbsoluteX-MultipleStartX)<abs(AbsoluteY-MultipleStartY)/2))
				{
					//switch from multiple selection mode to spacing mode (if we click-and-drag at the very left margin)
					MouseMode=4;
					startSpacingY=MultipleStartY;
					ShowRullerCounter=0;
					RullerType=1;
					MultipleStartX=MultipleStartY=-1;InvalidateRect(NULL,0);UpdateWindow();
					prevSpacingY=-1;
				}
				else if ((AbsoluteX>=16) && ((abs((AbsoluteX-MultipleStartX)*ViewZoom/100)>3) || (abs((AbsoluteY-MultipleStartY)*ViewZoom/100)>3)))
				{
					MultipleModeWasAccepted=2;
				}	
			}
			else
			{
				MultipleX=AbsoluteX;
				MultipleY=AbsoluteY;

				int tmp;
				int msX=MultipleStartX;
				int msY=MultipleStartY;
				if (msX>MultipleX) {tmp=MultipleX;MultipleX=msX;msX=tmp;}
				if (msY>MultipleY) {tmp=MultipleY;MultipleY=msY;msY=tmp;}

				tDocumentStruct	*ds=TheDocument+NumDocumentElements-1;
				for (int i=NumDocumentElements-1;i>=0;i--,ds--)
					if (ds->MovingDotState!=5)
						if ((ds->absolute_Y-ds->Above>=msY) && (ds->absolute_Y+ds->Below<=MultipleY) &&
							(ds->absolute_X>=msX) && (ds->absolute_X+ds->Length<=MultipleX))
						{
							if (ds->Type==1) ((CExpression*)(ds->Object))->SelectExpression(1);
							else if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(1);
							
							ds->MovingDotState=3;
							IsSelectionFresh=1;
						}
						else if (((nFlags&MK_SHIFT)==0) && (TouchMouseMode==0))
						{
								
							if (ds->Type==1) ((CExpression*)(ds->Object))->DeselectExpression();
							else if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(0);
							ds->MovingDotState=0;
							IsSelectionFresh=1;
						}
			}
			MultipleX=AbsoluteX;
			MultipleY=AbsoluteY;
			InvalidateRect(NULL,0);
		}
	}

	if (MouseMode==1) //right-click window scrolling
	{
		if ((nFlags&MK_RBUTTON)==0) 
			{RepaintTheView();MouseMode=0;}
		else
		{

			if ((StaticMessageWindow) && (StaticMessageWindow->IsWindowVisible()))
			{
				PresentationModeActiveTimer=0;
				StaticMessageWindow->ShowWindow(SW_HIDE);
				InvalidateRect(NULL,1);
			}

			//now we have to scroll the window
			if (ScrollModeWasAccepted)
			{
				if (point.x>MinMouseX) point.x=MinMouseX;
				if (point.y>MinMouseY) point.y=MinMouseY;
			}
			int MoveX=StartAbsoluteX-StartViewX-point.x*100/ViewZoom;
			int MoveY=StartAbsoluteY-StartViewY-point.y*100/ViewZoom;
			if (ScrollModeWasAccepted)
			{
				ViewX=StartViewX+MoveX;
				ViewY=StartViewY+MoveY;
				
				ScrollWindow(point.x-PrevMouseX,point.y-PrevMouseY);

				PrevMouseX=point.x;
				PrevMouseY=point.y;
				SetScrollPos(SB_HORZ,ViewX,1);
				SetScrollPos(SB_VERT,ViewY,1);
			}
			else
				if ((abs(MoveX*ViewZoom/100)>=5) || (abs(MoveY*ViewZoom/100)>=5))
				{
					ScrollModeWasAccepted=1;
					int clearscreen=0;
					if (PreopenDrawingToolboxX) {PreopenDrawingToolboxX=0;clearscreen=1;}
					//delete any mouse touching that was possibly done
					tDocumentStruct *ds=TheDocument;
					
					for (int i=0;i<NumDocumentElements;i++,ds++)
					{
						if ((ds->absolute_Y-ds->Above<=StartAbsoluteY) && (ds->absolute_Y+ds->Below>=StartAbsoluteY) &&
							(ds->absolute_X<=StartAbsoluteX) && (ds->absolute_X+ds->Length>=StartAbsoluteX))
						{
							clearscreen=1;
							if (ds->Type==1)
								((CExpression*)ds->Object)->DeselectExpression();
							else if (ds->Type==2)
								((CDrawing*)ds->Object)->SelectDrawing(0);
						}
					}
					if (clearscreen)
					{
						InvalidateRect(NULL,0);
						UpdateWindow();
					}
				}
		}
	}

	if (PreopenDrawingToolboxX)
	{
		if ((abs(PreopenDrawingToolboxX-point.x)>5) || (abs(PreopenDrawingToolboxY-point.y)>5) || (ScrollModeWasAccepted))
		{
			PreopenDrawingToolboxX=0;
			InvalidateRect(NULL,0);
			UpdateWindow();
		}
	}



	if ((MouseMode==2) || (MouseMode==102))  //moving objects around the document (holding it at the moving point or moving selections)
	{
		if (((nFlags&MK_LBUTTON)==0)  && (MouseMode==2))
			{RepaintTheView();MouseMode=0;}
		else
		{
			if ((AbsoluteX!=MovingStartX) || (AbsoluteY!=MovingStartY))
			{
				int deltaX=AbsoluteX-MovingStartX;
				int deltaY=AbsoluteY-MovingStartY;

				if ((SelectedDocumentObject) && (SelectedDocumentObject2) && (SelectedDocumentObject->MovingDotState==3))
				{
					//move all selected objects
					if (MovingFirstMove) {MovingFirstMove=0;UndoSave("move",20402);}

					int X=SelectedDocumentObject->absolute_X+deltaX;
					int Y=SelectedDocumentObject2->absolute_Y+deltaY;					

					if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
					{
						/*int corrx=0,corry=0;
						if (SelectedDocumentObject2->Type==1) corry+=-SelectedDocumentObject2->Above;
						if (SelectedDocumentObject->Type==2)
						{
							int x,y;
							((CDrawing*)(SelectedDocumentObject->Object))->FindRealCorner(&x,&y);
							corrx+=x;
						}
						if (SelectedDocumentObject2->Type==2)
						{
							int x,y;
							((CDrawing*)(SelectedDocumentObject2->Object))->FindRealCorner(&x,&y);
							corry+=y;
						}
						X+=corrx;
						Y+=corry;
						X=((X+GRID/2)/GRID)*GRID;
						Y=((Y+GRID/2)/GRID)*GRID;
						deltaX=X-SelectedDocumentObject->absolute_X-corrx;
						deltaY=Y-SelectedDocumentObject2->absolute_Y-corry;*/
						if (deltaX>1)
							deltaX=((deltaX+GRID/2)/GRID)*GRID;
						else if (deltaX<-1)
							deltaX=((deltaX-GRID/2)/GRID)*GRID;
						else 
							deltaX=0;
						if (deltaY>1)
							deltaY=((deltaY+GRID/2)/GRID)*GRID;
						else if (deltaY<-1)
							deltaY=((deltaY-GRID/2)/GRID)*GRID;
						else
							deltaY=0;
					}
					int i;
					tDocumentStruct *ds=TheDocument;
					for (i=NumDocumentElements-1;i>=0;i--,ds++)
						if (ds->MovingDotState==3)
						{
							ds->absolute_X+=deltaX;
							ds->absolute_Y+=deltaY;
						}
				}
				else if (SelectedDocumentObject)
				{
					//move this object only
					if (MovingFirstMove) {MovingFirstMove=0;UndoSave("move",20402);}


					int X=SelectedDocumentObject->absolute_X+deltaX;
					int Y=SelectedDocumentObject->absolute_Y+deltaY;

					if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
					{
						X=((X+GRID/2)/GRID)*GRID-3;
						Y=((Y+GRID/2)/GRID)*GRID;
						deltaX=X-SelectedDocumentObject->absolute_X;
						deltaY=Y-SelectedDocumentObject->absolute_Y;
					}
					SelectedDocumentObject->absolute_X=X;

					SelectedDocumentObject->absolute_Y=Y;
					if (GuidlineElement>=0)
					{
						SelectedDocumentObject->absolute_X=prevGuidlineX;
					}
				}
				InvalidateRect(NULL,0);
				//UpdateWindow();

				MovingStartX+=deltaX;
				MovingStartY+=deltaY;
			}
		}
	}


	//take care about special drawings (drawing box, funtion plotter...)

	try 
	{
		tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
		for (int ii=NumDocumentElements-1;ii>=0;ii--,ds--)
		if ((ds->Type==2) &&
			(ds->absolute_Y+ds->Below>ViewY) && 
			(ds->absolute_Y-ds->Above<ViewY+TheClientRect.bottom*100/ViewZoom))
		{
			if ((ds->Object) && (((CDrawing*)(ds->Object))->IsSpecialDrawing))
			{
				int redraw;
				if (ds!=SpecialDrawingHover)
					redraw=((CDrawing*)(ds->Object))->MouseMove(DC,0x7FFFFFFF,0x7FFFFFFF,0);
				else
					redraw=((CDrawing*)(ds->Object))->MouseMove(DC,
														ViewX+point.x*100/ViewZoom-ds->absolute_X,
														ViewY+point.y*100/ViewZoom-ds->absolute_Y,
														nFlags);
				
				if (redraw)	GentlyPaintObject(ds,DC);
			}
		}
	}
	catch (...)
	{

	}
	if (prevSpecialDrawingHover!=SpecialDrawingHover)
	{
		InvalidateRect(NULL,0);
		prevSpecialDrawingHover=SpecialDrawingHover;
	}

	this->ReleaseDC(DC);
	CView::OnMouseMove(nFlags, point);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

#pragma optimize("s",on)
void CMathomirView::OnRButtonDown(UINT nFlags, CPoint point)
{
	IsWindowOutOfFocus=0;
	SteadyCursorTimer=0;
	SpacebarPressedRecently=0;
	try
	{
	if (Tillens.Show)
	{
		Tillens.Show=0;
		RepaintTheView();
		return;
	}
	AutosaveFreetime=0;
	SetMousePointer();

	if ((ListPopup) && (ListPopup->IsWindowVisible()))
	{
		ListPopup->ShowWindow(SW_HIDE);
		return;
	}

	Toolbox->HideSubtoolbox();
	if ((Toolbox) && (Toolbox->ContextMenu)) 
	{
		Toolbox->ContextMenu->ShowWindow(SW_HIDE);
	}
	Popup->HidePopupMenu();
	//check if we pointed at anything
	
	//tDocumentStruct *ds;
	int AbsoluteX=ViewX+(int)point.x*100/(int)ViewZoom;
	int AbsoluteY=ViewY+(int)point.y*100/(int)ViewZoom;

	//right-clicked on streching handle - start rotating

	if (!ViewOnlyMode)
	if ((NumSelectedObjects) && (SelectedHandle) && ((GetKeyState(VK_LBUTTON)&0xFFFE)==0))
	{
		SelectionRectOrig=SelectionRect;
		MovingStartX=AbsoluteX;
		MovingStartY=AbsoluteY;
		MouseMode=11;
		ResizingFirstPass=1;
		goto on_rbuttondown_end;
	}

	if (!ViewOnlyMode)
	if ((ShowRullerCounter==0) && (RullerType==0) && (point.y<=12))
	{
		Popup->ShowPopupMenu(NULL,this,8,0);
		return;
	}

	
	

	if (MouseMode==102) //moving selections (pasted)
	{
		//the pasted objects (attached to the cursor) need to be dropped away.
		tDocumentStruct *ds=TheDocument;
		for (int k=0;k<NumDocumentElements;k++,ds++)
			if (ds->MovingDotState==3) {DeleteDocumentObject(ds);k--;ds--;}
		MouseMode=0;
		RepaintTheView();
	}
	else if ((GetKeyState(VK_LBUTTON)&0xFFFE)==0)
	{
		//clicked on screen
		//IsDrawingMode=0;

		StartAbsoluteX=ViewX+point.x*100/ViewZoom;
		StartAbsoluteY=ViewY+point.y*100/ViewZoom;
		StartViewX=ViewX;
		StartViewY=ViewY;
		PrevMouseX=point.x;
		PrevMouseY=point.y;
		//cliping the alowable area of the view screen
		MinMouseX=ViewX*ViewZoom/100+point.x;
		MinMouseY=ViewY*ViewZoom/100+point.y;
		MouseMode=1; //right click moving - screen scroll
		ScrollModeWasAccepted=0;
		PreopenDrawingToolboxTime=GetTickCount()+260;	
	}


on_rbuttondown_end:
	CView::OnRButtonDown(nFlags, point);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}
#pragma optimize("",on)

int MouseWheelDelta=0;
BOOL CMathomirView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	SelectedTab=0;
	PaintDrawingHotspot(1);
	if (Tillens.Show) {Tillens.Show=0;InvalidateRect(NULL,1);}
	//calculating absolute cursor coordinates before zoom change
	POINT cursor;
	int orgx;
	GetCursorPos(&cursor);
	orgx=cursor.x;
	ScreenToClient(&cursor);
	//if (cursor.x<0) return Toolbox->OnMouseWheel(nFlags,zDelta,pt);

	int AbsX=ViewX+cursor.x*100/ViewZoom;
	int AbsY=ViewY+cursor.y*100/ViewZoom;
	int pzoom=ViewZoom;
	if (PreopenDrawingToolboxX) {PreopenDrawingToolboxX=0;PreopenDrawingToolboxLock=1;}
	MouseWheelDelta+=zDelta;
	int is_scroll=0;
	if ((MouseOverScrollbar) && (!UseCTRLForZoom)) is_scroll=1;
	if ((UseCTRLForZoom) && (!(GetKeyState(VK_CONTROL)&0xFFFE))) is_scroll=1;
	if ((RightButtonTogglesWheel) && (GetKeyState(VK_RBUTTON)&0xFFFE)) is_scroll=(is_scroll)?0:1;

	if ((zDelta) && (abs(MouseWheelDelta)>=10)) //no internal call (internal call -> zDelta==0)
	if (is_scroll)
	{
		MouseWheelAnimation+=MouseWheelDelta*WheelScrollingSpeed;
		MouseWheelDelta=0;
		OnTimer(0xFFFF); //0xFFFF indicates internal call (only for smooth scrolling)
		/*int step=MouseWheelDelta*WheelScrollingSpeed;
		if (MouseWheelDirection)
			ViewY+=step/ViewZoom;
		else
			ViewY-=step/ViewZoom;
		
		MouseWheelDelta=0;
		if ((GetKeyState(VK_RBUTTON)&0xFFFE)==0) MouseOverScrollbar=5;
		if (ViewY<0) ViewY=0;
		SetScrollPos(SB_VERT,ViewY,1);
		InvalidateRect(NULL,0);
		UpdateWindow();*/
		return CView::OnMouseWheel(nFlags, zDelta, pt);
	}

	if (MouseWheelDelta>=120)
	{
		MouseWheelDelta=0;
		//zoom increasing

		if (ViewZoom<=30) ViewZoom=34;
		else if (ViewZoom<=34) ViewZoom=40;
		else if (ViewZoom<=40) ViewZoom=50;
		else if (ViewZoom<=50) ViewZoom=60;
		else if (ViewZoom<=60) ViewZoom=70;
		else if (ViewZoom<=70) ViewZoom=85;
		else if (ViewZoom<=85) ViewZoom=100;
		else if (ViewZoom<=100) ViewZoom=120;
		else if (ViewZoom<=120) ViewZoom=150;
		else if (ViewZoom<=150) ViewZoom=170;
		else if (ViewZoom<=170) ViewZoom=200;
		else if (ViewZoom<=200) ViewZoom=240;
		else if (ViewZoom<=240) ViewZoom=300;
		else if (ViewZoom<=300) ViewZoom=340;
		else if (ViewZoom<=340) ViewZoom=400;
		else if (ViewZoom<=400) ViewZoom=480;
		else if (ViewZoom==480) return CView::OnMouseWheel(nFlags, zDelta, pt);
		if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
	}
	if (MouseWheelDelta<=-120)
	{
		MouseWheelDelta=0;

		if (ViewZoom>=480) ViewZoom=400;
		else if (ViewZoom>=400) ViewZoom=340;
		else if (ViewZoom>=340) ViewZoom=300;
		else if (ViewZoom>=300) ViewZoom=240;
		else if (ViewZoom>=240) ViewZoom=200;
		else if (ViewZoom>=200) ViewZoom=170;
		else if (ViewZoom>=170) ViewZoom=150;
		else if (ViewZoom>=150) ViewZoom=120;
		else if (ViewZoom>=120) ViewZoom=100;
		else if (ViewZoom>=100) ViewZoom=85;
		else if (ViewZoom>=85) ViewZoom=70;
		else if (ViewZoom>=70) ViewZoom=60;
		else if (ViewZoom>=60) ViewZoom=50;
		else if (ViewZoom>=50) ViewZoom=40;
		else if (ViewZoom>=40) ViewZoom=34;
		else if (ViewZoom>=34) ViewZoom=30;
		else if (ViewZoom==30) return CView::OnMouseWheel(nFlags, zDelta, pt);
		if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

	}

	if (MouseWheelDelta==0)
	{

		//calculating new position in absolute coordinates
		int AbsX2=ViewX+cursor.x*100/ViewZoom;
		int AbsY2=ViewY+cursor.y*100/ViewZoom;

		//adjusting not to change absolute position

		ViewX-=AbsX2-AbsX;
		ViewY-=AbsY2-AbsY;
		int px=ViewX;
		int py=ViewY;
		if (ViewX<0) ViewX=0;
		if (ViewY<0) ViewY=0;
		if (ViewX>ViewMaxX) ViewX=ViewMaxX;
		if (ViewY>ViewMaxY) ViewY=ViewMaxY;


		if (((px!=ViewX) || (py!=ViewY)) && (MoveCursorOnWheel))
		{
			RepaintTheView(1);
			GetCursorPos(&cursor);
			SetCursorPos(cursor.x+(px-ViewX)*ViewZoom/100,cursor.y+(py-ViewY)*ViewZoom/100);
		}
		else if ((pzoom<ViewZoom) && (MoveCursorOnWheel))
		{
			//RECT rct;
			//GetClientRect(&rct);

			int dx=-cursor.x+(TheClientRect.right-TheClientRect.left)/2;
			int dy=-cursor.y+(TheClientRect.bottom-TheClientRect.top)/2;;
			dx=dx/12;
			dy=dy/12;
			if (ViewX-dx*100/ViewZoom<0) dx=ViewX*ViewZoom/100;
			if (ViewY-dy*100/ViewZoom<0) dy=ViewY*ViewZoom/100;

			ViewX-=dx*100/ViewZoom;
			ViewY-=dy*100/ViewZoom;
			RepaintTheView(1);
			GetCursorPos(&cursor);
			SetCursorPos(cursor.x+dx,cursor.y+dy);
		}
		else RepaintTheView(1);
	}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CMathomirView::OnZoom40()
{
	ViewZoom=40;
	RepaintTheView(1);

}

void CMathomirView::OnZoom60()
{
	ViewZoom=60;
	RepaintTheView(1);
}

void CMathomirView::SetMousePointer()
{
	Toolbox->HideUnhideColorbox(); //this will hide/uhide color box of the toolbox (depending on drawing mode)
	Toolbox->HideUnhideTextcontrolbox(); //this will hide/uhide text control box
	
	HCURSOR hc;

	if ((MouseOverScrollbar) && (!UseCTRLForZoom))
	{
		MouseOverScrollbar--;
		hc=theApp.LoadCursor(IDC_CURSOR_SCROLL);
	}	
	else if ((IsDrawingMode) && (ShowRullerCounter) && (!MouseTouchingSelection) && (MouseMode!=9) && (MouseMode!=11) && (MouseMode!=2) && (!(GetKeyState(VK_CONTROL)&0xFFFE)))
	{
		if ((IsDrawingMode==1) || (IsDrawingMode==27)) hc=theApp.LoadCursor(IDC_CURSOR_RECTANGLE);
		else if (IsDrawingMode==2) hc=theApp.LoadCursor(IDC_CURSOR_LINE);
		else if (IsDrawingMode==3) hc=theApp.LoadCursor(IDC_CURSOR_FREE);
		else if (IsDrawingMode==5) hc=theApp.LoadCursor(IDC_CURSOR_ARW);
		else if ((IsDrawingMode==4) || (IsDrawingMode==17)) hc=theApp.LoadCursor(IDC_CURSOR_CIRCLE);
		else hc=theApp.LoadCursor(IDC_CURSOR_RECT);
	}
	else if (KeyboardEntryObject)
	{
		hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
	}
	else if ((ClipboardExpression) || (ClipboardDrawing))
	{
		hc=theApp.LoadCursor(IDC_POINTER_COPY);
	}
	else
	{
		hc=::LoadCursor(NULL,IDC_ARROW);
	}

	SetCursor(hc);
}

int CMathomirView::RepaintTheView(int force_recalculate)
{
	//recalculate sizes of elements
	int i;
	CDC *DC=this->GetDC();
	int maxX=PaperWidth,maxY=PaperHeight;
	if (GuidlineElement>=0)
	{
		GuidlineElement=-1;
		PaintGuidlines(DC,-1);
	}
	if (NewlineAddObject>=0)
	{
		NewlineAddObject=-1;
		PaintNewlineAddMark(DC);
	}
	tDocumentStruct *ds=TheDocument;
	if (force_recalculate)
		for (i=0;i<NumDocumentElements;i++,ds++)
		{
			short l,a,b;
			if (ds->Type==1)
			{
				((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
				ds->Length=(short)((int)l*100/(int)ViewZoom);
				ds->Above=(short)((int)a*100/(int)ViewZoom);
				ds->Below=(short)((int)b*100/(int)ViewZoom);
			}
			else if (ds->Type==2)
			{
				((CDrawing*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&b);
				ds->Length=(short)((int)l*100/(int)ViewZoom);
				ds->Above=0;
				ds->Below=(short)((int)b*100/(int)ViewZoom);
			}
			if ((ds->absolute_X+ds->Length+20)>maxX) maxX=ds->absolute_X+ds->Length+20;
			if ((ds->absolute_Y+ds->Below+PaperHeight)>maxY) maxY=ds->absolute_Y+ds->Below+PaperHeight;
			ViewMaxX=maxX;
			ViewMaxY=maxY;
		}
	else
		for (i=0;i<NumDocumentElements;i++,ds++)
		{
			short l,a,b;
			if (ds->Type==1)
			{
				int chksm=((CExpression*)(ds->Object))->CalcChecksum();
				if ((ds->Checksum!=chksm) || (i>NumDocumentElements-50))
				{
					ds->Checksum=chksm;
					((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
					ds->Length=(short)((int)l*100/(int)ViewZoom);
					ds->Above=(short)((int)a*100/(int)ViewZoom);
					ds->Below=(short)((int)b*100/(int)ViewZoom);
				}
			}
			else if (ds->Type==2)
			{
				int chksm=((CDrawing*)(ds->Object))->CalcChecksum();
				if ((ds->Checksum!=chksm) || (i>NumDocumentElements-50))
				{
					ds->Checksum=chksm;
					((CDrawing*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&b);
					ds->Length=(short)((int)l*100/(int)ViewZoom);
					ds->Above=0;
					ds->Below=(short)((int)b*100/(int)ViewZoom);
				}
			}
			if ((ds->absolute_X+ds->Length+20)>maxX) maxX=ds->absolute_X+ds->Length+20;
			if ((ds->absolute_Y+ds->Below+PaperHeight)>maxY) maxY=ds->absolute_Y+ds->Below+PaperHeight;
			ViewMaxX=maxX;
			ViewMaxY=maxY;
		}
	this->ReleaseDC(DC);
	//prevViewZoom=ViewZoom;


	SetScrollRange(SB_HORZ,0,ViewMaxX,0);
	SetScrollRange(SB_VERT,0,ViewMaxY,0);
	SetScrollPos(SB_HORZ,ViewX,1);
	SetScrollPos(SB_VERT,ViewY,1);

	SetMousePointer();
	

	InvalidateRect(NULL,0);
	UpdateWindow();
	return 0;
}

void CMathomirView::OnZoom80()
{
	ViewZoom=80;
	RepaintTheView(1);
}

void CMathomirView::OnZoom100()
{
	ViewZoom=100;
	RepaintTheView(1);
}

void CMathomirView::OnZoom300()
{
	ViewZoom=150;
	RepaintTheView(1);
}

void CMathomirView::OnZoom400()
{
	ViewZoom=240;
	RepaintTheView(1);
}

int SurpressCharMessage=0;
int IsSHIFTALTDown=0;
int IsALTDown=0;
int IsSHIFTDown=0;
//char IsSHIFTJustPressed=0;
#pragma optimize("s",on)
UINT prevChar;
char lockRepeat;
void CMathomirView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (SurpressCharMessage) {SurpressCharMessage=0;return;}
	//initial keyboard testing (determining key codes when the shift key is used)
	//if (keystringmode==2) {keystring3[keystringcnt++]=nChar;return;}


	try
	{
		if ((Toolbox->Subtoolbox->m_IsSubtoolbox==-4) && (Toolbox->Subtoolbox->IsWindowVisible()))
		{
			Toolbox->Subtoolbox->OnChar(nChar,nRepCnt,nFlags);
			return;
		}

		AutosaveFreetime=0;
		UINT Flags=0;
		int CTRLstate=GetKeyState(VK_CONTROL)&0xFFFE;  //the CTRL key
		int SHIFTstate=GetKeyState(16)&0xFFFE; //the SHIFT key
		int CAPSstate=GetKeyState(20)&0x0001; //the CAPS LOCK key
		if (CTRLstate) Flags|=0x01;
		if (SHIFTstate) Flags|=0x02;
		if (CAPSstate) Flags|=0x04;
		int ALTstate=GetKeyState(18)&0xFFFE; //the ALT key


		if ((KeyboardEntryObject==0) && (ALTstate) && (nChar==' '))
		{
			//ALT+space is used to select objects
		}
		else
		{
			if ((prevChar==nChar) && (nChar!=8))
			{
				int i=0;
				while (keystring[i])
				{
					if (keystring[i]==VkKeyScan(nChar)) break;
					i++;
				}
				if (!lockRepeat)
				{
					if (keystring[i]==0)
					{
						SendKeyStroke(10,0,0); //converts the last character to uppercase
						lockRepeat=2;
					}
					else
					{
						SendKeyStroke(8,0,0); //sends backspace
						//simulating the keypress with the shift key pressed (to get the upper character)
						keybd_event(VK_SHIFT,0,0,0);
						keybd_event(keystring[i],0,0,0);
						keybd_event(VK_SHIFT,0,0x02,0);
						lockRepeat=1;
					}
					
				}
			}
			else if (lockRepeat==1)
			{
				if ((!CTRLstate) || (ALTstate)) SendKeyStroke(nChar,0,Flags);
				lockRepeat=2;
			}
			else
			{
				prevChar=nChar;
				lockRepeat=0;
				if ((!CTRLstate) || (ALTstate)) SendKeyStroke(nChar,0,Flags);
			}
		}
		CView::OnChar(nChar, nRepCnt, nFlags);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

char save_equation_image_to_file=0;
char IsSpacebarOnlyHit=0;
char InternalCaps=0;
char RepaintedOnCtrlWhileDrawing=0;
#pragma optimize("s",on)
void CMathomirView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	if ((ListPopup) && (ListPopup->IsWindowVisible()))
	{
		//when a list poup is visible - send up/down/return key to it.
		if ((nChar==VK_DOWN) || (nChar==VK_UP) || (nChar==VK_RETURN))
		{
			if ((nChar==VK_RETURN) && (ListPopup->m_SelectedOption>=0) && (ListPopup->m_SelectedOption<ListPopup->m_NumOptions))
			{
				SurpressCharMessage=1;
			}
			else if (nChar==VK_RETURN)
				ListPopup->ShowWindow(SW_HIDE); //if we only click Enter (without selecting anything from menu)
			ListPopup->OnKeyDown(nChar,nRepCnt,nFlags);
			ListPopup->OnChar(nChar,nRepCnt,nFlags);
			
			Sleep(20);
			this->SetFocus();
			return;
		}
		else
		{
			ListPopup->ShowWindow(SW_HIDE);
			if (nChar==VK_ESCAPE) SurpressCharMessage=1;
		}
		
	}

	SelectedTab=0;
	try{

		if (KeyboardEntryObject)
		{
			//if the typing mode is active, we are going to close subtoolbox when any key is hit and we will 
			//prevend its autoopening
			if ((Toolbox) && (Toolbox->Subtoolbox))
			{
				if (Toolbox->Subtoolbox->IsWindowVisible()) Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
			}
			ToolboxAutoopen=0;
		}

		if ((nChar==VK_CONTROL) && (ToolbarUseCross==0)) PaintDrawingHotspot();

	if (Tillens.Show)
	{
		Tillens.Show=0;
		RepaintTheView();
	}

	int deleted_already=0;
	if (ShowRullerCounter==0) InvalidateRect(NULL,0);
	ShowRullerCounter=5;


	AutosaveFreetime=0;
	if ((nChar==VK_CONTROL) && (IsDrawingMode))
	{
		if (RepaintedOnCtrlWhileDrawing==0)
		{
			InvalidateRect(NULL,0);
			RepaintedOnCtrlWhileDrawing=1;
		}
	}
	else
		RepaintedOnCtrlWhileDrawing=0;
	if ((GetKeyState(VK_MENU)&0xFFFE)==0) IsALTDown=0;
	IsSHIFTALTDown=0;
	IsSHIFTDown=0;
	UINT Flags=0;
	int CTRLstate=GetKeyState(VK_CONTROL)&0xFFFE;  //the CTRL key
	int SHIFTstate=GetKeyState(16)&0xFFFE; //the SHIFT key
	int CAPSstate=GetKeyState(20)&0x0001; //the CAPS LOCK key
	int ALTstate=GetKeyState(18)&0xFFFE; //the ALT key
	if (CTRLstate) Flags|=0x01;
	if (SHIFTstate) {Flags|=0x02;IsSHIFTDown=1;}
	//if (nChar==VK_SHIFT) IsSHIFTJustPressed=1;
	if (CAPSstate) Flags|=0x04;
	if (ALTstate) Flags|=0x08;
	if ((ALTstate) && (CTRLstate) && (nChar!=18)) 
		IsSHIFTALTDown=1; //alternative way for SHIFT+ALT is CTRL+ALT (to supress languague switching)
	int UseCTRLC=1;

	if ((!ALTstate) && (!CTRLstate) && (nChar==' '))
	{
		IsSpacebarOnlyHit=1;
		SpacebarPressedRecently=2;
		POINT cp;
		GetCursorPos(&cp);
		SpacebarPressedPos=cp.x+cp.y*32768;
	}
	else
	{
		IsSpacebarOnlyHit=0;
		if ((SpacebarPressedRecently) && (nChar!=VK_MENU) && (nChar!=VK_SHIFT) && (nChar!=VK_CONTROL)) SpacebarPressedRecently--;
	}

	//check if the Enter key is used in non-typing mode - select/deselect items
	if ((nChar==0x0D) && (ALTstate==0) && (KeyboardEntryObject==NULL))
	{
		SelectLastOrTouchedObject();
		return;
	}

	//THIS small part of code handles the CapsLock key (used for math/text typing mode toggle)
	if ((!SHIFTstate) && (!CTRLstate) && (UseCapsLock))
	{
		if ((InternalCaps!=CAPSstate) && ((nFlags&0x4000)==0))
		{
			keybd_event(VK_CAPITAL,MapVirtualKey(VK_CAPITAL,3),KEYEVENTF_KEYUP,0);
			keybd_event(VK_CAPITAL,MapVirtualKey(VK_CAPITAL,3),0,0);
			keybd_event(VK_CAPITAL,MapVirtualKey(VK_CAPITAL,3),KEYEVENTF_KEYUP,0);
			if (KeyboardEntryObject)
			{
				CDC *DC=this->GetDC();
				int tmp=IsALTDown;IsALTDown=1;
				((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,' ',0,0,0,1);
				IsALTDown=tmp;
				this->ReleaseDC(DC);
			}
			return ;
		}
	}
	else
		InternalCaps=CAPSstate;
	if ((UseCapsLock) && (nChar!=VK_CAPITAL) && (CAPSstate))
		DisplayShortText("SHIFT+CAPS toggles the Caps Lock!",1,1,7009,1);


	if (GuidlineElement>=0)
	{
		if ((CTRLstate) && (SnapToGuidlines))
		{
			GuidlineElement=-1;
			CDC *DC=GetDC();
			PaintGuidlines(DC,-1);
			ReleaseDC(DC);
		}
	} 


#ifdef TEACHER_VERSION
	if ((TheFileType=='r') && (DisableEditing))	goto Keydown_continue;
#endif


	if (!ViewOnlyMode)
	{
		if (nChar!=' ')
		if ((nChar==VK_INSERT) ||
			((MapVirtualKey(nChar,2)=='.') && (SpacebarPressedRecently) && (GetKeyState(VK_SPACE)&0xFFFE)))
		{
			//inserting from internal stack-clipboard
			if ((prevClipboardExpression) && (KeyboardEntryObject) && (KeyboardEntryBaseObject))
			{
				if (ClipboardExpression) delete ClipboardExpression;
				ClipboardExpression=new CExpression(NULL,NULL,100);
				ClipboardExpression->CopyExpression(prevClipboardExpression,0,0,0);
				short l,a,b;
				CDC *DC=this->GetDC();
				ClipboardExpression->CalculateSize(DC,ViewZoom,&l,&a,&b);
				this->ReleaseDC(DC);
				if (KeyboardEntryObject)
				{
					this->SendKeyStroke(6,0,0);
					/*if (prevClipboardExpression[1])
					{
						if (!ListPopup)
						{
							ListPopup=new PopupMenu();
							ListPopup->CreateEx(WS_EX_TOPMOST,AfxRegisterWndClass(CS_OWNDC),"MoM list",WS_CLIPCHILDREN | WS_POPUP,5,5,10,10,theApp.m_pMainWnd->m_hWnd,NULL,0);
						}
						ListPopup->m_SelectedOption=-1;
						::SetWindowLong(ListPopup->m_hWnd,GWL_EXSTYLE,GetWindowLong(ListPopup->m_hWnd,GWL_EXSTYLE)|WS_EX_LAYERED);
						::SetLayeredWindowAttributes(ListPopup->m_hWnd,0,96,LWA_ALPHA);
						ListPopup->ShowPopupMenu(NULL,this,10,0);
						
						//return keyboard focus back to the main window
						this->SetActiveWindow();
						this->SetFocus();
					}*/
				}
				else
				{
					this->SetMousePointer();
					InvalidateRect(NULL,0);
					UpdateWindow();
				}
			}
			if (nChar!=VK_INSERT) SurpressCharMessage=1;
			return;
		}

		if ((nChar=='S') && (CTRLstate) && (!SHIFTstate) && (!ALTstate))
		{
			//CTRL+S is used for saving
			DisplayShortText("Saving...",10,10,7000);
			this->GetDocument()->OnFileSave();
			return;		
		}

		/*if ((nChar==VK_F11) && (!CTRLstate))
		{
			if (SnapToGrid) 
			{SnapToGrid=0;DisplayShortText("Snap to grid OFF",10,10,7001);}
			else 
			{SnapToGrid=1;DisplayShortText("Snap to grid ON",10,10,7002);}
			AdjustMenu();
		}*/
		if ((nChar==VK_F12) && (!CTRLstate))
		{
			if (SnapToGuidlines) 
			{SnapToGuidlines=0;DisplayShortText("Guidelines OFF",10,10,7003);}
			else 
			{SnapToGuidlines=1;DisplayShortText("Guidelines ON",10,10,7004);}
			AdjustMenu();
		}
	}

	if ((KeyboardEntryObject==NULL) || //handling non keyboard-entry mode (plus some exceptions when there are selected object at the same time while typing mode is active)
		((nChar==VK_DELETE) && (NumSelectedObjects) && (IsSelectionFresh)) ||
		(((Flags&0x01)==1) && (nChar=='X') && ((nFlags&0x100)==0) && (NumSelectedObjects) && (IsSelectionFresh)) ||
		(((Flags&0x01)==1) && (nChar=='C') && ((nFlags&0x100)==0) && (NumSelectedObjects) && (IsSelectionFresh))) 
	{
		if ((nChar==' ') && ((GetKeyState(VK_MENU)&0xFFFE)==0))
		{
			int X,Y;
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(&p);
			X=p.x*100/ViewZoom+ViewX;
			Y=p.y*100/ViewZoom+ViewY;
			int NumTouchedDrawings=0;
			tDocumentStruct *TouchedDrawing;
			tDocumentStruct *ds=TheDocument;

			for (int i=0;i<NumDocumentElements;i++,ds++)
				if ((ds->absolute_X<=X) && (ds->absolute_X+ds->Length>=X) && 
					(ds->absolute_Y-ds->Above<=Y) && (ds->absolute_Y+ds->Below>=Y) &&
					(ds->MovingDotState!=5))
				{
					if (ds->Type==1) {NumTouchedDrawings=10;break;}
					if (ds->Type==2)
					{
						CDrawing *d=(CDrawing*)ds->Object;
						int nodeedit;
						CDC *DC=GetDC();
						int rx=p.x-(ds->absolute_X-ViewX)*ViewZoom/100;
						int ry=p.y-(ds->absolute_Y-ViewY)*ViewZoom/100;

						if (d->SelectObjectAtPoint(DC,ViewZoom,rx,ry,&nodeedit))
						{
							NumTouchedDrawings++;
							TouchedDrawing=ds;
						}
						ReleaseDC(DC);
					}
				}
			if (NumTouchedDrawings==1) 
			{
				if (/*(((CDrawing*)TouchedDrawing->Object)->IsNodeEdit) ||*/ (GetKeyState(VK_CONTROL)&0xFFFE) || (ToolbarEditNodes))
				if (((CDrawing*)TouchedDrawing->Object)->NodeX==-1)
				{
					//we are adding new node to the hand drawing (space bar is pressed when mouse pointer is above the
					//drawing with nodes visible)
					((CDrawing*)TouchedDrawing->Object)->SplitLineAtPos(X-TouchedDrawing->absolute_X,Y-TouchedDrawing->absolute_Y);
					((CDrawing*)TouchedDrawing->Object)->NodeX=(X-TouchedDrawing->absolute_X)*DRWZOOM;
					((CDrawing*)TouchedDrawing->Object)->NodeY=(Y-TouchedDrawing->absolute_Y)*DRWZOOM;					
					InvalidateRect(NULL,0);
					UpdateWindow();

					return;
				}
			}
		}

		
		if (((nFlags&0x100)==0) && (nChar==VK_F5) && (!CTRLstate)) OnHqRend();
		if (((nFlags&0x100)==0) && (nChar==VK_F4) && (!CTRLstate) && (!ViewOnlyMode)) 
		{
			if (IsDrawingMode) 
			{
				IsDrawingMode++;
				prevDrawingMode=-1;
				if (IsDrawingMode>4) IsDrawingMode=1;
			}
			else
			{
				IsDrawingMode=prevDrawingMode2;
				prevDrawingMode=-1;
				if (IsDrawingMode<=0) IsDrawingMode=2;
			}
		
			Toolbox->Subtoolbox->m_IsSubtoolbox=-4; //special hand-drawing toolbox
			Toolbox->Subtoolbox->AdjustPosition();
			
			Toolbox->Subtoolbox->InvalidateRect(NULL,0);
			Toolbox->Subtoolbox->UpdateWindow();
			NoImageAutogeneration=2;
			Toolbox->Subtoolbox->ShowWindow(SW_SHOWNA);
			SetMousePointer();
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(&point);
			OnMouseMove(0,point);
		}

		if (((nFlags&0x100)==0) && (nChar==VK_F2) && (!CTRLstate)) OnViewZoomin();
		if (((nFlags&0x100)==0) && (nChar==VK_F3) && (!CTRLstate)) OnViewZoomout();
		if (((nFlags&0x100)==0) && (nChar==VK_F1) && (!CTRLstate)) (F1SetsZoom)?OnViewZoomto1():OnHelpQuickguide();

		//this part of code checks 'live keyboard', that is keyboard commands 
		//when no keyboard entry mode is active (CTRL+X, CTRL+C, CTRL+V, DEL)
		if (((nFlags&0x100)==0) && (nChar==27)) //the ESC key
		{
			if (MouseMode==102)
			{
				tDocumentStruct *ds=TheDocument;
		
				for (int i=0;i<NumDocumentElements;i++,ds++)
					if (ds->MovingDotState==3) {DeleteDocumentObject(ds);i--;ds--;}
				MouseMode=0;
			}
			Toolbox->HideSubtoolbox();
			Popup->HidePopupMenu();
			if (ClipboardExpression)
			{
				//ClipboardExpression->Delete();
				delete ClipboardExpression;
				ClipboardExpression=NULL;
				SetCursor(::LoadCursor(NULL,IDC_ARROW));
				RepaintTheView();
			} 
			else if (ClipboardDrawing)
			{
				//ClipboardDrawing->Delete();
				delete ClipboardDrawing;
				ClipboardDrawing=NULL;
				SetCursor(::LoadCursor(NULL,IDC_ARROW));
				RepaintTheView();
			}
			else
			{
				int i;
				int is_deselected=0;
				for (i=0;i<NumDocumentElements;i++)
				{
					tDocumentStruct *ds=TheDocument+i;
					if (ds->MovingDotState==3) 
					{
						if (ds->Type==1) ((CExpression*)(ds->Object))->DeselectExpression();
						else if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(0);
						is_deselected=1;
					}
					if (ds->MovingDotState!=5) ds->MovingDotState=0;
				}
				if (is_deselected==0) IsDrawingMode=0;
				RepaintTheView();
			}

		}

		if ( (((nFlags&0x100)==0) && (nChar==VK_F9) && (!CTRLstate)) ||  //F9 (image copy)
			 (((nFlags&0x100)==0) && (nChar==VK_F8) && (!CTRLstate)) || //F8 (image save to file)
			 //(((nFlags&0x100)==0) && (nChar==VK_F7) && (!CTRLstate)) || //F7 (copy MathML code)
			 (((nFlags&0x100)==0) && (nChar==VK_F6) && (!CTRLstate)) || //F6 (copy LaTeX code)

			 //TODO - sprijeèiti da se ovo može sa CTRL ili SHIFT ili ALT?
			 //- što s drawing modom - kako onda postupiti
			 //(((nFlags&0x100)==0) && (nChar==' ')) || //selecting objects by spacebar
			 //(((nFlags&0x100)==0) && (nChar>='1') && (nChar<='6')) || //changing drawing line size
			 //(((nFlags&0x100)==0) && ((nChar=='K') || (nChar=='R') || (nChar=='G') || (nChar=='B') || (nChar=='Y'))) || //changing line color
			 ((((Flags&0x09)==0) || (IsDrawingMode)) && (nChar==VK_DELETE) && (nFlags&0x100) && (!ViewOnlyMode)) ||   //delete
			 (((Flags&0x01)==1) && (nChar=='X') && ((nFlags&0x100)==0) && (!ViewOnlyMode)) ||    //ctrl-x (cut)
			 (((Flags&0x01)==1) && (nChar=='C') && ((nFlags&0x100)==0)) )     //ctrl-c (copy)
		{
			//first check if there is any multiple seelection 
			int i;
			if (nChar!=' ')
			for (i=0;i<NumDocumentElements;i++)
			{
				if (TheDocument[i].MovingDotState==3)
				{
					if (nChar=='X') {OnEditCut();MouseTouchingSelection=0;this->SetMousePointer();}
					if (nChar=='C') {OnEditCopy();UseCTRLC=0;}
					if (nChar==VK_DELETE) {OnEditDelete();deleted_already=1;MouseTouchingSelection=0;this->SetMousePointer();}
					if (nChar==VK_F9) OnEditCopyImage();
					if (nChar==VK_F8) OnEditSaveequationimage();
					//if (nChar==VK_F7) OnEditCopymathmlcode();
					if (nChar==VK_F6) OnEditCopylatexcode();
					
					goto Keydown_continue;
				}
			}
			//find the object, the mouse is pointing at

			if ((nChar=='X') || (nChar=='C'))
			{
				if (ClipboardExpression)
				{
					//ClipboardExpression->Delete();
					delete ClipboardExpression;
					ClipboardExpression=NULL;
				}
			}

			POINT cursor;
			GetCursorPos(&cursor);
			ScreenToClient(&cursor);
			int absX=ViewX+cursor.x*100/ViewZoom;
			int absY=ViewY+cursor.y*100/ViewZoom;
			for (i=NumDocumentElements-1;i>=0;i--)
				if ((absX>=TheDocument[i].absolute_X) && (absX<=TheDocument[i].absolute_X+TheDocument[i].Length) &&
					(absY>=TheDocument[i].absolute_Y-TheDocument[i].Above) && (absY<=TheDocument[i].absolute_Y+TheDocument[i].Below) &&
					((TheDocument[i].MovingDotState!=5) || (AccessLockedObjects)))
				{
					//found an object document;
					char root_object_flag=0;
					RootObjectCopy=0;
					CExpression *expr=NULL;
					CDrawing *drw=NULL;
					if (TheDocument[i].Type==1)
					{
						//if ((nChar>='1') && (nChar<='6')) continue;
						//if ((nChar=='K') || (nChar=='R') || (nChar=='G') || (nChar=='B') || (nChar=='Y')) continue;
						expr =((CExpression*)(TheDocument[i].Object))->AdjustSelection();
						if ((nChar==VK_DELETE) && (expr) && (expr->DecodeInternalInsertionPoint())) continue; 
						if (nChar!=' ')
						if (expr==(CExpression*)TheDocument[i].Object)
						{
							//check if there is more than one selected object, if yes
							//then we are working with groups
							int x1,y1;
							CDrawing *tmp=(CDrawing*)ComposeDrawing(&x1,&y1,0,0);
							if (tmp)
							{
								if (tmp->NumItems>1)
								{
									expr=NULL;
									drw=(CDrawing*)1;
								}
								else
									root_object_flag=1;
								//tmp->Delete();
								delete tmp;
							}
						}
					}
					if (TheDocument[i].Type==2)
					{
						if ((nChar==' ') && (TheDocument[i].MovingDotState==3))
							drw=(CDrawing*)TheDocument[i].Object;
						else
						{
							int NodeEdit;
							int X=cursor.x-(TheDocument[i].absolute_X-ViewX)*ViewZoom/100;
							int Y=cursor.y-(TheDocument[i].absolute_Y-ViewY)*ViewZoom/100;
							CDC *DC=this->GetDC();
							drw = (CDrawing*)(((CDrawing*)(TheDocument[i].Object))->SelectObjectAtPoint(DC,ViewZoom,X,Y,&NodeEdit));
							if (drw) drw=((CDrawing*)(TheDocument[i].Object));
							this->ReleaseDC(DC);
							if (drw==NULL) continue;
						}
					}
					if (drw)
					{
						if ((nChar=='X') || (nChar=='C') || (nChar==VK_F9) || (nChar==VK_F8))
						{
							if ((ClipboardDrawing==NULL) || (nChar==VK_F9) || (nChar==VK_F8))
							{
								int x1,y1;
								if ((nChar==VK_F9) || (nChar==VK_F8))
								{
									CDrawing *image_drw;
									image_drw=(CDrawing*)ComposeDrawing(&x1,&y1,0,0);
									if (image_drw) 
									{
										if (nChar==VK_F8) save_equation_image_to_file=1;
										MakeImageOfDrawing((CObject*)image_drw);
										save_equation_image_to_file=0;
										//image_drw->Delete();
										delete image_drw;
									}
								}
								else 
								{
									ClipboardDrawing=(CDrawing*)ComposeDrawing(&x1,&y1,0,1);
									if (ClipboardDrawing)
										ClipboardDrawing->CopyToWindowsClipboard();
									MovingStartX2=ViewX+cursor.x*100/ViewZoom;
									MovingStartY2=ViewY+cursor.y*100/ViewZoom;
									MovingStartX=MovingStartX2-x1;
									MovingStartY=MovingStartY2-y1;
									moving_start_timer=GetTickCount();							
									SetCursor(theApp.LoadCursor(IDC_POINTER_COPY));
								}
							}
							UseCTRLC=0;
						}
						if ((nChar==VK_DELETE) || (nChar=='X'))
						{
							(nChar=='X')?UndoSave("pick up",20202):UndoSave("delete",20203);
							for (int kk=0;kk<NumDocumentElements;kk++)
							{
								tDocumentStruct *ds2=TheDocument+kk;
								if ((ds2->Type==2) && (ds2->Object) && (((CDrawing*)(ds2->Object))->IsSelected))
								{
									((CDrawing*)(ds2->Object))->Delete();
									DeleteDocumentObject(ds2);
									kk--;
								}
								if ((ds2->Type==1) && (ds2->Object) && ((ds2->MovingDotState==4) || (((CExpression*)ds2->Object)->m_Selection==0x7FFF))) //kubura
								{
									DeleteDocumentObject(ds2);
									kk--;
								}
							}
						}
						/*if (nChar==' ')
						{
							DontUseSpacebar=1;
							if (TheDocument[i].MovingDotState==3)
								TheDocument[i].MovingDotState=0;
							else
							for (int j=0;j<NumDocumentElements;j++)
								if (TheDocument[j].Type==2)
								{
									CDrawing *d=(CDrawing*)TheDocument[j].Object;
									if (d->IsSelected) TheDocument[j].MovingDotState=3;
								}							
						}
						if ((nChar>='1') && (nChar<='6'))
						{
							for (int j=0;j<NumDocumentElements;j++)
								if (TheDocument[j].Type==2)
								{
									CDrawing *d=(CDrawing*)TheDocument[j].Object;
									if (d->IsSelected) 
									{
										int w=650;
										if (nChar=='2') w=1000;
										if (nChar=='3') w=1500;
										if (nChar=='4') w=2000;
										if (nChar=='5') w=3000;
										if (nChar=='6') w=4000;
										d->SetLineWidth(w);
									}
								}							
						}
						if ((nChar=='K') || (nChar=='R') || (nChar=='G') || (nChar=='B') || (nChar=='Y'))
						{
							for (int j=0;j<NumDocumentElements;j++)
								if (TheDocument[j].Type==2)
								{
									CDrawing *d=(CDrawing*)TheDocument[j].Object;
									if (d->IsSelected) 
									{
										int c=0;
										if (nChar=='R') c=1;
										if (nChar=='G') c=2;
										if (nChar=='B') c=3;
										if (nChar=='Y') c=4;
										d->SetColor(c);
									}
								}	
						}*/
					}
					if (expr)
					{
						//found some selections inside pointed object, delete these selections
						if ((nChar=='X') || (nChar=='C'))
						{
							if (ClipboardExpression==NULL)
								ClipboardExpression=new CExpression(NULL,NULL,100);
							ClipboardExpression->CopyExpression(expr,1);
							if (root_object_flag)
							{
								ClipboardExpression->m_InternalInsertionPoint=0;
								RootObjectCopy=ClipboardExpression->CalcChecksum();
								RootObjectFontSize=expr->m_FontSize;
								//RootObjectFontSizeHQ=expr->m_FontSizeHQ;
							}
							UseCTRLC=0;
						}
						if ((nChar==VK_F9) || (nChar==VK_F8) || (nChar==VK_F7) || (nChar==VK_F6))
						{
							CExpression *image_expr=new CExpression(NULL,NULL,DefaultFontSize);
							image_expr->CopyExpression(expr,1);
							image_expr->m_FontSize=expr->m_FontSize;
							//image_expr->m_FontSizeHQ=expr->m_FontSizeHQ;
							/*if (nChar==VK_F7) 
								CopyMathMLCode((CObject*)image_expr);
							else */
							if (nChar==VK_F6)
								CopyLaTeXCode((CObject*)image_expr);
							else
							{
								if (nChar==VK_F8) save_equation_image_to_file=1;
								MakeImageOfExpression((CObject*)image_expr);
								save_equation_image_to_file=0;
							}
							//image_expr->Delete();
							delete image_expr;
						}
						if ((nChar==VK_DELETE) || (nChar=='X'))
						{
							deleted_already=1;
							(nChar=='X')?UndoSave("pick up",20202):UndoSave("delete",20203);
							CExpression *parent=expr->m_pPaternalExpression;
							if (expr->DeleteSelection()==2) expr=parent;
							if (expr->m_NumElements==0) expr->InsertEmptyElement(0,0,0);
						}

						if (ClipboardExpression) ClipboardExpression->CopyToWindowsClipboard();

						if ((nChar!='C') && (expr))
						{
							
							//check if the expression is left copletely empty, if yes, delete it
							expr=(CExpression*)(TheDocument[i].Object);
							if (expr->m_pPaternalExpression==NULL)
							if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
							{
								DeleteDocumentObject(&TheDocument[i]);
							}	
						}
						/*if (nChar==' ')
						{
							TheDocument[i].MovingDotState=(TheDocument[i].MovingDotState==3)?0:3;
							DontUseSpacebar=1;
						}*/
					}
					break;
				}
			RepaintTheView();
		}

		//CTRL+Z (Undo) - handling
		if ((((Flags&0x01)==1) && (nChar=='Z') && ((nFlags&0x100)==0)))
		{
			OnEditUndo();
		}
		//CTRL+V (paste) - handling
		if ((((Flags&0x01)==1) && (nChar=='V') && ((nFlags&0x100)==0)) && (!ClipboardExpression))
		{
			OnEditPaste();
			// p;
			//GetCursorPos(&p);
			//SetCursorPos(p.x+20,p.y+20);
		}
		if (!ViewOnlyMode)
		if ((((Flags&0x01)==1) && (nChar=='V') && ((nFlags&0x100)==0)) && ((ClipboardExpression) || (ClipboardDrawing)))     //ctrl-V (paste)
		{
			//find the object, the mouse is pointing at
			CDC *DC=this->GetDC();
			POINT cursor;
			GetCursorPos(&cursor);
			ScreenToClient(&cursor);
			
			if ((cursor.x<TheClientRect.left) || (cursor.x>TheClientRect.right) || (cursor.y<TheClientRect.top) || (cursor.y>TheClientRect.bottom))
				return;

			int absX=ViewX+cursor.x*100/ViewZoom;
			int absY=ViewY+cursor.y*100/ViewZoom;
			int i;
			for (i=NumDocumentElements-1;i>=0;i--)
				if ((TheDocument[i].Type==1) && 
					(absX>TheDocument[i].absolute_X) && 
					(absX<TheDocument[i].absolute_X+TheDocument[i].Length) &&
					(absY>TheDocument[i].absolute_Y-TheDocument[i].Above) && 
					(absY<TheDocument[i].absolute_Y+TheDocument[i].Below) &&
					((TheDocument[i].MovingDotState!=5)))
				{
					//found an object document;

					int X=(absX-TheDocument[i].absolute_X)*ViewZoom/100;
					int Y=(absY-TheDocument[i].absolute_Y)*ViewZoom/100;
					UndoSave("insert into",20303);
					((CExpression*)(TheDocument[i].Object))->CopyAtPoint(DC,ViewZoom,X,Y,ClipboardExpression);

					break;
				}
			if (i==-1)
			{
				//no object has been found the mouse is pointing at - we will create new object

				UndoSave("insert new",20401);

				if (ClipboardExpression)
				{
					int adjust_for_text=0;
					if ((ClipboardExpression->m_NumElements==1) && (ClipboardExpression->m_pElementList->Type==5) &&
						(((CExpression*)(ClipboardExpression->m_pElementList->pElementObject->Expression1))->m_StartAsText) &&
						(((CExpression*)(ClipboardExpression->m_pElementList->pElementObject->Expression1))->m_pElementList->Type==0))
					{
						adjust_for_text=1;
					}

					{
						//NumDocumentElements++;
						//CheckDocumentMemoryReservations();
						if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
						{
							absX=((absX+GRID/2)/GRID)*GRID-3;
							absY=((absY+GRID/2)/GRID)*GRID;
						}
						AddDocumentObject(1,absX,absY);
						//TheDocument[NumDocumentElements-1].absolute_X=absX;
						//TheDocument[NumDocumentElements-1].absolute_Y=absY;
						short l,a=0,b;
						//TheDocument[NumDocumentElements-1].Type=1;
						TheDocument[NumDocumentElements-1].Object=(CObject*)(new CExpression(NULL,NULL,DefaultFontSize));
						if (RootObjectCopy==ClipboardExpression->CalcChecksum())
						{
							((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_FontSize=RootObjectFontSize;
							//((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_FontSizeHQ=RootObjectFontSizeHQ;
						}
						if (adjust_for_text) 
							{((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_StartAsText=1;((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_Alignment=1;}
						else
							((CExpression*)(TheDocument[NumDocumentElements-1].Object))->CopyExpression(ClipboardExpression,0);

						((CExpression*)(TheDocument[NumDocumentElements-1].Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);			
						TheDocument[NumDocumentElements-1].Length=l*100/ViewZoom;
						TheDocument[NumDocumentElements-1].Above=a*100/ViewZoom;
						TheDocument[NumDocumentElements-1].Below=b*100/ViewZoom;
						TheDocument[NumDocumentElements-1].MovingDotState=0;
						tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
						if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
						if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;
					}
				}
				else if (ClipboardDrawing)
				{
					PasteDrawing(DC,cursor.x,cursor.y);
				}
			}
			this->ReleaseDC(DC);
			RepaintTheView();
		}
	}
	else
	{
		if (((nFlags&0x100)==0) && (nChar==VK_F5) && (!CTRLstate)) {OnHqRend();goto Keydown_exit;}
		if (((nFlags&0x100)==0) && (nChar==VK_F4) && (!CTRLstate) && (!ViewOnlyMode)) 
		{
			if (IsDrawingMode) 
			{
				IsDrawingMode++;
				prevDrawingMode=-1;
				if (IsDrawingMode>4) IsDrawingMode=1;
			}
			else
			{
				IsDrawingMode=prevDrawingMode2;
				prevDrawingMode=-1;
				if (IsDrawingMode<=0) IsDrawingMode=2;
			}
			Toolbox->Subtoolbox->m_IsSubtoolbox=-4; //special hand-drawing toolbox
			Toolbox->Subtoolbox->AdjustPosition();
			Toolbox->Subtoolbox->InvalidateRect(NULL,0);
			Toolbox->Subtoolbox->UpdateWindow();
			NoImageAutogeneration=2;
			Toolbox->Subtoolbox->ShowWindow(SW_SHOWNA);
			SetMousePointer();
			POINT point;
			GetCursorPos(&point);
			ScreenToClient(&point);
			OnMouseMove(0,point);
		}
		if (((nFlags&0x100)==0) && (nChar==VK_F2) && (!CTRLstate)) {OnViewZoomin();goto Keydown_exit;}
		if (((nFlags&0x100)==0) && (nChar==VK_F3) && (!CTRLstate)) {OnViewZoomout();goto Keydown_exit;}
		if (((nFlags&0x100)==0) && (nChar==VK_F1) && (!CTRLstate)) {(F1SetsZoom)?OnViewZoomto1():OnHelpQuickguide();goto Keydown_exit;}
		if (KeyboardEntryBaseObject)
		{
			//if (((nFlags&0x100)==0) && (nChar==VK_F7) && (!CTRLstate)) CopyMathMLCode(KeyboardEntryBaseObject->Object);
			if (((nFlags&0x100)==0) && (nChar==VK_F6) && (!CTRLstate)) CopyLaTeXCode(KeyboardEntryBaseObject->Object);
			if (((nFlags&0x100)==0) && (nChar==VK_F8) && (!CTRLstate)) 
			{
				save_equation_image_to_file=1;
				MakeImageOfExpression((CObject*)KeyboardEntryBaseObject->Object);
				save_equation_image_to_file=0;
			}
			if (((nFlags&0x100)==0) && (nChar==VK_F9) && (!CTRLstate)) 
			{
				MakeImageOfExpression((CObject*)KeyboardEntryBaseObject->Object);
			}
		}

		//CTRL+Z (Undo) - handling (for keyboard-entry mode)
		if ((((Flags&0x03)==1) && (nChar=='Z') && ((nFlags&0x100)==0)))
		{
			OnEditUndo();
			goto Keydown_exit;
		}

		//CTRL+X and CTRL+C when keyboard entry mode is active
		if (((Flags&0x03)==1) && (nChar=='X') && ((nFlags&0x100)==0) && ((nFlags&0x2000)==0))    //ctrl-x (cut)
		{
			if (KeyboardEntryObject)
			{
				CExpression *e=(CExpression*)KeyboardEntryObject;
				int i=0;
				for (;i<e->m_NumElements;i++)
					if ((e->m_pElementList+i)->IsSelected==2) break;
				if (i>=e->m_NumElements)
				{
					if (ClipboardExpression) delete ClipboardExpression;
					ClipboardExpression=new CExpression(NULL,NULL,100);
					ClipboardExpression->InsertEmptyElement(0,2,(char)0xB4);
					CDC *DC=this->GetDC();
					short l,a,b;
					ClipboardExpression->CalculateSize(DC,ViewZoom,&l,&a,&b);
					e->KeyboardKeyHit(DC,ViewZoom,6,0,0,0,0);
					this->ReleaseDC(DC);
					return;
				}
			}
			OnEditCut();
			//UndoSave("cut");
			//KeyboardSelectionCut();
		}
		if (((Flags&0x03)==1) && (nChar=='C') && ((nFlags&0x100)==0) && ((nFlags&0x2000)==0))     //ctrl-c (copy)
			OnEditCopy();
			//KeyboardSelectionCopy();

		//CTRL+V when keyboard entry mode is active
		if (((Flags&0x03)==1) && (nChar=='V') && ((nFlags&0x100)==0) && ((nFlags&0x2000)==0))
		{
			//UndoSave("paste");
			//KeyboardSelectionPaste();
			OnEditPaste();
		}

	}

Keydown_continue:

	if ((CTRLstate) && (nChar!=17)) //if some key pressed together with the CTRL key
	{
		if ((nChar=='C') && (!UseCTRLC)) return;
		//call the toolbox (for accelerator table)
		Toolbox->KeyboardHit(nChar+(nFlags&0x100),Flags);
		CView::OnKeyDown(nChar,nRepCnt,nFlags);
		return;
	}

	if (nFlags&0x100)
	{
		//extended keys
		if (nChar==VK_HOME) SendKeyStroke(14,0,Flags);
		if (nChar==VK_END) SendKeyStroke(15,0,Flags);
		if ((nChar==VK_DELETE) && (!deleted_already)) SendKeyStroke(7,0,Flags);
		if (nChar==VK_RIGHT) SendKeyStroke(5,0,Flags);
		if (nChar==VK_LEFT) SendKeyStroke(4,0,Flags);
		if (nChar==VK_UP) SendKeyStroke(3,0,Flags);
		if (nChar==VK_DOWN) SendKeyStroke(2,0,Flags);
		//if (!KeyboardEntryObject)
		{
			if (nChar==33) SendKeyStroke(11,0,Flags); //page up
			if (nChar==34) SendKeyStroke(12,0,Flags); //page down
		}

	}
	else
	{
		//regular keys
		if (nChar==VK_SHIFT) SendKeyStroke(0,0,Flags); //SHIFT
		if (nChar==20) SendKeyStroke(0,0,Flags); //CAPS_LOCK
	}
Keydown_exit:
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

int CMathomirView::ScrollCursorIntoView()
{
	//scrolls the document area so that the keyboard cursor can be seen

	int RepaintAll=0;

	if ((KeyboardEntryObject) && (KeyboardEntryBaseObject) && 
		(KeyboardEntryBaseObject-TheDocument>=0) && (KeyboardEntryBaseObject-TheDocument<NumDocumentElements) &&
		(KeyboardEntryBaseObject->Type==1))
	{
		int X,Y;
		if (((CExpression*)(KeyboardEntryBaseObject->Object))->GetKeyboardCursorPos(&X,&Y))
		{
			//automaticly scroll the window if cursor is outside visible area
			X=X*100/ViewZoom;
			Y=Y*100/ViewZoom;
			X+=KeyboardEntryBaseObject->absolute_X;
			Y+=KeyboardEntryBaseObject->absolute_Y;
			
			RepaintAll=1;

			int tmp=(((CExpression*)KeyboardEntryObject)->m_FontSize/8);//*ViewZoom/100;

			int bottom=KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Below;
			if (bottom>Y+6*tmp) bottom=Y+6*tmp;
			if (bottom>ViewY+(TheClientRect.bottom)*100/ViewZoom-tmp/2) //we want to show the entire object that is edited
			{this->SetScrollPos(SB_VERT,bottom-(TheClientRect.bottom)*100/ViewZoom+tmp/2,0);OnVScroll(SB_THUMBPOSITION,0,NULL);RepaintAll=0;}

	
			if (X>ViewX+(TheClientRect.right)*100/ViewZoom-tmp) 
				{OnHScroll(SB_THUMBPOSITION,X-(TheClientRect.right)*100/ViewZoom+tmp,NULL);RepaintAll=0;}
			if (X<ViewX+(TheClientRect.left)*100/ViewZoom-tmp) 
				{OnHScroll(SB_THUMBPOSITION,X-(TheClientRect.left)*100/ViewZoom-tmp,NULL);RepaintAll=0;}	
			if (Y>ViewY+(TheClientRect.bottom)*100/ViewZoom-tmp)
				{this->SetScrollPos(SB_VERT,Y-(TheClientRect.bottom)*100/ViewZoom+tmp,0);OnVScroll(SB_THUMBPOSITION,0,NULL);RepaintAll=0;}
			if (Y<ViewY+(TheClientRect.top)*100/ViewZoom+tmp) 
				{this->SetScrollPos(SB_VERT,Y-(TheClientRect.top)*100/ViewZoom-tmp,0);OnVScroll(SB_THUMBPOSITION,0,NULL);RepaintAll=0;}
				
		}
		else
			return -1; //some error
	}

	return RepaintAll;
}

int prevAutocompleteTriggered;
CExpression *prevAutocompleteSource;
CExpression *LastEditedExp2;
char KeystrokesList[16];
int KeystrokeNumElements[16];
int KeystrokesListLen;
#pragma optimize("s",on)
void CMathomirView::SendKeyStroke(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CExpression *clipboard=ClipboardExpression; //will remember if the object is inserted (if yes will scroll the screen later on, if needed)
	tDocumentStruct *prevBaseObject=KeyboardEntryBaseObject;
	tDocumentStruct *prevDocument=TheDocument;
	int fcolor=Toolbox->GetFormattingColor();
	nFlags=Toolbox->KeyboardHit(nChar,nFlags);
	if (nChar==0) return;

	

#ifdef TEACHER_VERSION
	if ((TheFileType=='r') && (DisableEditing))
		if ((nChar!=6) && (nChar!=11) && (nChar!=12)) nChar=27;
#endif

	
	CDC *DC=this->GetDC();



	if ((KeyboardEntryObject) && (nChar!=11) && (nChar!=12)) //if keyboard entry mode and not page up / page down was pressed
	{
		//check validity of important pointers
		try
		{
			if (KeyboardEntryBaseObject->Object==0) return;
			if (KeyboardEntryBaseObject->Type<0) return;
			if (KeyboardEntryBaseObject->Type>10) return;
			if (((CExpression*)(KeyboardEntryObject))->m_NumElements<0) return;
			if (((CExpression*)(KeyboardEntryObject))->m_pElementList==NULL) return;
		}
		catch (...)
		{
			this->ReleaseDC(DC);
			return;
		}
		int ret=1;
		if (nChar!=6)
		{
				GuidlineElement=-2;
				GuidlinesTemporaryDisabled=2;
				PaintGuidlines(DC,-1);  //deletes guidelines if exist
				NewlineAddObject=-1;
				PaintNewlineAddMark(DC);

				if (NumSelectedObjects) //deselect any selection if it exists
				{
					for (int i=0;i<NumDocumentElements;i++)
						if (TheDocument[i].MovingDotState==3) 
						{
							TheDocument[i].MovingDotState=0;
							if (TheDocument[i].Type==1)
								((CExpression*)TheDocument[i].Object)->DeselectExpression();
							else if (TheDocument[i].Type==2)
								((CDrawing*)TheDocument[i].Object)->SelectDrawing(0);
						}
					InvalidateRect(NULL,0);
					NumSelectedObjects=0;
					NumSelectedDrawings=0;
				}
				IsSelectionFresh=0;
		}

		
		if (nChar!=-1) ret=((CExpression*)(KeyboardEntryObject))->KeyboardKeyHit(DC,ViewZoom,nChar,nRepCnt,nFlags,fcolor,1);

		if ((ret==0) || (ret==88))
		{
			//keyboard entry finised - if expression left empty, delete it
			((CExpression*)KeyboardEntryObject)->KeyboardStop();
			if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
			{
				CExpression *expr=((CExpression*)(KeyboardEntryBaseObject->Object));
				if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
				{
					DeleteDocumentObject(KeyboardEntryBaseObject);
				}
			}
			KeyboardEntryObject=NULL;
			KeyboardEntryBaseObject=NULL;
			if (ret==88)
				OnEditUndo();
			RepaintTheView();
		}


		if ((ret==2) && (prevBaseObject!=KeyboardEntryBaseObject) && (prevDocument==TheDocument))
		{
			//focus was changed to another equation
			//delete the first if empty
			try
			{			
				if ((prevBaseObject) && (prevBaseObject->Type==1))
				{
					CExpression *expr=((CExpression*)(prevBaseObject->Object));
					if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || 
						(expr->m_NumElements==0))
					{
						DeleteDocumentObject(prevBaseObject);
					}
				}
			}
			catch (...)
			{

			}
		}

		if ((nChar!=6) && (nChar<=' ')) if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

		if ((nChar!=6) && (KeyboardEntryObject) && (((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry))
		{
			if ((nChar==0x08) || (nChar==0x0D) || (nChar==0x07)) //possible the typing mode was changed
			{
					CDC *mdc=Toolbox->GetDC();
					Toolbox->PaintTextcontrolbox(mdc);
					Toolbox->ReleaseDC(mdc);
			}
			//we are remembering keystrokes and check if the keystroke string triggers accelerator
			if ((nChar==' ') || (nChar<=13)) 
			{KeystrokesList[0]=0;KeystrokesListLen=0;}
			else
			{

			CElement *elm=(((CExpression*)KeyboardEntryObject)->m_pElementList+((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry-1)->pElementObject;
			int position=((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry-1;
			if (LastEditedExp2!=(CExpression*)KeyboardEntryObject)
			{
				{KeystrokesList[0]=0;KeystrokesListLen=0;LastEditedExp2=(CExpression*)KeyboardEntryObject;}
			}

			if (elm)
			{	
				{
					if (KeystrokesListLen>=16) {KeystrokesListLen=15;memmove(KeystrokesList,KeystrokesList+1,15);memmove(KeystrokeNumElements,KeystrokeNumElements+sizeof(int),15*sizeof(int));}
					KeystrokeNumElements[KeystrokesListLen]=position;
					if ((elm->Data1[0]==0) && (position>0)) KeystrokeNumElements[KeystrokesListLen]=position-1;
					KeystrokesList[KeystrokesListLen++]=(IsALTDown)?1:nChar;
				}		
		
				if ((!elm->m_Text) && (KeystrokesListLen>=1) && 
					(elm->Data1[0]!='\\') && (elm->Data1[0]!='9'))
				{
					int len=KeystrokesListLen;
					CExpression *found=Toolbox->CheckForKeycodes(KeystrokesList,&len);
					if ((found) && ((int)strlen(elm->Data1)<=len))
					{
						int element_num=KeystrokeNumElements[KeystrokesListLen-len];
						if ((element_num>=0) && ((KeystrokesListLen-len==0) || (KeystrokeNumElements[KeystrokesListLen-len-1]<element_num) || (element_num==0)))
						{
							tDocumentStruct *ds=KeyboardEntryBaseObject;
							for (int i=element_num;i<=position;i++)
								LastEditedExp2->DeleteElement(element_num);
							LastEditedExp2->InsertEmptyElement(element_num,1,0);
							KeyboardEntryBaseObject=ds;
							LastEditedExp2->m_IsKeyboardEntry=element_num+1;
							CExpression *tmp=new CExpression(NULL,NULL,100);
							tmp->CopyExpression(found,0);
							short l,a,b;
							tmp->CalculateSize(DC,ViewZoom,&l,&a,&b);
							ClipboardExpression=tmp;
							KeyboardEntryObject=(CObject*)LastEditedExp2;
							LastEditedExp2->KeyboardKeyHit(DC,ViewZoom,6,0,0,0,0);
							KeystrokesList[0]=0;KeystrokesListLen=0;
						}
					}
				}
			}
			}
		}


		//repaint the equation
		short l,a,b;
		int RepaintAll=0;

		if (KeyboardEntryBaseObject)
		{
			if (KeyboardEntryBaseObject->Type==1)
			{

				if ((nChar!=6) || (ret==2))
				{
					if (KeyboardEntryObject)
					{
						if (IsSpacebarOnlyHit!=10)
						/*if (!DisableAutocomplete) */((CExpression*)KeyboardEntryObject)->Autocomplete(0);

						if ((AutocompleteSource!=prevAutocompleteSource) || (AutocompleteTriggered!=prevAutocompleteTriggered))
						{
							RepaintAll=1;
							prevAutocompleteSource=AutocompleteSource;
							prevAutocompleteTriggered=AutocompleteTriggered;
						}
					}
				}
				

				((CExpression*)(KeyboardEntryBaseObject->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
				if (KeyboardEntryBaseObject->Above>a*100/ViewZoom) RepaintAll=1;
				KeyboardEntryBaseObject->Above=a*100/ViewZoom;
				if (KeyboardEntryBaseObject->Below>b*100/ViewZoom) RepaintAll=1;
				KeyboardEntryBaseObject->Below=b*100/ViewZoom;
				if (KeyboardEntryBaseObject->Length>l*100/ViewZoom) RepaintAll=1;
				KeyboardEntryBaseObject->Length=l*100/ViewZoom;

				if (ret==2) RepaintAll=1;

				if ((nChar!=6) || ((clipboard) && (ClipboardExpression==NULL))) //no timer messages will scroll the text
				{
					//scrolling the document area so that the keyboard cursor is always visible
					RepaintAll=ScrollCursorIntoView();

					//check if the text object crosses wraping guideline, then wrap the text box
					if (RepaintAll>=0)
					{
						CExpression *e=(CExpression*)KeyboardEntryBaseObject->Object;
						if ((e==(CExpression*)KeyboardEntryObject) && (e->IsTextContained(-1)))
						{
							for (int l=0;l<NumRullerGuidelines;l++) 
							{
								int ll=RullerGuidelines[l];
								if ((ll>=0) && (ll<NumDocumentElements) && (TheDocument[ll].absolute_Y<-1000) && (TheDocument[ll].absolute_Y>-1050))
								{
									int guidelineX=TheDocument[ll].absolute_X;
									if ((KeyboardEntryBaseObject->absolute_X<guidelineX) && (KeyboardEntryBaseObject->absolute_X+KeyboardEntryBaseObject->Length>guidelineX))
										e->AutowrapText(DC,(guidelineX-KeyboardEntryBaseObject->absolute_X)*ViewZoom/100,0);
									
								}
							}
						}
					}
					else RepaintAll=0;
				}
				if (RepaintAll)
				{
					InvalidateRect(NULL,0);
					//UpdateWindow();
					//RepaintTheView();
				}
				else
				{
					GentlyPaintObject(KeyboardEntryBaseObject,DC);
				}

				if (nChar>6)
				{
					//we will move the equation that is edited a bit down or up if
					//it grows over the equation near it. This is only done if there is space to move.
					int dmax=0;
					int disallow=0;
					tDocumentStruct *ds=TheDocument;
					for (int ii=0;ii<NumDocumentElements;ii++,ds++)
					{
						if (ds->Type==1)
							if ((((CExpression*)KeyboardEntryBaseObject->Object)->m_NumElements>1) || (((CExpression*)KeyboardEntryBaseObject->Object)->m_pElementList->Type!=1))
							{
								if ((ds->absolute_Y+ds->Below<KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Below) &&
									(ds->absolute_Y-ds->Above>KeyboardEntryBaseObject->absolute_Y-KeyboardEntryBaseObject->Above))
									{disallow=1;break;}
								if ((KeyboardEntryBaseObject->absolute_Y-KeyboardEntryBaseObject->Above<ds->absolute_Y+ds->Below+5) &&
									(KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Below>ds->absolute_Y-ds->Above-5))
								if ((KeyboardEntryBaseObject->absolute_X<ds->absolute_X+ds->Length) &&
									(KeyboardEntryBaseObject->absolute_X+KeyboardEntryBaseObject->Length>ds->absolute_X))
								if (ds!=KeyboardEntryBaseObject)
								{
									if ((ds->absolute_Y<KeyboardEntryBaseObject->absolute_Y) && (dmax>=0))
									{
										int d=3+ds->absolute_Y+ds->Below-KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Above;
										if (dmax<d) dmax=d;
									}
									else if ((ds->absolute_Y>KeyboardEntryBaseObject->absolute_Y) && (dmax<=0))
									{
										int d=-3+ds->absolute_Y-ds->Above-KeyboardEntryBaseObject->absolute_Y-KeyboardEntryBaseObject->Below;
										if (d<dmax) dmax=d;
									}
									else
										{disallow=1;break;} //we will not change y position if there is no room below
								}
							}
					}
					if ((dmax) && (abs(dmax)<25) && (!disallow))
					{
						KeyboardEntryBaseObject->absolute_Y+=dmax;
						InvalidateRect(NULL,0);
						UpdateWindow();
					}
				}
			}
			else if (KeyboardEntryBaseObject->Type==2)
			{
				short w,h;
				((CDrawing*)(KeyboardEntryBaseObject->Object))->CalculateSize(DC,ViewZoom,&w,&h);
				//GentlyPaintObject(KeyboardEntryBaseObject,DC);
				InvalidateRect(NULL,0);
				UpdateWindow();
			}
		}

	}
	else 
	{
		//*** if the typing mode is not acitve ***

		if (KeyboardEntryBaseObject==NULL)
		if (!ViewOnlyMode)
		if ((nChar>6) && (nChar!=11) && (nChar!=12))
		{
			//replacing or inserting individual character/operator into an expression
			//at the point where mouse is pointing

			POINT cursor;
			GetCursorPos(&cursor);
			ScreenToClient(&cursor);

			int absX=ViewX+cursor.x*100/ViewZoom;
			int absY=ViewY+cursor.y*100/ViewZoom;
			int i;
			tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
			for (i=NumDocumentElements-1;i>=0;i--,ds--)
			{
				if ((((absY>=ds->absolute_Y-ds->Above) && (absY<=ds->absolute_Y+ds->Below) &&
					(absX>=ds->absolute_X) && (absX<=ds->absolute_X+ds->Length)) || (ds==IsInsertionPointTouched) || ((NewlineAddObject&0x3FFFFFFF)==i)) &&
					(ds->MovingDotState!=5) && (ds->MovingDotState!=3))
				if (ds->Type==1)
				{
					CExpression *tmp=NULL;

					if (i==(NewlineAddObject&0x3FFFFFFF))
					{
						//clicked just below a multiline or text object - we will append a new line to that object
						CExpression *e=(CExpression*)ds->Object;
						int last_obj=e->m_NumElements-1;
						if (((e->m_pElementList+last_obj)->Type!=2) || ((e->m_pElementList+last_obj)->pElementObject->Data1[0]!=(char)0xFF))
						{
							//we will add line wrap to this object
							e->InsertEmptyElement(e->m_NumElements,2,(char)0xFF);
						}
						e->m_Alignment=1;
						e->DeselectExpression();
						e->m_Selection=e->m_NumElements+1;
						tmp=e;
					}
					else
					{
						if (IsDrawingMode)
						{
							short is_exp;
							char is_parenth;
							((CExpression*)ds->Object)->SelectObjectAtPoint(DC,ViewZoom,((absX-ds->absolute_X)*ViewZoom+50)/100,(absY-ds->absolute_Y)*ViewZoom/100,&is_exp,&is_parenth,2);
						}

						tmp=((CExpression*)ds->Object)->AdjustSelection();
						if ((tmp) && (/*(nChar==13) ||*/ (nChar==27) || /*(nChar==32) ||*/ (nChar==9))) break;

						if ((tmp) && (tmp->DecodeInternalInsertionPoint())) 
						{
							//if this is insertion into text
						}
						else if (tmp)
						{
							if ((nChar==13) || (nChar==' ')) break; 
							tmp->m_Selection=0;
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("insert into",20303);

							int ii;
							for (ii=0;ii<tmp->m_NumElements;ii++)
								if ((tmp->m_pElementList+ii)->IsSelected) break;
							if (tmp->DeleteSelection()!=2)	tmp->m_Selection=ii+1; //(the DeleteSelection returns 2 if the whole expression is deleted)
	
						}
						else
						{
							//nothing is selected - try another method (SelectObjectAtPoint)
							//this will reveal if any insertion point is touched
							short is_expression=0; 
							char is_parenth;
							QuickTypeUsed=1;
							tmp=(CExpression*)(((CExpression*)ds->Object)->SelectObjectAtPoint(DC,ViewZoom,(short)(cursor.x-(TheDocument[i].absolute_X-ViewX)*ViewZoom/100),(short)(cursor.y-(TheDocument[i].absolute_Y-ViewY)*ViewZoom/100),&is_expression,&is_parenth,1));
							QuickTypeUsed=0;
							if ((tmp==NULL) && (is_expression==0) && (((CExpression*)ds->Object)->m_MaxNumColumns==1) && (((CExpression*)ds->Object)->m_MaxNumRows==1)) continue;
							if ((is_expression<=0) || (is_expression==0x7FFF)) tmp=NULL;
							if ((tmp) && (tmp->m_Selection==0)) tmp=NULL;

							if (tmp) 
								((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("insert into",20303);
							else
								((CExpression*)ds->Object)->DeselectExpression();

						}
					}
					
					if (tmp)
					{
						//we will make insertion into the 'tmp' expression
						int x,y;
						POINT pp;
						if ((tmp->DecodeInternalInsertionPoint()) ||
							((tmp->m_Selection) && (tmp->DetermineInsertionPointType(tmp->m_Selection-1))))
						{
							nFlags=Toolbox->GetUniformFormatting();
							fcolor=Toolbox->GetUniformFormattingColor();
						}
						int inserting_position=tmp->m_Selection-1;
						if (tmp->KeyboardQuickType(DC,ViewZoom,nChar,nRepCnt,nFlags,fcolor,&x,&y))
						{
							QuickTypeUsed=2;
							LastQuickTypeObject=ds;
							
							short l,a,b;
							((CExpression*)ds->Object)->CalculateSize(DC,ViewZoom,&l,&a,&b);
							ds->Above=a*100/ViewZoom;
							ds->Below=b*100/ViewZoom;

							ds->Length=l*100/ViewZoom;
							pp.x=(ds->absolute_X-ViewX)*ViewZoom/100+x;
							pp.y=(ds->absolute_Y-ViewY)*ViewZoom/100+y;
							ClientToScreen(&pp);
							DisableMultitouch=2;
							SetCursorPos(pp.x,pp.y);
							ScreenToClient(&pp);
							if ((nChar!=8) && (nChar!=7)) 
							{
								OnMouseMove(0,pp);
							}


//*****************************************  //easycast during quick type??
/*
if ((nChar!=6) )
		{
			//we are remembering keystrokes and check if the keystroke string triggers accelerator
			if ((nChar==' ') || (nChar<=13)) 
			{KeystrokesList[0]=0;KeystrokesListLen=0;}
			else
			{

			CElement *elm=(tmp->m_pElementList+inserting_position)->pElementObject;
			int position=inserting_position;
			if (LastEditedExp2!=tmp)
			{
				KeystrokesList[0]=0;KeystrokesListLen=0;LastEditedExp2=tmp;
			}

			if (elm)
			{	
				{
					if (KeystrokesListLen>=16) {KeystrokesListLen=15;memmove(KeystrokesList,KeystrokesList+1,15);memmove(KeystrokeNumElements,KeystrokeNumElements+sizeof(int),15*sizeof(int));}
					KeystrokeNumElements[KeystrokesListLen]=position;
					if ((elm->Data1[0]==0) && (position>0)) KeystrokeNumElements[KeystrokesListLen]=position-1;
					KeystrokesList[KeystrokesListLen++]=(IsALTDown)?1:nChar;
				}		
		
				if ((!elm->m_Text) && (KeystrokesListLen>=1) && 
					(elm->Data1[0]!='\\') && (elm->Data1[0]!='9'))
				{
					int len=KeystrokesListLen;
					CExpression *found=Toolbox->CheckForKeycodes(KeystrokesList,&len);
					if ((found) && ((int)strlen(elm->Data1)<=len))
					{
						int element_num=KeystrokeNumElements[KeystrokesListLen-len];
						if ((element_num>=0) && ((KeystrokesListLen-len==0) || (KeystrokeNumElements[KeystrokesListLen-len-1]<element_num)))
						{
							//tDocumentStruct *ds=KeyboardEntryBaseObject;
							for (int i=element_num;i<=position;i++)
								LastEditedExp2->DeleteElement(element_num);
							//LastEditedExp2->InsertEmptyElement(element_num,1,0);
							//KeyboardEntryBaseObject=ds;
							CExpression *tmp=new CExpression(NULL,NULL,100);
							tmp->CopyExpression(found,0);
							short l,a,b;
							tmp->CalculateSize(DC,ViewZoom,&l,&a,&b);
							ClipboardExpression=tmp;
							LastEditedExp2->m_IsKeyboardEntry=element_num+1;
							LastEditedExp2->InsertEmptyElement(element_num,1,0);
							LastEditedExp2->m_KeyboardCursorPos=0;
							KeyboardEntryObject=(CObject*)LastEditedExp2;
							LastEditedExp2->KeyboardKeyHit(DC,ViewZoom,6,0,0);
							{
								short l,a,b;
								LastEditedExp2->CalculateSize(DC,ViewZoom,&l,&a,&b);
								int x,y;
								LastEditedExp2->GetKeyboardCursorPos(&x,&y);
								POINT pp;
								pp.x=(ds->absolute_X-ViewX)*ViewZoom/100+x;
								pp.y=(ds->absolute_Y-ViewY)*ViewZoom/100+y;
								ClientToScreen(&pp);
								SetCursorPos(pp.x,pp.y);
								if ((nChar!=8) && (nChar!=7)) OnMouseMove(0,pp);
							}
							if ((KeyboardEntryObject) && ((((CExpression*)KeyboardEntryObject)->m_pElementList+((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry-1)->Type==1))
							{
								((CExpression*)KeyboardEntryObject)->DeleteElement(((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry-1);
							}
							KeyboardEntryObject=NULL;
							LastEditedExp2->m_IsKeyboardEntry=0;
						}
					}
				}
			}
			}
		}
*/
//***************************************





						}
						if ((((CExpression*)(ds->Object))->m_NumElements==1) &&
							(((CExpression*)(ds->Object))->m_pElementList->Type==0))
							DeleteDocumentObject(ds);
					}
					else
						((CExpression*)(ds->Object))->DeselectExpression();
					break;					
				}
				else if ((ds->Type==2) && (nChar==' '))
				{
					CDrawing *d=(CDrawing*)ds->Object;
					int nodeedit;
					if (d->SelectObjectAtPoint(DC,ViewZoom,(absX-ds->absolute_X)*ViewZoom/100,(absY-ds->absolute_Y)*ViewZoom/100,&nodeedit))
					{
						if (nodeedit) break; //pressing space bar will add node, we don't wont quick type to be activated
					}
				}
				int margins=200/ViewZoom+1;
				if ((absY>=ds->absolute_Y-ds->Above-margins) && (absY<=ds->absolute_Y+ds->Below+margins) &&
					(absX>=ds->absolute_X-margins) && (absX<=ds->absolute_X+ds->Length+margins) &&
					(ds->MovingDotState!=5) && (ds->Type==1))
					break; //is clicked very near another object - ignore quick-type
			}
			if ((i==-1) && (nChar>=' ') && (absX>0) && (absY>0) && (cursor.x>0))
			{
				if (IsALTDown)
				{
					if (KeyboardEntryObject==NULL)
					{
						int prev=IsDrawingMode;
						int prev2=SelectedLineColor;
						int prev3=SelectedLineWidth;
						if (nChar=='1') IsDrawingMode=2;
						if (nChar=='2') IsDrawingMode=3;
						if (nChar=='3') IsDrawingMode=5;
						if (nChar=='4') IsDrawingMode=4;
						if (nChar=='5') IsDrawingMode=1;
						if (nChar=='6') IsDrawingMode=7;
						if (nChar=='7') IsDrawingMode=17;
						if (nChar=='8') IsDrawingMode=27;
						if (nChar=='9') IsDrawingMode=6;
						char cc=toupper(nChar);
						if (cc=='R') SelectedLineColor=1;
						if (cc=='G') SelectedLineColor=2;
						if (cc=='B') SelectedLineColor=0;
						if (cc=='T') SelectedLineWidth=DRWZOOM;
						if (cc=='H') SelectedLineWidth=65*DRWZOOM/100;
						if ((cc=='M') || (cc=='S')) SelectedLineWidth=2*DRWZOOM;
					
						if (prev!=IsDrawingMode)
						{
							this->SetMousePointer();
							return;
						}
						else if ((prev2!=SelectedLineColor) || (prev3!=SelectedLineWidth))
						{
							PaintDrawingHotspot();
							if (Toolbox) {Toolbox->InvalidateRect(0,NULL);Toolbox->UpdateWindow();}
							return;
						}
					}
				}
				else if (nChar==' ')
				{
					//if spacebar was hit, show the 'T' sign
					if (SpacebarPressedRecently)
					{
						POINT cp;
						GetCursorPos(&cp);
						this->ScreenToClient(&cp);
						DisplayShortText("T",cp.x-5,cp.y+28,0,3);
					}
				}
				else
				{


					//nothing found below the cursor - we are creating the new object
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("insert new",20401);

					//NumDocumentElements++;
					//CheckDocumentMemoryReservations();
					int fontsize=(SpecialDrawingHover)?DefaultFontSize*8/10:DefaultFontSize;  //note - inside drawing box we make somewhat smaller font
					absX-=fontsize/16;

					int make_text=0;
					if (GuidlineElement>=0)
					{
						absX=prevGuidlineX;
						make_text=SelectedGuidelineType;
					}
					else if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
					{
						absX=((absX+GRID/2)/GRID)*GRID-3;
						absY=((absY+GRID/2)/GRID)*GRID;
					}

					AddDocumentObject(1,absX,absY);
					tDocumentStruct *ds=TheDocument+NumDocumentElements-1;

					ds->Object=(CObject*)new CExpression(NULL,NULL,fontsize); 

					if (SpacebarPressedRecently) make_text=1;

					((CExpression*)(ds->Object))->m_Selection=1;
					((CExpression*)(ds->Object))->m_ModeDefinedAt=1+(make_text<<14);
					if (make_text)
					{
						nFlags=Toolbox->GetUniformFormatting();
						fcolor=Toolbox->GetUniformFormattingColor();
					}

					int x,y;
					POINT pp;
					if (((CExpression*)(ds->Object))->KeyboardQuickType(DC,ViewZoom,nChar,nRepCnt,nFlags,fcolor,&x,&y))
					{
						QuickTypeUsed=2;
						IsSpacebarOnlyHit=0;

						if (((CExpression*)(ds->Object))->m_pElementList->pElementObject)
							((CExpression*)(ds->Object))->m_pElementList->pElementObject->m_Text=make_text;
						/*else if (nChar==' ')
						{
							((CExpression*)(ds->Object))->m_StartAsText=1;
							((CExpression*)(ds->Object))->m_Alignment=1;
						}*/
						pp.x=(ds->absolute_X-ViewX)*ViewZoom/100+x;
						pp.y=(ds->absolute_Y-ViewY)*ViewZoom/100+y;
						ClientToScreen(&pp);
						SetCursorPos(pp.x,pp.y);
						make_text=1234; //just flag that we must call OnMouseMove (see down below)
					}
				
					short l,a,b;
					((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
					ds->Length=(short)((int)l*100/(int)ViewZoom);
					ds->Above=(short)((int)a*100/(int)ViewZoom);
					ds->Below=(short)((int)b*100/(int)ViewZoom);
					ds->MovingDotState=0;		
					DisableMultitouch=2;
					if (make_text==1234)
					{
						ScreenToClient(&pp);
						if ((nChar!=8) && (nChar!=7)) 
								OnMouseMove(0,pp);
					}
					
				}
			}
			RepaintTheView();
		}
		if (((nChar>=2) && (nChar<=5)) || (nChar==11) || (nChar==12))
		{
			//scorlling the main screen by means of keyboard (arrow keys, page up, page down)
			PaintDrawingHotspot(1);
			if (GuidlineElement>=0)
			{
				GuidlineElement=-1;
				PaintGuidlines(DC,-1);
			}
			if (NewlineAddObject>=0)
			{
				NewlineAddObject=-1;
				PaintNewlineAddMark(DC);
			}


			int px=ViewX;
			int py=ViewY;
			if (nChar==2) ViewY+=2400/ViewZoom;
			if (nChar==3) ViewY-=2400/ViewZoom;
			if (nChar==5) ViewX+=2400/ViewZoom;
			if (nChar==4) ViewX-=2400/ViewZoom;
			if (nChar==11)
			{
				RECT cl=TheClientRect;
				cl.bottom/=24;
				cl.bottom*=24;
				cl.bottom=cl.bottom*100/ViewZoom;
				ViewY-=cl.bottom;
			}
			if (nChar==12)
			{
				RECT cl=TheClientRect;
				cl.bottom/=24;
				cl.bottom*=24;
				cl.bottom=cl.bottom*100/ViewZoom;
				ViewY+=cl.bottom;
			}
			if (ViewX<0) ViewX=0;
			if (ViewY<0) ViewY=0;
			if ((px!=ViewX) || (py!=ViewY)) this->RestoreClipboardBackground();
			ScrollWindow((px-ViewX)*ViewZoom/100,(py-ViewY)*ViewZoom/100);
			SetScrollPos(SB_VERT,ViewY,1);
			SetScrollPos(SB_HORZ,ViewX,1);
		}
	}

	this->ReleaseDC(DC);
}
#pragma optimize("",on)

int Reenable;


//extern CRichEditCtrl *StaticMessageWindow;


extern int StaticMessageWindowCntr;
POINT prevShowRullerCursor;
extern int ToolbarHelpTimer;
int SteadyCursorX;
int SteadyCursorY;

int OnTimerCounter;
unsigned int prevTimerTickTime;
unsigned int TimerTickAccumulator;
void CMathomirView::OnTimer(UINT nIDEvent)
{
	//faster timing functions here
	unsigned int TimerTickTime=GetTickCount();
	unsigned int TimerTickDelta=TimerTickTime-prevTimerTickTime;
	if (TimerTickDelta>199) TimerTickDelta=199;
	if (nIDEvent==0xFFFF) //we are here only to make scroll - internall call
		TimerTickDelta=10;
	else
		prevTimerTickTime=TimerTickTime;
	
	try
	{
		//main window scroll animation - for smooth scrolling when scrolling by mouse wheel
		if (MouseWheelAnimation)
		{
			int multiply=TimerTickDelta;
			if (multiply>30) multiply=30;
			int step=MouseWheelAnimation;
			if (abs(step*multiply)>36000)
				step=step*multiply/60;
			else if (step>60*multiply) 
				step=60*multiply;
			else if (step<-60*multiply)
				step=-60*multiply;
			MouseWheelAnimation-=(step/ViewZoom)*ViewZoom;
			int prevY=ViewY;
			if (MouseWheelDirection)
				ViewY+=step/ViewZoom;
			else
				ViewY-=step/ViewZoom;
			if (ViewY!=prevY)
			{
				if ((GetKeyState(VK_RBUTTON)&0xFFFE)==0) MouseOverScrollbar=5;
				if (ViewY<0) ViewY=0;
				SetScrollPos(SB_VERT,ViewY,1);
				InvalidateRect(NULL,0);
				UpdateWindow();
			}
		}
		if (MouseWheelAnimationX)
		{
			int multiply=TimerTickDelta;
			if (multiply>30) multiply=30;
			int step=MouseWheelAnimationX;
			if (abs(step*multiply)>36000)
				step=step*multiply/60;
			else if (step>60*multiply) 
				step=60*multiply;
			else if (step<-60*multiply)
				step=-60*multiply;
			MouseWheelAnimationX-=(step/ViewZoom)*ViewZoom;
			int prevX=ViewX;
			ViewX-=step/ViewZoom;
			if (ViewX!=prevX)
			{
				if ((GetKeyState(VK_RBUTTON)&0xFFFE)==0) MouseOverScrollbar=5;
				if (ViewX<0) ViewX=0;
				SetScrollPos(SB_HORZ,ViewX,1);
				InvalidateRect(NULL,0);
				UpdateWindow();
			}
		}
	}
	catch(...)
	{
		FatalErrorHandling();
	}

	if (nIDEvent==0xFFFF) return; //this was an internal call only for scrolling

	if (((GetKeyState(VK_LBUTTON)&0xFFFE) && 
		((MouseMode==8) || //drawing node editing
		 (MouseMode==9) || //selection stretching
		 (MouseMode==5) || //multiple selections
		 (MouseMode==2) || (MouseMode==102) || //moving objects on moving dot
		 (MouseMode==4) || //adding space
		 (MouseMode==7) || //Moving of drawings
		 ((MouseMode==6) && (IsDrawingMode!=3) && (IsDrawingMode!=6) && (IsDrawingMode!=7) && (IsDrawingMode!=14) && (IsDrawingMode!=25) && (IsDrawingMode!=26)))) || 
		 (MouseMode==102)) //moving objects after being pasted
	{ 
		//Support for auto scrolling (when doing some actions with mouse and the mouse pointer goes out of main client window)
		int Speed=8;
		if ((MouseMode==7) || (MouseMode==2) || (MouseMode==102)) Speed=12;
		if ((MouseMode==8)) Speed=4;
		RECT r;
		GetClientRect(&r);
		POINT p2,p;
		GetCursorPos(&p);
		p2=p;
		ScreenToClient(&p);
		if (p.y>r.bottom) 
		{
			int d=(p.y-r.bottom)*Speed;
			if (d>350) d=350;
			MouseWheelAnimation-=d;
			SetCursorPos(p2.x,p2.y);
		}
		if (p.y<0) 
		{
			int d=-p.y*Speed;
			if (d>350) d=350;
			MouseWheelAnimation+=d;
			SetCursorPos(p2.x,p2.y);
		}
		if (p.x>r.right) 
		{
			int d=(p.x-r.right)*Speed;
			if (d>350) d=350;
			MouseWheelAnimationX-=d;
			SetCursorPos(p2.x,p2.y);
		}
		if (p.x<0) 
		{
			int d=-p.x*Speed;
			if (d>350) d=350;
			MouseWheelAnimationX+=d;
			SetCursorPos(p2.x,p2.y);
		}
	}

	//the further code is executed every 100ms
	TimerTickAccumulator+=TimerTickDelta;
	if (TimerTickAccumulator<100)
	{
		return;
	}
	TimerTickAccumulator-=100;


	{
		//we check if our window has the input focus
		HWND fgw=::GetForegroundWindow();
		if (fgw)
		{
			if ((theApp.m_pMainWnd==NULL) || (fgw!=theApp.m_pMainWnd->m_hWnd))
				IsWindowOutOfFocus=1;
			else
				if (IsWindowOutOfFocus>0) IsWindowOutOfFocus--;
		}
	}


	LeftClickTimer++;

	{
		//this generates the popup drawing toolbox when long time (1sec) the right-mouse-button was held down
		int t=MouseSteady;
		POINT cur;
		GetCursorPos(&cur);
		if ((abs(SteadyCursorX-cur.x)<4) && (abs(SteadyCursorY-cur.y)<4)) SteadyCursorTimer++; else {SteadyCursorTimer=0;MouseSteady=0;}
		SteadyCursorX=cur.x;
		SteadyCursorY=cur.y;
		if (SteadyCursorTimer>2) MouseSteady=1;
		if ((t!=MouseSteady) && (MouseMode==5)) //multi-selection frame (for quick-drawing of curly brackets and section lines)
		{
			this->InvalidateRect(NULL,0);
			this->UpdateWindow();
		}
		if ((SteadyCursorTimer>1) && (GetTickCount()>PreopenDrawingToolboxTime) && (!ScrollModeWasAccepted) && (ClipboardExpression==NULL) &&
			(MouseMode==1) && (PreopenDrawingToolboxX==0) && (PreopenDrawingToolboxLock==0) && (GetKeyState(VK_RBUTTON)&0xFFFE)) // for opening a drawing tools quick menu (the same as f4 key)
		{
			ScreenToClient(&cur);
			PreopenDrawingToolboxX=cur.x;
			PreopenDrawingToolboxY=cur.y;
			SteadyCursorTimer=0;
			this->InvalidateRect(NULL,0);
			this->UpdateWindow();
		}
	}	

	if ((Toolbox) && (Toolbox->Subtoolbox) && (Toolbox->Subtoolbox->IsWindowVisible()) && (!Toolbox->ContextMenu->IsWindowVisible()) && (!Popup->IsWindowVisible()))
	{
		RECT rr;
		POINT pp;
		GetCursorPos(&pp);
		Toolbox->Subtoolbox->GetWindowRect(&rr);
		if ((pp.x<rr.left-ToolboxSize) || (pp.x>rr.right+2*ToolboxSize/3) ||
			(pp.y<rr.top-ToolboxSize) || (pp.y>rr.bottom+2*ToolboxSize/3))
		{
			if ((Toolbox->ContextMenu==NULL) || (!(Toolbox->ContextMenu->IsWindowVisible())))
			{
				Toolbox->Subtoolbox->m_IsSubtoolbox=-1;
				Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
			}
		}
	}
	if ((Toolbox) && (ToolboxAutoopen))
	{
		ToolboxAutoopen++;
		if ((Toolbox->ContextMenu) && (Toolbox->ContextMenu->IsWindowVisible())) ToolboxAutoopen=0;
		if ((ToolboxAutoopen>12) || ((ToolboxAutoopen>8) && (Toolbox->m_FontModeElement!=1)) || ((ToolboxAutoopen>4) && (Toolbox->m_IsArrowSelected)))
		{
			if ((Toolbox->m_SelectedElement<48) && (Toolbox->m_SelectedElement>=0))
			{
				CToolbox *Subtoolbox=Toolbox->Subtoolbox;
				Subtoolbox->m_IsSubtoolbox=Toolbox->m_SelectedElement+1;
				Subtoolbox->m_SelectedElement=-1;
				Subtoolbox->AdjustPosition();
				Subtoolbox->InvalidateRect(NULL,1);
				Subtoolbox->UpdateWindow();
				NoImageAutogeneration=2;
				Subtoolbox->ShowWindow(SW_SHOWNA);
			}
			if ((Toolbox->m_FontModeElement==0) || //uniform fonts
				(Toolbox->m_FontModeElement==1)) //mixed fonts
			{
				CToolbox *Subtoolbox=Toolbox->Subtoolbox;
				Subtoolbox->m_IsSubtoolbox=-Toolbox->m_FontModeElement-1;
				Subtoolbox->AdjustPosition();
				Subtoolbox->InvalidateRect(NULL,0);
				Subtoolbox->UpdateWindow();
				Subtoolbox->m_SelectedElement=-1;
				Subtoolbox->m_IsArrowSelected=0;
				NoImageAutogeneration=2;
				Subtoolbox->ShowWindow(SW_SHOWNA);
			}

			ToolboxAutoopen=0;
		}
	}


	//Following code executes every 300ms
	OnTimerCounter++;
	if (OnTimerCounter<3) return;
	OnTimerCounter=0;


	
	try {



		if (ReenableMenu==1)
		{
			//reenables menu some time after the ALT key was used (the menu was disabled to prevent menu access with the alt key)
			CMenu *theMenu;
			theMenu=theApp.m_pMainWnd->GetMenu();

			theMenu->EnableMenuItem(0,MF_BYPOSITION|MF_ENABLED);
			theMenu->EnableMenuItem(1,MF_BYPOSITION|MF_ENABLED);
			theMenu->EnableMenuItem(2,MF_BYPOSITION|MF_ENABLED);
			theMenu->EnableMenuItem(3,MF_BYPOSITION|MF_ENABLED);
			theMenu->EnableMenuItem(4,MF_BYPOSITION|MF_ENABLED);
		}
		if (ReenableMenu>0) ReenableMenu--;




	if (QuickDrawingDisable) 
	{
		if ((QuickDrawingDisable>20) || (QuickDrawingDisable<0)) QuickDrawingDisable=20;
		QuickDrawingDisable--;
	}

	if ((QuickTypeUsed) && ((GetKeyState(VK_SHIFT)&0xFFFE)==0)) QuickTypeUsed--;
	if (DisableMultitouch) DisableMultitouch--;

	if ((ToolbarHelpTimer) && (UseToolbar) && (Toolbox->Toolbar))
	{
		if (ToolbarHelpTimer<10) ToolbarHelpTimer++;
		if (ToolbarHelpTimer==6)
		{
			Toolbox->Toolbar->ToolbarShowHelp();
		}
	}


	//handling of long-clicks.
	if (/*((ClipboardExpression==NULL) && (KeyboardEntryObject==NULL))*/ (TouchMouseMode==0) || ((GetKeyState(VK_LBUTTON)&0xFFFE)==0))
	{
		LongClickObject=NULL;
	}
	if (LongClickObject)
	{
		CPoint cpos;
		GetCursorPos(&cpos);
		ScreenToClient(&cpos);

		if ((((GetTickCount()-LongClickStartTime)>850) && (abs(LongClickPosition.x-cpos.x)<=2) && (abs(LongClickPosition.y-cpos.y)<=2)) ||
			(((GetTickCount()-LongClickStartTime)>650) && (abs(LongClickPosition.x-cpos.x)<=1) && (abs(LongClickPosition.y-cpos.y)<=1)))

		{
			CExpression *parent=LongClickObject;
			while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;
			int i;
			for (i=0;i<NumDocumentElements;i++)
				if (TheDocument[i].Object==(CObject*)parent) break;
			if (i<NumDocumentElements)
			{
				//LongClickObject->DeleteSelection();

				OnLButtonUp(0,cpos);
				LongClickObject->DeleteSelection();
				/*for (int j=0;j<LongClickObject->m_NumElements;j++)
				{
					tElementStruct *ts=LongClickObject->m_pElementList+j;
					if (ts->IsSelected)
					{
						if ((LongClickObject!=(CExpression*)KeyboardEntryObject) || (j!=LongClickObject->m_IsKeyboardEntry-1))
						{
							LongClickObject->DeleteElement(j);
							if ((LongClickObject==(CExpression*)KeyboardEntryObject) && (LongClickObject->m_IsKeyboardEntry>j))
								LongClickObject->m_IsKeyboardEntry--;
							j--;
						}
					}
				}*/
				//if ((LongClickObject->m_ParenthesesSelected) && (LongClickObject->m_pPaternalElement)) LongClickObject=LongClickObject->m_pPaternalExpression;

				if ((parent->m_NumElements==1) && (parent->m_pElementList->Type==0))
				{
					DeleteDocumentObject(TheDocument+i);
				}
				else
				{
					CDC *DC=this->GetDC();
					short l,a,b;
					parent->CalculateSize(DC,ViewZoom,&l,&a,&b);
					TheDocument[i].Length=l*100/ViewZoom;
					TheDocument[i].Above=a*100/ViewZoom;
					TheDocument[i].Below=b*100/ViewZoom;

					GentlyPaintObject(TheDocument+i,DC);
					this->ReleaseDC(DC);
				}
				InvalidateRect(NULL,0);
				UpdateWindow();
			}
			LongClickObject=NULL;
		}
	}

	if ((IsHighQualityRendering) && ((ClipboardExpression) || (KeyboardEntryObject)))
	{
		if (PresentationModeActiveTimer<10)
			PresentationModeActiveTimer++;
		else
		{
			DisplayShortText("Presentation mode is active (F5 to toggle)",1,1,7005,1);
		}
	}
	else
		PresentationModeActiveTimer=0;
	if (!ViewOnlyMode)
	if ((theApp.m_pMainWnd) && (theApp.m_pMainWnd->IsWindowVisible()))
	if (!Popup->IsWindowVisible())
	if ((MouseMode==0) && /*(IsDrawingMode==0) &&*/ (ClipboardExpression==NULL) && (ClipboardDrawing==NULL))
	{
		POINT cursor;
		GetCursorPos(&cursor);
		if (this->WindowFromPoint(cursor)==this)
		{
			this->ScreenToClient(&cursor);
			//int ttt=(14-ViewX)*ViewZoom/100;
			//if (ttt<8) ttt=8;
			int ttt=8;
			if (ShowRullerCounter==0) ttt=12;//max(ttt,14);
			if ((cursor.y<=12) && (cursor.y>=0) && (cursor.x>0) && (((abs(cursor.x-prevShowRullerCursor.x)<=1) && (abs(cursor.y-prevShowRullerCursor.y)<=1)) || ((ShowRullerCounter==0)&&(RullerType==0))))
			{
				if (ShowRullerCounter) 
				{
					if (!MouseIsTouchingExpression) ShowRullerCounter--;
					if (ShowRullerCounter==0) 
					{
						PaintDrawingHotspot(); 
						RullerType=0;
						RullerPositionPreselected=cursor.x*100/ViewZoom+ViewX;
						RullerPositionPreselected=((RullerPositionPreselected+GRID/2)/GRID)*GRID-3;
						RepaintTheView();
					}
				}
				/*else
				{
					ShowRullerCounter=0;
				}*/
			}
			else if ((cursor.x<=ttt) && (cursor.x>=0) && (cursor.y>0) && (((abs(cursor.x-prevShowRullerCursor.x)<=1) && (abs(cursor.y-prevShowRullerCursor.y)<=1)) || ((ShowRullerCounter==0)&&(RullerType==1))))
			{
				
				if (ShowRullerCounter) 
				{
					if (!MouseIsTouchingExpression) ShowRullerCounter--;
					if (ShowRullerCounter==0) 
					{
						PaintDrawingHotspot();
						RullerType=1;
						RepaintTheView();
					}
				}
				/*else
				{
					ShowRullerCounter=0;
				}*/
			}
			else
			{
				if (ShowRullerCounter==0) {ShowRullerCounter=5;RepaintTheView();}
				ShowRullerCounter=3;
				
			}
			prevShowRullerCursor=cursor;
		}
	}


	if (StaticMessageWindow)
	{
		if (StaticMessageWindowCntr)
		{
			StaticMessageWindowCntr--;
			if (StaticMessageWindowCntr==0) StaticMessageWindow->ShowWindow(SW_HIDE);
		}
	}
	if (IsMouseUsed) {IsMouseUsed--;}
	if (NoImageAutogeneration) NoImageAutogeneration--;
	if (Reenable)
	{
		if (Reenable==1)
		{
			Toolbox->AdjustPosition();
			Toolbox->EnableWindow(1);
			Toolbox->Keyboard->EnableWindow(1);
			if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->EnableWindow(1);
			this->EnableWindow(1);
			this->SetFocus();
		}
		Reenable--;
	}
	if (theApp.m_pMainWnd==NULL) return;

	if (TemporaryShowColoredSelection)
	{
		TemporaryShowColoredSelection--;
		if (TemporaryShowColoredSelection==0) this->InvalidateRect(NULL,0);
	}

	if ((theApp.m_pMainWnd) && (theApp.m_pMainWnd->IsWindowVisible()))
		if ((Popup) && (Popup->IsWindowVisible())) Popup->PaintWorkIndicator();
	
	if (!ViewOnlyMode)
	if (AutosaveOption)
	{
		AutosaveFreetime++;
		if ((AutosaveTime==0) || (AutosaveTime>GetTickCount()) || (AutosavePoints==0)) AutosaveTime=GetTickCount();
		DWORD timedelta=GetTickCount()-AutosaveTime;
		if ((timedelta*AutosavePoints>(DWORD)((AutosaveOption==3)?3500000:((AutosaveOption==2)?8000000:18000000))) && (AutosaveFreetime>=(5+9/AutosaveOption)))
		{
			if (TheFileType!='s')
			{
				DisplayShortText("Autosaving...",10,10,7006);
				this->GetDocument()->SaveMOMFile("autosave.mom",TheFileType);
				AutosaveTime=GetTickCount();
				AutosavePoints=0;
			}
		}
	}

	if (GetForegroundWindow()==theApp.m_pMainWnd)
	{
		//ensuring that the keyboard focus is set to the view window
		CWnd *frw=GetFocus();
		if ((frw==Toolbox) || (frw==Toolbox->Toolbar) || (frw==ListPopup) || (frw==theApp.m_pMainWnd))
		{
			this->SetFocus();
		}
	}

	//special handling - peek data in clipboard and check if there is any expression inside - if yes, copy it into local clipboard
	//only non-active window will peek into the windows clipboard
#ifdef TEACHER_VERSION
	if (TheFileType!='r')
#endif
	if (theApp.m_pMainWnd->GetActiveWindow()!=theApp.m_pMainWnd)
	if (theApp.m_pMainWnd->OpenClipboard())
	{
		HANDLE clipb_data;
		UINT format=RegisterClipboardFormat("MATHOMIR_EXPR");
		clipb_data=GetClipboardData(format);
		if (clipb_data==NULL) 
			CloseClipboard();
		else
		{
			int len=(int)GlobalSize(clipb_data);
			LPVOID pntr=GlobalLock(clipb_data);
			if (pntr==NULL)
				CloseClipboard();
			else
			{
				int *control=(int*)pntr;
				int *checksum=control+1;
				char *data=(char*)(checksum+1);
				if ((*control==0xBBCCDD11) && (ClipboardExpression!=NULL))
				{
					//command that clipboard must be deleted;
					delete ClipboardExpression;
					ClipboardExpression=NULL;
				}
				if ((*control==0xBBCCDD11) && (ClipboardDrawing!=NULL))
				{
					//command that clipboard must be deleted;
					delete ClipboardDrawing;
					ClipboardDrawing=NULL;
				}
				if ((*control==0xAABBCC11) && (*checksum!=LastTakenChecksum))
				{
					IsDrawingMode=0;
					LastTakenChecksum=*checksum;
					if (ClipboardExpression==NULL) ClipboardExpression=new CExpression(NULL,NULL,100);
					ClipboardExpression->Delete();
					ClipboardExpression->XML_input(data);
					if (KeyboardEntryObject) ((CExpression*)KeyboardEntryObject)->KeyboardStop();
				}
				if ((*control==0xAABBCC22) && (*checksum!=LastTakenChecksum))
				{
					IsDrawingMode=0;
					MovingStartX=*(int*)data;
					MovingStartY=*(((int*)data)+1);
					data=(char*)(((int*)data)+2);
					LastTakenChecksum=*checksum;
					if (ClipboardDrawing==NULL) ClipboardDrawing=new CDrawing();
					ClipboardDrawing->Delete();
					ClipboardDrawing->XML_input(data);
				}
				GlobalUnlock(clipb_data);
				CloseClipboard();
			}
		}
	}

	if (GuidlineClearTimer)
	{
		GuidlineClearTimer--;
		if (GuidlineClearTimer==0)
		{
			GuidlineElement=-1;
			CDC *DC=GetDC();
			PaintGuidlines(DC,0);
			ReleaseDC(DC);
		}
	}
	if (RepaintViewTimer) 
	{
		RepaintViewTimer--;
		if (RepaintViewTimer==0)
		{
			InvalidateRect(NULL,0);
		}
	}
	
	UINT Flags=0;
	int CTRLstate=GetKeyState(VK_CONTROL)&0xFFFE;  //the CTRL key
	int SHIFTstate=GetKeyState(16)&0xFFFE; //the SHIFT key
	int CAPSstate=GetKeyState(20)&0x0001; //the CAPS LOCK key
	if (CTRLstate) Flags|=0x01;
	if (SHIFTstate) Flags|=0x02;
	if (CAPSstate) Flags|=0x04;

#ifdef TEACHER_VERSION
	if (TheFileType=='r')
	{
		//we are going to inform the student about time

		DWORD now=GetTickCount();
		if (TheTimeLimit>0)
		{
			char xx=DisableEditing;
			
			DWORD tmp=TheExamStartTime+(DWORD)60000*(DWORD)TheTimeLimit;
			if (tmp<TheExamStartTime)
			{
				if ((now>tmp) && (now<(DWORD)0x7FFFFFFF)) DisableEditing=1;
			}
			else
				if (now>tmp) DisableEditing=1;
			if ((DisableEditing) && (xx!=DisableEditing))
			{
					char bff[128];
					CopyTranslatedString(bff,"Time elapsed!\r\n\r\nSave your work.",5050,127);
					//sprintf(bff,"Time elapsed!\r\n\r\nSave your work.");
					AfxMessageBox(bff,MB_OK | MB_ICONINFORMATION);
			}
		}
		if (TheTimeLimit>=5)
		{
			int warn;
			if (TheTimeLimit<=10) warn=2;
			else if (TheTimeLimit<=30) warn=5;
			else warn=10;
			DWORD tmp=TheExamStartTime+(DWORD)60000*(DWORD)((int)TheTimeLimit-warn);
			int t=0;
			if (tmp<TheExamStartTime)
			{
				if ((now>tmp) && (now<(DWORD)0x7FFFFFFF)) t=1;
			}
			else
				if (now>tmp) t=1;
			if ((t==1) && (WarningDisplayed==0))
			{
				WarningDisplayed=1;
				char bff[128];
				CopyTranslatedString(bff,"Time limited exam.\r\n\r\nMinutes left: ",5051,124);
				char bff2[10];
				sprintf(bff2,"%d",warn);
				strcat (bff,bff2);
				//sprintf(bff,"Time limited exam.\r\n\r\n%d minutes left.", warn);
				AfxMessageBox(bff,MB_OK | MB_ICONINFORMATION);
			}
		}
	}
#endif
	
	SendKeyStroke(6,0,Flags);
	CView::OnTimer(nIDEvent);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

#pragma optimize("s",on)
void CMathomirView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar!=VK_SHIFT)
		prevChar=0;

	try{


	if ((nChar==VK_CONTROL) && (ToolbarUseCross==0)) PaintDrawingHotspot(1);

	UINT Flags=0;
	if ((nChar==VK_CONTROL) && (IsDrawingMode))
	{
		RepaintTheView();
	}
	int CTRLstate=GetKeyState(VK_CONTROL)&0xFFFE;  //the CTRL key
	int SHIFTstate=GetKeyState(16)&0xFFFE; //the SHIFT key
	int CAPSstate=GetKeyState(20)&0x0001; //the CAPS LOCK key
	if (CTRLstate) Flags|=0x01;
	if (SHIFTstate) Flags|=0x02;
	if (CAPSstate) Flags|=0x04;

	if ((IsSpacebarOnlyHit) && (!CTRLstate) && (nChar==' '))
	{
		//if ((KeyboardEntryObject) && ((((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry>1) || (((CExpression*)KeyboardEntryObject)->m_KeyboardCursorPos!=0)))
		{
		IsSpacebarOnlyHit=10;
		SendKeyStroke(nChar,nRepCnt,nFlags);
		}
	}
	IsSpacebarOnlyHit=0;

	if (GuidlineElement>=0)
	{
		if ((!CTRLstate) && (!SnapToGuidlines))
		{
			GuidlineElement=-1;
			CDC *DC=GetDC();
			PaintGuidlines(DC,-1);
			ReleaseDC(DC);
		}
	}
	if ((nChar==VK_SHIFT) && (KeyboardEntryObject==NULL) && (QuickTypeUsed==0) && (IsDrawingMode==0))
	{
		//deselect elements once the shift key is released
		if (ClipboardDrawing==NULL)
		{
			CPoint c;
			GetCursorPos(&c);
			ScreenToClient(&c);

			TouchMouseMode=3;
			MovingStartX=ViewX+c.x*100/ViewZoom;
			MovingStartY=ViewY+c.y*100/ViewZoom;
		}
	}

	if (!CTRLstate)
		SendKeyStroke(0,0,Flags);
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

void CMathomirView::SelectLastOrTouchedObject()
{
	//this functions selects a mouse-touched object. If no object is touched,
	//then it selects the last inserted object (that is, freshest object)

	try
	{
	int found_something=0;
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(&cursor);
	MultipleStartX=-1;
	int absX=ViewX+cursor.x*100/ViewZoom;
	int absY=ViewY+cursor.y*100/ViewZoom;
	//if (!IsDrawingMode)
	for (int i=NumDocumentElements-1;i>=0;i--)
		if ((absX>=TheDocument[i].absolute_X) && (absX<=TheDocument[i].absolute_X+TheDocument[i].Length) &&
			(absY>=TheDocument[i].absolute_Y-TheDocument[i].Above) && (absY<=TheDocument[i].absolute_Y+TheDocument[i].Below) &&
			((TheDocument[i].MovingDotState!=5) || (AccessLockedObjects)))
		{
			if (TheDocument[i].MovingDotState==3)
			{
				TheDocument[i].MovingDotState=0;
				found_something=1;
				break;
			}
			else if (TheDocument[i].Type==1)
			{
				//if we are editing this expression by quick-type then we should allow Enter
				//CExpression *ex=(CExpression*)TheDocument[i].Object;
				if (LastQuickTypeObject)
				{
					//CExpression *baseparent=LastQuickTypeObject;
					//while (baseparent->m_pPaternalExpression) baseparent=baseparent->m_pPaternalExpression;
					if (LastQuickTypeObject==TheDocument+i) 
						return;
				}
				
				TheDocument[i].MovingDotState=3;
				found_something=1;
				break;
			}
			else if (TheDocument[i].Type==2)
			{
				CDrawing *drw=(CDrawing*)TheDocument[i].Object;
				if (drw->IsSelected) 
				{
					TheDocument[i].MovingDotState=3;
					found_something=1;
				}
			}
		}

	if (!found_something)
	{
		for (int i=NumDocumentElements-1;i>=0;i--)
			if ((TheDocument[i].MovingDotState!=3) && (TheDocument[i].MovingDotState!=5))
			{
				TheDocument[i].MovingDotState=3;
				break;
			}
	}
	RepaintTheView();
	if (Toolbox)
	{
		CDC *DC=Toolbox->GetDC();
		Toolbox->PaintColorbox(DC);
		Toolbox->ReleaseDC(DC);
	}
	LastQuickTypeObject=NULL;
	}
	catch(...)
	{
		
	}
}

//If CMathomirView has the input focus, then this function is called 
//whenever the ALT key or any ALT+key combination is pressed.
//(note: the Main Frame window forwards input focus to CMathomirView)
#pragma optimize("s",on)
extern int KeyboardExponentMode;
void CMathomirView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	try {
		if (nChar==VK_MENU)
		{
			//the alt key is pressed - we are temporarily disabling some menu items so that we dsable menu hotkeys.
			CMenu *theMenu;
			theMenu=theApp.m_pMainWnd->GetMenu();

			if (!EnableMenuShortcuts)
			{
				theMenu->EnableMenuItem(0,MF_BYPOSITION|MF_DISABLED);
				theMenu->EnableMenuItem(1,MF_BYPOSITION|MF_DISABLED);
				theMenu->EnableMenuItem(2,MF_BYPOSITION|MF_DISABLED);
			}
			theMenu->EnableMenuItem(3,MF_BYPOSITION|MF_DISABLED);
			theMenu->EnableMenuItem(4,MF_BYPOSITION|MF_DISABLED);
			
			ReenableMenu=2;

		}
	IsSpacebarOnlyHit=0;

	UINT Flags=0;
	int SmallCaps=1;
	int CTRLstate=GetKeyState(VK_CONTROL)&0xFFFE;	//the CTRL key
	int SHIFTstate=GetKeyState(16)&0xFFFE;			//the SHIFT key
	int CAPSstate=GetKeyState(20)&0x0001;			//the CAPS LOCK key
	if (CTRLstate) Flags|=0x01;
	if (SHIFTstate) Flags|=0x02;
	if (CAPSstate) Flags|=0x04;
	Flags|=0x08;									//setting the ALT key bit
	IsALTDown=1;									//to be used in expression.c
	if (SHIFTstate) IsSHIFTALTDown=1;				//to be used in expression.c
	else IsSHIFTDown=0;


	//check if the ALT+Space is used - select/deselect items
	/*if ((nChar==' ') && (KeyboardEntryObject==NULL))
	{
		SelectLastOrTouchedObject();
		return;
	}*/

	//determine if SmallCaps should be written (the nChar is always uppercase)
	if (SHIFTstate) SmallCaps=0;					
	if (CAPSstate) SmallCaps=!SmallCaps;

	//SPECIAL HANDLING - enabling menu shorcut keys (ALT+E, ALT+F, ALT+V and ALT+F4)
	//After  the ALT is pressed the VK_MENU code is first generated. This is detected here
	//and message is forwared to the Main Frame window.
	//This only works if every child window is disabled first - STRANGE INDEED!
	if (nChar==VK_F4)
	{
		theApp.m_pMainWnd->SendMessage(WM_CLOSE,0,0);
		return;
	}

	
	if ((MouseMode==0) && (SHIFTstate==0))
	if (((IsMenuAccessKey(nChar)) || (nChar==VK_F10))
		&& ((KeyboardEntryObject==NULL) || (KeyboardExponentMode==0)) && (CTRLstate==0) && (EnableMenuShortcuts))
	{
		//this part of code triggers the menu shortcuts

		Reenable=2;					   //trigger timmer that will ensure re-enabling the windows back (see OnTimer)
		this->EnableWindow(0);		   //disable the view window
		Toolbox->EnableWindow(0);	   //disable the toolbox window
		Toolbox->Keyboard->EnableWindow(0);  //disable the virtual keyboard window
		if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->EnableWindow(0);
		theApp.m_pMainWnd->SendMessage(WM_SYSCOMMAND,SC_KEYMENU,0x11); //activates the menu

		theApp.m_pMainWnd->SendMessage(WM_SYSKEYDOWN,nChar,nRepCnt|(nFlags<<16)); //notify the Main Frame window (see CMainFrm::OnSysKeyDown)
		CView::OnSysKeyDown(nChar,nRepCnt,nFlags); //must be (probably calls the DefWindowProc for the VK_MENU key)
		return;
	}


	//we proceed if any ALT+key combination is pressed - this is sent to SendKeyStroke

	//the following key code '0xBD' is returned when the '-' key is pressed wiht ALT (STRANGE key code  - is it the same on all keyboards?)
	//if (nChar==0xBD) nChar='-'; //special handling for the minus sign (with ALT must produce minus operator)
	
	if (KeyboardEntryObject)
	{
		//this code allows Space+Alt to be interpreted as Alt+Space (changing the math/text typing mode)
		CExpression *exp=(CExpression*)KeyboardEntryObject;
		if ((exp->m_NumElements!=1) || (exp->m_pElementList->Type!=1) || (exp->m_pElementList->pElementObject->Data1[0]!=0))
		if ((GetKeyState(VK_SPACE)&0xFFFE) && (nChar==VK_MENU))
			SendKeyStroke(' ',0,Flags);
	}

	if (nChar==VK_MENU) SendKeyStroke(0,0,Flags); //the ALT key
	else if (nChar==VK_SHIFT) SendKeyStroke(0,0,Flags); //the SHIFT key
	else if (!CTRLstate)
	{
		if (nChar==VK_RIGHT) SendKeyStroke(5,0,Flags);
		if (nChar==VK_LEFT) SendKeyStroke(4,0,Flags);
		if (nChar==VK_UP) SendKeyStroke(3,0,Flags);
		if (nChar==VK_DOWN) SendKeyStroke(2,0,Flags);

	}
	
	//we are re-enabling all windows and set focus to it - we are also calling the
	//OnMouseMove to repaint selections
	if ((MouseMode==0) && (EnableMenuShortcuts))
	{
		Toolbox->EnableWindow(1);
		Toolbox->Keyboard->EnableWindow(1);
		if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->EnableWindow(1);
		this->EnableWindow(1);
		this->SetFocus();
		CPoint pnt;
		GetCursorPos(&pnt);
		ScreenToClient(&pnt);
		UINT f=0;
		if (GetKeyState(VK_LBUTTON)&0xFFFE)  f=MK_LBUTTON;
		if (GetKeyState(VK_RBUTTON)&0xFFFE)  f=MK_RBUTTON;

		OnMouseMove(f,pnt);
	}
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}
#pragma optimize("",on)


void CMathomirView::OnHqRend()
{

	if (IsHighQualityRendering)
	{
		DisplayShortText("Edit mode",10,10,7007);
		IsHighQualityRendering=0;
	}
	else
	{
		DisplayShortText("Presentation mode",10,10,7008);
		IsHighQualityRendering=1;
	}
	
	AdjustMenu();
	if ((UseToolbar) && (Toolbox) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
	
	RepaintTheView(1);
}


int CMathomirView::AdjustMenu(void)
{
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	return 0;
}



void CMathomirView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (lockRepeat!=2)
		prevChar=0;
	IsSpacebarOnlyHit=0;
	IsALTDown=0;
	IsSHIFTALTDown=0;
	UINT Flags=0;
	int CTRLstate=GetKeyState(VK_CONTROL)&0xFFFE;  //the CTRL key
	int SHIFTstate=GetKeyState(16)&0xFFFE; //the SHIFT key
	int CAPSstate=GetKeyState(20)&0x0001; //the CAPS LOCK key
	if (CTRLstate) Flags|=0x01;
	if (SHIFTstate) Flags|=0x02;
	if (CAPSstate) Flags|=0x04;
	if (KeyboardEntryObject)
	{
		//received an character with ALT key pressed - no WM_CHAR will be generated

		if (nChar==18) SendKeyStroke(0,0,Flags); //the ALT key
		if (nChar==VK_SHIFT) SendKeyStroke(0,0,Flags); //the SHIFT key

	}
	else
	{
		if (nChar==18) Toolbox->KeyboardHit(0,Flags);
		CView::OnSysKeyUp(nChar, nRepCnt, nFlags);
	}
}

void CMathomirView::OnSysChar(UINT nChar,UINT nRepCnt, UINT nFlags)
{
	this->OnChar(nChar,nRepCnt,nFlags);
}

//function paints an document object to DC without flickering (because it first makes
//all the painting into memory bitmap)
CBitmap *GentlyPaintBitmap;
CDC *GentlyPaintBitmapDC;
int GentlyPaintBitmapX;
int GentlyPaintBitmapY;

int CMathomirView::GentlyPaintObject(tDocumentStruct * ds, CDC * DC)
{
 	int borderY=3+ViewZoom/256;
	int borderX=3+ViewZoom/256;
	int borderXsupplement=0;
	if ((ds->Type==1) && (ds->Object)) {borderXsupplement=((CExpression*)ds->Object)->m_MarginX/2;borderX+=borderXsupplement;}
	RECT rct; 
	rct.left=rct.top=0;
	rct.right=(ds->Length)*ViewZoom/100+2*borderX;
	rct.bottom=(ds->Above+ds->Below)*ViewZoom/100+2*borderY;

	RestoreClipboardBackground();

	CDC *bitmapDC;

	if ((GentlyPaintBitmap==NULL) || (GentlyPaintBitmapX<rct.right) || (GentlyPaintBitmapY<rct.bottom))
	{
		if (rct.right>GentlyPaintBitmapX) GentlyPaintBitmapX=rct.right;
		if (rct.bottom>GentlyPaintBitmapY) GentlyPaintBitmapY=rct.bottom;
		if (GentlyPaintBitmap)
		{
			//delete GentlyPaintBitmapDC;
			GentlyPaintBitmap->DeleteObject();
			delete GentlyPaintBitmap;
		}
		GentlyPaintBitmap=new CBitmap();
		GentlyPaintBitmap->CreateCompatibleBitmap(DC,GentlyPaintBitmapX,GentlyPaintBitmapY);
		if (GentlyPaintBitmapDC==NULL)
		{
			GentlyPaintBitmapDC=new CDC();
			GentlyPaintBitmapDC->CreateCompatibleDC(DC);
		}
		GentlyPaintBitmapDC->SelectObject(GentlyPaintBitmap);
	}
	bitmapDC=GentlyPaintBitmapDC;
	bitmapDC->FillSolidRect(&rct,RGB(255,255,255));


	//drawing page borders (if at the border)
	if (!ViewOnlyMode)
	if (ds->Type!=2)
	{
		int RelativeLeft=(15-ViewX)*ViewZoom/100+(ViewX-ds->absolute_X)*ViewZoom/100;
		int RelativeRight=(15-ViewX+PaperWidth)*ViewZoom/100+(ViewX-ds->absolute_X)*ViewZoom/100;
		int RelativeHeight=(PaperHeight)*ViewZoom/100;
		int RelativeStart=-(ViewY%PaperHeight)*ViewZoom/100+((ViewY-ds->absolute_Y)%PaperHeight)*ViewZoom/100+(ds->Above)*ViewZoom/100;

		if (RelativeLeft>0) bitmapDC->FillSolidRect(0,0,RelativeLeft+borderX,rct.bottom,DOCUMENT_AREA_BACKGROUND);
		if (RelativeRight<rct.right) bitmapDC->FillSolidRect(RelativeRight+borderX,0,(rct.right-RelativeRight)+2*borderX,rct.bottom,DOCUMENT_AREA_BACKGROUND);
		while (RelativeStart<rct.bottom)
		{
			bitmapDC->FillSolidRect(0,RelativeStart-5*ViewZoom/100+borderY,rct.right,10*ViewZoom/100,DOCUMENT_AREA_BACKGROUND);
			RelativeStart+=RelativeHeight;
		}
	}
	

	//drawing frame around selected objects
	if ((MouseMode!=9) && (MouseMode!=11)) //not stretching nor rotating
	if (ds->MovingDotState==3)
	{
		if (ds->Type==1)
			((CExpression*)(ds->Object))->SelectExpression(TemporaryShowColoredSelection?0:1);
		else if (ds->Type==2)
			((CDrawing*)(ds->Object))->SelectDrawing(TemporaryShowColoredSelection?0:2);
		
	}

	//paint the object
	int inY=ds->Above*ViewZoom/100;
	int x=(ds->absolute_X-ViewX)*ViewZoom/100-borderX;
	int y=(ds->absolute_Y-ViewY)*ViewZoom/100-inY-borderY;

	if (((MouseMode!=9) && (MouseMode!=11)) || (ds->MovingDotState!=3))
	{
		RECT tmpr=TheClientRect;
		tmpr.left=max(0,-x);
		tmpr.right=min(TheClientRect.right-x,rct.right);
		tmpr.top=max(0,-y);
		tmpr.bottom=min(TheClientRect.bottom-y,rct.bottom);

		if (ds->Type==1)
 			((CExpression*)(ds->Object))->PaintExpression(bitmapDC,ViewZoom,borderX,inY+borderY,&tmpr);
		else if (ds->Type==2)
			((CDrawing*)(ds->Object))->PaintDrawing(bitmapDC,-ViewZoom,borderX,inY+borderY,ds->absolute_X,ds->absolute_Y,&tmpr);

		//also paint the special drawing objects
		if ((SpecialDrawingHover) && (ds!=SpecialDrawingHover) &&
			(SpecialDrawingHover->absolute_X+SpecialDrawingHover->Length>=ds->absolute_X) &&
			(SpecialDrawingHover->absolute_X<=ds->absolute_X+ds->Length) &&
			(SpecialDrawingHover->absolute_Y+SpecialDrawingHover->Below>=ds->absolute_Y-ds->Above) &&
			(SpecialDrawingHover->absolute_Y<=ds->absolute_Y+ds->Below))
		{
			if (((CDrawing*)(SpecialDrawingHover->Object))->IsSpecialDrawing==50) //drawing box
			{
				short deltaX=(SpecialDrawingHover->absolute_X-ds->absolute_X)*ViewZoom/100;
				short deltaY=(SpecialDrawingHover->absolute_Y-ds->absolute_Y-ds->Above)*ViewZoom/100;
				((CDrawing*)(SpecialDrawingHover->Object))->PaintDrawing(bitmapDC,-ViewZoom,borderX+deltaX,2*inY+borderY+deltaY,SpecialDrawingHover->absolute_X,SpecialDrawingHover->absolute_Y);
			}
		}	
	}

	

	PaintDrawingHotspot(1);

	//painting the moving dot
	if (ds->Type==1)
	{
		int mds=max(MovingDotSize,MovingDotSize*(ViewZoom+420)/512);
		if ((ds->MovingDotState==2) /*|| (ds->MovingDotState==3)*/)
		{
			if ((MouseMode!=9) && (MouseMode!=11) && (MouseMode!=2) && (MouseMode!=102))
			{
				bitmapDC->FillSolidRect(borderX,(ds->Above+ds->Below)*ViewZoom/100-mds+2*borderY,mds,mds,RGB(0,0,0));
			}
		}
		else if ((ds->MovingDotState==1) || (MovingDotPermanent))
		{
			int xx=borderX;
			int yy=(ds->Above+ds->Below)*ViewZoom/100-mds+2*borderY;
			bitmapDC->FillSolidRect(xx+1,yy+1,mds-2,mds-2,RGB(176,176,176));
			bitmapDC->FillSolidRect(xx+2,yy+2,mds-4,mds-4,RGB(240,240,240));
		}
	}



	//bit-blt or transparent blt of the object to the window
	if ((ds->Type==2) && (ds->Object) && (((CDrawing*)(ds->Object))->IsSpecialDrawing!=51))
	{
		DC->TransparentBlt(x,y,rct.right,rct.bottom,bitmapDC,0,0,rct.right,rct.bottom,RGB(255,255,255));
	}
	else
	{
		DC->BitBlt(x,y,	rct.right,rct.bottom,bitmapDC,0,0,SRCCOPY);
	}

	if ((MouseMode!=9) && (MouseMode!=11))
	if (ds->MovingDotState==3)
	{
		int prop=DC->SetROP2(R2_MASKPEN);

		HBRUSH br=CreateSolidBrush(RGB(224,224,255));
		HPEN pn=GetPenFromPool(1,0,(ds->Type==1)?PALE_RGB(BLUE_COLOR):RGB(240,240,255));
		HBRUSH obr=(HBRUSH)DC->SelectObject(br);
		DC->SelectObject(pn);

		DC->Rectangle(x+borderXsupplement,y,x+rct.right-borderXsupplement,y+rct.bottom);
		DC->SelectObject(obr);
		DeleteObject(br);
		DC->SetROP2(prop);
	}
	
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(&cursor);
	if ((cursor.x>0) && (cursor.y>0))
		PaintClipboard(cursor.x,cursor.y);

	PaintDrawingHotspot();
	return 1;
}

#pragma optimize("s",on)
void CMathomirView::OnRButtonUp(UINT nFlags, CPoint point)
{
	SteadyCursorTimer=0;
	try
	{
	SetMousePointer();
	PreopenDrawingToolboxLock=0;

	if ((PreopenDrawingToolboxX) && (ScrollModeWasAccepted==0))
	{
		//we are going to open the handy popup drawing toolbox
		if (Toolbox->Subtoolbox)
		{
			if ((!(Toolbox->Subtoolbox->IsWindowVisible())) ||
				(Toolbox->Subtoolbox->m_IsSubtoolbox!=-4)) //if not already open
				OnKeyDown(VK_F4,0,0);
		}
		PreopenDrawingToolboxX=0;
	}


	if (MouseMode==1)  //right-click view scrolling
	{
		int anything=0;
		if (!ScrollModeWasAccepted)
		{
			//there was too little mouse movement to accept scrool mode - this was only
			//a right-click 

			EasycastListStart=0;
			//first check if we right-clicked at an object - open the context menu
			tDocumentStruct *ds;
			int AbsoluteX=StartAbsoluteX;
			int AbsoluteY=StartAbsoluteY;

			int i=-1;
			if (!ViewOnlyMode)
			if ((IsDrawingMode==0) || (GetKeyState(VK_CONTROL)&0xFFFE) || (MouseTouchingSelection))
				for (i=NumDocumentElements-1;i>=0;i--)
				{
			#ifdef TEACHER_VERSION
					if ((TheFileType=='r') && (DisableEditing)) {i=-1;break;}
			#endif
					ds=TheDocument+i;

					int ccc=1+100/ViewZoom;
					if ((AbsoluteY>=ds->absolute_Y-ds->Above) && (AbsoluteY<=ds->absolute_Y+ds->Below+ccc) &&
						(AbsoluteX>=ds->absolute_X) && (AbsoluteX<=ds->absolute_X+ds->Length) &&			
						((ds->MovingDotState!=5) || (AccessLockedObjects)))
					{
						//clicked at something, now examine this
						if (ds->MovingDotState==3) //right click at selection
						{
							Popup->ShowPopupMenu(NULL,(CWnd*)this,2,0);
							break;
						}
						else if (ds->Object)
						{
							if (ds->Type==1)
							{
								CExpression *selection;
								selection=((CExpression*)(ds->Object))->AdjustSelection();


								if (selection==NULL)
								{
									//handling for 'quick selection' (right click at insertion point)
									CExpression *obj=(CExpression*)(ds->Object);
									CDC *DC=this->GetDC();
									short is_expression;
									char is_parenthese;
									obj=(CExpression*)obj->SelectObjectAtPoint(DC,ViewZoom,(short)(point.x-(ds->absolute_X-ViewX)*ViewZoom/100),(short)(point.y-(ds->absolute_Y-ViewY)*ViewZoom/100),&is_expression,&is_parenthese);
									this->ReleaseDC(DC);

									if ((is_expression<=0) || (is_expression==0x7FFF)) obj=NULL;

									if ((obj) && ((obj->m_IsColumnInsertion) || (obj->m_IsRowInsertion)))
									{
										//right-click at column/row insertion point
										Popup->ShowPopupMenu(obj,(CWnd*)this,6,0);
										break;

									}

									if ((obj) && (obj->m_Selection) && (obj->m_Selection!=0x7FFF))
									{
										/*if (ClipboardExpression) 
										{
											//place-down with delete
											CDC *DC=this->GetDC();
											UndoSave("insert into",20303);
											int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
											int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;
											CExpression *dst=((CExpression*)(ds->Object))->CopyAtPoint(DC,ViewZoom,X,Y,ClipboardExpression);
											short l,a,b;
											((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
											char RepaintAll=0;
											l=(short)((int)l*100/(int)ViewZoom);
											a=(short)((int)a*100/(int)ViewZoom);
											b=(short)((int)b*100/(int)ViewZoom);

											if ((ds->Length>l) || (ds->Above>a) || (ds->Below>b)) RepaintAll=1;
											ds->Length=l;
											ds->Above=a;
											ds->Below=b;
											if (RepaintAll) 
												{InvalidateRect(NULL,0);UpdateWindow();} 
											else 
												GentlyPaintObject(ds, DC);
											this->ReleaseDC(DC);
										}
										else*/
										{
											//clicked at insertion point - we will select sourunding elements (quick-multitouch)
											int start=obj->m_Selection-1;
											int end=start;

											//determine the 'level'
											int level=MulLevel;
											if ((start==0) || (start==obj->m_NumElements)) level=0;
											if ((start>0) && ((obj->m_pElementList+start-1)->Type==2))
												level=min(GetOperatorLevel((obj->m_pElementList+start-1)->pElementObject->Data1[0]),level);
											if ((start<obj->m_NumElements) && ((obj->m_pElementList+start)->Type==2))
												level=min(GetOperatorLevel((obj->m_pElementList+start)->pElementObject->Data1[0]),level);

											while (start>0)
											{
												if (((obj->m_pElementList+start-1)->Type==11) || ((obj->m_pElementList+start-1)->Type==12)) 
													break;
												if (((obj->m_pElementList+start-1)->Type==2) &&
													(GetOperatorLevel((obj->m_pElementList+start-1)->pElementObject->Data1[0])<level))
													break;
												start--;
											}
											while (end<obj->m_NumElements)
											{
												if (((obj->m_pElementList+end)->Type==11) || ((obj->m_pElementList+end)->Type==12)) 
													break;
												if (((obj->m_pElementList+end)->Type==2) &&
													(GetOperatorLevel((obj->m_pElementList+end)->pElementObject->Data1[0])<level))
													break;
												end++;
											}						
											if (end!=start)
											{
												for (;start<end;start++)
												{
													if ((KeyboardEntryObject) && (KeyboardEntryObject==(CObject*)obj) && (start==obj->m_IsKeyboardEntry-1)) break;
													obj->SelectElement(1,start);
												}
												obj->m_Selection=0;
												CDC *DC=this->GetDC();
												GentlyPaintObject(ds,DC);
												this->ReleaseDC(DC);
												QuickSelectActive=1;
												selection=((CExpression*)(ds->Object))->AdjustSelection();

											}
											else
												obj->DeselectExpression();
										}
									}
								}

								int NumSelected=0;
								if (ds->Object==(CObject*)selection)
								{
									tDocumentStruct *ds2=TheDocument;
									for (int jj=0;jj<NumDocumentElements;jj++,ds2++)
									{
										if ((ds2->Type==1) && (((CExpression*)(ds2->Object))->m_Selection==0x7FFF)) NumSelected++;
										if ((ds2->Type==2) && (((CDrawing*)(ds2->Object))->IsSelected)) NumSelected++;
									}
								}
								if (NumSelected<2)
								{
									if (selection)
									{
										m_PopupMenuObject=ds;
										Popup->ShowPopupMenu(selection,(CWnd*)this,1,0);
										QuickSelectActive=0;
										break;
									}
								}
								else
								{
									Popup->ShowPopupMenu(NULL,(CWnd*)this,2,0);
									QuickSelectActive=0;
									break;
								}
								QuickSelectActive=0;
							}
							if ((ds->Type==2) && (ds->Object))
							{
								int NodeEdit;
								CDC *DC=this->GetDC();
								int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
								int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;
								CObject *obj=((CDrawing*)(ds->Object))->SelectObjectAtPoint(DC,ViewZoom,X,Y,&NodeEdit);
								this->ReleaseDC(DC);

								if (obj)
								{
									Popup->ShowPopupMenu(NULL,(CWnd*)this,2,0);
									break;
								}
							}
						}
					}
				}


			if ((i==-1) || ((IsDrawingMode) && (MouseTouchingSelection==0) && ((GetKeyState(VK_CONTROL)&0xFFFE)==0)))
			{
				//this is not a right click to an object, the context menu was not opened

				Toolbox->HideSubtoolbox();
				Popup->HidePopupMenu();
				if (ClipboardDrawing)
				{
					//ClipboardDrawing->Delete();
					delete ClipboardDrawing;
					ClipboardDrawing=NULL;
					SetCursor(::LoadCursor(NULL,IDC_ARROW));
					anything=1;
				}
				if (ClipboardExpression)
				{
					//ClipboardExpression->Delete();
					delete ClipboardExpression;
					ClipboardExpression=NULL;
					SetCursor(::LoadCursor(NULL,IDC_ARROW));
					anything=1;
				}
				else
				{
					//deleting multiple selection
					int i;
					tDocumentStruct *ds=TheDocument;
					int is_deselected=0;
					MultipleStartX=-1;
					for (i=0;i<NumDocumentElements;i++,ds++)
					{
						if ((ds->MovingDotState==3) || (ds->MovingDotState==4))
						{
							if (ds->Type==1) ((CExpression*)(ds->Object))->DeselectExpression();
							else if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(0);
							anything=1;
							is_deselected=1;
						}
						if (ds->MovingDotState!=5) ds->MovingDotState=0;
					}
					if (!is_deselected) 
					{
						if (IsDrawingMode)
							IsDrawingMode=0;
						else if (KeyboardEntryObject)
						{
							//finishing keyboard entry mode
							((CExpression*)KeyboardEntryObject)->KeyboardStop();
							CExpression *expr=(CExpression*)KeyboardEntryObject;
							if (expr->m_pPaternalExpression==NULL)
							if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
								if (KeyboardEntryBaseObject->Type==1)
									DeleteDocumentObject(KeyboardEntryBaseObject);
							KeyboardEntryObject=NULL;
							KeyboardEntryBaseObject=NULL;
							anything=1;
						}
					}
				}
			}
		}
		else
			anything=1;
		ScrollModeWasAccepted=0;
		RepaintTheView((anything)?0:1);
		MouseMode=0;
	}

	if (MouseMode==5) //multiple selection
	{
		MouseMode=0;
		MultipleStartX=MultipleStartY=-1;
		RepaintTheView();
	}


	if (MouseMode==11) //rotating
	{
		int oldpos=-1;
		for (int ii=0;ii<NumDocumentElements;ii++)
			if (TheDocument[ii].MovingDotState==3)
			{
				if (oldpos==-1) oldpos=ii;
				DeleteDocumentObject(TheDocument+ii);
				ii--;
			}
		MovingStartX=MovingStartY=0;
		CDC *DC=this->GetDC();

		short ww=0,hh=0;
		if (tmpDrawing2) 
		{
			tmpDrawing2->CalculateSize(DC,ViewZoom,&ww,&hh);
		}
		PasteDrawing(DC,(MultipleX-ViewX)*ViewZoom/100,(MultipleY-ViewY)*ViewZoom/100,(CObject*)tmpDrawing2,1);
		
		this->ReleaseDC(DC);
		MouseMode=0;
		if (tmpDrawing2)
		{
			delete tmpDrawing2;
			tmpDrawing2=NULL;
		}
		if (oldpos>=0) ChangeZOrderOfSelectedObjects(oldpos);

		RepaintTheView();
	}

	CView::OnRButtonUp(nFlags, point);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

#pragma optimize("s",on)
void CMathomirView::OnLButtonUp(UINT nFlags, CPoint point)
{
	char WasWindowOutOfFocus=IsWindowOutOfFocus;
	IsWindowOutOfFocus=0;
	SteadyCursorTimer=0;
	try{

	int AbsoluteX=ViewX+(int)point.x*100/(int)ViewZoom;
	int AbsoluteY=ViewY+(int)point.y*100/(int)ViewZoom;
	
	if ((Tillens.Show) && (Tillens.Enlarge!=-1))
	{
		Tillens.Enlarge=-1;
		RepaintTheView();
		return;
	}
	SetMousePointer();


	if (MouseMode==10)
	{
		MouseMode=0;
		SelectedTab=NULL;
		InvalidateRect(NULL,0);
		UpdateWindow();
		return;
	}
	
	if ((TouchMouseMode>0) && (TouchMouseMode<3)) //BABA2
	{
		if ((abs(AbsoluteX-MovingStartX)<=2) && (abs(AbsoluteY-MovingStartY)<=2))
		{
			//this was just a click
			
			TouchMouseMode=0;
			tDocumentStruct *ds;
			int ccc=1+100/ViewZoom;
			for (int i=NumDocumentElements-1;i>=0;i--)
			{
				ds=TheDocument+i;

				
				if (IsInsertionPointTouched!=ds)
				{
					if (ds->Type!=1) continue;
					if ((AbsoluteX<ds->absolute_X) || (AbsoluteX>ds->absolute_X+ds->Length) ||
						(AbsoluteY<ds->absolute_Y-ds->Above) || (AbsoluteY>ds->absolute_Y+ds->Below+ccc)) continue;
				}

				CExpression *selection=((CExpression*)ds->Object)->AdjustSelection();



				if ((ClipboardExpression) && (ds->Type==1) && (ds->MovingDotState!=5) && (!ViewOnlyMode) && (LeftClickTimer<4)) 
				{
					CDC *DC=this->GetDC();
					//if clipboard is non-empty, place down
					if ((ClipboardExpression->m_NumElements>1) || (ClipboardExpression->m_pElementList->Type))
					{
						UndoSave("insert into",20303);
						int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
						int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;
						CExpression *dst=((CExpression*)(ds->Object))->CopyAtPoint(DC,ViewZoom,X,Y,ClipboardExpression);
						short l,a,b;
						((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
						char RepaintAll=0;
						l=(short)((int)l*100/(int)ViewZoom);
						a=(short)((int)a*100/(int)ViewZoom);
						b=(short)((int)b*100/(int)ViewZoom);

						if ((ds->Length>l) || (ds->Above>a) || (ds->Below>b)) RepaintAll=1;
						ds->Length=l;
						ds->Above=a;
						ds->Below=b;
						if (RepaintAll) 
							{InvalidateRect(NULL,0);UpdateWindow();} 
						else 
							GentlyPaintObject(ds, DC);
						ClipboardExpression->AddToStackClipboard(0);
					}
					if (ClipboardExpression) {delete ClipboardExpression;ClipboardExpression=NULL;}
					SetCursor(::LoadCursor(NULL,IDC_ARROW));
					this->ReleaseDC(DC);
					InvalidateRect(NULL,0);
					UpdateWindow();
					return;
				}



			
				RootObjectCopy=0;
				if ((selection) && (!selection->DecodeInternalInsertionPoint()))
				{
						ClipboardExpression=new CExpression(NULL,NULL,100);
						ClipboardExpression->CopyExpression(selection,1);
						if (root_object_flag)
						{
							ClipboardExpression->m_InternalInsertionPoint=0;
							RootObjectCopy=ClipboardExpression->CalcChecksum();
							RootObjectFontSize=selection->m_FontSize; 
						}

						//special handling - when clicked on a HTML link element (we execute the link instead of copy)
						if ((ClipboardExpression->m_NumElements==1) && (ClipboardExpression->m_pElementList->Type==9))
						{
							CElement *elm=ClipboardExpression->m_pElementList->pElementObject;
							if ((elm->Expression2==0) && (elm->Expression3==0) && (elm->Data1[0]=='H') /*&& (*(char**)elm->Data3)*/)
							{
								if (*(char**)elm->Data3==NULL)
								{
									char str[512];
									str[0]=0;
									((CExpression*)elm->Expression1)->ConvertToPlainText(500,str);
									if (str[0])
									{
										ExecuteLink(str);
										//ShellExecute(NULL,NULL,(LPCSTR)str,NULL,NULL,SW_SHOWNORMAL);
										delete ClipboardExpression;
										ClipboardExpression=NULL;
									}
								}
								else
								{
									ExecuteLink(*(char**)elm->Data3);
									//ShellExecute(NULL,NULL,(LPCSTR)*(char**)elm->Data3,NULL,NULL,SW_SHOWNORMAL);
									delete ClipboardExpression;
									ClipboardExpression=NULL;
								}
							}
						}

						if (ClipboardExpression) SetCursor(theApp.LoadCursor(IDC_POINTER_COPY));
						InvalidateRect(NULL,0);
						UpdateWindow();
						return;
				}

				//nothing was selected, check if clicked in between elements
				short IsExpression;
				char IsParenthese;
				int X=point.x-(ds->absolute_X-ViewX)*ViewZoom/100;
				int Y=point.y-(ds->absolute_Y-ViewY)*ViewZoom/100;
				CObject *obj;
				CDC *DC=GetDC();
				obj=((CExpression*)(ds->Object))->SelectObjectAtPoint(DC,ViewZoom,X,Y,&IsExpression,&IsParenthese);
				if ((obj) && (!IsExpression) && (LeftClickTimer<4))
				{
					CElement *elm=(CElement*)obj;
					if ((elm) && (elm->m_Type==1) && (elm->m_Text) && (((CExpression*)elm->m_pPaternalExpression)->m_InternalInsertionPoint))
					{
						//clicked at text internal insertion point
						if (KeyboardEntryObject) 
						{
							CExpression *expr=(CExpression*)KeyboardEntryObject;
							expr->KeyboardStop();
							KeyboardEntryObject=NULL;
							if (expr->m_pPaternalExpression==NULL)
							if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
							if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
							{
								if (obj!=KeyboardEntryBaseObject->Object)
									DeleteDocumentObject(KeyboardEntryBaseObject);
							}
						}
						
						CExpression *selection=(CExpression*)elm->m_pPaternalExpression;
						KeyboardEntryBaseObject=ds;
						KeyboardEntryObject=(CObject*)selection;
						selection->KeyboardStart(DC,ViewZoom);

						short l,a,b;
						((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
						int XX,YY;
						((CExpression*)(ds->Object))->GetKeyboardCursorPos(&XX,&YY);
						XX+=(ds->absolute_X-ViewX)*ViewZoom/100;
						YY+=(ds->absolute_Y-ViewY)*ViewZoom/100;
						if (abs(XX-point.x)>20)
						{
							ViewX+=(XX-point.x)*64/ViewZoom;
							if (ViewX<0) ViewX=0;
						}

						InvalidateRect(NULL,0);
						UpdateWindow();
						return ;
					}
				}
				else if ((obj) && (IsExpression) && (IsExpression!=0x7FFF) && (LeftClickTimer<4))
				{
					//yes, clicked in between - start keyboard entry mode
					CExpression *prevKeyObj=NULL;
					if (KeyboardEntryObject)
					{
						//already active keyboard entry mode, switch it off
						CExpression *expr=(CExpression*)KeyboardEntryObject;
						prevKeyObj=expr;
						expr->KeyboardStop();

						KeyboardEntryObject=NULL;
						if (expr->m_pPaternalExpression==NULL)
						if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
						if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
						{
							if (obj!=KeyboardEntryBaseObject->Object)
								DeleteDocumentObject(KeyboardEntryBaseObject);
							RepaintTheView();
						}
						KeyboardEntryBaseObject=NULL;
						//RepaintTheView();
					}
					if ((((CExpression*)obj)->m_StartAsText) && (((CExpression*)obj)->m_NumElements==1) &&
						(((CExpression*)obj)->m_pElementList->Type==0))
						((CExpression*)obj)->m_Alignment=1;


					IsDrawingMode=0;
					KeyboardEntryObject=obj;
					KeyboardEntryBaseObject=ds;
					((CExpression*)(obj))->KeyboardStart(DC,ViewZoom);
					
					short l,a,b;
					((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
					int XX,YY;
					((CExpression*)(ds->Object))->GetKeyboardCursorPos(&XX,&YY);
					XX+=(ds->absolute_X-ViewX)*ViewZoom/100;
					YY+=(ds->absolute_Y-ViewY)*ViewZoom/100;
					if (abs(XX-point.x)>20)
					{
						ViewX+=(XX-point.x)*64/ViewZoom;
						if (ViewX<0) ViewX=0;
						InvalidateRect(NULL,0);
						UpdateWindow();
					}
					else if (prevKeyObj!=(CExpression*)KeyboardEntryObject)
					{
						InvalidateRect(NULL,0);
						UpdateWindow();
					}
				}
				else if (LeftClickTimer<4)
				{
					//it may be that it is clicked on the keyboard cursor itself - check it
					
					if (!WasWindowOutOfFocus)
					if ((ds->Object==KeyboardEntryObject) && (((CExpression*)ds->Object)->m_IsKeyboardEntry))
					{
						int cX,cY;
						if (((CExpression*)ds->Object)->GetKeyboardCursorPos(&cX,&cY))
						{
							int ActualSize=((CExpression*)ds->Object)->GetActualFontSize(ViewZoom);
							if ((X>=cX-ActualSize/5) && (X<cX+ActualSize/6) && (Y>cY-ActualSize/3) && (Y<cY+ActualSize/3))
							{
								CExpression *exp=((CExpression*)ds->Object);
								tElementStruct *ts=exp->m_pElementList+exp->m_IsKeyboardEntry-1;
								if ((ts->Type==1) && (exp->m_KeyboardCursorPos==0) && (ts->pElementObject->Data1[0]==0))
								{
									ts->pElementObject->m_Text=(ts->pElementObject->m_Text)?0:1;
									exp->m_ModeDefinedAt=exp->m_IsKeyboardEntry+((ts->pElementObject->m_Text)?(1<<14):0);
									Toolbox->PaintTextcontrolbox(Toolbox->GetDC());
									ReleaseDC(DC);
									return;
								}
							}
						}
					}
				}
				ReleaseDC(DC);
			}
		}
		else
		{
			TouchMouseMode=3;
			MovingStartX=AbsoluteX;
			MovingStartY=AbsoluteY;
		}
		
	}

	
	if ((ClipboardDrawing) && (MouseMode==7))
	{
		int cx=AbsoluteX-MovingStartX;
		int cy=AbsoluteY-MovingStartY;
		
		if ((SpecialDrawingHover)&&
			(((CDrawing*)SpecialDrawingHover->Object)->IsSpecialDrawing==50))
		{
			if (GetKeyState(VK_MENU)&0xFFFE)
			{
				//snap to point for drawing boxes
				int ppx=AbsoluteX;
				int ppy=AbsoluteY;
				if ((tmpDrawing) && (tmpDrawing->FindNerbyPoint(&ppx,&ppy,NULL,0,0,0,0)))
				{
					cx=ppx-MovingStartX;
					cy=ppy-MovingStartY;
				}
			}
		}
		else if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
		{
			cx=((cx+GRID/2)/GRID)*GRID;
			cy=((cy+GRID/2)/GRID)*GRID;
			int x1,y1;
			ClipboardDrawing->FindRealCorner(&x1,&y1);
			cx-=x1;
			cy-=y1;
		}
		
		int deltax=cx-(MovingStartX2-MovingStartX);
		int deltay=cy-(MovingStartY2-MovingStartY);

		int iii=0;
		for (int ii=0;ii<ClipboardDrawing->NumItems;ii++)
		{
			tDrawingItem *td=ClipboardDrawing->Items+ii;
			while ((iii<NumDocumentElements) && (TheDocument[iii].MovingDotState!=6)) iii++;
			int pos=iii;
			iii++;
			if ((pos>=0) && (pos<NumDocumentElements))
			{
				tDocumentStruct *ts=TheDocument+pos;
				if (ts->MovingDotState==6)
				{
					ts->MovingDotState=0;
					ts->absolute_X+=deltax;
					ts->absolute_Y+=deltay;
				}
			}
		}
		//moving drawing
		/*CDC *DC=this->GetDC();
		PasteDrawing(DC,point.x,point.y);

		this->ReleaseDC(DC);*/
		//ClipboardDrawing->Delete();
		delete ClipboardDrawing;
		ClipboardDrawing=NULL;
		SetCursor(::LoadCursor(NULL,IDC_ARROW));
		RepaintTheView();
		MouseMode=0;
	}
	/*else if (moving_start_timer)
	{
		moving_start_timer=0;
		for (int i=0;i<NumDocumentElements;i++)
			if (TheDocument[i].MovingDotState==4) TheDocument[i].MovingDotState=0;
	}*/
	
	if (MouseMode==2) //moving objects
	{
		if (MovingFirstMove)
		{
			if ((SelectedDocumentObject) && (SelectedDocumentObject->MovingDotState==2))
				if ((SelectedDocumentObject->Type==1) && 
					((((CExpression*)SelectedDocumentObject->Object)->m_NumElements>1) || 
					(((CExpression*)SelectedDocumentObject->Object)->m_pElementList->Type)))
			{
				if (ClipboardExpression) delete ClipboardExpression;
				ClipboardExpression=new CExpression(NULL,NULL,100);
				ClipboardExpression->CopyExpression(((CExpression*)SelectedDocumentObject->Object),1);
				ClipboardExpression->m_InternalInsertionPoint=0;
				RootObjectCopy=ClipboardExpression->CalcChecksum();
				RootObjectFontSize=((CExpression*)(SelectedDocumentObject->Object))->m_FontSize;
			}
		}

		GuidlineElement=-1;
		prevGuidlineX=0x7FFFFFFF; //supressing guidline background refreshing
		MouseMode=0;
		InvalidateRect(NULL);
		UpdateWindow();
	}

	if (MouseMode==9) //stretching
	{
		if (SelectedHandle==5)
		{
			RepaintTheView();
			MouseMode=0;
			SelectedHandle=0;
			RepaintTheView();
		}
		else
		{
			int oldpos=-1;
			for (int ii=0;ii<NumDocumentElements;ii++)
				if (TheDocument[ii].MovingDotState==3)
				{
					if (oldpos==-1) oldpos=ii;
					DeleteDocumentObject(TheDocument+ii);
					ii--;
				}
			int minx=SelectionRect.left,maxx=SelectionRect.right;
			if (maxx<minx) {minx=SelectionRect.right;maxx=SelectionRect.left;}
			int miny=SelectionRect.top,maxy=SelectionRect.bottom;
			if (maxy<miny) {miny=SelectionRect.bottom;maxy=SelectionRect.top;}
			MovingStartX=MovingStartY=0;
			CDC *DC=this->GetDC();
			PasteDrawing(DC,(minx-ViewX)*ViewZoom/100,(miny-ViewY)*ViewZoom/100,(CObject*)tmpDrawing2,1);
			this->ReleaseDC(DC);
			MouseMode=0;
			if (tmpDrawing2)
			{
				//tmpDrawing2->Delete();
				delete tmpDrawing2;
				tmpDrawing2=NULL;
			}
			if (oldpos>=0) ChangeZOrderOfSelectedObjects(oldpos);
			RepaintTheView();
		}
	}


	
	if (MouseMode==6) //drawing mode
	{
		if (tmpDrawing)
		{
			int AbsoluteX=ViewX*10+(int)point.x*1000/(int)ViewZoom;
			int AbsoluteY=ViewY*10+(int)point.y*1000/(int)ViewZoom;

			int x,y;
			short w,h;
			CDC *DC=this->GetDC();
			if ((IsDrawingMode==3) || (IsDrawingMode==14) || (IsDrawingMode==18) ||  //freehand, freehand arrow
				(((IsDrawingMode==6) || (IsDrawingMode==7)) && (LeftClickTimer>3)) ||    //erasers
				(IsDrawingMode==20) || (IsDrawingMode==21) || (IsDrawingMode==22) || (IsDrawingMode==23)) //check mark, exclamation mark, question mark, star
			{
				if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
				{
					AbsoluteX/=10;AbsoluteY/=10;
					AbsoluteX=((AbsoluteX+GRID/2)/GRID)*GRID;
					AbsoluteY=((AbsoluteY+GRID/2)/GRID)*GRID;
					AbsoluteX*=10;AbsoluteY*=10;
				}
				if ((prevXX!=AbsoluteX) || (prevYY!=AbsoluteY))
				{
					tmpDrawing->UpdateCreatingItem(AbsoluteX-MovingStartX,AbsoluteY-MovingStartY,AbsoluteX,AbsoluteY);
					//RepaintTheView();
					tmpDrawing->PaintDrawing(DC,ViewZoom,(MovingStartX/10-ViewX)*ViewZoom/100,(MovingStartY/10-ViewY)*ViewZoom/100,-1,-1);
					prevXX=AbsoluteX;
					prevYY=AbsoluteY;
				}
			}
			int retval=tmpDrawing->EndCreatingItem(&x,&y,(MovingStartX/10-ViewX),(MovingStartY/10-ViewY));
			if ((IsDrawingMode!=6) && (IsDrawingMode!=7) && (retval))  //not eraser
			{
				tmpDrawing->CalculateSize(DC,ViewZoom,&w,&h);

				UndoSave("drawing",20409);

				AddDocumentObject(2,MovingStartX/10+x,MovingStartY/10+y);

				tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
				LastDrawingCreated=NumDocumentElements-1;
				ds->Above=0;
				ds->Below=h;
				ds->Length=w;
				ds->MovingDotState=0;
				ds->Object=(CObject*)tmpDrawing;
				if ((tmpDrawing) && (tmpDrawing->IsSpecialDrawing)) IsDrawingMode=0;
				tmpDrawing=NULL;
			}

			this->ReleaseDC(DC);
		}
		MouseMode=0;
		RepaintTheView();
	}

	if (MouseMode==4) //spacing mode
	{
		int absy=AbsoluteY;
		if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
		{
			absy=((absy+GRID/2)/GRID)*GRID;
			startSpacingY=((startSpacingY+GRID/2)/GRID)*GRID;
		}
		if (startSpacingY>absy)
		{
			UndoSave("decrease spacing",20410);
			int delta=startSpacingY-absy;
			int i;
			for (i=0;i<NumDocumentElements;i++)
			{
				if (TheDocument[i].absolute_Y>=startSpacingY) TheDocument[i].absolute_Y-=delta;
			}
		}
		if (startSpacingY<absy)
		{
			UndoSave("increase spacing",20411);
			int delta=absy-startSpacingY;
			int i;
			for (i=0;i<NumDocumentElements;i++)
			{
				if (TheDocument[i].absolute_Y>=startSpacingY) TheDocument[i].absolute_Y+=delta;
			}
		}
		RepaintTheView();
		MouseMode=0;
	}

	if (MouseMode==0)
	{
		if ((SpecialDrawingHover) && (SpecialDrawingHover->Object) && (SpecialDrawingHover->Type==2))
		{
			((CDrawing*)(SpecialDrawingHover->Object))->MouseClick(0x7FFFFFFF,0x7FFFFFFF);
		}
	}

	if (MouseMode==5) //multiple selection mode
	{
		//if anything is selected, then we will deselct
		if (MultipleModeWasAccepted==1)
		{
			int i;
			for (i=0;i<NumDocumentElements;i++)
				if (TheDocument[i].MovingDotState==3)
				{
					TheDocument[i].MovingDotState=0;
					if ((TheDocument[i].Type==1) && (TheDocument[i].Object))
						((CExpression*)(TheDocument[i].Object))->DeselectExpression();
					if ((TheDocument[i].Type==2) && (TheDocument[i].Object))
						((CDrawing*)(TheDocument[i].Object))->SelectDrawing(0);
					MultipleModeWasAccepted=0;
				}
		}
		if ((MultipleModeWasAccepted==1) && (SpecialDrawingHover==NULL) && (LeftClickTimer<4))
		{
			//clicked on empty screen, we are adding new empty expression (or from clipboard)
			int make_text=0;
			if (GuidlineElement>=0)
			{
				AbsoluteX=prevGuidlineX;
				make_text=SelectedGuidelineType;
			}
			else if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
			{
				AbsoluteX=((AbsoluteX+GRID/2)/GRID)*GRID-3;
				AbsoluteY=((AbsoluteY+GRID/2)/GRID)*GRID;
			}			
			if (!IsDrawingMode)
			{
				if ((NewlineAddObject>=0) && ((NewlineAddObject&0x3FFFFFFF)<NumDocumentElements) && (TheDocument[NewlineAddObject&0x3FFFFFFF].Type==1))
				{
					int mark_for_redraw=0;
					if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
					{
						mark_for_redraw=1;
						CExpression *ee=(CExpression*)KeyboardEntryObject;
						if ((KeyboardEntryBaseObject->Object==KeyboardEntryObject) && 
							(ee->m_NumElements==1) &&
							((ee->m_pElementList->Type==0) || ((ee->m_pElementList->Type==1) && (ee->m_pElementList->pElementObject->Data1[0]==0))))
						{
							tDocumentStruct *ds=&TheDocument[NewlineAddObject&0x3FFFFFFF];
							if (ds>KeyboardEntryBaseObject) NewlineAddObject--; //shuld never happen
							DeleteDocumentObject(KeyboardEntryBaseObject);
							KeyboardEntryBaseObject=NULL;
							KeyboardEntryObject=NULL;
						}
						else
							((CExpression*)KeyboardEntryObject)->KeyboardStop();
					}

					//clicked just below a multiline or text object - we will append a new line to that object
					CExpression *e=(CExpression*)TheDocument[NewlineAddObject&0x3FFFFFFF].Object;
					int last_obj=e->m_NumElements-1;
					if (((e->m_pElementList+last_obj)->Type!=2) || ((e->m_pElementList+last_obj)->pElementObject->Data1[0]!=(char)0xFF))
					{
						//we will add line wrap to this object
						e->InsertEmptyElement(e->m_NumElements,2,(char)0xFF);
					}
					e->InsertEmptyElement(e->m_NumElements,1,0);
					if (NewlineAddObject&0x40000000) (e->m_pElementList+e->m_NumElements-1)->pElementObject->m_Text=1;
					e->m_IsKeyboardEntry=e->m_NumElements;
					e->m_Alignment=1;
					KeyboardEntryObject=(CObject*)e;
					KeyboardEntryBaseObject=&TheDocument[NewlineAddObject&0x3FFFFFFF];
					/*short l,a,b;
					CDC *DC=this->GetDC();
					e->CalculateSize(DC,ViewZoom,&l,&a,&b);
					
					this->GentlyPaintObject(KeyboardEntryBaseObject,DC);
					this->ReleaseDC(DC);*/
					NewlineAddObject=-1;
					if (mark_for_redraw) this->RepaintTheView();
				}
				else 
				{
					StartKeyboardEntryAt(AbsoluteX,AbsoluteY,make_text);
					if (KeyboardEntryObject) SetMousePointer();
				}
			}

		}
		else if (MultipleModeWasAccepted==2) //kokana
		{
			if (MakeQuickDrawing) //quick painting of curly brackets and section lines
			{
				int w=QuickDrawingWidth>>16;
				int width=QuickDrawingWidth&0xFFFF;
				if (width>32768) width=width|0xFFFF0000;
				CDrawing *drw=new CDrawing();
				if ((drw) && (w>0) && (w<40) && (width>-20000) && (width<20000))
				{
					//if (width*100/ViewZoom>abs(len*100/ViewZoom)/16+5) width=(abs(len*100/ViewZoom)/16+5)*ViewZoom/100;
					//if (width*100/ViewZoom<-abs(len*100/ViewZoom)/16-5) width=(-abs(len*100/ViewZoom)/16-5)*ViewZoom/100;

					

					width=width*100/ViewZoom;
					drw->StartCreatingItem((MakeQuickDrawing%2)?18:19);
					int x1=(MultipleStartX-ViewX);
					int y1=(MultipleStartY-ViewY);
					int x2=(MakeQuickDrawing<=2)?((MultipleX-ViewX)):((MultipleStartX+width-ViewX));
					int y2=(MakeQuickDrawing<=2)?((MultipleStartY+width-ViewY)):((MultipleY-ViewY));

					char is_vert=0;
					int len=abs(x1-x2);if (abs(y2-y1)>len) {len=abs(y2-y1);is_vert=1;}
					if (width>abs(len)/16+5) width=(abs(len)/16+5);
					if (width<-abs(len)/16-5) width=(-abs(len)/16-5);


					drw->UpdateCreatingItem(0,0,MultipleStartX,MultipleStartY);
					//drw->UpdateCreatingItem((x2-x1)*10,(y2-y1)*10,MultipleX,MultipleY);
					if (is_vert)
						drw->UpdateCreatingItem(width*10,(y2-y1)*10,MultipleX,MultipleY);
					else
						drw->UpdateCreatingItem((x2-x1)*10,width*10,MultipleX,MultipleY);
					drw->EndCreatingItem(&x1,&y1);
					AddDocumentObject(2,MultipleStartX+x1,MultipleStartY+y1);
					TheDocument[NumDocumentElements-1].Object=(CObject*)drw;
					drw->SetLineWidth(w*DRWZOOM/4);
					drw->SetColor(0);
				}
				
				MakeQuickDrawing=0;
			}

			//selecting
			MouseMode=0;
			MultipleStartX=MultipleStartY=-1;
			RepaintTheView();
		}
		else
		{
			//deselecting an existing selection
			MouseMode=0;
			MultipleStartX=MultipleStartY=-1;
			RepaintTheView();
		}
	}
	CView::OnLButtonUp(nFlags, point);
	}
	catch(...)
	{
		FatalErrorHandling();
	} 
}

#pragma optimize("s",on)
int CMathomirView::PopupCloses(int UserParam, int ExitCode)
{
	for (int ii=0;ii<NumDocumentElements;ii++)
		if ((TheDocument[ii].Type==2) && (TheDocument[ii].Object) && 
			(((CDrawing*)(TheDocument[ii].Object))->IsSelected))
			((CDrawing*)(TheDocument[ii].Object))->SelectDrawing(0);

	//popup menu has been closed, so repaint the entire screen, and recalculate sizes
	if ((m_PopupMenuObject) && (m_PopupMenuObject>=TheDocument) && (m_PopupMenuObject-TheDocument<NumDocumentElements))
	{
		if ((ExitCode==1) || (ExitCode==2)) //DEL or CUT
		{
			if (m_PopupMenuObject->Type==1)
			{
				//check if the object is entriely empty. If yes, delete the object
				CExpression *expr=(CExpression*)(m_PopupMenuObject->Object);
				if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
				{
					DeleteDocumentObject(m_PopupMenuObject);
					m_PopupMenuObject=NULL;
					RepaintTheView();
				}
			}
		}

		if (m_PopupMenuObject)
		{
			CDC *DC;
			DC=GetDC();
			short l,a=0,b=32767;
			if (m_PopupMenuObject->Type==1)
				((CExpression*)(m_PopupMenuObject->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
			else if (m_PopupMenuObject->Type==2)
				((CDrawing*)(m_PopupMenuObject->Object))->CalculateSize(DC,ViewZoom,&l,&b);
			if (b!=32767)
			{
			m_PopupMenuObject->Above=a*100/ViewZoom;
			m_PopupMenuObject->Below=b*100/ViewZoom;
			m_PopupMenuObject->Length=l*100/ViewZoom;
			}
			ReleaseDC(DC);
		}
	}

	InvalidateRect(NULL);
	UpdateWindow();

	if (KeyboardEntryObject)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
		SetCursor(hc);
	}
	else if ((ClipboardExpression) || (ClipboardDrawing))
	{
		HCURSOR hc=theApp.LoadCursor(IDC_POINTER_COPY);
		SetCursor(hc);
	}
	else
	{
		HCURSOR hc=::LoadCursor(NULL,IDC_ARROW);
		SetCursor(hc);
	}
	return 0;
}

/*int CMathomirView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return CView::OnMouseActivate(pDesktopWnd, nHitTest, message);
}*/

/*void CMathomirView::OnSelectionsFrame()
{
	FrameSelections=0x01;
	AdjustMenu();
	RepaintTheView();

}

void CMathomirView::OnSelectionsUnderline()
{
	FrameSelections=0x03;
	AdjustMenu();
	RepaintTheView();

}*/

/*void CMathomirView::OnSelectionsNone()
{
	FrameSelections=0x00;
	AdjustMenu();
	RepaintTheView();

}*/

void CMathomirView::OnSelectionsIntelligentframing()
{
	if (FrameSelections) FrameSelections=0; else FrameSelections=1;
	AdjustMenu();
	RepaintTheView();
}

void CMathomirView::OnKeyboardFixfontfornumbers()
{
	if (FixFontForNumbers)
		FixFontForNumbers=0;
	else
		FixFontForNumbers=1;
	AdjustMenu();
}

/*void CMathomirView::OnKeyboardAltforexponents()
{
	if (UseALTForExponents)
		UseALTForExponents=0;
	else
		UseALTForExponents=1;
	AdjustMenu();
}*/

/*int CMathomirView::CheckDocumentMemoryReservations(void)
{
	if (NumDocumentElements>=NumDocumentElementsReserved)
	{
		if (NumDocumentElementsReserved==0)
		{
			NumDocumentElementsReserved+=20;
			TheDocument=(tDocumentStruct*)malloc(NumDocumentElementsReserved*sizeof(tDocumentStruct));
		}
		else
		{
			NumDocumentElementsReserved+=50;
			TheDocument=(tDocumentStruct*)realloc((void*)TheDocument,NumDocumentElementsReserved*sizeof(tDocumentStruct));
			if (SpecialDrawingHover) SpecialDrawingHover=NULL;
			if (prevSpecialDrawingHover) prevSpecialDrawingHover=NULL;
		}
		if (TheDocument==NULL) {AfxMessageBox("Cannot reserve document memory!!!",MB_OK,NULL);return 0;}
	}
	return 0;
}*/
#pragma optimize("",on)


int CMathomirView::PaintClipboard(int X, int Y)
{
	
	X=ViewX+X*100/ViewZoom;
	Y=ViewY+Y*100/ViewZoom;

	X=(X-ViewX)*ViewZoom/100;
	Y=(Y-ViewY)*ViewZoom/100;

	// nothing to paint
	if ((ClipboardExpression==NULL) && (ClipboardDrawing==NULL))
	{
		RestoreClipboardBackground();
		return 0;
	}
	if (ClipboardExpression)
		ClipboardExpression->SelectExpression(3);
	else if (ClipboardDrawing)
		ClipboardDrawing->SelectDrawing(1);

	CDC *DC=GetDC();

	//first, check if ClipboardBackground image is already created
	if (ClipboardBackground.bmp==NULL)
	{
		ClipboardBackground.bmp=new CBitmap();
		ClipboardBackground.bmp->CreateCompatibleBitmap(DC,1,1);
		ClipboardBackground.DC=new CDC();
		ClipboardBackground.DC->CreateCompatibleDC(DC);
		ClipboardBackground.DC->SelectObject(ClipboardBackground.bmp);
		ClipboardBackground.Restored=1;
	}

	//now, check the size of the clipboard image
	short l=0,a=0,b=0;
	if (ClipboardExpression)
	{
		ClipboardBackground.ZoomLevel=min(80,ViewZoom);
		ClipboardExpression->CalculateSize(DC,ClipboardBackground.ZoomLevel,&l,&a,&b);
		if ((l>300) || (a+b>240))
		{
			ClipboardBackground.ZoomLevel=min(50,ViewZoom);
			ClipboardExpression->CalculateSize(DC,ClipboardBackground.ZoomLevel,&l,&a,&b);
		}
	}
	else
	{
		ClipboardBackground.ZoomLevel=ViewZoom;
		ClipboardDrawing->CalculateSize(DC,ClipboardBackground.ZoomLevel,&l,&b);
	}


	if ((l!=ClipboardBackground.Cx-10) || (a+b!=ClipboardBackground.Cy-10))
	{
		RestoreClipboardBackground();

		//the size differs, create new clipboard bitmap image
		ClipboardBackground.DC->DeleteDC();
		ClipboardBackground.bmp->DeleteObject();
		delete ClipboardBackground.bmp;
		delete ClipboardBackground.DC;

		ClipboardBackground.Cx=l+10;
		ClipboardBackground.Cy=a+b+10;

		ClipboardBackground.bmp=new CBitmap();
		ClipboardBackground.bmp->CreateCompatibleBitmap(DC,ClipboardBackground.Cx,ClipboardBackground.Cy);
		ClipboardBackground.DC=new CDC();
		ClipboardBackground.DC->CreateCompatibleDC(DC);
		ClipboardBackground.DC->SelectObject(ClipboardBackground.bmp);
	}

	int newx=X+5;
	int newy=Y+10;
	if (ClipboardDrawing)
	{
		newx=X-MovingStartX*ViewZoom/100;
		newy=Y-MovingStartY*ViewZoom/100;

		if ((SpecialDrawingHover==NULL) || (((CDrawing*)SpecialDrawingHover->Object)->IsSpecialDrawing!=50)) //note: we dont use snap to grid inside drawing box
		if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
		{
			int X=ViewX+newx*100/ViewZoom;
			int Y=ViewY+newy*100/ViewZoom;
		
			int x1,y1;
			ClipboardDrawing->FindRealCorner(&x1,&y1);
			X+=x1;
			Y+=y1;
			X=((X+GRID/2)/GRID)*GRID;
			Y=((Y+GRID/2)/GRID)*GRID;

			newx=(X-x1-ViewX)*ViewZoom/100;
			newy=(Y-y1-ViewY)*ViewZoom/100;
		}
		newx-=5;
		newy-=5;
	}
PaintDrawingHotspot(1);
	if ((abs(newx-ClipboardBackground.X)<10) && (abs(newy-ClipboardBackground.Y)<10))
	{
		CBitmap tmpbmp;
		CDC tmpdc;
		tmpbmp.CreateCompatibleBitmap(DC,ClipboardBackground.Cx+20,ClipboardBackground.Cy+20);
		tmpdc.CreateCompatibleDC(DC);
		tmpdc.SelectObject(tmpbmp);
		tmpdc.BitBlt(0,0,ClipboardBackground.Cx+20,ClipboardBackground.Cy+20,DC,ClipboardBackground.X-10,ClipboardBackground.Y-10,SRCCOPY);
		if (!ClipboardBackground.Restored)
			tmpdc.BitBlt(10,10,ClipboardBackground.Cx,ClipboardBackground.Cy,ClipboardBackground.DC,0,0,SRCCOPY);

		ClipboardBackground.DC->BitBlt(0,0,ClipboardBackground.Cx,ClipboardBackground.Cy,&tmpdc,10+newx-ClipboardBackground.X,10+newy-ClipboardBackground.Y,SRCCOPY);

		//now paint into the background image
		if (ClipboardExpression)
			ClipboardExpression->PaintExpression(&tmpdc,ClipboardBackground.ZoomLevel,10+newx-ClipboardBackground.X+5,10+newy-ClipboardBackground.Y+a+5,0);
		else if (ClipboardDrawing)
			ClipboardDrawing->PaintDrawing(&tmpdc,ClipboardBackground.ZoomLevel,10+newx-ClipboardBackground.X+5,10+newy-ClipboardBackground.Y+a+5,-1,-1,0);
		ClipboardBackground.Restored=0;

		DC->BitBlt(ClipboardBackground.X-10,ClipboardBackground.Y-10,ClipboardBackground.Cx+20,ClipboardBackground.Cy+20,&tmpdc,0,0,SRCCOPY);
		ClipboardBackground.X=newx;
		ClipboardBackground.Y=newy;
	}
	else
	{
		RestoreClipboardBackground();

		//get background image
		ClipboardBackground.X=newx;
		ClipboardBackground.Y=newy;
		ClipboardBackground.DC->BitBlt(0,0,ClipboardBackground.Cx,ClipboardBackground.Cy,DC,ClipboardBackground.X,ClipboardBackground.Y,SRCCOPY);

		//paint Clipboard
		if (ClipboardExpression)
			ClipboardExpression->PaintExpression(DC,ClipboardBackground.ZoomLevel,ClipboardBackground.X+5,ClipboardBackground.Y+a+5);
		else if (ClipboardDrawing)
			ClipboardDrawing->PaintDrawing(DC,ClipboardBackground.ZoomLevel,ClipboardBackground.X+5,ClipboardBackground.Y+a+5,-1,-1);
		ClipboardBackground.Restored=0;
	}
	ReleaseDC(DC);
PaintDrawingHotspot();
	return 0;
}

int CMathomirView::RestoreClipboardBackground(void)
{
	if (ClipboardBackground.Restored) return 0;
	if (ClipboardBackground.bmp==NULL) return 0;
	CDC *DC=GetDC();

	DC->BitBlt(ClipboardBackground.X,ClipboardBackground.Y,ClipboardBackground.Cx,ClipboardBackground.Cy,ClipboardBackground.DC,0,0,SRCCOPY);
	ClipboardBackground.Restored=1;

	ReleaseDC(DC);
	return 0;
}

int CMathomirView::DeleteDocumentObject(tDocumentStruct* object)
{
	int i;

	LastQuickTypeObject=NULL;

	for (i=0;i<NumDocumentElements;i++)
		if (TheDocument+i==object)
		{
			if ((KeyboardEntryObject) && (KeyboardEntryBaseObject==object))
			{
				//we are suspending keyboard entry mode
				KeyboardEntryBaseObject=NULL;
				KeyboardEntryObject=NULL;
			}
			if ((KeyboardEntryObject) && (KeyboardEntryBaseObject>object))
			{
				KeyboardEntryBaseObject--;
			}

			if (prevTouchedObject==object) prevTouchedObject=NULL;
			if (prevTouchedObject>object) prevTouchedObject--;
			if (SelectedDocumentObject==object) SelectedDocumentObject=NULL;
			if (SelectedDocumentObject>object) SelectedDocumentObject--;
			if (SelectedDocumentObject2==object) SelectedDocumentObject2=NULL;
			if (SelectedDocumentObject2>object) SelectedDocumentObject2--;
			if (SpecialDrawingHover==object) SpecialDrawingHover=NULL;
			if (SpecialDrawingHover>object) SpecialDrawingHover--;
			if (prevSpecialDrawingHover==object) prevSpecialDrawingHover=NULL;
			if (prevSpecialDrawingHover>object) prevSpecialDrawingHover--;
			if (m_PopupMenuObject==object) m_PopupMenuObject=NULL;
			if (m_PopupMenuObject>object) m_PopupMenuObject--;
			LongClickObject=NULL;

			if (object->Type==1)
			{
				//((CExpression*)(object->Object))->Delete();
				delete ((CExpression*)(object->Object));
			}
			else if (object->Type==2)
			{
				//((CDrawing*)(object->Object))->Delete();
				delete ((CDrawing*)(object->Object));
			}

			//int j;
			//for (j=i;j<NumDocumentElements-1;j++)
			//	TheDocument[j]=TheDocument[j+1];
			memmove(TheDocument+i,TheDocument+i+1,(NumDocumentElements-i-1)*sizeof(tDocumentStruct));
			NumDocumentElements--;
			break;
		}
	return 0;
}

void CMathomirView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	PagePrinted=pInfo->m_nCurPage;

	CView::OnPrint(pDC, pInfo);
}

BOOL CMathomirView::OnPreparePrinting(CPrintInfo* pInfo)
{
	int MaxY=1;
	int i;
	for (i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if (ds->absolute_Y>MaxY) MaxY=ds->absolute_Y+6;
	}
	MaxPagePrinted=MaxY/PaperHeight+1;
	pInfo->SetMaxPage(MaxPagePrinted);
	

	// default preparation
	return DoPreparePrinting(pInfo);
	//return CView::OnPreparePrinting(pInfo);
}

void CMathomirView::OnViewHalftonerendering()
{
	if (IsHalftoneRendering)
		IsHalftoneRendering=0;
	else
		IsHalftoneRendering=1;

	AdjustMenu();
	RepaintTheView(1);
}

void CMathomirView::OnPageA4portrait()
{
	PaperHeight=1650;//3*1100/2;
	PaperWidth=1165;//3*777/2;
	AdjustMenu();
	RepaintTheView();
}

void CMathomirView::OnPageA4landscape()
{
	PaperHeight=1165;//3*777/2;
	PaperWidth=1650;//3*1100/2;
	AdjustMenu();
	RepaintTheView();
}
void CMathomirView::OnPageLetterportrait()
{
	
	PaperHeight=1553;//3*1100/2;
	PaperWidth=1200;//3*777/2;
	AdjustMenu();
	RepaintTheView();
}

void CMathomirView::OnPageLetterlandscape()
{
	PaperHeight=1200;//3*1100/2;
	PaperWidth=1553;//3*777/2;
	AdjustMenu();
	RepaintTheView();
}


extern int BaseToolboxSize;

void CMathomirView::OnToolboxLarge()
{
	BaseToolboxSize=ToolboxSize=102;
	AutoResizeToolbox=0;
	Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
	Toolbox->AdjustPosition();
	Toolbox->InvalidateRect(NULL,0);
	Toolbox->UpdateWindow();
	AdjustPosition();
	AdjustMenu();
	RepaintTheView();
	theApp.m_pMainWnd->SetActiveWindow();
}

void CMathomirView::OnToolboxMedium()
{
	BaseToolboxSize=ToolboxSize=84;
	AutoResizeToolbox=0;
	Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
	Toolbox->AdjustPosition();
	Toolbox->InvalidateRect(NULL,0);
	Toolbox->UpdateWindow();
	AdjustPosition();
	AdjustMenu();
	RepaintTheView();
	theApp.m_pMainWnd->SetActiveWindow();
}

void CMathomirView::OnToolboxSmall()
{
	BaseToolboxSize=ToolboxSize=60;
	AutoResizeToolbox=0;
	Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
	Toolbox->AdjustPosition();
	Toolbox->InvalidateRect(NULL,0);
	Toolbox->UpdateWindow();
	AdjustPosition();
	AdjustMenu();
	RepaintTheView();
	theApp.m_pMainWnd->SetActiveWindow();
}

void CMathomirView::OnSaveoptionsSaveasdefault()
{
	Toolbox->SaveSettings(NULL);
	char str[120];
	CopyTranslatedString(str,"Settings saved to defaults.",5060,119);
	AfxMessageBox(str,MB_OK | MB_ICONINFORMATION,NULL);
}

void CMathomirView::OnSaveoptionsSaveas()
{
	char str[41];
	char filter[120];
	CopyTranslatedString(str,"Settings",5015,40); strcpy(filter,str);strcat(filter,"|*.set|");
	CopyTranslatedString(str,"All files",5011,40);strcat(filter,str);strcat(filter,"|*.*||\0");
	//char *filter="Settings|*.set|All files|*.*||\0";
	CFileDialog fd(FALSE,"set",NULL,0,filter,theApp.m_pMainWnd,0);
	if (fd.DoModal()==IDOK)
	{		
		Toolbox->SaveSettings(fd.m_pOFN->lpstrFile);

	}
}

void CMathomirView::OnSaveoptionsLoad()
{
	char str[41];
	char filter[120];
	CopyTranslatedString(str,"Settings",5015,40); strcpy(filter,str);strcat(filter,"|*.set|");
	CopyTranslatedString(str,"All files",5011,40);strcat(filter,str);strcat(filter,"|*.*||\0");
	//char *filter="Settings|*.set|All files|*.*||\0";
	CFileDialog fd(TRUE,"set",NULL,OFN_HIDEREADONLY,filter,theApp.m_pMainWnd,0);
	if (fd.DoModal()==IDOK)
	{		
		Toolbox->LoadSettings(fd.m_pOFN->lpstrFile);
		Toolbox->InvalidateRect(NULL,0);
		Toolbox->UpdateWindow();
	}
}

void CMathomirView::OnTextend()
{
	if (KeyboardEntryObject)
	{
		CExpression *expr=(CExpression*)KeyboardEntryObject;
		expr->KeyboardStop();
		KeyboardEntryObject=NULL;
		if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
		{
			DeleteDocumentObject(KeyboardEntryBaseObject);
			//RepaintTheView();
		}
		KeyboardEntryBaseObject=NULL;
		RepaintTheView();
	}	
}

#pragma optimize("s",on)
void CMathomirView::OnEditCut()
{
	if (ViewOnlyMode) return;
	//copying to clipboard and deleting selection
	CMathomirDoc* pDoc = GetDocument();

	if (TheKeyboardClipboard) {delete TheKeyboardClipboard;TheKeyboardClipboard=NULL;}
	if (KeyboardEntryObject)
	{
		KeyboardSelectionCut();
	}
	int i;
	
	if (TheKeyboardClipboard)
		pDoc->SaveMOMFile((char*)TheKeyboardClipboard,0);
	else
	{
		pDoc->SaveMOMFile(NULL,0);

		int ok=0;
		for (i=0;i<NumDocumentElements;i++)
		{
			tDocumentStruct *ds=TheDocument+i;
			if (ds->MovingDotState==3)  //selected
			{
				if (ok==0) UndoSave("cut selection",20412);
				ok=1;
				DeleteDocumentObject(ds);
				i--;
			}
		}

		if ((!ok) && (IsDrawingMode) && (TheDocument[NumDocumentElements-1].Type==2))
		{
			//we didn't find anything selected - then delete the last drawing that was created
			UndoSave("cut selection",20412);
			DeleteDocumentObject(&TheDocument[NumDocumentElements-1]);
		}
		MouseMode=0;
	}
	RepaintTheView();
}

#pragma optimize("s",on)
void CMathomirView::OnEditPaste()
{
	if (ViewOnlyMode) return;

	CMathomirDoc* pDoc = GetDocument();
	if (TheKeyboardClipboard) {delete TheKeyboardClipboard;TheKeyboardClipboard=NULL;}
	UndoSave("paste",20201);
	int r1=pDoc->OpenMOMFile(NULL);	
	if ((r1) && (TheKeyboardClipboard==NULL))
	{
		//something pasted - it is found as selection and we will move it to the cursor location to move together with the cursor

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(&p);
		if (p.x<0) p.x=0;
		if (p.y<0) p.y=0;
		RECT r;
		GetClientRect(&r);
		if (p.x>r.right) p.x=r.right;
		if (p.y>r.bottom) p.y=r.bottom;
		MovingStartX=p.x*100/ViewZoom+ViewX;
		MovingStartY=p.y*100/ViewZoom+ViewY;

		MovingFirstMove=0;
		int minX,minY;
		minX=minY=0x7FFFFFFF;
		SelectedDocumentObject=SelectedDocumentObject2=NULL;
		tDocumentStruct *ds2=TheDocument;
		for (int ii=0;ii<NumDocumentElements;ii++,ds2++)
			if (ds2->MovingDotState==3) 
			{
				int actX=0,actY=0;
				if ((ds2->Type==2) && (ds2->Object))
					((CDrawing*)(ds2->Object))->FindRealCorner(&actX,&actY);
				if (ds2->absolute_X+actX<minX) {minX=ds2->absolute_X+actX;SelectedDocumentObject=ds2;}
				if (ds2->absolute_Y-ds2->Above+actY<minY) {minY=ds2->absolute_Y-ds2->Above+actY;SelectedDocumentObject2=ds2;}
			}
		if (minX!=0x7FFFFFFF)
		{
			MouseMode=102;
			RepaintTheView();
			return;
		}
	}
	KeyboardSelectionPaste();	

	if (r1==0)
	{
		//try bitmap pasting from clipboard (DIB bitmap)
		if (theApp.m_pMainWnd->OpenClipboard())
		{
			HGLOBAL clipb_data=GetClipboardData(CF_DIB);
			if (clipb_data) 
			{
				int len=(int)GlobalSize(clipb_data);
				LPVOID pntr=GlobalLock(clipb_data);
				if (pntr)
				{
					//first copy header only
					char *x=(char*)malloc(len);
					memcpy(x,pntr,sizeof(BITMAPINFOHEADER));
					BITMAPINFOHEADER *bhdr=(BITMAPINFOHEADER*)x;

					//examine if the bitmap is 32-bit. If yes, try converting it to 24 bit (to save some memory)
					if (bhdr->biBitCount==32)
					{
						bhdr->biBitCount=24;
						int jumpover=0;
						if (bhdr->biCompression==BI_BITFIELDS) jumpover=12;
						//if (bhdr->biCompression==BI_ALPHABITFIELDS) jumpover=16;
						bhdr->biCompression=0;
						int j=sizeof(BITMAPINFOHEADER);
						int lx=0;
						for (int i=sizeof(BITMAPINFOHEADER)+jumpover;i<len;i+=4)
						{
							x[j++]=*((char*)pntr+i);
							x[j++]=*((char*)pntr+i+1);
							x[j++]=*((char*)pntr+i+2);
							lx++;
							if (lx==bhdr->biWidth) //at the end of scan line, pad with zeros for dword
							{
								while (j%4) x[j++]=0;
								lx=0;
							}
						}
						bhdr->biSizeImage=j-sizeof(BITMAPINFOHEADER);//3*bhdr->biHeight*bhdr->biWidth;
					}
					else
						memcpy(x,pntr,len);
					GlobalUnlock(clipb_data);

					CDrawing *elem=new CDrawing();
					elem->StartCreatingItem(52);
					int xx,yy;

					int colors=bhdr->biClrUsed;
					if (bhdr->biBitCount<24) colors=1<<bhdr->biBitCount;
					int tablesize=colors*sizeof(RGBQUAD);
					int isize=tablesize+sizeof(BITMAPINFOHEADER)+bhdr->biSizeImage+16;
					if (isize<len) isize=len;
					int lw=elem->Items->LineWidth;

					//TODO - maknuti ovaj dio koda u CBitmapImage

					elem->UpdateCreatingItem(bhdr->biWidth*10+lw/50,bhdr->biHeight*10+lw/50,bhdr->biWidth+lw/500,bhdr->biHeight+lw/500);
					elem->EndCreatingItem(&xx,&yy);
					((CBitmapImage*)(elem->SpecialData))->Image=(char*)x;
					((CBitmapImage*)(elem->SpecialData))->imgsize=len;
					((CBitmapImage*)(elem->SpecialData))->editing=0;
					((CBitmapImage*)(elem->SpecialData))->ShowMenu=0;

					AddDocumentObject(2,ViewX+1000/ViewZoom,ViewY+1000/ViewZoom);
					TheDocument[NumDocumentElements-1].Object=(CObject*)elem;
				}
			}
			CloseClipboard();
		}
	}

	RepaintTheView();

}

#pragma optimize("s",on)
void CMathomirView::OnEditCopy()
{
	CMathomirDoc* pDoc = GetDocument();

	if (TheKeyboardClipboard) {delete TheKeyboardClipboard;TheKeyboardClipboard=NULL;}
	if (KeyboardEntryObject)
	{
		KeyboardSelectionCopy();
	}

	if (TheKeyboardClipboard)
	{
		pDoc->SaveMOMFile((char*)TheKeyboardClipboard,0);
	}
	else 
		for (int i=0;i<NumDocumentElements;i++)
			if (TheDocument[i].MovingDotState==3)
			{
				pDoc->SaveMOMFile(NULL,0);
				break;
			}
	RepaintTheView();
}

void CMathomirView::OnEditDelete()
{

	if (ViewOnlyMode) return;

	int ok=0;
	CMathomirDoc* pDoc = GetDocument();
	int i;
	for (i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if (ds->MovingDotState==3)  //selected
		{
			if (ok==0) UndoSave("delete selection",20203);
			DeleteDocumentObject(ds);
			ok=1;
			i--;
		}
		MouseMode=0;
	}
	RepaintTheView();
}
#pragma optimize("",on)
short save_image_to_file_type=1;
void CMathomirView::SaveImageToFile(int cx,int cy,CDC *bmpDC)
{

	//exporting equation image to file
	int i=0;
	char *filter;
	filter="32bit .PNG (transparent, full color)|*.png|24bit .PNG (full color)|*.png|8bit .PNG (basic color)|*.png|1bit .PNG (black and white)|*.png|.JPG (JPEG, full color)|*.jpg|24bit .BMP (full color)|*.bmp|8bit .BMP (basic color)|*.bmp||\0";
	CFileDialog fd(FALSE,NULL,NULL,0,filter,theApp.m_pMainWnd,0);
	fd.m_pOFN->nFilterIndex=save_image_to_file_type;
	if (fd.DoModal()==IDOK)
	{
		save_image_to_file_type=(short)fd.m_pOFN->nFilterIndex;
		fd.m_pOFN->nFilterIndex--;
		int ln=(int)strlen(fd.m_pOFN->lpstrFile);
		if ((fd.m_pOFN->lpstrFile[ln-2]!='.') && (fd.m_pOFN->lpstrFile[ln-3]!='.') && (fd.m_pOFN->lpstrFile[ln-4]!='.'))
		{
			//we are adding extension;
			if (fd.m_pOFN->nFilterIndex<4) strcat(fd.m_pOFN->lpstrFile,".png");
			else if (fd.m_pOFN->nFilterIndex==4) strcat(fd.m_pOFN->lpstrFile,".jpg");
			else if (fd.m_pOFN->nFilterIndex>4) strcat(fd.m_pOFN->lpstrFile,".bmp");
		}

		if ((fd.m_pOFN->nFilterIndex==0) || (fd.m_pOFN->nFilterIndex==1) || 
			(fd.m_pOFN->nFilterIndex==4) || (fd.m_pOFN->nFilterIndex==5))
		{
			//32-bit and 24-bit images
			CImage image;
			if (fd.m_pOFN->nFilterIndex==0)
				image.Create(cx,cy,32,image.createAlphaChannel);  //for transparent .PNG
			else
				image.Create(cx,cy,24,0);  //non-transparent

			for (int i=0;i<cy;i++)
				for (int j=0;j<cx;j++)
				{
					COLORREF clr=bmpDC->GetPixel(j,i);

					if (fd.m_pOFN->nFilterIndex==0)
					{
						int *pnt=(int*)image.GetPixelAddress(j,i);
						*pnt=clr;
						if (clr==0x00FFFFFF) 
							*pnt=(*pnt)|0x00000000;
						else
							*pnt=(*pnt)|0xFF000000;
					}
					else
					{
						image.SetPixel(j,i,clr);
					}
				}
			if (fd.m_pOFN->nFilterIndex==0) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatPNG);				
			if (fd.m_pOFN->nFilterIndex==1) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatPNG);				
			if (fd.m_pOFN->nFilterIndex==4) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatJPEG);				
			if (fd.m_pOFN->nFilterIndex==5) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatBMP);				
		}
		else 
		{
			//8-bit images and 1-bit images
			CImage image;
			if (fd.m_pOFN->nFilterIndex==3) image.Create(cx,cy,1,0); else image.Create(cx,cy,8,0);
			if (image.IsDIBSection())
			{
				if (fd.m_pOFN->nFilterIndex!=3)
				{
					for (int i=0;i<256;i++)
					{
						RGBQUAD rgb;
						rgb.rgbBlue=(i*32)%256;
						rgb.rgbGreen=((i/8)*32)%256;
						rgb.rgbRed=((i/64)*32)%256;
						rgb.rgbReserved=0;
						image.SetColorTable(i,1,&rgb);
					}
					RGBQUAD rgb;
					rgb.rgbBlue=255;
					rgb.rgbGreen=255;
					rgb.rgbRed=255;
					image.SetColorTable(255,1,&rgb);

				}
				else
				{
					//1-bit images
					RGBQUAD rgb;
					rgb.rgbBlue=rgb.rgbGreen=rgb.rgbRed=0;
					rgb.rgbReserved=0;
					image.SetColorTable(0,1,&rgb);
					rgb.rgbBlue=rgb.rgbGreen=rgb.rgbRed=255;
					image.SetColorTable(1,1,&rgb);
				}
				for (int i=0;i<cy;i++)
					for (int j=0;j<cx;j++)
					{
						COLORREF clr=bmpDC->GetPixel(j,i);
						image.SetPixel(j,i,clr);
					}
				if (fd.m_pOFN->nFilterIndex==2) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatPNG);				
				if (fd.m_pOFN->nFilterIndex==3) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatPNG);				
				if (fd.m_pOFN->nFilterIndex==6) image.Save(fd.m_pOFN->lpstrFile,Gdiplus::ImageFormatBMP);				
			}
		}
	}
}


void CMathomirView::OnEditSaveequationimage()
{
	save_equation_image_to_file=1;
	OnEditCopyImage();
	save_equation_image_to_file=0;
}

void CMathomirView::OnEditCopyImage()
{
	CDC *DC=GetDC();
	CBitmap bmp;
	CDC bmpDC;
	int X1,Y1,X2,Y2;
	X1=0x7FFFFFFF;
	Y1=0x7FFFFFFF;
	X2=-1;
	Y2=-1;

	SetCursor(::LoadCursor(NULL,IDC_WAIT));

	int HQ=IsHighQualityRendering;
	int HT=IsHalftoneRendering;
	if (ForceHighQualityImage) IsHighQualityRendering=1; else IsHighQualityRendering=0;
	if (ForceHalftoneImage) IsHalftoneRendering=1; else IsHalftoneRendering=0;

	short l,a,b;
	int i;
	int numsel=0;
	for (i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if ((ds->MovingDotState==3) && (ds->Type==2)) //selected
			numsel++;
	}
	for (i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if (ds->MovingDotState==3)  //selected
		{
			if (ds->Type==1)
			{
				((CExpression*)(ds->Object))->DeselectExpression();
				((CExpression*)(ds->Object))->CalculateSize(DC,ImageSize,&l,&a,&b,-1,(numsel)?0:1);
			}
			else if (ds->Type==2)
			{
				((CDrawing*)(ds->Object))->SelectDrawing(0);
				((CDrawing*)(ds->Object))->CalculateSize(DC,ImageSize,&l,&b);
				a=0;
			}
			ds->Above=a;
			ds->Below=b;
			ds->Length=l;
			int FactX=ds->absolute_X*ImageSize/100;
			int FactY=ds->absolute_Y*ImageSize/100;
			if (X1>FactX) X1=FactX;
			if (Y1>FactY-a) Y1=FactY-a;
			if (X2<FactX+l) X2=FactX+l;
			if (Y2<FactY+b) Y2=FactY+b;
		}
	}

	bmp.CreateCompatibleBitmap(DC,X2-X1+8,Y2-Y1+6);
	bmpDC.CreateCompatibleDC(DC);
	bmpDC.SelectObject(bmp);

	//bmpDC.SelectObject(GetStockObject(WHITE_PEN));
	//bmpDC.SelectObject(GetStockObject(WHITE_BRUSH));
	bmpDC.FillSolidRect(0,0,X2-X1+8,Y2-Y1+6,RGB(255,255,255));
	
	for (i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if (ds->MovingDotState==3)  //selected
		{
			int FactX=ds->absolute_X*ImageSize/100;
			int FactY=ds->absolute_Y*ImageSize/100;
			PrintRendering=1;
			bmpDC.SetBkMode(OPAQUE);
			if (ds->Type==1)
			{
				((CExpression*)(ds->Object))->PaintExpression(&bmpDC,ImageSize,FactX-X1+3,FactY-Y1+3);
				((CExpression*)(ds->Object))->SelectExpression(1);
			}
			else if (ds->Type==2)
			{
				((CDrawing*)(ds->Object))->PaintDrawing(&bmpDC,ImageSize,FactX-X1+3,FactY-Y1+3,-1,-1);
				((CDrawing*)(ds->Object))->SelectDrawing(1);
			}
			PrintRendering=0;

		}
	}

	if (save_equation_image_to_file)
	{
		SaveImageToFile(X2-X1+6,Y2-Y1+6,&bmpDC);
	}
	else if ((theApp.m_pMainWnd->OpenClipboard()) && ((dont_empty_clipboard) || (EmptyClipboard())))
	{
		SetClipboardData(CF_BITMAP,bmp);
		CloseClipboard();
	}


	bmp.DeleteObject();

	IsHighQualityRendering=HQ;
	IsHalftoneRendering=HT;
	for (i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		a=0;
		if (ds->Type==1)
			((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
		else if (ds->Type==2)
			((CDrawing*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&b);

		ds->Above=a*100/ViewZoom;
		ds->Below=b*100/ViewZoom;
		ds->Length=l*100/ViewZoom;
	}
	ReleaseDC(DC);

	Sleep(50);
	if (KeyboardEntryObject)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
		SetCursor(hc);
	}
	else if (ClipboardExpression)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_POINTER_COPY);
		SetCursor(hc);
	}
	else
	{
		HCURSOR hc=::LoadCursor(NULL,IDC_ARROW);
		SetCursor(hc);
	}
	if (!dont_empty_clipboard)
		RepaintTheView();
}

int CMathomirView::MakeImageOfExpression(CObject * expression)
{
	if (expression==NULL) return 0;
	CBitmap bmp;
	CDC bmpDC;

	SetCursor(::LoadCursor(NULL,IDC_WAIT));
	int HQ=IsHighQualityRendering;
	int HT=IsHalftoneRendering;
	if (ForceHighQualityImage) IsHighQualityRendering=1; else IsHighQualityRendering=0;
	if (ForceHalftoneImage) IsHalftoneRendering=1; else IsHalftoneRendering=0;

	CExpression *expr=(CExpression*)expression;

	CDC *DC=GetDC();

	expr->DeselectExpression();
	short l,a,b;
	expr->CalculateSize(DC,ImageSize,&l,&a,&b,-1,1);
	expr->CalculateSize(DC,ImageSize,&l,&a,&b,-1,1); //we call it two times- QUICK FIX (because of GetCellAttributes does not work good)

	bmp.CreateCompatibleBitmap(DC,l+8,a+b+6);
	bmpDC.CreateCompatibleDC(DC);
	bmpDC.SelectObject(bmp);

	//bmpDC.SelectObject(GetStockObject(WHITE_PEN));
	//bmpDC.SelectObject(GetStockObject(WHITE_BRUSH));
	bmpDC.FillSolidRect(0,0,l+8,a+b+6,RGB(255,255,255));

	PrintRendering=1;
	expr->PaintExpression(&bmpDC,ImageSize,3,a+3);
	PrintRendering=0;

	if (save_equation_image_to_file)
	{
		SaveImageToFile(l+8,a+b+6,&bmpDC);
	}
	else if (theApp.m_pMainWnd->OpenClipboard())
	{
		if ((dont_empty_clipboard) || (EmptyClipboard()))
			SetClipboardData(CF_BITMAP,bmp);
		CloseClipboard();
	}

	bmp.DeleteObject();
	ReleaseDC(DC);
	Sleep(50);
	if (KeyboardEntryObject)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
		SetCursor(hc);
	}
	else if (ClipboardExpression)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_POINTER_COPY);
		SetCursor(hc);
	}
	else
	{
		HCURSOR hc=::LoadCursor(NULL,IDC_ARROW);
		SetCursor(hc);
	}
	IsHighQualityRendering=HQ;
	IsHalftoneRendering=HT;
	return 0;
}


int CMathomirView::MakeImageOfDrawing(CObject *drawing_obj)
{
	if (drawing_obj==NULL) return 0;
	CBitmap bmp;
	CDC bmpDC;
	CDrawing* drawing=(CDrawing*)drawing_obj;

	SetCursor(::LoadCursor(NULL,IDC_WAIT));
	int HQ=IsHighQualityRendering;
	int HT=IsHalftoneRendering;
	if (ForceHighQualityImage) IsHighQualityRendering=1;
	if (ForceHalftoneImage) IsHalftoneRendering=1;

	CDC *DC=GetDC();

	drawing->SelectDrawing(0);
	short w,h;
	drawing->CalculateSize(DC,ImageSize,&w,&h);

	bmp.CreateCompatibleBitmap(DC,w+2,h+2);
	bmpDC.CreateCompatibleDC(DC);
	bmpDC.SelectObject(bmp);

	bmpDC.FillSolidRect(0,0,w+2,h+2,RGB(255,255,255));

	PrintRendering=1;
	drawing->PaintDrawing(&bmpDC,ImageSize,1,1,-1,-1);
	PrintRendering=0;

	if (save_equation_image_to_file)
	{
		SaveImageToFile(w+2,h+2,&bmpDC);
	}
	else if (theApp.m_pMainWnd->OpenClipboard())
	{
		if ((dont_empty_clipboard) ||(EmptyClipboard()))
			SetClipboardData(CF_BITMAP,bmp);
		CloseClipboard();
	}

	bmp.DeleteObject();
	ReleaseDC(DC);
	Sleep(50);
	if (KeyboardEntryObject)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
		SetCursor(hc);
	}
	else if (ClipboardExpression)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_POINTER_COPY);
		SetCursor(hc);
	}
	else
	{
		HCURSOR hc=::LoadCursor(NULL,IDC_ARROW);
		SetCursor(hc);
	}
	IsHighQualityRendering=HQ;
	IsHalftoneRendering=HT;
	return 0;
}

#pragma optimize("s",on)
void CMathomirView::OnOutputimageSize200()
{
	ImageSize=200;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnOutputimageSize150()
{
	ImageSize=150;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();}

void CMathomirView::OnOutputimageSize100()
{
	ImageSize=100;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnOutputimageSize80()
{
	ImageSize=80;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnOutputimageForcehighquality()
{
	if (ForceHighQualityImage)
		ForceHighQualityImage=0;
	else
		ForceHighQualityImage=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnOutputimageForcehalftone()
{
	if (ForceHalftoneImage)
		ForceHalftoneImage=0;
	else
		ForceHalftoneImage=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnFontsizeVerylarge()
{
	DefaultFontSize=150; //bilo 185
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnFontsizeLarge()
{
	DefaultFontSize=120; //bilo 150
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnFontsizeNormal()
{
	DefaultFontSize=100;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnFontsizeSmall()
{
	DefaultFontSize=85; //bilo 80
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

#pragma optimize("",on)

void CMathomirView::OnEditUndo()
{
	if (UndoNumLevels==0) return; //nothing to restore

	//first stop the keyboard entry mode if it is active
	if (KeyboardEntryObject)
	{
		((CExpression*)KeyboardEntryObject)->KeyboardStop();
		CExpression *expr=(CExpression*)KeyboardEntryObject;
		if (expr->m_pPaternalExpression==NULL)
		if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
		{
			DeleteDocumentObject(KeyboardEntryBaseObject);
		}

	}


	prevTouchedObject=NULL;
	KeyboardEntryObject=NULL;
	KeyboardEntryBaseObject=NULL;
	SelectedDocumentObject=NULL;
	SelectedDocumentObject2=NULL;
	m_PopupMenuObject=NULL;

	UndoRestore();


	int i;
	int found_keyboard_entry=0;
	for (i=0;i<NumDocumentElements;i++)
	{
		if ((TheDocument[i].Type==1) && (TheDocument[i].Object))
		{
			CExpression *expr=((CExpression*)(TheDocument[i].Object));
			
			//check if any object was in keyboard entry mode - if yes start the keyboard entry
			CObject *ret=expr->KeyboardFindEntryPos();
			if (ret)
			{
				if (found_keyboard_entry==0)
				{
					KeyboardEntryObject=ret;
					KeyboardEntryBaseObject=TheDocument+i;
					found_keyboard_entry=1;
				}
				else
					((CExpression*)ret)->m_IsKeyboardEntry=0;
			}
			expr->DeselectExpression();
		}
	}

	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	RepaintTheView();
}

#pragma optimize("s",on)
int CMathomirView::UndoInit()
{
	return ((CMainFrame*)(theApp.m_pMainWnd))->UndoInit();

}

int CMathomirView::UndoRestore()
{
	return ((CMainFrame*)(theApp.m_pMainWnd))->UndoRestore();

}

int CMathomirView::UndoSave(char *undo_text,int unique_ID)
{
	return ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave(undo_text,unique_ID);
}

void CMathomirView::OnMovingdotLarge()
{
	MovingDotSize=8;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	RepaintTheView();
}

void CMathomirView::OnMovingdotMedium()
{
	MovingDotSize=6;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	RepaintTheView();
}

void CMathomirView::OnMovingdotSmall()
{
	MovingDotSize=5;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	RepaintTheView();
}

void CMathomirView::OnMovingdotPermanent()
{
	if (MovingDotPermanent)
		MovingDotPermanent=0;
	else
		MovingDotPermanent=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	RepaintTheView();
}
#pragma optimize("s",on)
void CMathomirView::OnHelpQuickguide()
{
	int use_internal_file=0;
	char DefaultFilename[512];
	if (1) 
	{
		int j;
		DefaultFilename[0]=0;
		strcpy(DefaultFilename,GetCommandLine());
		DefaultFilename[511]=0;
		j=1;
		while (DefaultFilename[j]!='"')
			j++;
		for (;j>=0;j--)
			if (DefaultFilename[j]=='\\') break;
		DefaultFilename[j]=0;

		int x=(int)strlen(DefaultFilename);
		strcpy(DefaultFilename+x,"\\HandyHelp.chm");
		FILE *f;
		f=fopen(DefaultFilename+1,"rb");
		if (f)
		{
			fclose (f);
		}
		else
		{
			strcpy(DefaultFilename+x,"\\HandyHelp.pdf");
			f=fopen(DefaultFilename+1,"rb");
			if (f)
			{
				fclose(f);
			}
			else
			{
				strcpy(DefaultFilename+x,"\\Mathomir.chm");
				use_internal_file=1;
			}
		}
	}

	HGLOBAL hg;
	HRSRC resinfo;
	resinfo=::FindResource(theApp.m_hInstance,MAKEINTRESOURCE(IDR_BINARY1),"BINARY");
	if (resinfo)
	{
		hg=LoadResource(NULL,resinfo);
		if (hg)
		{ 
			int size=64387;  //the exact size of the help file (from the resource)
			LPVOID pnt=LockResource(hg);
			if (pnt)
			{
				FILE *fil=NULL;
				if (use_internal_file)
				{
					fil=fopen(DefaultFilename+1,"w+b");
					if (fil==NULL)
					{
						GetTempPath(500,DefaultFilename+1);
						if (DefaultFilename[strlen(DefaultFilename)]!='\\') strcat(DefaultFilename+1,"\\");
						strcat(DefaultFilename+1,"Mathomir.chm");
						fil=fopen(DefaultFilename+1,"w+b");
					}
				
					if (fil) 
					{
						fwrite(pnt,size,1,fil);
						fclose(fil);
						Sleep(200);						
					}
				}
				ShellExecute(NULL,NULL,DefaultFilename+1,NULL,NULL,SW_NORMAL);
				UnlockResource(hg);
			}			
		}
	}
}
#pragma optimize("",on)

BOOL CMathomirView::OnEraseBkgnd(CDC* pDC)
{
	//will do nothing - the background will be repainted in OnDraw.
	return 1;

	//return CView::OnEraseBkgnd(pDC);
}


int CMathomirView::PasteDrawing(CDC *DC,int cursorX, int cursorY, CObject *drawing,int select)
{
	CDrawing *mydrw=(CDrawing*)drawing;
	if (mydrw==NULL) mydrw=ClipboardDrawing;
	if (mydrw==NULL) return 0;
	int AbsoluteX=ViewX+(int)cursorX*100/(int)ViewZoom;
	int AbsoluteY=ViewY+(int)cursorY*100/(int)ViewZoom;
	AbsoluteX-=MovingStartX;
	AbsoluteY-=MovingStartY;


	if ((MouseMode!=9) && (MouseMode!=11))
	{
		if ((SpecialDrawingHover) && 
			(((CDrawing*)SpecialDrawingHover->Object)->IsSpecialDrawing==50))
		{
			if (GetKeyState(VK_MENU)&0xFFFE)
			{
				//snap to point is used inside drawing boxes when ALT key is held
				int ppx=AbsoluteX+MovingStartX;
				int ppy=AbsoluteY+MovingStartY;
				if ((tmpDrawing) && (tmpDrawing->FindNerbyPoint(&ppx,&ppy,NULL,0,0,0,0)))
				{
					AbsoluteX=ppx-MovingStartX;
					AbsoluteY=ppy-MovingStartY;
				}
			}
		}
		else if ((IsShowGrid) && (!(GetKeyState(VK_MENU)&0xFFFE)))
		{
			AbsoluteX=((AbsoluteX+GRID/2)/GRID)*GRID;
			AbsoluteY=((AbsoluteY+GRID/2)/GRID)*GRID;
		
			int x1,y1;
			mydrw->FindRealCorner(&x1,&y1);
			AbsoluteX-=x1;
			AbsoluteY-=y1;
		}
	}



	for (int jj=0;jj<mydrw->NumItems;jj++)
	{
		tDrawingItem *di=mydrw->Items+jj;
		if ((di->Type==0) && (di->pSubdrawing))
		{
			//NumDocumentElements++;
			//CheckDocumentMemoryReservations();
			AddDocumentObject(2,0,0);

			tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
			ds->Type=2; //drawing
			CDrawing *tmp=new CDrawing();
			tmp->CopyDrawing((CDrawing*)di->pSubdrawing);
			ds->Object=(CObject*)tmp;
			short l=0,a=0,b=0;
			((CDrawing*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&b);
			ds->Length=(short)((int)l*100/(int)ViewZoom);
			ds->Above=0;
			ds->Below=(short)((int)b*100/(int)ViewZoom);
			ds->absolute_X=AbsoluteX+(di->X1/DRWZOOM);
			ds->absolute_Y=AbsoluteY+(di->Y1/DRWZOOM);
			ds->MovingDotState=0;if (select) ds->MovingDotState=3;

			if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
			if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;

			GentlyPaintObject(ds,DC);
		}
		if ((di->Type==2) && (di->pSubdrawing))
		{
			//NumDocumentElements++;
			//CheckDocumentMemoryReservations();
			AddDocumentObject(1,0,0);

			tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
			//ds->Type=1; //expression
			CExpression *tmp=new CExpression(NULL,NULL,((CExpression*)(di->pSubdrawing))->m_FontSize);
			//tmp->m_FontSizeHQ=((CExpression*)(di->pSubdrawing))->m_FontSizeHQ;
			tmp->CopyExpression((CExpression*)di->pSubdrawing,0);
			ds->Object=(CObject*)tmp;
			short l=0,a=0,b=0;
			((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
			ds->Length=(short)((int)l*100/(int)ViewZoom);
			ds->Above=(short)((int)a*100/(int)ViewZoom);
			ds->Below=(short)((int)b*100/(int)ViewZoom);
			ds->absolute_X=AbsoluteX+(di->X1/DRWZOOM);
			ds->absolute_Y=AbsoluteY+(di->Y1/DRWZOOM)+ds->Above;
			ds->MovingDotState=0;if (select) ds->MovingDotState=3;

			if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
			if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;

			GentlyPaintObject(ds,DC);
		}
	}


	return 0;
}

#pragma optimize("s",on)
void CMathomirView::OnEditAccesslockedobjects()
{
	if (AccessLockedObjects) AccessLockedObjects=0; else AccessLockedObjects=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();

}

void CMathomirView::OnGridFine()
{
	GRID=8;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	if (IsShowGrid) RepaintTheView();
}

void CMathomirView::OnGridMedium()
{
	GRID=16;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	if (IsShowGrid) RepaintTheView();
}

void CMathomirView::OnGridCoarse()
{
	GRID=24;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	if (IsShowGrid) RepaintTheView();
}

/*void CMathomirView::OnGridSnaptogrid()
{
	if (SnapToGrid) SnapToGrid=0; else SnapToGrid=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}*/

void CMathomirView::OnViewShowgrid()
{
	if (IsShowGrid) IsShowGrid=0; else IsShowGrid=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	RepaintTheView();
}
#pragma optimize("",on)


//combines document elements into one group (as group elements)
//it can compose drawing from touched or from selected elements (depends on parameter)
CObject * CMathomirView::ComposeDrawing(int *X, int *Y,int compose_from_selection,int select_for_moving)
{
	CDrawing *ret=NULL;
	for (int iii=0;iii<NumDocumentElements;iii++)
	{
		tDocumentStruct *ds2=TheDocument+iii;
		if (ds2->Object==NULL) continue;
		if (ds2->MovingDotState==5) continue; //locked object

		if ((ds2->Type==2) && 
			(((!compose_from_selection) && (((CDrawing*)(ds2->Object))->IsSelected) && (ds2->MovingDotState!=3)) ||
			 ((compose_from_selection) && (ds2->MovingDotState==3))))
		{
			if (ret==NULL)
			{
				ret=new CDrawing();
				*X=ds2->absolute_X;
				*Y=ds2->absolute_Y;
			}
			ret->CopyDrawingIntoSubgroup((CDrawing*)ds2->Object,ds2->absolute_X-*X,ds2->absolute_Y-*Y);
			int dx,dy,w,h;
			ret->AdjustCoordinates(&dx,&dy,&w,&h);
			*X+=dx;
			*Y+=dy;
			if (select_for_moving) ds2->MovingDotState=4; //flag for deletion
		}
		if ((ds2->Type==1) && 
			(((!compose_from_selection) && (((CExpression*)(ds2->Object))->m_Selection==0x7FFF) && (ds2->MovingDotState!=3)) ||
			 ((compose_from_selection) && (ds2->MovingDotState==3))))
		{
			if (ret==NULL)
			{
				ret=new CDrawing();
				*X=ds2->absolute_X;
				*Y=ds2->absolute_Y-ds2->Above;
			}
			ret->CopyExpressionIntoSubgroup((CExpression*)ds2->Object,ds2->absolute_X-*X,ds2->absolute_Y-ds2->Above-*Y,ds2->Length,ds2->Above+ds2->Below);
			int dx,dy,w,h;
			ret->AdjustCoordinates(&dx,&dy,&w,&h);
			*X+=dx;
			*Y+=dy;
			if (select_for_moving) ds2->MovingDotState=4; //flag for deletion
		}
	}
	return (CObject*)ret;
}


#pragma optimize("s",on)
/*void CMathomirView::OnKeyboardSimplevariablemode()
{
	IsSimpleVariableMode=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}*/

void CMathomirView::OnKeyboardVerysimplevariablemode()
{
	IsSimpleVariableMode=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnKeyboardGeneralvariablemode()
{
	IsSimpleVariableMode=0;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

#pragma optimize("s",on)
int CMathomirView::StartKeyboardEntryAt(int AbsoluteX, int AbsoluteY, int start_textmode)
{
	//AbsoluteX-=1;
	if (ViewOnlyMode) return 0;

		//clicked on empty screen, we are adding new empty expression
		if (KeyboardEntryObject)
		{
			CExpression *expr=(CExpression*)KeyboardEntryObject;
			expr->KeyboardStop();
			KeyboardEntryObject=NULL;
			if (expr->m_pPaternalExpression==NULL)
			if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
			if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
			{
				DeleteDocumentObject(KeyboardEntryBaseObject);
				//RepaintTheView();
			}
			KeyboardEntryBaseObject=NULL;
		}
		//NumDocumentElements++;
		//CheckDocumentMemoryReservations();
		/*if (((GetKeyState(VK_MENU)&0xFFFE) && (!SnapToGrid)) ||
			((!(GetKeyState(VK_MENU)&0xFFFE)) && (SnapToGrid)))
		{
			AbsoluteX=((AbsoluteX+GRID/2)/GRID)*GRID;
			AbsoluteY=((AbsoluteY+GRID/2)/GRID)*GRID;
		}*/
		AddDocumentObject(1,AbsoluteX,AbsoluteY);
		tDocumentStruct *ds;
		ds=TheDocument+NumDocumentElements-1;

		//ds->absolute_X=AbsoluteX; 
		//int RelativeX=(AbsoluteX-ViewX)*(int)ViewZoom/100;
		//ds->absolute_Y=AbsoluteY;
		//int RelativeY=(AbsoluteY-ViewY)*(int)ViewZoom/100;
		//ds->Type=1; //expression
		ds->Object=(CObject*)new CExpression(NULL,NULL,DefaultFontSize);
		//if (start_textmode) {((CExpression*)(ds->Object))->m_IsText=1;((CExpression*)(ds->Object))->m_Alignment=1;} //if we need to start the text box
		short l,a,b;
		CDC *DC=this->GetDC();
		((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
		ds->Length=(short)((int)l*100/(int)ViewZoom);
		ds->Above=(short)((int)a*100/(int)ViewZoom);
		ds->Below=(short)((int)b*100/(int)ViewZoom);
		ds->MovingDotState=0;

		if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
		if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;

		((CExpression*)(ds->Object))->m_Selection=0;
		((CExpression*)(ds->Object))->KeyboardStart(DC,ViewZoom);
		if (((CExpression*)(ds->Object))->m_pElementList->pElementObject)
			((CExpression*)(ds->Object))->m_pElementList->pElementObject->m_Text=(char)start_textmode;
		((CExpression*)(ds->Object))->m_ModeDefinedAt=1+(start_textmode<<14);
		KeyboardEntryObject=ds->Object;
		KeyboardEntryBaseObject=ds;
		MultipleStartX=MultipleStartY=-1;
		GentlyPaintObject(ds,DC);
		this->ReleaseDC(DC);
		CDC *mdc=Toolbox->GetDC();
		Toolbox->PaintTextcontrolbox(mdc);
		Toolbox->ReleaseDC(mdc);
		if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
	return 1;
}

#pragma optimize("s",on)
void CMathomirView::OnSymboliccalculatorEnable()
{
	if (IsMathDisabled==1)
		IsMathDisabled=0;
	else
		IsMathDisabled=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnImaginaryunitI()
{
	if (ImaginaryUnit=='i')
		ImaginaryUnit=0;
	else
		ImaginaryUnit='i';
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnImaginaryunitJ()
{
	if (ImaginaryUnit=='j')
		ImaginaryUnit=0;
	else
		ImaginaryUnit='j';
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();}


#pragma optimize("s",on)
/*void CMathomirView::OnEditCopymathmlcode()
{
	char *head="<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n";
	char *foot="</math>\r\n\r\n";
	int len=0;
	for (int i=0;i<NumDocumentElements;i++)
	{
		if (((TheDocument+i)->MovingDotState==3) && ((TheDocument+i)->Type==1))
		{
			len+=(int)strlen(head);
			len+=((CExpression*)((TheDocument+i)->Object))->MathML_output(NULL,1,1,1);
			len+=(int)strlen(foot);
		}
	}

	HGLOBAL mem=GlobalAlloc(GMEM_MOVEABLE,len+256);
	char *pnt=(char*)GlobalLock(mem);
	for (int i=0;i<NumDocumentElements;i++)
	{
		if (((TheDocument+i)->MovingDotState==3) && ((TheDocument+i)->Type==1))
		{
			strcpy(pnt,head);pnt+=strlen(head);
			int tt=((CExpression*)((TheDocument+i)->Object))->MathML_output(pnt,1,0,1);
			pnt+=tt;
			strcpy(pnt,foot);pnt+=strlen(foot);
		}
	}		
	*pnt=0;
	GlobalUnlock(mem);


	if ((theApp.m_pMainWnd->OpenClipboard()) && (EmptyClipboard()))
	{
		SetClipboardData(CF_TEXT,mem);
		CloseClipboard();
	}
}

#pragma optimize("s",on)
int CMathomirView::CopyMathMLCode(CObject *expr)
{
	SetCursor(::LoadCursor(NULL,IDC_WAIT));
	short l,a,b;
	CDC *DC;
	DC=pMainView->GetDC();
	((CExpression*)expr)->CalculateSize(DC,ViewZoom,&l,&a,&b);
	pMainView->ReleaseDC(DC);

	char *head="<math xmlns=\"http://www.w3.org/1998/Math/MathML\">\r\n";
	char *foot="</math>\r\n\r\n";
	int len=0;
	len+=(int)strlen(head);
	len+=((CExpression*)(expr))->MathML_output(NULL,1,1,1);
	len+=(int)strlen(foot);


	HGLOBAL mem=GlobalAlloc(GMEM_MOVEABLE,len+256);
	char *pnt=(char*)GlobalLock(mem);

	strcpy(pnt,head);pnt+=strlen(head);
	int tt=((CExpression*)expr)->MathML_output(pnt,1,0,1);
	pnt+=tt;
	strcpy(pnt,foot);pnt+=strlen(foot);
	
	*pnt=0;
	GlobalUnlock(mem);


	if ((theApp.m_pMainWnd->OpenClipboard()) && (EmptyClipboard()))
	{
		SetClipboardData(CF_TEXT,mem);
		CloseClipboard();
	}

	Sleep(50);
	SetMousePointer();

	return 1;
}*/

#pragma optimize("s",on)
int CMathomirView::CopyLaTeXCode(CObject *expr)
{
	SetCursor(::LoadCursor(NULL,IDC_WAIT));
	short l,a,b;
	CDC *DC;
	DC=pMainView->GetDC();
	((CExpression*)expr)->CalculateSize(DC,ViewZoom,&l,&a,&b);
	pMainView->ReleaseDC(DC);

	char *head="";
	char *foot="";
	int len=0;
	len+=(int)strlen(head);
	len+=((CExpression*)(expr))->LaTeX_output(NULL,1);
	len+=(int)strlen(foot);

	HGLOBAL mem=GlobalAlloc(GMEM_MOVEABLE,len+256);
	char *pnt=(char*)GlobalLock(mem);

	strcpy(pnt,head);pnt+=strlen(head);
	int tt=((CExpression*)expr)->LaTeX_output(pnt,0);
	pnt+=tt;
	strcpy(pnt,foot);pnt+=strlen(foot);
	
	*pnt=0;
	GlobalUnlock(mem);


	if ((theApp.m_pMainWnd->OpenClipboard()) && (EmptyClipboard()))
	{
		SetClipboardData(CF_TEXT,mem);
		CloseClipboard();
	}

	Sleep(50);
	SetMousePointer();
	/*if (KeyboardEntryObject)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
		SetCursor(hc);
	}
	else if (ClipboardExpression)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_POINTER_COPY);
		SetCursor(hc);
	}
	else
	{
		HCURSOR hc=::LoadCursor(NULL,IDC_ARROW);
		SetCursor(hc);
	}*/
	return 1;
}

#pragma optimize("s",on)
void CMathomirView::OnSelectionsShadowselections()
{
	if (ShadowSelection) ShadowSelection=0; else ShadowSelection=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnAutosaveNever()
{
	AutosaveOption=0;
	AutosaveTime=0;
	AutosavePoints=0;
}

void CMathomirView::OnAutosaveLow()
{
	AutosaveOption=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();

}

void CMathomirView::OnAutosaveMedium()
{
	AutosaveOption=2;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();

}

void CMathomirView::OnAutosaveHigh()
{
	AutosaveOption=3;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();

}

void CMathomirView::OnAutosaveLoadtheautosave()
{
	this->GetDocument()->OnFileNew();
	if (this->GetDocument()->IsModified()==0)
	{
		if (this->GetDocument()->OpenMOMFile("autosave.mom"))
		{
			AutosavePoints=AutosaveTime=0;
			this->GetDocument()->SetPathName("Untitled",0);
			this->GetDocument()->SetModifiedFlag(0);
			((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
			RepaintTheView();
			InvalidateRect(NULL,1);
		}
	}
}

void CMathomirView::OnZoomWheelzoomadjustspointer()
{
	if (MoveCursorOnWheel) MoveCursorOnWheel=0; else MoveCursorOnWheel=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}


void CMathomirView::OnKeyboardAltmenushortcuts()
{
	if (EnableMenuShortcuts) EnableMenuShortcuts=0; else EnableMenuShortcuts=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

#pragma optimize("s",on)
void CMathomirView::OnViewZoomin()
{
	if (ViewZoom>=480) return;
	PaintDrawingHotspot(1);

	if ((IsMouseUsed) && (KeyboardEntryObject==NULL))
	{
		IsMouseUsed=3;
		MouseWheelDelta=120;
		OnMouseWheel(0,0,0);
		return;
	}

	int sizex=TheClientRect.right*100/ViewZoom;
	int sizey=TheClientRect.bottom*100/ViewZoom;

	int lx_gap=0x7FFFFFFF;
	int dx_gap=0x7FFFFFFF;
	int ty_gap=0x7FFFFFFF;
	int by_gap=0x7FFFFFFF;
	int mindist=0x7FFFFFFF;
	int cscrx=ViewX+sizex/2;
	int cscry=ViewY+sizey/2;
	tDocumentStruct *center_object=NULL;
	for (int i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		int cx=ds->absolute_X+ds->Length/2;
		int cy=ds->absolute_Y-ds->Above/2+ds->Below/2;
		if (abs(cscrx-cx)+abs(cscry-cy)<mindist) {mindist=abs(cscrx-cx)+abs(cscry-cy);center_object=ds;}

		if ((ds->absolute_Y+ds->Below>ViewY) && (ds->absolute_Y-ds->Above<ViewY+sizey))
		{
			if ((ds->absolute_X<ViewX) && (ds->absolute_X+ds->Length>ViewX)) {lx_gap=0;}
			if ((ds->absolute_X>ViewX) && (lx_gap>ds->absolute_X-ViewX)) lx_gap=ds->absolute_X-ViewX;
			if ((ds->absolute_X<ViewX+sizex) && (ds->absolute_X+ds->Length>ViewX+sizex)) {dx_gap=0;}
			if ((ds->absolute_X+ds->Length<ViewX+sizex) && (dx_gap>ViewX+sizex-ds->absolute_X-ds->Length)) dx_gap=ViewX+sizex-ds->absolute_X-ds->Length;
		}
		if ((ds->absolute_X+ds->Length>ViewX) && (ds->absolute_X<ViewX+sizex))
		{
			if ((ds->absolute_Y-ds->Above<ViewY) && (ds->absolute_Y+ds->Below>ViewY)) {ty_gap=0;}
			if ((ds->absolute_Y-ds->Above>ViewY) && (ty_gap>ds->absolute_Y-ds->Above-ViewY)) ty_gap=ds->absolute_Y-ds->Above-ViewY;
			if ((ds->absolute_Y-ds->Above<ViewY+sizey) && (ds->absolute_Y+ds->Below>ViewY+sizey)) {by_gap=0;}
			if ((ds->absolute_Y+ds->Below<ViewY+sizey) && (by_gap>ViewY+sizey-ds->absolute_Y-ds->Below)) by_gap=ViewY+sizey-ds->absolute_Y-ds->Below;
		}
	}

	int x1=ViewX+lx_gap;
	int x2=ViewX+sizex-dx_gap;
	int y1=ViewY+ty_gap;
	int y2=ViewY+sizey-by_gap;
	if (NumDocumentElements==0) {x1=x2=ViewX;y1=y2=ViewY;}

	if (ViewZoom<=30) ViewZoom=34;
	else if (ViewZoom<=34) ViewZoom=40;
	else if (ViewZoom<=40) ViewZoom=50;
	else if (ViewZoom<=50) ViewZoom=60;
	else if (ViewZoom<=60) ViewZoom=70;
	else if (ViewZoom<=70) ViewZoom=85;
	else if (ViewZoom<=85) ViewZoom=100;
	else if (ViewZoom<=100) ViewZoom=120;
	else if (ViewZoom<=120) ViewZoom=150;
	else if (ViewZoom<=150) ViewZoom=170;
	else if (ViewZoom<=170) ViewZoom=200;
	else if (ViewZoom<=200) ViewZoom=240;
	else if (ViewZoom<=240) ViewZoom=300;
	else if (ViewZoom<=300) ViewZoom=340;
	else if (ViewZoom<=340) ViewZoom=400;
	else if (ViewZoom<=400) ViewZoom=480;
	int sizex2=TheClientRect.right*100/ViewZoom;
	int sizey2=TheClientRect.bottom*100/ViewZoom;

	int px=ViewX;
	int py=ViewY;

	if ((x2-x1>sizex2)&&(center_object)) 
		ViewX=center_object->absolute_X+center_object->Length/2-sizex2/2;
	else
		ViewX=(x1+x2)/2-sizex2/2;
	
	if ((y2-y1>sizey2)&&(center_object)) 
		ViewY=center_object->absolute_Y-center_object->Above/2+center_object->Below/2-sizey2/2;
	else
		ViewY=(y1+y2)/2-sizey2/2;

	if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
	{
		int X,Y;
		((CExpression*)((KeyboardEntryBaseObject->Object)))->GetKeyboardCursorPos(&X,&Y);
		X=X*100/ViewZoom;
		Y=Y*100/ViewZoom;
		X+=KeyboardEntryBaseObject->absolute_X;
		Y+=KeyboardEntryBaseObject->absolute_Y;
		if (ViewX+sizex2/4>X) ViewX=X-sizex2/4;
		if (ViewX+sizex2*3/4<X) ViewX=X-sizex2*3/4;
		if (ViewY+sizey2/4>Y) ViewY=Y-sizey2/4;
		if (ViewY+sizey2*3/4<Y) ViewY=Y-sizey2*3/4;
	}


	if (ViewX<px) ViewX=px;
	if (ViewY<py) ViewY=py;
	if (ViewX+sizex2>px+sizex) ViewX=px+sizex-sizex2;
	if (ViewY+sizey2>py+sizey) ViewY=py+sizey-sizey2;
	if (ViewX<0) ViewX=0;
	if (ViewY<0) ViewY=0;

	if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
	RepaintTheView(1);
	//if keyboard entry mode, adjust position of the cursor (trick: send dummy char -1)
	if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))	SendKeyStroke(-1,0,0);

}
#pragma optimize("s",on)
void CMathomirView::OnViewZoomout()
{
	PaintDrawingHotspot(1);
	//RECT r;
	//GetClientRect(&r);
	int sizex=TheClientRect.right*100/ViewZoom;
	int sizey=TheClientRect.bottom*100/ViewZoom;

	if (ViewZoom>=480) ViewZoom=400;
	else if (ViewZoom>=400) ViewZoom=340;
	else if (ViewZoom>=340) ViewZoom=300;
	else if (ViewZoom>=300) ViewZoom=240;
	else if (ViewZoom>=240) ViewZoom=200;
	else if (ViewZoom>=200) ViewZoom=170;
	else if (ViewZoom>=170) ViewZoom=150;
	else if (ViewZoom>=150) ViewZoom=120;
	else if (ViewZoom>=120) ViewZoom=100;
	else if (ViewZoom>=100) ViewZoom=85;
	else if (ViewZoom>=85) ViewZoom=70;
	else if (ViewZoom>=70) ViewZoom=60;
	else if (ViewZoom>=60) ViewZoom=50;
	else if (ViewZoom>=50) ViewZoom=40;
	else if (ViewZoom>=40) ViewZoom=34;
	else if (ViewZoom>=34) ViewZoom=30;
	int sizex2=TheClientRect.right*100/ViewZoom;
	int sizey2=TheClientRect.bottom*100/ViewZoom;

	ViewX+=(sizex-sizex2)/2; if (ViewX<0) ViewX=0;
	ViewY+=(sizey-sizey2)/2; if (ViewY<0) ViewY=0;
	
	if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

	RepaintTheView(1);

	//if keyboard entry mode, adjust position of the cursor (trick: send dummy char -1)
	if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))	SendKeyStroke(-1,0,0);
}


int choosenfont=0;
void CMathomirView::OnFontfacesFont1()
{
	LOGFONT lf;
	CHOOSEFONT f;
	CDC *DC=this->GetDC();
	((CMainFrame*)(theApp.m_pMainWnd))->GetLogicalFont(choosenfont,&lf,DC);
	ZeroMemory(&f,sizeof(CHOOSEFONT));
	f.lStructSize=sizeof(CHOOSEFONT);
	f.hwndOwner=theApp.m_pMainWnd->m_hWnd;
	f.hDC=DC->m_hDC;
	f.lpLogFont=&lf;
	f.Flags=CF_SCREENFONTS|CF_INITTOLOGFONTSTRUCT|CF_SCALABLEONLY|CF_LIMITSIZE;
	f.nSizeMin=10;
	f.nSizeMax=30;
	if (ChooseFont(&f))	((CMainFrame*)(theApp.m_pMainWnd))->SetLogicalFont(choosenfont,&lf,DC);

	this->ReleaseDC(DC);
	RepaintTheView(1);
	Toolbox->InvalidateRect(NULL,1);
	Toolbox->UpdateWindow();
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnFontfacesFont2()
{
	choosenfont=1;OnFontfacesFont1();choosenfont=0;
}

void CMathomirView::OnFontfacesFont3()
{
	choosenfont=2;OnFontfacesFont1();choosenfont=0;
}

void CMathomirView::OnFontfacesFont4()
{
	choosenfont=3;OnFontfacesFont1();choosenfont=0;
}

void CMathomirView::OnFontfacesSetfontstodefaullts()
{
	((CMainFrame*)(theApp.m_pMainWnd))->SetFontsToDefaults();
	RepaintTheView(1);
	Toolbox->InvalidateRect(NULL,1);
	Toolbox->UpdateWindow();
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

#pragma optimize("s",on)
void CMathomirView::KeyboardSelectionCut(int no_copy)
{
	if (!no_copy) KeyboardSelectionCopy(1);
	CExpression *e=((CExpression*)KeyboardEntryObject);

	int mi=0;
	int i=0;
	for (i=0;i<e->m_NumElements;i++)
	{
		if ((e->m_pElementList+i)->IsSelected==2) {mi=i;break;}
	}
	if (i>=e->m_NumElements) 
	{
		return;
	}

	tElementStruct *ts=e->m_pElementList+e->m_IsKeyboardEntry-1;
	if ((ts->Type==1) && (ts->pElementObject->Data1[0]==0)) e->DeleteElement(e->m_IsKeyboardEntry-1);

	e->DeleteSelection(2);
	
	if (mi) if ((e->m_pElementList+mi-1)->Type==0) mi--;
	if (mi>e->m_NumElements) mi=e->m_NumElements;
	e->m_IsKeyboardEntry=mi+1;
	e->InsertEmptyElement(mi,1,0);
	e->m_KeyboardCursorPos=0;
}

#pragma optimize("s",on)
void CMathomirView::KeyboardSelectionCopy(int no_deselect)
{
	if (TheKeyboardClipboard) delete TheKeyboardClipboard;
	TheKeyboardClipboard=new CExpression(NULL,NULL,DefaultFontSize);

	CExpression *e=((CExpression*)KeyboardEntryObject);

	TheKeyboardClipboard->CopyExpression(e,1,2);

	TheKeyboardClipboard->m_StartAsText=e->m_StartAsText;
	TheKeyboardClipboard->m_Alignment=e->m_Alignment;
	TheKeyboardClipboard->m_Color=e->m_Color;
	if (e->m_pPaternalExpression==NULL)
	{
		TheKeyboardClipboard->m_FontSize=e->m_FontSize;
		//TheKeyboardClipboard->m_FontSizeHQ=e->m_FontSizeHQ;	
	}
	if (!no_deselect) e->DeselectExpression();
	if ((TheKeyboardClipboard->m_NumElements==1) && (TheKeyboardClipboard->m_pElementList->Type==0)) 
	{
		delete TheKeyboardClipboard;
		TheKeyboardClipboard=0;
	}
}

extern char InhibitAllKeyHandling;
extern char InhibitParentheseMerging;
#pragma optimize("s",on)
void CMathomirView::KeyboardSelectionPaste()
{
	if (KeyboardEntryObject==NULL) return;
	int text_pasted=0;
	{
		//try to paste text 
		if (theApp.m_pMainWnd->OpenClipboard())
		{
			HANDLE clipb_data;
			clipb_data=GetClipboardData(CF_TEXT);
			if (clipb_data==NULL) 
				CloseClipboard();
			else
			{
				int len=(int)GlobalSize(clipb_data);
				LPVOID pntr=GlobalLock(clipb_data);
				if (pntr==NULL)
					CloseClipboard();
				else
				{
					CDC *DC=GetDC();
					int first_pass=1;
					//((CMainFrame*)theApp.m_pMainWnd)->UndoSave("text paste");
					((CMainFrame*)theApp.m_pMainWnd)->UndoDisableSaving();
					//int enterkey=0;
					InhibitParentheseMerging=1;
					int IsText=0;
					IsText=((CExpression*)KeyboardEntryObject)->DetermineInsertionPointType(((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry-1);

					for (int i=0;i<len;i++)
					{
						UINT ch,flags;
						int fcolor=0;
						flags=0;
						ch=(UINT)*((unsigned char*)pntr+i);
						if (Toolbox) {fcolor=Toolbox->GetFormattingColor();flags=Toolbox->KeyboardHit(ch,0);}
						IsSHIFTDown=0;
						if (i>=4096) break;
						if ((IsText==0) && (i>768)) break;
						if (ch==0) break;
						if (ch==10) continue;
						if (((ch==' ') || (ch==0x0D)) && (first_pass)) continue;
						if (IsText) 
						{
							InhibitAllKeyHandling=1;
						}
						else 
						{
							if ((i>0) && (i<len-2))
							{
								UINT ch1=(UINT)*((unsigned char*)pntr+i+1);
								UINT ch2=(UINT)*((unsigned char*)pntr+i-1);
								UINT ch3=(UINT)*((unsigned char*)pntr+i+2);
								if (((ch=='e') || (ch=='E')) && 
									((ch1=='+') || (ch1=='-')) && 
									(ch2>='0') && (ch2<='9') && (ch3>='0') && (ch3<='9'))
								{
									//handling strings like 1e+5
									((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,'*',0,flags,fcolor,0);
									((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,'1',0,flags,fcolor,0);
									((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,'0',0,flags,fcolor,0);
									((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,'^',0,flags,fcolor,0);
									if (ch1=='-')
										((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,'-',0,flags,fcolor,0);

									int is_pass=0;
									i+=2;
									int k=i+6;
									while ((i<k) && (i<len))
									{
										UINT ch=(UINT)*((unsigned char*)pntr+i);
										if (((ch<'0') || (ch>'9')) && (ch!='.') && ((ch!=',') || (!UseCommaAsDecimal)))
											{i--;break;}
										if ((ch!='0') || (is_pass))
										{
											((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,ch,0,flags,fcolor,0);
											is_pass=1;
										}
										i++;
									}
									((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,13,0,flags,fcolor,0);
									continue;
								}
								
							}
							if ((ch==' ') || (ch=='+') || (ch=='-') || (ch=='*') || (ch=='/') || 
								 (ch=='=') || (ch=='<') || (ch=='>') || (ch==',') || (ch==';') || (ch==9))
							{
								CExpression *p=(CExpression*)(KeyboardEntryObject);
								if (p)
								{
									CElement *e=p->m_pPaternalElement;
									if (e)
									{
										//exits high-order structures
										if (((e->m_Type==3)) || ((e->m_Type==6)) || ((e->m_Type==8)) ||
											((e->m_Type==1)) ||
											((e->m_Type==5) && (e->Expression2==KeyboardEntryObject)) ||
											((e->m_Type==7)))
											((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,13,0,flags,fcolor,0);
									}
								}
							}
							if (ch==0x0D) 
							{
								IsSHIFTDown=1;
								((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,13,0,flags,fcolor,0);
								IsSHIFTDown=0;
								continue;
							}
						}
						first_pass=0;
						((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,ch,0,flags,fcolor,0);
					}
					((CMainFrame*)theApp.m_pMainWnd)->UndoEnableSaving();
					ReleaseDC(DC);
					InhibitAllKeyHandling=0;
					InhibitParentheseMerging=0;
					GlobalUnlock(clipb_data);
					CloseClipboard();
					text_pasted=1;
				}

				
			}
		}
	}
	if ((TheKeyboardClipboard) &&  (!text_pasted) &&
		(((TheKeyboardClipboard->m_NumElements==1) && (TheKeyboardClipboard->m_pElementList->Type)) ||
		(TheKeyboardClipboard->m_NumElements>1)))
	{
		//UndoSave("paste");
		
		CDC *DC=GetDC();
		CExpression *e=((CExpression*)KeyboardEntryObject);
		
		short l,a,b;
		TheKeyboardClipboard->CalculateSize(DC,ViewZoom,&l,&a,&b);
		e->AdjustSelection(2);
		
		int is_A_table=0;if ((e->m_MaxNumColumns>1) || ((e->m_MaxNumRows>1))) is_A_table=1;
		int is_B_table=0;if ((TheKeyboardClipboard->m_MaxNumColumns>1) || ((TheKeyboardClipboard->m_MaxNumRows>1))) is_B_table=1;

		if ((!is_A_table) || (!is_B_table)) 
		{
			KeyboardSelectionCut(1);

			if (ClipboardExpression) delete ClipboardExpression;
			ClipboardExpression=new CExpression(NULL,NULL,100);
			ClipboardExpression->CopyExpression(TheKeyboardClipboard,0,1,0);
			short l,a,b;
			ClipboardExpression->CalculateSize(DC,ViewZoom,&l,&a,&b);
			e->KeyboardKeyHit(DC,ViewZoom,6,0,0,0,0);
			e->CalculateSize(DC,ViewZoom,&l,&a,&b);
			ReleaseDC(DC);
			this->ScrollCursorIntoView();
			return;
		}
		else 
		{
			//we are pasting a matrix into matrix - we will paste over rows/columns
			int row=0,col=0;
			for (int i=0;i<e->m_IsKeyboardEntry;i++)
			{
				if ((e->m_pElementList+i)->Type==11) col++;
				if ((e->m_pElementList+i)->Type==12) {col=0;row++;}
			}
			e->DeselectExpression();

			//add new rows and collumns if needed
			for (int i=0;i<TheKeyboardClipboard->m_MaxNumColumns-(e->m_MaxNumColumns-col);i++)
			{
				e->InsertEmptyElement(e->m_NumElements,11,0);
				e->InsertEmptyElement(e->m_NumElements,0,0);
			}
			e->AdjustMatrix();
			for (int i=0;i<TheKeyboardClipboard->m_MaxNumRows-(e->m_MaxNumRows-row);i++)
			{
				e->InsertEmptyElement(e->m_NumElements,12,0);
				e->InsertEmptyElement(e->m_NumElements,0,0);
			}

			//preselect them (the function CopyAtPoint will replace the selection)
			e->AdjustMatrix();
			for (int i=0;i<TheKeyboardClipboard->m_MaxNumRows;i++)
				for (int j=0;j<TheKeyboardClipboard->m_MaxNumColumns;j++)
					e->SelectMatrixElement(row+i,col+j,2);

			e->CopyAtPoint(DC,ViewZoom,0,0,TheKeyboardClipboard,2);
			ReleaseDC(DC);
			e->InsertEmptyElement(e->m_IsKeyboardEntry-1,1,0);
			e->m_KeyboardCursorPos=0;
			return;
		}
	}
}

#pragma optimize("s",on)
void CMathomirView::OnEditCopylatexcode()
{
	char *head="\\begin{array}{l}\r\n";
	char *foot="\r\n\\end{array}";
	int len=0;
	//len+=(int)strlen(head);
	for (int i=0;i<NumDocumentElements;i++)
	{		
		if (((TheDocument+i)->MovingDotState==3) && ((TheDocument+i)->Type==1))
		{
			
			len+=((CExpression*)((TheDocument+i)->Object))->LaTeX_output(NULL,1);
			len+=6;
		}		
	}
	//len+=(int)strlen(foot);

	HGLOBAL mem=GlobalAlloc(GMEM_MOVEABLE,len+256);
	char *pnt=(char*)GlobalLock(mem);
	//strcpy(pnt,head);pnt+=strlen(head);
	int start=0;
	for (int i=0;i<NumDocumentElements;i++)
	{		
		if (((TheDocument+i)->MovingDotState==3) && ((TheDocument+i)->Type==1))
		{
			if (start)
			{
				strcpy(pnt,"\\qquad \\\\\r\n\r\n");
				pnt+=6;	
			}
			start++;
			int tt=((CExpression*)((TheDocument+i)->Object))->LaTeX_output(pnt,0);
			pnt+=tt;
		
		}		
	}
	//strcpy(pnt,foot);pnt+=strlen(foot);
	*pnt=0;
	GlobalUnlock(mem);


	if ((theApp.m_pMainWnd->OpenClipboard()) && (EmptyClipboard()))
	{
		SetClipboardData(CF_TEXT,mem);
		CloseClipboard();
	}
}

void CMathomirView::OnKeyboardF1setszoomlevelto100()
{
	if (F1SetsZoom) F1SetsZoom=0; else F1SetsZoom=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnOutputimagePrintfontsasimages()
{
	if (PrintTextAsImage) PrintTextAsImage=0; else PrintTextAsImage=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

#pragma optimize("",on)

void CMathomirView::OnViewZoomto1()
{
	DWORD t,t2;
	if ((IsHalftoneRendering) && (IsHighQualityRendering) && (ViewZoom>300)) ViewZoom=300; //for speed
	
	while (ViewZoom!=DefaultZoom)
	{
		t=GetTickCount();
		if (ViewZoom>DefaultZoom) OnViewZoomout(); else OnViewZoomin();
		t2=40+t-GetTickCount();
		if (t2<10) continue;
		if (t2>40) t2=0;
		Sleep(t2);
	}
	
}



/*void CMathomirView::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CView::OnActivate(nState, pWndOther, bMinimized);

	// TODO: Add your message handler code here
}*/

BOOL CMathomirView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default
	POINT pt;
	RECT r;
	GetCursorPos(&pt);
	//ScreenToClient(&pt);
	GetWindowRect(&r);
	int width=GetSystemMetrics(SM_CXVSCROLL)+GetSystemMetrics(SM_CYFIXEDFRAME);
	if ((pt.x>r.right-width) && (pt.y<r.bottom-width) && (!UseCTRLForZoom))
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_SCROLL);
		SetCursor(hc);		
		MouseOverScrollbar=1;
		return true;
	}
	else
		return CView::OnSetCursor(pWnd, nHitTest, message);
}
extern int StaticMessageWindowTextclr;
extern int StaticMessageWindowBkclr;
HBRUSH CMathomirView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	HBRUSH hbr = CView::OnCtlColor(pDC, pWnd, nCtlColor);


	// TODO:  Change any attributes of the DC here
	if  (pWnd->m_hWnd==StaticMessageWindow->m_hWnd)
	{
		pDC->SetTextColor(StaticMessageWindowTextclr);
		pDC->SetBkColor(StaticMessageWindowBkclr);
		
	}

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

void CMathomirView::OnKeyboardAllowcommaasdecimalseparator()
{
	if (UseCommaAsDecimal) UseCommaAsDecimal=0;else UseCommaAsDecimal=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

/*void CMathomirView::OnSelectionsWidekeyboardcursor()
{
	if (UseWideCursor) UseWideCursor=0; else UseWideCursor=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}*/

void CMathomirView::OnGridandguidelinesSnaptoguidelines()
{
	if (SnapToGuidlines) SnapToGuidlines=0; else SnapToGuidlines=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}



void CMathomirView::OnMenu32880()
{
	//search for something
	if (!SearchPopup)
	{
		SearchPopup=new PopupMenu();
		SearchPopup->CreateEx(WS_EX_TOPMOST,AfxRegisterWndClass(CS_OWNDC),"MoM search",WS_CLIPCHILDREN | WS_POPUP,5,5,10,10,theApp.m_pMainWnd->m_hWnd,NULL,0);

	}
	SearchPopup->ShowPopupMenu(NULL,this,9,0);
	// TODO: Add your command handler code here
}

afx_msg void CMathomirView::OnNcRButtonDown(UINT nHitTest,CPoint point )
{
	if (nHitTest==HTVSCROLL) 
	{
		ScreenToClient(&point);
		DetermineTillensData(point.y);
		Tillens.Show=1;
		RepaintTheView();
	}
	else
		CWnd::OnNcRButtonDown(nHitTest,point);
}



void CMathomirView::OnToolboxandcontextmenuShowtoolbar()
{
	if (UseToolbar) UseToolbar=0;else UseToolbar=1;
	if ((AutoResizeToolbox) && (Toolbox)) Toolbox->AutoResize();
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	Toolbox->AdjustPosition();
	this->AdjustPosition();
}


void CMathomirView::OnKeyboardUsecapslocktotoggletypingmode()
{
	if (UseCapsLock) UseCapsLock=0; else UseCapsLock=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}


void CMathomirView::OnToolboxandcontextmenuGigantic()
{
	BaseToolboxSize=ToolboxSize=120;
	AutoResizeToolbox=0;
	Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
	Toolbox->AdjustPosition();
	Toolbox->InvalidateRect(NULL,0);
	Toolbox->UpdateWindow();
	AdjustPosition();
	AdjustMenu();
	RepaintTheView();
	theApp.m_pMainWnd->SetActiveWindow();
}


void CMathomirView::OnMouseReversemousewheelscrollingdirection()
{
	if (MouseWheelDirection) MouseWheelDirection=0; else MouseWheelDirection=1;
	AdjustMenu();
}

void CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis150()
{
	DefaultZoom=150;
	AdjustMenu();
	if ((Toolbox) && (Toolbox->Toolbar)) {Toolbox->ConfigureToolbar();}
	ViewZoom=DefaultZoom;
	RepaintTheView(1);
}

void CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis120()
{
	DefaultZoom=120;
	AdjustMenu();
	if ((Toolbox) && (Toolbox->Toolbar)) {Toolbox->ConfigureToolbar();}
	ViewZoom=DefaultZoom;
	RepaintTheView(1);
}

void CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis100()
{
	DefaultZoom=100;
	AdjustMenu();
	if ((Toolbox) && (Toolbox->Toolbar)) {Toolbox->ConfigureToolbar();}
	ViewZoom=DefaultZoom;
	RepaintTheView(1);
}

void CMathomirView::OnFontsizeanddefaultzoomDefaultzoomis80()
{
	DefaultZoom=80;
	AdjustMenu();
	if ((Toolbox) && (Toolbox->Toolbar)) {Toolbox->ConfigureToolbar();}
	ViewZoom=DefaultZoom;
	RepaintTheView(1);
}

void CMathomirView::OnZoomUsectrlforwheelzoom() //mouse function: scroll up/down
{
	UseCTRLForZoom=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnMouseMouse() //mouse function: zoom in/ou
{
	UseCTRLForZoom=0;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnMouseRightmousebuttontotogglemouse()
{
	if (RightButtonTogglesWheel) RightButtonTogglesWheel=0; else RightButtonTogglesWheel=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnMouseSlow()
{
	if (WheelScrollingSpeed<50) WheelScrollingSpeed=60; else WheelScrollingSpeed=30;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();

}

void CMathomirView::OnPagenumerationNone()
{
	PageNumeration=PageNumeration&0xFFFFFFF0;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnPagenumeration()
{
	PageNumeration=(PageNumeration&0xFFFFFFF0)|0x01;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnPagenumeration32899()
{
	PageNumeration=(PageNumeration&0xFFFFFFF0)|0x02;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnPagenumerationPage1of10()
{
	PageNumeration=(PageNumeration&0xFFFFFFF0)|0x03;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnPagenumerationBottom()
{
	int is_bottom=PageNumeration&0x10;
	if (is_bottom) PageNumeration&=0xFFFFFFEF; else PageNumeration|=0x10;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnPagenumerationRight()
{
	int is_right=PageNumeration&0x20;
	if (is_right) PageNumeration&=0xFFFFFFDF; else PageNumeration|=0x20;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnPagenumerationExcludefirstpage()
{
	int is_excludefirst=PageNumeration&0x40;
	if (is_excludefirst) PageNumeration&=0xFFFFFFBF; else PageNumeration|=0x40;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();

	InvalidateRect(NULL,0);
	UpdateWindow();
}

void CMathomirView::OnToolboxandcontextmenuAuto()
{
	AutoResizeToolbox=1;
	if (Toolbox) Toolbox->AutoResize();
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnMenu32905()
{
	if ((!Toolbox) || (!Toolbox->Keyboard)) return;

	if (Toolbox->Keyboard->IsWindowVisible())
		Toolbox->Keyboard->ShowWindow(SW_HIDE);
	else
	{
		NoImageAutogeneration=2;
		Toolbox->Keyboard->AdjustPosition();
		Toolbox->Keyboard->InvalidateRect(NULL,0);
		Toolbox->AdjustKeyboardFont();
		NoImageAutogeneration=2;
		Toolbox->Keyboard->ShowWindow(SW_SHOW);
	}
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}

void CMathomirView::OnKeyboardUsecomplexindexes()
{
	if (UseComplexIndexes) UseComplexIndexes=0; else UseComplexIndexes=1;
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
}
