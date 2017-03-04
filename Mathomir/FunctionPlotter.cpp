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
extern tDocumentStruct *SpecialDrawingHover;
extern int PrintRendering;
extern CExpression* ClipboardExpression;

char DrawingThreadUsed;
CRITICAL_SECTION section;
int wwxx=0;

DWORD WINAPI DrawingThread(LPVOID lpParameter)
{
	int crit_sec=0;
	if (wwxx==0){wwxx=1;InitializeCriticalSection(&section);}

	//we are not entering if the window is scrolled (or otherwise painted)
	//because some bug corrupts memory in multi-threading
	CFunctionPlotter *Plotter=(CFunctionPlotter*)lpParameter;
	int timer=0;
	while ((GetKeyState(VK_RBUTTON)&0x8000) || 
		  (GetKeyState(33)&0x8000) ||
		  (GetKeyState(34)&0x8000) ||
		  (GetKeyState(VK_LEFT)&0x8000) ||
		  (GetKeyState(VK_RIGHT)&0x8000) ||
		  (GetKeyState(VK_UP)&0x8000) ||
		  (GetKeyState(VK_DOWN)&0x8000) ||
		  (timer<15)) //we must wait a bit!!!! STRANGE, otherwise function drawing is not ok
	{
		if (Plotter->abort_request) goto PlotterThread_exit;
		Sleep(10);
		timer++;
	}

	//Entering the ciritcal seciton - one by one graph is ploted
	//NOTE - if the critical section is removed then all graphs are plotted simultaneously
	//       but, unfortunately, there is a bug and software can crash (often on some PCs)
	timer=0;
	while (!TryEnterCriticalSection(&section)) 
	{
		Sleep(10);
		if (Plotter->abort_request) goto PlotterThread_exit;
		timer++;
		if (timer>200) goto PlotterThread_exit;
	}

	//we entered the critical section !
	DrawingThreadUsed++;
	crit_sec=1;
	try
	{
		if (Plotter->Base->IsSpecialDrawing==51) Plotter->PlotFunction(0);
	}
	catch(...)
	{

	}

PlotterThread_exit:
	Plotter->ThreadHandle=NULL;
	if (DrawingThreadUsed>0) DrawingThreadUsed--;
	if (crit_sec) LeaveCriticalSection(&section);
	ExitThread(0);
	return 0;
}

int double_compare( const void *arg1, const void *arg2 )
{
	if (_isnan(*(double*)arg1))
	{
		if (_isnan(*(double*)arg2)) return 0;
		return -1;
	}
	if (_isnan(*(double*)arg2)) return 1;
   if (*(double*)arg1>*(double*)arg2) return 1;
   if (*(double*)arg1<*(double*)arg2) return -1;
   return 0;
}

//constructor
CFunctionPlotter::CFunctionPlotter(CDrawing *BaseItem)
{
	Base=BaseItem;
	Plot=NULL;
	is_y_log=is_x_log=0;
	abort_request=0;
	show_no_scale=0;
	calc_y=0;
	analyze=0;
	any_function_defined=0;
	m_X1=m_Y1=m_X2=m_Y2=0;
	ThreadID=0;
	ThreadHandle=NULL;
	TheState=0;
	MX=50;
	MY=25;
}

//destructor (should release all previously reserved memory)
CFunctionPlotter::~CFunctionPlotter(void)
{
	abort_request=1;
	while (ThreadHandle) Sleep(10);
	if (Plot) delete Plot;
}

//Copies relevant data from another object of the same type (this is used
//when a copy of an existing object is to be made)
int CFunctionPlotter::CopyFrom(CDrawing *Original)
{
	if (Original->IsSpecialDrawing!=51) return 0;
	CFunctionPlotter *org=(CFunctionPlotter*)Original->SpecialData;
	if (!org) return 0;

	is_y_log=org->is_y_log;
	is_x_log=org->is_x_log;
	show_no_scale=org->show_no_scale;
	analyze=org->analyze;
	any_function_defined=org->any_function_defined;
	m_X1=org->m_X1;
	m_Y1=org->m_Y1;
	m_X2=org->m_X2;
	m_Y2=org->m_Y2;
	MX=org->MX;
	MY=org->MY;
	ThreadID=0;
	ThreadHandle=NULL;
	TheState=0;
	Plot=NULL;

	return 1;
}

