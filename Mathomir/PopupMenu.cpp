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

#include "PopupMenu.h"
#include ".\popupmenu.h"
#include "mainfrm.h"
#include "toolbox.h"
#include "drawing.h"
#include "math.h"


extern short CElementInitType; //passed through global variable for speed
extern CObject *CElementInitPaternalExpression; //passed through global variable for speed
extern RECT TheClientRect;
extern CExpression* ClipboardExpression;
extern CMathomirView *pMainView;
extern int PlusLevel;
extern int MulLevel;
extern int EqLevel;
extern unsigned char OperatorLevelTable[256];
extern char* TheKeycodeString;
#define GetOperatorLevel(x) ((int)OperatorLevelTable[(unsigned char)(x)])
extern int QuickSelectActive; 
extern char FontAdditionalData;
extern CExpression *prevClipboardExpression;
extern CExpression *BackspaceStorage;

int TSize,TSize_1p2,TSize_1p3,TSize_2p3,TSize_1p20, TSize_1p25;

int SearchStartPosition;
unsigned int LocalLinks[64];
// ***************************************************************
// PopupMenu calculation thread (only calls Yield_calc function)
// ***************************************************************
DWORD CalcThreadID=0;
HANDLE CalcThreadHandle;
DWORD WINAPI CalcThread(LPVOID lpParameter)
{
	PopupMenu *Menu=(PopupMenu*)lpParameter;
	Sleep(150);  //without this it doesn't work (strange!!!!) - the popup menu must frist be displayed, then we can add calculated options
	Menu->SymbolicComputation();
	CalcThreadID=0;
	if ((Menu->m_OwnerType==3) && (Menu->m_SelectedOption==-1))
		{Menu->m_SelectedOption=0;Menu->PaintThePopupMenu();}
	ExitThread(0);
}



IMPLEMENT_DYNAMIC(PopupMenu, CWnd)
PopupMenu::PopupMenu()
{
	m_Owner=NULL;
/*int Popup_CursorX;
int Popup_CursorY;
unsigned char entry_box_first_call;
char PopupMenuSecondPassChoosing;
CEdit *ValueEntryBox;
int ValueEntryBoxData;
char ValueEntryBoxString[300];*/
	Popup_CursorX=Popup_CursorY=0;
	entry_box_first_call=0;
	PopupMenuSecondPassChoosing=0;
	ValueEntryBox=NULL;
	ValueEntryBoxData=0;
	ValueEntryBoxString[0]=0;

}

int PopupOption_Y;

#pragma optimize("s",on)
int PopupMenu::AddMenuOption(int X, int Cx, char * text, int Data, int new_line)
{
	Options[m_NumOptions].Y=PopupOption_Y;
	Options[m_NumOptions].X=X;
	Options[m_NumOptions].Cx=Cx;
	Options[m_NumOptions].Cy=TSize/4;
	CopyTranslatedString(Options[m_NumOptions].Text,text,Data,40);
	/*if (strlen(text)<32) 
		strcpy(Options[m_NumOptions].Text,text);
	else
	{
		memcpy(Options[m_NumOptions].Text,text,31);
		Options[m_NumOptions].Text[31]=0;
	}*/
	Options[m_NumOptions].IsChecked=0;
	Options[m_NumOptions].IsEnabled=(Data>=0)?1:0;
	Options[m_NumOptions].Data=Data;
	if (new_line) PopupOption_Y+=Options[m_NumOptions].Cy;
	m_NumOptions++;
	return 0;
}

#pragma optimize("s",on)
int PopupMenu::AddMenuOptionButton(int X, char * text, int Data, int button_ndx, int new_line)
{
	Options[m_NumOptions].Y=PopupOption_Y;
	Options[m_NumOptions].X=X;
	Options[m_NumOptions].Cx=(TSize<70)?17:((TSize<80)?19:25); 
	Options[m_NumOptions].Cy=Options[m_NumOptions].Cx; 
	CopyTranslatedString(Options[m_NumOptions].Text,text,Data,40);
	
	//strcpy(Options[m_NumOptions].Text,text);
	Options[m_NumOptions].IsChecked=0;
	Options[m_NumOptions].IsEnabled=(Data>=0)?1:0;
	Options[m_NumOptions].Data=Data;
	Options[m_NumOptions].IsButton=button_ndx;
	if (new_line) PopupOption_Y+=Options[m_NumOptions].Cy+TSize_1p25;
	m_NumOptions++;
	return 0;
}

#pragma optimize("s",on)
int PopupMenu::AddCheckedMenuOptionButton(int X, char * text, int is_checked, int Data, int button_ndx, int new_line)
{
	Options[m_NumOptions].Y=PopupOption_Y;
	Options[m_NumOptions].X=X;
	Options[m_NumOptions].Cx=(TSize<70)?17:((TSize<80)?19:25); 
	Options[m_NumOptions].Cy=Options[m_NumOptions].Cx; 
	//strcpy(Options[m_NumOptions].Text,text);
	CopyTranslatedString(Options[m_NumOptions].Text,text,Data,32);

	Options[m_NumOptions].IsChecked=2+((is_checked)?1:0);
	Options[m_NumOptions].IsEnabled=(Data>=0)?1:0;
	Options[m_NumOptions].Data=Data;
	Options[m_NumOptions].IsButton=button_ndx;
	if (new_line) PopupOption_Y+=Options[m_NumOptions].Cy+TSize/15;
	m_NumOptions++;
	return 0;
}

#pragma optimize("s",on)
int PopupMenu::AddCheckedMenuOption(int X, int Cx, char * text, int is_checked, int Data, int new_line)
{
	Options[m_NumOptions].Y=PopupOption_Y;
	Options[m_NumOptions].X=X;
	Options[m_NumOptions].Cx=Cx;
	Options[m_NumOptions].Cy=TSize/4;
	//strcpy(Options[m_NumOptions].Text,text);
	CopyTranslatedString(Options[m_NumOptions].Text,text,Data,32);
	Options[m_NumOptions].IsChecked=2+((is_checked)?1:0);
	Options[m_NumOptions].IsEnabled=(Data>=0)?1:0;
	Options[m_NumOptions].Data=Data;
	if (new_line) PopupOption_Y+=Options[m_NumOptions].Cy;
	m_NumOptions++;
	return 0;
}


int YorderQSort(const void *first, const void *second)
{
	//sorts documents object on Y coordinate
	tDocumentStruct *f=*(tDocumentStruct**)first;
	tDocumentStruct *s=*(tDocumentStruct**)second;

	if (f->absolute_Y<s->absolute_Y) return -1;
	if (f->absolute_Y>s->absolute_Y) return 1;
	return 0;

}

CExpression *ExtractedSelection=NULL;
extern int RullerPositionPreselected;
int DeletableGuidelineObject;
extern CToolbox *Toolbox;
int EasycastListStart=0;


