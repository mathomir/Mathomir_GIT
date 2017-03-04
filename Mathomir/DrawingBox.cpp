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
#include "StdAfx.h"
#include ".\drawing.h"
#include "Mathomir.h"
#include "mainfrm.h"
#include "math.h"
#include "MathomirDoc.h"
#include "MathomirView.h"
#include "expression.h"
#include "popupmenu.h"
#include "math.h"
#include "float.h"
#include "toolbox.h"

extern CMathomirView *pMainView;
extern int SelectedLineWidth;
extern tDocumentStruct *SpecialDrawingHover;
extern int NumSelectedObjects;
extern CToolbox *Toolbox;
extern CDrawing* tmpDrawing;
extern int LastDrawingCreated;

//constructor
CDrawingBox::CDrawingBox(CDrawing *BaseItem)
{
	Base=BaseItem;
	IsToolboxShown=0;
	TheState=0;
	ToolboxX=0;
	ToolboxY=0;
	ToolboxHeight=0;
	ToolboxLength=0;
	ToolboxSelectedItem=0;
	prevDrawingBoxData=-1;
	CommandLine=NULL;
}

//destructor (should release all previously reserved memory)
CDrawingBox::~CDrawingBox(void)
{
	if (CommandLine) delete CommandLine;
}

//Copies relevant data from another object of the same type (this is used
//when a copy of an existing object is to be made)
int CDrawingBox::CopyFrom(CDrawing *Original)
{
	if (Original->IsSpecialDrawing!=50) return 0;
	CDrawingBox *org=(CDrawingBox*)Original->SpecialData;
	if (!org) return 0;

	this->CommandLine=NULL;
	this->IsToolboxShown=0;
	this->TheState=0;
	this->ToolboxHeight=org->ToolboxHeight;
	this->ToolboxLength=org->ToolboxLength;
	this->ToolboxSelectedItem=0;
	this->ToolboxX=org->ToolboxX;
	this->ToolboxY=org->ToolboxY;
	return 1;
}