//Paints the object interior (painting starts from X,Y coordinates - upper left corner)
int wwwxxx;
CRITICAL_SECTION section2;
int CFunctionPlotter::Paint(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg)
{
	MX=(PrintRendering?50:max(50,2*ToolboxSize/5+20));
	MY=MX/2;
	const int LeftMargin=MX*zoom/100;
	const int BottomMargin=MY*zoom/100;

	X+=min(Base->Items->X1,Base->Items->X2)/DRWZOOM*zoom/100+1;
	Y+=min((Base->Items+1)->Y1,(Base->Items+1)->Y2)/DRWZOOM*zoom/100+1;
	const int Xlen=abs(Base->Items->X1-Base->Items->X2)/DRWZOOM*zoom/100;
	const int Ylen=abs((Base->Items+1)->Y1-(Base->Items+1)->Y2)/DRWZOOM*zoom/100;

	// we will not paint interior if the window is too small
	if ((Xlen<LeftMargin+10) || (Ylen<BottomMargin+10)) return 0;

	//entering the critical section - no two thread can enter here at the same time!
	if (wwwxxx==0) {InitializeCriticalSection(&section2);wwwxxx=1;}
	if (!TryEnterCriticalSection(&section2)) return 0;

	//painting the gray area
	DC->FillSolidRect(X,Y,LeftMargin,Ylen-2,RGB(224,224,224));
	DC->FillSolidRect(X,Y+Ylen-BottomMargin,Xlen,BottomMargin,RGB(224,224,224));

	//if we are printing, then force function plotting
	if ((Base->NumItems>=9) && (PrintRendering)) PlotFunction(1,DC);

	if ((TheState==96) || (TheState==94)) //Y ranging  and X ranging - stretching the bitmap
	{
		if (Plot) //the plot bitmap exists
		{
			CSize sz=Plot->GetBitmapDimension();
			CDC mDC;
			mDC.CreateCompatibleDC(DC);
			mDC.SelectObject(Plot);

			double Xmax,Ymax,Xmin,Ymin;
			this->PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);

			if ((is_y_log==0) && (TheState==96))
			{
				int Ylen=(Base->Items+1)->Y2/DRWZOOM-MY;
				double Ys=Ymax-(Ymax-Ymin)*(double)m_Y1/(double)Ylen;
				double Ye=Ymax-(Ymax-Ymin)*(double)m_Y2/(double)Ylen;
				double f=(Ye-Ymin)/(Ys-Ymin);
				double Ymax2=Ymin+(Ymax-Ymin)/f;
				int ysize=(int)((sz.cy-1)*((Ymax2-Ymin)/(Ymax-Ymin)));
				DC->StretchBlt(X+LeftMargin,Y,sz.cx-2,sz.cy-1,&mDC,0,sz.cy-1-ysize,sz.cx-2,ysize,SRCCOPY);
			}
			else if ((is_x_log==0) && (TheState==94))
			{
				int Xlen=(Base->Items)->X2/DRWZOOM-MX;
				double Xs=Xmin+(Xmax-Xmin)*((double)m_X1-MX)/(double)Xlen;
				double Xe=Xmin+(Xmax-Xmin)*((double)m_X2-MX)/(double)Xlen;
				double f=(Xe-Xmin)/(Xs-Xmin);
				double Xmax2=Xmin+(Xmax-Xmin)/f;
				int xsize=(int)((sz.cx-2)*((Xmax2-Xmin)/(Xmax-Xmin)));
				DC->StretchBlt(X+LeftMargin,Y,sz.cx-2,sz.cy-1,&mDC,0,0,xsize,sz.cy-1,SRCCOPY);
			}
		}
	}
	else if ((Base->NumItems>=9) && ((Base->Items+8)->Type==2))
	{
		//we are bit-blting the function image (if the image not exist, or is not right dimensions we start the plotting thread)
		if (!Plot)
		{
			//nasty trick!!! - the lin/log configuration of the function plotter
			//is stored into m_Alignment
			CExpression *y0=(CExpression*)((Base->Items+6)->pSubdrawing);
			CExpression *x0=(CExpression*)((Base->Items+4)->pSubdrawing);
			if (y0->m_Alignment) is_y_log=1; else is_y_log=0;
			if (x0->m_Alignment) is_x_log=1; else is_x_log=0;

			//start the plotting thread and create the bitmap
			if (GetCurrentThreadId()!=ThreadID)
				PlotFunction(1,NULL,zoom);
		}

		if (Plot)
		{
			//check the bitmap size, does it fit? If not, we are recreating the plot bitmap image.
			CSize sz=Plot->GetBitmapDimension();
			if ((sz.cx!=Xlen-LeftMargin) || (sz.cy!=Ylen-BottomMargin)) 
			{
				if (GetCurrentThreadId()!=ThreadID)
					PlotFunction(1,NULL,zoom);
			}

			if (zoom>12)
			{
				CDC mDC;
				mDC.CreateCompatibleDC(DC);
				mDC.SelectObject(Plot);

				int deltaX=0,deltaY=0;

				if (TheState==3) //are we moving the bitmap with mouse click-and-drag?
				{
					deltaX=(m_X2-m_X1)*zoom/100;
					deltaY=(m_Y2-m_Y1)*zoom/100;
					//DC->FillSolidRect(X+LeftMargin,Y,sz.cx-2,sz.cy-2,RGB(255,255,255));
				}

				DC->BitBlt(	max(X+LeftMargin,X+LeftMargin+deltaX),
							max(Y,Y+deltaY),
							min(sz.cx-2,sz.cx-deltaX-2),min(sz.cy-1,sz.cy-deltaY-1),&mDC,max(0,-deltaX),max(0,-deltaY),SRCCOPY);
			
				if (((TheState==100) || (TheState==102)) && (sz.cx>150)) //zoom-ing in, print out the helper text
				{
					DC->SetTextAlign(TA_LEFT);
					DC->SetTextColor(RGB(160,160,160));
					DC->SetBkColor(RGB(255,255,255));
					DC->SetBkMode(TRANSPARENT);
					DC->SelectObject(GetFontFromPool(4,0,0,MX/3));
					DC->TextOut(X+LeftMargin+30,Y+3,"Select the zoom-in area.",24);
				}					

				if ((any_function_defined==0) && (sz.cx>150) && (sz.cy>50)) //if no function is defined, print out the helper text
				{
					DC->SetTextAlign(TA_CENTER);
					DC->SetTextColor(RGB(160,160,160));
					DC->SetBkColor(RGB(255,255,255));
					DC->SetBkMode(TRANSPARENT);
					DC->SelectObject(GetFontFromPool(4,0,0,MX/3));
					DC->TextOut(X+LeftMargin+(sz.cx)/2,Y+(sz.cy)/2-MX/3,"drop an equation here,",22);
					DC->TextOut(X+LeftMargin+(sz.cx)/2,Y+(sz.cy)/2,"or click at colored",19);
					DC->TextOut(X+LeftMargin+(sz.cx)/2,Y+(sz.cy)/2+MX/3,"buttons to type it.",19);
				}
			}
		}
	}

	if (Plot)
	{
		if ((TheState==97) || (TheState==96)) //Y ranging - painting the line
		{
			CSize sz=Plot->GetBitmapDimension();
			DC->FillSolidRect(X,Y+m_Y2*ViewZoom/100,sz.cx+LeftMargin,1,RGB(0,0,0));
		}
		if ((TheState==95) || (TheState==94)) //X ranging - painting the line
		{
			CSize sz=Plot->GetBitmapDimension();
			DC->FillSolidRect(X+m_X2*ViewZoom/100,Y,1,sz.cy+BottomMargin,RGB(0,0,0));
		}
	}
	if (TheState==102) //zoom in - drawing rectangle (selection frame)
	{
		DC->SelectObject(GetStockObject(BLACK_PEN));
		DC->MoveTo(X+m_X1*ViewZoom/100,Y+m_Y1*ViewZoom/100);
		DC->LineTo(X+m_X2*ViewZoom/100,Y+m_Y1*ViewZoom/100);
		DC->LineTo(X+m_X2*ViewZoom/100,Y+m_Y2*ViewZoom/100);
		DC->LineTo(X+m_X1*ViewZoom/100,Y+m_Y2*ViewZoom/100);
		DC->LineTo(X+m_X1*ViewZoom/100,Y+m_Y1*ViewZoom/100);
	}
	

	//painting control buttons (ony when mouse pointer howers above the graph window)
	if ((SpecialDrawingHover) && (SpecialDrawingHover->Object==(CObject*)Base) && (!PrintRendering) && 
		(show_no_scale==0))
	{
		const int tt=(ToolboxSize+28)/16;
		const int hh=(ToolboxSize+28)/53;
		const int zz=(ToolboxSize+28)/18;
		const int xstart=X+LeftMargin+ToolboxSize/6;
		const int xstep=ToolboxSize/3;
		const int ystart=Y+ToolboxSize/6;

		if (!ClipboardExpression) 
		{
			//the plus sign
			if ((TheState!=102) && (TheState!=100)) DC->FillSolidRect(xstart-2*zz,ystart-2*zz,4*zz,4*zz,(TheState==2)?BLUE_COLOR:RGB(224,224,224));
			DC->FillSolidRect(xstart-tt,ystart-hh,2*tt,2*hh,(TheState==2)?RGB(255,255,255):RGB(0,0,0));
			DC->FillSolidRect(xstart-hh,ystart-tt,2*hh,2*tt,(TheState==2)?RGB(255,255,255):RGB(0,0,0));
		}
		if ((TheState!=102) && (TheState!=100))
		{
			if (!ClipboardExpression)
			{
				//the minus sign
				DC->FillSolidRect(xstart+xstep-2*zz,ystart-2*zz,4*zz,4*zz,(TheState==1)?BLUE_COLOR:RGB(224,224,224));
				DC->FillSolidRect(xstart+xstep-tt,ystart-hh,2*tt,2*hh,(TheState==1)?RGB(255,255,255):RGB(0,0,0));

				if ((any_function_defined) && (Xlen>2*ToolboxSize))
				{
					//the analyze sign
					DC->FillSolidRect(xstart+3*xstep-2*zz,ystart-2*zz,4*zz,4*zz,(TheState==15)?BLUE_COLOR:RGB(224,224,224));
					DC->FillSolidRect(xstart+3*xstep-tt,ystart-hh/2,2*tt,hh,(TheState==15)?RGB(255,255,255):RGB(0,0,0));
					DC->FillSolidRect(xstart+3*xstep-hh,ystart-hh,2*hh,2*hh+1,(TheState==15)?RGB(255,255,255):(analyze)?RGB(255,0,0):RGB(0,0,0));


					//the 'adjust' sign
					DC->FillSolidRect(xstart+2*xstep-2*zz,ystart-2*zz,4*zz,4*zz,(TheState==99)?BLUE_COLOR:RGB(224,224,224));
					POINT p[10];
					const int hhb=ToolboxSize/27;
					p[0].x=xstart+2*xstep;		p[0].y=ystart-3*hhb;
					p[1].x=p[0].x+2*hhb;		p[1].y=p[0].y+2*hhb;
					p[2].x=p[0].x+hhb-1;		p[2].y=p[1].y-1;
					p[3].x=p[2].x;				p[3].y=p[1].y+2*hhb;
					p[4].x=p[1].x;				p[4].y=p[3].y-1;
					p[5].x=p[0].x;				p[5].y=p[4].y+2*hhb;
					p[6].x=p[0].x-2*hhb;		p[6].y=p[4].y;
					p[7].x=p[0].x-hhb+1;		p[7].y=p[3].y;
					p[8].x=p[7].x;				p[8].y=p[2].y;
					p[9].x=p[0].x-2*hhb;		p[9].y=p[1].y;
					if (TheState==99)
					{
						DC->SelectObject(GetStockObject(WHITE_BRUSH));
						DC->SelectObject(GetStockObject(WHITE_PEN));
					}
					else
					{
						DC->SelectObject(GetStockObject(BLACK_BRUSH));
						DC->SelectObject(GetStockObject(BLACK_PEN));
					}
					DC->Polygon(p,10);
				}

				if (zoom>=60)
				{
					//the lin/log sign
					DC->FillSolidRect(X+hh,Y+Ylen-4*zz-hh-1,4*zz,4*zz,(TheState==98)?BLUE_COLOR:RGB(200,200,200));
					DC->SelectObject(GetFontFromPool(4,0,0,tt*2));
					DC->SetTextAlign(TA_CENTER | TA_BASELINE);
					DC->SetBkMode(TRANSPARENT);
					DC->SetTextColor(0);
					DC->TextOut(X+hh+2*zz,Y+Ylen-2*zz-hh-3,(is_y_log)?"LOG":"LIN",3);
					DC->TextOut(X+hh+2*zz,Y+Ylen-hh-2,(is_x_log)?"LOG":"LIN",3);
				}
			}

			//three color buttons (black, green, red)
			if (Ylen>ToolboxSize)
			{
				int zzz;
				zzz=(TheState==8)?zz:zz/2;DC->FillSolidRect(X+Xlen-ToolboxSize/6-2*zzz,Y+ToolboxSize/6-2*zzz,4*zzz,4*zzz,RGB(0,0,0));
				zzz=(TheState==9)?zz:zz/2;DC->FillSolidRect(X+Xlen-ToolboxSize/6-2*zzz,Y+ToolboxSize/3+ToolboxSize/6-2*zzz,4*zzz,4*zzz,RGB(0,192,0));
				zzz=(TheState==10)?zz:zz/2;DC->FillSolidRect(X+Xlen-ToolboxSize/6-2*zzz,Y+2*ToolboxSize/3+ToolboxSize/6-2*zzz,4*zzz,4*zzz,RGB(192,0,0));
				zzz=(TheState==11)?zz:zz/2;DC->FillSolidRect(X+Xlen-ToolboxSize/6-2*zzz,Y+ToolboxSize+ToolboxSize/6-2*zzz,4*zzz,4*zzz,RGB(0,0,192));

			}
		}
	}

	//painting grid and axes
	DC->SetTextColor(0);
	if (show_no_scale==0)
	{
		int prevquality=IsHighQualityRendering;
		IsHighQualityRendering=1;

		double Xmax,Xmin,Ymax,Ymin;
		PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);

		DC->IntersectClipRect(X,Y,X+Xlen-1,Y+Ylen-1);
		CPen GrayPen(PS_SOLID,1,RGB(208,208,208));

		if (is_x_log)
		{
			double Xmx=Xmax;
			double Xmn=Xmin;
			if (TheState==3) //window scroll
			{
				int Xlenn=(Base->Items)->X2/DRWZOOM-MX;
				int DeltaX=m_X2-m_X1;
				double fx=pow(10.0,log10(Xmax/Xmin)*(DeltaX)/Xlenn);
				Xmx=Xmax/fx;
				Xmn=Xmin/fx;
			}
			if (TheState==94) //X ranging
			{
				int Xlenn=(Base->Items)->X2/DRWZOOM-MX;
				double Xs=m_X1-MX;
				Xs=Xmin*pow(10.0,log10(Xmax/Xmin)*Xs/Xlenn);
				double Xe=m_X2-MX;
				Xmx=Xmin*pow(10.0,Xlenn/Xe*log10(Xs/Xmin));
			}

			if (Xmx<=1e-100) Xmx=1;
			if (Xmn<=1e-100) Xmn=Xmax/1e+10;
			double xspan=Xmx/Xmn;
			double xstep;
			double first=-1;

			if (xspan>=4)
				xstep=10;
			else
				xstep=pow(10.0,0.1*log10(xspan));

			first=pow(min(10,xstep),(int)(log10(Xmn)/log10(min(10,xstep))+1e-30));
			if (Xmn<1) first/=min(10,xstep);

			double steplen=(Xlen-LeftMargin)/log10(xspan)*log10(xstep);
			int last_x=-1000;
			while (first<Xmx)
			{
				double x=(log10(first)-log10(Xmn))*(Xlen-LeftMargin)/log10(xspan);
				{
					x+=LeftMargin;
					int xx;
					CPen GrayThin(PS_SOLID,1,RGB(160,160,160));
					DC->SelectObject(GrayThin);
					DC->SetROP2(R2_MASKPEN);
					xx=(int)x;				  if (xx>=LeftMargin) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
					if ((xstep>5) && (steplen>15))
					{
						DC->SelectObject(GrayPen);
						xx=(int)(x+steplen*0.301);if ((xx>=LeftMargin)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.477);if ((xx>=LeftMargin) && (steplen>40)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.602);if ((xx>=LeftMargin)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.699);if ((xx>=LeftMargin) && (steplen>40)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.778);if ((xx>=LeftMargin)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.845);if ((xx>=LeftMargin) && (steplen>40)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.903);if ((xx>=LeftMargin)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
						xx=(int)(x+steplen*0.954);if ((xx>=LeftMargin) && (steplen>40)) {DC->MoveTo(X+xx,Y);DC->LineTo(X+xx,Y+Ylen-BottomMargin-1);}
					}
					DC->SetROP2(R2_COPYPEN);
					if ((int)x>=LeftMargin)
					if ((int)x>last_x)
					{
						char number[64];
						number[0]=0;
						ShowNumberWithPrecision(first,first/10.0,number);
						int fsize=15;
						if (strlen(number)>6) fsize=12;
						if (strlen(number)>9) fsize=10;
						fsize=fsize*MX*zoom/5000;
						DC->SelectObject(GetFontFromPool(0,0,0,fsize));
						DC->SetTextAlign(TA_CENTER);
						DC->TextOut(X+(int)x,Y+Ylen-BottomMargin,number);
						CSize cs;
						cs=DC->GetTextExtent(number,(int)strlen(number));
						last_x=(int)x+cs.cx+LeftMargin/8; 
					}
				}
				first*=xstep;
			}
		}

		if (is_y_log)
		{
			double Ymx=Ymax;
			double Ymn=Ymin;
			if (TheState==3) //window scroll
			{
				int Ylenn=(Base->Items+1)->Y2/DRWZOOM-MY;
				int DeltaY=m_Y2-m_Y1;
				double fy=pow(10.0,log10(Ymax/Ymin)*(DeltaY)/Ylenn);
				Ymx=Ymax*fy;
				Ymn=Ymin*fy;
			}
			if (TheState==96) //Y ranging
			{
				int Ylenn=(Base->Items+1)->Y2/DRWZOOM-MY;
				double Ys=Ylenn-m_Y1;
				Ys=Ymin*pow(10.0,log10(Ymax/Ymin)*Ys/Ylenn);
				double Ye=Ylenn-m_Y2;
				Ymx=Ymin*pow(10.0,Ylenn/Ye*log10(Ys/Ymin));
			}


			if (Ymx<=1e-100) Ymx=1;
			if (Ymn<=1e-100) Ymn=Ymx/1e+10;
			double yspan=Ymx/Ymn;
			double ystep;
			double first=-1;
			if (yspan>=4) ystep=10;
			else
			{
				ystep=pow(10.0,0.1*log10(yspan)); //=yspan^(1/10)
			}

			first=pow(min(10,ystep),(int)(log10(Ymn)/log10(min(10,ystep))+1e-30));
			if (Ymin<1) first/=min(10,ystep);

			double steplen=(Ylen-BottomMargin)/log10(yspan)*log10(ystep);
			int last_y=100000;
			while (first<Ymx)
			{
				double y=Ylen-BottomMargin-(log10(first)-log10(Ymn))*(Ylen-BottomMargin)/log10(yspan);
				{
					int xx;
					CPen GrayThin(PS_SOLID,1,RGB(160,160,160));
					DC->SelectObject(GrayThin);
					DC->SetROP2(R2_MASKPEN);
					xx=(int)y;				  if (xx<=Ylen-BottomMargin) {DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
					if ((ystep>5) && (steplen>15))
					{
						DC->SelectObject(GrayPen);
						xx=(int)(y-steplen*0.301);if ((xx<=Ylen-BottomMargin))					{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.477);if ((xx<=Ylen-BottomMargin) && (steplen>40))	{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.602);if ((xx<=Ylen-BottomMargin))					{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.699);if ((xx<=Ylen-BottomMargin) && (steplen>40))	{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.778);if ((xx<=Ylen-BottomMargin))					{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.845);if ((xx<=Ylen-BottomMargin) && (steplen>40))	{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.903);if ((xx<=Ylen-BottomMargin))					{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
						xx=(int)(y-steplen*0.954);if ((xx<=Ylen-BottomMargin) && (steplen>40))	{DC->MoveTo(X+LeftMargin,Y+xx);DC->LineTo(X+Xlen-2,Y+xx);}
					}
					DC->SetROP2(R2_COPYPEN);
					if ((int)y<=Ylen-BottomMargin)
					if (last_y-(int)y>zoom/5)
					{
						char number[64];
						number[0]=0;
						ShowNumberWithPrecision(first,first/10.0,number);
						int fsize=15;
						if (strlen(number)>6) fsize=12;
						if (strlen(number)>9) fsize=10;
						fsize=fsize*MX*zoom/5000;
						DC->SelectObject(GetFontFromPool(0,0,0,fsize));
						DC->SetTextAlign(TA_RIGHT);
						DC->TextOut(X+LeftMargin-1,Y+(int)y-fsize/2,number);
						last_y=(int)y;
					}
				}
				first*=ystep;
			}
		}

		if (is_x_log==0)
		{
			double Xmx=Xmax;
			double Xmn=Xmin;
			if (TheState==3) //ploter window scrolling
			{
				int DeltaX=m_X2-m_X1;
				int Xlenn=(Base->Items)->X2/DRWZOOM-MX;
				double dx=(Xmax-Xmin)/Xlenn*DeltaX;
				Xmx=Xmax-dx;
				Xmn=Xmin-dx;
			}
			if (TheState==94) //X ranging
			{
				int Xlenn=(Base->Items)->X2/DRWZOOM-MX;
				double Xs=Xmin+(Xmax-Xmin)*((double)m_X1-MX)/(double)Xlenn;
				double Xe=Xmin+(Xmax-Xmin)*((double)m_X2-MX)/(double)Xlenn;
				double f=(Xe-Xmin)/(Xs-Xmin);
				Xmx=Xmin+(Xmax-Xmin)/f;
			}

			double xspan=fabs(Xmx-Xmn);
			double xstep=1.0;
			if (xspan>10.0)
			{
				while (xspan>10.0*xstep) xstep*=10;
				if (xspan<2.0*xstep) xstep/=5;
				if (xspan<5.0*xstep) xstep/=2;
			}
			else if (xspan<2.0)
			{
				while (xspan<2.0*xstep) {xstep/=10.0;}
				if (xspan>10*xstep) {xstep*=2;}
				if (xspan<5.0*xstep) {xstep/=2;}
			}
			else if (xspan<5)
			{
				xstep=0.5;
			}
			if (xstep<1e-48) xstep=1e-48;

			double first=((long long)((Xmn)/xstep))*xstep;
			int last_x=-1000;
			while (first<Xmx)
			{
				int x=(int)(((double)(Xlen-LeftMargin)*(first-Xmn)/xspan));
				if (x>=0)
				{
					x+=LeftMargin;
					DC->SelectObject(GrayPen);
					DC->SetROP2(R2_MASKPEN);
					DC->MoveTo(X+x,Y);
					DC->LineTo(X+x,Y+Ylen-BottomMargin);
					if ((fabs(first)<fabs(first+xstep)) && (fabs(first)<fabs(first-xstep)))
					{
						DC->SelectObject(GetPenFromPool(1,0,RGB(224,224,224)));
						DC->MoveTo(X+x+1,Y);DC->LineTo(X+x+1,Y+Ylen-BottomMargin);
						DC->MoveTo(X+x-1,Y);DC->LineTo(X+x-1,Y+Ylen-BottomMargin);
					}
					DC->SetROP2(R2_COPYPEN);

					if (x>last_x/*-last_x>LeftMargin*/)
					{
						char number[64];
						number[0]=0;
						ShowNumberWithPrecision(first,xstep/5.1,number);
						int fsize=15;
						if (strlen(number)>6) fsize=12;
						if (strlen(number)>9) fsize=10;
						fsize=fsize*MX*zoom/5000;
						DC->SelectObject(GetFontFromPool(0,0,0,fsize));
						DC->SetTextAlign(TA_CENTER);
						DC->TextOut(X+x,Y+Ylen-BottomMargin,number);
						CSize cs;
						if (number[0]=='-')
							cs=DC->GetTextExtent(number+1,(int)strlen(number+1)); 
						else
							cs=DC->GetTextExtent(number,(int)strlen(number));
						last_x=x+cs.cx+LeftMargin/10;
					}
				}
				double test=first;
				first+=xstep;
				if (test==first) break;
			}
		}

		if (is_y_log==0)
		{
			double Ymx=Ymax;
			double Ymn=Ymin;
			if (TheState==3) //window scroll
			{
				int DeltaY=m_Y2-m_Y1;
				int Ylenn=(Base->Items+1)->Y2/DRWZOOM-MY;
				double dy=(Ymax-Ymin)/Ylenn*DeltaY;
				Ymx=Ymax+dy;
				Ymn=Ymin+dy;
			}
			if (TheState==96) //Y ranging
			{
				int Ylenn=(Base->Items+1)->Y2/DRWZOOM-MY;
				double Ys=Ymax-(Ymax-Ymin)*(double)m_Y1/(double)Ylenn;
				double Ye=Ymax-(Ymax-Ymin)*(double)m_Y2/(double)Ylenn;
				double f=(Ye-Ymin)/(Ys-Ymin);
				Ymx=Ymin+(Ymax-Ymin)/f;
			}

			double yspan=fabs(Ymx-Ymn);
			double ystep=1.0;
			if (yspan>10.0)
			{
				while (yspan>10.0*ystep) ystep*=10;
				if (yspan<2.0*ystep) ystep/=5;
				if (yspan<5.0*ystep) ystep/=2;
				
			}
			else if (yspan<2.0)
			{
				while (yspan<2.0*ystep) {ystep/=10.0;}
				if (yspan>10.0*ystep) {ystep*=2;}
				if (yspan<5.0*ystep) {ystep/=2;}
			}
			else if (yspan<5)
			{
				ystep=0.5;
			}

			if (ystep<1e-48) ystep=1e-48;
			double first=((long long)((Ymn)/ystep))*ystep;
			int last_y=100000;
			while (first<Ymx)
			{
				int y=(int)(((double)(Ylen-BottomMargin)*(first-Ymn)/yspan));
				if (y>=0)
				{
					y=Ylen-BottomMargin-y;
					DC->SelectObject(GrayPen);
					DC->SetROP2(R2_MASKPEN);
					DC->MoveTo(X+LeftMargin,Y+y);
					DC->LineTo(X+Xlen,Y+y);
					if ((fabs(first)<fabs(first+ystep)) && (fabs(first)<fabs(first-ystep))) 
					{
						DC->SelectObject(GetPenFromPool(1,0,RGB(224,224,224)));
						DC->MoveTo(X+LeftMargin,Y+y+1);DC->LineTo(X+Xlen,Y+y+1);
						DC->MoveTo(X+LeftMargin,Y+y-1);DC->LineTo(X+Xlen,Y+y-1);
					}
					DC->SetROP2(R2_COPYPEN);

					if (last_y-y>zoom/5)
					{ 
						char number[64];
						number[0]=0;
						ShowNumberWithPrecision(first,ystep/5.1,number);
						int fsize=15;
						if (strlen(number)>6) fsize=12;
						if (strlen(number)>9) fsize=10;
						fsize=fsize*MX*zoom/5000;
						DC->SelectObject(GetFontFromPool(0,0,0,fsize));
						DC->SetTextAlign(TA_RIGHT);
						DC->TextOut(X+LeftMargin-1,Y+y-fsize/2,number);
						last_y=y;
					}
				}
				double test=first;
				first+=ystep;
				if (test==first) break;
			}
		}

		//now drawing X and Y names

		if ((Base->NumItems>=9) && (ThreadHandle==NULL))
		{
			CExpression *var;
			for (int kk=0;kk<4;kk++)
			if (Base->NumItems>8+kk)
			{
				CExpression *func=(CExpression*)((Base->Items+8+kk)->pSubdrawing);
				if ((func) && (func->m_pElementList->Type) && (Plot))
				{
					int position;
					int start_point;
					char type;
					if (func->PlotterGetEquationInfo(&var,&position,&start_point,&type))
					{
						CExpression *tmp=new CExpression(NULL,NULL,100);
						if (start_point>1)
						{
							for (int i=0;i<start_point-1;i++)
								tmp->InsertElement(func->m_pElementList+i,tmp->m_NumElements);
							short l,a,b;
							tmp->CalculateSize(DC,zoom*3/4*MX/50,&l,&a,&b);
							DC->FillSolidRect(X+1,Y+1,l,a+b+2,RGB(200,200,200));
							tmp->PaintExpression(DC,zoom*3/4*MX/50,X+1,Y+a+1,ClipReg);
							tmp->Delete();
						}
						tmp->InsertElement(var->m_pElementList+position,0);
						short l,a,b;
						tmp->CalculateSize(DC,zoom*3/4*MX/50,&l,&a,&b);
						DC->FillSolidRect(X+Xlen-l-3,Y+Ylen-b-a-2,l,a+b+1,RGB(200,200,200));
						tmp->PaintExpression(DC,zoom*3/4*MX/50,X+Xlen-l-3,Y+Ylen-b-2,ClipReg);

						delete tmp;
						break;
					}
				}
			}
		}
		DC->SelectClipRgn(NULL);
		IsHighQualityRendering=prevquality;
	}

	//if in edit mode (keyboard editing)
	if (TheState>=104)
	{
		short l,a,b;
		if (KeyboardEntryObject)
		{
			CExpression *tmp=(CExpression*)((Base->Items+TheState-100)->pSubdrawing);
			tmp->CalculateSize(DC,ViewZoom*MX/50,&l,&a,&b);
			a+=2;b+=1;

			int x0;
			int y0;
			POINT pp[10];
			if (TheState==104) // Xmin
			{
				x0=X+LeftMargin+10;
				y0=Y+Ylen-BottomMargin-a-b-10;
				pp[0].x=x0;pp[0].y=y0;
				pp[1].x=pp[0].x+l;pp[1].y=pp[0].y;
				pp[2].x=pp[1].x;pp[2].y=pp[1].y+a+b;
				pp[3].x=pp[0].x+3;pp[3].y=pp[2].y;
				pp[4].x=pp[3].x-10;pp[4].y=pp[2].y+20;
				pp[5].x=pp[0].x;pp[5].y=pp[2].y-3;
				pp[6].x=pp[0].x;pp[6].y=pp[0].y;
			}
			if (TheState==105) // Xmax
			{
				x0=X+Xlen-l-10;
				y0=Y+Ylen-BottomMargin-a-b-10;
				pp[0].x=x0;pp[0].y=y0;
				pp[1].x=pp[0].x+l; pp[1].y=pp[0].y;
				pp[2].x=pp[1].x;   pp[2].y=pp[1].y+a+b-3;
				pp[3].x=pp[2].x+10;pp[3].y=pp[2].y+20;
				pp[4].x=pp[2].x-3; pp[4].y=pp[1].y+a+b;
				pp[5].x=pp[0].x;   pp[5].y=pp[4].y;
				pp[6].x=pp[0].x;   pp[6].y=pp[0].y;
			}
			if (TheState==106) // Ymin
			{
				x0=X+LeftMargin+10;
				y0=Y+Ylen-BottomMargin-a-b-10;
				pp[0].x=x0;pp[0].y=y0;
				pp[1].x=pp[0].x+l;pp[1].y=pp[0].y;
				pp[2].x=pp[1].x;pp[2].y=pp[1].y+a+b;
				pp[3].x=pp[0].x+3;pp[3].y=pp[2].y;
				pp[4].x=pp[3].x-20;pp[4].y=pp[2].y+10;
				pp[5].x=pp[0].x;pp[5].y=pp[2].y-3;
				pp[6].x=pp[0].x;pp[6].y=pp[0].y;
			}
			if (TheState==107) // Ymax
			{
				x0=X+LeftMargin+10;
				y0=Y+10;
				pp[0].x=x0+3;pp[0].y=y0;
				pp[1].x=pp[0].x+l;pp[1].y=pp[0].y;
				pp[2].x=pp[1].x;pp[2].y=pp[1].y+a+b;
				pp[3].x=x0;pp[3].y=pp[2].y;
				pp[4].x=x0;pp[4].y=pp[0].y+3;
				pp[5].x=x0-20;pp[5].y=pp[0].y-10;
				pp[6].x=pp[0].x;pp[6].y=pp[0].y;
			}
			if ((TheState==108) ||
				(TheState==109) ||
				(TheState==110) ||
				(TheState==111)) //function definitions
			{
				
				x0=X+Xlen-l-20-ToolboxSize/6;
				y0=Y+10+ToolboxSize/6+(TheState-108)*ToolboxSize/3;
				pp[0].x=x0-3;pp[0].y=y0;
				pp[1].x=pp[0].x+l-2+6;pp[1].y=pp[0].y;
				pp[2].x=pp[1].x+20;pp[2].y=pp[1].y-10;
				pp[3].x=pp[1].x+3;pp[3].y=pp[1].y+3;
				pp[4].x=pp[3].x;pp[4].y=pp[0].y+a+b;
				pp[5].x=pp[0].x;pp[5].y=pp[4].y;
				pp[6].x=pp[0].x;pp[6].y=pp[0].y;
			}
			DC->SelectObject(GetStockObject(WHITE_BRUSH));
			DC->SelectObject(GetStockObject(BLACK_PEN));
			DC->Polygon(pp,7);

			tmp->PaintExpression(DC,ViewZoom*MX/50,x0,y0+a,ClipReg);
		}
		else
		{
			int type=1;
			if ((TheState==4) || (TheState==5))
				type=2;
			TheState=0;

			DWORD hid=GetCurrentThreadId();
			if (hid!=ThreadID)
				PlotFunction(type);
		}
	}
	LeaveCriticalSection(&section2);
	return 1;
}

#pragma optimize("s",on)
int CFunctionPlotter::MouseClick(int X, int Y)
{
	if (X!=0x7FFFFFFF)
	{
		CDC *DC=pMainView->GetDC();
		MouseMove(DC,-X,-Y,0);
		pMainView->ReleaseDC(DC);
	}

	if (ThreadHandle) 
	if (GetKeyState(VK_LBUTTON)&0x8000)
	{
		abort_request=1;
		Sleep(80);
	}
	if (DrawingThreadUsed) return 0;


	if ((X==0x7FFFFFFF) && (Y==0x7FFFFFFF))
	{
		//left mouse button up
		if (TheState==96)
		{
			//Y ranging
			double Xmax,Ymax,Xmin,Ymin;
			this->PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);
			int Ylen=(Base->Items+1)->Y2/DRWZOOM-MY;
			if (is_y_log==0)
			{
				double Ys=Ymax-(Ymax-Ymin)*(double)m_Y1/(double)Ylen;
				double Ye=Ymax-(Ymax-Ymin)*(double)m_Y2/(double)Ylen;
				double f=(Ye-Ymin)/(Ys-Ymin);
				Ymax=Ymin+(Ymax-Ymin)/f;
			}
			else
			{
				double Ys=Ylen-m_Y1;
				Ys=Ymin*pow(10.0,log10(Ymax/Ymin)*Ys/Ylen);
				double Ye=Ylen-m_Y2;
				Ymax=Ymin*pow(10.0,Ylen/Ye*log10(Ys/Ymin));

			}
			((CExpression*)((Base->Items+7)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+7)->pSubdrawing))->GenerateASCIINumber(Ymax,(long long)Ymax,0,5,0);
			TheState=0;
			PlotFunction(1);			
			return 0;

		}
		if (TheState==94)
		{
			//X ranging
			double Xmax,Ymax,Xmin,Ymin;
			this->PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);
			int Xlen=(Base->Items)->X2/DRWZOOM-MX;
			if (is_x_log==0)
			{
				double Xs=Xmin+(Xmax-Xmin)*((double)m_X1-MX)/(double)Xlen;
				double Xe=Xmin+(Xmax-Xmin)*((double)m_X2-MX)/(double)Xlen;
				double f=(Xe-Xmin)/(Xs-Xmin);
				Xmax=Xmin+(Xmax-Xmin)/f;
			}
			else
			{
				double Xs=m_X1-MX;
				Xs=Xmin*pow(10.0,log10(Xmax/Xmin)*Xs/Xlen);
				double Xe=m_X2-MX;
				Xmax=Xmin*pow(10.0,Xlen/Xe*log10(Xs/Xmin));
			}
			((CExpression*)((Base->Items+5)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+5)->pSubdrawing))->GenerateASCIINumber(Xmax,(long long)Xmax,0,5,0);
			TheState=0;
			PlotFunction(1);			
			return 0;

		}
		if (TheState==102) //zoom in (+) tool drag finisheed
		{
			int Xlen=(Base->Items)->X2/DRWZOOM-MX;
			int Ylen=(Base->Items+1)->Y2/DRWZOOM-MY;
			double Xmax,Ymax,Xmin,Ymin;
			this->PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);
			double a,b,c,d;
			if (is_x_log==0)
			{
				a=Xmin+(Xmax-Xmin)*(double)min(m_X1-MX,m_X2-MX)/(double)Xlen;
				b=Xmin+(Xmax-Xmin)*(double)max(m_X1-MX,m_X2-MX)/(double)Xlen;
				if (m_X1==m_X2) {a=Xmin-(Xmax-Xmin);b=Xmax+(Xmax-Xmin);}
				Xmin=a;
				Xmax=b;
			}
			else
			{
				if (m_X1==m_X2) 
				{
					Xmin/=10;
					Xmax*=10;
				}
				else
				{
					double x1=min(m_X1-MX,m_X2-MX);
					double x2=max(m_X1-MX,m_X2-MX);
					Xmin=Xmin*pow(10.0,log10(Xmax/Xmin)*x1/Xlen);
					Xmax=Xmin*pow(10.0,log10(Xmax/Xmin)*x2/Xlen);
				}
			}

			if (is_y_log==0)
			{
				c=Ymax-(Ymax-Ymin)*(double)max(m_Y1,m_Y2)/(double)Ylen;
				d=Ymax-(Ymax-Ymin)*(double)min(m_Y1,m_Y2)/(double)Ylen;
				if (m_Y1==m_Y2) {c=Ymin-(Ymax-Ymin);d=Ymax+(Ymax-Ymin);}

				Ymin=c;
				Ymax=d;
			}
			else
			{
				if (m_Y1==m_Y2) 
				{
					Ymin/=10;
					Ymax*=10;
				}
				else
				{
					double y1=Ylen-max(m_Y1,m_Y2);
					double y2=Ylen-min(m_Y1,m_Y2);
					Ymin=Ymin*pow(10.0,log10(Ymax/Ymin)*y1/Ylen);
					Ymax=Ymin*pow(10.0,log10(Ymax/Ymin)*y2/Ylen);
				}
			}
			((CExpression*)((Base->Items+6)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+6)->pSubdrawing))->GenerateASCIINumber(Ymin,(long long)Ymin,0,5,0);
			((CExpression*)((Base->Items+7)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+7)->pSubdrawing))->GenerateASCIINumber(Ymax,(long long)Ymax,0,5,0);
			((CExpression*)((Base->Items+4)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+4)->pSubdrawing))->GenerateASCIINumber(Xmin,(long long)Xmin,0,5,0);
			((CExpression*)((Base->Items+5)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+5)->pSubdrawing))->GenerateASCIINumber(Xmax,(long long)Xmax,0,5,0);
			this->PlotFunction(1);
			TheState=0;
			if ((SpecialDrawingHover) && (this->any_function_defined==0)) //draw the object
			{
				CDC *DC=pMainView->GetDC();
				pMainView->GentlyPaintObject(SpecialDrawingHover,DC);
				pMainView->ReleaseDC(DC);
			}
			

			return 0;

		}
		if (TheState==3) //moving the plot with mouse finished
		{
			TheState=0;
			int Xlen=(Base->Items)->X2/DRWZOOM-MX;
			int Ylen=(Base->Items+1)->Y2/DRWZOOM-MY;
			double Xmax,Ymax,Xmin,Ymin;
			this->PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);

			if (is_x_log==0)
			{
				int DeltaX=m_X2-m_X1;
				double dx=(Xmax-Xmin)/Xlen*DeltaX;
				Xmin-=dx;
				Xmax-=dx;
			}
			else
			{
				int DeltaX=m_X2-m_X1;
				double fx=pow(10.0,log10(Xmax/Xmin)*(DeltaX)/Xlen);
				Xmax/=fx;
				Xmin/=fx;
			}

			if (is_y_log==0)
			{
				int DeltaY=m_Y2-m_Y1;
				double dy=(Ymax-Ymin)/Ylen*DeltaY;
				Ymin+=dy;
				Ymax+=dy;
			}
			else
			{
				int DeltaY=m_Y2-m_Y1;
				double fy=pow(10.0,log10(Ymax/Ymin)*(DeltaY)/Ylen);
				Ymax*=fy;
				Ymin*=fy;
			}

			((CExpression*)((Base->Items+6)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+6)->pSubdrawing))->GenerateASCIINumber(Ymin,(long long)Ymin,0,5,0);
			((CExpression*)((Base->Items+7)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+7)->pSubdrawing))->GenerateASCIINumber(Ymax,(long long)Ymax,0,5,0);
			((CExpression*)((Base->Items+4)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+4)->pSubdrawing))->GenerateASCIINumber(Xmin,(long long)Xmin,0,5,0);
			((CExpression*)((Base->Items+5)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+5)->pSubdrawing))->GenerateASCIINumber(Xmax,(long long)Xmax,0,5,0);
			this->PlotFunction(1);
		}

		return 0;
	}

	if ((KeyboardEntryObject) && (TheState>=104))
	{
		int i;
		//mouse click at formula editing box (changes the keyboard cursor position)
		tDocumentStruct *ds=TheDocument;
		for (i=0;i<NumDocumentElements;i++,ds++)
			if (ds->Object==(CObject*)Base) break;

		if (i<NumDocumentElements)
		{
			short l,a,b;
			CExpression *tmp=(CExpression*)((Base->Items+TheState-100)->pSubdrawing);
			CDC *DC=pMainView->GetDC();
			tmp->CalculateSize(DC,ViewZoom,&l,&a,&b);
			

			int x0;
			int y0;
			int LeftMargin=MX*ViewZoom/100;
			int BottomMargin=MY*ViewZoom/100;
			//int Xlen=ds->Length;
			//int Ylen=ds->Below;
			int Xlen=abs(Base->Items->X1-Base->Items->X2)/DRWZOOM*ViewZoom/100;
			int Ylen=abs((Base->Items+1)->Y1-(Base->Items+1)->Y2)/DRWZOOM*ViewZoom/100;
			if (TheState==104) // Xmin
			{
				x0=LeftMargin+10;
				y0=Ylen-BottomMargin-a-b-10;
			}
			if (TheState==105) // Xmax
			{
				x0=Xlen-l-10;
				y0=Ylen-BottomMargin-a-b-10;
			}
			if (TheState==106) // Ymin
			{
				x0=LeftMargin+10;
				y0=Ylen-BottomMargin-a-b-10;
			}
			if (TheState==107) // Ymax
			{
				x0=LeftMargin+10;
				y0=10;
			}
			if ((TheState==108) ||
				(TheState==109) ||
				(TheState==110) ||
				(TheState==111)) 
			{
				x0=Xlen-l-20-ToolboxSize/6;
				y0=10+ToolboxSize/6+(TheState-108)*ToolboxSize/3;
			}
			short isexpression;
			char isparenthese;
			CExpression *sel=(CExpression*)tmp->SelectObjectAtPoint(DC,ViewZoom,-X*ViewZoom/100-x0,-Y*ViewZoom/100-y0-a,&isexpression,&isparenthese,2);
			if ((sel) && (isexpression) && (sel->m_Selection) && (sel->m_Selection!=0x7ffff))
			{
				((CExpression*)KeyboardEntryObject)->KeyboardStop();
				if (sel->KeyboardStart(DC,ViewZoom))
					KeyboardEntryObject=(CObject*)sel;
			}
			tmp->DeselectExpression();
			pMainView->ReleaseDC(DC);
		}
	}


	if (!ClipboardExpression)
	{
		int edit_at_position=TheState;
		if (edit_at_position==97)
		{
			//Y ranging
			TheState=96;
			return 1;
		}
		if (edit_at_position==95)
		{
			//X ranging
			TheState=94;
			return 1;
		}
		if (edit_at_position==98)
		{
			//lin/log adjustments

			if ((is_x_log==0) && (is_y_log==0)) 
				is_y_log=1;
			else if ((is_x_log==0) && (is_y_log))
				is_x_log=1;
			else if ((is_x_log) && (is_y_log))
				is_y_log=0;
			else
			{is_x_log=0;is_y_log=0;}

			//nasty trick - the lin/log configuration is stored into m_Alignment
			CExpression *y0=(CExpression*)((Base->Items+6)->pSubdrawing);
			CExpression *x0=(CExpression*)((Base->Items+4)->pSubdrawing);
			if (is_x_log) x0->m_Alignment=1;else x0->m_Alignment=0;//x0->m_FontSize-1; else x0->m_FontSizeHQ=x0->m_FontSize;
			if (is_y_log) y0->m_Alignment=1;else y0->m_Alignment=0;//x0->m_FontSize-1; else y0->m_FontSizeHQ=y0->m_FontSize;

			TheState=0;
			PlotFunction(2);
		}
		if (edit_at_position==99)
		{
			//adjust y range
			TheState=0;
			PlotFunction(2); 
			return 0;
		}
		if ((edit_at_position==0) || (edit_at_position==2) || (edit_at_position==100))
		{
			if (edit_at_position==0) 
				TheState=3;
			if (edit_at_position==2) {TheState=100;}
			if (edit_at_position==100) {TheState=102;}

			m_X2=m_X1=-X;
			m_Y2=m_Y1=-Y;
			return 0;
		}
		if (edit_at_position==15)
		{
			//analyze button -toggle
			analyze=(analyze)?0:1;
			PlotFunction(1);
			return 0;
		}
		if ((edit_at_position==1) && (Base->NumItems>=8))
		{
			//zoom out feature
			double Xmin,Xmax,Ymin,Ymax;
			this->PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);
			if (is_x_log==0)
			{
				double dx=Xmax-Xmin;
				Xmin-=dx;
				Xmax+=dx;
			}
			else
			{
				Xmin/=10.0;
				Xmax*=10.0;
			}

			if (is_y_log==0)
			{
				double dy=Ymax-Ymin;
				Ymin-=dy;
				Ymax+=dy;
			}
			else
			{
				Ymin/=10.0;
				Ymax*=10.0;
			}
			((CExpression*)((Base->Items+6)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+6)->pSubdrawing))->GenerateASCIINumber(Ymin,(long long)Ymin,0,5,0);
			((CExpression*)((Base->Items+7)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+7)->pSubdrawing))->GenerateASCIINumber(Ymax,(long long)Ymax,0,5,0);
			((CExpression*)((Base->Items+4)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+4)->pSubdrawing))->GenerateASCIINumber(Xmin,(long long)Xmin,0,5,0);
			((CExpression*)((Base->Items+5)->pSubdrawing))->Delete();
			((CExpression*)((Base->Items+5)->pSubdrawing))->GenerateASCIINumber(Xmax,(long long)Xmax,0,5,0);

			PlotFunction(1);
			return 0;
		}

		if (Base->NumItems==11)
		{
			//special handling for compatibility - if there is no fourth function expression, make it now
			//(we only had three functions in earlier verisions)
			if (Base->NumItemsReserved<12) 
			{
				Base->NumItemsReserved++;
				Base->Items=(tDrawingItem*)realloc(Base->Items,Base->NumItemsReserved*sizeof(tDrawingItem));
			}
			(Base->Items+Base->NumItems)->pSubdrawing=(void*)new CExpression(NULL,NULL,100);
			(Base->Items+Base->NumItems)->Type=2;
			Base->NumItems++;
		}
		
		if (edit_at_position>=Base->NumItems) return 0;
		if ((Base->Items+edit_at_position)->Type!=2) return 0;

		X=-X;
		Y=-Y;
		int Xlen=(Base->Items)->X2/DRWZOOM;
		int Ylen=(Base->Items+1)->Y2/DRWZOOM;

		//will start the keyboard entry
		KeyboardEntryObject=(CObject*)((Base->Items+edit_at_position)->pSubdrawing);
		((CExpression*)KeyboardEntryObject)->m_FontSize=100;
		((CExpression*)KeyboardEntryObject)->DeselectExpression();
		((CExpression*)KeyboardEntryObject)->m_Selection=(((CExpression*)KeyboardEntryObject)->m_pElementList->Type)?((CExpression*)KeyboardEntryObject)->m_NumElements+1:1;

		for (int i=0;i<NumDocumentElements;i++)
			if ((TheDocument+i)->Object==(CObject*)Base)
			{
				KeyboardEntryBaseObject=TheDocument+i;
				break;
			}
		TheState=edit_at_position+100;
		CDC *DC=pMainView->GetDC();
		((CExpression*)KeyboardEntryObject)->KeyboardStart(DC,ViewZoom);
		((CExpression*)KeyboardEntryObject)->m_Selection=0;
		pMainView->ReleaseDC(DC);
		return 1;
	}

	if (ClipboardExpression)
	{
		int add_at_position=0;
		X=-X;
		Y=-Y;
		int Xlen=(Base->Items)->X2/DRWZOOM;
		int Ylen=(Base->Items+1)->Y2/DRWZOOM;
		if (TheState==4) add_at_position=4;
		if (TheState==5) add_at_position=5;
		if (TheState==6) add_at_position=6;
		if (TheState==7) add_at_position=7;
		if (TheState==8) add_at_position=8;
		if (TheState==9) add_at_position=9;
		if (TheState==10) add_at_position=10;
		if (TheState==11) add_at_position=11;
		if (add_at_position==0)
		{
			//adding at main window - first function adding
			add_at_position=8;
		}

		if (add_at_position==0) return 0;

		//delete the current expression if it exists
		if (Base->NumItems>add_at_position)
		{
			tDrawingItem *di=Base->Items+add_at_position;
			if (di->Type==2) //expression
				delete ((CExpression*)(di->pSubdrawing));
			di->pSubdrawing=ClipboardExpression;
			di->X1=0;di->X2=0;
			di->Y1=0;di->Y2=0;
			di->LineWidth=0;
			ClipboardExpression=NULL;
			if (add_at_position<=7)
				PlotFunction(1);
			else
				PlotFunction(2);
			return 1;
		}
	}
	
	return 1;
}
#pragma optimize("",on)