#pragma optimize("s",on)
int PopupMenu::ShowPopupMenu(CExpression* expression, CWnd*owner, int OwnerType,int UserParam,int no_reposition)
{
	PopupMenuSecondPassChoosing=0;

	MovingMode=0;
	TSize=(ToolboxSize)?ToolboxSize:BaseToolboxSize;
	if (TSize<60) TSize=60;
	if (TSize>60) 
	{
		TSize=72;
		if (ToolboxSize<200)
		{
			TSize=60+max(ToolboxSize-60,0)/2;
		}
	}
	TSize_1p2=TSize/2;
	TSize_1p3=TSize/3;
	TSize_2p3=2*TSize/3;
	TSize_1p20=TSize/20;
	TSize_1p25=TSize/25;

	if (ValueEntryBox)
	{
		ValueEntryBox->DestroyWindow();
		delete ValueEntryBox;
		ValueEntryBox=0;
	}
	m_Owner=owner;
	m_OwnerType=OwnerType;
	m_theSelectedElement=NULL;
	m_UserParam=UserParam;

	m_Expression=expression;
	if (!no_reposition) m_IsFirstPass=1; else m_IsFirstPass=2;

	POINT cursor;
	GetCursorPos(&cursor);
	pMainView->ScreenToClient(&cursor);
	Popup_CursorX=cursor.x*100/ViewZoom+ViewX;
	Popup_CursorY=cursor.y*100/ViewZoom+ViewY;
	int force_calculator=0;



	/*if (OwnerType==10) //stack-clipoard
	{
		m_NumOptions=0;
		memset((void*)&Options,0,sizeof(Options));
		PopupOption_Y=TSize_1p20;
		int ZoomLvl=80*TSize/60;

		for (int i=1;i<5;i++)
			if (prevClipboardExpression[i])
			{
				short l,a,b;
				CDC *dcc=GetDC();
				prevClipboardExpression[i]->CalculateSize(dcc,ZoomLvl,&l,&a,&b);
				ReleaseDC(dcc);

				Options[m_NumOptions].Y=PopupOption_Y;
				Options[m_NumOptions].X=TSize/8;
				Options[m_NumOptions].Cx=l+TSize/4;
				Options[m_NumOptions].Cy=a+b+TSize/8;
				strcpy(Options[m_NumOptions].Text,"");
				Options[m_NumOptions].IsButton=0;
				Options[m_NumOptions].Graphics=prevClipboardExpression[i];
				Options[m_NumOptions].IsChecked=0;
				Options[m_NumOptions].IsEnabled=1;
				Options[m_NumOptions].IsGraphicsSensitive=0;
				Options[m_NumOptions].Data=110+i-1; 
				Options[m_NumOptions].DataArray[0]=l;
				Options[m_NumOptions].DataArray[1]=a;
				Options[m_NumOptions].DataArray[2]=b;
				Options[m_NumOptions].DataArray[3]=ZoomLvl;
				Options[m_NumOptions].DataArray[4]=TSize/8;  //X coordinate of expression
				Options[m_NumOptions].DataArray[5]=PopupOption_Y+a+TSize/8;  //Y coordinae of expression
				PopupOption_Y+=Options[m_NumOptions].Cy;
				m_NumOptions++;
			}
		PopupOption_Y+=TSize_1p20;
		goto popupmenu_end_showpopup;
	}*/

	if (OwnerType==9) //called from menu "search"
	{
		m_NumOptions=0;
		memset((void*)&Options,0,sizeof(Options));

		PopupOption_Y=TSize_1p20;

		SearchStartPosition=-1;
		AddMenuOption(TSize_1p3,2*TSize+TSize/2,"String search:",-801,1);
		PopupOption_Y+=30;
		AddMenuOption(TSize_1p3,TSize,"down",801,0);

		ValueEntryBox=new CEdit();
		ValueEntryBox->Create(ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_VISIBLE | WS_CHILD | WS_BORDER,CRect(Options[1].X,Options[1].Y-1-20,Options[1].X+2*TSize,Options[1].Y+Options[1].Cy-20+2),this,1);
		HFONT font=GetFontFromPool(4,0,1,TSize/4-2);
		ValueEntryBox->SendMessage(WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
		ValueEntryBox->SetFocus();
		ValueEntryBoxData=1;
		//CWnd::OnLButtonDown(nFlags, point);
		AddMenuOption(TSize+TSize/2+TSize_1p3,TSize,"up",802,1);
		PopupOption_Y+=10;

		goto popupmenu_end_showpopup;
		
	}

	if (OwnerType==8) //adding guidelines from ruller
	{
		m_NumOptions=0;
		memset((void*)&Options,0,sizeof(Options));

		//check if we clicked at already defined position
		int i;
		int deletable=0;
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
						deletable=1;
						DeletableGuidelineObject=i;
						break;
					}
				}
			}
		}

		PopupOption_Y=TSize_1p20;
		if (deletable)
			AddMenuOption(TSize_1p3,3*TSize,"Delete guideline",823,1);
		else if (NumRullerGuidelines<12)
		{
			AddMenuOption(TSize_1p3,3*TSize,"Normal guideline",820,1);
			AddMenuOption(TSize_1p3,3*TSize,"Text guideline",821,1);
			AddMenuOption(TSize_1p3,3*TSize,"Text autowrap guideline",822,1);
		}
		//PopupOption_Y+=10;
		goto popupmenu_end_showpopup;
	}

	if (OwnerType==6) //column/row insertion point
	{
		m_NumOptions=0;
		memset((void*)&Options,0,sizeof(Options));

		PopupOption_Y=3*TSize_1p20;
		AddMenuOption(0,3*TSize,"Line type:",-601,1);

		AddMenuOption(TSize_1p3,3*TSize,"No line",601,1);
		AddMenuOption(TSize_1p3,3*TSize,"Single line",602,1);
		AddMenuOption(TSize_1p3,3*TSize,"Double line",603,1);

		goto popupmenu_end_showpopup;
	}

	if (OwnerType==7) //defining keycode scans for accelerated typing
	{
		m_NumOptions=0;
		memset((void*)&Options,0,sizeof(Options));
		PopupOption_Y=3*TSize_1p20;
		AddMenuOption(0,2*TSize+TSize/2,"Easycast string:",-831,1);
		PopupOption_Y+=1*TSize_1p20;
		AddMenuOption(TSize_1p3+TSize,1*TSize,"Accept",830,1);

		ValueEntryBox=new CEdit();
		ValueEntryBox->Create(ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_VISIBLE | WS_CHILD | WS_BORDER,CRect(TSize_1p3,Options[1].Y-1,TSize_1p3+TSize,Options[1].Y+Options[1].Cy+2),this,1);
		HFONT font=GetFontFromPool(4,0,1,TSize/4-2);
		ValueEntryBox->SendMessage(WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
		ValueEntryBox->SetFocus();
		ValueEntryBox->SetWindowText(TheKeycodeString);
		ValueEntryBoxData=1;

		PopupOption_Y+=TSize_1p3;
		AddMenuOption(0,2*TSize+TSize/2,"All defined Easycasts:",-832,1);

		
		int k=0;
		while (1)
		{
			char *keycode;
			CExpression *graphics=Toolbox->ReturnKeycode(EasycastListStart,&keycode);
			if (graphics==NULL) break;
			AddMenuOption(TSize_1p3,TSize,keycode,-802-k,0);

			CExpression *image=new CExpression(0,0,100);
			image->CopyExpression(graphics,0);

			short l,a,b;
			CDC *dcc=GetDC();
			image->CalculateSize(dcc,70,&l,&a,&b);
			ReleaseDC(dcc);

			Options[m_NumOptions].Y=PopupOption_Y;
			Options[m_NumOptions].X=TSize+ToolboxSize;
			Options[m_NumOptions].Cx=l+2;
			Options[m_NumOptions].Cy=a+b+TSize/8;
			strcpy(Options[m_NumOptions].Text,"");
			Options[m_NumOptions].IsButton=0;
			Options[m_NumOptions].Graphics=image;
			Options[m_NumOptions].IsChecked=0;
			Options[m_NumOptions].IsEnabled=0;
			Options[m_NumOptions].IsGraphicsSensitive=0;
			Options[m_NumOptions].Data=-902-k; //
			Options[m_NumOptions].DataArray[0]=l;
			Options[m_NumOptions].DataArray[1]=a;
			Options[m_NumOptions].DataArray[2]=b;
			Options[m_NumOptions].DataArray[3]=70;
			Options[m_NumOptions].DataArray[4]=TSize;  //X coordinate of expression
			Options[m_NumOptions].DataArray[5]=PopupOption_Y+a+TSize/8;  //Y coordinae of expression
			PopupOption_Y+=Options[m_NumOptions].Cy;
			Options[m_NumOptions-1].Y+=a-ToolboxSize/40;
			m_NumOptions++;

			k++;
			EasycastListStart++;
			if (k>3) 
			{
				if (Toolbox->ReturnKeycode(EasycastListStart,&keycode))
				{
					PopupOption_Y+=1*TSize_1p20;
					AddMenuOption(TSize_1p3,TSize,"more...",831,1);
				}
				break;
			}
		}
		goto popupmenu_end_showpopup;
	}

	m_HaveCutDel=0;
	if ((OwnerType==1))
	{
		m_HaveCutDel=1;
	}

	int AddPasteSpecial=0;
	PopupOption_Y=TSize_1p20;


	//create/delete a helping expression that will store the selection (for symbolic-computation purposes)
	if (ExtractedSelection==NULL) ExtractedSelection=new CExpression(NULL,NULL,100);
	ExtractedSelection->Delete();



	if ((m_Expression==NULL) && (m_OwnerType==2))
	{
		//the drawing popup menu
		//options: delete, line size, group, ungroup, node edit?, sizing, rotating, mirroring, 
		//         lock, unlock, arrange (left,center,right, top, center, ..)

		CDrawing *drw;
		tDocumentStruct *dss=NULL;
		int NumDrawings=0;
		int NumExpressions=0;
		int Any_uncombineable=0;
		int Any_locked=0;
		int Any_unlocked=0;
		int Any_nonselected=0;
		int common_color=100;
		for (int ii=0;ii<NumDocumentElements;ii++)
		{
			tDocumentStruct *ds=TheDocument+ii;
			if (ds->Object)
			{
				if (ds->Type==2)
					if ((((CDrawing*)(ds->Object))->IsSelected) || (ds->MovingDotState==3))
					{
						NumDrawings++;
						if (ds->MovingDotState!=3) Any_nonselected=1;
						if (ds->MovingDotState==5) Any_locked++; else Any_unlocked++;
						drw=(CDrawing*)(ds->Object);
						if (drw)
						{
							if (common_color==100) common_color=drw->m_Color;
							if (drw->m_Color!=common_color) common_color=-100;
							if (drw->IsSpecialDrawing) Any_uncombineable=1;
							if ((drw->Items) && (drw->Items->Type!=1)) Any_uncombineable=1;
						}
						dss=ds;
					}
				if (ds->Type==1)
					if ((((CExpression*)(ds->Object))->m_Selection==0x7FFF) || (ds->MovingDotState==3))
					{
						if (ds->MovingDotState==3) force_calculator=1;
						NumExpressions++;
						if (ds->MovingDotState!=3) Any_nonselected=1;
						if (ds->MovingDotState==5) Any_locked++; else Any_unlocked++;
						if (ds->Object)
						{
							if (common_color==100) common_color=((CExpression*)(ds->Object))->m_Color;
							if (((CExpression*)(ds->Object))->m_Color!=common_color) common_color=-100;
						}
						dss=ds;
					}
			}
		}

		m_NumOptions=0;
		memset((void*)&Options,0,sizeof(Options));

		//the 'delete' option
		AddMenuOption(TSize/3,TSize,"Delete ",501,0);
		if (common_color!=-1) common_color=common_color&0xF7;
			AddCheckedMenuOption(TSize/8+4*TSize/3-TSize/10,TSize/3+2,"A ",(common_color==-1)?1:0,580,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3+TSize/3,TSize/3,"Red ",(common_color==1)?1:0,582,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3+TSize_2p3,TSize_1p2,"Blu ",(common_color==3)?1:0,584,1);


		if (Any_nonselected) 
			AddMenuOption(TSize/3,TSize,"Select ",570,0);
		else
		{
			if (NumExpressions+NumDrawings==1)
				AddCheckedMenuOption(TSize/3-TSize/5+1,TSize,"Lock ",Any_locked,567,0);
			else 
				if (Any_unlocked) AddMenuOption(TSize/3,TSize,"Lock all  ",568,0);
		}

		AddCheckedMenuOption(TSize/8+4*TSize/3,TSize/3,"Blk ",(common_color==0)?1:0,581,0);
		AddCheckedMenuOption(TSize/8+4*TSize/3+TSize/3,TSize/3,"Grn ",(common_color==2)?1:0,583,0);
		AddCheckedMenuOption(TSize/8+4*TSize/3+TSize_2p3,TSize/3,"Gry ",(common_color==4)?1:0,585,1);

		if (Any_nonselected)
		{
			if (NumExpressions+NumDrawings==1)
			{
				int fill_option=0;
				if (NumExpressions==0)
				{
					char is_closed;
					((CDrawing*)(dss->Object))->IsOpenPath(0,&is_closed);
					if (is_closed) fill_option=1;
				}
				AddCheckedMenuOption(TSize/3-TSize/5+1,2*TSize_1p2,"Lock ",Any_locked,567,(fill_option==1)?0:1);
				if (fill_option)	AddCheckedMenuOption(5*TSize/3+TSize/6,1*TSize_1p2,"Fill ",((((CDrawing*)(dss->Object))->m_Color&0xF8)==8)?1:0,589,1);
			}
			else
				if (Any_unlocked) AddMenuOption(TSize/3,3*TSize_1p2,"Lock all  ",568,1);
		}
		if ((Any_locked) && (NumExpressions+NumDrawings>1))
			AddMenuOption(TSize/3,3*TSize_1p2,"Unlock all  ",569,1);




		if (NumDrawings)
		{
			PopupOption_Y+=TSize/10;
			AddMenuOption(0,5*TSize_1p2,"Line width:",-502,1);
			AddMenuOptionButton(1*TSize/3,"Hair ",576,66,0);
			AddMenuOptionButton(TSize_2p3,"Thin ",502,67,0);
			AddMenuOptionButton(TSize,"3/2 ",577,77,0);
			AddMenuOptionButton(4*TSize/3,"Medium ",503,68,0);
			AddMenuOptionButton(5*TSize/3,"Thick ",504,69,0);
			AddMenuOptionButton(2*TSize,"Fat ",566,70,1);
			if ((NumDrawings==1) && (NumExpressions==0) && (dss) && (((CDrawing*)dss->Object)->IsSpecialDrawing==0))
			{
				char is_closed=0;
				int is_open=((CDrawing*)(dss->Object))->IsOpenPath(0,&is_closed,NULL);
				if ((is_open) || (is_closed) || (((CDrawing*)(dss->Object))->NumItems==1))
				{
					AddMenuOption(TSize/3,TSize_2p3+TSize/3,"dash-dash ",591,0);
					AddMenuOption(TSize+TSize/2-2,TSize_2p3+6,"dash-dot ",592,1);
				}
			}

		}


		if (NumDrawings+NumExpressions>1)
		{
			PopupOption_Y+=TSize/10;
			AddMenuOption(0,5*TSize_1p2,"Arrange:",-560,1);

			AddMenuOptionButton(1*TSize/3,"Left align",560,71,0);
			AddMenuOptionButton(TSize_2p3,"H. center align",562,72,0);
			AddMenuOptionButton(TSize,"Right align",564,73,0);
			AddMenuOptionButton(4*TSize/3+TSize/8,"Top align",561,74,0);
			AddMenuOptionButton(5*TSize/3+TSize/8,"V. center align",563,75,0);
			AddMenuOptionButton(6*TSize/3+TSize/8,"Bottom align",565,76,1);

			PopupOption_Y+=TSize_1p20;
			AddMenuOptionButton(6*TSize/3+TSize/8,"Vertical distribute",578,82,0);
			if ((NumExpressions>1) && (NumDrawings==0))
				AddMenuOptionButton(5*TSize/3+TSize/8,"Align to equal sign",579,80,0);
			AddMenuOption(TSize/3,TSize,"Group ",505,1);


			if ((NumExpressions==0) && (NumDrawings>1) && (Any_uncombineable==0))
							AddMenuOption(TSize/3,TSize,"Combine ",514,1);

		}
		else if ((dss->Type==2) && (dss->Object) && (((CDrawing*)(dss->Object))->Items) &&
			(((CDrawing*)(dss->Object))->Items->Type!=1))
		{
			PopupOption_Y+=TSize/10;
			AddMenuOption(0,5*TSize_1p2,"Arrange:",-560,1);
			AddMenuOption(TSize/3,TSize,"Ungroup ",506,1);
		}
		else if ((NumDrawings==1) && (NumExpressions==0) && (Any_uncombineable==0) && (dss->Type==2) && (dss->Object) &&
			(((CDrawing*)(dss->Object))->NumItems>1))
		{
			PopupOption_Y+=TSize/10;
			AddMenuOption(0,5*TSize_1p2,"Arrange:",-560,1);
			AddMenuOption(TSize/3,TSize,"Break apart ",513,1);
		}
	

		PopupOption_Y+=TSize/10;
		AddMenuOption(0,4*TSize_1p2-TSize/5,"Size:",-507,0);
		AddMenuOption(6*TSize/3-TSize/5,TSize*2/3,"??",515,1);
		AddMenuOptionButton(TSize/3,"-50% ",507,33,0);
		AddMenuOptionButton(TSize_2p3,"-15% ",508,34,0);
		AddMenuOptionButton(TSize,"-5% ",509,35,0);
		AddMenuOptionButton(4*TSize/3+TSize/8,"+5% ",510,36,0);
		AddMenuOptionButton(5*TSize/3+TSize/8,"+15% ",511,37,0);
		AddMenuOptionButton(6*TSize/3+TSize/8,"+50% ",512,38,1);


		if (NumDrawings>0)
		{
			PopupOption_Y+=TSize_1p20;
			AddMenuOptionButton(TSize/3,"H. stretch -50% ",517,42,0);
			AddMenuOptionButton(TSize_2p3,"H. stretch -15% ",518,43,0);
			AddMenuOptionButton(TSize,"H. stretch -5% ",519,44,0);
			AddMenuOptionButton(4*TSize/3+TSize/8,"H. stretch +5% ",520,45,0);
			AddMenuOptionButton(5*TSize/3+TSize/8,"H. stretch +15% ",521,46,0);
			AddMenuOptionButton(6*TSize/3+TSize/8,"H. stretch +50% ",522,47,1);

			PopupOption_Y+=TSize_1p20;

			AddMenuOptionButton(TSize/3,"V. stretch -50% ",527,48,0);
			AddMenuOptionButton(TSize_2p3,"V. stretch -15% ",528,49,0);
			AddMenuOptionButton(TSize,"V. stretch -5% ",529,50,0);
			AddMenuOptionButton(4*TSize/3+TSize/8,"V. stretch +5% ",530,51,0);
			AddMenuOptionButton(5*TSize/3+TSize/8,"V. stretch +15% ",531,52,0);
			AddMenuOptionButton(6*TSize/3+TSize/8,"V. stretch +50% ",532,53,1);

			if ((NumDrawings>1) || (dss==NULL) || (((CDrawing*)(dss->Object))->IsSpecialDrawing==0))
			{

				PopupOption_Y+=TSize_1p20;
				AddMenuOptionButton(1*TSize/3,"Horizontal mirror ",535,54,0);
				AddMenuOptionButton(TSize_2p3,"Vertical mirror ",536,55,1);

				PopupOption_Y+=TSize/10;
				AddMenuOption(0,3*TSize_1p2,"Rotate:",-536,0);
				AddMenuOption(5*TSize/3,TSize*2/3,"??",573,1);

				AddMenuOptionButton(1*TSize/3,"+5° ",540,56,0);
				AddMenuOptionButton(TSize_2p3,"+15° ",541,57,0);
				AddMenuOptionButton(TSize,"+45° ",542,58,0);
				AddMenuOptionButton(4*TSize/3,"+60° ",574,59,0);
				AddMenuOptionButton(5*TSize/3,"+90° ",571,60,1);
				AddMenuOptionButton(1*TSize/3,"-5° ",543,61,0);
				AddMenuOptionButton(TSize_2p3,"-15° ",544,62,0);
				AddMenuOptionButton(TSize,"-45° ",545,63,0);
				AddMenuOptionButton(4*TSize/3,"-60° ",575,64,0);
				AddMenuOptionButton(5*TSize/3,"-90° ",572,65,1);
			}
		}

		if ((NumExpressions==0) && (NumDrawings==1) && (dss))
		{

		

		}

		//the 'Edit nodes' option
		if ((NumDrawings==1) && (NumExpressions==0))
		{
			PopupOption_Y+=TSize/10;
			if (dss)
			{
				int is_open=((CDrawing*)(dss->Object))->IsOpenPath(0);
				if (is_open)
				{
					AddMenuOption(TSize/3,TSize*2+TSize_1p2,"Close path ",593,1);
				}
				if ((((CDrawing*)(dss->Object))->OriginalForm==4) || //circle
					(((CDrawing*)(dss->Object))->OriginalForm==16)||//6-polygon
					(((CDrawing*)(dss->Object))->OriginalForm==17)) //center drawn circle
				{
					AddMenuOption(TSize/3,TSize*2+TSize_1p2,"Add center point ",590,1);
					//AddMenuOption(TSize/3,TSize*2,"Add radius line ",591,1);
					//AddMenuOption(TSize/3,TSize*2,"Add diameter line ",592,1);
				}
				if ((((CDrawing*)(dss->Object))->OriginalForm==9) ||  //xy graph
					(((CDrawing*)(dss->Object))->OriginalForm==8))    //xy graph
				{
					AddMenuOption(TSize/3,TSize*2+TSize_1p2,"Add grid lines ",594,1);
				}
								
				
				//if ((((CDrawing*)(dss->Object))->OriginalForm==2) ||  //line
				//	(((CDrawing*)(dss->Object))->OriginalForm==18))   //section divider
				if ((is_open) || ((((CDrawing*)dss->Object)->NumItems==1) && (((CDrawing*)dss->Object)->Items->Type==1)))
				{
					//AddMenuOption(TSize/3,TSize*2+TSize_1p2,"Add arrows ",595,1);
					PopupOption_Y+=TSize/10;
					AddMenuOption(0,3*TSize_1p2,"Ending(s):",-595,1);
					AddMenuOptionButton(1*TSize/3,"arrow ",595,91,0);
					AddMenuOptionButton(2*TSize/3,"arrow (narrow) ",596,92,0);
					AddMenuOptionButton(3*TSize/3,"dot",597,93,1);
				}
			}

			if ((dss==NULL) || (((CDrawing*)(dss->Object))->IsSpecialDrawing==0))
			{
				//AddCheckedMenuOption(TSize/3-TSize/5,2*TSize+TSize_1p2,"Edit nodes  ",drw->IsNodeEdit,550,1);
				//if (drw->IsNodeEdit)
				if ((ToolbarEditNodes) || (GetKeyState(VK_CONTROL)&0xfffe))
					AddMenuOption(TSize/3,2*TSize+TSize_1p2,"Add node (Spacebar)  ",551,1);
			}
		}

		goto popupmenu_end_showpopup;
	}

	//find the parent expression to all selections
	if ((m_OwnerType==3) && (UserParam==0))
	{
		//in keyboard entry mode - while pressing two '?'
		m_Expression->SelectExpression(1);
		m_MenuType=0;
	}
	else
	{
		m_Expression=m_Expression->AdjustSelection();
	}

	int single_simple_object_selected=0;
	int common_color=-100;
	int at_least_one_variable=0;
	int add_even_chars=0;

	if (m_Expression==NULL)
	{
		//no selections - only EXIT will be available
		m_HaveCutDel=0;
		m_MenuType=0;
	}
	else 
	{
		int No=0;

		//check how many elements is selected int the expression
		int i;
		StartExtendedSelection=0x7FFF;
		EndExtendedSelection=0;
		for (i=0;i<m_Expression->m_NumElements;i++)
		{
			tElementStruct *theElement=m_Expression->m_pElementList+i;
			if (theElement->IsSelected) 
			{
				if (theElement->Type==1) at_least_one_variable=1;
				if (i<StartExtendedSelection) StartExtendedSelection=i;
				if (i>EndExtendedSelection) EndExtendedSelection=i;
				if (m_theSelectedElement==NULL) m_theSelectedElement=theElement;
				if ((theElement->Type>0) && (theElement->Type!=11) && (theElement->Type!=12) && (theElement->pElementObject))
				{
					if (common_color==-100) common_color=theElement->pElementObject->m_Color;
					if (theElement->pElementObject->m_Color!=common_color) common_color=100;
				}
				No++;
			}
		}

		LevelExtendedSelection=ExtractSelection(0,m_Expression->m_NumElements-1,&StartExtendedSelection,&EndExtendedSelection);	
		if (LevelExtendedSelection==-1) ExtractedSelection->Delete();
		/*if (m_Expression->m_IsText==1) 
			ExtractedSelection->Delete();*/
		else if ((StartExtendedSelection==0) && (EndExtendedSelection==m_Expression->m_NumElements-1))
		{
			//ExtractedSelection->m_ParentheseData=m_Expression->m_ParentheseData;
			//ExtractedSelection->m_ParentheseHeightFactor=m_Expression->m_ParentheseHeightFactor;
			ExtractedSelection->m_ParentheseShape=m_Expression->m_ParentheseShape;
			ExtractedSelection->m_ParenthesesFlags=m_Expression->m_ParenthesesFlags;
			if (m_Expression->m_DrawParentheses) ExtractedSelection->m_ParenthesesFlags|=0x01;
		}

		
		if ((m_OwnerType!=3) || (UserParam!=0))  //not for pop-up menues invoked by double '='
		{
			m_MenuType=0;
			if (at_least_one_variable) m_MenuType=6;
			if (m_Expression->m_Selection==0x7FFF) 
			{
				m_MenuType=1; //the parentheses menu
				if (m_Expression->IsTextContained(-1)) add_even_chars=1;
			}
			else if (No==1)
			{
				single_simple_object_selected=1;
				if (m_theSelectedElement->Type==5) // parentheses object (does this ever happen?)
					m_MenuType=4;
				if ((m_theSelectedElement->Type==1) ||  //variable
					(m_theSelectedElement->Type==6)) //function
					m_MenuType=2; //the font
				if (m_theSelectedElement->Type==7)
					m_MenuType=3;  //symbol size/height (sigma, pi, integral)
				if (m_theSelectedElement->Type==10) //condition list as an element
					m_MenuType=5;
				if ((m_theSelectedElement->Type==9) && (m_theSelectedElement->pElementObject->Data1[0]=='H'))
					m_MenuType=7; //HTML link menu
			}

			if ((ClipboardExpression) && (ClipboardExpression->m_NumElements>0))
			{
				//if there is something in the clipboard, we may wish to open "paste special - implanting paste" menu
				//that is, posibility to insert selected element into clipboard expression

				if (ClipboardExpression->m_NumElements>1) 
					AddPasteSpecial=1;
				else if (ClipboardExpression->m_pElementList) 
				{
					//if only one object in clibpobard then it must not be variable, operator or dummy
					if (ClipboardExpression->m_pElementList->Type>2)
						AddPasteSpecial=1;
				}

			}
		}
	}


	//Let's create menu options

	m_NumOptions=0;
	memset((void*)&Options,0,sizeof(Options));
	if ((m_HaveCutDel) || ((m_OwnerType==3) && (UserParam)))
	{

		if (m_OwnerType!=3)
		{
			if (m_MenuType==1) common_color=m_Expression->m_Color;
			AddMenuOption(TSize/3,TSize+TSize/8,"Pick up ",1,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3-TSize/10,TSize/3+2,"A ",(common_color==-1)?1:0,80,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3+TSize/3,TSize/3,"Red ",(common_color==1)?1:0,82,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3+TSize_2p3,TSize/2,"Blu ",(common_color==3)?1:0,84,1);

			AddMenuOption(TSize/3,TSize+TSize/8,"Delete ",2,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3,TSize/3,"Blk ",(common_color==0)?1:0,81,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3+TSize/3,TSize/3,"Grn ",(common_color==2)?1:0,83,0);
			AddCheckedMenuOption(TSize/8+4*TSize/3+TSize_2p3,TSize/3,"Gry ",(common_color==4)?1:0,85,1);

			if (m_MenuType==1)
			{
				int is_sel=0;
				for (int ii=0;ii<NumDocumentElements;ii++)
					if ((TheDocument[ii].Type==1) && 
						(TheDocument[ii].Object==(CObject*)m_Expression) && 
						(TheDocument[ii].MovingDotState==5)) 
						is_sel=1;
				AddCheckedMenuOption(TSize/3-TSize/5+1,3*TSize_1p2,"Lock ",is_sel,8,1);
			}

			if (QuickSelectActive)
			{
				PopupOption_Y+=TSize/10; //separator
				AddMenuOption(0,3*TSize_1p2,"Qick multitouch:",-411,1);
				AddMenuOption(TSize/3,TSize,"Copy ",9,1);
				if (ClipboardExpression) AddMenuOption(TSize/3,TSize,"Paste ",29,1);
				
			}
		
		}


		if ((m_MenuType!=1) || (add_even_chars))
		{
			PopupOption_Y+=TSize/10;

			AddMenuOption(0,2*TSize,"Decoration:",-44,1);

			AddMenuOptionButton(TSize/3,"None ",3,1,0);
			AddMenuOptionButton(TSize_2p3,"Strikeout ",4,2,0);
			AddMenuOptionButton(TSize,"Encircle ",5,3,0);
			AddMenuOptionButton(4*TSize/3,"Underline ",6,4,0);
			AddMenuOptionButton(5*TSize/3,"Overline ",7,5,0);
			AddMenuOptionButton(2*TSize,"Underbrace",130,79,1);
			if ((m_theSelectedElement) && ((m_theSelectedElement->Type!=9) || (m_theSelectedElement->pElementObject->Data1[0]!='H')))
				AddMenuOption(TSize/3,2*TSize+TSize/4,"Convert to hyperlink",78,1);

		}
	}

	if ((m_MenuType==2) && ((owner==Toolbox) || (owner==Toolbox->Subtoolbox)) && (UserParam<16) && (m_Expression->m_pElementList->pElementObject))
	{
		//adding color options for font formatting menu (right clicked at the toolbox header 'U' option)
		int common_color=m_Expression->m_pElementList->pElementObject->m_Color;
		AddCheckedMenuOption(TSize/8+4*TSize/3-TSize/10,TSize/3+2,"A ",(common_color==-1)?1:0,80,0);
		AddCheckedMenuOption(TSize/8+4*TSize/3+TSize/3,TSize/3,"Red ",(common_color==1)?1:0,82,0);
		AddCheckedMenuOption(TSize/8+4*TSize/3+TSize_2p3,TSize/2,"Blu ",(common_color==3)?1:0,84,1);
		AddCheckedMenuOption(TSize/8+4*TSize/3,TSize/3,"Blk ",(common_color==0)?1:0,81,0);
		AddCheckedMenuOption(TSize/8+4*TSize/3+TSize/3,TSize/3,"Grn ",(common_color==2)?1:0,83,0);
		AddCheckedMenuOption(TSize/8+4*TSize/3+TSize_2p3,TSize/3,"Gry ",(common_color==4)?1:0,85,1);
	}

	if (((m_MenuType==2) || (m_MenuType==6) || (add_even_chars)) && (m_theSelectedElement!=NULL)) //the font menu
	{
		PrepareFontMenu(PopupOption_Y);
	}
	if ((m_MenuType==1) || (m_MenuType==4))  //the parentheses menu
	{
		PrepareParenthesesMenu(PopupOption_Y);
	}
	if ((m_MenuType==3) && (m_theSelectedElement!=NULL))  //the symbol height (sigma, pi, integral) menu
	{
		PrepareSymbolMenu(PopupOption_Y);
	}
	if ((m_MenuType==5) && (m_theSelectedElement!=NULL))
	{
		PrepareConditionListMenu();
	}
	if ((m_MenuType==7) && (m_theSelectedElement!=NULL))
	{
		PopupOption_Y+=TSize/5; //separator
		AddMenuOption(0,4*TSize,"Hyperlink",77,1);
		if ((*(char**)m_theSelectedElement->pElementObject->Data3)!=NULL)
			AddMenuOption(0,4*TSize,*(char**)m_theSelectedElement->pElementObject->Data3,-77,1);

		//add list of all availabe internal links
		
		int kkk=0; //babaluj
		tDocumentStruct **list=(tDocumentStruct**)malloc(sizeof(tDocumentStruct*)*256);
		for (int i=0;i<NumDocumentElements;i++)
		{
			tDocumentStruct *ds=TheDocument+i;
			if (ds->Type==1)
			{
				if (((CExpression*)ds->Object)->m_IsHeadline)
				{
					*(list+kkk)=ds;
					kkk++;
				}
				CExpression *e=((CExpression*)ds->Object)->GetLabel();
				if (e)
				{
					*(list+kkk)=ds;
					kkk++;
				}
			}
			if (kkk>250) break;
		}
		if (kkk)
		{

			PopupOption_Y+=TSize/4;
			AddMenuOption(0,2*TSize,"Internal links:",-78,1);
			qsort(list,kkk,sizeof(tDocumentStruct*),YorderQSort);
			
			//list now contains pointers to document objects sorted by Y coordinate
			int i;
			CDC *DC=GetDC();
			DC->SelectObject(GetFontFromPool(4,0,0,TSize/4));
			for (i=0;i<25;i++,EasycastListStart++)
			{
				if (EasycastListStart>=kkk) break;
				tDocumentStruct *ds=*(list+EasycastListStart);

				char buff[128];
				buff[0]=0;
				CExpression *e=(CExpression*)ds->Object;
				int is_label=0; 
				if (e->m_IsHeadline==0) {e=e->GetLabel();is_label=1;}
				e->ConvertToPlainText(100,buff,is_label);
				if ((e->m_NumElements==1) && (e->m_pElementList->Type==0)) 
					if (is_label) sprintf(buff,"(#%d)",EasycastListStart+1); else sprintf(buff,"#%d",EasycastListStart+1);
				buff[39]=0;
				CSize sz=DC->GetTextExtent(buff);
				int tmp=0;
				if ((e->m_IsHeadline>=3) || (e->m_IsHeadline==0)) tmp=10;
				AddMenuOption(TSize/2+((is_label)?5:0),max(sz.cx-tmp,1),buff,850+i,1);
				LocalLinks[i]=((unsigned int)(ds-TheDocument))+((is_label)?0x80000000:0);
				if (tmp) PopupOption_Y-=TSize/22;
			}
			ReleaseDC(DC);
			if (EasycastListStart<kkk)
			{
				AddMenuOption(0,TSize,"more...",849,1);
			}
		}
		free(list);
	}

	if ((m_Expression) && (m_Expression->m_IsMatrixElementSelected))
	{
		PopupOption_Y+=TSize/5; //separator
		int found_selected=0,found_spacer=0,has_internallines=0;
		for (int i=0;i<m_Expression->m_NumElements;i++)
		{
			tElementStruct *ts=m_Expression->m_pElementList+i;
			if (ts->IsSelected) found_selected=1;
			if (((ts->Type==11) || (ts->Type==12)) && (found_selected)) {found_spacer=1;continue;}
			if ((ts->IsSelected) && (found_spacer)) {has_internallines=1;break;}
		}
		AddMenuOption(0,TSize*2,"Table lines:",-650,1);
		AddMenuOptionButton(TSize/3,"No border",650,83,0);
		AddMenuOptionButton(TSize_2p3,"Single line border",651,84,0);
		AddMenuOptionButton(TSize,"Double line border",652,85,(has_internallines)?0:1);
		if (has_internallines)
		{
			AddMenuOptionButton(TSize/3+TSize+TSize/8,"No lines",653,86,0);
			AddMenuOptionButton(TSize_2p3+TSize+TSize/8,"Single lines",654,87,0);
			AddMenuOptionButton(TSize+TSize+TSize/8,"Double lines",655,88,1);
		}

		if ((m_MenuType!=1) && (m_MenuType!=4))
		{
			/*int isMultiline=0;
			for (int ii=0;ii<m_Expression->m_NumElements;ii++)
				if (((m_Expression->m_pElementList+ii)->Type==12) || 
				(((m_Expression->m_pElementList+ii)->Type==2) && ((m_Expression->m_pElementList+ii)->pElementObject->Data1[0]==(char)0xFF)))
				{isMultiline=1;break;}
			if (isMultiline)*/
			{
				AddMenuOption(0,TSize*2,"Cell alignment:",-630,1);
				PopupOption_Y+=TSize_1p20;
				AddMenuOptionButton(TSize/3,"Align left ",630,39,0);
				AddMenuOptionButton(TSize_2p3,"Align center ",631,40,0);
				AddMenuOptionButton(TSize,"Align right ",632,41,1);

			}
		}
	}
	

	if (AddPasteSpecial) //adding a special menu that enables "implanting paste"
	{

		//ExtractedSelection->Delete(); //no math options when doing implanting

		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize,"Implanting:",-4,1);

		//option: Graphics representation of clipboard expression (sensitive)
		short l,a,b;
		int ZoomLevel=120*TSize/60;
		CDC *dcc=this->GetDC();
		ClipboardExpression->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
		if ((l>2*TSize) || (a+b>3*TSize_1p2))
		{
			ZoomLevel=100*TSize/60;
			ClipboardExpression->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
			if ((l>3*TSize) || (a+b>2*TSize))
			{
				ZoomLevel=80*TSize/60;
				ClipboardExpression->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
				if ((l>3*TSize) || (a+b>2*TSize))
				{
					ZoomLevel=70*TSize/60;
					ClipboardExpression->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
				}
			}
		}
		this->ReleaseDC(dcc);
		Options[m_NumOptions].Y=PopupOption_Y;
		Options[m_NumOptions].X=0;
		Options[m_NumOptions].Cx=l+TSize_1p2;
		Options[m_NumOptions].Cy=a+b+TSize/8;
		strcpy(Options[m_NumOptions].Text,"");
		Options[m_NumOptions].IsButton=0;
		Options[m_NumOptions].Graphics=ClipboardExpression;
		Options[m_NumOptions].IsChecked=0;
		Options[m_NumOptions].IsEnabled=1;
		Options[m_NumOptions].IsGraphicsSensitive=1;
		Options[m_NumOptions].Data=60; //
		Options[m_NumOptions].DataArray[0]=l;
		Options[m_NumOptions].DataArray[1]=a;
		Options[m_NumOptions].DataArray[2]=b;
		Options[m_NumOptions].DataArray[3]=ZoomLevel;
		Options[m_NumOptions].DataArray[4]=TSize/4;  //X coordinate of expression
		Options[m_NumOptions].DataArray[5]=PopupOption_Y+a+TSize/8;  //Y coordinae of expression
		PopupOption_Y+=Options[m_NumOptions].Cy;
		m_NumOptions++;
	}

popupmenu_end_showpopup:

	//the EXIT option
	{
		PopupOption_Y+=TSize/10;
		if (m_OwnerType==3) PopupOption_Y+=TSize/10;

		AddMenuOption(0,TSize+TSize/8,"Exit ",0,1);
	}

	entry_box_first_call=1;


	if ((m_OwnerType!=10) && (m_OwnerType!=9) && (m_OwnerType!=8) && (m_OwnerType!=7) && (m_OwnerType!=6)) // search tool or ruller menu
	if (!IsMathDisabled)
	if ((force_calculator) ||
		((ExtractedSelection) && (ExtractedSelection->m_pElementList->Type)))
	{
		//start thread that will run symbolic calculator and additionally 
		//insert options into menu 
		if ((CalcThreadID) && (CalcThreadHandle))
		{
			TerminateThread(CalcThreadHandle,0);
			Sleep(50);
		}
		SECURITY_ATTRIBUTES sa;
		sa.bInheritHandle=FALSE;
		sa.lpSecurityDescriptor=NULL;
		sa.nLength=sizeof(sa);
		CalcThreadHandle=CreateThread(&sa,0,CalcThread,(LPVOID)this,0,&(CalcThreadID));
	}

	
	m_SelectedOption=-1;
	if ((m_OwnerType==3) && (m_UserParam))
	{
		m_SelectedOption=0;
		while (Options[m_SelectedOption].IsEnabled==0) m_SelectedOption++;
	}

	//m_IsFirstPass=1;
	NoImageAutogeneration=3;
	if (OwnerType==10) ShowWindow(SW_SHOWNA); else ShowWindow(SW_SHOW);
	
	InvalidateRect(NULL,0);
	UpdateWindow();

	if (m_OwnerType!=9)
		SetCapture();

	return 1;

}

#pragma optimize("s",on)
int PopupMenu::PrepareConditionListMenu()
{
	PopupOption_Y+=TSize/10; //separator
	AddMenuOption(0,TSize_1p2,"Condition List:",-5,1);
	AddCheckedMenuOption(TSize_1p2,3*TSize_1p2,"Left bar ",(m_theSelectedElement->pElementObject->Data1[0]&0x01),70,1);
	AddCheckedMenuOption(TSize_1p2,3*TSize_1p2,"Right bar ",(m_theSelectedElement->pElementObject->Data1[0]&0x02),71,1);

	PopupOption_Y+=TSize/10;  //separator
	AddCheckedMenuOption(TSize_1p2,3*TSize_1p2,"Left align ",(m_theSelectedElement->pElementObject->Data2[0]==0),72,1);
	AddCheckedMenuOption(TSize_1p2,3*TSize_1p2,"Center align ",(m_theSelectedElement->pElementObject->Data2[0]==1),73,1);
	AddCheckedMenuOption(TSize_1p2,3*TSize_1p2,"Right align ",(m_theSelectedElement->pElementObject->Data2[0]==2),74,1);

	return 1;
}

#pragma optimize("s",on)
int PopupMenu::PrepareSymbolMenu(int y)
{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,5*TSize_1p2,"Symbol:",-3,1);
		AddCheckedMenuOptionButton(TSize/3,"Small ",(m_theSelectedElement->pElementObject->Data2[0]==2),52,15,0);
		AddCheckedMenuOptionButton(TSize_2p3,"Medium ",(m_theSelectedElement->pElementObject->Data2[0]==1),51,16,0);
		AddCheckedMenuOptionButton(TSize,"Large ",(m_theSelectedElement->pElementObject->Data2[0]==0),50,17,1);


		//PopupOption_Y+=TSize/15; //separator
		AddCheckedMenuOption(TSize/3-TSize/5,2*TSize,"Limits inline ",(m_theSelectedElement->pElementObject->Data2[1]==1),53,1);

		return y;
}