//Paints the object interior (painting starts from X,Y coordinates - upper left corner)
int CDrawingBox::Paint(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg,int no_background)
{
	if (absX<0) return 0;
	if (!IsToolboxShown) return 0;

	int x1,y1,w,h;
	int ux,uy;
	int sx,sy;

	Base->AdjustCoordinates(&x1,&y1,&w,&h);
	GetDrawingBoxGrid(&ux,&uy,&sx,&sy);

	sx=sx*ViewZoom/100;
	sy=sy*ViewZoom/100;
	ux=ux*ViewZoom;
	uy=uy*ViewZoom;

	if (!no_background)
	{
		//paint coordinate system grid

		DC->SetROP2(R2_MASKPEN);
		DC->SelectObject(GetPenFromPool(1,0,RGB(224,224,224)));

		int step=10;
		if (abs(ux/ViewZoom)>60) step=1;
		for (int i=X+sx,j=0;i<X+w*ViewZoom/100-3;i=X+sx+abs(ux)*j/1000,j+=step) 
			{DC->MoveTo(i,Y+3);DC->LineTo(i,Y+h*ViewZoom/100-3);}
		for (int i=X+sx,j=0;i>X+3;i=X+sx-abs(ux)*j/1000,j+=step) 
			{DC->MoveTo(i,Y+3);DC->LineTo(i,Y+h*ViewZoom/100-3);}

		step=10;
		if (abs(uy/ViewZoom)>60) step=1;
		for (int i=Y+sy,j=0;i<Y+h*ViewZoom/100-3;i=Y+sy+abs(uy)*j/1000,j+=step) 
			{DC->MoveTo(X+3,i);DC->LineTo(X+w*ViewZoom/100-3,i);}
		for (int i=Y+sy,j=0;i>Y+3;i=Y+sy-abs(uy)*j/1000,j+=step) 
			{DC->MoveTo(X+3,i);DC->LineTo(X+w*ViewZoom/100-3,i);}
		
		//painting zero and unit lenghts
		CBrush br(RGB(176,176,176));
		CBrush br2(BLUE_COLOR);
		if (TheState==1) DC->SelectObject(br2); else DC->SelectObject(br);
		DC->SelectObject(GetPenFromPool(1,(TheState==1)?1:0,RGB(176,176,176)));
		DC->Ellipse(X+sx-3,Y+sy-3,X+sx+3,Y+sy+3);

		int tmp1=sx+ux/100;
		int tmp2=sy+uy/100;
		if ((tmp1>0) && (tmp1<w*ViewZoom/100-2))
		{
			DC->FillSolidRect(X+tmp1,Y+sy-5,2,10,(TheState==2)?BLUE_COLOR:RGB(176,176,176));
		}
		if ((tmp2>0) && (tmp2<h*ViewZoom/100-2))
		{
			DC->FillSolidRect(X+sx-5,Y+tmp2,10,2,(TheState==3)?BLUE_COLOR:RGB(176,176,176));
		}
		DC->SetROP2(R2_COPYPEN);

		POINT cp;
		GetCursorPos(&cp);
		pMainView->ScreenToClient(&cp);
		MouseMove(DC,(cp.x-X)*100/ViewZoom,(cp.y-Y)*100/ViewZoom,0x1234);
	}

	if ((TheState==100) && (CommandLine))
	{
		if (KeyboardEntryObject==(CObject*)CommandLine)
		{
			//the command line is active - show the command line text
			short l,a,b;
			CommandLine->CalculateSize(DC,100,&l,&a,&b);
			int xx,yy;
			xx=X+3*ViewZoom/100;
			yy=Y+3*ViewZoom/100+20;
			if (KeyboardEntryBaseObject->absolute_X<ViewX) xx+=(ViewX-KeyboardEntryBaseObject->absolute_X)*ViewZoom/100; 
			if (KeyboardEntryBaseObject->absolute_Y<ViewY) yy+=(ViewY-KeyboardEntryBaseObject->absolute_Y)*ViewZoom/100; 
			CommandLine->PaintExpression(DC,100,xx,yy,ClipReg);
		}
		else
		{
			//the command line is no more active (typing mode ended when user pressed Enter key) - execute command
			ExecuteCommandLine(X,Y,absX,absY);
			TheState=0;
			delete CommandLine;
			CommandLine=NULL;
			pMainView->RepaintTheView(1);
		}
	}

	//calculate position of the toolbox
	CBitmap bmp;
	int iconwidth=0;
	if (ToolboxSize<90) {bmp.LoadBitmap(IDB_TOOLBAR);iconwidth=25;} else {bmp.LoadBitmap(IDB_TOOLBAR2);iconwidth=32;}
	int toolbox_len=9*(iconwidth+5);
	int toolbox_height=iconwidth+2;

	int xx=w*ViewZoom/100-toolbox_len+X-ViewZoom/100;
	int yy=Y+ViewZoom/100+1;
	RECT cr;
	pMainView->GetClientRect(&cr);
	cr.right=cr.right*100/ViewZoom;
	if (absX+w>ViewX+cr.right) 
	{
		xx=(ViewX+cr.right-absX)*ViewZoom/100+X-toolbox_len+ViewZoom/100;
	}
	if (absY-ViewY<0) yy=(ViewY-absY)*ViewZoom/100+Y;
	if (xx-X<ViewZoom/100) xx=X+ViewZoom/100;

	xx-=2;

	ToolboxX=(xx-X)*100/ViewZoom;
	xx=ToolboxX*ViewZoom/100+X; //to round xx according to ViewZoom
	ToolboxY=(yy-Y)*100/ViewZoom;
	ToolboxLength=toolbox_len;
	ToolboxHeight=toolbox_height;
	CDC mdc;
	mdc.CreateCompatibleDC(DC);
	mdc.SelectObject(bmp);
	CRgn myrgn;
	myrgn.CreateRectRgn(X,Y,X+w*ViewZoom/100,yy+toolbox_height);
	DC->SelectClipRgn(&myrgn);

	for (int i=0;i<9;i++)
	{
		int iconpos=0;
		int DrawingTool=-1;

		if (IsToolboxShown==2)
		{
			//options for selections (align, group, mirror...)
			if (i==0) iconpos=11;
			if (i==1) iconpos=12;
			if (i==2) iconpos=13;
			if (i==3) iconpos=14;
			if (i==4) iconpos=15;
			if (i==5) iconpos=16;
			if (i==6) iconpos=22;
			if (i==7) iconpos=20;
			if (i==8) iconpos=21;
		}
		else
		{
			//options for drawing tools
			if (i==0) {iconpos=32;DrawingTool=2;} //line
			if (i==1) {iconpos=35;DrawingTool=1;} //rect
			if (i==2) {iconpos=36;DrawingTool=4;} //circle
			if (i==3) {iconpos=41;DrawingTool=17;} //cienter-circle
			if (i==4) {iconpos=44;DrawingTool=18;} //section line
			if (i==5) {iconpos=38;DrawingTool=7;} //eraser small
			if (i==6) {iconpos=37;DrawingTool=6;} //eraser big
			if (i==7) {iconpos=42;DrawingTool=26;} //diagonals
			if (i==8) {iconpos=43;DrawingTool=25;} //diagonals
		}

		CBitmap bbb;
		bbb.CreateCompatibleBitmap(DC,iconwidth,iconwidth);
		CDC ccc;
		ccc.CreateCompatibleDC(DC);
		ccc.SelectObject(bbb);
		ccc.BitBlt(0,0,iconwidth,iconwidth,&mdc,iconpos*iconwidth+1,0,SRCCOPY);

		DC->FillSolidRect(xx+(i*(iconwidth+5)),yy,iconwidth+5,toolbox_height,RGB(208,208,255));
		if (ToolboxSelectedItem==i+1)
			DC->FillSolidRect(xx+(i*(iconwidth+5)),yy,iconwidth+1,toolbox_height,RGB(160,160,224));
		if (i!=8)
		{
			DC->FillSolidRect(xx+(i*(iconwidth+5))+iconwidth+3,yy,1,toolbox_height,RGB(240,240,255));
			DC->FillSolidRect(xx+(i*(iconwidth+5))+iconwidth+2,yy,1,toolbox_height,RGB(160,160,192));
		}
		

		DC->TransparentBlt(xx+(i*(iconwidth+5))+2,yy+1,iconwidth-1,iconwidth-1,&ccc,0,0,iconwidth-1,iconwidth-1,RGB(255,255,255));

		if (DrawingTool==IsDrawingMode)
			PaintCheckedSign(DC,xx+i*(iconwidth+5)+iconwidth/2+2,yy+iconwidth/2,iconwidth/2,1);
	}
	//toolbox border lines
	DC->FillSolidRect(xx,yy,toolbox_len,1,RGB(160,160,192));
	DC->FillSolidRect(xx,yy,1,toolbox_height,RGB(160,160,192));
	DC->FillSolidRect(xx,yy+toolbox_height-1,toolbox_len,1,RGB(160,160,192));
	DC->FillSolidRect(xx+toolbox_len,yy,1,toolbox_height,RGB(160,160,192));
	DC->SelectClipRgn(NULL);

	return 1;
}