int CFunctionPlotter::MouseMove(CDC *DC,int X, int Y, UINT flags)
{
	if ((TheState>=104)) return 0; //if keyboard editing limits
	if ((MouseMode!=0) || (IsDrawingMode) ||(X==0x7FFFFFFF)) 
	{	
		if ((X==0x7FFFFFFF) && (TheState))
		{
			MouseClick(0x7FFFFFFF,0x7FFFFFFF); //we will simulate button-up (to finish the current action)
			TheState=0;
			
		}
		return 0;
	}

	if (SpecialDrawingHover==NULL) return 0;
	int mx,my;
	mx=(SpecialDrawingHover->absolute_X-ViewX)*ViewZoom/100;
	my=(SpecialDrawingHover->absolute_Y-ViewY)*ViewZoom/100;

	if (DrawingThreadUsed) return 0;

	if (KeyboardEntryObject) return 0;
	
	if (TheState==100)
	{
		//zoom in mode
		m_X2=m_X1=X;
		m_Y2=m_Y1=Y;
		return 1;
	}

	if (TheState==102)
	{
		//zoom in - drawing the seletion frame
		if (flags&MK_LBUTTON)
		{
			m_X2=X;
			m_Y2=Y;
			return 1;
		}
		return 0;
	}
	
	if (TheState==3)
	{
		//ploter view scrolling (mouse click and drag)
		if (flags&MK_LBUTTON)
		{
			m_X2=X;
			m_Y2=Y;
			return 1;
		}
		else
		{
			m_X1=-1000;
		}

	}
	if (TheState==96)
	{
		//Y ranging
		if (flags&MK_LBUTTON)
		{
			m_Y2=min(Y,SpecialDrawingHover->Below-27);
			return 1;
		}
		else
		{
			m_X1=-1000;
			TheState=0;
		}
	}
	if (TheState==94)
	{
		//X ranging
		if (flags&MK_LBUTTON)
		{
			m_X2=max(X,52);
			return 1;
		}
		else
		{
			m_X1=-1000;
			TheState=0;
		}
	}

	if ((abs(MX-X)<MX/8) && (Y>SpecialDrawingHover->Below-MY))
	{
		//minimum X
		if ((TheState) &&
			(TheState!=4))
		{
			TheState=0;
			Base->PaintDrawing(DC,ViewZoom,mx,my,SpecialDrawingHover->absolute_X,SpecialDrawingHover->absolute_Y);
		}

		TheState=4;
		POINT p[7];
		const int n1=MX*ViewZoom/1000;
		const int n2=MX*ViewZoom/500;
		const int n3=MX*ViewZoom/250;
		p[0].x=mx+MX*ViewZoom/100+1;  p[0].y=my+SpecialDrawingHover->Below*ViewZoom/100-MY*ViewZoom/100;
		p[1].x=p[0].x+n2;		p[1].y=p[0].y+n2;
		p[2].x=p[0].x+n1;		p[2].y=p[0].y+n2;
		p[3].x=p[0].x+n1;		p[3].y=p[0].y+n3;
		p[4].x=p[0].x-n1;		p[4].y=p[0].y+n3;
		p[5].x=p[0].x-n1;		p[5].y=p[0].y+n2;
		p[6].x=p[0].x-n2;		p[6].y=p[0].y+n2;
		DC->SelectObject(GetPenFromPool(1,1));	
		HBRUSH brush=CreateSolidBrush(BLUE_COLOR);
		DC->SelectObject(brush);
		DC->Polygon(p,7);			
		DeleteObject(brush);
	}
	else if ((abs(SpecialDrawingHover->Length-X)<MX/4) && (Y>SpecialDrawingHover->Below-MY))
	{
		//maximum X
		TheState=5;
		POINT p[5];
		const int n1=MX*ViewZoom/1000;
		const int n2=MX*ViewZoom/500;
		const int n3=MX*ViewZoom/250;
		p[0].x=mx+SpecialDrawingHover->Length*ViewZoom/100;  p[0].y=my+SpecialDrawingHover->Below*ViewZoom/100-MY*ViewZoom/100;
		p[0].x-=(Base->Items->LineWidth)*ViewZoom/DRWZOOM/100;
		p[1].x=p[0].x;			p[1].y=p[0].y+n3;
		p[2].x=p[0].x-n1;		p[2].y=p[0].y+n3;
		p[3].x=p[0].x-n1;		p[3].y=p[0].y+n2;
		p[4].x=p[0].x-n2;		p[4].y=p[0].y+n2;
		DC->SelectObject(GetPenFromPool(1,1));			
		HBRUSH brush=CreateSolidBrush(BLUE_COLOR);
		DC->SelectObject(brush);

		DC->Polygon(p,5);
		DeleteObject(brush);

	}
	else if ((abs(SpecialDrawingHover->Below-MY-Y)<MX/8) && (X<MX))
	{
		//minimum Y
		if ((TheState) &&
			(TheState!=6))
		{
			TheState=0;
			Base->PaintDrawing(DC,ViewZoom,mx,my,SpecialDrawingHover->absolute_X,SpecialDrawingHover->absolute_Y);
		}
		TheState=6;
		POINT p[7];
		const int n1=MX*ViewZoom/1000;
		const int n2=MX*ViewZoom/500;
		const int n3=MX*ViewZoom/250;
		p[0].x=mx+MX*ViewZoom/100;  p[0].y=my+SpecialDrawingHover->Below*ViewZoom/100-MY*ViewZoom/100;
		p[0].y-=(Base->Items->LineWidth)*ViewZoom/DRWZOOM/100;
		p[1].x=p[0].x-n2;		p[1].y=p[0].y+n2;
		p[2].x=p[0].x-n2;		p[2].y=p[0].y+n1;
		p[3].x=p[0].x-n3;		p[3].y=p[0].y+n1;
		p[4].x=p[0].x-n3;		p[4].y=p[0].y-n1;
		p[5].x=p[0].x-n2;		p[5].y=p[0].y-n1;
		p[6].x=p[0].x-n2;		p[6].y=p[0].y-n2;
		DC->SelectObject(GetPenFromPool(1,1));
		HBRUSH brush=CreateSolidBrush(BLUE_COLOR);
		DC->SelectObject(brush);
		DC->Polygon(p,7);	
		DeleteObject(brush);

	}
	else if ((abs(Y)<MX/4) && (X<MX))
	{
		//maximum Y
		TheState=7;
		POINT p[7];
		const int n1=MX*ViewZoom/1000;
		const int n2=MX*ViewZoom/500;
		const int n3=MX*ViewZoom/250;
		p[0].x=mx+MX*ViewZoom/100;  p[0].y=my;
		p[0].y+=(Base->Items->LineWidth)*ViewZoom/DRWZOOM/100;
		p[1].x=p[0].x-n2;		p[1].y=p[0].y+n2;
		p[2].x=p[0].x-n2;		p[2].y=p[0].y+n1;
		p[3].x=p[0].x-n3;		p[3].y=p[0].y+n1;
		p[4].x=p[0].x-n3;		p[4].y=p[0].y;
		DC->SelectObject(GetPenFromPool(1,1));			
		HBRUSH brush=CreateSolidBrush(BLUE_COLOR);
		DC->SelectObject(brush);
		DC->Polygon(p,5);
		DeleteObject(brush);
	}
	else if ((Y<SpecialDrawingHover->Below-MY) && (X>MX))
	{
		//inside the view section
		int new_mode=0;
		if (!ClipboardExpression)
		{
			if ((Y<ToolboxSize*30/ViewZoom) && (X<MX+ToolboxSize*66/ViewZoom) && (X>MX+ToolboxSize*33/ViewZoom))
				new_mode=1; //zoom out
			if ((Y<ToolboxSize*30/ViewZoom) && (X>MX) && (X<MX+ToolboxSize*33/ViewZoom))
				new_mode=2;
			if (any_function_defined)
			{
				if ((Y<ToolboxSize*30/ViewZoom) && (X>MX+ToolboxSize*66/ViewZoom) && (X<MX+ToolboxSize*100/ViewZoom))
					new_mode=99;
				if ((Y<ToolboxSize*30/ViewZoom) && (X>MX+ToolboxSize*100/ViewZoom) && (X<MX+ToolboxSize*133/ViewZoom))
					new_mode=15;
			}
		}
		if ((X>SpecialDrawingHover->Length-ToolboxSize*30/ViewZoom) && (Y>0) && (Y<ToolboxSize*33/ViewZoom))
			new_mode=8;		
		if ((X>SpecialDrawingHover->Length-ToolboxSize*30/ViewZoom) && (Y>ToolboxSize*33/ViewZoom) && (Y<ToolboxSize*66/ViewZoom))
			new_mode=9;	
		if ((X>SpecialDrawingHover->Length-ToolboxSize*30/ViewZoom) && (Y>ToolboxSize*66/ViewZoom) && (Y<ToolboxSize*100/ViewZoom))
			new_mode=10;
		if ((X>SpecialDrawingHover->Length-ToolboxSize*30/ViewZoom) && (Y>ToolboxSize*100/ViewZoom) && (Y<ToolboxSize*133/ViewZoom))
			new_mode=11;
		if (TheState!=new_mode) {TheState=new_mode;return 1;}
		return 0;
	}
	else if ((Y>SpecialDrawingHover->Below-ToolboxSize*30/ViewZoom) && (X<ToolboxSize*30/ViewZoom) && (!ClipboardExpression))
	{
		//lin/log button
		TheState=98;
		return 1;
	}
	else if ((X<MX) && (Y<SpecialDrawingHover->Below-MY))
	{
		//Y ranging
		TheState=97;
		m_Y2=Y;
		m_Y1=Y;
		return 1;
		
	}
	else if ((Y>SpecialDrawingHover->Below-MY) && (X>MX))
	{
		//X ranging
		TheState=95;
		m_X2=X;
		m_X1=X;
		return 1;
		
	}
	else
	{
		if ((TheState) && (KeyboardEntryObject==NULL))
		{
		
			TheState=0;	
			return 1;
		}

	}

	return 0;
}

