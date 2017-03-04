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

extern char XMLFileVersion; //defines XML file version when loading/saving files

extern tDocumentStruct *SpecialDrawingHover; 
extern CToolbox *Toolbox;
extern CMathomirView *pMainView;
extern PopupMenu *Popup;
extern CDrawing* ClipboardDrawing;
extern int LastTakenChecksum;
extern int MovingStartX;
extern int MovingStartY;
extern CExpression* ClipboardExpression;
extern int NumSelectedObjects;
int SelectedLineWidth=2*DRWZOOM;
int SelectedLineColor=0;
int LastDrawingCreated;
extern char DrawingThreadUsed;



HPEN GrayThin=NULL;
HPEN GrayFat=NULL;
CDrawing::CDrawing(void)
{
	if (GrayThin==NULL)
	{
		GrayThin=CreatePen(PS_SOLID,1,RGB(160,160,160));
		GrayFat=CreatePen(PS_SOLID,2,RGB(160,160,160));
	}
	Items=NULL;
	NumItems=0;
	NumItemsReserved=0;
	IsSelected=0;
	OriginalForm=0;
	//IsNodeEdit=0;
	IsSpecialDrawing=0;
	NodeX=-1;
	NodeY=-1;
	m_Color=-1;
}

CDrawing::~CDrawing(void)
{
	//special handling - if clipboard is deleted, send command to other windows that must be deleted
	if (this==ClipboardDrawing)
	{
		if (theApp.m_pMainWnd->OpenClipboard())
		{
			HANDLE hmem=GlobalAlloc(GMEM_ZEROINIT,16);
			LPVOID pntr=GlobalLock(hmem);
			int *control=(int*)pntr;
			*control=0xBBCCDD11; //special code
			GlobalUnlock(hmem);
			UINT format=RegisterClipboardFormat("MATHOMIR_EXPR");
			HANDLE ret=SetClipboardData(format,hmem);
			CloseClipboard();
		}
	}
	Delete();

}

int Drawing_temp_form;
int Drawing_temp_start;
DWORD Drawing_last_tick=0;
int Drawing_prev_x,Drawing_prev_y;
int Drawing_inside_create=0;
//this functions starts creation of new drawing item of given type
//X and Y are in absolute coordintes
//ItemForm = 1-Rectangle, 2-Circle
int Drawing_start_x,Drawing_start_y;
#pragma optimize("s",on)
int CDrawing::StartCreatingItem(int ItemForm)
{
	Drawing_temp_form=ItemForm;
	Drawing_temp_start=NumItems;	
	int lw=SelectedLineWidth;
	this->m_Color=SelectedLineColor;
	int reserving=1;
	IsSpecialDrawing=0;
	if (ItemForm==1) reserving=4; //rectangle
	if (ItemForm==2) reserving=1; //line
	if (ItemForm==3) reserving=1; //freehand
	if (ItemForm==4) reserving=48; //circle
	if (ItemForm==5) reserving=3;  //arrow;
	if (ItemForm==6) {reserving=4;lw=0;m_Color=0;}  //eraser,large
	if (ItemForm==7) {reserving=4;lw=0;m_Color=0;}  //eraser, small
	if (ItemForm==8) reserving=6;  //coordinate system
	if (ItemForm==9) reserving=6;  //coordinate system, central
	if (ItemForm==10) {reserving=2;Drawing_prev_x=1;Drawing_prev_y=1;}  //raster, linear X, linear Y
	if (ItemForm==11) {reserving=2;Drawing_prev_x=1;Drawing_prev_y=1;}  //raster, log X, linear Y
	if (ItemForm==12) {reserving=2;Drawing_prev_x=1;Drawing_prev_y=1;}  //raster, linear X, log Y
	if (ItemForm==13) {reserving=2;Drawing_prev_x=1;Drawing_prev_y=1;}  //raster, log X, log Y
	if (ItemForm==14) reserving=3; //freehand arrow
	if (ItemForm==15) reserving=5; //5-side polygon
	if (ItemForm==16) reserving=6; //6-side polygon
	if (ItemForm==17) reserving=48; //center-drawn circle
	if (ItemForm==18) {reserving=1;} //horizontal line/section divider
	if (ItemForm==19) {reserving=10;} //parenthese
	if (ItemForm==20) {reserving=4;} //Okay
	if (ItemForm==21) {reserving=8;} //Exclamation mark
	if (ItemForm==22) {reserving=10;} //Question mark
	if (ItemForm==23) {reserving=2;} //X mark
	if (ItemForm==25) {reserving=0;Drawing_start_x=Drawing_start_y=-1;} //diagonals
	if (ItemForm==26) {reserving=0;Drawing_start_x=Drawing_start_y=-1;} //diagonals
	if (ItemForm==27) reserving=6; //center-drawn rectangle
	if (ItemForm==28) reserving=48; //sinus function
	if (ItemForm==29) reserving=24; //x^2 function
	if (ItemForm==30) reserving=3; //triangle 90 degree
	if (ItemForm==31) reserving=3; //triangle - two equal sides
	if (ItemForm==32) reserving=48; //sector of circle
	if (ItemForm==33) reserving=48; //segment of circle

	if (ItemForm==50) //drawing box
	{
		m_Color=0;
		reserving=4;
		IsSpecialDrawing=50;
		SpecialData=new CDrawingBox(this);
	} 
	if (ItemForm==51) //funciton ploter
	{
		m_Color=0;
		reserving=4;
		IsSpecialDrawing=51;
		SpecialData=new CFunctionPlotter(this);
	} 
	if (ItemForm==52) //bitmap image box
	{
		lw=2*DRWZOOM;
		m_Color=0;
		reserving=4;
		IsSpecialDrawing=52;
		SpecialData=new CBitmapImage(this);
	} 

 
	NumItems+=reserving;
	if (NumItems>65000) NumItems=65000;
	if (NumItems>NumItemsReserved) NumItemsReserved=NumItems;
	if (Items)
		Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
	else
		Items=(tDrawingItem*)malloc(NumItemsReserved*sizeof(tDrawingItem));

	tDrawingItem *di;
	for (int i=0;i<reserving;i++)
	{
		di=Items+Drawing_temp_start+i;
		di->Type=1; //Line
		di->X1=0;di->Y1=0;
		di->X2=0;di->Y2=0;
		di->LineWidth=lw;
	}

	if (ItemForm==51)
	{
		CExpression *tmp=new CExpression(NULL,NULL,100);
		tmp->GenerateASCIINumber(0,0,1,0,0);CopyExpressionIntoSubgroup(tmp,0,0,0,0);
		tmp->Delete();tmp->GenerateASCIINumber(100.0,100,1,0,0);CopyExpressionIntoSubgroup(tmp,0,0,0,0);
		tmp->Delete();tmp->GenerateASCIINumber(0,0,1,0,0);CopyExpressionIntoSubgroup(tmp,0,0,0,0);
		tmp->Delete();tmp->GenerateASCIINumber(100.0,100,1,0,0);CopyExpressionIntoSubgroup(tmp,0,0,0,0);
		tmp->Delete();CopyExpressionIntoSubgroup(tmp,0,0,0,0); //function 1
		tmp->Delete();CopyExpressionIntoSubgroup(tmp,0,0,0,0); //function 2
		tmp->Delete();CopyExpressionIntoSubgroup(tmp,0,0,0,0); //function 3
		tmp->Delete();CopyExpressionIntoSubgroup(tmp,0,0,0,0); //function 4
		delete tmp;
	}
	return 1;
}