#pragma optimize("s",on)
int CDrawingBox::MouseClick(int X, int Y)
{
	if ((X==0x7FFFFFFF) && (Y==0x7FFFFFFF)) return 0;
	
	if ((TheState==100) && (CommandLine) && (CommandLine->m_IsKeyboardEntry))
	{
		//adds coordinate values into command line (when clicked somewhere inside the drawing box)
		if (GetKeyState(VK_MENU)&0xFFFE)
		if (SpecialDrawingHover)
		{
			int px=-X+SpecialDrawingHover->absolute_X;
			int py=-Y+SpecialDrawingHover->absolute_Y;
			Base->FindNerbyPoint(&px,&py,NULL,0,0,0,0);
			X=SpecialDrawingHover->absolute_X-px;
			Y=SpecialDrawingHover->absolute_Y-py;
		}

		int unit_size_x, unit_size_y;
		int startx,starty;
		GetDrawingBoxGrid(&unit_size_x,&unit_size_y,&startx,&starty);	
		float posx=(float)(-X-startx)/(float)unit_size_x;
		float posy=(float)(-Y-starty)/(float)unit_size_y;
		tElementStruct *ts=CommandLine->m_pElementList+CommandLine->m_IsKeyboardEntry-1;
		if (ts->pElementObject->Data1[0]==0) CommandLine->DeleteElement(CommandLine->m_IsKeyboardEntry-1);
		ts=CommandLine->m_pElementList+CommandLine->m_NumElements-1;
		if (ts->Type==1) 
		{
			char ch=ts->pElementObject->Data1[0];
			if (((ch>=0) && (ch<='9')) || (ch=='.')) CommandLine->InsertEmptyElement(CommandLine->m_NumElements,2,',');
		}
		if (ts->Type==5) CommandLine->InsertEmptyElement(CommandLine->m_NumElements,2,',');
		CommandLine->GenerateASCIINumber(posx,(long long)(posx+(posx>=0)?0.01:-0.01),(posx-(long long)(posx+(posx>=0)?0.01:-0.01))<1e-100?1:0,2,(ts->Type==0)?0:CommandLine->m_NumElements);
		CommandLine->InsertEmptyElement(CommandLine->m_NumElements,2,',');
		CommandLine->GenerateASCIINumber(posy,(long long)(posy+(posy>=0)?0.01:-0.01),(posy-(long long)(posy+(posy>=0)?0.01:-0.01))<1e-100?1:0,2,CommandLine->m_NumElements);
		CommandLine->InsertEmptyElement(CommandLine->m_NumElements,1,0);
		CommandLine->m_IsKeyboardEntry=CommandLine->m_NumElements;
		CommandLine->m_KeyboardCursorPos=0;
		return 0;
	}

	if ((TheState==101))
	{
		for (int i=0;i<NumDocumentElements;i++)
		{
			tDocumentStruct *ds=TheDocument+i;
			if (ds->Type==2)
			{
				CDrawing *d=(CDrawing*)ds->Object;
				if ((d->IsSpecialDrawing==50) && (d!=Base) && (((CDrawingBox*)(d->SpecialData))->TheState==100))
					((CDrawingBox*)(d->SpecialData))->TheState=0;
			}
		}
		TheState=100;
		if (CommandLine) delete CommandLine;
		CommandLine=new CExpression(NULL,NULL,80);

		//will start the keyboard entry
		if (KeyboardEntryObject) 
		{
			((CExpression*)KeyboardEntryObject)->KeyboardStop(); 
			if ((KeyboardEntryBaseObject) && 
				(((CExpression*)KeyboardEntryBaseObject->Object)->m_NumElements==1) && 
				(((CExpression*)KeyboardEntryBaseObject->Object)->m_pElementList->Type==0))
				pMainView->DeleteDocumentObject(KeyboardEntryBaseObject);
		}
		KeyboardEntryObject=(CObject*)CommandLine;
		((CExpression*)KeyboardEntryObject)->DeselectExpression();
		((CExpression*)KeyboardEntryObject)->m_Selection=1;

		for (int i=0;i<NumDocumentElements;i++)
			if ((TheDocument+i)->Object==(CObject*)Base)
			{
				KeyboardEntryBaseObject=TheDocument+i;
				break;
			}
		CDC *DC=pMainView->GetDC();
		((CExpression*)KeyboardEntryObject)->KeyboardStart(DC,ViewZoom);
		((CExpression*)KeyboardEntryObject)->m_Selection=0;
	
		pMainView->ReleaseDC(DC);
		return 0;	
	}

	if ((ToolboxSelectedItem)) //the drawing box
	{
		if (IsToolboxShown==2) //align and grouping of selections
		{
			if (ToolboxSelectedItem==7) //grouping
			{
				if (NumSelectedObjects<2) return 1;
				//grouping
				CDrawing *tmpdrw=NULL;
				int found=0;
				int StartX=0x7FFFFFFF;
				int StartY=0x7FFFFFFF;
				tDocumentStruct *prevelement=NULL;
				for (int ii=0;ii<NumDocumentElements;ii++)
				{
					tDocumentStruct *ds=TheDocument+ii;
					
					if ((ds->Object) && (ds->MovingDotState==3))
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
						found++;
					}
				}

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
				ds->MovingDotState=3;
				ds->Object=(CObject*)tmpdrw;
				pMainView->RepaintTheView();
				return 1;
			}
			else //aligning and mirror
			{
				int MaxX,MaxY;
				int StartX=0x7FFFFFFF;
				int StartY=0x7FFFFFFF;
				MaxX=MaxY=0x80000000;
				for (int kkk=0;kkk<NumDocumentElements;kkk++)
				{
					tDocumentStruct *ds2=TheDocument+kkk;
					if ((ds2->Object) && (ds2->MovingDotState==3)) 
					{
						if (ds2->absolute_X<StartX) StartX=ds2->absolute_X;
						if (ds2->absolute_Y-ds2->Above<StartY) StartY=ds2->absolute_Y-ds2->Above;
						if (ds2->absolute_X+ds2->Length>MaxX) MaxX=ds2->absolute_X+ds2->Length;
						if (ds2->absolute_Y+ds2->Below>MaxY) MaxY=ds2->absolute_Y+ds2->Below;
					}
				}
				if (StartX==0x7FFFFFFF) return 1;
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("align",20113);
				if (ToolboxSelectedItem==1) {StartY=-1;} //left
				else if (ToolboxSelectedItem==2) {StartX=(StartX+MaxX)/2;StartY=-1;} //H.center
				else if (ToolboxSelectedItem==3) {StartX=MaxX;StartY=-1;} //right
				else if (ToolboxSelectedItem==4) {StartX=-1;} //top
				else if (ToolboxSelectedItem==5) {StartX=-1;StartY=(StartY+MaxY)/2;} //V.center
				else if (ToolboxSelectedItem==6) {StartX=-1;StartY=MaxY;} //bottom

				for (int kkk=0;kkk<NumDocumentElements;kkk++)
				{
					tDocumentStruct *ds=TheDocument+kkk;
					if ((ds->Object) && (ds->MovingDotState==3)) 
					{
						if (ToolboxSelectedItem==1) ds->absolute_X=StartX;
						else if (ToolboxSelectedItem==2) ds->absolute_X=StartX-ds->Length/2;
						else if (ToolboxSelectedItem==3) ds->absolute_X=StartX-ds->Length;
						else if (ToolboxSelectedItem==4) ds->absolute_Y=StartY+ds->Above;
						else if (ToolboxSelectedItem==5)
						{	
							if (ds->Type==1)  ds->absolute_Y=StartY;
							if (ds->Type==2)  ds->absolute_Y=StartY-ds->Below/2;
						}
						else if (ToolboxSelectedItem==6) ds->absolute_Y=StartY-ds->Below;
						else if (ToolboxSelectedItem==8) //horizontal mirror
						{
							if (ds->Type==2) 
								((CDrawing*)(ds->Object))->ScaleForFactor(-1,1);
							ds->absolute_X=StartX+(MaxX-ds->absolute_X-ds->Length);
						}
						else if (ToolboxSelectedItem==9) //vertical mirror
						{
							if (ds->Type==2) 
								((CDrawing*)(ds->Object))->ScaleForFactor(1,-1);
							ds->absolute_Y=StartY+(MaxY-ds->absolute_Y-ds->Below);
						}
					}
				}
				pMainView->RepaintTheView();
				return 1;
			}
		}
		if (ToolboxSelectedItem==1) //line
		{
			IsDrawingMode=2;
			return 1;
		}
		if (ToolboxSelectedItem==2) //rectangle
		{
			IsDrawingMode=1;
			return 1;
		}
		if (ToolboxSelectedItem==3) //circle
		{
			IsDrawingMode=4;
			return 1;
		}
		if (ToolboxSelectedItem==4) //center-circle
		{
			IsDrawingMode=17;
			return 1;
		}
		if (ToolboxSelectedItem==5) //section line
		{
			IsDrawingMode=18;
			return 1;
		}
		if (ToolboxSelectedItem==6) //small eraser
		{
			IsDrawingMode=7;
			return 1;
		}
		if (ToolboxSelectedItem==7) //big eraser
		{
			IsDrawingMode=6;
			return 1;
		}
		if (ToolboxSelectedItem==8) //diagonals
		{
			IsDrawingMode=26;
			return 1;
		}
		if (ToolboxSelectedItem==9) //diagonals
		{
			IsDrawingMode=25;
			return 1;
		}
	}
	return 0;
}
#pragma optimize("",on)