int CFunctionPlotter::XML_output(char * output, int num_tabs, char only_calculate)
{
	return 0;
}

char *CFunctionPlotter::XML_input(char * file)
{
	return file;
}


//this function plots the mathematical function
//first four items are the frame
//inputs: reset_plot-(0-plot the funciton, 1 - prepare no Y ranging, 2-prepare with Y ranging);  PrintDC - when printing the device context, or NULL
int CFunctionPlotter::PlotFunction(int reset_plot, CDC *PrintDC,short ViewZoom)
{
	if (ViewZoom==0) 
		ViewZoom=::ViewZoom;
	int Xlen;
	int Ylen;
	if (reset_plot)
	{
		if ((MouseMode==9) || (MouseMode==11)) return 1; //steretching or rotating
		if (ViewZoom<13) return 1;
		//wait max 2 seconds until the current plotting finishes
		int cnt=0;
		while (ThreadHandle)
		{
			abort_request=1;
			Sleep(10);
			if (cnt>200) 
			{
				TerminateThread(ThreadHandle,0);
				if (DrawingThreadUsed>0) DrawingThreadUsed--;
				ThreadHandle=NULL;
				break;
			}
			cnt++;
		}
		abort_request=0;
		if (reset_plot==2) calc_y=1;

		//get the dimensions of the function ploter frame
		Xlen=abs(Base->Items->X1-Base->Items->X2)/DRWZOOM*ViewZoom/100;
		Ylen=abs((Base->Items+1)->Y1-(Base->Items+1)->Y2)/DRWZOOM*ViewZoom/100;

		//reduce for size of 'borders' (coordinate system)
		Xlen-=MX*ViewZoom/100;
		Ylen-=MY*ViewZoom/100;

		//delete the previous bitmap, if dimensions are different
		if (Plot)
		{
			CSize sz=Plot->GetBitmapDimension();
			if ((sz.cx!=Xlen) || (sz.cy!=Ylen))
			{
				delete Plot;
				Plot=NULL;
			}
		}

		//create the new bitmap if already not exists
		CDC *DC=pMainView->GetDC();
		if (Plot==NULL)
		{
			Plot=new CBitmap();
			Plot->CreateCompatibleBitmap(DC,Xlen,Ylen);
			Plot->SetBitmapDimension(Xlen,Ylen);
		}
		//clear the plot bitmap
		CDC mDC;
		if (PrintDC) mDC.CreateCompatibleDC(PrintDC); else mDC.CreateCompatibleDC(DC);
		CBitmap *prev_bitmap=mDC.SelectObject(Plot);
		mDC.FillSolidRect(0,0,Xlen,Ylen,RGB(255,255,255));
		mDC.SelectObject(prev_bitmap);
		pMainView->ReleaseDC(DC);

		if (PrintDC==NULL)
		{
			//start the plotting thread (we are plotting in background except if we are printing)
			SECURITY_ATTRIBUTES sa;
			sa.bInheritHandle=FALSE;
			sa.lpSecurityDescriptor=NULL;
			sa.nLength=sizeof(sa);
			ThreadHandle=CreateThread(&sa,0,DrawingThread,(LPVOID)this,0,&(ThreadID));
			return 1;
		}
	}

	any_function_defined=0;

	//check if the function is defined (first four elements are the frame, next four elements are minimum/maximum limits)
	if (Base->NumItems<9) return 0;


	//find this object (function plotter) in the main document
	int absX,absY;
	tDocumentStruct *ds=TheDocument;
	int i_cnt;
	for (i_cnt=0;i_cnt<NumDocumentElements;i_cnt++,ds++)
		if (ds->Object==(CObject*)Base)
		{
			absX=ds->absolute_X;
			absY=ds->absolute_Y;
			break;
		}
	if (i_cnt==NumDocumentElements) return 0; //this object was not found


	//getting bittmap dimensions
	CSize cs=Plot->GetBitmapDimension();
	Xlen=cs.cx;
	Ylen=cs.cy;	


	
	CExpression *Func[4];
	tDrawingItem *pItem=Base->Items+8;
	int any_found=0;
	for (int kk=0;kk<4;kk++,*pItem++)
	{
		Func[kk]=NULL;
		if (Base->NumItems>=kk+9)  
		{
			if ((pItem->pSubdrawing) && 
				(((CExpression*)(pItem->pSubdrawing))->IsSuitableForComputation()) &&
				(((CExpression*)(pItem->pSubdrawing))->m_pElementList->Type!=0))
			{
				Func[kk]=new CExpression(NULL,NULL,100);
				Func[kk]->CopyExpression((CExpression*)(pItem->pSubdrawing),0);
				any_found=1;
			}
		}
	}
	if (!any_found) return 0;

	any_function_defined=1;
	

	//create compatible DC 
	CDC *DC=pMainView->GetDC();
	CDC mDC;
	if (PrintDC) mDC.CreateCompatibleDC(PrintDC); else mDC.CreateCompatibleDC(DC);
	pMainView->ReleaseDC(DC);
	mDC.SelectObject(GetStockObject(BLACK_PEN));

	
	//how precise we will calculate
	int density=2;
	if (analyze) density=5;


	//prepare for calculation of Y values
	void *VarPos[4];
	for (int kk=0;kk<4;kk++)
	{
		if (Func[kk]) 
		{
			VarPos[kk]=Func[kk]->PlotterPrepareVariablePositions();
			if (VarPos[kk]==NULL) 
			{
				delete Func[kk];
				Func[kk]=NULL;
			}
		}
	}


	//create an arrays that will hold Y calculated values
	double *YY[4];
	for (int kk=0;kk<4;kk++)
	{
		if (Func[kk]) YY[kk]=new double[(Xlen+5)*density];
	}
	double Xmax,Xmin,Ymax,Ymin;
	PlotFunctionGetBondaries(&Xmin,&Xmax,&Ymin,&Ymax);

	//we will calculate Y values at least for every X
	show_no_scale=1;
	CPen pen1(PS_SOLID,max(ViewZoom/200,1),RGB(0,0,0));
	CPen pen2(PS_SOLID,max(ViewZoom/200,1),RGB(0,192,0));
	CPen pen3(PS_SOLID,max(ViewZoom/200,1),RGB(192,0,0));
	CPen pen4(PS_SOLID,max(ViewZoom/200,1),RGB(0,0,192));
	
	double *sort_buff=(double *)malloc(sizeof(double)*Xlen*density);
	int start_drawing_point=0;
	double prevy[4];
	prevy[3]=prevy[2]=prevy[1]=prevy[0]=sqrt(-1.0);
	double f1=(Xmax-Xmin)/(double)Xlen/(double)density;
	double f2=log10(Xmax/Xmin)/(double)Xlen/(double)density;
	
	for (int i=0;i<Xlen*density;i++)
	{
		//we will suspend the plotting if creen is scrolling or some other action is being done
		while ((GetKeyState(VK_RBUTTON)&0x8000) || 
		  (GetKeyState(33)&0x8000) ||
		  (GetKeyState(34)&0x8000) ||
		  (GetKeyState(VK_LEFT)&0x8000) ||
		  (GetKeyState(VK_RIGHT)&0x8000) ||
		  (GetKeyState(VK_UP)&0x8000) ||
		  (GetKeyState(VK_DOWN)&0x8000))
		{
			if (abort_request) break;
			Sleep(10);
		}

		//compute 'y' value in this point 'x'
		for (int kk=0;kk<4;kk++)
		{
			if (abort_request) break;
			if (Func[kk])
			{
				double X;
				if (is_x_log==0)
					X=((double)i+0)*f1+Xmin;
				else
					X=Xmin*pow(10.0,i*f2);
				*(YY[kk]+i)=Func[kk]->PlotterCalculateFunctionValue(X,VarPos[kk]);
			}
		}

		//if abort is requested we finish
		if (abort_request) 
		{
			i=density*Xlen-1;Sleep(150); //will force finish (why do we need to sleep?)
		} 

		//from time to type refresh the plotter (we don't do it very often because is slow)
		if (((((!analyze) && (i%32==31)) || ((analyze) && (i%64==63))) && (PrintDC==NULL)) || (i==density*Xlen-1))
		{
			if (i==density*Xlen-1) show_no_scale=0;

			CBitmap *prev_bitmap=mDC.SelectObject(Plot);

			//if we need to auto-adjust y scale, we will do this form time to time
			if ((calc_y) && ((i%128==127) || (i==density*Xlen-1)))
			{
				mDC.FillSolidRect(0,0,Xlen,Ylen,RGB(255,255,255));
				Ymax=-1e+100;
				Ymin=1e+100;
				for (int kk=0;kk<4;kk++)
					if (Func[kk])
					{
						memcpy(sort_buff,YY[kk],sizeof(double)*i);
						qsort((void*)sort_buff,i,sizeof(double), double_compare );
						int k=0;
						while ((_isnan(sort_buff[k])) && (k<i)) k++;
						if (k<i)
						{
							double vall;
							vall=sort_buff[k+(i-k)/20];if (vall<Ymin) Ymin=vall;
							vall=sort_buff[k+(i-k)*19/20];if (vall>Ymax) Ymax=vall;
						}
					}

				if (Ymax<Ymin) {Ymax=100.0;Ymin=0.0;} //should not happen
				else if (Ymax-Ymin<1e-14) Ymax=Ymin+1e-14;

				double span=Ymax-Ymin;
				if (is_y_log==0)
				{
					Ymin-=span*0.1;
					Ymax+=span*0.1;
				}
				else
				{
					Ymin*=0.6;
					Ymax*=1.5;
					if (Ymax<0) Ymax=1;
					if (Ymin<0) Ymin=Ymax/1e+10;
				}
				if (Ymax-Ymin<1e-48) 
				{
					Ymax*=1.01; Ymin*=0.99;
				}
			
				((CExpression*)((Base->Items+6)->pSubdrawing))->Delete();
				((CExpression*)((Base->Items+6)->pSubdrawing))->GenerateASCIINumber(Ymin,(long long)Ymin,0,5,0);
				((CExpression*)((Base->Items+7)->pSubdrawing))->Delete();
				((CExpression*)((Base->Items+7)->pSubdrawing))->GenerateASCIINumber(Ymax,(long long)Ymax,0,5,0);
				start_drawing_point=0;
				prevy[2]=prevy[1]=prevy[0]=sqrt(-1.0);
			}

			//compute scaling factors
			double F1=(is_y_log==0)?((double)Ylen/(double)(Ymax-Ymin)):((double)Ylen/log10(Ymax/Ymin));
			//double F2=(double)Ylen/log10(Ymax/Ymin);

			//now we start drawing pre-calculated points into the plotter bitmap
			if (!abort_request)
			for (int j=start_drawing_point;j<i;j++)
				for (int kk=0;kk<4;kk++)
					if (Func[kk])
					{
						double Y=(is_y_log==0)?((double)(*(YY[kk]+j)-Ymin)):(log10(*(YY[kk]+j))-log10(Ymin));
						Y*=F1;

						//if we are analyzing this function, then check for special points (minimums, maximums, intersections...)
						int special_point=0;
						double Y2=Y;
						if (analyze)
						{
							
							if ((j>1) && (j<i-1))
							{
								double n,a,b,c,d;
								n=*(YY[kk]+j-2);
								a=*(YY[kk]+j-1);
								b=*(YY[kk]+j);
								c=*(YY[kk]+j+1);
								d=*(YY[kk]+j+2);
								if ((b>a) && (b>c)) special_point=1; //maximum
								if ((b<a) && (b<c)) special_point=2; //minimum
								
								if ((a<n) && (a<b) && (special_point==1)) special_point=0; 
								if ((a>n) && (a>b) && (special_point==2)) special_point=0;
								if ((c<b) && (c<d) && (special_point==1)) special_point=4; //break point
								if ((c>b) && (c>d) && (special_point==2)) special_point=4; //break point

								//check for intersections
								for (int zz=kk+1;zz<4;zz++)
								if (Func[zz])
								{
									double a2,b2,c2;
									a2=*(YY[zz]+j-1);
									b2=*(YY[zz]+j);
									c2=*(YY[zz]+j+1);

									if (((a2>a) && (b2<b)) || ((a2<a) && (b2>b)))
									if (fabs(b2-b)<=fabs(a2-a))
									{
										special_point=3; //intersection
										if (is_y_log==0)
										{
											double aa=(b-a)*F1;
											double bb=fabs(b2-b)/fabs(a2-a);
											double cc=aa*bb;
											Y2=Y-cc/2;
										}
									}

									if (((c2>c) && (b2<b)) || ((c2<c) && (b2>b)))
									if (fabs(b2-b)<=fabs(c2-c))
									{
										special_point=3; //intersection
										if (is_y_log==0)
										{
											double aa=(c-b)*F1;
											double bb=fabs(b2-b)/fabs(c2-c);
											double cc=aa*bb;
											Y2=Y+cc/2;
										}
									}
								}
							}
						}

						if (Y>16000) Y=16000;
						if (Y<-16000) Y=-16000;
						if ((!_isnan(Y)) && (!_isnan(prevy[kk])))
						{
							int X=j/density;
							int Xp=(j-1)/density;
							mDC.SelectObject((kk==0)?pen1:((kk==1)?pen2:((kk==2)?pen3:pen4)));
							mDC.MoveTo(Xp,Ylen-(int)prevy[kk]);
							mDC.LineTo(X,Ylen-(int)Y);

							if (special_point) //if analysis was enabled, then we also paint minimum/maximum/intersection...
							{
								int zed=Ylen-(int)Y2;
								if (special_point==4) //paint doted vertical line when we detected that function has a break
								{
									CPen pen4(PS_DOT,max(ViewZoom/200,1),RGB(255,0,0));
									mDC.SelectObject(pen4);
									mDC.MoveTo(X,0);
									mDC.LineTo(X,Ylen);
								}
								else //paint colored point
									mDC.FillSolidRect(X-2,zed-2,4,4,RGB(255,0,(special_point==3)?255:0));
					
								int fontsz=7+MX*ViewZoom/1000;
								mDC.SelectObject(GetFontFromPool(4,0,0,fontsz));

								char txt[64];
								txt[0]=0;
							
								double Xd;
								if (is_x_log==0)
								{
									Xd=(double)j*f1+Xmin;
									if (special_point==4) Xd+=f1/2;
									ShowNumberWithPrecision(Xd,f1,txt);
								}
								else
								{
									Xd=Xmin*pow(10.0,j*f2);
									double zed2=Xmin*pow(10.0,(j+1)*f2)-Xmin*pow(10.0,j*f2);
									ShowNumberWithPrecision(Xd,zed2,txt);
								}
								char txt2[64];
								txt2[0]=0;

								double Yd=*(YY[kk]+j);
								double Ydd=fabs(*(YY[kk]+j-1)-Yd);
								if (special_point!=4) 
								{
									if (is_y_log==0)
										ShowNumberWithPrecision(Yd,max(Ydd,1/F1/density),txt2);
									else
										ShowNumberWithPrecision(Yd,Yd/100,txt2);
								}
								mDC.SetTextAlign(TA_CENTER);
								mDC.SetBkMode(TRANSPARENT);
								if (special_point==1) //maximum
								{
									mDC.TextOut(X,Ylen-(int)Y-6-7*fontsz/4,txt);
									mDC.TextOut(X,Ylen-(int)Y-6-fontsz,txt2);
								}
								if (special_point==2) //minimum
								{
									mDC.TextOut(X,Ylen-(int)Y+3+fontsz/2,txt);
									mDC.TextOut(X,Ylen-(int)Y+3+5*fontsz/4,txt2);
								}
								if (special_point==3) //intersection
								{
									mDC.SetTextAlign(TA_RIGHT);
									mDC.TextOut(X-1,Ylen-(int)Y2,txt);
									mDC.TextOut(X-1,Ylen-(int)Y2+3*fontsz/4,txt2);
								}
								if (special_point==4) //curve break (vertical asymptote)
								{
									mDC.SetTextAlign(TA_RIGHT);
									mDC.TextOut(X-1,Ylen-6-fontsz,txt);
								}
							}
						}
						prevy[kk]=Y;
					}
				
			start_drawing_point=i-2;

			if ((analyze) && (i==density*Xlen-1) && (!abort_request) && (PrintDC==NULL) && (Func[0]) && (is_x_log==0))
			{
				//print out the numerically computed integral of the first function
				double Integral=0;
				int ok=1;
				for (int j=0;j<=i;j++)
				if (!_isnan(*(YY[0]+j)))
				{
					if (fabs(*YY[0]+j)>1e+36) ok=0; //we had unusually high numbers - we cannot trust the result any more
					if (j<i)
						Integral+=(*(YY[0]+j)+*(YY[0]+j+1))*f1/2.0;
					else
						Integral+=*(YY[0]+j)*f1;
				}

				if (ok)
				{
					int fontsz=7+MX*ViewZoom/1000;
					mDC.SelectObject(GetFontFromPool(4,0,0,fontsz));
					mDC.SetTextAlign(TA_RIGHT);
					mDC.SetBkMode(TRANSPARENT);
					mDC.SetTextColor(0);
					char tt[64];strcpy(tt,"Int=");
					ShowNumberWithPrecision(Integral,f1,tt);
					mDC.TextOut(Xlen-2,Ylen-3-fontsz,tt);
				}
			}

			mDC.SelectObject(prev_bitmap);
			if (PrintDC==NULL)
			{
				CDC *DC=pMainView->GetDC();
				Base->PaintDrawing(DC,ViewZoom,(absX-ViewX)*ViewZoom/100,(absY-ViewY)*ViewZoom/100,absX,absY);
				pMainView->ReleaseDC(DC);
				Sleep(10);
			}
		}
	}

	free(sort_buff);

	for (int kk=0;kk<4;kk++)
		if (Func[kk])
		{
			delete YY[kk];
			Func[kk]->PlotterReleaseVariablePositions(VarPos[kk]);
			delete Func[kk];
		}
	mDC.DeleteDC();

	show_no_scale=0;
	calc_y=0;
	return 0;
}