//called whenever a new coordinate is to be updated (onMouseMove)
#pragma optimize("s",on)
int CDrawing::UpdateCreatingItem(int X, int Y, int absX, int absY)
{

	if ((GetKeyState(VK_SHIFT)&0xFFFE) || (GetKeyState(VK_RBUTTON)&0xFFFE)) //if the SHIFT key is held
	{
		if ((Drawing_temp_form==1) || (Drawing_temp_form==4) ||
			((Drawing_temp_form>=8) && (Drawing_temp_form<=13)) ||
			((Drawing_temp_form>=15) && (Drawing_temp_form<=17)) ||
			((Drawing_temp_form>=20) && (Drawing_temp_form<=23)) ||
			(Drawing_temp_form==27) || (Drawing_temp_form==30))
		{
			if (abs(X)<abs(Y))
			{
				if (Y<0) {Y=-abs(X);absY=-abs(absX);} else {Y=abs(X);absY=abs(absX);}
			}
			else
			{
				if (X<0) {X=-abs(Y);absX=-abs(absY);} else {X=abs(Y);absX=abs(absY);}
			}
		}
		if (Drawing_temp_form==31) //triangle with two equal sides - special handling, make equal side triangle
		{
			if (Y>0) Y=(int)((double)abs(X)*0.866); else Y=-(int)((double)abs(X)*0.866);
		}

		if ((Drawing_temp_form==2) || (Drawing_temp_form==5))
		{
			if (abs(X)<abs(Y))
			{
				if (abs(X)<abs(Y)/2) X=0; else X=abs(Y)*((X<0)?(-1):1);
			}
			else
			{
				if (abs(Y)<abs(X)/2) Y=0; else Y=abs(X)*((Y<0)?(-1):1);
			}
		}
	}

	int dx=absX-X;
	int dy=absY-Y;

	X=X*DRWZOOM/10;
	Y=Y*DRWZOOM/10;
	int oabsX=absX;
	int oabsY=absY;
	absX/=10;
	absY/=10;

	if (GetKeyState(VK_MENU)&0xFFFE)
	{
		if ((SpecialDrawingHover) && 
			(((CDrawing*)(SpecialDrawingHover->Object))->IsSpecialDrawing==50) &&
			(IsDrawingMode!=6) && (IsDrawingMode!=7) && (IsDrawingMode!=25) && (IsDrawingMode!=26))
		{
			if (this->FindNerbyPoint(&absX,&absY,NULL,0,0,0,0))
			{
				dx/=10;
				dy/=10;

				X=(absX-dx)*DRWZOOM;
				Y=(absY-dy)*DRWZOOM;
			}
		}
	}
	

	if ((Drawing_temp_form==1) || (Drawing_temp_form==27)) //Rectangle, center drawn rectangle
	{
		int x1,x2,y1,y2;
		if (Drawing_temp_form==1)
		{
			x1=0;x2=X;if (x2<x1) {x2=0;x1=X;}
			y1=0;y2=Y;if (y2<y1) {y2=0;y1=Y;}
			int len=max(abs(x2-x1),abs(y2-y1));
			int wdth;
			int lww=max(3*DRWZOOM/2,Items->LineWidth);
			if (len<50*lww) wdth=3*lww/2; else wdth=2*lww;
			if ((abs(x2-x1)<wdth) && (abs(y2-y1)>4*abs(x2-x1))) {x2=x1=0;} //line drawing in rectangle mode
			if ((abs(y2-y1)<wdth) && (abs(x2-x1)>4*abs(y2-y1))) {y2=y1=0;} //line drawing in rectangle mode
		}
		else
		{
			x1=-X;x2=X;if (x2<x1) {x2=-X;x1=X;}
			y1=-Y;y2=Y;if (y2<y1) {y2=-Y;y1=Y;}
		}
		tDrawingItem *di=Items+Drawing_temp_start;

		di->X1=x1;di->X2=x2;
		di->Y1=y1;di->Y2=y1;
		di++;
		di->X1=x2;di->X2=x2;
		di->Y1=y1;di->Y2=y2;
		di++;
		di->X1=x2;di->X2=x1;
		di->Y1=y2;di->Y2=y2;
		di++;
		di->X1=x1;di->X2=x1;
		di->Y1=y2;di->Y2=y1;
		di++;
		NumItems=4;
		if ((Drawing_inside_create) && (Drawing_temp_form==27))
		{
			NumItems=6;
			di->X1=-5*DRWZOOM;
			di->X2=5*DRWZOOM;
			di->Y1=0;
			di->Y2=0;
			di++;
			di->Y1=-5*DRWZOOM;
			di->Y2=5*DRWZOOM;
			di->X1=0;
			di->X2=0;
		}
	}

	if (Drawing_temp_form==2) //Line
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		if ((oabsX==-1) && (oabsY==-1) && ((di->X2) || (di->Y2)))
		{
			this->InsertItemAt(this->NumItems);		
			di=Items+this->NumItems-1;
			di->X1=(di-1)->X2;
			di->Y1=(di-1)->Y2;
		}
		di->X2=X;
		di->Y2=Y;
		

		//special corrections because GDI draws wrong lines that are thicker than one pixel
		if (abs(X-di->X1)<min(di->LineWidth,2*DRWZOOM)) di->X2=di->X1;
		if (abs(Y-di->Y1)<min(di->LineWidth,2*DRWZOOM)) di->Y2=di->Y1;

	}
	if ((Drawing_temp_form==3) || (Drawing_temp_form==14))//Freehand line, freehand arrow
	{
		tDrawingItem *di=Items+NumItems-1;
		int x1=di->X1;
		int y1=di->Y1;
		int x2=X;//di->X2;
		int y2=Y;//di->Y2;
		int minlen=(300+ViewZoom)*DRWZOOM;
		if (Drawing_inside_create) minlen=10;
		if ((Drawing_temp_form==14) && (!Drawing_inside_create)) minlen=2000*DRWZOOM;
		int ffactor=1;
		if ((SpecialDrawingHover) && (!Drawing_inside_create))
			ffactor=2;
		if (sqrt((double)(((double)x2-x1)*((double)x2-x1)+((double)y2-y1)*((double)y2-y1)))>minlen/ViewZoom/ffactor)
		{
			int hor_vert_line=0;
			if ((GetKeyState(VK_SHIFT)&0xFFFE) || (GetKeyState(VK_RBUTTON)&0xFFFE)) hor_vert_line=1;

			int addnew=1;
			if (GetTickCount()-Drawing_last_tick<(unsigned int)(64/ffactor))
				addnew=0;
			else
				Drawing_last_tick=GetTickCount();

			if ((addnew) && (hor_vert_line))
			{
				double x2=(Items+NumItems-1)->X1;
				double y2=(Items+NumItems-1)->Y1;
				double a1=atan2((double)Y-y2,(double)X-x2);
				double len=sqrt((x2-(double)X)*(x2-(double)X)+(y2-(double)Y)*(y2-(double)Y));
				double b1=(a1+3.14159)/3.14159*2.0;  //bilo *4
				double b2=(int)(b1+0.5);
				double a2=b2*3.14159/2.0-3.14159; //bilo /4
				int YY=(int)(y2+len*sin(a2));
				int XX=(int)(x2+len*cos(a2));
				POINT p;
				GetCursorPos(&p);
				p.x+=(XX-X)/100*ViewZoom/768;
				p.y+=(YY-Y)/100*ViewZoom/768;
				SetCursorPos(p.x,p.y);
				X=XX;
				Y=YY;
			}

			if (addnew)
			{
				if (((NumItems-Drawing_temp_start>2) && (Drawing_temp_form==3)) ||
					((NumItems-Drawing_temp_start>4) && (Drawing_temp_form==14)))
				{
					double x1=(Items+NumItems-3)->X1;
					double y1=(Items+NumItems-3)->Y1;
					double X=(Items+NumItems-2)->X1;
					double Y=(Items+NumItems-2)->Y1;
					double x2=(Items+NumItems-1)->X1;
					double y2=(Items+NumItems-1)->Y1;
					double l1=sqrt((double)((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)))*1000/DRWZOOM;
					double l2=sqrt((double)((x2-X)*(x2-X)+(y2-Y)*(y2-Y)))*1000/DRWZOOM;
					double l3=sqrt((double)((X-x1)*(X-x1)+(Y-y1)*(Y-y1)))*1000/DRWZOOM;
					double factorf=(l2*l3)/(l2+l3)/l1*3.0;
					if (factorf==0) factorf=0.001;
					double a1=atan2((double)Y-y1,(double)X-x1);
					double a2=atan2((double)y2-Y,(double)x2-X);
					if (abs(a1-a2)>abs(a1+6.28318-a2)) a1+=6.28318;
					if (abs(a2-a1)>abs(a2+6.28318-a1)) a2+=6.28318;
				
					if ((abs(a1-a2)<0.5/ffactor) || 
						((abs(abs(a1-a2)-3.14159)<0.5/ffactor) && (hor_vert_line))) //when drawing horizontal/vertical lines we can 'return' (undraw)
					if (!Drawing_inside_create)
					if (((l2+l3<l1+(double)20.0/factorf/(double)ffactor) && (!hor_vert_line)) || (hor_vert_line) ||
						((l2+l3<l1*1.015) && (hor_vert_line)))
					{
						*(Items+NumItems-2)=*(Items+NumItems-1);
						(Items+NumItems-3)->X2=(Items+NumItems-2)->X1;
						(Items+NumItems-3)->Y2=(Items+NumItems-2)->Y1;
						NumItems--;
					}
				}
				NumItems+=1;
				if (NumItems>65000) NumItems=65000;
				if (NumItems>NumItemsReserved) 
				{	
					NumItemsReserved=NumItems+50;
					Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
				}
				tDrawingItem *ddi=Items+NumItems-1;
				ddi->Type=1;
				ddi->X1=ddi->X2=X;
				ddi->Y1=ddi->Y2=Y;
				ddi->LineWidth=SelectedLineWidth;
				di=Items+NumItems-2;
			}
		}
		di->X2=X;
		di->Y2=Y;
		if (Drawing_temp_form==14)
		{
			double angle=0;

			angle=(double)(di->X2-di->X1)/(double)(di->Y2-di->Y1);
			angle=atan(angle);

			int sign=1;
			int rad=15*DRWZOOM;
			double len=sqrt((double)(di->X1-di->X2)*(double)(di->X1-di->X2)+(double)(di->Y1-di->Y2)*(double)(di->Y1-di->Y2));
			if (di->Y2<di->Y1) sign=-1;
			
			tDrawingItem *di=Items+Drawing_temp_start;
			di->X1=X;
			di->Y1=Y;
			di->X2=(int)(rad*sin(angle-2.7)*sign)+X;
			di->Y2=(int)(rad*cos(angle-2.7)*sign)+Y;
			di++;
			di->X1=X;
			di->Y1=Y;
			di->X2=(int)(rad*sin(angle+2.7)*sign)+X;
			di->Y2=(int)(rad*cos(angle+2.7)*sign)+Y;
		}
	}
	if (Drawing_temp_form==28)  //sinus function 
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		int x1,x2,y1,y2;
		x1=0;x2=X;
		y1=0;y2=Y;
		int negate=1;
		if (x2<x1) {int t=x2;x2=x1;x1=t;}
		if (y2<y1) {int t=y2;y2=y1;y1=t;negate=-1;}
		int rmin=(int)sqrt((double)max(x2-x1,y2-y1)*100)/DRWZOOM;
		if (rmin<6) rmin=6;
		int points=min(32,rmin);
		if ((SpecialDrawingHover) && (!Drawing_inside_create)) {points=3*points/2;if (points<9) points=9;}
		NumItems=Drawing_temp_start+points;
		for (int i=0;i<points;i++)
		{
			di->X1=(int)((x2-x1)*i/points+x1);
			di->Y1=negate*(int)((y2-y1)*sin(2*3.14159*i/points)/2)+y1+(y2-y1)/2;
			di->X2=(int)((x2-x1)*(i+1)/points+x1);
			di->Y2=negate*(int)((y2-y1)*sin(2*3.14159*(i+1)/points)/2)+y1+(y2-y1)/2;
			di++;
		}
	}
	if (Drawing_temp_form==29)  //x^2 function
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		int x1,x2,y1,y2;
		x1=0;x2=X;
		y1=0;y2=Y;
		if (x2<x1) {int t=x2;x2=x1;x1=t;}
		int points=16;
		if ((SpecialDrawingHover) && (!Drawing_inside_create)) points=24;
		NumItems=Drawing_temp_start+points;
		int kk=(points/2);
		for (int i=0;i<points;i++)
		{
			di->X1=(int)((x2-x1)*i/points+x1);
			di->Y1=((int)(((i-kk)*(i-kk))*(y1-y2)/(kk*kk))+y2);
			di->X2=(int)((x2-x1)*(i+1)/points+x1);
			di->Y2=((int)(((i+1-kk)*(i+1-kk))*(y1-y2)/(kk*kk))+y2);
			di++;
		}
	}
	if ((Drawing_temp_form==4) || (Drawing_temp_form==15) || (Drawing_temp_form==16) || (Drawing_temp_form==17)) //Circle, 5-polygon, 6-polygon
	{
		int x1,x2,y1,y2;
		x1=0;x2=X;if (Drawing_temp_form==17) x1=-X;
		y1=0;y2=Y;if (Drawing_temp_form==17) y1=-Y;
		if (x2<x1) {int t=x2;x2=x1;x1=t;}
		if (y2<y1) {int t=y2;y2=y1;y1=t;}
		if (Drawing_temp_form==4)
		{
			int len=max(abs(x2-x1),abs(y2-y1));
			int wdth;
			int lww=max(3*DRWZOOM/2,Items->LineWidth);
			if (len<50*lww) wdth=3*lww/2; else wdth=2*lww;
			if ((abs(x2-x1)<wdth) && (abs(y2-y1)>4*abs(x2-x1))) {x2=x1=0;} //line drawing in rectangle mode
			if ((abs(y2-y1)<wdth) && (abs(x2-x1)>4*abs(y2-y1))) {y2=y1=0;} //line drawing in rectangle mode
		}
		tDrawingItem *di=Items+Drawing_temp_start;

		int rmin=(int)sqrt((double)max(x2-x1,y2-y1)*4*1000/DRWZOOM)/33+1;
		if (rmin<3) rmin=3;
		int points=min(32,rmin);
		if ((SpecialDrawingHover) && (!Drawing_inside_create)) {points=3*points/2;if (points<6) points=6;}
		if (Drawing_temp_form==15) points=5;
		if (Drawing_temp_form==16) points=6;
		NumItems=Drawing_temp_start+points; 
		for (int i=0;i<points;i++)
		{
			di->X1=(int)((x2-x1)*sin(2*3.14159*i/points)/2)+x1+(x2-x1)/2;
			di->Y1=(int)((y2-y1)*cos(2*3.14159*i/points)/2)+y1+(y2-y1)/2;
			di->X2=(int)((x2-x1)*sin(2*3.14159*(i+1)/points)/2)+x1+(x2-x1)/2;
			di->Y2=(int)((y2-y1)*cos(2*3.14159*(i+1)/points)/2)+y1+(y2-y1)/2;
			di++;
		}
		(Items+Drawing_temp_start+points-1)->X2=(Items+Drawing_temp_start)->X1;
		(Items+Drawing_temp_start+points-1)->Y2=(Items+Drawing_temp_start)->Y1;

		if ((Drawing_inside_create) && (points<30) && (Drawing_temp_form==17))
		{
			NumItems+=2;
			di->X1=-5*DRWZOOM;
			di->X2=5*DRWZOOM;
			di->Y1=0;
			di->Y2=0;
			di++;
			di->Y1=-5*DRWZOOM;
			di->Y2=5*DRWZOOM;
			di->X1=0;
			di->X2=0;
		}

	}

	if ((Drawing_temp_form==32)) //sector of circle
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		
		double radius=sqrt((double)X*X+(double)Y*Y);
		int points=min(36,(int)(sqrt(radius)*16/DRWZOOM));
		if ((SpecialDrawingHover) && (!Drawing_inside_create)) {points=3*points/2;if (points<6) points=6;}

		double arc=atan2(-(double)(Y),(double)(X));
		double step=0;
		if (arc>=0)
		{
			points=(int)((double)points*2*fabs(arc)/6.28);
			if (points<4) points=4;
			step=2*arc/points;
			arc=-arc;
		}
		else
		{
			arc=-arc;
			points=(int)((double)points*(6.28-2*fabs(arc))/6.28);
			if (points<4) points=4;
			step=(6.28-2*arc)/points;
		}

		points+=2;
		NumItems=Drawing_temp_start+points; 
		di->Y1=0;
		di->X1=0;
		di->Y2=-(int)(radius*sin(arc));
		di->X2=(int)(radius*cos(arc));
		di++;
		for (int i=0;i<points+1;i++)
		{
			di->Y1=-(int)(radius*sin(arc));
			di->X1=(int)(radius*cos(arc));
			di->Y2=-(int)(radius*sin(arc+step));
			di->X2=(int)(radius*cos(arc+step));
			di++;
			arc+=step;
		}
		(Items+Drawing_temp_start+points-1)->X2=(Items+Drawing_temp_start)->X1;
		(Items+Drawing_temp_start+points-1)->Y2=(Items+Drawing_temp_start)->Y1;
	}

	if ((Drawing_temp_form==33)) //segment of circle
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		int Cx,Cy;
		double radius=abs(Y);
		int points=min(36,(int)(sqrt(radius)*16/DRWZOOM));
		if ((SpecialDrawingHover) && (!Drawing_inside_create)) {points=3*points/2;if (points<6) points=6;}
		double arc=1;
		double step=0;
		{
			Cy=0;
			double t=0;
			if (X>0) 
			{
				Cx=X-(int)radius;t=-Cx; 
				if (t>radius) t=radius;
				if (t<-radius) t=-radius;
				arc=acos(t/radius);
				arc=-arc;
				points=(int)((double)points*fabs(2*arc)/6.28);
				if (points<4) points=4;
				step=2*fabs(arc)/points;

			}
			else 
			{
				Cx=X+(int)radius;t=-Cx;
				if (t>radius) t=radius;
				if (t<-radius) t=-radius;
				arc=acos(t/radius);
				points=(int)((double)points*(6.28-2*fabs(arc))/6.28);
				if (points<4) points=4;
				step=(6.28-2*fabs(arc))/points;

			}

		}

		//points=(int)((double)points*fabs(arc)/6.28);
		//if (points<3) points=3;
		//points=45;
		//double step=2*fabs(arc)/points;

		points+=1;
		NumItems=Drawing_temp_start+points; 
		for (int i=0;i<points+1;i++)
		{
			di->Y1=-(int)(radius*sin(arc))+Cy;
			di->X1=(int)(radius*cos(arc))+Cx;
			di->Y2=-(int)(radius*sin(arc+step))+Cy;
			di->X2=(int)(radius*cos(arc+step))+Cx;
			di++;
			arc+=step;
		}
		(Items+Drawing_temp_start+points-1)->X2=(Items+Drawing_temp_start)->X1;
		(Items+Drawing_temp_start+points-1)->Y2=(Items+Drawing_temp_start)->Y1;
	}

	if (Drawing_temp_form==5) //Arrow
	{
		tDrawingItem *di=Items+Drawing_temp_start;

		di->X2=X;
		di->Y2=Y;
		double angle=0;
		//if (abs(di->X1-di->X2)<abs(di->Y1-di->Y2))
		{
			angle=(double)(di->X2-di->X1)/(double)(di->Y2-di->Y1);
			angle=atan(angle);
		}
		/*else
		{
			angle=(double)(di->Y2-di->Y1)/(double)(di->X2-di->X1);
			angle=-atan(angle)+3.14159/2.0;
		}*/

		int sign=1;
		int rad=15*DRWZOOM;
		double len=sqrt((double)(di->X1-di->X2)*(double)(di->X1-di->X2)+(double)(di->Y1-di->Y2)*(double)(di->Y1-di->Y2));
		if (len<20*DRWZOOM) rad=8*DRWZOOM;
		if (di->Y2<di->Y1) sign=-1;
		di++;
		di->X1=X;
		di->Y1=Y;
		di->X2=(int)(rad*sin(angle-2.7)*sign)+X;
		di->Y2=(int)(rad*cos(angle-2.7)*sign)+Y;
		di++;
		di->X1=X;
		di->Y1=Y;
		di->X2=(int)(rad*sin(angle+2.7)*sign)+X;
		di->Y2=(int)(rad*cos(angle+2.7)*sign)+Y;
	}

	if (Drawing_temp_form==30) //Triangle 90 degree
	{
		tDrawingItem *di=Items+Drawing_temp_start;

		di->X1=0;
		di->Y1=0;
		di->X2=0;
		di->Y2=Y;
		di++;
		di->X1=0;
		di->Y1=Y;
		di->X2=X;
		di->Y2=0;
		di++;
		di->X1=X;
		di->Y1=0;
		di->X2=0;
		di->Y2=0;
		
	}

	if (Drawing_temp_form==31) //Triangle - two equal sides
	{
		tDrawingItem *di=Items+Drawing_temp_start;

		di->X1=0;
		di->Y1=0;
		di->X2=X;
		di->Y2=0;
		di++;
		di->X1=X;
		di->Y1=0;
		di->X2=X/2;
		di->Y2=Y;
		di++;
		di->X1=X/2;
		di->Y1=Y;
		di->X2=0;
		di->Y2=0;		
	}

	if ((Drawing_temp_form==25) || (Drawing_temp_form==26))  //diagonals
	{
		tDrawingItem *di;
		int sz=12;

		int tt;
		int a,b,c;
		double A;
		int use_y=0;

		//working at higher precision (x10)
		absX=oabsX;
		absY=oabsY;

		if (Drawing_start_x==-1) {Drawing_start_x=absX;Drawing_start_y=absY;}
		if (abs(Drawing_start_x-absX)>abs(Drawing_start_y-absY))
		{
			//X is longer
			if (Drawing_start_x>absX) {tt=absX;absX=Drawing_start_x;Drawing_start_x=tt;tt=absY;absY=Drawing_start_y;Drawing_start_y=tt;}
			A=1;
			if (absX!=Drawing_start_x)
				A=((double)absY-(double)Drawing_start_y)/((double)absX-(double)Drawing_start_x);
			a=Drawing_start_x;
			b=absX;
			c=Drawing_start_y;
		}
		else
		{
			//Y is longer
			if (Drawing_start_y>absY) {tt=absX;absX=Drawing_start_x;Drawing_start_x=tt;tt=absY;absY=Drawing_start_y;Drawing_start_y=tt;}
			A=1;
			if (absY!=Drawing_start_y)
				A=((double)absX-(double)Drawing_start_x)/((double)absY-(double)Drawing_start_y);
			a=Drawing_start_y;
			b=absY;
			c=Drawing_start_x;
			use_y=1;
		}

		Drawing_start_x=oabsX;
		Drawing_start_y=oabsY;

		for (int d=a;d<=b;d++)
		{
		
			if (use_y==0)
			{
				absX=d;
				absY=(int)(c+(A*(d-a)+0.5));
			}
			else
			{
				absY=d;
				absX=(int)(c+(A*(d-a)+0.5));
			}

			if (Drawing_temp_form==25)
			{
				tt=abs(((absY-absX))%100);
				if ((tt==1) && (absY>absX)) {absY--;tt=0;}
				if ((tt==1) && (absY<absX)) {absY++;tt=0;}
			}
			else
			{
				tt=abs(((absY+absX))%100);
				if ((tt==1) && (absY+absX>0)) {absY--;tt=0;}
				if ((tt==1) && (absY+absX<0)) {absY++;tt=0;}
			}

			if (tt==0)
			{
				int l22=sz*DRWZOOM;
				int l11=sz*DRWZOOM;
				int sabsX=absX/10;
				int sabsY=absY/10;
				for (int i=0;i<NumDocumentElements;i++)
				{
					tDocumentStruct *ds=TheDocument+i;
					if ((ds->Type==2) && (ds->absolute_X<=sabsX+sz) && (ds->absolute_X+ds->Length>=sabsX-sz)
						&& (ds->absolute_Y<=sabsY+sz) && (ds->absolute_Y+ds->Below>=sabsY-sz)
						&& (ds->MovingDotState!=5) && (ds->Object) && (((CDrawing*)(ds->Object))->IsSpecialDrawing==0))
					{
						int l1,l2;
						((CDrawing*)(ds->Object))->FindDiagonalLength((sabsX-ds->absolute_X)*DRWZOOM,(sabsY-ds->absolute_Y)*DRWZOOM,&l1,&l2,(Drawing_temp_form==25)?1:-1);
						if (l2<l22) l22=l2;
						if (l1<l11) l11=l1;
					}
				}
				NumItems++;
				if (NumItems>65000) NumItems=65000;
				if (NumItems>NumItemsReserved)
				{
					NumItemsReserved=NumItems+20;
					Items=(tDrawingItem*)realloc((void*)Items,NumItemsReserved*sizeof(tDrawingItem));

				}
				int deltaX=absX-oabsX;
				int deltaY=absY-oabsY;
				deltaX=deltaX*DRWZOOM/10;
				deltaY=deltaY*DRWZOOM/10;
				di=Items+NumItems-1;
				di->X1=X+deltaX-l22;
				di->Y1=Y+deltaY-((Drawing_temp_form==25)?l22:-l22);
				di->X2=X+deltaX+l11;
				di->Y2=Y+deltaY+((Drawing_temp_form==25)?l11:-l11);;
				di->Type=1;
				di->LineWidth=4*DRWZOOM/10;
				
				for (int ii=Drawing_temp_start;ii<NumItems;ii++)
				{
					tDrawingItem *di2=Items+ii;
					if (di2!=di)
					{
						if (((abs((di2->X1-di->X1)-(di2->Y1-di->Y1))<ViewZoom*DRWZOOM/100) && (Drawing_temp_form==25)) ||
							((abs((di2->X1-di->X1)+(di2->Y1-di->Y1))<ViewZoom*DRWZOOM/100) && (Drawing_temp_form==26)))//if has the right position/direction/offset
						{
							if ((di2->X1<=di->X1) && (di2->X2>=di->X1))
							{
								if ((di2->X1<=di->X2) && (di2->X2>=di->X2)) 
								{
									//the di2 completely covers di
									di->X1=di2->X1;di->Y1=di2->Y1;
									di->X2=di2->X2;di->Y2=di2->Y2;
									for (int j=ii;j<NumItems;j++) *(Items+j)=*(Items+j+1);
									NumItems--;
									break;
								}
								else
								{
									//the di2 covers only one part of di
									di->X1=di2->X1;
									di->Y1=di2->Y1;
									for (int j=ii;j<NumItems;j++) *(Items+j)=*(Items+j+1);
									NumItems--;
									if (di>di2) di--;
								}
							}
							else if ((di2->X1<=di->X2) && (di2->X2>=di->X2))
							{
									//the di2 covers only other part of di
									di->X2=di2->X2;
									di->Y2=di2->Y2;
									for (int j=ii;j<NumItems;j++) *(Items+j)=*(Items+j+1);
									NumItems--;
									if (di>di2) di--;
							}
						}
					}
				}
			}
		}
	}
	if ((Drawing_temp_form==6) || (Drawing_temp_form==7)) //Eraser, large and small
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		int sz=16*DRWZOOM;
		if (Drawing_temp_form==7) sz=5*DRWZOOM;

		di->X1=X-sz;di->X2=X;
		di->Y1=Y-sz;di->Y2=Y-sz;
		di++;
		di->X1=X;di->X2=X;
		di->Y1=Y-sz;di->Y2=Y;
		di++;
		di->X1=X;di->X2=X-sz;
		di->Y1=Y;di->Y2=Y;
		di++;
		di->X1=X-sz;di->X2=X-sz;
		di->Y1=Y;di->Y2=Y-sz;

		sz/=DRWZOOM;
		if ((absX!=0) && (absY!=0))
		for (int i=0;i<NumDocumentElements;i++)
		{
			tDocumentStruct *ds=TheDocument+i;
			if ((ds->Type==2) && (ds->absolute_X<=absX) && (ds->absolute_X+ds->Length>=absX-sz)
				&& (ds->absolute_Y<=absY) && (ds->absolute_Y+ds->Below>=absY-sz)
				&& (ds->MovingDotState!=5))
			{
				if (ds->Object)
				{
					int ret=((CDrawing*)(ds->Object))->EraseSquare(absX-ds->absolute_X-sz,absY-ds->absolute_Y-sz,absX-ds->absolute_X,absY-ds->absolute_Y,NULL);
					if (ret==2)
					{
						pMainView->DeleteDocumentObject(ds);
						i--;
					}
				}
			}
		}
	}

	if ((Drawing_temp_form==8) || (Drawing_temp_form==9)) //Coordinate system
	{
		tDrawingItem *di=Items+Drawing_temp_start;

		int tt=0x7FFFFFFF;
		if (Drawing_temp_form==9) tt=2;
		di->X1=X/tt;
		di->Y1=0;
		di->X2=X/tt;
		di->Y2=Y;
		if ((Drawing_temp_form==9) && (Y>0))
		{
			di++;
			di->X1=X/tt;
			di->Y1=0;
			di->X2=di->X1+4*DRWZOOM;
			di->Y2=di->Y1+8*DRWZOOM;
			di++;
			di->X1=X/tt;
			di->Y1=0;
			di->X2=di->X1-4*DRWZOOM;
			di->Y2=di->Y1+8*DRWZOOM;
		}
		else
		{
			di++;
			di->X1=X/tt;
			di->Y1=Y;
			di->X2=di->X1+4*DRWZOOM;
			di->Y2=di->Y1+((Y<0)?8*DRWZOOM:-8*DRWZOOM);
			di++;
			di->X1=X/tt;
			di->Y1=Y;
			di->X2=di->X1-4*DRWZOOM;
			di->Y2=di->Y1+((Y<0)?8*DRWZOOM:-8*DRWZOOM);
		}
		di++;
		di->X1=0;
		di->Y1=Y/tt;
		di->X2=X;
		di->Y2=Y/tt;
		if ((Drawing_temp_form==9) && (X<0))
		{
			di++;
			di->X1=0;
			di->Y1=Y/tt;
			di->X2=di->X1-8*DRWZOOM;
			di->Y2=di->Y1+4*DRWZOOM;
			di++;
			di->X1=0;
			di->Y1=Y/tt;
			di->X2=di->X1-8*DRWZOOM;
			di->Y2=di->Y1-4*DRWZOOM;
		}
		else
		{
			di++;
			di->X1=X;
			di->Y1=Y/tt;
			di->X2=di->X1-((X>0)?8*DRWZOOM:-8*DRWZOOM);
			di->Y2=di->Y1+4*DRWZOOM;
			di++;
			di->X1=X;
			di->Y1=Y/tt;
			di->X2=di->X1-((X>0)?8*DRWZOOM:-8*DRWZOOM);
			di->Y2=di->Y1-4*DRWZOOM;
		}
	}
	if ((Drawing_temp_form==20) || (Drawing_temp_form==21) || 
		(Drawing_temp_form==22) || (Drawing_temp_form==23)) //Okay sign, exclamation mark, question mark, star
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		int x1,y1,x2,y2;
		int minsize=8*DRWZOOM;
		if ((Drawing_temp_form==21) || (Drawing_temp_form==22)) minsize=12*DRWZOOM;
		if ((X>-minsize) && (X<minsize)) X=minsize;
		if ((Y>-minsize) && (Y<minsize)) Y=minsize;

		x1=-minsize;x2=X;if (X<-minsize) {x1=X;x2=minsize;}
		y1=-minsize;y2=Y;if (Y<-minsize) {y1=Y;y2=minsize;}
		int w=abs(x2-x1);
		int h=abs(y2-y1);
		//if ((w<16*DRWZOOM) || (h<16*DRWZOOM)) {x1=-16*DRWZOOM;y1=-16*DRWZOOM;x2=0;y2=0;w=16*DRWZOOM;h=16*DRWZOOM;}

		if (abs(x2)>abs(y2)) x2=(x2<0)?-abs(y2):abs(y2);
		if (abs(y2)>abs(x2)) y2=(y2<0)?-abs(x2):abs(x2);
		if (abs(x1)>abs(y1)) x1=(x1<0)?-abs(y1):abs(y1);
		if (abs(y1)>abs(x1)) y1=(y1<0)?-abs(x1):abs(x1);
		w=abs(x2-x1);
		h=abs(y2-y1);


		if (Drawing_temp_form==20)
		{
			int lw=2*DRWZOOM;
			if (h>24*DRWZOOM) lw=3*DRWZOOM;
			if (h>80*DRWZOOM) lw=4*DRWZOOM;
			di->X1=x1;
			di->Y1=y1+(2*h)/3;
			di->X2=x1+(1*w)/3;
			di->Y2=y1+h;
			di->LineWidth=lw;
			di++;
			di->X1=x1+(1*w)/3;
			di->Y1=y1+h;
			di->X2=x2;
			di->Y2=y1;
			di->LineWidth=lw;
			NumItems=Drawing_temp_start+2;
			if (h>40*DRWZOOM)
			{
				NumItems=Drawing_temp_start+4;
				di++;
				di->X1=x1;
				di->Y1=y1+(2*h)/3;
				di->X2=x1+(1*w)/3;
				di->Y2=y1+86*h/100;
				di->LineWidth=lw;
				di++;
				di->X1=x1+(1*w)/3;
				di->Y1=y1+86*h/100;
				di->X2=x2;
				di->Y2=y1;
				di->LineWidth=lw;
			}
		}
		if (Drawing_temp_form==21)
		{
			int lw=2*DRWZOOM;
			if (h>48*DRWZOOM) lw=3*DRWZOOM;
			if (h>100*DRWZOOM) lw=4*DRWZOOM;			
			di->X1=x1+3*w/9;di->Y1=y1;
			di->X2=x1+6*w/9;di->Y2=y1;
			di->LineWidth=lw;
			di++;
			di->X1=x1+6*w/9;di->Y1=y1;
			di->X2=x1+5*w/9;di->Y2=y1+h*7/9;
			di->LineWidth=lw;
			di++;
			di->X1=x1+5*w/9;di->Y1=y1+h*7/9;
			di->X2=x1+4*w/9;di->Y2=y1+h*7/9;
			di->LineWidth=lw;
			di++;
			di->X1=x1+4*w/9;di->Y1=y1+h*7/9;
			di->X2=x1+3*w/9;di->Y2=y1;
			di->LineWidth=lw;
			di++;

			di->X1=x1+w/2;di->Y1=y1+h*15/18;
			di->X2=x1+w*11/18;di->Y2=y1+h*17/18;
			di->LineWidth=lw;
			di++;
			di->X1=x1+w*11/18;di->Y1=y1+h*17/18;
			di->X2=x1+w/2;di->Y2=y1+h*19/18;
			di->LineWidth=lw;
			di++;
			di->X1=x1+w/2;di->Y1=y1+h*19/18;
			di->X2=x1+w*7/18;di->Y2=y1+h*17/18;
			di->LineWidth=lw;
			di++;
			di->X1=x1+w*7/18;di->Y1=y1+h*17/18;
			di->X2=x1+w/2;di->Y2=y1+h*15/18;
			di->LineWidth=lw;
			di++;
		}
		if (Drawing_temp_form==22)
		{
			int lw=2*DRWZOOM;
			if (h>36*DRWZOOM) lw=3*DRWZOOM;
			if (h>80*DRWZOOM) lw=4*DRWZOOM;			
			if (h>120*DRWZOOM) lw=5*DRWZOOM;			
			di->X1=				x1+2*w/8; di->Y1=				y1+1*h/10;
			di->X2=(di+1)->X1=	x1+3*w/8; di->Y2=(di+1)->Y1=	y1+0*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+5*w/8; di->Y2=(di+1)->Y1=	y1+0*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+6*w/8; di->Y2=(di+1)->Y1=	y1+1*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+6*w/8; di->Y2=(di+1)->Y1=	y1+3*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+4*w/8; di->Y2=(di+1)->Y1=	y1+6*h/10;	di->LineWidth=lw; di++;
			di->X2=				x1+4*w/8; di->Y2=				y1+7*h/10;	di->LineWidth=lw; di++;
			/*di->X2=(di+1)->X1=	x1+6*w/8; di->Y2=(di+1)->Y1=	y1+2*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+2*w/8; di->Y2=(di+1)->Y1=	y1+6*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+2*w/8; di->Y2=(di+1)->Y1=	y1+7*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+3*w/8; di->Y2=(di+1)->Y1=	y1+8*h/10;	di->LineWidth=lw; di++;
			di->X2=(di+1)->X1=	x1+5*w/8; di->Y2=(di+1)->Y1=	y1+8*h/10;	di->LineWidth=lw; di++;
			di->X2=			 	x1+6*w/8; di->Y2=		   	y1+7*h/10;	di->LineWidth=lw;     di++;
			*/
			di->X1=x1+w/2;di->Y1=y1+h*16/18;
			di->X2=x1+w*10/18;di->Y2=y1+h*17/18;
			di->LineWidth=lw;
			di++;
			di->X1=x1+w*10/18;di->Y1=y1+h*17/18;
			di->X2=x1+w/2;di->Y2=y1+h*18/18;
			di->LineWidth=lw;
			di++;
			di->X1=x1+w/2;di->Y1=y1+h*18/18;
			di->X2=x1+w*8/18;di->Y2=y1+h*17/18;
			di->LineWidth=lw;
			di++;
			di->X1=x1+w*8/18;di->Y1=y1+h*17/18;
			di->X2=x1+w/2;di->Y2=y1+h*16/18;
			di->LineWidth=lw;
			di++;
		}
		if (Drawing_temp_form==23)
		{
			int lw=2*DRWZOOM;
			if (h>24*DRWZOOM) lw=3*DRWZOOM;
			if (h>80*DRWZOOM) lw=4*DRWZOOM;		

			di->X1=x1; di->Y1=y1;
			di->X2=x2; di->Y2=y2;
			di->LineWidth=lw;
			di++;

			di->X1=x2; di->Y1=y1;
			di->X2=x1; di->Y2=y2;
			di->LineWidth=lw;
		}
	}
	if (Drawing_temp_form==19) //parenthese
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		if (abs(Y)>=abs(X))
		{
			//vertical parenthese
			int c=10; 
			if (abs(Y)>150*DRWZOOM) c=14;
			if (abs(Y)>300*DRWZOOM) c=18;
			if (abs(Y)>600*DRWZOOM) c=24;

			di->X1=0;di->Y1=0;
			di->X2=(di+1)->X1=X/4; di->Y2=(di+1)->Y1=0+Y/c/4; //new
			di++;
			di->X2=(di+1)->X1=X/2; di->Y2=(di+1)->Y1=Y/c; //fixed
			di++;
			di->X2=(di+1)->X1=X/2; di->Y2=(di+1)->Y1=(c/2-1)*Y/c-Y/c/3;  // new
			di++;
			di->X2=(di+1)->X1=X/2+X/4; di->Y2=(di+1)->Y1=(c/2)*Y/c-Y/c/3; //
			di++;
			di->X2=(di+1)->X1=X;   di->Y2=(di+1)->Y1=(c/2)*Y/c; //fixed - centerpoint
			di++;
			di->X2=(di+1)->X1=X/2+X/4; di->Y2=(di+1)->Y1=(c/2)*Y/c+Y/c/3; //new
			di++;
			di->X2=(di+1)->X1=X/2; di->Y2=(di+1)->Y1=(c/2+1)*Y/c+Y/c/3; //fixed
			di++;
			di->X2=(di+1)->X1=X/2; di->Y2=(di+1)->Y1=(c-1)*Y/c; //new
			di++;
			di->X2=(di+1)->X1=X/4; di->Y2=(di+1)->Y1=Y-Y/c/4;
			di++;
			di->X2=0; di->Y2=Y;
		}
		else
		{
			//horizontal parenthese
			int c=10; 
			if (abs(X)>150*DRWZOOM) c=14;
			if (abs(X)>300*DRWZOOM) c=18;
			if (abs(X)>600*DRWZOOM) c=24;

			di->Y1=0;di->X1=0;
			di->Y2=(di+1)->Y1=Y/4; di->X2=(di+1)->X1=X/c/4;
			di++;
			di->Y2=(di+1)->Y1=Y/2; di->X2=(di+1)->X1=X/c;
			di++;
			di->Y2=(di+1)->Y1=Y/2; di->X2=(di+1)->X1=(c/2-1)*X/c-X/c/3;
			di++;
			di->Y2=(di+1)->Y1=Y/2+Y/4; di->X2=(di+1)->X1=(c/2)*X/c-X/c/3;
			di++;
			di->Y2=(di+1)->Y1=Y; di->X2=(di+1)->X1=(c/2)*X/c;
			di++;
			di->Y2=(di+1)->Y1=Y/2+Y/4; di->X2=(di+1)->X1=(c/2)*X/c+X/c/3;
			di++;
			di->Y2=(di+1)->Y1=Y/2; di->X2=(di+1)->X1=(c/2+1)*X/c+X/c/3;
			di++;
			di->Y2=(di+1)->Y1=Y/2; di->X2=(di+1)->X1=(c-1)*X/c;
			di++;
			di->Y2=(di+1)->Y1=Y/4; di->X2=(di+1)->X1=X-X/c/4;
			di++;
			di->Y2=0; di->X2=X;
		}
	}
	if (Drawing_temp_form==18) //horizontal line - section divider
	{
		tDrawingItem *di=Items+Drawing_temp_start;
		int thin=0;
		if ((SpecialDrawingHover) || (GetKeyState(VK_SHIFT)&0xFFFE) || (GetKeyState(VK_RBUTTON)&0xFFFE)) thin=1;

		if ((abs(Y)>abs(X)) && ((abs(Y)>5*DRWZOOM) || (SpecialDrawingHover)))
		{
			di->Y2=Y;
			di->Y1=0;
			di->X1=0;
			di->X2=0;
			
			if (!thin)
			{
				if (abs(di->Y2-di->Y1)<350*DRWZOOM) di->LineWidth=2*DRWZOOM;
				else if (abs(di->Y2-di->Y1)<950*DRWZOOM) di->LineWidth=3*DRWZOOM;
				else di->LineWidth=4*DRWZOOM;
			}
			else
				di->LineWidth=6*DRWZOOM/10;
			if (abs(Y)<3*abs(X)/2)
			{
				if (Y<0) di->X2=(X>0)?-Y:Y; else di->X2=(X>0)?Y:-Y;
			}
		}
		else
		{
			if ((abs(X)<5*DRWZOOM) && (!thin))
			{
				int offset=absX*DRWZOOM-X;
				di->X2=PaperWidth*DRWZOOM-offset-48*DRWZOOM;
				di->X1=15*DRWZOOM+48*DRWZOOM-offset;
				di->Y2=0;
				di->Y1=0;
			}
			else
			{
				di->X2=X;
				di->X1=0;
				di->Y1=0;
				di->Y2=0;
			}
			if (!thin)
			{
				if (abs(di->X2-di->X1)<350*DRWZOOM) di->LineWidth=2*DRWZOOM;
				else if (abs(di->X2-di->X1)<950*DRWZOOM) di->LineWidth=3*DRWZOOM;
				else di->LineWidth=4*DRWZOOM;
			}
			else
				di->LineWidth=6*DRWZOOM/10;
			if (abs(X)<3*abs(Y)/2)
			{
				if (X<0) di->Y2=(Y>0)?-X:X; else di->Y2=(Y>0)?X:-X;
			}
			
		}
	}
	if ((Drawing_temp_form>=10) && (Drawing_temp_form<=13)) //raster, linear and log
	{
		int num_lines_x=Drawing_prev_x;
		int num_lines_y=Drawing_prev_y;
		int min=15*DRWZOOM,max=25*DRWZOOM;
		if ((Drawing_temp_form==11) || (Drawing_temp_form==13)) {min=70*DRWZOOM;max=120*DRWZOOM;}
		if (abs(X)/num_lines_x>max)
		{
			while (abs(X)/num_lines_x>min) num_lines_x++;
		}
		if (abs(X)/num_lines_x<min)
		{
			while (abs(X)/num_lines_x<max) {num_lines_x--;if (num_lines_x<=1) break;}
		}
		if (num_lines_x<1) num_lines_x=1;
		Drawing_prev_x=num_lines_x;
		min=15*DRWZOOM;max=25*DRWZOOM;
		if ((Drawing_temp_form==12) || (Drawing_temp_form==13)) {min=70*DRWZOOM;max=120*DRWZOOM;}
		if (abs(Y)/num_lines_y>max)
		{
			while (abs(Y)/num_lines_y>min) num_lines_y++;
		}
		if (abs(Y)/num_lines_y<min)
		{
			while (abs(Y)/num_lines_y<max) {num_lines_y--;if (num_lines_y<=1) break;}
		}
		if (num_lines_y<1) num_lines_y=1;
		Drawing_prev_y=num_lines_y;		

		NumItems=Drawing_temp_start+num_lines_x+num_lines_y+2;
		if (NumItems>65000) NumItems=65000;
		if ((Drawing_temp_form==11) || (Drawing_temp_form==13)) NumItems+=num_lines_x*8;
		if ((Drawing_temp_form==12) || (Drawing_temp_form==13)) NumItems+=num_lines_y*8;
		if (NumItems>NumItemsReserved)
		{
			NumItemsReserved=NumItems;
			Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
		}

		tDrawingItem *di=Items+Drawing_temp_start;
		int pos=0;
		for (int i=0;i<num_lines_x+1;i++)
		{
			di->LineWidth=DRWZOOM;
			di->pSubdrawing=NULL;
			di->Type=1;
			di->X1=pos;
			di->X2=di->X1;
			di->Y1=0;
			di->Y2=Y;
			di++;
			if ((Drawing_temp_form==11) || (Drawing_temp_form==13))
			if (i!=num_lines_x)
			for (int j=2;j<10;j++)
			{
				int kk=X/num_lines_x;
				di->LineWidth=6*DRWZOOM/10;
				di->pSubdrawing=NULL;
				di->Type=1;
				di->X1=(int)(log10((double)j)*kk+pos);
				if (kk<0)
					di->X1=pos+kk-(int)(log10((double)j)*kk);
				di->X2=di->X1;
				di->Y1=0;
				di->Y2=Y;
				di++;
			}
			pos+=X/num_lines_x;

		}
		pos=0;
		for (int i=0;i<num_lines_y+1;i++)
		{
			di->LineWidth=DRWZOOM;
			di->pSubdrawing=NULL;
			di->Type=1;
			di->Y1=pos;
			di->Y2=di->Y1;
			di->X1=0;
			di->X2=X;
			di++;
			if ((Drawing_temp_form==12) || (Drawing_temp_form==13))
			if (i!=num_lines_y)
			for (int j=2;j<10;j++)
			{
				int kk=Y/num_lines_y;
				di->LineWidth=6*DRWZOOM/10;
				di->pSubdrawing=NULL;
				di->Type=1;
				di->Y1=(int)(log10((double)j)*kk+pos);
				if (kk>0)
					di->Y1=pos+kk-(int)(log10((double)j)*kk);
				di->Y2=di->Y1;
				di->X1=0;
				di->X2=X;
				di++;
			}
			pos+=Y/num_lines_y;
		}
	}

	if ((Drawing_temp_form==50) || (Drawing_temp_form==51) || (Drawing_temp_form==52)) //drawing box, function ploter, bitmap image
	{
		int x1,x2,y1,y2;
		x1=0;x2=X;if (x2<x1) {x2=0;x1=X;}
		y1=0;y2=Y;if (y2<y1) {y2=0;y1=Y;}
		tDrawingItem *di=Items+Drawing_temp_start;

		int wdth=DRWZOOM;
		if (Drawing_temp_form==52) 
			(abs(x1-x2)<30*DRWZOOM)?wdth=DRWZOOM/10:wdth=2*DRWZOOM;
		di->X1=x1;di->X2=x2;
		di->Y1=y1;di->Y2=y1;
		di->LineWidth=wdth;
		di++;
		di->X1=x2;di->X2=x2;
		di->Y1=y1;di->Y2=y2;
		di->LineWidth=wdth;
		di++;
		di->X1=x2;di->X2=x1;
		di->Y1=y2;di->Y2=y2;
		di->LineWidth=wdth;
		di++;
		di->X1=x1;di->X2=x1;
		di->Y1=y2;di->Y2=y1;
		di->LineWidth=wdth;
	}
	return 1;
}