int CDrawingBox::MouseMove(CDC *DC,int X, int Y, UINT flags)
{		
	int redraw_flag=0;
	if ((TheState==100) && (CommandLine==NULL)) TheState=0;

	int zX=X*ViewZoom/100;
	int zY=Y*ViewZoom/100;
	ToolboxSelectedItem=0;
	int xx=ToolboxX*ViewZoom/100;
	int yy=ToolboxY*ViewZoom/100;
	int xx2=(ToolboxX)*ViewZoom/100+ToolboxLength;
	int yy2=(ToolboxY)*ViewZoom/100+ToolboxHeight;
	int iconwidth=0;
	if (ToolboxSize<90) iconwidth=25; else iconwidth=32;
	if ((zX>xx) && (zX<xx2) && (zY>=yy) && (zY<=yy2))
	{
		ToolboxSelectedItem=(zX-xx)/(iconwidth+5)+1;
	}
	IsToolboxShown=1;  //show the toolbox
	if (NumSelectedObjects>0) IsToolboxShown=2;
	if ((X==0x7FFFFFFF) && (Y==0x7FFFFFFF))
	{
		//special case - called when cursor is outside drawing
		IsToolboxShown=0;
	}	
	
	if ((CommandLine) && (TheState) && (!IsToolboxShown)) IsToolboxShown=1;

	if (prevDrawingBoxData!=ToolboxSelectedItem+IsToolboxShown) 
	{
		redraw_flag=1;
		prevDrawingBoxData=ToolboxSelectedItem+IsToolboxShown;
	}
	if ((X==0x7FFFFFFF) && (Y==0x7FFFFFFF))
	{
		return redraw_flag;
	}	
		
	if (SpecialDrawingHover==NULL) return redraw_flag;

	int mx,my;
	mx=(SpecialDrawingHover->absolute_X-ViewX)*ViewZoom/100;
	my=(SpecialDrawingHover->absolute_Y-ViewY)*ViewZoom/100;

	//mx and my contain upper left corner
	int unit_size_x, unit_size_y;
	int startx,starty;
	GetDrawingBoxGrid(&unit_size_x,&unit_size_y,&startx,&starty);	
	float posx=(float)(X-startx)/(float)unit_size_x;
	float posy=(float)(Y-starty)/(float)unit_size_y;

	if (SpecialDrawingHover->absolute_X<ViewX) mx=0;
	if (SpecialDrawingHover->absolute_Y<ViewY) my=0;
	DC->SetTextAlign(TA_TOP);
	DC->SetBkColor(RGB(255,255,255));
	DC->SetBkMode(OPAQUE);
	DC->SelectObject(GetFontFromPool(4,0,0,12));
	char txt[64];
	char txt2[32];

	if (1)
	{
		static int LastIntersectionPointX=-1;
		static int LastIntersectionPointY=-1;
		int xx=X+SpecialDrawingHover->absolute_X;
		int yy=Y+SpecialDrawingHover->absolute_Y;

		if ((GetKeyState(VK_MENU)&0xFFFE) && (flags!=0x1234) &&
			(Base->FindNerbyPoint(&xx,&yy,NULL,0,0,0,0)))
		{				
			CPen pen(PS_SOLID,1,RGB(0,255,0));
			HANDLE olpen=DC->SelectObject(pen);
			DC->Arc((xx-ViewX)*ViewZoom/100-2,(yy-ViewY)*ViewZoom/100-2,(xx-ViewX)*ViewZoom/100+3,(yy-ViewY)*ViewZoom/100+3,0,0,1,0);

			LastIntersectionPointX=xx;
			LastIntersectionPointY=yy;
			DC->SetBkColor(RGB(255,255,255));
			xx-=SpecialDrawingHover->absolute_X;
			yy-=SpecialDrawingHover->absolute_Y;
			posx=(float)(xx-startx)/(float)unit_size_x;
			posy=(float)(yy-starty)/(float)unit_size_y;
			DC->SelectObject(olpen);
		}
		else if (LastIntersectionPointX!=-1) 
		{
			if (flags!=0x1234) pMainView->RepaintTheView();
			LastIntersectionPointX=-1;
			DC->SetBkColor(RGB(255,255,255));
		}

		if (abs(unit_size_x)<=10) 
			sprintf(txt,"x:%5.1f, ",(float)posx);
		else
			sprintf(txt,"x:%6.2f, ",(float)posx);
		if (abs(unit_size_y)<=10)
			sprintf(txt2,"y:%5.1f  ",(float)posy);
		else
			sprintf(txt2,"y:%6.2f  ",(float)posy);
		strcat(txt,txt2);
		DC->TextOut(mx+4*ViewZoom/100,my+3*ViewZoom/100,txt);		
	}

	if (TheState==100) return redraw_flag;
	int prevstate=TheState;

	if ((MouseMode==0) && (IsDrawingMode==0))
	{
		int cx=4;
		int cy=3+15*100/ViewZoom;
		int command_selected=0;
		if (SpecialDrawingHover->absolute_X<ViewX) cx+=ViewX-SpecialDrawingHover->absolute_X;
		if (SpecialDrawingHover->absolute_Y<ViewY) cy+=ViewY-SpecialDrawingHover->absolute_Y;

		if ((X>=cx) && (X<=cx+10*100/ViewZoom) && (Y>=cy) && (Y<=cy+10*100/ViewZoom)) 
		{
			command_selected=1;
			TheState=101;
		}
		
		DC->SelectObject(GetPenFromPool(2,command_selected,0));
		DC->MoveTo(mx+4*ViewZoom/100,my+3*ViewZoom/100+15);
		DC->LineTo(mx+4*ViewZoom/100+5,my+3*ViewZoom/100+20);
		DC->LineTo(mx+4*ViewZoom/100,my+3*ViewZoom/100+25);			
		if (command_selected) return redraw_flag;
	}

	int tmp=0;
	if ((IsDrawingMode==0) && (MouseMode==0))
	{
		if ((X>startx-3) && (X<startx+3) && (Y>starty-3) && (Y<starty+3)) tmp=1;
		if ((X>startx+unit_size_x-2) && (X<startx+unit_size_x+2) && (Y>starty-5) && (Y<starty+5)) tmp=2;
		if ((X>startx-5) && (X<startx+5) && (Y>starty+unit_size_y-2) && (Y<starty+unit_size_y+2)) tmp=3;
		if ((flags&MK_LBUTTON) && (TheState)) 
			tmp=TheState;
	}
	if (tmp)
	{
		TheState=tmp;
		if (flags&MK_LBUTTON)
		{

			if (tmp==1)
			{
				startx=X;
				starty=Y;
			}
			if (tmp==2) {unit_size_x=X-startx;unit_size_y=abs(unit_size_x)*((unit_size_y<0)?-1:1);}
			if (tmp==3) unit_size_y=Y-starty;
			if (unit_size_x==0) unit_size_x=1;
			if (unit_size_y==0) unit_size_y=-1;
			while (Base->NumItems<8)
			{
				Base->NumItems++;
				if (Base->NumItems>Base->NumItemsReserved) Base->NumItemsReserved=Base->NumItems;
				Base->Items=(tDrawingItem*)realloc(Base->Items,Base->NumItemsReserved*sizeof(tDrawingItem));
				(Base->Items+Base->NumItems-1)->Type=2;
				(Base->Items+Base->NumItems-1)->X1=0;
				(Base->Items+Base->NumItems-1)->X2=0;
				(Base->Items+Base->NumItems-1)->Y1=0;
				(Base->Items+Base->NumItems-1)->Y2=0;
				(Base->Items+Base->NumItems-1)->LineWidth=0;
				(Base->Items+Base->NumItems-1)->pSubdrawing=new CExpression(NULL,NULL,100);
			}
			tDrawingItem *di=Base->Items+4;
			while (((CExpression*)(di->pSubdrawing))->m_pElementList->Type) ((CExpression*)(di->pSubdrawing))->DeleteElement(0);
			((CExpression*)(di->pSubdrawing))->GenerateASCIINumber((double)startx,startx,1,0,0);
			di++;
			while (((CExpression*)(di->pSubdrawing))->m_pElementList->Type) ((CExpression*)(di->pSubdrawing))->DeleteElement(0);
			((CExpression*)(di->pSubdrawing))->GenerateASCIINumber((double)starty,starty,1,0,0);
			di++;
			while (((CExpression*)(di->pSubdrawing))->m_pElementList->Type) ((CExpression*)(di->pSubdrawing))->DeleteElement(0);
			((CExpression*)(di->pSubdrawing))->GenerateASCIINumber((double)unit_size_x,unit_size_x,1,0,0);
			di++;
			while (((CExpression*)(di->pSubdrawing))->m_pElementList->Type) ((CExpression*)(di->pSubdrawing))->DeleteElement(0);
			((CExpression*)(di->pSubdrawing))->GenerateASCIINumber((double)unit_size_y,unit_size_y,1,0,0);
			pMainView->RepaintTheView();
		}
	}
	else if ((!(flags&MK_LBUTTON)) && (flags!=0x1234))
	{
		TheState=0;
	}
	if (prevstate!=TheState)
	{
		pMainView->RepaintTheView();
	}

	//handling the drawing box

	if ((MouseMode==6) && (tmpDrawing) && ((IsDrawingMode==1) || (IsDrawingMode==4) || (IsDrawingMode==2) || (IsDrawingMode==17) || (IsDrawingMode==27)))
	{
		int x1=0x7FFFFFFF;
		int x2=-x1;
		int y1=x1;
		int y2=x2;
		for (int i=0;i<tmpDrawing->NumItems;i++)
		{
			tDrawingItem *di=tmpDrawing->Items+i;
			if (di->X1<x1) x1=di->X1;
			if (di->X2<x1) x1=di->X2;
			if (di->Y1<y1) y1=di->Y1;
			if (di->Y2<y1) y1=di->Y2;
			if (di->X1>x2) x2=di->X1;
			if (di->X2>x2) x2=di->X2;
			if (di->Y1>y2) y2=di->Y1;
			if (di->Y2>y2) y2=di->Y2;
		}
		if (IsDrawingMode==2)
		{
			double xx=((double)x2-(double)x1)/DRWZOOM;
			double yy=((double)y2-(double)y1)/DRWZOOM;
			xx/=unit_size_x;
			yy/=unit_size_y;
			float l=(float)sqrt(xx*xx+yy*yy);
			float angle=(float)(atan2((double)(y2-y1),(double)(x2-x1))*180.0/3.14159265);
			if (((int)(angle*10+1)%150)<=2) DC->SetBkColor(RGB(255,255,0));
			sprintf(txt,"len:%5.1f, ang:%5.1f  ",(float)l,(float)angle);
			DC->TextOut(mx+4*ViewZoom/100,my+3*ViewZoom/100+12,txt);
		}
		else
		{
			int w=(x2-x1)/DRWZOOM;
			int h=(y2-y1)/DRWZOOM;
			if (w==h) DC->SetBkColor(RGB(255,255,0));

			sprintf(txt,"w:%5.1f, h:%5.1f  ",(float)fabs((float)w/(float)unit_size_x),(float)fabs((float)h/(float)unit_size_y));
			DC->TextOut(mx+4*ViewZoom/100,my+3*ViewZoom/100+12,txt);
		}
	}
	return redraw_flag;
	
}