int CFunctionPlotter::PlotFunctionGetBondaries(double *Xmin, double *Xmax, double *Ymin, double *Ymax)
{
	*Xmax=*Ymax=100;
	*Xmin=*Ymin=0;
	if (Base->NumItems>=6)
	{	
		int prec2;
		CExpression *x=(CExpression*)((Base->Items+5)->pSubdrawing);
		if (x->m_pElementList->Type)
		if (!(x->IsPureNumber(0,x->m_NumElements,Xmax,&prec2)))
		{
			tPureFactors PF;
			PF.N1=1.0;
			PF.N2=1.0;
			PF.is_frac1=0;
			PF.prec1=0;
			x->StrikeoutCommonFactors(0,x->m_NumElements-1,1,NULL,0,0,1,&PF);
			x->StrikeoutRemove(0,x->m_NumElements-1);
			*Xmax=PF.N1/PF.N2;
		}
	}
	if (Base->NumItems>=5)
	{	
		int prec2;
		CExpression *x=(CExpression*)((Base->Items+4)->pSubdrawing);
		if (x->m_pElementList->Type)
		if (!(x->IsPureNumber(0,x->m_NumElements,Xmin,&prec2)))
		{
			tPureFactors PF;
			PF.N1=1.0;
			PF.N2=1.0;
			PF.is_frac1=0;
			PF.prec1=0;
			x->StrikeoutCommonFactors(0,x->m_NumElements-1,1,NULL,0,0,1,&PF);
			x->StrikeoutRemove(0,x->m_NumElements-1);
			*Xmin=PF.N1/PF.N2;
		}
	}
	if (Base->NumItems>=8)
	{	
		int prec2;
		CExpression *x=(CExpression*)((Base->Items+7)->pSubdrawing);
		if (x->m_pElementList->Type)
		if (!(x->IsPureNumber(0,x->m_NumElements,Ymax,&prec2)))
		{
			tPureFactors PF;
			PF.N1=1.0;
			PF.N2=1.0;
			PF.is_frac1=0;
			PF.prec1=0;
			x->StrikeoutCommonFactors(0,x->m_NumElements-1,1,NULL,0,0,1,&PF);
			x->StrikeoutRemove(0,x->m_NumElements-1);
			*Ymax=PF.N1/PF.N2;
		}
	}
	if (Base->NumItems>=7)
	{	 
		int prec2;
		CExpression *x=(CExpression*)((Base->Items+6)->pSubdrawing);
		if (x->m_pElementList->Type)
		if (!(x->IsPureNumber(0,x->m_NumElements,Ymin,&prec2)))
		{
			tPureFactors PF;
			PF.N1=1.0;
			PF.N2=1.0;
			PF.is_frac1=0;
			PF.prec1=0;
			x->StrikeoutCommonFactors(0,x->m_NumElements-1,1,NULL,0,0,1,&PF);
			x->StrikeoutRemove(0,x->m_NumElements-1);
			*Ymin=PF.N1/PF.N2;
		}
	}

	if (*Xmin>=*Xmax) *Xmin=*Xmax-1;
	if (*Ymin>=*Ymax) *Ymin=*Ymax-1;

	if (is_x_log)
	{
		if (*Xmax<=1e-100) *Xmax=1;
		if (*Xmin<=1e-100) *Xmin=*Xmax/1e+10;
	}
	if (is_y_log)
	{
		if (*Ymax<=1e-100) *Ymax=1;
		if (*Ymin<=1e-100) *Ymin=*Ymax/1e+10;
	}
	return 1;
}