extern int LeftClickTimer;
//the function returns X and Y coordinates of the upper left corner of the item
#pragma optimize("s",on)
int CDrawing::EndCreatingItem(int *X, int *Y, int absX, int absY)
{	
	
	if ((X==NULL) || (Y==NULL)) return 0;
	this->OriginalForm=Drawing_temp_form;
	if (!Drawing_inside_create)
	if ((Drawing_temp_form==3) || (Drawing_temp_form==14)) //freehand drawing, freehand arrow
	{
		int has_any=5;
		while (has_any)
		{
			has_any--;
			int delta=0;if (Drawing_temp_form==14) delta=2;
		}
		for (int ii=0;ii<NumItems;ii++)
		{
			tDrawingItem *di=Items+ii;
			if ((di->X1==di->X2) && (di->Y1==di->Y2))
			{
				for (int iii=ii+1;iii<NumItems-1;iii++)
					*(Items+iii)=*(Items+iii+1);
				NumItems--;
				ii--;
			}
		}
		while(NumItems>1)
		{
			//check if the last segment and semi-last segment can be joined)
			int x1=(Items+NumItems-1)->X2;
			int y1=(Items+NumItems-1)->Y2;
			int x2=(Items+NumItems-1)->X1;
			int y2=(Items+NumItems-1)->Y1;
			int x3=(Items+NumItems-2)->X1;
			int y3=(Items+NumItems-2)->Y1;
			float a1=atan2((float)(y1-y2),(float)(x1-x2));
			float a2=atan2((float)(y1-y3),(float)(y1-x3));
			if (fabs(a1-a2)<0.06)
			{
				(Items+NumItems-2)->Y2=y1;
				(Items+NumItems-2)->X2=x1;
				NumItems--;
			}
			else if ((x1==x2) && (x1==x3))
			{
				(Items+NumItems-2)->Y2=y1;
				NumItems--;
			}
			else if ((y1==y2) && (y1==y3))
			{
				(Items+NumItems-2)->X2=x1;
				NumItems--;
			}
			else if ((abs(y1-y2)<(3*DRWZOOM/2)) && (abs(x1-x2)<(3*DRWZOOM/2)))
			{
				(Items+NumItems-2)->X2=x1;
				NumItems--;
			}
			else
				break;
		}
	}

	if ((Drawing_temp_form==6) || (Drawing_temp_form==7)) //eraser, large and small
	{
		//rearange sizes and positions of all drawing objects (erased objects can be smaller)
		int i;
		for (i=0;i<NumDocumentElements;i++)
		{
			tDocumentStruct *ds=TheDocument+i;

			if ((ds->Type==2) && (ds->Object))
			{
				int x1,y1,w,h;
				((CDrawing*)(ds->Object))->AdjustCoordinates(&x1,&y1,&w,&h);
				ds->absolute_X+=x1;
				ds->absolute_Y+=y1;
				ds->Above=0;
				ds->Length=w;
				ds->Below=h;

			}
		}
	}

	if (Drawing_temp_form==1) //rectangle - line drawing with rectangle tool
	{
		if (Items->X1==(Items+2)->X1)
		{
			Items->Y2=(Items+2)->Y2;
			this->NumItems=1;
		}
		if (Items->Y1==(Items+2)->Y1)
		{
			this->NumItems=1;
		}
	}
	if (Drawing_temp_form==4) //ellipse- line drawing with ellipse tool
	{
		int i;
		int maxy=0xFFFFFFFF;
		int maxx=0xFFFFFFFF;
		int miny=0x7FFFFFFF;
		int minx=0x7FFFFFFF;
		int is_flatx=1;
		int is_flaty=1;
		for (i=0;i<this->NumItems-1;i++)
		{
			tDrawingItem *di1=Items+i;
			tDrawingItem *di2=Items+i+1;
			if (di1->Y1>maxy) maxy=di1->Y1;
			if (di1->Y2>maxy) maxy=di1->Y2;
			if (di2->Y1>maxy) maxy=di2->Y1;
			if (di2->Y2>maxy) maxy=di2->Y2;

			if (di1->Y1<miny) miny=di1->Y1;
			if (di1->Y2<miny) miny=di1->Y2;
			if (di2->Y1<miny) miny=di2->Y1;
			if (di2->Y2<miny) miny=di2->Y2;

			if (di1->X1>maxx) maxx=di1->X1;
			if (di1->X2>maxx) maxx=di1->X2;
			if (di2->X1>maxx) maxx=di2->X1;
			if (di2->X2>maxx) maxx=di2->X2;

			if (di1->X1<minx) minx=di1->X1;
			if (di1->X2<minx) minx=di1->X2;
			if (di2->X1<minx) minx=di2->X1;
			if (di2->X2<minx) minx=di2->X2;

			if (di1->X1!=di2->X2) is_flatx=0;
			if (di1->Y1!=di2->Y2) is_flaty=0;
		}
		if (is_flatx)
		{
			Items->X1=Items->X2=0;
			Items->Y1=miny;
			Items->Y2=maxy;
			this->NumItems=1;
		}
		else if (is_flaty)
		{
			Items->Y1=Items->Y2=0;
			Items->X1=minx;
			Items->X2=maxx;
			this->NumItems=1;
		}
	
	}


	int w,h,x,y,x2,y2;
	AdjustCoordinates(X,Y,&w,&h,absX,absY);


	if (Drawing_temp_form==51) //function plotter
	{
		//check if it is too small;
		if (w<ViewZoom/2) return 0;
		if (h<ViewZoom/2) return 0;

		for (int k=4;k<this->NumItems;k++)
			(Items+k)->X1=(Items+k)->X2=(Items+k)->Y1=(Items+k)->Y2=2*DRWZOOM;

	}

	//checking if the size of object is too small - if yes, we do not create this drawing
	//(the only exeption is if we had a long press (>100ms) with line-drawing tool in what
	// case we will create a single dot)
	FindRealCorner(&x,&y,&x2,&y2);
	if ((x*2>=x2) && (y*2>=y2) && 
		((LeftClickTimer<2) || (this->NumItems>1) || (this->Items->Type!=1) || (this->Items->X1!=this->Items->X2) || (this->Items->Y1!=this->Items->Y2))) return 0;

	//all OK.
	return 1;
}
#pragma optimize("",on)