#pragma optimize("s",on)
int PopupMenu::PrepareParenthesesMenu(int y)
{

	int haveParentheses;
	char Shape;
	//int Height;
	int isMultiline=0;
	CExpression *theexp;
	if (m_MenuType==1) theexp=m_Expression; else theexp=((CExpression*)(m_theSelectedElement->pElementObject->Expression1));
	if (!theexp) return 0;
	haveParentheses=theexp->m_ParenthesesFlags&0xFD; //bit 2 represents automatic (bits 1 and 8 represent forcing)
	Shape=theexp->m_ParentheseShape;
	//Height=theexp->m_ParentheseHeightFactor;
	for (int ii=0;ii<theexp->m_NumElements;ii++)
		if (((theexp->m_pElementList+ii)->Type==12) || 
			(((theexp->m_pElementList+ii)->Type==2) && ((theexp->m_pElementList+ii)->pElementObject->Data1[0]==(char)0xFF)))
		{isMultiline=1;break;}


	
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,5*TSize_1p2,"Brackets:",-2,1);
		int tt=TSize_1p3-1-ToolboxSize/64;
		if ((haveParentheses&0x80)==0)
			AddCheckedMenuOptionButton(TSize_1p3,"None ",(haveParentheses==0),30,21,0);
		AddCheckedMenuOptionButton(TSize_1p3+tt,"Curved ",((haveParentheses) && (Shape=='(')),32,22,0);
		AddCheckedMenuOptionButton(TSize_1p3+2*tt,"Square ",((haveParentheses) && (Shape=='[')),33,23,0);
		AddCheckedMenuOptionButton(TSize_1p3+3*tt,"Curly ",((haveParentheses) && (Shape=='{')),34,24,0);
		AddCheckedMenuOptionButton(TSize_1p3+4*tt,"Bars ",((haveParentheses) && (Shape=='|')),35,25,0);
		AddCheckedMenuOptionButton(TSize_1p3+5*tt,"Open left ",((haveParentheses) && (Shape=='l')),56,32,0);
		AddCheckedMenuOptionButton(TSize_1p3+6*tt,"Open right ",((haveParentheses) && (Shape=='r')),57,31,1);

		AddCheckedMenuOptionButton(TSize_1p3,"Slash ",((haveParentheses) && (Shape=='/')),36,26,0);
		AddCheckedMenuOptionButton(TSize_1p3+tt,"Doble-bar ",((haveParentheses) && (Shape=='\\')),37,27,0);
		AddCheckedMenuOptionButton(TSize_1p3+2*tt,"Angled ",((haveParentheses) && (Shape=='<')),38,28,0);
		AddCheckedMenuOptionButton(TSize_1p3+3*tt,"Bra ",((haveParentheses) && (Shape=='a')),58,89,0);
		AddCheckedMenuOptionButton(TSize_1p3+4*tt,"Ket ",((haveParentheses) && (Shape=='k')),59,90,0);
		AddCheckedMenuOptionButton(TSize_1p3+5*tt,"Box ",((haveParentheses) && (Shape=='b')),39,29,0);
		AddCheckedMenuOptionButton(TSize_1p3+6*tt,"Cross ",((haveParentheses) && (Shape=='x')),49,30,1);

		PopupOption_Y+=TSize_1p20;  //separator
		AddCheckedMenuOptionButton(TSize_1p3,"Ceiling ",((haveParentheses) && (Shape=='c')),40,97,0);
		AddCheckedMenuOptionButton(TSize_1p3+tt,"Floor ",((haveParentheses) && (Shape=='f')),41,98,0);

		AddCheckedMenuOptionButton(TSize_1p3+4*tt,"Exclude left/top ",(m_Expression->m_ParenthesesFlags&0x08),151,20,0);
		AddCheckedMenuOptionButton(TSize_1p3+5*tt,"Exclude right/bottom ",(m_Expression->m_ParenthesesFlags&0x10),152,19,0);
		AddCheckedMenuOptionButton(TSize_1p3+6*tt,"Horizontal layout ",(m_Expression->m_ParenthesesFlags&0x04),150,18,1);


		/*AddCheckedMenuOptionButton(4*TSize/3+TSize/6,"Small ",(Height==2),42,15,0);
		AddCheckedMenuOptionButton(5*TSize/3+TSize/6,"Medium ",(Height==1),41,16,0);
		AddCheckedMenuOptionButton(6*TSize/3+TSize/6,"Large ",(Height==0),40,17,1);*/

		if ((theexp->m_pPaternalElement) && (haveParentheses) && (theexp->m_Selection==0x7FFF) && (theexp->m_ParenthesesSelected) && (theexp->m_pPaternalElement->m_Type==5))
		{
 			AddCheckedMenuOption(TSize/3-TSize/6,TSize*2,"Have index ",(theexp->m_pPaternalElement->Expression2!=NULL),21,1);
		}

		if (m_MenuType==4) return y;

		PopupOption_Y+=TSize/10;
		AddMenuOption(0,5*TSize_1p2,"B. contents:",-42,1);
		AddMenuOptionButton(TSize/3,"Size -50% ",54,33,0);
		AddMenuOptionButton(TSize_2p3,"Size -15% ",47,34,0);
		AddMenuOptionButton(TSize,"Size -5% ",45,35,0);
		AddMenuOptionButton(4*TSize/3+TSize/8,"Size +5% ",46,36,0);
		AddMenuOptionButton(5*TSize/3+TSize/8,"Size +15% ",48,37,0);
		AddMenuOptionButton(6*TSize/3+TSize/8,"Size +50% ",55,38,1);

		if (isMultiline)
		{
			PopupOption_Y+=TSize_1p20;
			AddCheckedMenuOptionButton(TSize/3,"Align left ",(m_Expression->m_Alignment==1),90,39,0);
			AddCheckedMenuOptionButton(TSize_2p3,"Align center ",((m_Expression->m_Alignment!=1) && (m_Expression->m_Alignment!=2)),91,40,0);
			AddCheckedMenuOptionButton(TSize,"Align right ",(m_Expression->m_Alignment==2),92,41,1);
			//if (m_Expression->m_MaxNumColumns==1) AddMenuOptionButton(6*TSize/3+TSize/8,"Inline spacing ",96,83,1);
		}

		if (m_Expression->m_pPaternalExpression==NULL)
		{
			AddCheckedMenuOption(TSize/3-TSize/5,2*TSize,"Vertical orientation",(m_Expression->m_IsVertical==1),93,1);
		}
		
		if (haveParentheses)
		if (((m_Expression->m_pPaternalElement) && (m_Expression->m_pPaternalElement->m_Type!=5)) ||
			(m_Expression->m_pPaternalExpression==NULL))
		{
			AddMenuOption(TSize/3,2*TSize,"Expand outside",94,1);
		}
		if (haveParentheses)
		if ((m_Expression->m_pPaternalElement) && (m_Expression->m_pPaternalElement->m_Type==5) && (m_Expression->m_pPaternalExpression->m_NumElements==1))
		{
			AddMenuOption(TSize/3,2*TSize,"Condense outside",95,1);
		}

	return y;
}

#pragma optimize("s",on)
int PopupMenu::PrepareFontMenu(int y)
{
	if (m_theSelectedElement==NULL) return y;
		int font=0;
		int vmods=0;
		if ((m_theSelectedElement->Type==1) || (m_theSelectedElement->Type==6))
			font=m_theSelectedElement->pElementObject->Data2[0];
		if (m_theSelectedElement->Type==1) 
			vmods=m_theSelectedElement->pElementObject->m_VMods;
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,2*TSize,"Font:",-1,1);
		AddCheckedMenuOption(TSize/3-TSize/5,5*TSize/2,"ABCDEFG abcdefgh ",((font>>5)==0),10,1);
		AddCheckedMenuOption(TSize/3-TSize/5,5*TSize/2,"ABCDEFG abcdefgh ",((font>>5)==1),11,1);
		AddCheckedMenuOption(TSize/3-TSize/5,5*TSize/2,"ABCDEFG abcdefgh ",((font>>5)==2),12,1);
		AddCheckedMenuOption(TSize/3-TSize/5,5*TSize/2,"ABCDEFG abcdefgh ",((font>>5)==3),13,1);

		PopupOption_Y+=TSize/10; //separator
		AddCheckedMenuOptionButton(TSize/3,"Italic ",(font&0x02),14,6,0);
		AddCheckedMenuOptionButton(TSize_2p3,"Bold ",(font&0x01),15,7,0);
		AddCheckedMenuOptionButton(TSize+TSize/6,"Dash ",(vmods==0x04),16,8,0);
		AddCheckedMenuOptionButton(4*TSize/3+TSize/6,"Arrow ",(vmods==0x08),17,9,0);
		AddCheckedMenuOptionButton(5*TSize/3+TSize/6,"Hat ",(vmods==0x0C),18,10,0);
		AddCheckedMenuOptionButton(6*TSize/3+TSize/6,"Hacek ",(vmods==0x1C),67,94,1);
		AddCheckedMenuOptionButton(TSize+TSize/6,"Dot ",(vmods==0x14),24,78,0);
		AddCheckedMenuOptionButton(4*TSize/3+TSize/6,"Double dot ",(vmods==0x18),25,81,0);
		AddCheckedMenuOptionButton(5*TSize/3+TSize/6,"Triple dot ",(vmods==0x20),68,96,0);
		AddCheckedMenuOptionButton(6*TSize/3+TSize/6,"Tilde ",(vmods==0x24),69,95,1);

		if ((FontAdditionalData&0x80) && ((m_Owner==Toolbox) || (m_Owner==Toolbox->Subtoolbox)))
		{
			PopupOption_Y+=TSize/10; //separator
			AddCheckedMenuOption(TSize/3-TSize/5,5*TSize/2,"Use as singleshot ",FontAdditionalData&0x01,26,1);
		}


		if ((m_OwnerType!=0) && (m_MenuType!=6))
		{
			if ((m_theSelectedElement->Type==1) && ((m_theSelectedElement->pElementObject==NULL) || (m_theSelectedElement->pElementObject->m_Text==0))) //variable
			{
				PopupOption_Y+=TSize_1p20; //separator
				AddMenuOption(0,5*TSize_1p2,"Variable:",-17,1);
				char ch=m_theSelectedElement->pElementObject->Data1[0];

				AddCheckedMenuOptionButton(TSize/3,"Has index ",(m_theSelectedElement->pElementObject->Expression1!=NULL),19,11,(ch<'A')?1:0);
				if (ch>='A')
				if ((m_theSelectedElement->pElementObject->m_VMods)!=0x10) //test if this is a measurement unit
				{
					AddMenuOptionButton(TSize_2p3+TSize/6,"Convert to unit",23,14,0);
					AddMenuOptionButton(TSize+TSize/6,"Convert to function",20,12,1);

				}
				else
				{
					AddMenuOptionButton(TSize_2p3+TSize/6,"Convert to variable",27,13,1); 
				}				
			}

			if (m_theSelectedElement->Type==6) //function
			{
				PopupOption_Y+=TSize_1p20; //separator
				AddMenuOption(0,5*TSize_1p2,"Function:",-18,1);
				AddCheckedMenuOptionButton(TSize/3,"Has index ",(m_theSelectedElement->pElementObject->Expression2!=NULL),21,11,0);
				AddMenuOptionButton(TSize_2p3+TSize/6,"Convert to variable ",22,13,1);
			}
		}
	return y;
}

#pragma optimize("",on)

PopupMenu::~PopupMenu()
{
}


BEGIN_MESSAGE_MAP(PopupMenu, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// PopupMenu message handlers

void PopupMenu::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	if (m_IsFirstPass)
	{
		//calculating x and y sizes of the popup window
		{
			m_SizeY=PopupOption_Y+TSize/10;
			m_SizeX=0;
			int i;
			for (i=0;i<m_NumOptions;i++)
				if (Options[i].X+Options[i].Cx>m_SizeX) m_SizeX=Options[i].X+Options[i].Cx;
		}


		POINT cursor;
		RECT desktop;
		GetCursorPos(&cursor);
		::GetWindowRect(GetDesktopWindow()->m_hWnd,&desktop);
		if (cursor.x>desktop.right) desktop.right=GetSystemMetrics(SM_CXVIRTUALSCREEN);
		if (cursor.y>desktop.bottom)	desktop.bottom=GetSystemMetrics(SM_CYVIRTUALSCREEN);

		if (((m_OwnerType==3) || (m_OwnerType==10)) && (KeyboardEntryBaseObject) && (m_IsFirstPass==1))
		{
			//for keyboard-called popup window (double '?' was pressed), or the stack-clipboard popup ('Ins' key was pressed)
			//the pop-up menu will be positioned near the keyboard cursor rather than mouse cursor
			RECT mainwnd;
			pMainView->GetWindowRect(&mainwnd);
			int X,Y;
			short l,a,b;
			CDC *DC=pMainView->GetDC();
			((CExpression *)(KeyboardEntryBaseObject->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
			pMainView->ReleaseDC(DC);
			((CExpression *)(KeyboardEntryBaseObject->Object))->GetKeyboardCursorPos(&X,&Y);
			cursor.x=mainwnd.left+(KeyboardEntryBaseObject->absolute_X-ViewX)*ViewZoom/100-30+X;
			cursor.y=mainwnd.top+(KeyboardEntryBaseObject->absolute_Y-ViewY)*ViewZoom/100+Y+5;
			if (cursor.x+m_SizeX>desktop.right) cursor.x=desktop.right-m_SizeX;
			if (m_OwnerType==10) cursor.y+=15;;
			if (cursor.y+Options[m_NumOptions-1].Y+ToolboxSize/4>desktop.bottom)
			{
				cursor.y-=a+b+Options[m_NumOptions-1].Y+ToolboxSize/3;
			}
		}
		

		//add some additional space (from bottom of the desktop) if we expect some math options to be added later
		int AdditionalSpace=0;
		if ((ExtractedSelection) && (ExtractedSelection->m_pElementList->Type))
		{
			AdditionalSpace=desktop.bottom/8;
			if (m_IsFirstPass>3) AdditionalSpace=desktop.bottom/24;
			if (AdditionalSpace>128) AdditionalSpace=128;
		}

		int x=cursor.x+10;
		int y=cursor.y+10;
		if (x+m_SizeX>desktop.right) x=desktop.right-m_SizeX-2;
		if (m_IsFirstPass==3) 
		{
			RECT rct;
			GetWindowRect(&rct);
			x=rct.left;
			AdditionalSpace=0;
		}
		if (y+m_SizeY>desktop.bottom-AdditionalSpace) y=desktop.bottom-m_SizeY-2-AdditionalSpace;
		if (m_IsFirstPass==4)
		{
			RECT rct;
			GetWindowRect(&rct);
			y=rct.top;
		}
		if (m_IsFirstPass==2)
			SetWindowPos(0,x,y,m_SizeX,m_SizeY,SWP_NOZORDER|SWP_NOMOVE);
		else
		{
			if (m_OwnerType==9) {RECT rr;theApp.m_pMainWnd->GetWindowRect(&rr);x=min(rr.left+150,desktop.right-100);y=min(rr.top+250,desktop.bottom-50);}
			SetWindowPos(0,x,y,m_SizeX,m_SizeY,SWP_NOZORDER);
		}

		if (((m_OwnerType==3) || (m_OwnerType==10)) && (KeyboardEntryBaseObject))
		{
			//adjusting mouse pointer position out of the popup window (if the popup window was activated by keyboard)
			POINT cr;
			RECT pr;
			GetWindowRect(&pr);
			pr.bottom=pr.top+m_SizeY;
			pr.right=pr.left+m_SizeX;
			GetCursorPos(&cr);
			if ((cr.x>pr.left-10) && (cr.x<pr.right+2) && (cr.y>pr.top-10) && (cr.y<pr.bottom+2))
			{
				int left=cr.x-(pr.left-10);
				int right=(pr.right+2)-cr.x;
				if ((left<=right) && (pr.left-10>1))
				{
					for (int i=cr.x;i>pr.left-10;i-=10)
						{SetCursorPos(i,cr.y);Sleep(10);}
					SetCursorPos(pr.left-10,cr.y);
				}
				else if (pr.right+2<desktop.right)
				{
					for (int i=cr.x;i<pr.right+2;i+=10)
						{SetCursorPos(i,cr.y);Sleep(10);}
					SetCursorPos(pr.right+2,cr.y);
				}
			}	
		}

		m_IsFirstPass=0;
	}

	PaintThePopupMenu();
}

CObject *prevSelectedGraphicsObject=NULL;
CPoint prevCurPos;
void PopupMenu::OnMouseMove(UINT nFlags, CPoint point)
{
	
	if (MovingMode)
	{
		// the whole context menu window is getting moved by the cursor (held by the upper left corner)
		if ((nFlags&MK_LBUTTON)==0) 
			MovingMode=0;
		else
		{
			ClientToScreen(&point);
			point.x-=(MovingMode-1)%20;
			point.y-=(MovingMode-1)/20;
			SetWindowPos(NULL,point.x,point.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
			return CWnd::OnMouseMove(nFlags, point);
		}
	}

	RECT wr;
	GetWindowRect(&wr);

	if (prevCurPos!=point)
	{
		if ((point.x>0) && (point.x<wr.right-wr.left) && (point.y>0) && (point.y<wr.bottom-wr.top))
			m_SelectedOption=-1;
		if (m_OwnerType!=3) 
			m_SelectedOption=-1;
	}
	else
		return;
	prevCurPos=point;

	CObject *SelectedGraphics=NULL;
	POINT cursor;
	GetCursorPos(&cursor);
	POINT cursor2;
	cursor2=cursor;
	ScreenToClient(&cursor2);
	RECT desktop;
	::GetWindowRect(GetDesktopWindow()->m_hWnd,&desktop);
	if (cursor.x>desktop.right) desktop.right=GetSystemMetrics(SM_CXVIRTUALSCREEN);
	if (cursor.y>desktop.bottom)	desktop.bottom=GetSystemMetrics(SM_CYVIRTUALSCREEN);
	if (((cursor.y+2>desktop.bottom) || (cursor.x+2>desktop.right))
		&& (cursor2.x>=0) && (cursor2.x<=m_SizeX) && (cursor2.y>=0) && (cursor2.y<=m_SizeY))
	{
		if (cursor.y+2>desktop.bottom) m_IsFirstPass=3; else m_IsFirstPass=4;
		InvalidateRect(NULL);
		UpdateWindow();
	}
	SetCursor(::LoadCursor(NULL,IDC_ARROW));

	if ((m_OwnerType==9) || (m_OwnerType==10))
		ReleaseCapture();
	else
		if (!ValueEntryBox) SetCapture();

	m_SelectedSuboption=0;

	int i;
	for (i=0;i<m_NumOptions;i++)
	{
		if ((point.x>Options[i].X) && (point.x<Options[i].X+Options[i].Cx) &&
			(point.y>Options[i].Y) && (point.y<Options[i].Y+Options[i].Cy))
		{
			//found an option that the mouse is pointing at

			if (Options[i].IsEnabled)
			{
				m_SelectedOption=i;
::SetWindowLong(this->m_hWnd,GWL_EXSTYLE,GetWindowLong(this->m_hWnd,GWL_EXSTYLE)&(~WS_EX_LAYERED));
	//::SetLayeredWindowAttributes(ListPopup->m_hWnd,0,140,LWA_ALPHA);

				//if this is a graphics object, then the expression must be selected
				if ((Options[i].Graphics) && (Options[i].IsGraphicsSensitive))
				{
					short IsExpression;
					char IsParenthese;
					Options[i].Graphics->DeselectExpression();
					CDC *dcc=this->GetDC();
					SelectedGraphics=Options[i].Graphics->SelectObjectAtPoint(dcc,
						(short)(Options[i].DataArray[3]),
						(short)(point.x-Options[i].DataArray[4]),
						(short)(point.y-Options[i].DataArray[5]),
						&IsExpression,&IsParenthese);
					this->ReleaseDC(dcc);
				}
				else if (Options[i].Graphics)
				{
					if ((m_OwnerType==10) && (point.x>Options[i].DataArray[4])) //stack-clipboard
					{
						Options[i].Graphics->SelectExpression(1);
					}
					else if (point.x>TSize_1p2+Options[i].DataArray[4])
					{
						Options[i].Graphics->SelectExpression(1);
					}
					else
					{
						Options[i].Graphics->DeselectExpression();
						m_SelectedSuboption=2;
						if (point.x<TSize/4+Options[i].DataArray[4])
							m_SelectedSuboption=1;
					}
				}
			}

			break;
		}
	}

	//deselect any other graphics object (ones that are not selected)
	int j;
	for (j=0;j<m_NumOptions;j++)
		if (j!=m_SelectedOption)
			if (Options[j].Graphics)
				Options[j].Graphics->DeselectExpression();
				
	if ((m_prevSelectedOption!=m_SelectedOption) || 
		(SelectedGraphics!=prevSelectedGraphicsObject) ||
		(m_prevSelectedSuboption!=m_SelectedSuboption))
	{
		
		PaintThePopupMenu();
		m_prevSelectedSuboption=m_SelectedSuboption;
		m_prevSelectedOption=m_SelectedOption;
		prevSelectedGraphicsObject=SelectedGraphics;
	}
	CWnd::OnMouseMove(nFlags, point);

}

int VerticalDistributionQSort(const void *first, const void *second)
{
	if (*((int*)first+1)>*((int*)second+1)) return 1;
	if (*((int*)first+1)>*((int*)second+1)) return -1;
	return 0;
}


//int MenuButtonsLoaded=-1;

int PopupMenu::PaintThePopupMenu(void)
{
	CBitmap MenuButtons;
	CDC MenuButtonsDC;

	CDC* pdc=this->GetDC();
	CDC* dc=new CDC();
	dc->CreateCompatibleDC(pdc);
	RECT rct;
	this->GetClientRect(&rct);
	CBitmap tmpbitmap;
	tmpbitmap.CreateCompatibleBitmap(pdc,rct.right,rct.bottom);
	dc->SelectObject(tmpbitmap);
	dc->FillSolidRect(0,0,rct.right,rct.bottom,SHADOW_BLUE_COLOR);

	if (ToolboxSize>100) 
		MenuButtons.LoadBitmap(IDB_MENUITEMS2); 
	else 
		MenuButtons.LoadBitmap(IDB_MENUITEMS);
	MenuButtonsDC.CreateCompatibleDC(dc);
	MenuButtonsDC.SelectObject(MenuButtons);

	//drawing menu
	int i;
	for (i=0;i<m_NumOptions;i++)
	{
		char is_bold=(m_SelectedOption==i)?1:0;
		if (Options[i].IsEnabled)
		{
			char bold=0;
			char face=4;
			if ((Options[i].Data>=10) && (Options[i].Data<=13)) face=Options[i].Data-10; //font options
			
			dc->SelectObject(GetFontFromPool(face,0,bold,TSize/4));
			if (m_SelectedOption==i)
				dc->SetTextColor(BLUE_COLOR);
			else
				dc->SetTextColor(0);
		}
		else
		{
			dc->SelectObject(GetFontFromPool(4,0,0,TSize/5+2));
			dc->SetTextColor(SHADOW_BLUE_COLOR3);
		}

		dc->SetTextAlign(TA_TOP);
	
		if (Options[i].Text[0])
		{
			dc->SetBkColor(SHADOW_BLUE_COLOR);

			if (Options[i].IsButton)
			{
				//menu item buttons
				
				int buttonx=Options[i].IsButton*19-19;
				int buttony=0;if (TSize>70) buttony=18;
				if (TSize>75)
				{
					buttonx=Options[i].IsButton*25-25;
					buttony=0;
					if (buttonx>=25*50) {buttonx-=25*50;buttony=25;}
				}

				if (m_SelectedOption==i)
				{
					dc->FillSolidRect(Options[i].X+2,Options[i].Y+1,Options[i].Cx,Options[i].Cy,PALE_RGB(BLUE_COLOR));
					dc->TransparentBlt(Options[i].X+2,
										Options[i].Y+1,
										Options[i].Cx,
										Options[i].Cy,
										&MenuButtonsDC,
										buttonx,
										buttony,
										Options[i].Cx,
										Options[i].Cy,
										MenuButtonsDC.GetPixel(5,5)); //transparent color can be found at this pixel
					//display the helper text
					for (int jj=i;jj>=0;jj--)
						if (Options[jj].IsEnabled==0)
						{
							dc->SelectObject(GetFontFromPool(4,0,0,TSize/5+2));
							dc->SetTextColor(SHADOW_BLUE_COLOR3);
							char tmp[128];
							strcpy(tmp,Options[jj].Text);
							strcat(tmp," ");
							strcat(tmp,Options[i].Text);
							dc->SetBkColor(SHADOW_BLUE_COLOR);
							dc->TextOut(Options[jj].X+2,Options[jj].Y+1,tmp);
							break;
						}
				}
				else
				{
					dc->BitBlt(Options[i].X+2,
								Options[i].Y+1,
								Options[i].Cx,
								Options[i].Cy,
								&MenuButtonsDC,
								buttonx,
								buttony,
								SRCCOPY);
				}
				if (Options[i].IsChecked&0x01)
				{
					PaintCheckedSign(dc,Options[i].X+2*Options[i].Cx/3,Options[i].Y+2*Options[i].Cy/3,Options[i].Cx/2,Options[i].IsChecked&0x01);
				}
			}
			else if (Options[i].IsChecked&0x02)
			{

				if (((Options[i].Data>=81) && (Options[i].Data<90)) || 
					((Options[i].Data>=581) && (Options[i].Data<589)))
				{
					//special handling for color options
					int clr=0;
					int lrg=0;
					clr=Options[i].Data-81;
					if (clr>100) clr-=500;
					clr=ColorTable[clr];
					if (m_SelectedOption==i) lrg=TSize/24; else lrg=1;
					dc->FillSolidRect(Options[i].X+TSize/8+1-lrg,Options[i].Y+1+TSize_1p20-lrg,TSize/8+2*lrg,TSize/8+2*lrg,clr);
					if (Options[i].IsChecked&0x01) 
						PaintCheckedSign(dc,Options[i].X+TSize/8+2,Options[i].Y+TSize/12+1,TSize/6,Options[i].IsChecked&0x01);
				}
				else
				{
					CSize len=dc->GetTextExtent(Options[i].Text,(int)strlen(Options[i].Text));
					if (len.cx>Options[i].Cx+TSize/12-TSize/5)	dc->SelectObject(GetFontFromPool(4,0,0,(TSize)/5+2));

					dc->TextOut(Options[i].X+TSize/5+1,Options[i].Y+1,Options[i].Text); //place for Check
					if (is_bold)
					{
						dc->SetBkMode(TRANSPARENT);
						dc->TextOut(Options[i].X+TSize/5+2,Options[i].Y+1,Options[i].Text); //place for Check
						dc->SetBkMode(OPAQUE);
					}

					if (Options[i].IsChecked&0x01)
					if ((Options[i].Data==80) || (Options[i].Data==580)) //special handling for color "auto"
						PaintCheckedSign(dc,Options[i].X+TSize/5+2,Options[i].Y+TSize/12+1,TSize/6,Options[i].IsChecked&0x01);
					else
						PaintCheckedSign(dc,Options[i].X+2,Options[i].Y+1,TSize/6,Options[i].IsChecked&0x01);
				}
				
			}
			else
			{
				CSize len=dc->GetTextExtent(Options[i].Text,(int)strlen(Options[i].Text));
				if (len.cx>Options[i].Cx+TSize/12)	dc->SelectObject(GetFontFromPool(4,0,0,TSize/5+1));
				dc->TextOut(Options[i].X+2,Options[i].Y+1,Options[i].Text);
				if (is_bold)
				{
					dc->SetBkMode(TRANSPARENT);
					dc->TextOut(Options[i].X+3,Options[i].Y+1,Options[i].Text);
					dc->SetBkMode(OPAQUE);

				}

			}
		}
		if (Options[i].Graphics) 
		{
			short l,a,b;
			char tmpvf=Options[i].Graphics->m_IsVertical;
			Options[i].Graphics->m_IsVertical=0;
			Options[i].Graphics->CalculateSize(dc,Options[i].DataArray[3],&l,&a,&b);
			Options[i].Graphics->PaintExpression(dc,
				Options[i].DataArray[3],  //ZOOM
				Options[i].DataArray[4]+(((Options[i].IsGraphicsSensitive) || (m_OwnerType==10))?0:TSize_1p2),  
				Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]);  

			if ((is_bold) && (m_SelectedSuboption==0) && (Options[i].IsGraphicsSensitive==0))
				Options[i].Graphics->PaintExpression(dc,
				Options[i].DataArray[3],  //ZOOM
				Options[i].DataArray[4]+1+(((Options[i].IsGraphicsSensitive) || (m_OwnerType==10))?0:TSize_1p2),  
				Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]);  

			Options[i].Graphics->m_IsVertical=tmpvf;
			if ((!Options[i].IsGraphicsSensitive) && (Options[i].Data>0) && (m_OwnerType!=10))
			{
				CBrush blue(BLUE_COLOR);
				if ((m_SelectedSuboption==1) && (m_SelectedOption==i))
				{
					dc->SelectObject(GetPenFromPool(1,1));
					dc->SelectObject(blue);
				}
				else
				{
					dc->SelectObject(GetStockObject(BLACK_PEN));
					dc->SelectObject(GetStockObject(BLACK_BRUSH));
				}
				POINT p[3];
				p[0].x=Options[i].DataArray[4];p[0].y=Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]-TSize/12;
				p[1].x=Options[i].DataArray[4]+TSize/6;p[1].y=Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]-TSize/12;
				p[2].x=Options[i].DataArray[4]+TSize/12;p[2].y=Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]+TSize/12;
				dc->Polygon(p,3);
				if ((m_SelectedSuboption==2) && (m_SelectedOption==i))
				{
					dc->SelectObject(GetPenFromPool(1,1));
					dc->SelectObject(blue);
				}
				else
				{
					dc->SelectObject(GetStockObject(BLACK_PEN));
					dc->SelectObject(GetStockObject(BLACK_BRUSH));
				}
				dc->Rectangle(Options[i].DataArray[4]+TSize/4,
					Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]-TSize/12,
					Options[i].DataArray[4]+TSize/4+TSize/6,
					Options[i].DataArray[5]-Options[i].DataArray[1]+Options[i].DataArray[1]+TSize/12);

			}
			//dc->BitBlt(Options[i].DataArray[4],Options[i].DataArray[5]-Options[i].DataArray[1],
			//	Options[i].DataArray[0],Options[i].DataArray[1]+Options[i].DataArray[2],&bmpDC,0,0,SRCCOPY);
			
		}
	}

	dc->SelectObject(GetPenFromPool(1,0,SHADOW_BLUE_COLOR2));

	dc->MoveTo(0,0);
	dc->LineTo(rct.right-1,0);
	dc->LineTo(rct.right-1,rct.bottom-1);
	dc->LineTo(0,rct.bottom-1);
	dc->LineTo(0,0);

	CPoint cursor;
	GetCursorPos(&cursor);
	ScreenToClient(&cursor);
	if ((cursor.x+cursor.y<TSize/4) && (cursor.x>0) && (cursor.y>0))
		dc->SelectObject(GetPenFromPool(1,1,0));
	else
		dc->SelectObject(GetPenFromPool(1,0,SHADOW_BLUE_COLOR2));

	dc->MoveTo(1,TSize/4-1);dc->LineTo(TSize/4,0);
	dc->MoveTo(1,TSize/4-3-1);dc->LineTo(TSize/4-3,0);
	dc->MoveTo(1,TSize/4-6-1);dc->LineTo(TSize/4-6,0);
	dc->MoveTo(1,TSize/4-9-1);dc->LineTo(TSize/4-9,0);
	dc->MoveTo(1,TSize/4-12-1);dc->LineTo(TSize/4-12,0);
	dc->MoveTo(1,TSize/4-15-1);dc->LineTo(TSize/4-15,0);
	pdc->BitBlt(0,0,rct.right,rct.bottom,dc,0,0,SRCCOPY);
	delete dc;

	ReleaseDC(pdc);
	return 0;
}