int CDrawingBox::XML_output(char * output, int num_tabs, char only_calculate)
{
	return 0;
}

char *CDrawingBox::XML_input(char * file)
{
	return file;
}


void CDrawingBox::GetDrawingBoxGrid(int *unit_size_x, int *unit_size_y, int *startx, int *starty)
{
	int x1,y1,w,h;
	Base->AdjustCoordinates(&x1,&y1,&w,&h);
	*unit_size_x=GRID;
	*unit_size_y=-GRID;
	*startx=5;
	*starty=h-5;
	if (Base->NumItems>=8)
	{
		tDrawingItem *di=Base->Items+4;
		double N;
		int prec;
		if ((di->Type==2) && (((CExpression*)di->pSubdrawing)->IsPureNumber(0,((CExpression*)di->pSubdrawing)->m_NumElements,&N,&prec)))
			*startx=(int)(N+((N>=0)?0.01:-0.01));
		di++;
		if ((di->Type==2) && (((CExpression*)di->pSubdrawing)->IsPureNumber(0,((CExpression*)di->pSubdrawing)->m_NumElements,&N,&prec)))
			*starty=(int)(N+((N>=0)?0.01:-0.01));
		di++;
		if ((di->Type==2) && (((CExpression*)di->pSubdrawing)->IsPureNumber(0,((CExpression*)di->pSubdrawing)->m_NumElements,&N,&prec)))
			*unit_size_x=(int)(N+((N>=0)?0.01:-0.01));
		di++;
		if ((di->Type==2) && (((CExpression*)di->pSubdrawing)->IsPureNumber(0,((CExpression*)di->pSubdrawing)->m_NumElements,&N,&prec)))
			*unit_size_y=(int)(N+((N>=0)?0.01:-0.01));
	}
}