//empties the drawing
int CDrawing::Delete(void)
{
	if ((IsSpecialDrawing==50) && (SpecialData))
	{
		delete (CDrawingBox*)SpecialData;
	}
	if ((IsSpecialDrawing==51) && (SpecialData))
	{
		delete (CFunctionPlotter*)SpecialData;
	}
	if ((IsSpecialDrawing==52) && (SpecialData))
	{
		delete (CBitmapImage*)SpecialData;
	}
	m_Color=-1;
	IsSpecialDrawing=0;
	IsSelected=0;
	OriginalForm=0;
	//IsNodeEdit=0;
	NodeX=NodeY=-1;
	if (Items)
	{
		for (int i=0;i<NumItems;i++)
		{
			tDrawingItem *di=Items+i;
			if ((di->Type==0) && (di->pSubdrawing))
			{
				//((CDrawing*)(di->pSubdrawing))->Delete();
				delete ((CDrawing*)(di->pSubdrawing));
			}
			if ((di->Type==2) && (di->pSubdrawing))
			{
				//((CExpression*)(di->pSubdrawing))->Delete();
				delete ((CExpression*)(di->pSubdrawing));
			}
		}
		free(Items);
		Items=NULL;
		NumItemsReserved=0;

	}
	NumItems=0;
	return 1;
}