#pragma optimize("s",on)
void PopupMenu::OnLButtonDown(UINT nFlags, CPoint point)
{

	int DirectCallFlag=0;
	try{

		//very special handling - direct call from toolbar
		if ((point.x==-10000) && (point.y==-10001) && (nFlags>=1) && (nFlags<1000))
		{
			m_SelectedOption=0;
			Options[m_SelectedOption].Data=0;
			DirectCallFlag=1;
			goto DirectCall;
			
		}

	if ((point.x+point.y<TSize/4) && (point.x>0) && (point.y>0)) {MovingMode=point.x+point.y*20+1;SetCapture();}
	RECT wr;
	POINT cursor;
	GetWindowRect(&wr);
	GetCursorPos(&cursor);
	if ((point.x!=-1) || (point.y!=-1))
	if ((cursor.x<wr.left) || (cursor.x>wr.right) || (cursor.y<wr.top) || (cursor.y>wr.bottom))
	{
		//clicked outside popup meni, sound alarm
		HidePopupMenu();
		return;
		/*MessageBeep(0);
		GetClientRect(&wr);
		CDC *dc=this->GetDC();
		dc->SelectObject(GetPenFromPool(3,1));
		dc->MoveTo(2,2);dc->LineTo(wr.right-2,2);dc->LineTo(wr.right-2,wr.bottom-2);dc->LineTo(2,wr.bottom-2);dc->LineTo(2,2);
		Sleep(50);
		InvalidateRect(NULL,0);
		UpdateWindow();*/
	}

	char isShiftDown=GetKeyState(16)&0xFFFE;

	if ((ValueEntryBox) && (nFlags!=0x1234))
	{
		if (ValueEntryBox==this->WindowFromPoint(cursor))
		{
			//if clicked on the entry box
			ValueEntryBox->ScreenToClient(&cursor);
			ValueEntryBox->SendMessage(WM_LBUTTONDOWN,0,cursor.x+(cursor.y<<16));
			return;
		}
	}


	char delete_clipboard_at_exit=0;
	if ((m_SelectedOption>=0) && (m_SelectedOption<63))
	{
		int dataval=Options[m_SelectedOption].Data;

		if (0)
		{
DirectCall:
			dataval=nFlags;
		}
		
		//if (Options[m_SelectedOption].Data==0) //EXIT
		//{
		//	//do nothing, just exit
		//}

		if ((dataval>=110) && (dataval<115)) //stack-clipboard option
		{
			pMainView->OnEditUndo();
			int d=dataval-110;
			for (int i=0;i<m_NumOptions;i++)
				if (Options[i].Graphics) Options[i].Graphics->DeselectExpression();
			if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
			{
				if (ClipboardExpression) delete ClipboardExpression;
				ClipboardExpression=new CExpression(NULL,NULL,100);
				ClipboardExpression->CopyExpression(this->Options[d].Graphics,0,0,0);
				CDC *DC=pMainView->GetDC();
				((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,6,0,0,0,0);
				pMainView->ReleaseDC(DC);
			}
			
		}

		if (dataval==849) //more local links
		{
			this->ShowPopupMenu(m_Expression,m_Owner,m_OwnerType,m_MenuType,1);
			return;
		}
		if ((dataval>=850) && (dataval<=899)) //local links
		{
			int is_label=LocalLinks[dataval-850]&0x80000000;
			int orderno=LocalLinks[dataval-850]&0x7FFFFFFF;

			if (TheDocument[orderno].Type==1)
			for (int i=0;i<m_Expression->m_NumElements;i++)
			{
				tElementStruct *ts=m_Expression->m_pElementList+i;
				if ((ts->IsSelected) && (ts->Type==9) && (ts->pElementObject))
				{
					CElement *elm=ts->pElementObject;
					if ((elm->Data1[0]=='H') && (elm->Expression1) && (elm->Expression2==NULL) && (elm->Expression3==NULL)) //hyperlink element
					{
						CExpression *exp=(CExpression*)elm->Expression1;
						if (*((char**)elm->Data3)==NULL)
						{
							char *link=(char*)malloc(340);
							*((char**)elm->Data3)=link;
						}
						char buff[350];
						buff[0]=0;
						if (is_label)
						{
							CExpression *e=((CExpression*)TheDocument[orderno].Object)->GetLabel();
							if (e) e->ConvertToPlainText(330,buff);
						}
						else
						{
							((CExpression*)TheDocument[orderno].Object)->ConvertToPlainText(330,buff);
						}
						buff[339]=0;
						char buff2[350];
						buff2[0]=0;
						exp->ConvertToPlainText(330,buff2);
						buff2[339]=0;
						int swap_all=0;
						if (strcmp(buff2,*((char**)elm->Data3))==0) swap_all=1;
						strcpy(*((char**)elm->Data3),buff);

						if ((swap_all) || (((exp->m_pElementList->Type==0) || ((exp->m_pElementList->Type==1) && (exp->m_pElementList->pElementObject->Data1[0]==0))) && (exp->m_NumElements==1)))
						{
							//empty, fill it inside
							if (is_label)
							{
								CExpression *e=((CExpression*)TheDocument[orderno].Object)->GetLabel();
								exp->CopyExpression(e,0);
							}
							else
							{
								exp->CopyExpression((CExpression*)TheDocument[orderno].Object,0);
								exp->m_IsHeadline=0;
							}
						}
					}
				}
			}

		}



		if (dataval==823)
		{
			if ((DeletableGuidelineObject>=0) && (DeletableGuidelineObject<NumDocumentElements))
				pMainView->DeleteDocumentObject(&TheDocument[DeletableGuidelineObject]);
			pMainView->RepaintTheView();
		}
		if ((dataval>=820) && (dataval<=822))  //adding a guideline at this position
		{
			CExpression *e=new CExpression(NULL,NULL,100);
			if (dataval==820) AddDocumentObject(1,RullerPositionPreselected,-1101);  //nomal guideline
			else if (dataval==821) AddDocumentObject(1,RullerPositionPreselected,-1051);  //text guideline
			else AddDocumentObject(1,RullerPositionPreselected,-1001); //text autowrap guideline

			TheDocument[NumDocumentElements-1].MovingDotState=0;
			TheDocument[NumDocumentElements-1].Object=(CObject*)e;
			pMainView->RepaintTheView();
		}

		if (dataval==831) // 'more... easycasts
		{
			this->ShowPopupMenu(m_Expression,m_Owner,m_OwnerType,m_MenuType,1);
			return;
		}
		if (dataval==830) //redefining keycode strings (accelerator keycodes used when typing)
		{
			if (ValueEntryBox)
			{
				entry_box_first_call=1;

				char string[65];
				ValueEntryBox->GetWindowText(string,64);
				int beep=0;
				for (int i=0;i<(int)strlen(string);i++)
				{
					unsigned char c=(unsigned char)string[i];
					if ((c!=10) && (c!=13) && (c<=32)) beep=1;
					if ((c<=32) && (c!=0))
					{
						memmove(&string[i],&string[i+1],64-i);
						i--;
					}
				}
				string[9]=0;
				if (strlen(string)>8) beep=1;
				if (string[0]==' ') beep=1;
				ValueEntryBox->SetWindowText(string);
				
				//now check if similar easycast code is alredy defined
				int k=0;
				int len2=(int)strlen(string);
				if (len2) while (1)
				{
					char *keycode;
					CExpression *g=Toolbox->ReturnKeycode(k,&keycode);
					if (!g) break;
					int len1=(int)strlen(keycode);
					

					if (TheKeycodeString!=keycode)
					{
						for (int l=0;l<=len1-len2;l++)
							if (strncmp(&keycode[l],string,len2)==0)
								if ((l<len1-len2) || (l==0)) {beep=1;break;}
						for (int l=0;l<=len2-len1;l++)
							if (strncmp(&string[l],keycode,len1)==0)
								if ((l<len2-len1) || (l==0)) {beep=1;break;}
					}
						
					k++;
				}

				if (beep)
				{
					MessageBeep(1);
					Sleep(250);
					EasycastListStart=0;
					ShowPopupMenu(m_Expression,m_Owner,m_OwnerType,m_MenuType,1);
					return;
				}
				strcpy(TheKeycodeString,string);

			}
		}

		if ((dataval==801) || (dataval==802)) //search
		{
			if (ValueEntryBox)
			{
				entry_box_first_call=1;

				char string[65];
				ValueEntryBox->GetWindowText(string,64);
				for (int i=0;i<(int)strlen(string);i++)
					if ((((unsigned char)string[i])<32) && (string[i]!=0))
					{memmove(&string[i],&string[i+1],64-i);i--;}
				ValueEntryBox->SetWindowText(string);
				//ValueEntryBox->SetSel(0,strlen(string),0);

				if (strlen(string))
				{
					char search_first_pass=1;
search_second_pass:
					//create sorted list (on Y coordinate) of document objects
					tDocumentStruct **list=(tDocumentStruct**)malloc(sizeof(tDocumentStruct*)*NumDocumentElements);
					tDocumentStruct *dss=TheDocument;					
					for (int i=0;i<NumDocumentElements;i++,dss++)
					{
						*(list+i)=dss;
						if (dss->Type==1) ((CExpression*)dss->Object)->DeselectExpression();

					}
					qsort(list,NumDocumentElements,sizeof(tDocumentStruct*),YorderQSort);

					//list now contains pointers to document objects sorted by Y coordinate

					int tmp=1;
					if (dataval==802) tmp=-1;
					//if (dataval==801) SearchStartPosition++; //down direction
					//if (dataval==802) SearchStartPosition--; //up direction
					//if (SearchStartPosition>NumDocumentElements) SearchStartPosition=NumDocumentElements;
					//if (SearchStartPosition<0) SearchStartPosition=-1;

					int i;
					for (i=SearchStartPosition+tmp;((i<NumDocumentElements) && (i>=0));((dataval==801)?i++:i--))
					{
						tDocumentStruct *ds=*(list+i);

						if (ds->Type==1)
						{
							CExpression *e=(CExpression*)ds->Object;
							if (e->SearchForString(string))
							{
								e->SelectExpression(1);
								int xx=80*100/ViewZoom;
								ViewY=ds->absolute_Y-ds->Above-xx;if (ViewY<0) ViewY=0;
								if ((ViewX>ds->absolute_X) || ((ViewX+TheClientRect.right*100/ViewZoom)<ds->absolute_X+ds->Length))
								{ViewX=ds->absolute_X-xx;if (ViewX<0) ViewX=0;}
								int ggg=ShadowSelection;
								CDC *DC=pMainView->GetDC();
								pMainView->RepaintTheView();
								for (int j=0;j<2;j++)
								{
									ShadowSelection=1;
									pMainView->GentlyPaintObject(ds,DC);Sleep(100);
									ShadowSelection=0;
									pMainView->GentlyPaintObject(ds,DC);Sleep(100);
								}
								pMainView->ReleaseDC(DC);
								ShadowSelection=ggg;
								break;
							}					
						}
					}
					//SearchStartPosition=i;
					free(list);
					if ((i>=NumDocumentElements) || (i<0)) 
					{
						if ((nFlags==0x1234) && (search_first_pass)) {search_first_pass=0;SearchStartPosition=-1;goto search_second_pass;}
						char str[80];
						CopyTranslatedString(str,"Nothing found",5040,79);
						MessageBox(str);
					}
					else
						SearchStartPosition=i;
					
				}
				return;
			}
		}

		//column/row insertion points
		if ((dataval>=600) && (dataval<=699))
		{
			if ((dataval>=630) && (dataval<=632))
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("table formatting",20413); 
				for (int i=0;i<m_Expression->m_MaxNumRows;i++)
					for (int j=0;j<m_Expression->m_MaxNumColumns;j++)
					{
						int elm=m_Expression->FindMatrixElement(i,j,1);
						if ((m_Expression->m_pElementList+elm)->IsSelected)
						{
							tCellAttributes attrib;
							if (m_Expression->GetCellAttributes(i,j,&attrib))
							{
								if (dataval==630) *attrib.alignment='l';
								if (dataval==631) *attrib.alignment='c';
								if (dataval==632) *attrib.alignment='r';
							}
						}
					}
				m_Expression->AdjustMatrix();
			}

			if ((dataval>=650) && (dataval<=655))
			{
				int top=0,left=0;
				int bottom=0,right=0;
				for (int i=0;i<m_Expression->m_MaxNumRows;i++)
					for (int j=0;j<m_Expression->m_MaxNumColumns;j++)
					{
						int start=m_Expression->FindMatrixElement(i,j,1);
						if ((m_Expression->m_pElementList+start)->IsSelected) {top=i;left=j;i=32760;break;}
					}
				for (int i=m_Expression->m_MaxNumRows-1;i>=0;i--)
					for (int j=m_Expression->m_MaxNumColumns-1;j>=0;j--)
					{
						int start=m_Expression->FindMatrixElement(i,j,1);
						if ((m_Expression->m_pElementList+start)->IsSelected) {bottom=i;right=j;i=-1;break;}
					}
				if ((top<=bottom) && (left<=right))
				{
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("table formatting",20413); 

					for (int i=top;i<=bottom;i++)
						for (int j=left;j<=right;j++)
						{
							//doing border on cell
							tCellAttributes attrib;
							if (m_Expression->GetCellAttributes(i,j,&attrib))
							{
								if (i==top)
									*attrib.top_border=(dataval==650)?' ':((dataval==651)?'-':((dataval==652)?'=':*attrib.top_border));
								else
									*attrib.top_border=(dataval==653)?' ':((dataval==654)?'-':((dataval==655)?'=':*attrib.top_border));
								
								if (j==right)
									*attrib.right_border=(dataval==650)?' ':((dataval==651)?'-':((dataval==652)?'=':*attrib.right_border));
								else
									*attrib.right_border=(dataval==653)?' ':((dataval==654)?'-':((dataval==655)?'=':*attrib.right_border));

								if (i==bottom)
									*attrib.bottom_border=(dataval==650)?' ':((dataval==651)?'-':((dataval==652)?'=':*attrib.bottom_border));
								else
									*attrib.bottom_border=(dataval==653)?' ':((dataval==654)?'-':((dataval==655)?'=':*attrib.bottom_border));

								if (j==left)
									*attrib.left_border=(dataval==650)?' ':((dataval==651)?'-':((dataval==652)?'=':*attrib.left_border));
								else
									*attrib.left_border=(dataval==653)?' ':((dataval==654)?'-':((dataval==655)?'=':*attrib.left_border));
							}

							//doing border on left side of a nerby cell	
							if ((j<m_Expression->m_MaxNumColumns-1) && (j==right))
							{
								if (m_Expression->GetCellAttributes(i,j+1,&attrib))
								{
									*attrib.left_border=(dataval==650)?' ':((dataval==651)?'-':((dataval==652)?'=':*attrib.left_border));
								}
							}

							//doing border on left side of a nerby cell	
							if ((i<m_Expression->m_MaxNumRows-1) && (i==bottom))
							{
								if (m_Expression->GetCellAttributes(i+1,j,&attrib))
								{
									*attrib.top_border=(dataval==650)?' ':((dataval==651)?'-':((dataval==652)?'=':*attrib.top_border));
								}
							}
						}
						m_Expression->AdjustMatrix();
				}
			}
			if ((dataval>=601) && (dataval<=603) && ((m_Expression->m_IsColumnInsertion) || (m_Expression->m_IsRowInsertion))) //no line, single line, double line
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("table formatting",20413); 

				char c=' ';
				if (dataval==602) c='-';
				if (dataval==603) c='=';
				if (m_Expression->m_IsColumnInsertion)
				{
					for (int i=0;i<m_Expression->m_MaxNumRows;i++)
					{
						int t_column=m_Expression->m_ColumnSelection;
						if (t_column>m_Expression->m_MaxNumColumns-1) t_column=m_Expression->m_MaxNumColumns-1;
						tCellAttributes attrib;
						if (m_Expression->GetCellAttributes(i,t_column,&attrib))
						{
							if (m_Expression->m_ColumnSelection==m_Expression->m_MaxNumColumns)
								*attrib.right_border=c;
							else
								*attrib.left_border=c;
						}
					}
				}
				else
				{
					for (int i=0;i<m_Expression->m_MaxNumColumns;i++)
					{
						int t_row=m_Expression->m_RowSelection;
						if (t_row>m_Expression->m_MaxNumRows-1) t_row=m_Expression->m_MaxNumRows-1;
						tCellAttributes attrib;
						if (m_Expression->GetCellAttributes(t_row,i,&attrib))
						{
							if (m_Expression->m_RowSelection==m_Expression->m_MaxNumRows)
								*attrib.bottom_border=c;
							else
								*attrib.top_border=c;
						}
					}
				}
				m_Expression->AdjustMatrix();
			}
		}

		//drawing and selection (options 500...599)
		if ((dataval>=500) && (dataval<599))
		{
			CDrawing *tmpdrw=NULL;
			tDocumentStruct *prevelement=NULL;
			int StartX,StartY;
			int MaxX,MaxY;
			int found=0;

			int data=dataval;
			for (int ii=0;ii<NumDocumentElements;ii++)
			{
				tDocumentStruct *ds=TheDocument+ii;
				if ((ds->Object) && ((ds->MovingDotState==3) || 
					((ds->Type==2) && (((CDrawing*)(ds->Object))->IsSelected)) ||
					((ds->Type==1) && (((CExpression*)(ds->Object))->m_Selection==0x7FFF))))
				{
					//we found an element that is either selected or touched
					//find upper left, and lower right corrner of selection
					if (found==0)
					{
						StartX=0x7FFFFFFF;
						StartY=0x7FFFFFFF;
						MaxX=MaxY=0x80000000;
						for (int kkk=ii;kkk<NumDocumentElements;kkk++)
						{
							tDocumentStruct *ds2=TheDocument+kkk;
							if ((ds2->Object) && ((ds2->MovingDotState==3) || 
							((ds2->Type==2) && (((CDrawing*)(ds2->Object))->IsSelected)) ||
								((ds2->Type==1) && (((CExpression*)(ds2->Object))->m_Selection==0x7FFF))))
							{
								if (ds2->absolute_X<StartX) StartX=ds2->absolute_X;
								if (ds2->absolute_Y-ds2->Above<StartY) StartY=ds2->absolute_Y-ds2->Above;
								if (ds2->absolute_X+ds2->Length>MaxX) MaxX=ds2->absolute_X+ds2->Length;
								if (ds2->absolute_Y+ds2->Below>MaxY) MaxY=ds2->absolute_Y+ds2->Below;
							}
						}
					}

					if (data==590) //add center point (to cyrcle)
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("add centerpoint",20100);
						CDrawing *drw=(CDrawing*)(ds->Object);
						int minx,miny,maxx,maxy;
						drw->FindRealCorner(&minx,&miny,&maxx,&maxy);
						drw->InsertItemAt(drw->NumItems);
						tDrawingItem *di=drw->Items+drw->NumItems-1;
						di->LineWidth=DRWZOOM;
						di->Type=1;
						di->pSubdrawing=NULL;
						di->X1=(minx+maxx)/2*DRWZOOM;
						di->Y1=(miny+maxy)/2*DRWZOOM;
						di->X2=di->X1;
						di->Y2=di->Y1;



					}
					if (data==591)
					{
						((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("make dashed",20101);
						((CDrawing*)(ds->Object))->MakeDashed(0);
					}
					if (data==592)
					{
						((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("dash-dot",20102);
						((CDrawing*)(ds->Object))->MakeDashed(1);
					}
					if ((data==595) || (data==596) || (data==597)) //line endings (arrow, narrow arrow, dot)
					{
						//malo poboljšati i poveæati strelice - napraviti dvije vrste strelice - napraviti pojedinaène strelice
						CDrawing * drw=((CDrawing*)(ds->Object));

						POINT pp[64];
						char is_closed;
						char num_points;
						int is_open=drw->IsOpenPath(0,&is_closed,(LPPOINT)&pp[0],&num_points);
						if ((drw->NumItems==1) && (drw->Items->Type==1))
						{
							//simple straight line
							is_closed=0;num_points=2;
							is_open=1;
							pp[0].x=drw->Items->X1;
							pp[0].y=drw->Items->Y1;
							pp[1].x=drw->Items->X2;
							pp[1].y=drw->Items->Y2;
						}
						if ((is_open) && (num_points>=2))
						{
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("add arrows",20103);

							double ang=atan2((double)(pp[1].y-pp[0].y),(double)(pp[1].x-pp[0].x));
							double ang2=atan2((double)(pp[num_points-1].y-pp[num_points-2].y),(double)(pp[num_points-1].x-pp[num_points-2].x));

							//r=lenght of line
							double r=0;
							for (int ii=0;ii<drw->NumItems;ii++)
							{
								tDrawingItem *di=drw->Items+ii;
								if (di->Type==1)
									r+=sqrt((double)(di->X1-di->X2)*(double)(di->X1-di->X2)+(double)(di->Y1-di->Y2)*(double)(di->Y1-di->Y2));
							}

							unsigned char arrow_end=3;
							int end_distance1=abs(pp[0].x/DRWZOOM+ds->absolute_X-Popup_CursorX)+abs(pp[0].y/DRWZOOM+ds->absolute_Y-Popup_CursorY);
							int end_distance2=abs(pp[num_points-1].x/DRWZOOM+ds->absolute_X-Popup_CursorX)+abs(pp[num_points-1].y/DRWZOOM+ds->absolute_Y-Popup_CursorY);
							if ((end_distance2<end_distance1/4) && (end_distance2<15))
								arrow_end&=0xFE;
							if ((end_distance1<end_distance2/4) && (end_distance1<15))
								arrow_end&=0xFD;

							r-=DRWZOOM;
							r/=2;
							if (r<=DRWZOOM) r=DRWZOOM;
							r=min(r,145*DRWZOOM/10);
							double dang=0.39;
							if (data==597)
							{
								dang=0.785;
								r=3*DRWZOOM;
							}
							else if (data==596)
							{
								dang=0.17;
								r=min(r,13*DRWZOOM);
								if (drw->Items->LineWidth<4*DRWZOOM) r=min(r,115*DRWZOOM/10);
								if (drw->Items->LineWidth<2*DRWZOOM) r=min(r,10*DRWZOOM);
							}
							int X=(int)(cos(ang+dang)*r+0.5);
							int Y=(int)(sin(ang+dang)*r+0.5);
							int X2=(int)(cos(ang-dang)*r+0.5);
							int Y2=(int)(sin(ang-dang)*r+0.5);

							int X3=(int)(cos(ang2+dang)*r+0.5);
							int Y3=(int)(sin(ang2+dang)*r+0.5);
							int X4=(int)(cos(ang2-dang)*r+0.5);
							int Y4=(int)(sin(ang2-dang)*r+0.5);

							int num_lines=2;
							if (data==597) num_lines=4;

							if (arrow_end==3) num_lines*=2;
							for (int i=0;i<num_lines;i++)
							{
								drw->InsertItemAt(drw->NumItems);
								(drw->Items+drw->NumItems-1)->Type=1;
								(drw->Items+drw->NumItems-1)->pSubdrawing=0;
								(drw->Items+drw->NumItems-1)->LineWidth=(drw->Items->LineWidth)*3/((data==597)?2:3);
							}
							tDrawingItem *di2=drw->Items+drw->NumItems-num_lines;

							
							if (arrow_end&0x01)
							{
								POINT *p=pp;
								if (data==597)
								{
									di2->X1=(tDrwXY)(p->x+X);	di2->Y1=(tDrwXY)(p->y+Y);
									di2->X2=(tDrwXY)(p->x+X2);	di2->Y2=(tDrwXY)(p->y+Y2);
									di2++;
									di2->X1=(tDrwXY)(p->x+X2);	di2->Y1=(tDrwXY)(p->y+Y2);
									di2->X2=(tDrwXY)(p->x-X);	di2->Y2=(tDrwXY)(p->y-Y);
									di2++;
									di2->X1=(tDrwXY)(p->x-X);	di2->Y1=(tDrwXY)(p->y-Y);
									di2->X2=(tDrwXY)(p->x-X2);	di2->Y2=(tDrwXY)(p->y-Y2);
									di2++;
									di2->X1=(tDrwXY)(p->x-X2);	di2->Y1=(tDrwXY)(p->y-Y2);
									di2->X2=(tDrwXY)(p->x+X);	di2->Y2=(tDrwXY)(p->y+Y);
									di2++;
								}
								else
								{
									di2->X1=(tDrwXY)(p->x);		di2->Y1=(tDrwXY)(p->y);
									di2->X2=(tDrwXY)(p->x+X);	di2->Y2=(tDrwXY)(p->y+Y);
									di2++;
									di2->X1=(tDrwXY)(p->x);		di2->Y1=(tDrwXY)(p->y);
									di2->X2=(tDrwXY)(p->x+X2);	di2->Y2=(tDrwXY)(p->y+Y2);
									di2++;
								}
							}
							if (arrow_end&0x02)
							{
								POINT *p=pp+num_points-1;
								if (data==597)
								{
									di2->X1=(tDrwXY)(p->x+X3);	di2->Y1=(tDrwXY)(p->y+Y3);
									di2->X2=(tDrwXY)(p->x+X4);	di2->Y2=(tDrwXY)(p->y+Y4);
									di2++;
									di2->X1=(tDrwXY)(p->x+X4);	di2->Y1=(tDrwXY)(p->y+Y4);
									di2->X2=(tDrwXY)(p->x-X3);	di2->Y2=(tDrwXY)(p->y-Y3);
									di2++;
									di2->X1=(tDrwXY)(p->x-X3);	di2->Y1=(tDrwXY)(p->y-Y3);
									di2->X2=(tDrwXY)(p->x-X4);	di2->Y2=(tDrwXY)(p->y-Y4);
									di2++;
									di2->X1=(tDrwXY)(p->x-X4);	di2->Y1=(tDrwXY)(p->y-Y4);
									di2->X2=(tDrwXY)(p->x+X3);	di2->Y2=(tDrwXY)(p->y+Y3);
									di2++;
								}
								else
								{
									di2->X1=(tDrwXY)(p->x);		di2->Y1=(tDrwXY)(p->y);
									di2->X2=(tDrwXY)(p->x-X3);	di2->Y2=(tDrwXY)(p->y-Y3);
									di2++;
									di2->X1=(tDrwXY)(p->x);		di2->Y1=(tDrwXY)(p->y);
									di2->X2=(tDrwXY)(p->x-X4);	di2->Y2=(tDrwXY)(p->y-Y4);
									di2++;
								}
								
							}
							drw->OriginalForm=0;
						}
					}
					/*if (data==591) //add radius line
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("add radius/dia.");
						CDrawing *drw=(CDrawing*)(ds->Object);
						int minx,miny,maxx,maxy;
						drw->FindRealCorner(&minx,&miny,&maxx,&maxy);
						drw->InsertItemAt(drw->NumItems);
						tDrawingItem *di=drw->Items+drw->NumItems-1;
						di->LineWidth=DRWZOOM;
						di->Type=1;
						di->pSubdrawing=NULL;
						di->X1=(minx+maxx)/2*DRWZOOM;
						di->Y1=(miny+maxy)/2*DRWZOOM;
						di->X2=(Popup_CursorX-ds->absolute_X)*DRWZOOM;
						di->Y2=(Popup_CursorY-ds->absolute_Y)*DRWZOOM;
						
					}*/
					if (data==594) //add grid lines to coordinate system
					{
						((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("add grid lines",20104);
						CDrawing *drw=(CDrawing*)(ds->Object);
						int minx,miny,maxx,maxy;
						drw->FindRealCorner(&minx,&miny,&maxx,&maxy);
						int cx=(minx+maxx)/2;
						int cy=(miny+maxy)/2;
						int lx=cx;
						int ly=cy;
						int form=0x0F;
						if (drw->OriginalForm==8)
						{
							cx=drw->Items->X1;
							cy=drw->Items->Y1;
							cx/=DRWZOOM;
							cy/=DRWZOOM;
							if (cx>maxx/2) form&=0x0E; else form&=0x0D;
							if (cy>maxy/2) form&=0x0B; else form&=0x07;
							lx=maxx-minx;
							ly=maxy-miny;
						}
						drw->OriginalForm=0;

						int g=1;
						for (int ii=GRID;ii<lx;ii+=GRID)
						{				
							tDrawingItem *di;
							if (form&0x01)
							{
							drw->InsertItemAt(0);
							di=drw->Items;
							di->LineWidth=(g%5)?(DRWZOOM/2):DRWZOOM;
							di->Type=1;
							di->pSubdrawing=NULL;
							di->X1=(ii+cx)*DRWZOOM;
							di->Y1=miny*DRWZOOM;
							di->X2=(ii+cx)*DRWZOOM;
							di->Y2=maxy*DRWZOOM;
							}
							if (form&0x02)
							{
							drw->InsertItemAt(0);
							di=drw->Items;
							di->LineWidth=(g%5)?(DRWZOOM/2):DRWZOOM;
							di->Type=1;
							di->pSubdrawing=NULL;
							di->X1=(cx-ii)*DRWZOOM;
							di->Y1=miny*DRWZOOM;
							di->X2=(cx-ii)*DRWZOOM;
							di->Y2=maxy*DRWZOOM;
							}
							g++;
						}
						g=1;
						for (int ii=GRID;ii<ly;ii+=GRID)
						{	
							tDrawingItem *di;
							if (form&0x04)
							{
							drw->InsertItemAt(0);
							di=drw->Items;
							di->LineWidth=(g%5)?(DRWZOOM/2):DRWZOOM;
							di->Type=1;
							di->pSubdrawing=NULL;
							di->X1=minx*DRWZOOM;
							di->Y1=(ii+cy)*DRWZOOM;
							di->X2=maxx*DRWZOOM;
							di->Y2=(ii+cy)*DRWZOOM;
							}
							if (form&0x08)
							{
							drw->InsertItemAt(0);
							di=drw->Items;
							di->LineWidth=(g%5)?(DRWZOOM/2):DRWZOOM;
							di->Type=1;
							di->pSubdrawing=NULL;
							di->X1=minx*DRWZOOM;
							di->Y1=(cy-ii)*DRWZOOM;
							di->X2=maxx*DRWZOOM;
							di->Y2=(cy-ii)*DRWZOOM;
							}
							g++;
						}
					}
					if (data==593) //close path
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("close path",20105);
						CDrawing *drw=(CDrawing*)(ds->Object);
						drw->IsOpenPath(1);
					}

					if (data==501) //delete
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("delete",20106);
						pMainView->DeleteDocumentObject(ds);
						ii--;
					}
					if (data==570) //select
					{
						ds->MovingDotState=3;
					}
					if ((data>=580) && (data<590)) //color changing
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("color",20107);
						if (data==589) ((CDrawing*)(ds->Object))->SetColor(0x08);
						else if (ds->Type==1) ((CExpression*)(ds->Object))->SetColor(data-581);
						else if (ds->Type==2) ((CDrawing*)(ds->Object))->SetColor(data-581);
					}
					if (ds->Type==2)  //aplicable only to drawings
					{
						CDrawing *drw=((CDrawing*)(ds->Object));
						int lw=0;
						if (data==576) lw=65*DRWZOOM/100; //hair thin line;
						if (data==502) lw=DRWZOOM; //thin line;
						if (data==577) lw=3*DRWZOOM/2; //3/2 line;
						if (data==503) lw=2*DRWZOOM; //medim line
						if (data==504) lw=3*DRWZOOM; //thick line
						if (data==566) lw=4*DRWZOOM; //fat line
						if (lw)
						{
							if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("line width",20108);
							drw->SetLineWidth(lw);
							int x,y,w,h;
							drw->AdjustCoordinates(&x,&y,&w,&h);
							ds->absolute_X+=x;
							ds->absolute_Y+=y;
						}
						/*if (data==550) //node edit
						{
							if (drw->IsNodeEdit) drw->SetNodeEdit(0); else drw->SetNodeEdit(1);
						}*/
						if (data==551) //add node
						{
							drw->SplitLineAtPos(Popup_CursorX-ds->absolute_X,Popup_CursorY-ds->absolute_Y);
						}
						if (data==513) //break apart
						{
							if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("break apart",20109);
							drw->BreakApart(NULL,NULL);
						}
						if (data==514) //combine
						{
							if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("combine",20110);
							drw->Combine();
							break;
						}
					}
					if ((data==567) || (data==568)) //Lock, Lock all
					{
						if ((ds->MovingDotState==5) && (data==567))
							ds->MovingDotState=0;
						else
						{
							if (ds->Object)
							{
								if (ds->Type==1) ((CExpression*)(ds->Object))->DeselectExpression();
								if (ds->Type==2) ((CDrawing*)(ds->Object))->SelectDrawing(0);
							}
							ds->MovingDotState=5;
						}
					}
					if (data==569) //Unlock all
					{
						ds->MovingDotState=0;
					}
					if ((data==579) && (found==0)) //align to equal sign
					{
						int maxright=0;
						int pass=0;
						((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("align",20113);

						for (;pass<2;pass++)
						for (int indx=0;indx<NumDocumentElements;indx++)
						{
							tDocumentStruct *dsx=TheDocument+indx;
							if (dsx->Type==1)
							if ((dsx->Object) && ((dsx->MovingDotState==3) || 
								((((CExpression*)(dsx->Object))->m_Selection==0x7FFF))))
							{
								CExpression *exp=(CExpression*)dsx->Object;
								int iij=0;
								tElementStruct *ts=exp->m_pElementList;
								for (;iij<exp->m_NumElements;iij++)
								{
									if (ts->Type==2)
									{
										char ch=ts->pElementObject->Data1[0];
										if ((ch=='=') || (ch=='<') || (ch=='>') || (ch==(char)0xB9) ||
											(ch==(char)0x01) || (ch==(char)0x02) || (ch==(char)0xB8) ||
											(ch==(char)0xA3) || (ch==(char)0xB3) || (ch==(char)0xBB) ||
											(ch==(char)0xA0) || (ch==(char)0x40) || (ch==(char)0xB5) ||
											(ch=='1') || (ch=='2'))
											break;
									}
									ts++;
								}
								int equalpos=ts->X_pos;
								if (iij==exp->m_NumElements) equalpos=(exp->m_pElementList+exp->m_NumElements-1)->X_pos+(exp->m_pElementList+exp->m_NumElements-1)->Length+exp->m_MarginX;
								equalpos=equalpos*100/ViewZoom;
								if (pass==0)
								{
									if (equalpos>maxright) maxright=equalpos;
								}
								else
								{
									dsx->absolute_X=StartX+maxright-equalpos;
								}
							}
						}
					}
					if (data==578) //vertical distribution
					{
						if (found==0)
						{
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("v. distribution",20112);
							int numelm=0;
							for (int indx=0;indx<NumDocumentElements;indx++)
							{
								tDocumentStruct *dsx=TheDocument+indx;
								if ((dsx->Object) && ((dsx->MovingDotState==3) || 
									((dsx->Type==2) && (((CDrawing*)(dsx->Object))->IsSelected)) ||
									((dsx->Type==1) && (((CExpression*)(dsx->Object))->m_Selection==0x7FFF))))
									numelm++;
							}
							int *list=(int*)malloc(sizeof(int)*numelm*2);
							numelm=0;
							for (int indx=0;indx<NumDocumentElements;indx++)
							{
								tDocumentStruct *dsx=TheDocument+indx;
								if ((dsx->Object) && ((dsx->MovingDotState==3) || 
									((dsx->Type==2) && (((CDrawing*)(dsx->Object))->IsSelected)) ||
									((dsx->Type==1) && (((CExpression*)(dsx->Object))->m_Selection==0x7FFF))))
								{list[numelm++]=indx;list[numelm++]=dsx->absolute_Y;}
							}
							qsort(list,numelm/2,2*sizeof(int),VerticalDistributionQSort);

							
							for (int indx=0;indx<numelm/2;indx++)
							{
								tDocumentStruct *dsx=TheDocument+*(list+indx*2);
								int theY=StartY;
								for (int indx2=0;indx2<indx;indx2++)
								{
									tDocumentStruct *dsx2=TheDocument+*(list+indx2*2);
									if ((dsx->absolute_X<=dsx2->absolute_X+dsx2->Length) && (dsx->absolute_X+dsx->Length>=dsx2->absolute_X)) 
									{
										int xxy=0;
										if ((dsx->Type==1) || (dsx2->Type==1)) xxy=8;
										if (dsx2->absolute_Y+dsx2->Below+xxy>theY) theY=dsx2->absolute_Y+dsx2->Below+xxy;
									}
								}
								dsx->absolute_Y=theY+dsx->Above;
							}
							free(list);

						}
					}

					if ((data>=560) && (data<=565)) //align
					{
						if (found==0)
						{
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("align",20113);
							if (data==560) {StartY=-1;} //left
							if (data==561) {StartX=-1;} //top
							if (data==562) {StartX=(StartX+MaxX)/2;StartY=-1;} //H.center
							if (data==563) {StartX=-1;StartY=(StartY+MaxY)/2;} //V.center
							if (data==564) {StartX=MaxX;StartY=-1;} //right
							if (data==565) {StartX=-1;StartY=MaxY;} //bottom
						}
						if (data==560) ds->absolute_X=StartX;
						if (data==562) ds->absolute_X=StartX-ds->Length/2;
						if (data==564) ds->absolute_X=StartX-ds->Length;
						if (data==561) ds->absolute_Y=StartY+ds->Above;
						if (data==565) ds->absolute_Y=StartY-ds->Below;
						if (ds->Type==1) if (data==563) ds->absolute_Y=StartY;
						if (ds->Type==2) if (data==563) ds->absolute_Y=StartY-ds->Below/2;
					}
					if (((data>=540) && (data<=549)) || ((data>=571) && (data<=575))) //rotate
					{
						if (found==0)
						{
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("rotate",20114);
							StartX=(StartX+MaxX)/2;
							StartY=(StartY+MaxY)/2;
						}
						int x1=0,y1=0,w=0,h=0;
						double angle=0;
						if (data==540) angle=5;
						if (data==541) angle=15;
						if (data==542) angle=45;
						if (data==543) angle=-5;
						if (data==544) angle=-15;
						if (data==545) angle=-45;
						if (data==571) angle=90;
						if (data==572) angle=-90;
						if (data==574) angle=60;
						if (data==575) angle=-60;
						if (data==573) 
						{
							angle=atof(ValueEntryBoxString);
							if (ValueEntryBox)
							{
								ValueEntryBox->DestroyWindow();
								delete ValueEntryBox;
								ValueEntryBox=NULL;
							}
							else if (found==0)
							{
								ValueEntryBox=new CEdit();
								ValueEntryBox->Create(ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_VISIBLE | WS_CHILD | WS_BORDER,CRect(Options[m_SelectedOption].X,Options[m_SelectedOption].Y-1,Options[m_SelectedOption].X+Options[m_SelectedOption].Cx,Options[m_SelectedOption].Y+Options[m_SelectedOption].Cy+2),this,1);
								HFONT font=GetFontFromPool(4,0,1,TSize/4-2);
								ValueEntryBox->SendMessage(WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
								ValueEntryBox->SetFocus();
								ValueEntryBoxData=m_SelectedOption;
								CWnd::OnLButtonDown(nFlags, point);
								return;
							}
						}
						angle=angle*3.14159265/180.0;
						if (ds->Type==2)
						{
							((CDrawing*)(ds->Object))->RotateForAngle((float)angle,-ds->absolute_X+StartX,-ds->absolute_Y+StartY,&x1,&y1,&w,&h);
							ds->absolute_X=StartX+x1;
							ds->absolute_Y=StartY+y1;
							ds->Length=w;
							ds->Below=h;
						}
						if (ds->Type==1)
						{
							double curangle;
							int Y=ds->absolute_Y;
							int X=ds->absolute_X;
							double rad=sqrt((double)(X-StartX)*(double)(X-StartX)+(double)(Y-StartY)*(double)(Y-StartY));
							curangle=atan2((double)(-Y+StartY),(double)(X-StartX));
							curangle+=angle;
							ds->absolute_X=(int)(rad*cos(curangle))+StartX;
							ds->absolute_Y=-(int)(rad*sin(curangle))+StartY;
						}						
					}
					if (((data>=507) && (data<=512)) || 
						 ((data>=515) && (data<=536))) //-50%...+50% scaling/stretching, horizontal/vertical mirror
					{
						//ostaju za iskoristiti 516,523,524,525,526,533,534
						if (found==0)
						{
							if ((data==535) || (data==536))
								((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("mirror",20115);
							else
								((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("size",20116);
							StartX=(StartX+MaxX)/2;
							StartY=(StartY+MaxY)/2;
						}
						float factorx=1.0f;
						float factory=1.0f;
						if (data==507) {factorx=0.5f;factory=0.5f;}
						if (data==508) {factorx=0.85f;factory=0.85f;}
						if (data==509) {factorx=0.95f;factory=0.95f;}
						if (data==510) {factorx=1.05f;factory=1.05f;}
						if (data==511) {factorx=1.15f;factory=1.15f;}
						if (data==512) {factorx=1.5f;factory=1.5f;}
						if (data==517) {factorx=0.5f;factory=1.0f;}
						if (data==518) {factorx=0.85f;factory=1.0f;}
						if (data==519) {factorx=0.95f;factory=1.0f;}
						if (data==520) {factorx=1.05f;factory=1.0f;}
						if (data==521) {factorx=1.15f;factory=1.0f;}
						if (data==522) {factorx=1.5f;factory=1.0f;}
						if (data==527) {factorx=1.0f;factory=0.5f;}
						if (data==528) {factorx=1.0f;factory=0.85f;}
						if (data==529) {factorx=1.0f;factory=0.95f;}
						if (data==530) {factorx=1.0f;factory=1.05f;}
						if (data==531) {factorx=1.0f;factory=1.15f;}
						if (data==532) {factorx=1.0f;factory=1.5f;}
						if (data==535) {factorx=-1.0f;factory=1.0f;} //horizontal mirror
						if (data==536) {factorx=1.0f;factory=-1.0f;} //vertical mirror
						if (data==515) 
						{
							if ((ValueEntryBoxString[0]=='+') || (ValueEntryBoxString[0]=='-')) 
								factorx=factory=(float)(1+atof(ValueEntryBoxString)/100);
							else 
								factorx=factory=(float)(atof(ValueEntryBoxString)/100);

							if (ValueEntryBox)
							{
								ValueEntryBox->DestroyWindow();
								delete ValueEntryBox;
								ValueEntryBox=NULL;
							}
							else if (found==0)
							{
								ValueEntryBox=new CEdit();
								ValueEntryBox->Create(ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_VISIBLE | WS_CHILD | WS_BORDER,CRect(Options[m_SelectedOption].X,Options[m_SelectedOption].Y-1,Options[m_SelectedOption].X+Options[m_SelectedOption].Cx,Options[m_SelectedOption].Y+Options[m_SelectedOption].Cy+2),this,1);
								HFONT font=GetFontFromPool(4,0,1,TSize/4-2);
								ValueEntryBox->SendMessage(WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
								ValueEntryBox->SetFocus();
								ValueEntryBoxData=m_SelectedOption;
								CWnd::OnLButtonDown(nFlags, point);
								return;
							}
						}
						if (ds->Type==2)
							((CDrawing*)(ds->Object))->ScaleForFactor(factorx,factory);
						if (ds->Type==1)
						{
							if ((factorx!=1.0) && (factory!=1.0))
								((CExpression*)(ds->Object))->ChangeFontSize(factorx);
						}

						ds->Length=abs((int)(ds->Length*factorx));
						ds->Below=abs((int)(ds->Below*factory));
						float gx=(float)(ds->absolute_X-StartX)*(factorx);
						float gy=(float)(ds->absolute_Y-StartY)*(factory);
						ds->absolute_X=StartX+(int)gx;		
						if ((gx-(int)gx)>0.5) ds->absolute_X++;
						if (factorx<0) ds->absolute_X-=ds->Length;
						ds->absolute_Y=StartY+(int)gy;
						if ((gy-(int)gy)>0.5) ds->absolute_Y++;
						if (factory<0) ds->absolute_Y-=ds->Below;
					}

					if ((data==506) && (ds->Type==2)) //ungroup option
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("ungroup",20117);
						CDrawing *drw=(CDrawing*)ds->Object;
						for (int kk=0;kk<drw->NumItems;kk++)
						{
							tDrawingItem *di=drw->Items+kk;
							if (di->pSubdrawing)
							{
								short l,a=0,b;
								if (di->Type==2)
								{
									CDC *DC=this->GetDC();
									((CExpression*)(di->pSubdrawing))->CalculateSize(DC,ViewZoom,&l,&a,&b);
									this->ReleaseDC(DC);
								}
								//CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;
								AddDocumentObject(((di->Type==0)?2:1),ds->absolute_X+(di->X1)/DRWZOOM,ds->absolute_Y+(di->Y1)/DRWZOOM+a*100/ViewZoom);
								ds=TheDocument+ii;
								tDocumentStruct *ds2=TheDocument+NumDocumentElements-1;
								ds2->Above=0;
								ds2->Below=(di->Y2-di->Y1)/DRWZOOM;
								ds2->Length=(di->X2-di->X1)/DRWZOOM;
								ds2->MovingDotState=0;
								ds2->Object=(CObject*)di->pSubdrawing;
								if (di->Type==0)
									((CDrawing*)(di->pSubdrawing))->SelectDrawing(0);
								else if (di->Type==2)
									((CExpression*)(di->pSubdrawing))->DeselectExpression();
								for (int kkk=kk;kkk<drw->NumItems-1;kkk++)
									*(drw->Items+kkk)=*(drw->Items+kkk+1);
								drw->NumItems--;
								kk--;
							}
							//if (drw->NumItems==0)
							//	pMainView->DeleteDocumentObject(ds);
						}
						if (drw->NumItems==0)
						{
							pMainView->DeleteDocumentObject(ds);
							ii--;
						}
					}

					if (data==505) //group option
					{
						if (found==0) ((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("group",20118);
						int X=ds->absolute_X;
						int Y=ds->absolute_Y;
						if (ds->Type==1) Y-=ds->Above;
						if (tmpdrw==NULL)
						{
							StartX=X;
							StartY=Y;
							tmpdrw=new CDrawing();
						}
						if (ds->Type==2)
							tmpdrw->CopyDrawingIntoSubgroup((CDrawing*)ds->Object,X-StartX,Y-StartY);
						else if (ds->Type==1)
							tmpdrw->CopyExpressionIntoSubgroup((CExpression*)ds->Object,X-StartX,Y-StartY,ds->Length,ds->Below+ds->Above);
						int xx,yy,w,h;
						tmpdrw->AdjustCoordinates(&xx,&yy,&w,&h);
						StartX+=xx;
						StartY+=yy;
						//if (X<StartX) StartX=X;
						//if (Y<StartY) StartY=Y;
						if ((found) && (prevelement)) {pMainView->DeleteDocumentObject(prevelement);ii--;}
						prevelement=TheDocument+ii;
					}


					found++;
				}
			}
			if ((data==505) && (found>1)) //group
			{
				if (prevelement) 
					pMainView->DeleteDocumentObject(prevelement);
				//CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;
				AddDocumentObject(2,StartX,StartY);
				tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
				short w=0,h=0;
				CDC *DC=pMainView->GetDC();
				tmpdrw->CalculateSize(DC,ViewZoom,&w,&h);
				pMainView->ReleaseDC(DC);
				ds->Above=0;
				ds->Below=h;
				ds->Length=w;
				ds->MovingDotState=0;
				ds->Object=(CObject*)tmpdrw;
			} 
			else if (tmpdrw)
			{
				delete tmpdrw;
			}
			pMainView->RepaintTheView();


			if (DirectCallFlag) return;
		}




		if ((((Options[m_SelectedOption].Data>=3) && (Options[m_SelectedOption].Data<=7)) || (Options[m_SelectedOption].Data==130)) && (m_Expression))
		{
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("decoration",20200);
			int data=Options[m_SelectedOption].Data;
			int ii;
			for (ii=0;ii<m_Expression->m_NumElements;ii++)
			{
				tElementStruct *ts=m_Expression->m_pElementList+ii;
				if (ts->IsSelected) 
				{
					if ((ts->Type!=11) && (ts->Type!=12)) //vertical and horizontal separators cannot be decorated
					{
						if (data==3) ts->Decoration=0; //no-decoration
						if (data==4) ts->Decoration=1; //strikeout
						if (data==5) ts->Decoration=2; //encircled
						if (data==6) ts->Decoration=3; //underline
						if (data==7) ts->Decoration=4; //overline
						if (data==130) ts->Decoration=5; //underbrace
					}
					else 
						ts->Decoration=0;
				}
			}
		}
		if (Options[m_SelectedOption].Data==8) //then "lock" option
		{

			for (int ii=0;ii<NumDocumentElements;ii++)
			{
				tDocumentStruct *ds=TheDocument+ii;
				if ((ds->Type==1) && (ds->Object) && (ds->Object==(CObject*)m_Expression))
				{
					if (ds->MovingDotState==5)
						ds->MovingDotState=0;
					else
					{
						m_Expression->DeselectExpression();
						ds->MovingDotState=5;
					}
				}
			}
		}
		if ((Options[m_SelectedOption].Data==1) || (Options[m_SelectedOption].Data==2) 
			 || (Options[m_SelectedOption].Data==9) || (Options[m_SelectedOption].Data==29)) //CUT or DEL or COPY or PASTE
		{
			if (Options[m_SelectedOption].Data==29)
			{
				if (ClipboardExpression)
				{
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("Paste",20201);
					m_Expression->CopyAtPoint(NULL,ViewZoom,-1,-1,ClipboardExpression);
					delete_clipboard_at_exit=1;
				}
			}
			else
			{
			if ((Options[m_SelectedOption].Data==1) || (Options[m_SelectedOption].Data==9))
			{
				//copies selection into clipboard
				ClipboardExpression=new CExpression(NULL,NULL,100);
				ClipboardExpression->CopyExpression(m_Expression,1);
			}

			if ((m_Expression->AdjustSelection()) && (Options[m_SelectedOption].Data!=9))
			{	
				int is_keyboard_entry=0;
				if (m_Expression==(CExpression*)KeyboardEntryObject)
				{
					//mark the element where the keyboard entry is active
					is_keyboard_entry=1;
					(m_Expression->m_pElementList+m_Expression->m_IsKeyboardEntry-1)->Decoration|=0x40;
				}

				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave((Options[m_SelectedOption].Data==1)?"Pick up":"Delete",(Options[m_SelectedOption].Data==1)?20202:20203);
				CExpression *parent=m_Expression->m_pPaternalExpression;
				if (m_Expression->DeleteSelection()==2) {m_Expression=parent;is_keyboard_entry=0;}
				if (m_Expression->m_NumElements==0) {m_Expression->InsertEmptyElement(0,0,0);is_keyboard_entry=0;}
				if ((is_keyboard_entry) && (m_Expression==(CExpression*)KeyboardEntryObject))
				{
					//change the keyboard entry focus point to the element that is marked
					for (int kk=0;kk<m_Expression->m_NumElements;kk++)
						if ((m_Expression->m_pElementList+kk)->Decoration&0x40)
						{
							(m_Expression->m_pElementList+kk)->Decoration&=0x3F;
							m_Expression->m_IsKeyboardEntry=kk+1;
							break;
						}
				}
			}
			}

		}
		if ((Options[m_SelectedOption].Data>=90) && (Options[m_SelectedOption].Data<99))  //alignment options
		{
			int kaka=Options[m_SelectedOption].Data;
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave((kaka==93)?"vertical":((kaka>93)?"expanding":"alignment"),(kaka==93)?20204:((kaka>93)?20205:20206));
			if (kaka==90) m_Expression->m_Alignment=1;
			if (kaka==91) m_Expression->m_Alignment=0;
			if (kaka==92) m_Expression->m_Alignment=2;
			if ((kaka>=90) && (kaka<=92) && ((m_Expression->m_MaxNumColumns>1) || (m_Expression->m_MaxNumRows>1)))
			{
				tCellAttributes attrib;
				for (int i=0;i<m_Expression->m_MaxNumColumns;i++)
					for (int j=0;j<m_Expression->m_MaxNumRows;j++)
						if (m_Expression->GetCellAttributes(j,i,&attrib))
							*attrib.alignment=' ';
			}
			if (kaka==93) m_Expression->m_IsVertical=(m_Expression->m_IsVertical)?0:1;
			if (kaka==94) //expand ouside (adds insertion point outside of an expression)
			{
				m_Expression->InsertEmptyElement(0,5,m_Expression->m_ParentheseShape);
				m_Expression->m_ParenthesesFlags&=0x02;
				CExpression *exp=((CExpression*)(m_Expression->m_pElementList->pElementObject->Expression1));
				exp->m_Alignment=m_Expression->m_Alignment;
				exp->m_StartAsText=m_Expression->m_StartAsText;
				exp->m_FontSize=m_Expression->m_FontSize;
				//exp->m_FontSizeHQ=m_Expression->m_FontSizeHQ;
				exp->m_Color=m_Expression->m_Color;
				while (m_Expression->m_NumElements>1)
				{
					exp->InsertElement(m_Expression->m_pElementList+1,exp->m_NumElements);
					m_Expression->DeleteElement(1);
				}
				m_Expression->m_MaxNumColumns=1;
				m_Expression->m_MaxNumRows=1;
				if (m_Expression->m_MatrixRows) {HeapFree(ProcessHeap,0,m_Expression->m_MatrixRows);m_Expression->m_MatrixRows=NULL;}
				if (m_Expression->m_MatrixColumns) {HeapFree(ProcessHeap,0,m_Expression->m_MatrixColumns);m_Expression->m_MatrixColumns=NULL;}
			}
			if (kaka==95) //condense outside
			{
				for (int ii=0;ii<m_Expression->m_NumElements;ii++)
				{
					m_Expression->m_pPaternalExpression->InsertElement(m_Expression->m_pElementList+ii,m_Expression->m_pPaternalExpression->m_NumElements);
				}
				m_Expression->m_pPaternalExpression->m_ParentheseShape=m_Expression->m_ParentheseShape;
				m_Expression->m_pPaternalExpression->m_ParenthesesFlags=m_Expression->m_ParenthesesFlags;
				m_Expression->m_pPaternalExpression->m_ParenthesesFlags|=0x81;
				//m_Expression->m_pPaternalExpression->m_ParentheseHeightFactor=m_Expression->m_ParentheseHeightFactor;
				m_Expression->m_pPaternalExpression->m_Alignment=m_Expression->m_Alignment;
				m_Expression->m_pPaternalExpression->m_StartAsText=m_Expression->m_StartAsText;
				m_Expression->m_pPaternalExpression->m_FontSize=m_Expression->m_FontSize;
				//m_Expression->m_pPaternalExpression->m_FontSizeHQ=m_Expression->m_FontSizeHQ;
				m_Expression->m_pPaternalExpression->m_Color=m_Expression->m_Color;

				m_Expression->m_pPaternalExpression->DeleteElement(0);
			}
			/*if (kaka==96) //toggle inline spacing
			{
				m_Expression->m_IsText=(m_Expression->m_IsText)?0:1;
			}*/

		}
		if ((Options[m_SelectedOption].Data>=80) && (Options[m_SelectedOption].Data<90))
		{
			//color options
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("color",20107);
			int data=Options[m_SelectedOption].Data;
			if (m_MenuType==1)
			{
				m_Expression->SetColor(data-81);
			}
			else if (m_theSelectedElement)
			{
				int ii;
				for (ii=0;ii<m_Expression->m_NumElements;ii++)
				{
					tElementStruct *ts=m_Expression->m_pElementList+ii;
					if ((ts->IsSelected) && 
						(ts->pElementObject)) //types 0,12,11 don't have objects
						ts->pElementObject->SetColor(data-81);
				}
			}
		}

		if ((Options[m_SelectedOption].Data>=70) && (Options[m_SelectedOption].Data<80))
		{
			//condition list options
			int data=Options[m_SelectedOption].Data;
			if (data==70)  //left bar
			{
				if (m_theSelectedElement->pElementObject->Data1[0]&0x01)
					m_theSelectedElement->pElementObject->Data1[0]&=0xFE;
				else
					m_theSelectedElement->pElementObject->Data1[0]|=0x01;
			}
			if (data==71)  //right bar
			{
				if (m_theSelectedElement->pElementObject->Data1[0]&0x02)
					m_theSelectedElement->pElementObject->Data1[0]&=0xFD;
				else
					m_theSelectedElement->pElementObject->Data1[0]|=0x02;
			}
			if (data==72) //left aligned
			{
				m_theSelectedElement->pElementObject->Data2[0]=0;
				if (m_theSelectedElement->pElementObject->Expression1) ((CExpression*)(m_theSelectedElement->pElementObject->Expression1))->m_Alignment=1;
				if (m_theSelectedElement->pElementObject->Expression2) ((CExpression*)(m_theSelectedElement->pElementObject->Expression2))->m_Alignment=1;
				if (m_theSelectedElement->pElementObject->Expression3) ((CExpression*)(m_theSelectedElement->pElementObject->Expression3))->m_Alignment=1;
			}
			if (data==73) 
			{
				m_theSelectedElement->pElementObject->Data2[0]=1;  //center aligned
				if (m_theSelectedElement->pElementObject->Expression1) ((CExpression*)(m_theSelectedElement->pElementObject->Expression1))->m_Alignment=0;
				if (m_theSelectedElement->pElementObject->Expression2) ((CExpression*)(m_theSelectedElement->pElementObject->Expression2))->m_Alignment=0;
				if (m_theSelectedElement->pElementObject->Expression3) ((CExpression*)(m_theSelectedElement->pElementObject->Expression3))->m_Alignment=0;
			}
			if (data==74) 
			{
				m_theSelectedElement->pElementObject->Data2[0]=2;  //right aligned
				if (m_theSelectedElement->pElementObject->Expression1) ((CExpression*)(m_theSelectedElement->pElementObject->Expression1))->m_Alignment=2;
				if (m_theSelectedElement->pElementObject->Expression2) ((CExpression*)(m_theSelectedElement->pElementObject->Expression2))->m_Alignment=2;
				if (m_theSelectedElement->pElementObject->Expression3) ((CExpression*)(m_theSelectedElement->pElementObject->Expression3))->m_Alignment=2;
			}

			if (data==78) //converting to hyperlink
			{
				int l=m_Expression->m_NumElements;
				for (int i=0;i<l;i++)
				{
					if ((m_Expression->m_pElementList+i)->IsSelected)
					{
						m_Expression->InsertEmptyElement(i,9,'H'); //adds hyperlink element
						l++;

						CExpression *e=(CExpression*)((m_Expression->m_pElementList+i)->pElementObject->Expression1);
						for (int j=i+1;j<l;j++)
						{
							tElementStruct *ts=m_Expression->m_pElementList+j;
							if (ts->IsSelected==0) break;
							ts->IsSelected=0;
							e->InsertElement(ts,e->m_NumElements);
							m_Expression->DeleteElement(j);
							j--;
							l--;
						}

						(m_Expression->m_pElementList+i)->IsSelected=1;
						m_theSelectedElement=m_Expression->m_pElementList+i;
						break;
					}
				}

			}

			if (data==77) //hyperlink definition
			{
				if (*((char**)m_theSelectedElement->pElementObject->Data3)==NULL)
				{
					char *link=(char*)malloc(340);
					strcpy(link,"http://");
					*((char**)m_theSelectedElement->pElementObject->Data3)=link;
				}
				if (ValueEntryBox)
				{
					char *url=*((char**)m_theSelectedElement->pElementObject->Data3);
					ValueEntryBox->GetWindowTextA(url,299);
					int len=(int)strlen(url);
					for (int i=0;i<len;i++)
					{
						if (url[i]<32)
						{
							for (int j=i;j<len-1;j++)
								url[j]=url[j+1];
							len--;
							i--;
						}					
					}
					url[len]=0;
					if (len==0) *((char**)m_theSelectedElement->pElementObject->Data3)=NULL;
					
					ValueEntryBox->DestroyWindow();
					delete ValueEntryBox;
					ValueEntryBox=NULL;
				}
				else
				{
					ValueEntryBox=new CEdit();
					ValueEntryBox->Create(ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN|WS_VISIBLE | WS_CHILD | WS_BORDER,CRect(Options[m_SelectedOption].X,Options[m_SelectedOption].Y-1,Options[m_SelectedOption].X+Options[m_SelectedOption].Cx,Options[m_SelectedOption].Y+Options[m_SelectedOption].Cy+2),this,1);
					HFONT font=GetFontFromPool(4,0,1,TSize/4-2);
					ValueEntryBox->SendMessage(WM_SETFONT,(WPARAM)font,MAKELPARAM(1,0));
					char *url=*((char**)m_theSelectedElement->pElementObject->Data3);
					ValueEntryBox->SetWindowTextA(url);
					ValueEntryBox->SetFocus();
					ValueEntryBoxData=m_SelectedOption;
					CWnd::OnLButtonDown(nFlags, point);
					return;
				}

			}
		}

		if ((Options[m_SelectedOption].Data>=50) && (Options[m_SelectedOption].Data<60) && (m_theSelectedElement))
		{
			//symbol size (height) menu handling
			int data=Options[m_SelectedOption].Data;
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("symbol height",20208);
			if (data==50) //large size
				m_theSelectedElement->pElementObject->Data2[0]=0;
			if (data==51) //medium size
				m_theSelectedElement->pElementObject->Data2[0]=1;
			if (data==52) //small size
				m_theSelectedElement->pElementObject->Data2[0]=2;
			if (data==53) //limits aside
				m_theSelectedElement->pElementObject->Data2[1]=(m_theSelectedElement->pElementObject->Data2[1]==1)?0:1;


		}
		if (Options[m_SelectedOption].Data==60)
		{
			//the "implanting paste" option
			
			//first create a temporary storage expression
			CExpression *tmpExpression=new CExpression(NULL,NULL,100);
			if (tmpExpression==NULL) return;

			//then copy current selection object into it
			tmpExpression->CopyExpression(m_Expression,1);
			short l,a,b;
			CDC *dcc=this->GetDC();
			tmpExpression->CalculateSize(dcc,ViewZoom,&l,&a,&b);

			//copy this into the clipboard expression, at exact mouse position
			ClipboardExpression->CopyAtPoint(dcc,
				(short)(Options[m_SelectedOption].DataArray[3]),
				(short)(point.x-Options[m_SelectedOption].DataArray[4]),
				(short)(point.y-Options[m_SelectedOption].DataArray[5]),tmpExpression);

			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("implanting",20209);

			ClipboardExpression->CalculateSize(dcc,ViewZoom,&l,&a,&b);
			m_Expression->CopyAtPoint(dcc,ViewZoom,-1,-1,ClipboardExpression);
			this->ReleaseDC(dcc);

			//deleting clipboard, and temporary element;
			if (ClipboardExpression) 
			{
				delete ClipboardExpression;
				ClipboardExpression=NULL;
				Options[m_SelectedOption].Graphics=NULL;
			}
			delete tmpExpression;

		}

		//computation choice
		if ((Options[m_SelectedOption].Data==61) || (Options[m_SelectedOption].Data==62))
		{

			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("computation",20210);
			if (ClipboardExpression) delete_clipboard_at_exit=1;

			if ((this->m_Expression==NULL) && (m_SelectedSuboption!=2))
			{
				//handling for system of equations - there was a selected group of equations

				//search low-left corner
				int minX=0x7FFFFFFF,maxY=-0x7FFFFFFF;
				for (int i=0;i<NumDocumentElements;i++)
					if (TheDocument[i].MovingDotState==3)
					{
						if (TheDocument[i].absolute_X<minX) minX=TheDocument[i].absolute_X;
						if (TheDocument[i].absolute_Y+TheDocument[i].Below>maxY) maxY=TheDocument[i].absolute_Y+TheDocument[i].Below;
					}
				
				//create the new element in the main document
				tDocumentStruct *ds;

				//NumDocumentElements++;
				//pMainView->CheckDocumentMemoryReservations();

				CExpression *copy2=new CExpression(NULL,NULL,DefaultFontSize);
				copy2->CopyExpression(Options[m_SelectedOption].Graphics,0);

				short l,a,b;
				CDC *DC;
				DC=pMainView->GetDC();
				((CExpression*)(copy2))->CalculateSize(DC,100,&l,&a,&b);
				pMainView->ReleaseDC(DC);

				int delta2=a+b+5+(a+b)/6;

				if (IsShowGrid)
				{
					delta2=(delta2/GRID+1)*GRID;
				}

				AddDocumentObject(1,minX,maxY+a+5+a/6);
				ds=TheDocument+NumDocumentElements-1;
				//ds->absolute_X=minX; 
				//ds->absolute_Y=maxY+a+5+a/6;
				//ds->Type=1; //expression
				ds->Object=(CObject*)copy2;
				ds->Length=(short)((int)l*100/(int)ViewZoom);
				ds->Above=(short)((int)a*100/(int)ViewZoom);
				ds->Below=(short)((int)b*100/(int)ViewZoom);
				ds->MovingDotState=(char)0x80;

				
				((CMainFrame*)(theApp.m_pMainWnd))->RearangeObjects(delta2);

				pMainView->m_PopupMenuObject=ds;

				if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
				if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;
				pMainView->RepaintTheView();

			}
			else if (m_SelectedSuboption==1)
			{
				// an updated copy of expression is to be created just
				// below the edited expression

				// search the main document and find the edited object
				CExpression *parent=m_Expression;
				while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;
				tDocumentStruct *org_ds;
				int org_ds_pos=0;
				int i=0;
				for (i=0;i<NumDocumentElements;i++)
				{
					org_ds=TheDocument+i;
					org_ds_pos=i;
					if (org_ds->Object==(CObject*)parent) break;
				}
				

				if ((m_OwnerType==3) && (KeyboardEntryBaseObject))
				{
					parent=(CExpression*)(KeyboardEntryBaseObject->Object);
					i=0;
					
				}
				if (i<NumDocumentElements)
				{			
					//the edited object is found - proceed
					short l,a,b;
					CExpression *copy2;
					int fs=((CExpression*)(org_ds->Object))->m_FontSize;
					//int fsh=((CExpression*)(org_ds->Object))->m_FontSizeHQ;
					if ((m_OwnerType!=3) || (KeyboardEntryBaseObject==NULL))
					{
						CExpression *copy=new CExpression(NULL,NULL,100);
						copy->CopyExpression(parent,0);
						int ps=parent->m_ParentheseShape;
						//int ph=parent->m_ParentheseHeightFactor;
						//int pd=parent->m_ParentheseData;
						int fp=parent->m_ParenthesesFlags;

						if (Options[m_SelectedOption].Data==62)
						{
							CExpression *parent2=Options[m_SelectedOption].Parent;
							int ps2=parent2->m_ParentheseShape;
							//int ph2=parent2->m_ParentheseHeightFactor;
							//int pd2=parent2->m_ParentheseData;
							int fp2=parent2->m_ParenthesesFlags;
							parent2->CopyExpression(Options[m_SelectedOption].Graphics,0);
							parent2->m_ParentheseShape=ps2;
							//parent2->m_ParentheseHeightFactor=ph2;
							//parent2->m_ParentheseData=pd2;
							parent2->m_ParenthesesFlags=fp2;
						}
						else
						{
							if ((StartExtendedSelection==0) && (EndExtendedSelection==m_Expression->m_NumElements-1))
							{
								int prnth=m_Expression->m_ParenthesesFlags;
								m_Expression->CopyExpression(Options[m_SelectedOption].Graphics,0);
								m_Expression->m_ParenthesesFlags=prnth;
							}
							else
							{
								for (int ii=StartExtendedSelection;ii<=EndExtendedSelection;ii++)
									m_Expression->DeleteElement(StartExtendedSelection);

								m_Expression->InsertSequence(m_Expression->GetDefaultElementType(LevelExtendedSelection),StartExtendedSelection,Options[m_SelectedOption].Graphics,0,Options[m_SelectedOption].Graphics->m_NumElements-1);
							}
						}
						copy2=new CExpression(NULL,NULL,fs);
						//copy2->m_FontSizeHQ=fsh;
						copy2->CopyExpression(parent,0);
						parent->CopyExpression(copy,0);
						parent->m_ParentheseShape=ps;
						//parent->m_ParentheseHeightFactor=ph;
						//parent->m_ParentheseData=pd;
						parent->m_ParenthesesFlags=fp;
						copy2->m_ParentheseShape=ps;
						//copy2->m_ParentheseHeightFactor=ph;
						//copy2->m_ParentheseData=pd;
						copy2->m_ParenthesesFlags=fp;
						delete copy;

						CDC *DC;
						DC=pMainView->GetDC();
						parent->CalculateSize(DC,100,&l,&a,&b);
						((CExpression*)(copy2))->CalculateSize(DC,100,&l,&a,&b);
						pMainView->ReleaseDC(DC);
					}
					else
					{
						//m_OwnerType==3 -> in keyboard entry mode when two times '?' was pressed
						copy2=new CExpression(NULL,NULL,fs);
						//copy2->m_FontSizeHQ=fsh;
						copy2->CopyExpression(Options[m_SelectedOption].Graphics,0);
						
						if (this->m_UserParam==0) copy2->InsertEmptyElement(0,2,'=');
						m_Expression->Delete();
						((CExpression*)(KeyboardEntryObject))->KeyboardStop();
						CDC *DC;
						DC=pMainView->GetDC();
						((CExpression*)(copy2))->CalculateSize(DC,100,&l,&a,&b);
						copy2->m_Selection=copy2->m_NumElements+1;
						copy2->KeyboardStart(DC,ViewZoom);
						pMainView->ReleaseDC(DC);
					}



					//create the new element in the main document
					tDocumentStruct *ds;

					//NumDocumentElements++;
					//pMainView->CheckDocumentMemoryReservations();

					org_ds=(TheDocument+org_ds_pos);

					int delta=org_ds->Below+a+(org_ds->Below+a)/6+5;
					int delta2=a+b+5+(a+b)/6;

					if (IsShowGrid)
					{
						delta=(delta/GRID+1)*GRID;
						delta2=(delta2/GRID+1)*GRID;
					}


					AddDocumentObject(1,org_ds->absolute_X,org_ds->absolute_Y+delta);
					ds=TheDocument+NumDocumentElements-1;
					//ds->absolute_X=org_ds->absolute_X; 
					//ds->absolute_Y=org_ds->absolute_Y+delta;
					//ds->Type=1; //expression
					ds->Object=(CObject*)copy2;
					ds->Length=(short)((int)l*100/(int)ViewZoom);
					ds->Above=(short)((int)a*100/(int)ViewZoom);
					ds->Below=(short)((int)b*100/(int)ViewZoom);
					ds->MovingDotState=(char)0x80;
					for (int i=0;i<NumDocumentElements-1;i++)
						if ((TheDocument+i)->absolute_Y<ds->absolute_Y)
							(TheDocument+i)->MovingDotState|=(char)0x40;

					((CMainFrame*)(theApp.m_pMainWnd))->RearangeObjects(delta2);
					
					pMainView->m_PopupMenuObject=ds;

					if (m_OwnerType==3)
					{
						KeyboardEntryObject=ds->Object;
						KeyboardEntryBaseObject=ds;
					}
					

					if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
					if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;
					pMainView->RepaintTheView();
				}
			}
			else if (m_SelectedSuboption==2)
			{
				if ((m_OwnerType==3) && (KeyboardEntryObject))
				{
					((CExpression*)KeyboardEntryObject)->KeyboardStop();
					KeyboardEntryObject=NULL;
					KeyboardEntryBaseObject=NULL;
				}
				if (ClipboardExpression==NULL) ClipboardExpression=new CExpression(NULL,NULL,100);
				ClipboardExpression->CopyExpression(Options[m_SelectedOption].Graphics,0);
				delete_clipboard_at_exit=0;
			}
			else
			{		
				if (Options[m_SelectedOption].Data==62)
				{
					CExpression *parent2=Options[m_SelectedOption].Parent;
					int ps=parent2->m_ParentheseShape;
					//int ph=parent2->m_ParentheseHeightFactor;
					//int pd=parent2->m_ParentheseData;
					int fp=parent2->m_ParenthesesFlags;
					parent2->CopyExpression(Options[m_SelectedOption].Graphics,0);
					parent2->m_ParentheseShape=ps;
					//parent2->m_ParentheseHeightFactor=ph;
					//parent2->m_ParentheseData=pd;
					parent2->m_ParenthesesFlags=fp;
				}
				else
				{
					if ((StartExtendedSelection==0) && (EndExtendedSelection==m_Expression->m_NumElements-1))
					{
						int prnth=m_Expression->m_ParenthesesFlags;
						m_Expression->CopyExpression(Options[m_SelectedOption].Graphics,0);
						m_Expression->m_ParenthesesFlags=prnth;
						m_Expression->AddToStackClipboard(1);
					}
					else
					{
						for (int ii=StartExtendedSelection;ii<=EndExtendedSelection;ii++)
							m_Expression->DeleteElement(StartExtendedSelection);

						Options[m_SelectedOption].Graphics->AddToStackClipboard(1);
						m_Expression->InsertSequence(m_Expression->GetDefaultElementType(LevelExtendedSelection),StartExtendedSelection,Options[m_SelectedOption].Graphics,0,Options[m_SelectedOption].Graphics->m_NumElements-1);
					}
				}
			}
		}


		if (((Options[m_SelectedOption].Data>=30) && (Options[m_SelectedOption].Data<50)) || 
			((Options[m_SelectedOption].Data>=54) && (Options[m_SelectedOption].Data<60)) || 
			((Options[m_SelectedOption].Data>=150) && (Options[m_SelectedOption].Data<160)))
		{
			//parenthese menu handling
			CExpression *tmpExpression=m_Expression;
			if (m_MenuType==4)
				tmpExpression=((CExpression*)(m_theSelectedElement->pElementObject->Expression1));

			int data=Options[m_SelectedOption].Data;

			if ((data==45) || (data==46) || (data==47) || (data==48) || (data==55) || (data==54))
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("font size",20211);
			else
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("parentheses format",20212);

			if (data==30) //'none' option chosen (no parentheses)
			{
				tmpExpression->m_ParenthesesFlags&=0xFE; //clearing the first bit
			}
			if (((data>=32) && (data<42)) || (data==49) || ((data>=56) && (data<=59))) //shape options
			{
				tmpExpression->m_ParenthesesFlags|=0x01; //seting the first bit
				if (data==32) tmpExpression->m_ParentheseShape='('; // ( ) shape
				if (data==33) tmpExpression->m_ParentheseShape='['; // [ ] shape
				if (data==34) tmpExpression->m_ParentheseShape='{'; // { } shape
				if (data==35) tmpExpression->m_ParentheseShape='|'; // | | shape
				if (data==36) tmpExpression->m_ParentheseShape='/'; // / / shape
				if (data==37) tmpExpression->m_ParentheseShape='\\'; /* \ \ shape  */
				if (data==38) tmpExpression->m_ParentheseShape='<'; /* < > shape  */
				if (data==39) tmpExpression->m_ParentheseShape='b'; /* box shape  */
				if (data==40) tmpExpression->m_ParentheseShape='c'; /* ceiling*/
				if (data==41) tmpExpression->m_ParentheseShape='f'; /* floor*/
				if (data==49) tmpExpression->m_ParentheseShape='x'; /* crossed shape  */
				if (data==56) tmpExpression->m_ParentheseShape='l'; /* <]  */
				if (data==57) tmpExpression->m_ParentheseShape='r'; /* [>  */
				if (data==58) tmpExpression->m_ParentheseShape='a'; /* <|  - bra*/
				if (data==59) tmpExpression->m_ParentheseShape='k'; /* |> - ket  */
			}
			if (((data>=43) && (data<49)) || (data==54) || (data==55)) //size options
			{
				if (data==45) m_Expression->ChangeFontSize((float)0.95);  //all contents -5%
				if (data==46) m_Expression->ChangeFontSize((float)1.05);  //all contents +5%
				if (data==47) m_Expression->ChangeFontSize((float)0.85);  //all contents -15%
				if (data==48) m_Expression->ChangeFontSize((float)1.15);  //all contents +15%
				if (data==54) m_Expression->ChangeFontSize((float)0.5);  //all contents -50%
				if (data==55) m_Expression->ChangeFontSize((float)1.5);  //all contents +50%

			}
			if ((data>=150) && (data<160)) //other data options
			{
				if (data==150) //horizontal
				{
					if (tmpExpression->m_ParenthesesFlags&0x04) 
						tmpExpression->m_ParenthesesFlags&=0xFB;
					else
						tmpExpression->m_ParenthesesFlags|=0x04;
				}
				if (data==151) //no left/top pare
				{
					if (tmpExpression->m_ParenthesesFlags&0x08) 
						tmpExpression->m_ParenthesesFlags&=0xF7;
					else
						tmpExpression->m_ParenthesesFlags|=0x08;
				}
				if (data==152) //no right/bottom perenthese
				{
					if (tmpExpression->m_ParenthesesFlags&0x10) 
						tmpExpression->m_ParenthesesFlags&=0xEF;
					else
						tmpExpression->m_ParenthesesFlags|=0x10;
				}
			}

		}

		if ((((Options[m_SelectedOption].Data>=10) && (Options[m_SelectedOption].Data<29)) || 
			((Options[m_SelectedOption].Data>=67) && (Options[m_SelectedOption].Data<=69))) && 
			(m_theSelectedElement))
		{
			//font menu handling

			int data=Options[m_SelectedOption].Data;
			if (data==19) //'have index' for variable
			{
				if (m_theSelectedElement->pElementObject->Expression1)
				{
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index remove",20213);
					delete ((CExpression*)(m_theSelectedElement->pElementObject->Expression1));
					m_theSelectedElement->pElementObject->Expression1=NULL;
				}
				else
				{
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index add",20214);
					m_theSelectedElement->pElementObject->Expression1=(CObject*)(new CExpression(m_theSelectedElement->pElementObject,m_Expression,m_theSelectedElement->pElementObject->FontSizeForType(1)));
					//((CExpression*)(m_theSelectedElement->pElementObject->Expression1))->m_FontSizeHQ=m_theSelectedElement->pElementObject->FontSizeForTypeHQ(1);
				}
			}
			else if (data==20) //convert to function
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("convert to function",20215);
				m_theSelectedElement->Type=6;
				CElementInitPaternalExpression=(CObject*)m_theSelectedElement->pElementObject->m_pPaternalExpression;
				CElementInitType=6;
				CElement *newElement=new CElement(); 
				newElement->Empty(1);
				if (m_theSelectedElement->pElementObject->Expression1)
				{
					newElement->Expression2=(CObject*)new CExpression(newElement,(CExpression*)(newElement->m_pPaternalExpression),((CExpression*)(m_theSelectedElement->pElementObject->Expression1))->m_FontSize);
					//((CExpression*)(newElement->Expression2))->m_FontSizeHQ=((CExpression*)(m_theSelectedElement->pElementObject->Expression1))->m_FontSizeHQ;
					((CExpression*)(newElement->Expression2))->CopyExpression(((CExpression*)(m_theSelectedElement->pElementObject->Expression1)),0);
				}
				newElement->m_Type=6;
				newElement->m_Color=m_theSelectedElement->pElementObject->m_Color;
				int ii;
				for (ii=0;ii<24;ii++)
				{
					newElement->Data1[ii]=m_theSelectedElement->pElementObject->Data1[ii];
				}
				newElement->Data2[0]=m_theSelectedElement->pElementObject->Data2[0];
				delete m_theSelectedElement->pElementObject;
				m_theSelectedElement->pElementObject=newElement;

			}
			else if (data==21) //'have index' for function or parentheses
			{
				if ((m_MenuType==1) || (m_MenuType==4))
				{
					//for parentheses
					CExpression *tmpExpression=m_Expression;
					if (m_MenuType==4)	tmpExpression=((CExpression*)(m_theSelectedElement->pElementObject->Expression1));
					if (tmpExpression->m_pPaternalElement)
					{
						if (tmpExpression->m_pPaternalElement->Expression2)
						{
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index remove",20213);
							delete ((CExpression*)(tmpExpression->m_pPaternalElement->Expression2));
							tmpExpression->m_pPaternalElement->Expression2=NULL;
						}
						else
						{
							((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index add",20214);
							tmpExpression->m_pPaternalElement->Expression2=(CObject*)(new CExpression(tmpExpression->m_pPaternalElement,tmpExpression->m_pPaternalExpression,((CElement*)(tmpExpression->m_pPaternalElement))->FontSizeForType(2)));
							//((CExpression*)(tmpExpression->m_pPaternalElement->Expression2))->m_FontSizeHQ=((CElement*)(tmpExpression->m_pPaternalElement))->FontSizeForTypeHQ(2);

						}
					}

				}
				else
				{
					//for function

					if (m_theSelectedElement->pElementObject->Expression2)
					{
						((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index remove",20213);
						delete ((CExpression*)(m_theSelectedElement->pElementObject->Expression2));
						m_theSelectedElement->pElementObject->Expression2=NULL;
					}
					else
					{
						((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index add",20214);
						m_theSelectedElement->pElementObject->Expression2=(CObject*)(new CExpression(m_theSelectedElement->pElementObject,m_Expression,m_theSelectedElement->pElementObject->FontSizeForType(2)));
						//((CExpression*)(m_theSelectedElement->pElementObject->Expression2))->m_FontSizeHQ=m_theSelectedElement->pElementObject->FontSizeForTypeHQ(2);

					}
				}
			}
			else if (data==22) //convert to variable
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("convert to variable",20216);
				m_theSelectedElement->Type=1;
				CElementInitPaternalExpression=(CObject*)m_theSelectedElement->pElementObject->m_pPaternalExpression;
				CElementInitType=1;
				CElement *newElement=new CElement(); 
				if (m_theSelectedElement->pElementObject->Expression2)
				{
					newElement->Expression1=(CObject*)new CExpression(newElement,((CExpression*)newElement->m_pPaternalExpression),((CExpression*)(m_theSelectedElement->pElementObject->Expression2))->m_FontSize);
					//((CExpression*)(newElement->Expression1))->m_FontSizeHQ=((CExpression*)(m_theSelectedElement->pElementObject->Expression2))->m_FontSizeHQ;
					((CExpression*)(newElement->Expression1))->CopyExpression(((CExpression*)(m_theSelectedElement->pElementObject->Expression2)),0);
				}
				newElement->m_Type=1;
				newElement->m_Color=m_theSelectedElement->pElementObject->m_Color;
				int ii;
				for (ii=0;ii<24;ii++)
				{
					newElement->Data1[ii]=m_theSelectedElement->pElementObject->Data1[ii];
					newElement->Data2[ii]=m_theSelectedElement->pElementObject->Data2[0];
				}
				delete m_theSelectedElement->pElementObject;
				m_theSelectedElement->pElementObject=newElement;

			}
			else if ((data==23) || (data==27)) //convert to unit, convert to variable (from unit)
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave((data==23)?"convert to unit":"convert to variable",(data==23)?20217:20216);
				if ((m_theSelectedElement->pElementObject->m_VMods)!=0x10)
				{
					m_theSelectedElement->pElementObject->m_VMods=0x10;
				}
				else
				{
					m_theSelectedElement->pElementObject->m_VMods=0;
				}

			}
			else if (data==26) //uns as singleshot
			{
				if (FontAdditionalData&0x01) FontAdditionalData&=0xFE; else FontAdditionalData|=0x01;
			}
			else
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("font format",20218);
				char font_first=0;
				if ((m_theSelectedElement) && (m_theSelectedElement->pElementObject)) font_first=m_theSelectedElement->pElementObject->Data2[0];

				for (int ii=0;ii<m_Expression->m_NumElements;ii++)
				{
					tElementStruct *ts=m_Expression->m_pElementList+ii;
					if (((ts->Type==1) || (ts->Type==6)) && (ts->IsSelected))
					{
						int i;
						for (i=0;i<24;i++)  //for every character
						{
							char font=ts->pElementObject->Data2[i];
							char vmods=ts->pElementObject->m_VMods;
							if (data==10) //sans-serif font choosen
								font=0x00|(font&0x1F);
							if (data==11) //serif font choosen
								font=0x20|(font&0x1F);
							if (data==12) //proportinal font choosen
								font=0x40|(font&0x1F);
							if (data==13) //greek/symbol font choosen
								font=0x60|(font&0x1F);
							if (data==14) //italic
								font=(font&0xFD)|((font_first&0x02)?0:2);
							if (data==15) //bold
								font=(font&0xFE)|((font_first&0x01)?0:1);
							if (data==16) //dash
							{
								if (vmods==0x04) vmods=0; else vmods=0x04;
							}
							if (data==17) //arrow
							{
								if (vmods==0x08) vmods=0; else vmods=0x08;
							}
							if (data==18) //hat
							{
								if (vmods==0x0C) vmods=0; else vmods=0x0C;
							}
							if (data==24) //dot
							{
								if (vmods==0x14) vmods=0; else vmods=0x14;
							}
							if (data==67) //hacek
							{
								if (vmods==0x1C) vmods=0; else vmods=0x1C;
							}
							if (data==69) //triple dot
							{
								if (vmods==0x24) vmods=0; else vmods=0x24;
							}
							if (data==68) //tilde
							{
								if (vmods==0x20) vmods=0; else vmods=0x20;
							}
							if (data==25) //double dot
							{
								if (vmods==0x18) vmods=0; else vmods=0x18;
							}
							ts->pElementObject->Data2[i]=font;
							if (i==0) ts->pElementObject->m_VMods=vmods;
							if (ts->Type==6) break;
						}
					}
				}
			}

			//update popup menu
			/*int ii;
			for (ii=0;ii<m_NumOptions;ii++)
				if (Options[ii].Data==-1) //first font menu option
				{
					int y=Options[ii].Y-TSize/10;
					int tmp=m_NumOptions;
					m_NumOptions=ii;
					PrepareFontMenu(y);
					m_NumOptions=tmp;
					InvalidateRect(NULL,0);
					UpdateWindow();
					Sleep(200);
					break;
				}*/
		}

		int dd=Options[m_SelectedOption].Data;
		if ((isShiftDown) &&
			(dd!=0) && (dd!=1) && (dd!=2) && (dd!=8) && (dd!=9) && (dd!=29) && //exit,pick up,delete,lock,copy,paste
			(dd!=60) && (dd!=20) && (dd!=23) && (dd!=27) && (dd!=22) && //implanting, to function, to unit, to variable
			(dd!=567) && (dd!=568) && (dd!=569) && //lock, loack all, unlock
			(dd!=505) && (dd!=506) && (dd!=513) && (dd!=514) && //group,ungorup,combine,break apart
			(dd!=515) && (dd!=573) && // '??' options
			/*(dd!=550) &&*/ (dd!=551)) //node edit, add node
		{	
			//we are not closing the context menu
			int tmp=Options[m_SelectedOption].IsChecked&0x01;
			if ((dd>=580) && (dd<=585)) UncheckOptions(580,585);//colors
			if ((dd>=80) && (dd<=85)) UncheckOptions(80,85); //colors
			if ((dd>=72) && (dd<=74)) UncheckOptions(72,74); //alignment (condition list)
			if ((dd>=50) && (dd<=52)) UncheckOptions(50,52); //small,medium,large (symbol)
			if (((dd>=30) && (dd<=39)) || (dd==49) || (dd==56) || (dd==57))
			{
				UncheckOptions(30,39);
				UncheckOptions(49,49);
				UncheckOptions(56,57);
			}
			if ((dd>=40) && (dd<=42)) UncheckOptions(40,42); //small,medium,large (parenthese)
			if ((dd>=90) && (dd<=92)) UncheckOptions(90,92); //alignment (expression)
			if ((dd>=10) && (dd<=13)) UncheckOptions(10,13); //font optins
			if ((dd>=16) && (dd<=18)) {UncheckOptions(16,18);UncheckOptions(24,25);} //dash,arrow,hat
			if ((dd>=24) && (dd<=25)) {UncheckOptions(16,18);UncheckOptions(24,25);} //dot, double dot

			if (Options[m_SelectedOption].IsChecked&0x02)
				if (tmp)
					Options[m_SelectedOption].IsChecked=2;
				else
					Options[m_SelectedOption].IsChecked=3;
			tmp=m_SelectedOption;
			pMainView->RepaintTheView();
			this->SetActiveWindow();
			CWnd::OnLButtonDown(nFlags, point);
			Sleep(100);
			m_SelectedOption=tmp;
			this->PaintThePopupMenu();
			PopupMenuSecondPassChoosing=1;
			return;
		}

		ReleaseCapture();
		ShowWindow(SW_HIDE);
		if (m_OwnerType==1) //the main view
		{
			((CMathomirView*)pMainView)->PopupCloses(m_UserParam,Options[m_SelectedOption].Data);
		}
		if (m_OwnerType==0) //the toolbox
		{
			((CToolbox*)m_Owner)->PopupCloses(m_UserParam,(PopupMenuSecondPassChoosing)?1:Options[m_SelectedOption].Data);
		}
		if (m_OwnerType==3) //keyboard entry (double '?')
		{
			if (KeyboardEntryObject) //keyboard entry mode
				((CExpression*)KeyboardEntryObject)->KeyboardPopupClosed(m_UserParam,Options[m_SelectedOption].Data);
			else if (m_SelectedSuboption==0)
			{
				delete_clipboard_at_exit=0;
				if (ClipboardExpression) delete ClipboardExpression;
				ClipboardExpression=new CExpression(NULL,NULL,100);
				ClipboardExpression->CopyExpression(m_Expression,0);
			}
		}
		//delete all objects
		int ii;
		for (ii=0;ii<m_NumOptions;ii++)
			if ((Options[ii].Graphics) && (m_OwnerType!=10))
			{
				if (Options[ii].Graphics!=ClipboardExpression)
				{
					delete Options[ii].Graphics;
				}			
			}

		if ((Options[m_SelectedOption].Data==61) || (Options[m_SelectedOption].Data==62) || (Options[m_SelectedOption].Data==29))
		{
			//delete the clipboard after the substitution event
			if (delete_clipboard_at_exit)
			if (ClipboardExpression)
			{
				delete ClipboardExpression;
				ClipboardExpression=NULL;
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

int PopupMenu::UncheckOptions(int from, int to)
{
	for (int i=0;i<this->m_NumOptions;i++)
		if ((Options[i].Data>=from) && (Options[i].Data<=to) &&
			(Options[i].IsChecked&0x02))
			Options[i].IsChecked=2;
	return 1;
}

#pragma optimize("s",on)
int PopupMenu::HidePopupMenu(void)
{
	//exiting popup menu
	if ((IsWindowVisible()) && (m_Owner!=NULL))
	{
		if (m_OwnerType==1) //the main view
		{
			((CMathomirView*)pMainView)->PopupCloses(m_UserParam,0);
		}
		if (m_OwnerType==0) //the toolbox
		{
			((CToolbox*)m_Owner)->PopupCloses(m_UserParam,0); //EXIT CODE=0;
		}
		if ((m_OwnerType==3) && (KeyboardEntryObject)) //keyboard entry mode
		{
			((CExpression*)KeyboardEntryObject)->KeyboardPopupClosed(m_UserParam,0);
		}
		int ii;
		for (ii=0;ii<m_NumOptions;ii++)
			if ((Options[ii].Graphics) && (m_OwnerType!=10))
			{
				if (Options[ii].Graphics!=ClipboardExpression)
				{
					delete Options[ii].Graphics;
				}
			}
	}
	if (this->IsWindowVisible())
	{
		ReleaseCapture();
		ShowWindow(SW_HIDE);
	}
	return 0;
}



void PopupMenu::OnRButtonDown(UINT nFlags, CPoint point)
{
	HidePopupMenu();

	CWnd::OnRButtonDown(nFlags, point);
}

void PopupMenu::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar==27)
		HidePopupMenu();
	if ((nChar==13) && (m_SelectedOption>=0))
	{
		//the enter key
		PopupMenu::OnLButtonDown(0,CPoint(-1,-1));
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

#pragma optimize("s",on)
void PopupMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	::SetWindowLong(this->m_hWnd,GWL_EXSTYLE,GetWindowLong(this->m_hWnd,GWL_EXSTYLE)&(~WS_EX_LAYERED));
	//::SetLayeredWindowAttributes(ListPopup->m_hWnd,0,140,LWA_ALPHA);

	int X=0;
	int Y=-1;
	int CX=10;
	int CY=0;
	if (m_SelectedOption>=0) 
	{
		X=Options[m_SelectedOption].X;
		Y=Options[m_SelectedOption].Y;
		CX=Options[m_SelectedOption].Cx;
	}
	int found=-1;
	int dist=0x7FFF;
	if ((nChar==VK_DOWN) || (nChar==VK_UP))
	{
		for (int i=0;i<m_NumOptions;i++)
		{
			int xdist=0;
			if (Options[i].X>X+CX) xdist=Options[i].X-X-CX;
			if (Options[i].X+Options[i].Cx<X) xdist=X-(Options[i].X+Options[i].Cx);
			if ((i!=m_SelectedOption) && (Options[i].IsEnabled) && (!Options[i].IsGraphicsSensitive))
			{				
				if ((nChar==VK_DOWN) && (Options[i].Y>Y) && (xdist<Options[i].Y-Y))
				{
					int d=(Options[i].Y-Y)+xdist;
					if (d<dist) {dist=d;found=i;}
				}
				if ((nChar==VK_UP) && (Options[i].Y<Y) && (xdist<Y-Options[i].Y))
				{
					int d=(Y-Options[i].Y)+xdist;
					if (d<dist) {dist=d;found=i;}
				}
			}
		}
	}
	if (nChar==VK_RIGHT)
	{
		if ((Options[m_SelectedOption].Graphics) && (Options[m_SelectedOption].IsEnabled) && (!Options[m_SelectedOption].IsGraphicsSensitive))
		{
			if (m_SelectedSuboption==1) {m_SelectedSuboption=2;found=m_SelectedOption;}
			else if (m_SelectedSuboption==2) {m_SelectedSuboption=0;found=m_SelectedOption;}
		}
		if (found<0)
		for (int i=0;i<m_NumOptions;i++)
			if ((i!=m_SelectedOption) && (Options[i].IsEnabled) && (!Options[i].IsGraphicsSensitive))
				if ((Options[i].X>X) && (abs(Options[i].Y-Y)<Options[i].X-X))
				{
					int d=(Options[i].X-X)+abs(Options[i].Y-Y);
					if (d<dist) {dist=d;found=i;m_SelectedSuboption=0;}
				}
	}
	if (nChar==VK_LEFT)
	{
		if ((Options[m_SelectedOption].Graphics) && (Options[m_SelectedOption].IsEnabled) && (!Options[m_SelectedOption].IsGraphicsSensitive))
		{
			if (m_SelectedSuboption==0) {m_SelectedSuboption=2;found=m_SelectedOption;}
			else if (m_SelectedSuboption==2) {m_SelectedSuboption=1;found=m_SelectedOption;}
		}
		if (found<0)
		for (int i=0;i<m_NumOptions;i++)
			if ((i!=m_SelectedOption) && (Options[i].IsEnabled) && (!Options[i].IsGraphicsSensitive))
				if ((Options[i].X<X) && (abs(Options[i].Y-Y)<X-Options[i].X))
				{
					int d=(X-Options[i].X)+abs(Options[i].Y-Y);
					if (d<dist) {dist=d;found=i;m_SelectedSuboption=0;}
				}
	}

	if (found>=0)
	{
		m_SelectedOption=found;
		if ((Options[found].Graphics==NULL) || (Options[found].IsGraphicsSensitive)) m_SelectedSuboption=0;
		for (int i=0;i<m_NumOptions;i++)
			if (Options[i].Graphics) 
				((CExpression*)Options[i].Graphics)->SelectExpression(((i==found) && (m_SelectedSuboption==0))?1:0);
		PaintThePopupMenu();
	}
	CWnd::OnKeyDown(nChar,nRepCnt,nFlags);
}


#pragma optimize("s",on)
BOOL PopupMenu::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (ValueEntryBox)
	{
		ValueEntryBox->GetWindowText(ValueEntryBoxString,299);
		for (int i=0;i<(int)strlen(ValueEntryBoxString); i++)
			if (ValueEntryBoxString[i]==0x0D)
			{
				m_SelectedOption=ValueEntryBoxData;
				if (entry_box_first_call==1) 
				{
					entry_box_first_call=0;PopupMenu::OnLButtonDown(0x1234,CPoint(-1,-1));
				}
				return 1;
			}
		return CWnd::OnCommand(wParam, lParam);
	}
	return 1;
}

#pragma optimize("",on)
//extracts the selected part of the expression into the ExtractedSelection
//but it expands limits of selections to factors/summands
//it returns Operator Level
int PopupMenu::ExtractSelection(int StartPos,int EndPos,int * StartSel, int * EndSel)
{
	if (ExtractedSelection==NULL) return -1;
	if (*EndSel<*StartSel) return -1;

	if (*EndSel==*StartSel)
	{
		ExtractedSelection->InsertElement(m_Expression->m_pElementList+*StartSel,0);
		return MulLevel;
	}

	int Level=m_Expression->FindLowestOperatorLevel(StartPos,EndPos,'+');
	int StartFound=0;
	int EndFound=0;
	int pos=StartPos;
	while (1)
	{
		char et,p;
		int l=m_Expression->GetElementLen(pos,EndPos,Level,&et,&p);
		if (l==0) return -1;

		if ((*StartSel>=pos) && (*EndSel<=pos+l-1))
		{
			if (((p) && (*StartSel==pos)) || /* ((*StartSel==pos+p) && (*EndSel==pos+l-1))*/
				((Level==MulLevel) && (*StartSel==StartPos) && ((m_Expression->m_pElementList+pos)->Type==2) && (GetOperatorLevel((m_Expression->m_pElementList+pos)->pElementObject->Data1[0])==PlusLevel)))
			{
				*EndSel=pos+l-1;
				StartFound++;
				EndFound++;
				break;
			}
			return ExtractSelection(pos+p,pos+l-1,StartSel,EndSel);
		}

		if ((*StartSel<=pos+l-1) && (*StartSel>=pos))
		{
			*StartSel=pos;
			StartFound++;
		}
		if ((*EndSel<=pos+l-1) && (*EndSel>=pos))
		{
			*EndSel=pos+l-1;
			EndFound++;
		}

		pos+=l;
		if (pos>EndPos) break;
	}


	if ((StartFound==1) && (EndFound==1))
	{
		int found_nl=0;
		for (int i=*StartSel;i<=*EndSel;i++)
		{
			if ((m_Expression->m_pElementList+i)->Type==12) found_nl=1;
			ExtractedSelection->InsertElement(m_Expression->m_pElementList+i,ExtractedSelection->m_NumElements);
		}
		if (found_nl)
		{
			ExtractedSelection->m_Alignment=m_Expression->m_Alignment;
		}


		return Level;
	}

	return -1;
}


int PopupMenu_AddMathHeader=0;
int PopupMenu::SymbolicComputation(void)
{ 
	int is_suitable=0;
	if (m_OwnerType==0) return 0;
	
	//The following section is abut solving systems of equations
	//It generates a list of expression and gives it to SolveSystemOfEquations method
	if ((ExtractedSelection==NULL) || (ExtractedSelection->m_pElementList->Type==0))
	{
		//create an array of expressions
		CExpression *System[24];
		int NumEquations=0;

		for (int i=0;i<NumDocumentElements;i++)
		{
			if ((TheDocument[i].MovingDotState==3) && (TheDocument[i].Type==1) && (TheDocument[i].Object))
			{
				//check if this expression is valid for inclusion into system
				CExpression *tmp=(CExpression*)TheDocument[i].Object;
				if (tmp->IsTextContained(-1)) continue;
				if ((tmp->m_NumElements==1) && 
					((tmp->m_pElementList->Type==0) || (tmp->m_pElementList->Type==1) || (tmp->m_pElementList->Type==2) || 
					(tmp->m_pElementList->Type==11) || (tmp->m_pElementList->Type==12))) continue;
				if (!tmp->IsSuitableForComputation()) continue;
				int lvl=tmp->FindLowestOperatorLevel((char)0xD7);
				if (lvl>EqLevel) 
				{
					//for expression of higher-than equation level, include directly
					System[NumEquations]=new CExpression(NULL,NULL,100);
					System[NumEquations]->CopyExpression((CExpression*)TheDocument[i].Object,0);
					NumEquations++;
					if (NumEquations>16) break; //current limit is 16
					continue;
				}
				else
				{
					//for other expression, pare to equation level
					//first parse the expression in that areform: 'Equation1;Equation2;Equation3'
					int pos=0;
					while (1)
					{
						char et,p;
						int l=tmp->GetElementLen(pos,tmp->m_NumElements-1,min(lvl,EqLevel-1),&et,&p);
						if (l==0) break;
						int lvl2=tmp->FindLowestOperatorLevel(pos+p,pos+l-1,(char)0xD7);
						if (lvl2==EqLevel)
						{
							//second, pare an complex equation: 'exrpession1=expression2=expression3'
							//into simpler equations: 'expression1=expression2' and 'expression2=expression3'
							int pos2=pos+p;
							int ppos=pos+p;
							while (1)
							{
								char et2,p2;
								int l2=tmp->GetElementLen(pos2,pos+l-1,EqLevel,&et2,&p2);
								if (l2==0) break;
								if (pos2>pos+p)
								{
									System[NumEquations]=new CExpression(NULL,NULL,100);
									for (int kk=ppos;kk<pos2;kk++)
										System[NumEquations]->InsertElement(tmp->m_pElementList+kk,System[NumEquations]->m_NumElements);
									for (int kk=pos2;kk<pos2+l2;kk++)
										System[NumEquations]->InsertElement(tmp->m_pElementList+kk,System[NumEquations]->m_NumElements);
									NumEquations++;
									if (NumEquations>16) {pos2=tmp->m_NumElements; break;}
								}
								ppos=pos2+p2;
								pos2+=l2;
								if (pos2>pos+l-1) break;
							}
						}
						pos+=l;
						if (pos>tmp->m_NumElements-1) break;
					}
				}
			}
		}

		if (NumEquations<=1) 
		{
			if (NumEquations==1) delete System[0];
			return 0; //nothing to work with
		}

		PopupMenu_AddMathHeader=6;
		if (NumEquations<=16) //we don't even try for large systems (too dangerous)
		{
			CExpression *solution=new CExpression(NULL,NULL,100); //create an dummy object
			solution->SolveSystemOfEquations(System,&NumEquations,this);
			delete solution;
		}
		
		for (int i=0;i<NumEquations;i++) delete System[i];
		return 1;
	}


	//substitution of variables - when there is clibpoard expression pressent
	//the variable is extracted from clipbard equation and substituted to the other equation
	if (ClipboardExpression)
	{
		PopupMenu_AddMathHeader=6;

		if (ExtractedSelection->IsSuitableForComputation()==0) return 0;
		if (ClipboardExpression->IsSuitableForComputation()==0) return 0;
		CExpression *parent=m_Expression;
		while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;
		if (parent->IsSuitableForComputation()==0) return 0;

		CExpression *Base=new CExpression(NULL,NULL,100);
		Base->CopyExpression(parent,0);
		CExpression *tmp_clipboard=new CExpression(NULL,NULL,100);
		tmp_clipboard->CopyExpression(ClipboardExpression,0);

		if (tmp_clipboard->m_pElementList->Type==2)
		{
			//just ignore the equal sign at the very beginning
			char ch=tmp_clipboard->m_pElementList->pElementObject->Data1[0];
			if ((ch=='=') || (ch=='<') || (ch=='>') || (ch==(char)0xB9) || (ch==1) || (ch==2) || (ch==(char)0xA3) || (ch==(char)0xB3))
				tmp_clipboard->DeleteElement(0);
		}

		Base->CodeDecodeUnitsOfMeasurement(0,-1);
		tmp_clipboard->CodeDecodeUnitsOfMeasurement(0,-1);
		ExtractedSelection->CodeDecodeUnitsOfMeasurement(0,-1);

		if (tmp_clipboard->FindLowestOperatorLevel((char)0xD7)!=EqLevel)
		{
			//the clipboard is not an equation, but an general expression
			//we will replace the Base elements in a 'stupid' manner			
			try
			{
				if (Base->FindReplace(0,Base->m_NumElements-1,ExtractedSelection,tmp_clipboard))
				{
					if (AddMathMenuOption(Base,parent)) {delete tmp_clipboard;return 1;}
				}
			}
			catch (...)
			{
			}
		}
		else 
		{
			//the clipboard contains an equation, we make a 'smart' substitution
			if (Base->MakeSubstitution(tmp_clipboard,ExtractedSelection))
			{
				if (AddMathMenuOption(Base,parent)) {delete tmp_clipboard;return 1;}
			}
		}
		delete Base;
		delete tmp_clipboard;
		return 0;
	}

	//we work on extracted selection
	ExtractedSelection->CodeDecodeUnitsOfMeasurement(0,ExtractedSelection->m_NumElements-1);
	is_suitable=ExtractedSelection->IsSuitableForComputation(); // is_suitable==2 if only pure numbers are contained in expression
	if (!is_suitable) 
	{
		//try with auto-correction
		CExpression *tmp=new CExpression(NULL,NULL,100);
		tmp->CopyExpression(ExtractedSelection,0);
		tmp->m_ParentheseShape=ExtractedSelection->m_ParentheseShape;
		tmp->m_ParenthesesFlags=ExtractedSelection->m_ParenthesesFlags;
		if (tmp->IsSuitableForComputation(1))
		{
			PopupMenu_AddMathHeader=5;
			if (!AddMathMenuOption(tmp)) {delete tmp;return 0;}
			return 1;
		}
		else
		{
			delete tmp;
			return 0;
		}
	}

	if (ExtractedSelection->m_NumElements==1)
	{
		// handling for pure numbers - rounding and format
		double N;
		int prec;
		int is_pi=0;
		int is_e=0;
		int is_pure=0;
		if (ExtractedSelection->IsPureNumber(0,ExtractedSelection->m_NumElements,&N,&prec))
			is_pure=1;
		if ((ExtractedSelection->m_NumElements==1) && (ExtractedSelection->m_pElementList->Type==1) &&
			(strcmp(ExtractedSelection->m_pElementList->pElementObject->Data1,"p")==0) &&
			((ExtractedSelection->m_pElementList->pElementObject->Data2[0]&0xE0)==0x60))
			{is_pi=1;is_pure=1;}
		if ((ExtractedSelection->m_NumElements==1) && (ExtractedSelection->m_pElementList->Type==1) &&
			(strcmp(ExtractedSelection->m_pElementList->pElementObject->Data1,"e")==0) &&
			((ExtractedSelection->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60) &&
			((ExtractedSelection->m_pElementList->pElementObject->m_VMods)==0))
			{is_e=1;is_pure=1;}
		if ((ExtractedSelection->m_NumElements==1) && (is_pure))
		{
			if (is_pi) {N=asin(1.0)*2;prec=4;}
			if (is_e) {N=exp(1.0);prec=4;}
			PopupMenu_AddMathHeader=2;
			int rounding_done=0;
			int prec_increase=2;
			while (prec+prec_increase<14)
			{
				CExpression *E1=new CExpression(NULL,NULL,100);
				E1->GenerateASCIINumber(N,(long long)(N+(N>0)?0.01:-0.01),(fabs(N-(long long)N)<1e-100)?1:0,prec+prec_increase,0);
				int last=E1->m_NumElements-1;
				tElementStruct *ts=E1->m_pElementList+last;
				int ln=(int)strlen(ts->pElementObject->Data1);
				while (ln)
				{
					if (ts->pElementObject->Data1[ln-1]=='0') {ts->pElementObject->Data1[--ln]=0;continue;}
					if (ts->pElementObject->Data1[ln-1]=='.') ts->pElementObject->Data1[--ln]=0;
					break;
				}
				if (!AddMathMenuOption(E1)) delete E1; else {rounding_done=1;break;}
				if ((ExtractedSelection->m_pElementList->pElementObject->Data1[14]!=0) || (ExtractedSelection->m_pElementList->pElementObject->Data1[15]!=(char)(128+126))) break;
				prec_increase++;
				if (prec+prec_increase>10) break;
			}
			if ((prec>0) && (fabs(N)>pow((double)10,(double)(-prec+1))))
			{
				CExpression *E1=new CExpression(NULL,NULL,100);
				E1->GenerateASCIINumber(N,(long long)(N+(N>0)?0.01:-0.01),(fabs(N-(long long)N)<1e-100)?1:0,prec-1,0);
				if (!AddMathMenuOption(E1)) delete E1; else rounding_done=1;
			}
			if ((prec>1) && (fabs(N)>pow((double)10,(double)(-prec+2))))
			{
				CExpression *E1=new CExpression(NULL,NULL,100);
				E1->GenerateASCIINumber(N,(long long)(N+(N>0)?0.01:-0.01),(fabs(N-(long long)N)<1e-100)?1:0,prec-2,0);
				if (!AddMathMenuOption(E1)) delete E1; else rounding_done=1;
			}
			if (((fabs(N)>=10000.0) || (fabs(N)<0.001)) && (this->LevelExtendedSelection<=MulLevel))
			{
				//expressing numbers in scientific form
				CExpression *E1=new CExpression(NULL,NULL,100);
				int exp=(int)log10(fabs(N));
				if (fabs(N)<1.0) exp--;
				double re=N/pow((double)10,(double)exp);
				E1->GenerateASCIINumber(re,(long long)(re+(re>0)?0.01:-0.01),(fabs(re-(long long)re)<1e-100)?1:0,min(prec+abs(exp),8),0);
				tElementStruct *ts=E1->m_pElementList;
				int ln=(int)strlen(ts->pElementObject->Data1);
				while (ln)
				{
					if (ts->pElementObject->Data1[ln-1]=='0') {ts->pElementObject->Data1[--ln]=0;continue;}
					if (ts->pElementObject->Data1[ln-1]=='.') ts->pElementObject->Data1[--ln]=0;
					break;
				}
				E1->InsertEmptyElement(E1->m_NumElements,2,(char)0xD7);
				E1->InsertEmptyElement(E1->m_NumElements,3,0);
				CExpression *a=(CExpression*)((E1->m_pElementList+E1->m_NumElements-1)->pElementObject->Expression1);
				CExpression *e=(CExpression*)((E1->m_pElementList+E1->m_NumElements-1)->pElementObject->Expression2);
				a->GenerateASCIINumber(10.0,10,1,0,0);
				if (exp<0)
				{
					e->InsertEmptyElement(0,2,'-');
					e->GenerateASCIINumber((double)-exp,-exp,1,0,1);
				}
				else
					e->GenerateASCIINumber((double)exp,exp,1,0,0);
				if ((E1->m_pElementList->Type==2) && (E1->m_pElementList->pElementObject->Data1[0]=='-') &&
					((E1->m_pElementList+1)->Type==1) && (strcmp((E1->m_pElementList+1)->pElementObject->Data1,"1")==0))
				{
					//deletes first part in numbers: -1*10^x
					E1->DeleteElement(1);
					E1->DeleteElement(1);
				}
				if ((E1->m_pElementList->Type==1) && (strcmp(E1->m_pElementList->pElementObject->Data1,"1")==0))
				{
					//deletes first part in numbers: 1*10^x
					E1->DeleteElement(0);
					E1->DeleteElement(0);
				}
				if (!AddMathMenuOption(E1)) delete E1; else rounding_done=1;
			}

			//making factorization of integer numbers
			if (ExtractedSelection->IsPureNumber(0,ExtractedSelection->m_NumElements,&N,&prec))
			{
				if ((fabs(N-(long long)N)<1e-100) && (fabs(N)>2.9))
				{
					int minus=0;
					if (N<0) {minus=1;N=-N;}
					long long n=(long long)(N+0.01);
					CExpression *result=new CExpression(NULL,NULL,100);
					
					long long z=2;
					do
					{
						int cnt=0;
						while ((n%z)==0) {n=n/z;cnt++;}
						if (cnt==1)
						{
							if (result->m_pElementList->Type) 
								result->InsertEmptyElement(result->m_NumElements,2,(char)0xD7);
							int pos=result->m_NumElements;
							if (result->m_pElementList->Type==0) pos=0;
							result->GenerateASCIINumber((double)z,z,1,0,pos);
						}
						else if (cnt>1)
						{
							if (result->m_pElementList->Type) 
								result->InsertEmptyElement(result->m_NumElements,2,(char)0xD7);
							result->InsertEmptyElement(result->m_NumElements,3,0);
							CExpression *a=((CExpression*)(result->m_pElementList+result->m_NumElements-1)->pElementObject->Expression1);
							CExpression *e=((CExpression*)(result->m_pElementList+result->m_NumElements-1)->pElementObject->Expression2);
							a->GenerateASCIINumber((double)z,z,1,0,0);
							e->GenerateASCIINumber((double)cnt,(long long)cnt,1,0,0);
						}
						if (z==2) z++; else z+=2;
					}while (z<3000);

					if ((n>1) && (result->m_pElementList->Type))
					{
						result->InsertEmptyElement(result->m_NumElements,2,(char)0xD7);
						result->GenerateASCIINumber((double)n,n,1,0,result->m_NumElements);
					}
					if ((minus) && (result->m_pElementList->Type))
						result->InsertEmptyElement(0,2,'-');

					PopupMenu_AddMathHeader=1;					
					if ((result->m_pElementList->Type==0) || (!AddMathMenuOption(result))) delete result;
				}
			}
			if (rounding_done) return 1;
		}
		else if ((ExtractedSelection->m_pElementList->Type==1) && (is_pure==0) &&
				 ((ExtractedSelection->m_pElementList->pElementObject->m_VMods)!=0x10)) //not unit (kg, m ,s, V, A, rad...)
		{
			//this is a variable... make variable handling

			PopupMenu_AddMathHeader=3;
 
			CExpression *parent=m_Expression;
			while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;

			CExpression *tmp=new CExpression(NULL,NULL,100);
			tmp->CopyExpression(parent,0);

			int cnt=0;

			try
			{
				int ret=1;
				int prev_ret=1;
				while ((ret=tmp->ExtractVariable(ExtractedSelection,0,ExtractedSelection->m_NumElements,ret)) && (cnt<50))
				{
					if (tmp->ContainsVariable(0,tmp->m_NumElements-1,ExtractedSelection,0,ExtractedSelection->m_NumElements)==0)
					{
						//error - variable cannot be extracted
						delete tmp;
						return 0;
					}
					if ((ret==4) && (prev_ret!=ret))
					{
						CExpression *tmp2=new CExpression(NULL,NULL,100);
						tmp2->CopyExpression(tmp,0);
						AddMathMenuOption(tmp2,parent);
					}
					prev_ret=ret;
					cnt++;
				}
				if (tmp->MakeExpressionBeautiful()) cnt++;
			}
			catch(...)
			{
				delete tmp;
				return 0;
			}

			if (cnt==0) 
				delete tmp;
			else
				if (!AddMathMenuOption(tmp,parent)) 
					delete tmp;

			//polynomization
			PopupMenu_AddMathHeader=4;

			CExpression *tmp2=new CExpression(NULL,NULL,100);
			parent=m_Expression;
			tmp2->CopyExpression(m_Expression,0);

			cnt=0;
			try
			{
				while (1)
				{
					tmp2->CopyExpression(parent,0);
					if (tmp2->Polynomize(ExtractedSelection)>1) 
					{
						cnt=1;
						if (tmp2->MakeExpressionBeautiful()) cnt++;
						break;
					}
					parent=parent->m_pPaternalExpression;
					if (!parent) break;
				}
			}
			catch(...)
			{
				delete tmp2;
				return 0;
			}

			if (cnt==0) {delete tmp2;return 0;}
			if (!AddMathMenuOption(tmp2,parent)) {delete tmp2;return 0;}
			return 1;
		}

	}

	PopupMenu_AddMathHeader=1;


	for (int i=-1;i<4;i++)
	{
		CExpression *E1=new CExpression(NULL,NULL,100);
		E1->CopyExpression(ExtractedSelection,0);E1->m_ParenthesesFlags=ExtractedSelection->m_ParenthesesFlags;
		E1->PROFILERClear();

		int cntr=0;
		try
		{
			if ((i==-1) && (is_suitable==2))
				while ((E1->Compute(0,E1->m_NumElements-1,10)) && (cntr<50))
				{ cntr++;if (!IsWindowVisible()) {delete E1;return 0;}}
		
			if (i==0)
				while ((E1->Compute(0,E1->m_NumElements-1,0)) && (cntr<50))
				{ cntr++;if (!IsWindowVisible()) {delete E1;return 0;}}

			if (i==1)
				while ((E1->Compute(0,E1->m_NumElements-1,1)) && (cntr<50)) 
				{ cntr++;if (!IsWindowVisible()) {delete E1;return 0;}}

			if (i==2)
				while ((E1->Compute(0,E1->m_NumElements-1,2)) && (cntr<50)) 
				{ cntr++;if (!IsWindowVisible()) {delete E1;return 0;}} 

			if (i==3) {E1->FactorizeExpression(1);cntr++;}
			
			if (E1->MakeExpressionBeautiful()) cntr++;
		}
		catch(...)
		{
			delete E1;
			return 0;
		}
		

		E1->PROFILEREnd();
		int added=0;
		if ((cntr>0) /*&& (cntr<50)*/)
		{
			added=AddMathMenuOption(E1);
		}

		if (!added) delete E1;

	}


	//here we are checking if we clicked at some kind of vector of pure numbers (calculating its average and sum)

	if (ExtractedSelection->m_NumElements>=3)
	{
		int level=ExtractedSelection->FindLowestOperatorLevel();
		int is_numbers=1;
		double sum=0;
		int number=0;
		int precision=0;
		if ((level>=0) && (GetOperatorLevel('=')>level))
		{
			int pos=0;
			while (1)
			{
				char et,p;
				int l=ExtractedSelection->GetElementLen(pos,ExtractedSelection->m_NumElements-1,level,&et,&p);
				if (l==0) break;

				tPureFactors PF;
				PF.N1=PF.N2=1.0;
				PF.is_frac1=0;
				PF.prec1=0;
				CExpression *ex1=new CExpression(NULL,NULL,100);
				for (int i=pos+p;i<pos+l;i++)
					ex1->InsertElement(ExtractedSelection->m_pElementList+i,ex1->m_NumElements);
				if (ex1->StrikeoutCommonFactors(0,ex1->m_NumElements-1,1,NULL,0,0,1,&PF))
				{
					number++;
					sum+=PF.N1/PF.N2;
					if (PF.prec1>precision) precision=PF.prec1;
				}
				else
					is_numbers=0;
				delete ex1;


				pos+=l;
				if (pos>ExtractedSelection->m_NumElements-1) break;
			}
			if (is_numbers)
			{
				PopupMenu_AddMathHeader=7;
				CExpression *E1=new CExpression(NULL,NULL,100);
				E1->GenerateASCIINumber(sum,(long long)sum,0,precision,0);
				if (!AddMathMenuOption(E1)) delete E1;

				PopupMenu_AddMathHeader=8;
				E1=new CExpression(NULL,NULL,100);
				sum=sum/(double)number;
				E1->GenerateASCIINumber(sum,(long long)sum,0,precision,0);
				if (!AddMathMenuOption(E1)) delete E1;
			}
		}
	}



	return 0;
}


extern int CalcStructuralChecksumOnly;
#pragma optimize("s",on)
int PopupMenu::AddMathMenuOption(CExpression * E1,CExpression *original)
{
	E1->CodeDecodeUnitsOfMeasurement(0,-1);
	E1->MakeExpressionBeautiful();

	if (E1->m_pElementList->Type==0) return 0;
	
	CalcStructuralChecksumOnly=1;
	int S1=E1->CalcChecksum();
	if (m_Expression)
	{
		if (original==NULL) original=ExtractedSelection;
		int S0=original->CalcChecksum();		
		if (S1==S0) {CalcStructuralChecksumOnly=0;return 0;}
	}
	else
	{
		//in chase of systems of equations (check if any selected is equal)
		for (int i=0;i<NumDocumentElements;i++)
		if ((TheDocument[i].MovingDotState==3) && (TheDocument[i].Type==1))
		{
			CExpression *org=(CExpression*)TheDocument[i].Object;
			int S0=org->CalcChecksum();
			if (S1==S0) {CalcStructuralChecksumOnly=0;return 0;}
		}
	}
	if ((PopupMenu_AddMathHeader!=7) && (PopupMenu_AddMathHeader!=8))
	for (int j=0;j<m_NumOptions;j++)
	{
		if ((Options[j].Graphics) && (Options[j].IsGraphicsSensitive==0))
		{

			int S2=Options[j].Graphics->CalcChecksum();
			if (S1==S2) {CalcStructuralChecksumOnly=0;return 0;}
		}
	}
	CalcStructuralChecksumOnly=0;


	if (PopupMenu_AddMathHeader==1)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize,"Equals to:",-1000,1);
	}

	if (PopupMenu_AddMathHeader==2)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize,"Rounding:",-1001,1);
	}
	if (PopupMenu_AddMathHeader==3)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize,"Extracted:",-1002,1);
	}
	if (PopupMenu_AddMathHeader==4)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize*3/2,"Polynome-like:",-1003,1);
	}
	if (PopupMenu_AddMathHeader==5)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize*3/2,"Corrected:",-1004,1);
	}
	if (PopupMenu_AddMathHeader==6)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize*3/2,"Substituted:",-1005,1);
	}
	if (PopupMenu_AddMathHeader==7)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize*3/2,"Sum of array:",-1006,1);
	}
	if (PopupMenu_AddMathHeader==8)
	{
		PopupOption_Y+=TSize/10; //separator
		AddMenuOption(0,TSize*3/2,"Average:",-1007,1);
	}
	PopupMenu_AddMathHeader=0;

	short l,a,b;
	int ZoomLevel=90*TSize/60;
	CDC *dcc=this->GetDC();

	char tmpvf=E1->m_IsVertical;
	E1->m_IsVertical=0;
	E1->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
	if ((l>5*TSize) || (a+b>5*TSize/9))
	{
		ZoomLevel=82*TSize/60;
		E1->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
		if ((l>5*TSize) || (a+b>5*TSize/9))
		{
			ZoomLevel=74*TSize/60;
			if (ZoomLevel<66) ZoomLevel=66;
			E1->CalculateSize(dcc,ZoomLevel,&l,&a,&b);
		}
	}
	E1->m_IsVertical=tmpvf;

	this->ReleaseDC(dcc);
	if (l>800) l=800;

	Options[m_NumOptions].Y=PopupOption_Y;
	Options[m_NumOptions].X=TSize_1p20;
	Options[m_NumOptions].Cx=l+TSize/4+TSize_1p2;
	Options[m_NumOptions].Cy=a+b+2;
	strcpy(Options[m_NumOptions].Text,"");
	Options[m_NumOptions].IsButton=0;
	Options[m_NumOptions].Graphics=E1;
	Options[m_NumOptions].IsChecked=0;
	Options[m_NumOptions].IsEnabled=1;
	Options[m_NumOptions].IsGraphicsSensitive=0;
	Options[m_NumOptions].Data=(original!=ExtractedSelection)?62:61; //
	Options[m_NumOptions].DataArray[0]=l+TSize_1p2;
	Options[m_NumOptions].DataArray[1]=a;
	Options[m_NumOptions].DataArray[2]=b;
	Options[m_NumOptions].DataArray[3]=ZoomLevel;
	Options[m_NumOptions].DataArray[4]=TSize_1p20;  //X coordinate of expression
	Options[m_NumOptions].DataArray[5]=PopupOption_Y+a;  //Y coordinae of expression
	Options[m_NumOptions].Parent=original;
	PopupOption_Y+=Options[m_NumOptions].Cy;

	m_NumOptions++;	


	m_IsFirstPass=2;
	InvalidateRect(NULL,0);
	UpdateWindow();
	if ((m_OwnerType==3) && (m_UserParam==0) && (m_SelectedOption==-1))
	{
		Sleep(60);
		m_SelectedOption=m_NumOptions-1;
		m_SelectedSuboption=0;
		Options[m_SelectedOption].Graphics->SelectExpression(1);
		PaintThePopupMenu();
	}
	return 1;
}

int PopupMenuWorkIndicatorState;
#pragma optimize("s",on)
int PopupMenu::PaintWorkIndicator(void)
{
	
	CDC *DC=GetDC();

	PopupMenuWorkIndicatorState++;
	if (PopupMenuWorkIndicatorState>2) PopupMenuWorkIndicatorState=0;
	if (CalcThreadID==0) PopupMenuWorkIndicatorState=3;
	RECT r;
	GetClientRect(&r);
	if ((r.left==0) && (r.top==0))
	{
		r.bottom-=TSize_1p25;
		r.left=TSize_1p25;
		r.top=r.bottom-TSize_1p25;
		r.right=r.left+TSize_1p25;

		DC->FillSolidRect(&r,(PopupMenuWorkIndicatorState==0)?0:SHADOW_BLUE_COLOR);
		r.left+=TSize/12;
		r.right+=TSize/12;
		DC->FillSolidRect(&r,(PopupMenuWorkIndicatorState==1)?0:SHADOW_BLUE_COLOR);
		r.left+=TSize/12;
		r.right+=TSize/12;
		DC->FillSolidRect(&r,(PopupMenuWorkIndicatorState==2)?0:SHADOW_BLUE_COLOR);
		DC->SelectObject(GetStockObject(WHITE_BRUSH));
	}

	ReleaseDC(DC);
	return 0;
}
#pragma optimize("",on)