int CFunctionPlotter::ShowNumberWithPrecision(double number, double precision,char *string)
{
	precision=fabs(precision);
	if (precision<1e-100) precision=1e-100;

	if (fabs(number)/precision>1e+50) precision=fabs(number)/(1e+20);

	int qualifier=0;
	while ((fabs(number)>=1000.0) && (precision>10.0))
	{
		qualifier++;
		number/=1000.0;
		precision/=1000.0;
	}

	if ((fabs(number)<0.001) && (fabs(number)>1e-100))
	{
		while (fabs(number)<1.0)
		{
			qualifier--;
			number*=1000.0;
			precision*=1000.0;
		}
	}

	char t[64];
	int x=(int)log10(precision);
	if ((x>0) || ((x==0) && (fabs(number)<=fabs(precision))))
	{
		if (fabs(number)<precision) number=0;
		double tmp=pow(10.0,(double)x);
		number/=tmp;
		if (fabs(number)<1) number=0;
		sprintf(t,"%.0lf",number);
		for (int i=0;i<x;i++)
			strcat(t,"0");
	}
	else
	{
		char format[48];
		sprintf(format,"%%.%dlf",min(14,-x));
		sprintf(t,format,number);
	}


	int isminus=0;
	if (t[0]=='-') {isminus=1;memmove(t,t+1,strlen(t));}

	//leading zeros
	while ((t[0]=='0') && (t[1]!=0)) 
	{
		memmove(t+0,t+1,47);
		t[47]=0;
	}

	if (t[0]=='.') {memmove(t+1,t,strlen(t)+1);t[0]='0';}
	if (t[strlen(t)-1]=='.') strcat(t,"0");

	int allzeros=1;
	int kk=0;
	while (t[kk])
	{
		if ((t[kk]!='0') && (t[kk]!='.')) allzeros=0;
		kk++;
	}

	if (isminus) strcat(string,"-");
	strcat(string,t);

	if (!allzeros)
	{
		if (qualifier==1) strcat(string,"k");
		if (qualifier==2) strcat(string,"M");
		if (qualifier==3) strcat(string,"G");
		if (qualifier>=4)
		{
			char ttt[10];
			sprintf(ttt,"+e%d",qualifier*3);
			strcat(string,ttt);
		}
		if (qualifier==-1) strcat(string,"m");
		if (qualifier==-2) strcat(string,"u");
		if (qualifier==-3) strcat(string,"n");
		if (qualifier==-4) strcat(string,"p");
		if (qualifier<=-5)
		{
			char ttt[10];
			sprintf(ttt,"-e%d",(-qualifier)*3);
			strcat(string,ttt);
		}
	}
	return 1;
}