#pragma optimize("s",on)
int CDrawing::InsertEmptyElement(int form, int Cx, int Cy)
{
	Drawing_inside_create=1;
	
	if ((form==1) || (form==2) || (form==4) || (form==5) || (form==8) ||
		(form==9) || (form==10) || (form==11) || (form==12) || (form==13) ||
		(form==15) || (form==16) || (form==17) || (form==19) ||
		(form==20) || (form==21) || (form==22) || (form==23) || 
		(form==27) || (form==28) || (form==29) || (form==30) || (form==31) ||
		(form==32) || (form==33)) //rectangle, line, circle, arrow
	{
		OriginalForm=form; //used from toolbox
		int x,y;
		if (StartCreatingItem(form))
			if (UpdateCreatingItem(Cx*10,Cy*10,0,0))
				if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
	}
	if (form==18) //section line
	{
		OriginalForm=form; //used from toolbox
		int x,y;
		if (StartCreatingItem(form))
			if (UpdateCreatingItem(Cx*10,Cy,0,0))
				if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
	}
	if ((form==3) || (form==14)) //freehand line
	{
		int x,y;
		if (StartCreatingItem(form))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(40,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(60,10,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(80,30,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,40,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(120,60,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(130,80,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(130,100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(120,120,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,140,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(80,160,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(70,180,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(70,200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(80,210,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(90,230,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(130,240,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(150,240,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(170,230,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(180,220,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,220,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(240,200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(290,180,0,0);Drawing_last_tick=0;
			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
			return 1;
		} 
	}
	if (form==6) //eraser, large
	{
		int x,y;
		if (StartCreatingItem(3))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-200,400,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-230,400,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-230,230,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-100,300,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-200,200,0,0);Drawing_last_tick=0;
			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
		}
	}
	if (form==7) //eraser, small
	{
		int x,y;
		if (StartCreatingItem(3))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(250,-250,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(320,-280,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(300,-200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(50,50,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(30,50,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,20,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(50,-50,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,0,0,0);Drawing_last_tick=0;
			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
		}
	}
	if (form==25) //diagonals
	{
		int x,y;
		if (StartCreatingItem(3))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-100,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-50,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(50,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(-50,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(50,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(150,0,0,0);Drawing_last_tick=0;
			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
		}

	}
	if (form==26) //diagonals
	{
		int x,y;
		if (StartCreatingItem(3))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(150,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(50,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(150,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(100,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(250,-100,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(150,0,0,0);Drawing_last_tick=0;
			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
		}
	}
	if (form==50) //drawing box
	{
		int x,y;
		if (StartCreatingItem(3))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,40,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,40,0,0);Drawing_last_tick=0;

			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
		}
	}
	if (form==51) //function ploter
	{
		int x,y;
		if (StartCreatingItem(3))
		{
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,200,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(0,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(10,0,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(10,195,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,195,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(10,195,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(40,170,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(90,160,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(120,140,0,0);Drawing_last_tick=0;
			UpdateCreatingItem(200,20,0,0);Drawing_last_tick=0;
			if (EndCreatingItem(&x,&y)) goto end_EndCreatingItem;
		}
	}
	if (form==52) //bitmap image
	{
		int x,y;
		StartCreatingItem(52);
		{
			UpdateCreatingItem(0,0,0,0);
			UpdateCreatingItem(160,160,0,0);
			EndCreatingItem(&x,&y);
			goto end_EndCreatingItem;
		}
	}
	Drawing_inside_create=0;
	return 0;

end_EndCreatingItem:
	OriginalForm=form;
	Drawing_inside_create=0;
	return 1;
}
#pragma optimize("",on)


int CDrawing::CalculateSize(CDC* DC, short zoom, short *width, short *height)
{
	int i;
	*width=*height=0;
	if (Items==NULL) return 0;

	int wd=0,hg=0;

	tDrawingItem *di=Items;
	for (i=0;i<NumItems;i++,di++)
	{
		//tDrawingItem *di=Items+i;
		if (di->Type==1) //line
		{
			if (wd<(di->X1+di->LineWidth+halfDRWZOOM)) wd=(di->X1+di->LineWidth+halfDRWZOOM);
			if (wd<(di->X2+di->LineWidth+halfDRWZOOM)) wd=(di->X2+di->LineWidth+halfDRWZOOM);
			if (hg<(di->Y1+di->LineWidth+halfDRWZOOM)) hg=(di->Y1+di->LineWidth+halfDRWZOOM);
			if (hg<(di->Y2+di->LineWidth+halfDRWZOOM)) hg=(di->Y2+di->LineWidth+halfDRWZOOM);
		}
		else if ((di->Type==0) && (di->pSubdrawing)) //subdrawing
		{
			short w,h;
			((CDrawing*)(di->pSubdrawing))->CalculateSize(DC,zoom,&w,&h);
			di->X2=di->X1+w*DRWZOOM*100/zoom;
			di->Y2=di->Y1+h*DRWZOOM*100/zoom;
			if (wd<di->X2) wd=di->X2;
			if (hg<di->Y2) hg=di->Y2;
		}
		else if ((di->Type==2) && (di->pSubdrawing))
		{
			short l,a,b;
			((CExpression*)(di->pSubdrawing))->CalculateSize(DC,zoom,&l,&a,&b);
			di->LineWidth=a; //stored for later usage
			di->X2=di->X1+l*DRWZOOM*100/zoom;
			di->Y2=di->Y1+(a+b)*DRWZOOM*100/zoom;
			if (!this->IsSpecialDrawing)
			{
				if (wd<di->X2) wd=di->X2;
				if (hg<di->Y2) hg=di->Y2;
			}
		}
	}

	*width=((wd*zoom)/DRWZOOM+50)/100;
	*height=((hg*zoom)/DRWZOOM+50)/100;
	
	return 1;
}


//this function paints the drawing 
//MUST BE VERY FAST
void CDrawing::PaintDrawing(CDC * DC, short zoom2, short X, short Y,int absX,int absY,RECT *ClipReg,int color)
{
	int no_background=0;
	int zoom=zoom2; //to change from short to int
	if (zoom<0) {zoom=-zoom;no_background=1;}

	if (m_Color!=-1) color=ColorTable[m_Color&0x07];

	if (IsSpecialDrawing)
	{
		NodeX=NodeY=-1; //it is not possible to node-edit special drawings (drawing-boxes or similar)

		if ((IsSpecialDrawing==52) && (SpecialData))
		{
			((CBitmapImage*)SpecialData)->Paint(DC,zoom,X,Y,absX,absY,ClipReg);
			if ((!IsSelected) && (((CBitmapImage*)SpecialData)->editing==0) && (((CBitmapImage*)SpecialData)->Image))
				return; //we do not show the frame around the bitmap when bitmap is loaded
		}

		if ((IsSpecialDrawing==51) && (SpecialData))
			((CFunctionPlotter*)SpecialData)->Paint(DC,zoom,X,Y,absX,absY,ClipReg);

		if ((IsSpecialDrawing==50) && (SpecialData))
			((CDrawingBox*)SpecialData)->Paint(DC,zoom,X,Y,absX,absY,ClipReg,no_background);
	}

	int prevwidth=-3;

	int ShowNodes=0;
	int sz=0;
	if ((IsSelected==1) && (IsSpecialDrawing==0) && ((AllowQuickEditNodes()) || (GetKeyState(VK_CONTROL)&0xFFFE)))
	{
		ShowNodes=1;
		sz=2*MovingDotSize/3+1;
		if (ViewZoom>100) sz+=sz*(ViewZoom-100)/384;
	}

	int i;
	tDrawingItem *di=Items;
	for (i=0;i<NumItems;i++,di++)
	{
		if (di->Type==1) //line
		{
			int width=(int)di->LineWidth;
			int use_shadowed_line=0;
			if (prevwidth!=width)
			{
				prevwidth=width;
				int zwidth=width*zoom;
				if (zwidth<(130*DRWZOOM))
				{
					if (zwidth<(70*DRWZOOM))
						DC->SelectObject(((color==0) && (IsSelected==0))?GrayThin:GetPenFromPool(1,IsSelected,PALE_RGB(color)));
					else
						DC->SelectObject(GetPenFromPool(1,IsSelected,color));
				}
				else
				{
					if (zwidth<(200*DRWZOOM))
					{
						DC->SelectObject((color==0)?GrayFat:GetPenFromPool(2,IsSelected,PALE_RGB(color)));
						use_shadowed_line=1; //this will paint another thin line over the fat gray line to make illusion of pixel-and-half width
					}
					else
						DC->SelectObject(GetPenFromPool(zwidth/100/DRWZOOM,IsSelected,color));
				}
			}

			POINT pp[64];
			if (((m_Color&0xF8)==0x08) && (NumItems<64))
			{
				//painting color-filled shapes
				char is_closed;
				IsOpenPath(0,&is_closed,(LPPOINT)&pp[0]);
				if (is_closed)
				{
					CBrush brsh(PALE_RGB(PALE_RGB(color)));
					HBRUSH brold=(HBRUSH)DC->SelectObject(brsh);
					for (int ii=0;ii<is_closed;ii++)
					{
						pp[ii].x=X+(pp[ii].x*zoom+50*DRWZOOM)/DRWZOOM/100;
						pp[ii].y=Y+(pp[ii].y*zoom+50*DRWZOOM)/DRWZOOM/100;
						i++;
						di++;
					}
					DC->Polygon((LPPOINT)pp,is_closed);
					DC->SelectObject(brold);
					continue;
				}
				else
					m_Color&=0x07;
			}

			int j=0;
			while (1)
			{
				pp[j].x=X+(di->X1*zoom+50*DRWZOOM)/100/DRWZOOM;
				pp[j].y=Y+(di->Y1*zoom+50*DRWZOOM)/100/DRWZOOM;
				j++;
				if (i+j==NumItems) break;
				if (j==63) break;
				tDrawingItem *di2=di+1;
				if ((di2->Type!=di->Type) || (di2->LineWidth!=width) ||
					(di2->X1!=di->X2) || (di2->Y1!=di->Y2)) break;
				di++;
			}

			if ((j==1) && (di->X1==di->X2) && (di->Y1==di->Y2)) //single dot painting
				DC->Ellipse(pp[0].x-1,pp[0].y-1,pp[0].x+1,pp[0].y+1);

			pp[j].x=X+(di->X2*zoom+50*DRWZOOM)/100/DRWZOOM;
			pp[j].y=Y+(di->Y2*zoom+50*DRWZOOM)/100/DRWZOOM;
			
			DC->Polyline((LPPOINT)pp,j+1);
			if (use_shadowed_line)
			{
				DC->SelectObject(GetPenFromPool(1,IsSelected,color));
				DC->Polyline((LPPOINT)pp,j+1);
				prevwidth=-3; //this will force selecting the pen again
			}

			if (ShowNodes)
			{
				COLORREF clr;
				for (int k=0;k<j;k++)
				{
					clr=RGB(160,0,0);
					if (((Items+i+k)->X1==NodeX) && ((Items+i+k)->Y1==NodeY)) clr=RGB(255,0,0);
					DC->FillSolidRect(pp[k].x-sz/2,pp[k].y-sz/2,sz,sz,clr);
				}
				clr=RGB(160,0,0);
				if (((Items+i+j-1)->X2==NodeX) && ((Items+i+j-1)->Y2==NodeY)) clr=RGB(255,0,0);
				DC->FillSolidRect(pp[j].x-sz/2,pp[j].y-sz/2,sz,sz,clr);
			}

			i+=j-1;
		}
		else if (di->pSubdrawing)
		{	
			prevwidth=-3;

			if (di->Type==0) //subdrawing
			{
				((CDrawing*)(di->pSubdrawing))->PaintDrawing(DC,zoom,X+di->X1*zoom/100/DRWZOOM,Y+di->Y1*zoom/100/DRWZOOM,absX+di->X1/DRWZOOM,absY+di->Y1/DRWZOOM,ClipReg,color);
			}
			if ((di->Type==2) && (!IsSpecialDrawing)) //expression 
			{
				//the value of 'above' was stored in LineWidth
				((CExpression*)(di->pSubdrawing))->PaintExpression(DC,zoom,X+di->X1*zoom/100/DRWZOOM,Y+(di->LineWidth)+(di->Y1)*zoom/100/DRWZOOM,ClipReg,color);
			}
		}
	}

	return;
}

void CDrawing::SelectDrawing(char select)
{
	IsSelected=select;
	if (select==0) NodeX=NodeY=-1;

	tDrawingItem *di=Items;
	for (int i=NumItems-1;i>=0;i--,di++)
		if (di->pSubdrawing)
		{
			if (di->Type==0) ((CDrawing*)di->pSubdrawing)->SelectDrawing(select);
			if ((di->Type==2) && (IsSpecialDrawing!=51)) ((CExpression*)di->pSubdrawing)->SelectExpression(select);
		}
}

int CDrawing::CalcChecksum(void)
{
	int ret=NumItems+m_Color*2;
	tDrawingItem *di=Items;
	for (int i=1;i<=NumItems;i++,di++)
	{
		ret+=di->LineWidth+di->Type*128+di->X1*4+di->X2*8+di->Y1*16+di->Y2*32;
		if (di->pSubdrawing)
		{
			if (di->Type==0) ret+=((CDrawing*)(di->pSubdrawing))->CalcChecksum();
			else if (di->Type==2) ret+=((CExpression*)(di->pSubdrawing))->CalcChecksum();
		}
		ret*=i;
	}
	return ret;
}

//copies expression data into windows clipboard
//should be fast
int CDrawing::CopyToWindowsClipboard(void)
{
	if (theApp.m_pMainWnd->OpenClipboard())
	{
		char tmp[64];
		int len=XML_output(tmp,0,1);
		if ((len>0) && (len<1000*DRWZOOM))
		{
			EmptyClipboard();
			HANDLE hmem=GlobalAlloc(GMEM_ZEROINIT,len+256);
			LPVOID pntr=GlobalLock(hmem);
			int *control=(int*)pntr;
			int *checksum=control+1;
			int *X=checksum+1;
			int *Y=X+1;
			char *data=(char*)(Y+1);
			*control=0xAABBCC22;
			LastTakenChecksum=*checksum=CalcChecksum()+rand();
			*X=MovingStartX;
			*Y=MovingStartY;

			XML_output(data,0,0);
			GlobalUnlock(hmem);
			UINT format=RegisterClipboardFormat("MATHOMIR_EXPR");
			HANDLE ret=SetClipboardData(format,hmem);
			CloseClipboard();
		}
	}
	return 0;
}

int CDrawing::CopyDrawing(CDrawing* Original)
{
	Delete();
	Items=(tDrawingItem*)malloc(sizeof(tDrawingItem)*Original->NumItems);
	IsSpecialDrawing=Original->IsSpecialDrawing;
	m_Color=Original->m_Color;
	OriginalForm=Original->OriginalForm;
	if (IsSpecialDrawing)
	{
		if (IsSpecialDrawing==50)
		{
			SpecialData=new CDrawingBox(this);
			((CDrawingBox*)SpecialData)->CopyFrom(Original);
		}
		if (IsSpecialDrawing==51) 
		{
			SpecialData=new CFunctionPlotter(this);
			((CFunctionPlotter*)SpecialData)->CopyFrom(Original);
		}
		if (IsSpecialDrawing==52) 
		{
			SpecialData=new CBitmapImage(this);
			((CBitmapImage*)SpecialData)->CopyFrom(Original);
		}
	}
	if (Items)
	{
		NumItemsReserved=Original->NumItems;
		NumItems=Original->NumItems;
		for (int i=0;i<NumItems;i++)
		{
			tDrawingItem *di=Items+i;
			*di=*(Original->Items+i);
			if ((di->Type==0) && (di->pSubdrawing)) //subdrawing
			{
				CDrawing *tmp=new CDrawing();
				tmp->CopyDrawing((CDrawing*)di->pSubdrawing);
				di->pSubdrawing=(void*)tmp;
			}
			if ((di->Type==2) && (di->pSubdrawing))
			{
				CExpression *tmp=new CExpression(NULL,NULL,((CExpression*)(di->pSubdrawing))->m_FontSize);
				//tmp->m_FontSizeHQ=((CExpression*)(di->pSubdrawing))->m_FontSizeHQ;
				tmp->CopyExpression((CExpression*)di->pSubdrawing,0);
				di->pSubdrawing=(void*)tmp;
			}
		}
	}
	//special handling - if copied into clipboard, copy also into windows clipboard
	if (this==ClipboardDrawing)
	{
		CopyToWindowsClipboard();
	}
	return 0;
}

CObject* CDrawing::SelectObjectAtPoint(CDC* DC, short zoom, short X, short Y, int *NodeEdit,int internal_call)
{
	NodeX=NodeY=-1;
	*NodeEdit=0;
	CObject *ret=NULL;

	//we are going to ignore request for function plotter as much as possible (while it is drawing) because this lowers posibility of crash!!
	if ((IsSpecialDrawing==51) && (DrawingThreadUsed) && (SpecialDrawingHover))  return NULL;

	//special handling for bitmap (clicking at interior)
	if ((IsSpecialDrawing==52) && (!internal_call)) 
	{
		if ((X<((CBitmapImage*)SpecialData)->MenuX+45) && (X>3*ViewZoom/100) && (Y>3*ViewZoom/100))
		{
			if ((Y-((CBitmapImage*)SpecialData)->MenuY)<36) *NodeEdit=0xFFFF8000; //magic value returned - marks that mouse is pointing at a special drawing interrior
		}
		((CBitmapImage*)SpecialData)->ShowMenu=1;
		SelectDrawing(1);
		return (CObject*)this;
	}

	int sz;
	if (/*(IsNodeEdit) ||*/	(AllowQuickEditNodes()) || ((GetKeyState(VK_CONTROL)&0xFFFE))) 
	{
		sz=MovingDotSize/3+1; 
		if (ViewZoom>100) sz+=sz*(ViewZoom-100)/384;
	}
	else
		sz=0;

	tDrawingItem *di=Items+NumItems-1;
	for (int i=NumItems-1;i>=0;i--,di--)
	{
		if (di->Type==1)  //line
		{
			int x1=((di->X1)*zoom)/100/DRWZOOM;
			int x2=((di->X2)*zoom)/100/DRWZOOM;
			int y1=((di->Y1)*zoom)/100/DRWZOOM;
			int y2=((di->Y2)*zoom)/100/DRWZOOM;
			
			if (sz)	//If node-editing is active - check if mouse is pointing at a node
			{
				if ((x1-sz<X) && (x1+sz>X) && (y1-sz<Y) && (y1+sz>Y))
				{
					NodeX=di->X1;
					NodeY=di->Y1;
					*NodeEdit=NodeX+NodeY;
					ret=(CObject*)this;
					break;
				}
				if ((x2-sz<X) && (x2+sz>X) && (y2-sz<Y) && (y2+sz>Y))
				{
					NodeX=di->X2;
					NodeY=di->Y2;
					*NodeEdit=NodeX+NodeY;
					ret=(CObject*)this;
					break;
				}
			}
			double l1=sqrt((double)(((double)x2-x1)*(x2-x1)+((double)y2-y1)*(y2-y1)));
			double l2=sqrt((double)(((double)x2-X)*(x2-X)+((double)y2-Y)*(y2-Y)));
			double l3=sqrt((double)(((double)X-x1)*(X-x1)+((double)Y-y1)*(Y-y1)));
			double factorf=(l2*l3)/(l2+l3);
			if (factorf==0) factorf=0.001;
			factorf/=((double)max(di->LineWidth*ViewZoom/100,34*DRWZOOM/10))/2.0/(double)DRWZOOM;
			if (l2+l3<l1+(double)zoom/100.0/factorf) 
			{
				ret=(CObject*)this;
				break;
			}
		}
		else if (di->pSubdrawing)
		{
			if (di->Type==0)
			{
				int ne;
				CObject *tmpret;
				tmpret=((CDrawing*)(di->pSubdrawing))->SelectObjectAtPoint(DC,zoom,X-(di->X1)*zoom/100/DRWZOOM,Y-(di->Y1)*zoom/100/DRWZOOM,&ne,1);
				if (ne) *NodeEdit=ne;
				if (tmpret)	{ret=tmpret;break;}
			}
			if ((di->Type==2) && (!IsSpecialDrawing)) //expression
			{
				if ((di->X1<X*100*DRWZOOM/zoom) && (di->X2>X*100*DRWZOOM/zoom) && 
					(di->Y1<Y*100*DRWZOOM/zoom) && (di->Y2>Y*100*DRWZOOM/zoom))
					{ret=(CObject*)di->pSubdrawing;break;}
			}
		}
	}

	if ((IsSpecialDrawing) && (ret==NULL) && (internal_call==0))
	{		
		*NodeEdit=0xFFFF8000; //magic value returned - marks that mouse is pointing at a special drawing interrior
		return NULL;	
	}

	if (ret) SelectDrawing(1);
	return ret;
}

#pragma optimize("s",on)
int CDrawing::XML_output(char * output, int num_tabs, char only_calculate)
{
	int len=0;
	static char tmpstr[256];

	if (num_tabs>16) num_tabs=16;

	int i;
	tDrawingItem *di=Items;
	for (i=0;i<NumItems;i++,di++)
	{
		if (!only_calculate) {memset(output,9,num_tabs);output+=num_tabs;}
		len+=num_tabs;

		if (di->Type==0) //subdrawing
		{
			if (XMLFileVersion==1)
				sprintf(tmpstr,"<group X1=\"%d\" Y1=\"%d\" X2=\"%d\" Y2=\"%d\">\r\n",  //instead of 'gr' it was 'group' in old version
					di->X1*1000/DRWZOOM,di->Y1*1000/DRWZOOM,di->X2*1000/DRWZOOM,di->Y2*1000/DRWZOOM);
			else
				sprintf(tmpstr,"<gr d=\"%d,%d;%d,%d\">\r\n",  //instead of 'gr' it was 'group' in old version
					di->X1,di->Y1,di->X2,di->Y2);
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
			int tt=((CDrawing*)(di->pSubdrawing))->XML_output(output,num_tabs+1,only_calculate);
			len+=tt;
			if (!only_calculate) output+=tt;
			memset(tmpstr,9,num_tabs);tmpstr[num_tabs]=0;
			if (XMLFileVersion==1) strcat(tmpstr,"</group>\r\n"); else strcat(tmpstr,"</gr>\r\n"); //instead of /gr it was /group in old version
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
		}
		else if (di->Type==2) //subexpression
		{
			if (XMLFileVersion==1)
				sprintf(tmpstr,"<subexp X1=\"%d\" Y1=\"%d\" X2=\"%d\" Y2=\"%d\">\r\n",
					di->X1*1000/DRWZOOM,di->Y1*1000/DRWZOOM,di->X2*1000/DRWZOOM,di->Y2*1000/DRWZOOM);
			else
				sprintf(tmpstr,"<subexp d=\"%d,%d;%d,%d\">\r\n",
					di->X1,di->Y1,di->X2,di->Y2);
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
			int tt=((CExpression*)(di->pSubdrawing))->XML_output(output,num_tabs+1,only_calculate);
			len+=tt;
			if (!only_calculate) output+=tt;
			memset(tmpstr,9,num_tabs);tmpstr[num_tabs]=0;
			strcat(tmpstr,"</subexp>\r\n");
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
		}
		else if (di->Type==1) //line segment
		{
			int typecode=di->Type+10*(m_Color+1);
			if (XMLFileVersion==1) 
				sprintf(tmpstr,"<draw type=\"%d\" ",typecode); 
			else 
			{
				if ((typecode!=11) && (typecode!=1)) //black color (m_Color=-1 or m_Color=0) line segment
					sprintf(tmpstr,"<dw t=\"%d\" ",typecode); 
				else
					strcpy(tmpstr,"<dw ");
			}
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}

			if ((IsSpecialDrawing) && (i==0))
			{
				sprintf(tmpstr,"spec=\"%d\" ",IsSpecialDrawing);
				len+=(int)strlen(tmpstr);
				if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
			}


			if (XMLFileVersion==1)
			{
				sprintf(tmpstr,"width=\"%d\" X1=\"%ld\" Y1=\"%ld\" X2=\"%ld\" Y2=\"%ld\" ",  //instead of 'w=' it was 'width=' in older version
					di->LineWidth*1000/DRWZOOM,di->X1*1000/DRWZOOM,di->Y1*1000/DRWZOOM,di->X2*1000/DRWZOOM,di->Y2*1000/DRWZOOM);
			}
			else
			{
				sprintf(tmpstr,"d=\"%d|%ld,%ld;%ld,%ld",di->LineWidth,di->X1,di->Y1,di->X2,di->Y2);
			}

			int jj=3;
			while ((i<NumItems-1) && (di->LineWidth==(di+1)->LineWidth) &&
				(di->Type==(di+1)->Type) &&
				(di->X2==(di+1)->X1) &&
				(di->Y2==(di+1)->Y1))
			{
				char fstr[64];
				i++;
				di++;
				if (XMLFileVersion==1)
					sprintf(fstr,"X%d=\"%d\" Y%d=\"%d\" ",jj,di->X2*1000/DRWZOOM,jj,di->Y2*1000/DRWZOOM);
				else
				{
					if (di->X2==(di-1)->X2) sprintf(fstr,";:,%d",di->Y2);
					else if (di->Y2==(di-1)->Y2) sprintf(fstr,";%d,:",di->X2);
					else sprintf(fstr,";%d,%d",di->X2,di->Y2);
				}
				strcat(tmpstr,fstr);
				jj++;
				if (jj>18) break;
				if ((XMLFileVersion==1) && (jj>9)) break;
			}
			if (XMLFileVersion==1) strcat(tmpstr,"/>\r\n"); else  strcat(tmpstr,"\" />\r\n");
			

			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
		}
	}

	//handling special drawings

	int datalen=0;

	if (IsSpecialDrawing==52)
		datalen=((CBitmapImage*)SpecialData)->XML_output(output,num_tabs,only_calculate);

	if (IsSpecialDrawing==51)
		datalen=((CFunctionPlotter*)SpecialData)->XML_output(output,num_tabs,only_calculate);

	if (IsSpecialDrawing==50)
		datalen=((CDrawingBox*)SpecialData)->XML_output(output,num_tabs,only_calculate);
	
	len+=datalen;
	if (!only_calculate) output+=datalen;

	return len;
}

#pragma optimize("s",on)
char *CDrawing::XML_input(char * file)
{
	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;

	while (1)
	{
		file=mf->XML_search("",file); //find anything
		if (file==NULL) return NULL;

		if (file[0]=='/')
		{
			if (strncmp(file,"/obj",4)==0) return file+4;
			if (strncmp(file,"/o",2)==0) return file+2;
			if (strncmp(file,"/group",6)==0) return file+6;
			if (strncmp(file,"/gr",3)==0) return file+2;
			if (strncmp(file,"/subexp",7)==0) {file+=7;continue;}
		}
		if (this->IsSpecialDrawing==52)
		{
			char *file2=((CBitmapImage*)SpecialData)->XML_input(file);
			if (!file2) return NULL;
			if (file2!=file) {file=file2;continue;}
		}
		if (this->IsSpecialDrawing==51)
		{
			char *file2=((CFunctionPlotter*)SpecialData)->XML_input(file);
			if (!file2) return NULL;
			if (file2!=file) {file=file2;continue;}
		}
		if (this->IsSpecialDrawing==50)
		{
			char *file2=((CDrawingBox*)SpecialData)->XML_input(file);
			if (!file2) return NULL;
			if (file2!=file) {file=file2;continue;}
		}
		
		if ((strncmp(file,"group",5)==0) ||
			(strncmp(file,"gr",2)==0) ||
			(strncmp(file,"subexp",6)==0))
		{
			NumItems++;
			if (NumItems>65000) NumItems=65000;
			if (NumItems>NumItemsReserved) NumItemsReserved=NumItems;
			if (Items)
				Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
			else
				Items=(tDrawingItem*)malloc(NumItemsReserved*sizeof(tDrawingItem));
			tDrawingItem *di=Items+NumItems-1;
			di->LineWidth=0;

			if (file[0]=='s')
			{
				di->Type=2;
				di->pSubdrawing=(void *)(new CExpression(NULL,NULL,100));
				file+=6;
			}
			else
			{
				di->Type=0;
				di->pSubdrawing=(void *)(new CDrawing());
				if (file[2]=='o') file+=5; else file+=2;
			}
			static char attribute[64];
			static char value[256];
			do
			{
				file=mf->XML_read_attribute(attribute,value,file,256);
				if (file==NULL) return NULL;
				int ttt=atoi(value);
				if (strcmp(attribute,"X1")==0) {di->X1=ttt*DRWZOOM/1000;}
				if (strcmp(attribute,"Y1")==0) {di->Y1=ttt*DRWZOOM/1000;}
				if (strcmp(attribute,"X2")==0) {di->X2=ttt*DRWZOOM/1000;}
				if (strcmp(attribute,"Y2")==0) {di->Y2=ttt*DRWZOOM/1000;}
				if (strcmp(attribute,"d")==0) //data is stored as: X1,Y2;X2,Y2
				{
					int parsepos=0,parseelm=0;char ch=',';
					while (ch)
					{
						if (((ch<'0') || (ch>'9')) && (ch!='-'))
						{
							ttt=atoi(value+parsepos);
							if (parseelm==0) di->X1=ttt;
							if (parseelm==1) di->Y1=ttt;
							if (parseelm==2) di->X2=ttt;
							if (parseelm==3) di->Y2=ttt;
							parseelm++;
						}
						ch=*(value+(parsepos++));
					}
				}
			} while (attribute[0]);

			if (di->Type==0)
				file=((CDrawing*)(di->pSubdrawing))->XML_input(file);
			else
				file=((CExpression*)(di->pSubdrawing))->XML_input(file);

			continue;
		}
		
		if ((strncmp(file,"dw",2)!=0) &&
			(strncmp(file,"draw",4)!=0))
			continue; //we expect dwg or draw (draw is to support old file version)

		//'dwg' has been found
		if (file[1]=='r') file+=4; else file+=2;

		NumItems++;
		if (NumItems>65000) NumItems=65000;
		if (NumItems>NumItemsReserved) NumItemsReserved=NumItems;
		if (Items)
			Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
		else
			Items=(tDrawingItem*)malloc(NumItemsReserved*sizeof(tDrawingItem));
		tDrawingItem *di=Items+NumItems-1;
		di->pSubdrawing=NULL;
		di->Type=1;
			
		static char attribute[64];
		static char value[256];

		do
		{
			file=mf->XML_read_attribute(attribute,value,file,256);
			if (file==NULL) return NULL;
			int ttt=atoi(value);
			if ((strcmp(attribute,"t")==0) || (strcmp(attribute,"type")==0)) {int val=atoi(value);di->Type=val%10;if (val>=10) m_Color=val/10-1;}
			if (strcmp(attribute,"X1")==0) {di->X1=ttt*DRWZOOM/1000; }
			if (strcmp(attribute,"Y1")==0) {di->Y1=ttt*DRWZOOM/1000; }
			if (strcmp(attribute,"X2")==0) {di->X2=ttt*DRWZOOM/1000; }
			if (strcmp(attribute,"Y2")==0) {di->Y2=ttt*DRWZOOM/1000; }
			if ((strcmp(attribute,"X3")==0) || 
				(strcmp(attribute,"X4")==0) ||
				(strcmp(attribute,"X5")==0) ||
				(strcmp(attribute,"X6")==0) ||
				(strcmp(attribute,"X7")==0) ||
				(strcmp(attribute,"X8")==0) ||
				(strcmp(attribute,"X9")==0))
			{
				NumItems++;
				if (NumItems>65000) NumItems=65000;
				if (NumItems>NumItemsReserved) NumItemsReserved=NumItems;
				if (Items)
					Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
				else
					Items=(tDrawingItem*)malloc(NumItemsReserved*sizeof(tDrawingItem));
				di=Items+NumItems-1;
				di->LineWidth=(di-1)->LineWidth;
				di->pSubdrawing=NULL;
				di->Type=(di-1)->Type;
				di->X1=(di-1)->X2;
				di->Y1=(di-1)->Y2;
				di->X2=ttt*DRWZOOM/1000;
			}
			if ((strcmp(attribute,"Y3")==0) || 
				(strcmp(attribute,"Y4")==0) ||
				(strcmp(attribute,"Y5")==0) ||
				(strcmp(attribute,"Y6")==0) ||
				(strcmp(attribute,"Y7")==0) ||
				(strcmp(attribute,"Y8")==0) ||
				(strcmp(attribute,"Y9")==0))
			{
				di->Y2=ttt*DRWZOOM/1000;
			}
			if (strcmp(attribute,"width")==0)
			{
				di->LineWidth=ttt*DRWZOOM/1000;
			}
			if (strcmp(attribute,"spec")==0) 
			{
				IsSpecialDrawing=ttt;
				if (IsSpecialDrawing==50)
				{
					SpecialData=new CDrawingBox(this);
				}
				if (IsSpecialDrawing==51)
				{
					SpecialData=new CFunctionPlotter(this);
				}
				if (IsSpecialDrawing==52)
				{
					SpecialData=new CBitmapImage(this);
				}
			}
			if (strcmp(attribute,"d")==0) //data is given as: width|X1,Y1;X2,Y2;X3,Y3;X4,Y4...
			{
				int parsepos=0,parseelm=0;char ch=',';
				while (ch)
				{
					if (((ch<'0') || (ch>':')) && (ch!='-'))
					{
						ttt=atoi(value+parsepos);
						if (parseelm==0) di->LineWidth=ttt;
						if (parseelm==1) di->X1=ttt;
						if (parseelm==2) di->Y1=ttt;
						if (parseelm==3) di->X2=ttt;
						if (parseelm==4) di->Y2=ttt;
						if (parseelm>4)
						{
							if (parseelm&0x01) //creating the new line segment item
							{
								NumItems++;
								if (NumItems>65000) NumItems=65000;
								if (NumItems>NumItemsReserved) NumItemsReserved=NumItems;
								if (Items)
									Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
								else
									Items=(tDrawingItem*)malloc(NumItemsReserved*sizeof(tDrawingItem));
								di=Items+NumItems-1;

								if (*(value+parsepos)==':')
									ttt=(di-1)->X2; //colon means that the value is the same as in item before
								di->LineWidth=(di-1)->LineWidth;
								di->pSubdrawing=NULL;
								di->Type=(di-1)->Type;
								di->X1=(di-1)->X2;
								di->Y1=(di-1)->Y2;
								di->X2=ttt;
							}
							else
							{
								if (*(value+parsepos)==':') 
									ttt=(di-1)->Y2; //colon means that the value is the same as in item before
								di->Y2=ttt;
							}
						}
						parseelm++;
					}
					ch=*(value+(parsepos++));
				}
			}
		} while (attribute[0]);
	}

	return NULL;
}
#pragma optimize("",on)

// Erases the square drawing part
int CDrawing::EraseSquare(int X1, int Y1, int X2, int Y2, CDrawing *parent)
{
	if (IsSpecialDrawing) return 0;

	int is_touched=0;
	X1*=DRWZOOM;
	Y1*=DRWZOOM;
	X2*=DRWZOOM;
	Y2*=DRWZOOM;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;

		if (di->Type==2) continue; //subexpression

		if (di->Type==0) //the subdrawing
		{
			if ((X2>di->X1) && (X1<di->X2) && (Y2>di->Y1) && (Y1<di->Y2))
			{
				int ret=((CDrawing*)(di->pSubdrawing))->EraseSquare((X1-di->X1)/DRWZOOM,(Y1-di->Y1)/DRWZOOM,(X2-di->X1)/DRWZOOM,(Y2-di->Y1)/DRWZOOM,this);
				if (ret==2) //should delete the subdrawing
				{
					//((CDrawing*)(di->pSubdrawing))->Delete();
					delete ((CDrawing*)(di->pSubdrawing));
					for (int j=i;j<NumItems-1;j++)
						*(Items+j)=*(Items+j+1);
					NumItems--;
					i--;
					this->OriginalForm=0;
				}
			}
			continue;
		}

		if ((di->X1>=X1) && (di->X2>=X1) && (di->X1<=X2) && (di->X2<=X2) &&
			(di->Y1>=Y1) && (di->Y2>=Y1) && (di->Y1<=Y2) && (di->Y2<=Y2))
		{
			//the whole line should be erased
			for (int j=i;j<NumItems-1;j++)
				*(Items+j)=*(Items+j+1);
			NumItems--;
			i--;
			this->OriginalForm=0;
			is_touched=1;
			continue;
		}

		int x1,y1,x2,y2,x3,y3,x4,y4;
		int a,b,c,d;
		//find crosspoints with edges of the erasing rectangle
		a=FindCrosspointY(di,Y1,X1,X2,&x1,&y1);
		b=FindCrosspointX(di,X2,Y1,Y2,&x2,&y2);
		c=FindCrosspointY(di,Y2,X1,X2,&x3,&y3);
		d=FindCrosspointX(di,X1,Y1,Y2,&x4,&y4);
		if ((a) || (b) || (c) || (d)) is_touched=1;

		if ((a) && (b))  //crossing upper and right edges
		{
			if (InsertItemAt(i+1)==0) return 1;
			di=Items+i;
			if ((di->X1<di->X2) || (di->Y1<di->Y2))
			{
				di->X2=x1;di->Y2=y1;
				di++;
				di->X1=x2;di->Y1=y2;
			}
			else
			{
				di->X2=x2;di->Y2=y2;
				di++;
				di->X1=x1;di->Y1=y1;
			}
			BreakApart(di,parent);
			i++;
		} 
		else if ((a) && (c))  //crossing upper and bottom edges
		{
			if (InsertItemAt(i+1)==0) return 1;
			di=Items+i;
			if (di->Y1<di->Y2)
			{
				di->X2=x1;di->Y2=y1;
				di++;
				di->X1=x3;di->Y1=y3;
			}
			else
			{
				di->X2=x3;di->Y2=y3;
				di++;
				di->X1=x1;di->Y1=y1;
			}
			BreakApart(di,parent);
			i++;
		} 
		else if ((a) && (d))  //crossing upper and left edges
		{
			if (InsertItemAt(i+1)==0) return 1;
			di=Items+i;
			if ((di->X1<di->X2) || (di->Y1>di->Y2))
			{
				di->X2=x4;di->Y2=y4;
				di++;
				di->X1=x1;di->Y1=y1;
			}
			else
			{
				di->X2=x1;di->Y2=y1;
				di++;
				di->X1=x4;di->Y1=y4;
			}
			BreakApart(di,parent);
			i++;
		} 
		else if ((b) && (c))  //crossing left and bottom edges
		{
			if (InsertItemAt(i+1)==0) return 1;
			di=Items+i;
			if ((di->X1<di->X2) || (di->Y1>di->Y2))
			{
				di->X2=x3;di->Y2=y3;
				di++;
				di->X1=x2;di->Y1=y2;
			}
			else
			{
				di->X2=x2;di->Y2=y2;
				di++;
				di->X1=x3;di->Y1=y3;
			}
			BreakApart(di,parent);
			i++;
		} 
		else if ((b) && (d))  //crossing left and right edges
		{
			if (InsertItemAt(i+1)==0) return 1;
			di=Items+i;
			if (di->X1<di->X2)
			{
				di->X2=x4;di->Y2=y4;
				di++;
				di->X1=x2;di->Y1=y2;
			}
			else
			{
				di->X2=x2;di->Y2=y2;
				di++;
				di->X1=x4;di->Y1=y4;
			}
			BreakApart(di,parent);
			i++;
		} 
		else if ((c) && (d))  //crossing bottom and left
		{
			if (InsertItemAt(i+1)==0) return 1;
			di=Items+i;
			if ((di->X1<di->X2) || (di->Y1<di->Y2))
			{
				di->X2=x4;di->Y2=y4;
				di++;
				di->X1=x3;di->Y1=y3;
			}
			else
			{
				di->X2=x3;di->Y2=y3;
				di++;
				di->X1=x4;di->Y1=y4;
			}
			BreakApart(di,parent);
			i++;
		} 
		else if (a)
		{
			if (di->Y1<di->Y2)
			{
				di->X2=x1;
				di->Y2=y1;
			}
			else
			{
				di->X1=x1;
				di->Y1=y1;
			}
			BreakApart(di,parent);
		}
		else if (b)
		{
			if (di->X1<di->X2)
			{
				di->X1=x2;
				di->Y1=y2;
			}
			else
			{
				di->X2=x2;
				di->Y2=y2;
			}
			BreakApart(di,parent);
		}
		else if (c)
		{
			if (di->Y1<di->Y2)
			{
				di->X1=x3;
				di->Y1=y3;
			}
			else
			{
				di->X2=x3;
				di->Y2=y3;
			}
			BreakApart(di,parent);
		}
		else if (d)
		{
			if (di->X1<di->X2)
			{
				di->X2=x4;
				di->Y2=y4;
			}
			else
			{
				di->X1=x4;
				di->Y1=y4;
			}
			BreakApart(di,parent);
		}
	}

	if (NumItems==0) return 2; //scheduled for deletion
	return is_touched;
}

// finds crospoint of an drawing item with vertical line
int CDrawing::FindCrosspointX(tDrawingItem * di, int X, int Y1, int Y2, int * pX, int * pY)
{
	if ((di->X1<X) && (di->X2<X)) return 0; //no crosspoint
	if ((di->X1>X) && (di->X2>X)) return 0; //no crosspoint

	if (di->X1==di->X2) return 0;
	if ((di->Y1<Y1) && (di->Y2<Y1)) return 0;
	if ((di->Y1>Y2) && (di->Y2>Y2)) return 0;

	double a=(double)(di->Y2-di->Y1)/(double)(di->X2-di->X1);
	*pY=(int)(a*(X-di->X1)+di->Y1);
	*pX=X;
	if ((*pY<Y1) || (*pY>Y2)) return 0;
	return 1;
}

// finds crosspoint of an drawing item with horizontal line
int CDrawing::FindCrosspointY(tDrawingItem * di, int Y, int X1, int X2, int * pX, int * pY)
{
	if ((di->Y1<Y) && (di->Y2<Y)) return 0; //no crosspoint
	if ((di->Y1>Y) && (di->Y2>Y)) return 0; //no crosspoint

	if (di->Y1==di->Y2) return 0;
	if ((di->X1<X1) && (di->X2<X1)) return 0;
	if ((di->X1>X2) && (di->X2>X2)) return 0;

	double a=(double)(di->X2-di->X1)/(double)(di->Y2-di->Y1);
	*pX=(int)(a*(Y-di->Y1)+di->X1);
	*pY=Y;
	if ((*pX<X1) || (*pX>X2)) return 0;
	return 1;
}

int CDrawing::InsertItemAt(int pos)
{
	NumItems++;
	if (NumItems>65000) NumItems=65000;
	if (NumItems>NumItemsReserved) 
	{	
		NumItemsReserved=NumItems;
		Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
		if (Items==NULL) return 0;
	}
	for (int j=NumItems-1;j>=pos;j--)
		if (j>0) *(Items+j)=*(Items+j-1);

	return 1;
}

//the following function extract part of drawing that is not 
//connected to other parts of drawings into a separate drawing
int CDrawing::BreakApart(tDrawingItem* di,CDrawing *parent)
{
	this->OriginalForm=0;

	double angle=10;
	//if there are crossed lines, make them melted together
	if (di==NULL) //for speed
	{	
		for (int i=0;i<NumItems-1;i++)
		{			
			for (int j=0;j<NumItems;j++)
			if (i!=j)
			{
				tDrawingItem *di2=Items+i;
				tDrawingItem *di3=Items+j;

				//check if two lines are crossed, if yes add node at the cross point
				//this way, there will be a node at every cross point
				int x1=di2->X1;
				int y1=di2->Y1;
				int x2=di2->X2;
				int y2=di2->Y2;
				int x3=di3->X1;
				int y3=di3->Y1;
				int x4=di3->X2;
				int y4=di3->Y2;
				int xx,yy;
				int found=0;
				if ((x2-x1)!=0)
				{
					double a=(double)(y2-y1)/(double)(x2-x1);
					double ya=a*(x3-x1)+y1-y3;
					double yb=a*(x4-x1)+y1-y4;
					if ((fabs(ya)>1) && (fabs(yb)>1))
					if (((ya>0) && (yb<0)) || ((ya<0) && (yb>0)))
					{
						if (x4==x3)
						{
							xx=(int)x3;
							yy=(int)y3+(int)ya;
						}
						else
						{
							a=(double)(y4-y3)/(double)(x4-x3);
							ya=fabs(ya);
							yb=fabs(yb);
							xx=(int)(x3+(double)(x4-x3)*(ya/(ya+yb)));
							yy=(int)(y3+a*(xx-x3));
						}
						if ((xx>min(x3,x4)) && (xx<max(x3,x4)) &&
							(yy>min(y3,y4)) && (yy<max(y3,y4)) &&
							(xx>min(x1,x2)) && (xx<max(x1,x2))) found=1;
					}
				}
				else if (x4!=x3)
				{
					double a=(double)(y4-y3)/(double)(x4-x3);
					xx=(int)x1;
					yy=(int)(y3+a*(x1-x3));
					if ((x1<max(x3,x4)) && (x1>min(x3,x4)))
					if ((yy<max(y1,y2)) && (yy>min(y1,y2)))
						found=1;
				}

				if (found)
				{
					di2->X2=(int)xx;
					di2->Y2=(int)yy;
					di3->X2=(int)xx;
					di3->Y2=(int)yy;
					NumItems+=2;
					if (NumItems>65000) NumItems=65000;
					if (NumItems>NumItemsReserved) 
					{
						NumItemsReserved=NumItems+50;
						Items=(tDrawingItem*)realloc(Items,sizeof(tDrawingItem)*NumItemsReserved);
					}
					(Items+NumItems-2)->LineWidth=(Items+i)->LineWidth;
					(Items+NumItems-2)->Type=1;
					(Items+NumItems-2)->X1=(int)xx;
					(Items+NumItems-2)->Y1=(int)yy;
					(Items+NumItems-2)->X2=x2;
					(Items+NumItems-2)->Y2=y2;
					(Items+NumItems-1)->LineWidth=(Items+j)->LineWidth;
					(Items+NumItems-1)->Type=1;
					(Items+NumItems-1)->X1=(int)xx;
					(Items+NumItems-1)->Y1=(int)yy;
					(Items+NumItems-1)->X2=x4;
					(Items+NumItems-1)->Y2=y4;
				}
			}
		}

		//find the sharpest angle between two lines
		int is_closed=1;
		double angle2=10;
		for (int i=0;i<NumItems;i++)
		{
			int num_fnd=0;
			for (int j=0;j<NumItems;j++)
			if (i!=j)
			{
				tDrawingItem *di2=Items+i;
				tDrawingItem *di3=Items+j;
				int x=0x7FFFFFFF,y,x1,y1,x2,y2;
				if ((di3->X1==di2->X1) && (di3->Y1==di2->Y1)) {x=di2->X1;y=di2->Y1;x1=di2->X2;y1=di2->Y2;x2=di3->X2;y2=di3->Y2;}
				if ((di3->X2==di2->X1) && (di3->Y2==di2->Y1)) {x=di2->X1;y=di2->Y1;x1=di2->X2;y1=di2->Y2;x2=di3->X1;y2=di3->Y1;}
				if ((di3->X1==di2->X2) && (di3->Y1==di2->Y2)) {x=di2->X2;y=di2->Y2;x1=di2->X1;y1=di2->Y1;x2=di3->X2;y2=di3->Y2;}
				if ((di3->X2==di2->X2) && (di3->Y2==di2->Y2)) {x=di2->X2;y=di2->Y2;x1=di2->X1;y1=di2->Y1;x2=di3->X1;y2=di3->Y1;}
				if (x!=0x7FFFFFFF)
				{
					num_fnd++;
					if (j>i)
					{
						double r1=atan2((double)(y1-y),(double)(x1-x));
						double r2=atan2((double)(y2-y),(double)(x2-x));
						double r=fabs(r1-r2);
						if (r>3.1415926) r=2*3.1415926-r;
						if (r<angle) angle=r;
						else if (r<angle2) angle2=r;
					}
				}
			}
			if (num_fnd<2) is_closed=0;
		}
		if (is_closed) angle=angle2;
	}



	int first_pass=1;
	angle=angle*1.1;

break_apart_again:
	int break_all=0;
	if (di==NULL) 
	{
		//no part of the drawing is specifically targeted for separation,
		//therefore, we will spearate all phisically unconected parts
		break_all=1;
		di=Items;
	}
	if (NumItems<=0) return 0;
	if (di->Type!=1) return 0;
	
	di->LineWidth|=0x8000; //mark the first line
	int any_found=1;
	int has_others;
	while (any_found)
	{
		any_found=0;
		has_others=0;
		di=Items;
		for (int k=0;k<NumItems;k++,di++)
			if ((di->LineWidth)&0x8000)
			{
				tDrawingItem *di2=Items;
				for (int i=0;i<NumItems;i++,di2++)
					if ((di2->LineWidth&0x8000)==0)
						if (((di2->X1==di->X1) && (di2->Y1==di->Y1)) || 
							((di2->X2==di->X1) && (di2->Y2==di->Y1)) ||
							((di2->X1==di->X2) && (di2->Y1==di->Y2)) ||
							((di2->X2==di->X2) && (di2->Y2==di->Y2)))
						{
							double r;
							if (!first_pass)
							{

								int x,y,x1,y1,x2,y2;
								if ((di2->X1==di->X1) && (di2->Y1==di->Y1)) {x=di->X1;y=di->Y1;x1=di->X2;y1=di->Y2;x2=di2->X2;y2=di2->Y2;}
								if ((di2->X2==di->X1) && (di2->Y2==di->Y1)) {x=di->X1;y=di->Y1;x1=di->X2;y1=di->Y2;x2=di2->X1;y2=di2->Y1;}
								if ((di2->X1==di->X2) && (di2->Y1==di->Y2)) {x=di->X2;y=di->Y2;x1=di->X1;y1=di->Y1;x2=di2->X2;y2=di2->Y2;}
								if ((di2->X2==di->X2) && (di2->Y2==di->Y2)) {x=di->X2;y=di->Y2;x1=di->X1;y1=di->Y1;x2=di2->X1;y2=di2->Y1;}
								int ii;
								for (ii=0;ii<NumItems;ii++)
									if ((ii!=i) && (ii!=k))
									{
										tDrawingItem *di3=Items+ii;	
										if ((di3->X1==x) && (di3->Y1==y) ||
											(di3->X2==x) && (di3->Y2==y)) break;
									}
								if (ii<NumItems)
									r=-1.0;
								else
								{
									double r1=atan2((double)(y1-y),(double)(x1-x));
									double r2=atan2((double)(y2-y),(double)(x2-x));
									r=fabs(r1-r2);
									if (r>3.1415926) r=2*3.1415926-r;
								}
							
							}
							if ((first_pass) || (r>angle))
							{
								any_found=1;
								di2->LineWidth|=0x8000;
							}
						}	
			}
			else 
				has_others++;
	}


	if (has_others) 
	{
		CDrawing *temp=new CDrawing();
		temp->IsSelected=0;
		temp->OriginalForm=0;
		temp->m_Color=m_Color;
		int jj=0;
		for (int i=0;i<NumItems;i++)
		{
			tDrawingItem *di2=Items+i;
			if ((di2->LineWidth)&0x8000)
			{
				di2->LineWidth&=0x7FFF;
				temp->NumItems++;
				if (temp->NumItems>65000) temp->NumItems=65000;
				if (temp->NumItems>temp->NumItemsReserved) 
				{
					if (temp->NumItemsReserved==0) temp->NumItemsReserved=max(NumItems-has_others,temp->NumItems);
					else temp->NumItemsReserved=temp->NumItems+50;
					if (temp->Items) temp->Items=(tDrawingItem*)realloc(temp->Items,sizeof(tDrawingItem)*temp->NumItemsReserved);
					else temp->Items=(tDrawingItem*)malloc(sizeof(tDrawingItem)*temp->NumItemsReserved);
				}
				*((temp->Items)+jj)=*di2;
				jj++;

				for (int j=i;j<NumItems-1;j++) *(Items+j)=*(Items+j+1);
				NumItems--;
				i--;
			}
		}

		if (parent==NULL)
		{
			//the main document must be updated
			tDocumentStruct *ds=TheDocument;
			for (int i=0;i<NumDocumentElements;i++,ds++)
			{				
				if (ds->Object==(CObject*)this)
				{
					AddDocumentObject(2,ds->absolute_X,ds->absolute_Y);
					ds=TheDocument+i;
					tDocumentStruct *ds2=TheDocument+NumDocumentElements-1;
					int x1,y1,w,h;
					temp->AdjustCoordinates(&x1,&y1,&w,&h);
					ds2->absolute_X=ds->absolute_X+x1;
					ds2->absolute_Y=ds->absolute_Y+y1;
					ds2->Length=w;
					ds2->Below=h;
					ds2->Above=0;
					ds2->Object=(CObject*)temp;

					AdjustCoordinates(&x1,&y1,&w,&h);
					ds->absolute_X+=x1;
					ds->absolute_Y+=y1;
					ds->Above=0;
					ds->Below=h;
					ds->Length=w;
					break;
				}
			}
		}
		else
		{
			//the parent drawing must be updated
			for (int i=0;i<parent->NumItems;i++)
			{
				tDrawingItem *di=parent->Items+i;
				if ((di->Type==0) && (di->pSubdrawing==this))
				{
					parent->NumItems++;
					if (parent->NumItems>65000) parent->NumItems=65000;
					if (parent->NumItems>parent->NumItemsReserved) 
					{
						parent->NumItemsReserved=parent->NumItems;
						parent->Items=(tDrawingItem*)realloc(parent->Items,sizeof(tDrawingItem)*parent->NumItemsReserved);
					}
					di=parent->Items+i;
					tDrawingItem *di2=parent->Items+parent->NumItems-1;
					di2->Type=0;
					di2->pSubdrawing=(void*)temp;
					di2->LineWidth=di->LineWidth;
					int x1,y1,w,h;
					temp->AdjustCoordinates(&x1,&y1,&w,&h);
					di2->X1=di->X1+x1*DRWZOOM;
					di2->Y1=di->Y1+y1*DRWZOOM;
					di2->X2=di->X1+w*DRWZOOM;
					di2->Y2=di->Y1+h*DRWZOOM;

					
					AdjustCoordinates(&x1,&y1,&w,&h);
					di->X1+=x1*DRWZOOM;
					di->Y1+=y1*DRWZOOM;
					di->X2=di->X1+w*DRWZOOM;
					di->Y2=di->Y1+h*DRWZOOM;
					break;
				}
			}
		}
	}

	for (int i=0;i<NumItems;i++)
		(Items+i)->LineWidth&=0x7FFF;

	if (!break_all) return 0;

	if (has_others) 
	{
		if (first_pass) first_pass=2;
		di=NULL;
		goto break_apart_again;
	}
	else if (first_pass==1)
	{
		first_pass=0;
		di=NULL;
		goto break_apart_again;
	}


	
	return 0;
}

int CDrawing::CopyExpressionIntoSubgroup(CExpression* Original, int X, int Y, int width, int height)
{
	CExpression *tmp;
	NumItems++;
	if (NumItems>65000) NumItems=65000;
	if (NumItems>NumItemsReserved) 
	{
		NumItemsReserved=NumItems;
		if (Items)
			Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
		else
			Items=(tDrawingItem*)malloc(sizeof(tDrawingItem)*NumItemsReserved);
	}
	if (Items)
	{
		tmp=new CExpression(NULL,NULL,Original->m_FontSize);
		//tmp->m_FontSizeHQ=Original->m_FontSizeHQ;
		tmp->CopyExpression(Original,0);
		tDrawingItem *di=Items+NumItems-1;
		di->pSubdrawing=(void*)tmp;
		di->Type=2;
		di->X1=X*DRWZOOM;
		di->Y1=Y*DRWZOOM;
		di->X2=di->X1+width*DRWZOOM;
		di->Y2=di->Y1+height*DRWZOOM;
		//di->LineWidth=data;

		//int mx,my,w,h;
		//AdjustCoordinates(&mx,&my,&w,&h);

	}
		
	return 0;
}
int CDrawing::CopyDrawingIntoSubgroup(CDrawing* Original, int X, int Y)
{
	CDrawing *tmp;
	NumItems++;
	if (NumItems>65000) NumItems=65000;
	if (NumItems>NumItemsReserved) 
	{
		NumItemsReserved=NumItems;
		if (Items)
			Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
		else
			Items=(tDrawingItem*)malloc(sizeof(tDrawingItem)*NumItemsReserved);
	}
	if (Items)
	{
		tmp=new CDrawing();
		tmp->CopyDrawing(Original);
		int a,b,c,d;
		tmp->AdjustCoordinates(&a,&b,&c,&d);
		tDrawingItem *di=Items+NumItems-1;
		di->pSubdrawing=(void*)tmp;
		di->Type=0;
		di->X1=(X+a)*DRWZOOM;
		di->Y1=(Y+b)*DRWZOOM;
		di->X2=di->X1+c*DRWZOOM;
		di->Y2=di->Y1+d*DRWZOOM;

		//int mx,my,w,h;
		//AdjustCoordinates(&mx,&my,&w,&h);

	}

	//special handling - if copied into clipboard, copy also into windows clipboard
	if (this==ClipboardDrawing)
	{
		CopyToWindowsClipboard();
	}
	return 0;
}

int CDrawing::SetLineWidth(int width)
{
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		di->LineWidth=width;//*DRWZOOM;
		if ((di->Type==0) && (di->pSubdrawing))
		{
			((CDrawing*)(di->pSubdrawing))->SetLineWidth(width);
		}
	}
	return 0;
}

int CDrawing::ScaleForFactor(float factorx,float factory)
{
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		if ((di->Type==0) && (di->pSubdrawing))
		{
			int w=abs((int)((di->X2-di->X1)*factorx));
			int h=abs((int)((di->Y2-di->Y1)*factory));
			((CDrawing*)(di->pSubdrawing))->ScaleForFactor(factorx,factory);
			di->X1=(int)(di->X1*factorx);
			if (factorx<0) di->X1-=w;
			di->Y1=(int)(di->Y1*factory);
			if (factory<0) di->Y1-=h;
			di->X2=di->X1+w;
			di->Y2=di->Y1+h;
		}
		else if ((di->Type==2) && (di->pSubdrawing))
		{
			int w=abs((int)((di->X2-di->X1)*factorx));
			int h=abs((int)((di->Y2-di->Y1)*factory));
			//((CExpression*)(di->pSubdrawing))->ScaleForFactor(factorx,factory);
			if ((factorx!=1.0) && (factory!=1.0))
			{
				float fx=fabs(factorx);	
				float fy=fabs(factory); 
				if ((fx>1.0) && (fy>1.0)) fx=min(fx,fy);
				else if ((fx<=1.0) && (fy>1.0)) fx=1.0;
				else if ((fx>1.0) && (fy<1.0)) fx=1.0;
				else fx=max(fx,fy);
				((CExpression*)(di->pSubdrawing))->ChangeFontSize(fx);
			}
			di->X1=(int)(di->X1*factorx);
			if (factorx<0) di->X1-=w;
			di->Y1=(int)(di->Y1*factory);
			if (factory<0) di->Y1-=h;
			di->X2=di->X1+w;
			di->Y2=di->Y1+h;
		}
		else
		{
			di->X1=(int)(di->X1*factorx);
			di->Y1=(int)(di->Y1*factory);
			di->X2=(int)(di->X2*factorx);
			di->Y2=(int)(di->Y2*factory);
		}
	}
	int x,y,w,h;
	AdjustCoordinates(&x,&y,&w,&h);

	return 0;
}

int CDrawing::RotateForAngle(float angle, int centerX, int centerY, int* newX1, int* newY1, int* newW, int* newH)
{
	if (IsSpecialDrawing) return 0;
	double curangle;
	centerX*=DRWZOOM;
	centerY*=DRWZOOM;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;

		if ((di->Type==0) && (di->pSubdrawing))
		{
			int x1,y1,w,h;
			((CDrawing*)(di->pSubdrawing))->RotateForAngle(angle,(centerX-di->X1)/DRWZOOM,(centerY-di->Y1)/DRWZOOM,&x1,&y1,&w,&h);
			di->X1=x1*DRWZOOM;
			di->Y1=y1*DRWZOOM;
			di->X2=di->X1+w*DRWZOOM;
			di->Y2=di->Y1+h*DRWZOOM;
		}
		else if ((di->Type==2) && (di->pSubdrawing))
		{
			int x1=0,y1=0;
			short l,a,b;
			CDC *DC=pMainView->GetDC();
			((CExpression*)(di->pSubdrawing))->CalculateSize(DC,ViewZoom,&l,&a,&b);
			a=a*100/ViewZoom;
			pMainView->ReleaseDC(DC);
			int Y=di->Y1+a;
			double rad=sqrt((double)(di->X1-centerX)*(double)(di->X1-centerX)+(double)(Y-centerY)*(double)(Y-centerY));
			curangle=atan2((double)(-Y+centerY),(double)(di->X1-centerX));
			curangle+=angle;
			di->X1=(int)(rad*cos(curangle));
			di->Y1=-(int)(rad*sin(curangle))-a;

			//((CDrawing*)(di->pSubdrawing))->RotateForAngle(angle,(centerX-di->X1)/DRWZOOM,(centerY-di->Y1)/DRWZOOM,&x1,&y1,&w,&h);
			//di->X1=x1*DRWZOOM;
			//di->Y1=y1*DRWZOOM;
			//di->X2=di->X1+w*DRWZOOM;
			//di->Y2=di->Y1+h*DRWZOOM;
		}
		else
		{
			double rad=sqrt((double)(di->X1-centerX)*(double)(di->X1-centerX)+(double)(di->Y1-centerY)*(double)(di->Y1-centerY));
			curangle=atan2((double)(-di->Y1+centerY),(double)(di->X1-centerX));
			curangle+=angle;
			di->X1=(int)(rad*cos(curangle));
			di->Y1=-(int)(rad*sin(curangle));

			rad=sqrt((double)(di->X2-centerX)*(double)(di->X2-centerX)+(double)(di->Y2-centerY)*(double)(di->Y2-centerY));
			curangle=atan2((double)(-di->Y2+centerY),(double)(di->X2-centerX));
			curangle+=angle;
			di->X2=(int)(rad*cos(curangle));
			di->Y2=-(int)(rad*sin(curangle));
		}
	}

	AdjustCoordinates(newX1,newY1,newW,newH);

	return 0;
}

//high precission coordinates X and Y (10x)
int CDrawing::MoveNodeCoordinate(int X, int Y)
{
	int deltax;
	int deltay;
	int num_found=0;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		if ((di->Type==0) && (di->pSubdrawing))
		{
			int x1,y1,w,h;
			((CDrawing*)(di->pSubdrawing))->MoveNodeCoordinate(X-(di->X1)*10/DRWZOOM,Y-(di->Y1)*10/DRWZOOM);
			((CDrawing*)(di->pSubdrawing))->AdjustCoordinates(&x1,&y1,&w,&h);
			di->X1+=x1*DRWZOOM;
			di->Y1+=y1*DRWZOOM;
			di->X2=di->X1+w*DRWZOOM;
			di->Y2=di->Y1+h*DRWZOOM;
		}
		if (di->Type==1)
		{
			int n=3*DRWZOOM/2; if (ViewZoom>=256) n=n*16/(ViewZoom/16);
			if ((abs(di->X1-NodeX)<n) && (abs(di->Y1-NodeY)<n))
			{
				deltax=di->X1-X*DRWZOOM/10;
				deltay=di->Y1-Y*DRWZOOM/10;
				di->X1=X*DRWZOOM/10;
				di->Y1=Y*DRWZOOM/10;
				num_found++;
			}
			if ((abs(di->X2-NodeX)<n) && (abs(di->Y2-NodeY)<n))
			{
				deltax=di->X2-X*DRWZOOM/10;
				deltay=di->Y2-Y*DRWZOOM/10;
				di->X2=X*DRWZOOM/10;
				di->Y2=Y*DRWZOOM/10;
				num_found++;
			}
			if ((di->X1==di->X2) && (di->Y1==di->Y2) && (NumItems>1))
			{
				for (int j=i;j<NumItems-1;j++)
					*(Items+j)=*(Items+j+1);
				NumItems--;
				i--;
				this->OriginalForm=0;
			}
		}
	}

	if ((GetKeyState(VK_SHIFT)&0xFFFE) || (GetKeyState(VK_RBUTTON)&0xFFFE))
	if (num_found>0)
	{
		int x,y;
		x=X*DRWZOOM/10;
		y=Y*DRWZOOM/10;

		for (int i=0;i<NumItems;i++)
		{
			tDrawingItem *di2=Items+i;
			if ((di2->Type==1) && (di2->X1!=x) && (di2->Y1!=y))
			{
				int dx=di2->X1-x;
				int dy=di2->Y1-y;
				double ddist=sqrt((double)dx*(double)dx+(double)dy*(double)dy)/3.0/(double)DRWZOOM;
				int dist=(int)(ddist*ddist);
				di2->X1-=100*deltax/(100+dist);
				di2->Y1-=100*deltay/(100+dist);
			}
			if ((di2->Type==1) && (di2->X2!=x) && (di2->Y2!=y))
			{
				int dx=di2->X2-x;
				int dy=di2->Y2-y;
				double ddist=sqrt((double)dx*(double)dx+(double)dy*(double)dy)/3.0/(double)DRWZOOM;
				int dist=(int)(ddist*ddist);
				di2->X2-=100*deltax/(100+dist);
				di2->Y2-=100*deltay/(100+dist);
			}
		}
	}
	NodeX=X*DRWZOOM/10;
	NodeY=Y*DRWZOOM/10;

	return 0;
}


//this functions readjust internal coordinates of drawing items so that the upper
//left corner is set to (0,0).
int CDrawing::AdjustCoordinates(int* x1, int* y1, int* w, int* h, int absX, int absY)
{
	int x=0x7FFFFFFF;
	int y=0x7FFFFFFF;
	int x2=0x80000000;
	int y2=0x80000000;
	tDrawingItem *di=Items;
	for (int i=0;i<NumItems;i++)
	{
		if (di->Type==1)
		{
			int lw=di->LineWidth;
			if (di->X1-lw<x) x=di->X1-lw;
			if (di->X2-lw<x) x=di->X2-lw;
			if (di->Y1-lw<y) y=di->Y1-lw;
			if (di->Y2-lw<y) y=di->Y2-lw;

			if (di->X1+lw>x2) x2=di->X1+lw;
			if (di->X2+lw>x2) x2=di->X2+lw;
			if (di->Y1+lw>y2) y2=di->Y1+lw;
			if (di->Y2+lw>y2) y2=di->Y2+lw;
		}
		if (di->Type==0) //subdrawing
		{
			int a,b,c,d;
			((CDrawing*)(di->pSubdrawing))->AdjustCoordinates(&a,&b,&c,&d);
			a*=DRWZOOM;
			b*=DRWZOOM;
			c*=DRWZOOM;
			d*=DRWZOOM;
			di->X1+=a;
			di->X2=di->X1+c;
			di->Y1+=b;
			di->Y2=di->Y1+d;
			if (di->X1<x) x=di->X1;
			if (di->Y1<y) y=di->Y1;
			if (di->X2>x2) x2=di->X2;
			if (di->Y2>y2) y2=di->Y2;			
		}
		if (di->Type==2) //subexpression
		{
			if (di->X1<x) x=di->X1;
			if (di->Y1<y) y=di->Y1;

			if (di->X2>x2) x2=di->X2;
			if (di->Y2>y2) y2=di->Y2;			
		}
		di++;
	}
	di=Items;
	x=((x+halfDRWZOOM)/DRWZOOM)*DRWZOOM;
	y=((y+halfDRWZOOM)/DRWZOOM)*DRWZOOM;
	x2=((x2+3*DRWZOOM/2-1)/DRWZOOM)*DRWZOOM;
	y2=((y2+3*DRWZOOM/2-1)/DRWZOOM)*DRWZOOM;

	//checking if the zero desn't move (the 0,0 should have the same absolute postion after readjustment)
	if (absX!=0x7FFFFFFF)
	{
		int deltaX=absX*ViewZoom/100 - (absX+(x/DRWZOOM))*ViewZoom/100+(x/DRWZOOM);
		int deltaY=absY*ViewZoom/100 - (absY+(y/DRWZOOM))*ViewZoom/100+(y/DRWZOOM);
		if (deltaX)
		{
			x-=32*DRWZOOM/ViewZoom;
			x2-=32*DRWZOOM/ViewZoom;
		}
		if (deltaY)
		{
			y-=32*DRWZOOM/ViewZoom;
			y2-=32*DRWZOOM/ViewZoom;
		}
	}

	for (int i=0;i<NumItems;i++)
	{
		di->X1-=x;
		di->X2-=x;
		di->Y1-=y;
		di->Y2-=y;
		di++;
	}
	if ((NodeX!=-1) || (NodeY!=-1)) {NodeX-=x;NodeY-=y;}

	*x1=x/DRWZOOM;
	*y1=y/DRWZOOM;
	*w=(x2-x)/DRWZOOM;
	*h=(y2-y)/DRWZOOM;
	return 0;
}

int CDrawing::AnyNodeSelected(void)
{
	if (((AllowQuickEditNodes()) && (IsSelected)) ||
		((GetKeyState(VK_CONTROL)&0xFFFE) && (IsSelected)))
	{
		if ((NodeX!=-1) || (NodeY!=-1)) return 1;
	}
	tDrawingItem *di=Items;
	for (int i=0;i<NumItems;i++,di++)
	{
		if ((di->pSubdrawing) && (di->Type==0))
		{
			if (((CDrawing*)(di->pSubdrawing))->AnyNodeSelected()) return 1;
		}
	}
	return 0;
}

//returns 1 if it is ok to allow quick node editing (while menu/toolbar option for quick node editing is on)
int CDrawing::AllowQuickEditNodes(void)
{
	if (ToolbarEditNodes) 
	{
		if (!IsSelected) return 1;
		if ((this->OriginalForm>=8) && (this->OriginalForm<=13)) return 0; //coordinate systems, rasters
		if ((this->OriginalForm>=19) && (this->OriginalForm<=26)) return 0; //curly brackets, okay, !, ?, X, diagonal lines
		int maxlen=0;
		tDrawingItem *di=Items;
		for (int i=0;i<NumItems;i++,di++)
		{
			int len=abs(di->X1-di->X2)+abs(di->Y1-di->Y2);
			if (len>maxlen) maxlen=len;
		}
		if ((maxlen-(5*DRWZOOM))*ViewZoom>(700*DRWZOOM)) return 1; //there is enough distance between two nodes to grip the line with the mouse
	}
	return 0;
}

// returns coordinates of the real upper left corner
int CDrawing::FindRealCorner(int* X, int* Y, int *X2,int *Y2)
{
	int x1,y1,x2,y2;
	x1=y1=0x7FFFFFFF;
	x2=y2=-0x7FFFFFFF;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		if (di->Type==1)
		{
			if (di->X1<x1) x1=di->X1;
			if (di->X2<x1) x1=di->X2;
			if (di->Y1<y1) y1=di->Y1;
			if (di->Y2<y1) y1=di->Y2;

			if (di->X1>x2) x2=di->X1;
			if (di->X2>x2) x2=di->X2;
			if (di->Y1>y2) y2=di->Y1;
			if (di->Y2>y2) y2=di->Y2;
		}
		if ((di->Type==0) && (di->pSubdrawing))
		{
			int tmpx,tmpy,tmpx2,tmpy2;
			((CDrawing*)(di->pSubdrawing))->FindRealCorner(&tmpx,&tmpy,&tmpx2,&tmpy2);
			tmpx*=DRWZOOM;
			tmpy*=DRWZOOM;
			tmpx2*=DRWZOOM;
			tmpy2*=DRWZOOM;
			if (di->X1+tmpx<x1) x1=di->X1+tmpx;
			if (di->Y1+tmpy<y1) y1=di->Y1+tmpy;

			if (di->X1+tmpx2>x2) x2=di->X1+tmpx2;
			if (di->Y1+tmpy2>y2) y2=di->Y1+tmpy2;

		}
		if ((di->Type==2) && (di->pSubdrawing))
		{
			if (di->X1<x1) x1=di->X1;
			if (di->Y1<y1) y1=di->Y1;		

			if (di->X2>x2) x2=di->X2;
			if (di->Y2>y2) y2=di->Y2;		
		}
	}
	*X=x1/DRWZOOM;
	*Y=y1/DRWZOOM;
	if (X2) *X2=x2/DRWZOOM;
	if (Y2) *Y2=y2/DRWZOOM;
	return 0;
}


//adds new node to line
int CDrawing::SplitLineAtPos(int X, int Y)
{

	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;

		if ((di->Type==0) &&  (di->pSubdrawing))
		{
			int tmpret=((CDrawing*)(di->pSubdrawing))->SplitLineAtPos(X-(di->X1)/DRWZOOM,Y-(di->Y1)/DRWZOOM);
			if (tmpret==1) return 1;
		}
		if (di->Type==1)  //line
		{

			int x1=(di->X1)/DRWZOOM;
			int x2=(di->X2)/DRWZOOM;
			int y1=(di->Y1)/DRWZOOM;
			int y2=(di->Y2)/DRWZOOM;

			double l1=sqrt((double)(((double)x2-x1)*(x2-x1)+((double)y2-y1)*(y2-y1)));
			double l2=sqrt((double)(((double)x2-X)*(x2-X)+((double)y2-Y)*(y2-Y)));
			double l3=sqrt((double)(((double)X-x1)*(X-x1)+((double)Y-y1)*(Y-y1)));
			double factorf=(l2*l3)/(l2+l3);
			if (factorf==0) factorf=0.001;
			if (l2+l3<l1+1.5/factorf)
			{
				NumItems++;
				if (NumItems>65000) NumItems=65000;
				if (NumItemsReserved<NumItems)
				{
					NumItemsReserved=NumItems;
					Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
				}
				di=Items+i;
				for (int kk=NumItems-1;kk>i+1;kk--)
					*(Items+kk)=*(Items+kk-1);
				
				(di+1)->LineWidth=di->LineWidth;
				(di+1)->Type=1;
				(di+1)->X1=X*DRWZOOM;
				(di+1)->Y1=Y*DRWZOOM;
				(di+1)->X2=di->X2;
				(di+1)->Y2=di->Y2;
				di->X2=X*DRWZOOM;
				di->Y2=Y*DRWZOOM;
				this->OriginalForm=0;
				return 1;
			}
		}
	}
	return 0;
}

//should return 1 if mouse click is processed (there will be no further processing)
//X=Y=0x7FFFFFFF when button is up
#pragma optimize("s",on)
int CDrawing::MouseClick(int X, int Y)
{
	if (ViewOnlyMode) return 0;

	if ((IsSpecialDrawing==52) && (SpecialData))
		return ((CBitmapImage*)SpecialData)->MouseClick(X,Y);

	if ((IsSpecialDrawing==51) && (SpecialData))
		return ((CFunctionPlotter*)SpecialData)->MouseClick(X,Y);

	if ((IsSpecialDrawing==50) && (SpecialData))
		return ((CDrawingBox*)SpecialData)->MouseClick(X,Y);

	return 0;
}
#pragma optimize("",on)

//X=Y=0x7FFFFFFF when mouse is moving outside 
//function must return 1 if the object is to be redrawn
int CDrawing::MouseMove(CDC *DC,int X, int Y, UINT flags)
{
	if (ViewOnlyMode) return 0;

	if ((IsSpecialDrawing==52) && (SpecialData))
		return ((CBitmapImage*)SpecialData)->MouseMove(DC,X,Y,flags);

	if ((IsSpecialDrawing==51) && (SpecialData))
		return ((CFunctionPlotter*)SpecialData)->MouseMove(DC,X,Y,flags);

	if ((IsSpecialDrawing==50) && (SpecialData))
		return ((CDrawingBox*)SpecialData)->MouseMove(DC,X,Y,flags);

	return 0;
}

// returns lenght of the diagonal from given point to drawing lines
int CDrawing::FindDiagonalLength(int X, int Y, int* l1, int* l2, int direction)
{
	*l1=0x7FFFFFFF;
	*l2=0x7FFFFFFF;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		if ((di->Type==0) && (di->pSubdrawing)) //subdrawing
		{
			int l11,l22;
			((CDrawing*)(di->pSubdrawing))->FindDiagonalLength(X-di->X1,Y-di->Y1,&l11,&l22,direction);
			if (l11<*l1) *l1=l11;
			if (l22<*l2) *l2=l22;
		}
		if ((di->Type==2) && (di->pSubdrawing)) //subexpression
		{
			//does nothing
		}
		if ((di->Type==1) && (di->LineWidth>45*DRWZOOM/100)) //line
		{
			int x1=di->X1-X;
			int y1=di->Y1-Y;
			int x2=di->X2-X;
			int y2=di->Y2-Y;
			if (x2<x1) {int t;t=x1;x1=x2;x2=t;t=y1;y1=y2;y2=t;}

			//now we have coordinates x1,y2 and x2,y2 where (X,Y) is 0,0 of coordinate sytem

			double a2;
			double a=(double)direction;
			int xs,ys;
			if (x2!=x1) 
			{
				a2=((double)y2-(double)y1)/((double)x2-(double)x1);
				if (a2==a) continue;
				xs=(int)((-a2*x1+y1)/(a-a2));
				ys=(int)(xs*a);
				if ((xs<x1) || (xs>x2)) continue;
			}
			else
			{
				xs=x1;
				ys=(int)(xs*a);
				if ((ys<y2) && (ys<y1)) continue;
				if ((ys>y2) && (ys>y1)) continue;
			}
			if ((xs>0) && (ys>0))
				if (*l1>xs) *l1=xs;
			if ((xs<0) && (ys<0))
				if (*l2>-xs) *l2=-xs;
			if ((xs>0) && (ys<0))
				if (*l1>xs) *l1=xs;
			if ((xs<0) && (ys>0))
				if (*l2>-xs) *l2=-xs;
		}
	}
	return 0;
}


int CDrawing::SetColor(int color)
{
	if (color==0x08) 
	{
		if (m_Color==-1) m_Color=0x08;
		else if (m_Color&0x08) m_Color=m_Color&0x07;
		else m_Color=m_Color|0x08;
	}
	else
		 if ((m_Color&0xF8)==8) m_Color=color|0x8; else m_Color=color;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		if ((di->pSubdrawing) && (di->Type==0))
		{
			((CDrawing*)(di->pSubdrawing))->SetColor(color);
		}
		if ((di->pSubdrawing) && (di->Type==2))
		{
			((CExpression*)(di->pSubdrawing))->SetColor(color&0x07);
		}
	}

	return 1;
}

//combines all selected drawings into this one
int CDrawing::Combine(void)
{
	int absX,absY;
	int ii;
	for (ii=0;ii<NumDocumentElements;ii++)
		if (TheDocument[ii].Object==(CObject*)this)
		{
			absX=TheDocument[ii].absolute_X;
			absY=TheDocument[ii].absolute_Y;
			break;
		}
	if (ii==NumDocumentElements) return 0;

	for (int i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if ((ds->Type==2) && (ds->Object) && (ds->Object!=(CObject*)this) &&
			((ds->MovingDotState==3) || (((CDrawing*)(ds->Object))->IsSelected)))
		{
			CDrawing *drw=(CDrawing*)ds->Object;
			for (int j=0;j<drw->NumItems;j++)
			{
				tDrawingItem *di=drw->Items+j;
				NumItems++;
				if (NumItems>65000) NumItems=65000;
				if (NumItems>NumItemsReserved) 
				{	
					NumItemsReserved=NumItems+50;
					Items=(tDrawingItem*)realloc(Items,NumItemsReserved*sizeof(tDrawingItem));
				}
				Items[NumItems-1].LineWidth=di->LineWidth;
				Items[NumItems-1].pSubdrawing=di->pSubdrawing;
				Items[NumItems-1].Type=di->Type;
				int deltax=(ds->absolute_X-absX)*DRWZOOM;
				int deltay=(ds->absolute_Y-absY)*DRWZOOM;
				Items[NumItems-1].X1=di->X1+deltax;
				Items[NumItems-1].Y1=di->Y1+deltay;
				Items[NumItems-1].X2=di->X2+deltax;
				Items[NumItems-1].Y2=di->Y2+deltay;
			}

			pMainView->DeleteDocumentObject(ds);
			i--;
		}
	}


	int x,y,w,h;
	this->AdjustCoordinates(&x,&y,&w,&h);
	TheDocument[ii].absolute_X+=x;
	TheDocument[ii].absolute_Y+=y;
	TheDocument[ii].Below=h;
	TheDocument[ii].Length=w;


	return 1;
}


//checks if this drawing is an open-path drawing. If yes, it can close it.
//In addition, this function can also check if this is a closed path drawing and
//return points for drawing it (used for Polygone() API function)
int CDrawing::IsOpenPath(int close_path, char *is_closed_path,LPPOINT points, char *num_points_found)
{
	int num_points=0;
	if (is_closed_path) *is_closed_path=0;
	if (this->NumItems==0) return 0;

	//mark the frist element
	this->Items->LineWidth|=0x8000;
	int P1=0; //first pointer
	int S1=1; //first pointer direction
	int P2=0; //second pointer
	int S2=2; //second pointer direction

	if (points)
	{
		(points)->x=this->Items->X1;
		(points)->y=this->Items->Y1;
		num_points++;
		(points+num_points)->x=this->Items->X2;
		(points+num_points)->y=this->Items->Y2;
		num_points++;
	}
    
	while (1)
	{
		int oP1=P1;
		int oP2=P2;

		tDrawingItem *di=this->Items+P1;
		for (int j=0;j<this->NumItems;j++)
		if (P1!=j)
		{
			tDrawingItem *di2=this->Items+j;
			if (di2->LineWidth&0x8000) continue;
			if (S1==1)		
			{
				if ((di->X1==di2->X1) && (di->Y1==di2->Y1)) {P1=j;S1=2;di2->LineWidth|=0x8000;break;}
				if ((di->X1==di2->X2) && (di->Y1==di2->Y2))	{P1=j;S1=1;di2->LineWidth|=0x8000;break;}
			}
			else
			{
				if ((di->X2==di2->X1) && (di->Y2==di2->Y1)) {P1=j;S1=2;di2->LineWidth|=0x8000;break;}
				if ((di->X2==di2->X2) && (di->Y2==di2->Y2))	{P1=j;S1=1;di2->LineWidth|=0x8000;break;}
			}
		}


		di=this->Items+P2;
		for (int j=0;j<this->NumItems;j++)
		if (P2!=j)
		{
			tDrawingItem *di2=this->Items+j;
			if (di2->LineWidth&0x8000) continue;
			if (S2==1)		
			{
				if ((di->X1==di2->X1) && (di->Y1==di2->Y1)) {P2=j;S2=2;di2->LineWidth|=0x8000;break;}
				if ((di->X1==di2->X2) && (di->Y1==di2->Y2))	{P2=j;S2=1;di2->LineWidth|=0x8000;break;}
			}
			else	
			{
				if ((di->X2==di2->X1) && (di->Y2==di2->Y1)) {P2=j;S2=2;di2->LineWidth|=0x8000;break;}
				if ((di->X2==di2->X2) && (di->Y2==di2->Y2))	{P2=j;S2=1;di2->LineWidth|=0x8000;break;}
			}
		}

		//check if we are finished (nowhere to move the pointer)
		if ((oP1==P1) && (oP2==P2)) break;
		if ((points) && (num_points<63))
		{
			if (oP1!=P1)
			{
				di=this->Items+P1;
				memmove(points+1,points,num_points*sizeof(POINT));
				if (S1==1) {(points)->x=di->X1;(points)->y=di->Y1;}
				else {(points)->x=di->X2;(points)->y=di->Y2;}
				num_points++;
			}
			if (oP2!=P2)
			{
				di=this->Items+P2;
				if (S2==1) {(points+num_points)->x=di->X1;(points+num_points)->y=di->Y1;}
				else {(points+num_points)->x=di->X2;(points+num_points)->y=di->Y2;}
				num_points++;
			}
		}
	}

	//check if all items were marked
	for (int i=0;i<NumItems;i++)
		if (((Items+i)->LineWidth&0x8000)==0) 
			goto isopenpath_end; 
		else 
			(Items+i)->LineWidth&=0x7FFF;

	if (num_points_found) *num_points_found=num_points;
	//check if this is a closed path
	if (P1==P2) goto isopenpath_end;

	{
		tDrawingItem *di=this->Items+P1;
		tDrawingItem *di2=this->Items+P2;
		if (((S1==1) && (S2==1) && (di->X1==di2->X1) && (di->Y1==di2->Y1)) ||
			((S1==1) && (S2==2) && (di->X1==di2->X2) && (di->Y1==di2->Y2)) ||
			((S1==2) && (S2==1) && (di->X2==di2->X1) && (di->Y2==di2->Y1)) ||
			((S1==2) && (S2==2) && (di->X2==di2->X2) && (di->Y2==di2->Y2)))
		{
			if (is_closed_path) *is_closed_path=1; 
			if (points) 
			{
				*is_closed_path=num_points;
			}
			goto isopenpath_end;
		}
	}


	//force closing path
	if (close_path)
	{
		this->InsertItemAt(NumItems);
		tDrawingItem *di=Items+NumItems-1;

		tDrawingItem *di2=Items+P1;
		di->LineWidth=di2->LineWidth;
		di->pSubdrawing=NULL;
		if (S1==1) {di->X1=di2->X1;di->Y1=di2->Y1;} else {di->X1=di2->X2;di->Y1=di2->Y2;}
		di2=Items+P2;
		if (S2==1) {di->X2=di2->X1;di->Y2=di2->Y1;} else {di->X2=di2->X2;di->Y2=di2->Y2;}
	}
	return 1;

isopenpath_end:
	for (int i=0;i<NumItems;i++)
		(Items+i)->LineWidth&=0x7FFF;
	return 0;
}

//this function makes drawing line dashed
#pragma optimize("s",on)
int CDrawing::MakeDashed(char dash_dot)
{
	char draw=1;
	int dlen=7*DRWZOOM;

	int dphase=0;
	int numitems=this->NumItems;
	for (int i=0;i<numitems;i++)
	{
		tDrawingItem *di=Items+i;
		int x1=di->X1;
		int x2=di->X2;
		int y1=di->Y1;
		int y2=di->Y2;
		if ((x2==x1) && (y2==y1)) continue;

		if (abs(x2-x1)>abs(y2-y1))
		{
			int pos=x1;
			int pos2=y1;
			int opos=pos;
			int opos2=pos2;
			double a=(double)(y2-y1)/(double)(x2-x1);
			int exit=0;
			while (!exit)
			{
				int dashlen=dlen;
				if ((dash_dot) && ((draw&0x03)==0x03)) dashlen=dlen/4;
				int step=(int)(((double)(dashlen-dphase)*(double)(x2-x1))/sqrt((double)(x2-x1)*(double)(x2-x1)+(double)(y2-y1)*(double)(y2-y1)));
				dphase=0;
				if (((step>0) && (pos+step>=x2)) ||
					((step<0) && (pos+step<=x2)))	
				    {dphase=dashlen*(x2-pos)/step;step=x2-pos;exit=1;}
				pos+=step;
				pos2=(int)((double)(pos-x1)*a)+y1;
				if (draw&0x01)
				{
					this->InsertItemAt(this->NumItems);
					tDrawingItem *di2=Items+NumItems-1;
					di2->LineWidth=(Items+i)->LineWidth;
					di2->pSubdrawing=0;
					di2->X1=pos;di2->Y1=pos2;di2->X2=opos;di2->Y2=opos2;
				}
				if (!exit) draw++;
				opos=pos;opos2=pos2;
			}
		}
		else
		{
			int pos=y1;
			int pos2=x1;
			int opos=pos;
			int opos2=pos2;
			double a=(double)(x2-x1)/(double)(y2-y1);
			int exit=0;
			while (!exit)
			{
				int dashlen=dlen;
				if ((dash_dot) && ((draw&0x03)==0x03)) dashlen=dlen/4;
				int step=(int)(((double)(dashlen-dphase)*(double)(y2-y1))/sqrt((double)(x2-x1)*(double)(x2-x1)+(double)(y2-y1)*(double)(y2-y1)));
				dphase=0;
				if (((step>0) && (pos+step>=y2)) ||
					((step<0) && (pos+step<=y2)))	
				    {dphase=dashlen*(y2-pos)/step;step=y2-pos;exit=1;}
				pos+=step;
				pos2=(int)((double)(pos-y1)*a)+x1;
				if (draw&0x01)
				{
					this->InsertItemAt(this->NumItems);
					tDrawingItem *di2=Items+NumItems-1;
					di2->LineWidth=(Items+i)->LineWidth;
					di2->pSubdrawing=0;
					di2->Y1=pos;di2->X1=pos2;di2->Y2=opos;di2->X2=opos2;
				}
				if (!exit) draw++;
				opos=pos;opos2=pos2;
			}
		}

	}
	for (int i=numitems;i<NumItems;i++)
	{
		*(Items+i-numitems)=*(Items+i);
	}
	this->NumItems-=numitems;
	return 1;
}
#pragma optimize("",on)


//X and Y are in absolute coordinates - we are searching 
//for nearby point on drawing (intersection...) 
//X and Y are in absolute coordinates

//TODO: this function is too slow with many objects displayed
int NearbyPointX;
int NearbyPointY;
float NearbyPointDistance;
int CDrawing::FindNerbyPoint(int *X, int *Y, CDrawing *drw, int X1,int Y1,int X2,int Y2)
{	
	RECT r;
	pMainView->GetClientRect(&r);
	r.right=r.right*100/ViewZoom;
	r.bottom=r.bottom*100/ViewZoom;
	r.left=ViewX;
	r.right+=ViewX;
	r.bottom+=ViewY;
	r.top=ViewY;
	if (drw==NULL)
	{
		NearbyPointX=-1;
		NearbyPointY=-1;
		NearbyPointDistance=DRWZOOM;

		tDocumentStruct *ds=TheDocument;
		for (int i=0;i<NumDocumentElements;i++,ds++)
		{
			if ((ds->Type==2) && (ds->Object))
			if ((ds->absolute_Y-15<*Y) && (ds->absolute_Y+ds->Below+15>*Y) && 
				(ds->absolute_X-15<*X) && (ds->absolute_X+ds->Length+15>*X))
			{
				CDrawing *drw1=(CDrawing*)ds->Object;
				if (drw1->IsSpecialDrawing==0)
				{
					tDocumentStruct *ds2=TheDocument;
					for (int j=0;j<NumDocumentElements;j++,ds2++)
					{
						if ((ds2->Type==2) && (ds2->Object))
						if ((ds2->absolute_Y-15<*Y) && (ds2->absolute_Y+ds2->Below+15>*Y) && 
							(ds2->absolute_X-15<*X) && (ds2->absolute_X+ds2->Length+15>*X))
						{
							CDrawing *drw2=(CDrawing*)ds2->Object;
							if (drw2->IsSpecialDrawing==0)
								drw1->FindNerbyPoint(X,Y,drw2,ds->absolute_X,ds->absolute_Y,ds2->absolute_X,ds2->absolute_Y);
						}
					}
				}
			}
		}
		if (NearbyPointDistance<6)
		{
			*X=NearbyPointX;
			*Y=NearbyPointY;
			return 1;
		}

		return 0;
	}

	tDrawingItem *di=this->Items;
	for (int i=0;i<this->NumItems;i++,di++)
	{
		if (di->Type==0) //subdrawing
		{
			CDrawing *subdrw=(CDrawing*)di->pSubdrawing;
			subdrw->FindNerbyPoint(X,Y,drw,X1+di->X1/DRWZOOM,Y1+di->Y1/DRWZOOM,X2,Y2);
			continue;
		}
		tDrawingItem *di2=drw->Items;
		for (int j=0;j<drw->NumItems;j++,di2++)
		{
			if (di2->Type==0) //subdrawing
			{
				CDrawing *subdrw=(CDrawing*)di2->pSubdrawing;
				this->FindNerbyPoint(X,Y,subdrw,X1,Y1,X2+di2->X1/DRWZOOM,Y2+di2->Y1/DRWZOOM);
				continue;
			}
			if ((di->Type==1) && (di2->Type==1))
			{
				//find intersection point of two lines
				double x,y;
				int fnd=0;
				int continued=0;
				double aa1,aa2;
				if (this==drw)
				{
					if (((di->X1==di2->X1) && (di->Y1==di2->Y1)) ||
						((di->X1==di2->X2) && (di->Y1==di2->Y2)) ||
						((di->X2==di2->X1) && (di->Y2==di2->Y1)) ||
						((di->X2==di2->X2) && (di->Y2==di2->Y2)))
					{
						aa1=atan2((double)(di->X2-di->X1),(double)(di->Y2-di->Y1));
						aa2=atan2((double)(di2->X2-di2->X1),(double)(di2->Y2-di2->Y1));
						double aa=fabs(aa1-aa2);
						if ((abs(di->X2-di->X1)>5*DRWZOOM) || (abs(di->Y2-di->Y1)>5*DRWZOOM) ||
							(abs(di2->X2-di2->X1)>5*DRWZOOM) || (abs(di2->Y2-di2->Y1)>5*DRWZOOM))
						{
							if ((aa<0.60) || (aa>6.28-0.60)) continued=1;
						}
						else
						{
							if ((aa<1.00) || (aa>6.28-1.00)) continued=1;
						}
					}
				}

				if (continued)
				{
					//do nothing
				}
				else if ((di->X1!=di->X2) && (di2->X1!=di2->X2))
				{
					double a1=(double)(di->Y2-di->Y1)/(double)(di->X2-di->X1);
					double a2=(double)(di2->Y2-di2->Y1)/(double)(di2->X2-di2->X1);

					double y10=(double)(di->Y1/DRWZOOM+Y1)-a1*(di->X1/DRWZOOM+X1);
					double y20=(double)(di2->Y1/DRWZOOM+Y2)-a2*(di2->X1/DRWZOOM+X2);


					if (continued) 
					{
					
					}
					if (a1!=a2)
					{
						x=(y20-y10)/(a1-a2);
						y=y10+x*a1;					
						if (((min(di->X1,di->X2)/DRWZOOM+X1-1<=x) && (max(di->X1,di->X2)/DRWZOOM+X1+1>=x)) &&
							((min(di2->X1,di2->X2)/DRWZOOM+X2-1<=x) && (max(di2->X1,di2->X2)/DRWZOOM+X2+1>=x)))
							fnd=1;
					}
				}
				else if ((di->X1==di->X2) && (di2->X1!=di2->X2))
				{
					double a2=(double)(di2->Y2-di2->Y1)/(double)(di2->X2-di2->X1);

					double y20=(double)(di2->Y1/DRWZOOM+Y2)-a2*(di2->X1/DRWZOOM+X2);

					x=di->X1/DRWZOOM+X1;
					y=a2*x+y20;									

					if (((min(di->Y1,di->Y2)/DRWZOOM+Y1-1<=y) && (max(di->Y1,di->Y2)/DRWZOOM+Y1+1>=y)) &&
						((min(di2->X1,di2->X2)/DRWZOOM+X2-1<=x) && (max(di2->X1,di2->X2)/DRWZOOM+X2+1>=x)))
						fnd=1;
				}
				else if ((di->X1!=di->X2) && (di2->X1==di2->X2))
				{
					double a1=(double)(di->Y2-di->Y1)/(double)(di->X2-di->X1);

					double y10=(double)(di->Y1/DRWZOOM+Y1)-a1*(di->X1/DRWZOOM+X1);

					x=di2->X1/DRWZOOM+X2;
					y=a1*x+y10;				
					if (((min(di2->Y1,di2->Y2)/DRWZOOM+Y2-1<=y) && (max(di2->Y1,di2->Y2)/DRWZOOM+Y2+1>=y)) &&
						((min(di->X1,di->X2)/DRWZOOM+X1-1<=x) && (max(di->X1,di->X2)/DRWZOOM+X1+1>=x)))
						fnd=1;
				}
				if (fnd)
				{
					float ddistance=(float)sqrt((*X-x)*(*X-x)+(*Y-y)*(*Y-y));
					if (ddistance<NearbyPointDistance)
					{
						NearbyPointDistance=ddistance;
						NearbyPointX=(int)x;
						NearbyPointY=(int)y;
					}
				}

				if (j==0)
				{
					if ((di2->X1!=(drw->Items+drw->NumItems-1)->X2) || (di2->Y1!=(drw->Items+drw->NumItems-1)->Y2))
					{
						x=di2->X1/DRWZOOM+X2;
						y=di2->Y1/DRWZOOM+Y2;
						float ddistance=(float)sqrt((*X-x)*(*X-x)+(*Y-y)*(*Y-y));
						if (ddistance<NearbyPointDistance)
						{
							NearbyPointDistance=ddistance;
							NearbyPointX=(int)x;
							NearbyPointY=(int)y;
						}
					}
				}
				if (j==drw->NumItems-1)
				{
					if ((di2->X2!=(drw->Items)->X1) || (di2->Y2!=(drw->Items)->Y1))
					{
						x=di2->X2/DRWZOOM+X2;
						y=di2->Y2/DRWZOOM+Y2;
						float ddistance=(float)sqrt((*X-x)*(*X-x)+(*Y-y)*(*Y-y));
						if (ddistance<NearbyPointDistance)
						{
							NearbyPointDistance=ddistance;
							NearbyPointX=(int)x;
							NearbyPointY=(int)y;
						}
					}
				}
				if (j<drw->NumItems-1)
				{
					if ((di2->X2!=(di2+1)->X1) || (di2->Y2!=(di2+1)->Y1))
					{
						x=di2->X2/DRWZOOM+X2;
						y=di2->Y2/DRWZOOM+Y2;
						float ddistance=(float)sqrt((*X-x)*(*X-x)+(*Y-y)*(*Y-y));
						if (ddistance<NearbyPointDistance)
						{
							NearbyPointDistance=ddistance;
							NearbyPointX=(int)x;
							NearbyPointY=(int)y;
						}
					}
				}
				if (j>0)
				{
					if ((di2->X1!=(di2-1)->X2) || (di2->Y1!=(di2-1)->Y2))
					{
						x=di2->X1/DRWZOOM+X2;
						y=di2->Y1/DRWZOOM+Y2;
						float ddistance=(float)sqrt((*X-x)*(*X-x)+(*Y-y)*(*Y-y));
						if (ddistance<NearbyPointDistance)
						{
							NearbyPointDistance=ddistance;
							NearbyPointX=(int)x;
							NearbyPointY=(int)y;
						}
					}
				}
			}
		}
	}

	return 1;
}

void CDrawing::FindBottomRightDrawingPoint(int *X, int *Y)
{
	//searches the drawingclipboard for the bottom right point
	*X=0;
	*Y=0;
	int max=0;
	for (int i=0;i<NumItems;i++)
	{
		tDrawingItem *di=Items+i;
		if ((di->Type==0) && (di->pSubdrawing))
		{
			int x1,y1;
			((CDrawing*)(di->pSubdrawing))->FindBottomRightDrawingPoint(&x1,&y1);
			x1+=di->X1;
			y1+=di->Y1;
			if (x1+y1>max) {max=x1+y1;*X=x1;*Y=y1;}
		}
		if (di->Type==1)
		{
			if ((di->X1+di->Y1)>max) {max=di->X1+di->Y1;*X=di->X1;*Y=di->Y1;}
			if ((di->X2+di->Y2)>max) {max=di->X2+di->Y2;*X=di->X2;*Y=di->Y2;}
		}
	}
}