#pragma optimize("s",on)
int CDrawingBox::ExecuteCommandLine(short X, short Y, int absX, int absY)
{
	int kk=0;
	while (kk<CommandLine->m_NumElements)
	{
		tElementStruct *ts=CommandLine->m_pElementList+kk;
		if (ts->Type==1)
		{
			int cmd=0;
			float p1[10];
			if (strnicmp(ts->pElementObject->Data1,"line",4)==0) cmd=1;
			if (strnicmp(ts->pElementObject->Data1,"vline",5)==0) cmd=2;
			if (strnicmp(ts->pElementObject->Data1,"rect",4)==0) cmd=3;
			if (strnicmp(ts->pElementObject->Data1,"square",6)==0) cmd=3;
			if (strnicmp(ts->pElementObject->Data1,"ellip",5)==0) cmd=4;
			if (strnicmp(ts->pElementObject->Data1,"circ",4)==0) cmd=7;
			if (strnicmp(ts->pElementObject->Data1,"rot",3)==0) cmd=5;
			if (strnicmp(ts->pElementObject->Data1,"move",4)==0) cmd=6;
			if (strnicmp(ts->pElementObject->Data1,"dashdot",7)==0) cmd=9;
			else if (strnicmp(ts->pElementObject->Data1,"dash",4)==0) cmd=8;
			if (strnicmp(ts->pElementObject->Data1,"hair",4)==0) cmd=10;
			if (strnicmp(ts->pElementObject->Data1,"thin",4)==0) cmd=11;
			if (strnicmp(ts->pElementObject->Data1,"thick",5)==0) cmd=13;
			if (strnicmp(ts->pElementObject->Data1,"medium",6)==0) cmd=12;
			if (strnicmp(ts->pElementObject->Data1,"black",4)==0) cmd=14;
			if (strnicmp(ts->pElementObject->Data1,"gray",4)==0) cmd=15;
			if (strnicmp(ts->pElementObject->Data1,"red",4)==0) cmd=16;
			if (strnicmp(ts->pElementObject->Data1,"blue",5)==0) cmd=17;
			if (strnicmp(ts->pElementObject->Data1,"green",6)==0) cmd=18;

			kk++;
			int opos=kk;
			int params=0;
			while (kk<CommandLine->m_NumElements)
			{
				int okk=2;
				double N;
				int p;
				ts=CommandLine->m_pElementList+kk;
				char chh=0;
				if (ts->pElementObject) chh=ts->pElementObject->Data1[0];
				if ((ts->Type==1) && (((chh<'0') || (chh>'9')) && (chh!='.'))) break;
				if (kk==CommandLine->m_NumElements-1) okk=0;
				if ((ts->Type==2) && ((ts->pElementObject->Data1[0]==',') || (ts->pElementObject->Data1[0]==';'))) okk=1;
				if ((ts->Type==11) || (ts->Type==12)) okk=1;
				if (okk!=2)
				{
					if (CommandLine->IsPureNumber(opos,kk-opos-okk+1,&N,&p))
						p1[params++]=(float)N;
					opos=kk+1;
				}
				kk++;
			}

			int ux,uy,sx,sy;
			GetDrawingBoxGrid(&ux,&uy,&sx,&sy);
			int InsertPositionY=max(absY,ViewY)+33;
			int InsertPositionX=max(absX,ViewX)+3;
			if ((((cmd>=1) && (cmd<=4)) || (cmd==7)) && (params>=1) && (params<=2))
			{
				//we will check if there already exist an object at position InsertPositionY, InsertPositionX and readjust insert positions;
				int k=0;
				while (k<20)
				{
					int ii;
					for (ii=0;ii<NumDocumentElements;ii++)
						if ((TheDocument[ii].absolute_X==InsertPositionX) && (TheDocument[ii].absolute_Y==InsertPositionY))
						{
							InsertPositionY+=TheDocument[ii].Below;
							break;
						}
					if (ii==NumDocumentElements) break;
					k++;
				}
			}

			if (((cmd==1) || (cmd==2)) && (params>=1))
			{
				if (params<=2)
				{
					//line length[,angle]
					//vline length[,angle]
					int x1,y1;
					if (params<2) p1[1]=0;
					if (cmd==2) p1[1]+=90;
					//p1[0]=p1[0]*100/ViewZoom;
					AddDocumentObject(2,InsertPositionX,InsertPositionY);
					CDrawing *drw=new CDrawing();
					drw->StartCreatingItem(2);
					drw->UpdateCreatingItem((int)(p1[0]*ux*cos(p1[1]/180*3.14159265)*10),(int)(p1[0]*uy*sin(p1[1]/180*3.14159265)*10),X,Y);
					drw->EndCreatingItem(&x1,&y1);
					TheDocument[NumDocumentElements-1].Object=(CObject*)drw;
					LastDrawingCreated=NumDocumentElements-1;
				}
				else
				{
					//line x1,y1,x2,y2[,x3,y3...]
					//GetDrawingBoxGrid(&ux,&uy,&sx,&sy);
					int x1,y1;
					if (params<4) p1[3]=p1[1];
					AddDocumentObject(2,X*100/ViewZoom+ViewX+(int)(p1[0]*ux)+sx,Y*100/ViewZoom+ViewY+(int)(p1[1]*uy)+sy);
					CDrawing *drw=new CDrawing();
					drw->StartCreatingItem(2);
					for (int iii=2;iii<params;iii+=2)
						drw->UpdateCreatingItem((int)((p1[iii]-p1[0])*ux*10),(int)((p1[iii+1]-p1[1])*uy*10),-1,-1);
					drw->EndCreatingItem(&x1,&y1);
					TheDocument[NumDocumentElements-1].absolute_X+=x1;
					TheDocument[NumDocumentElements-1].absolute_Y+=y1;
					TheDocument[NumDocumentElements-1].Object=(CObject*)drw;
					LastDrawingCreated=NumDocumentElements-1;
					int w,h;
					drw->AdjustCoordinates(&x1,&y1,&w,&h);
					TheDocument[NumDocumentElements-1].Length=w;
					TheDocument[NumDocumentElements-1].Below=h;
				}
				
			}
			if (((cmd==3) || (cmd==4) || (cmd==7)) && (params>=1))
			{
				if (params<=2)
				{
					//rect width[,height]
					//ellip widht[,height]
					//circ widht[,height]
					int x1,y1;
					//p1[0]=p1[0]*100/ViewZoom;
					//p1[1]=p1[1]*100/ViewZoom;
					if (params<2) p1[1]=p1[0];
					AddDocumentObject(2,InsertPositionX,InsertPositionY);
					CDrawing *drw=new CDrawing();
					drw->StartCreatingItem((cmd==3)?1:4);
					drw->UpdateCreatingItem((int)(p1[0]*ux*10),(int)(p1[1]*uy*10),X,Y);
					drw->EndCreatingItem(&x1,&y1);
					TheDocument[NumDocumentElements-1].Object=(CObject*)drw;
					LastDrawingCreated=NumDocumentElements-1;
				}
				else
				{
					//rect x1,y1,x2,y2
					//ellip x1,y1,x2,y2
					//circ x1,y1,x2,y2
					//circ x1,y1,radius
					//GetDrawingBoxGrid(&ux,&uy,&sx,&sy);
					int x1,y1;
					AddDocumentObject(2,X*100/ViewZoom+ViewX+(int)(p1[0]*ux)+sx,Y*100/ViewZoom+ViewY+(int)(p1[1]*uy)+sy);
					CDrawing *drw=new CDrawing();
					if (params==4)
					{
						drw->StartCreatingItem((cmd==3)?1:4);
						drw->UpdateCreatingItem((int)((p1[2]-p1[0])*ux*10),(int)((p1[3]-p1[1])*uy*10),X,Y);
					}
					else
					{
						drw->StartCreatingItem(17); //center drawn circle
						drw->UpdateCreatingItem((int)((p1[2])*ux*10),(int)((p1[2])*uy*10),X,Y);
					}
					drw->EndCreatingItem(&x1,&y1);
					TheDocument[NumDocumentElements-1].absolute_X+=x1;
					TheDocument[NumDocumentElements-1].absolute_Y+=y1;
					TheDocument[NumDocumentElements-1].Object=(CObject*)drw;
					LastDrawingCreated=NumDocumentElements-1;
					int w,h;
					drw->AdjustCoordinates(&x1,&y1,&w,&h);
					TheDocument[NumDocumentElements-1].Length=w;
					TheDocument[NumDocumentElements-1].Below=h;
				}
			}
			if ((LastDrawingCreated>0) && (LastDrawingCreated<NumDocumentElements) &&
				(TheDocument[LastDrawingCreated].Type==2) && (TheDocument[LastDrawingCreated].Object))
			{
				if (cmd==5)
				{
					//rot angle
					int cx=TheDocument[LastDrawingCreated].absolute_X+TheDocument[LastDrawingCreated].Length/2;
					int cy=TheDocument[LastDrawingCreated].absolute_Y+TheDocument[LastDrawingCreated].Below/2;
					int x1,y1,w,h;
					((CDrawing*)(TheDocument[LastDrawingCreated].Object))->RotateForAngle((float)(p1[0]/180*3.14159265),cx,cy,&x1,&y1,&w,&h);
				}
				if (cmd==6)
				{
					//move xlen,[ylen]
					TheDocument[LastDrawingCreated].absolute_X+=(int)(ux*p1[0]);
					if (params==2) TheDocument[LastDrawingCreated].absolute_Y+=(int)(uy*p1[1]);
				}
				if ((cmd==8) || (cmd==9))
				{
					//dashed
					CDrawing *drw=(CDrawing*)TheDocument[LastDrawingCreated].Object;
					drw->MakeDashed((cmd==9)?1:0);
				}
				if (cmd==10) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetLineWidth(DRWZOOM*65/100);
				if (cmd==11) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetLineWidth(DRWZOOM);
				if (cmd==12) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetLineWidth(2*DRWZOOM);
				if (cmd==13) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetLineWidth(3*DRWZOOM);
				if (cmd==14) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetColor(0);
				if (cmd==15) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetColor(4);
				if (cmd==16) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetColor(1);
				if (cmd==17) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetColor(3);
				if (cmd==18) ((CDrawing*)TheDocument[LastDrawingCreated].Object)->SetColor(2);
			}
		}
		else kk++;
	}

	return 1;
}