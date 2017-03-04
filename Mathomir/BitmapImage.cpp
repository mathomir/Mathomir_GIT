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
#include "atlimage.h"
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

//constructor
CBitmapImage::CBitmapImage(CDrawing *BaseItem)
{
	Base=BaseItem;
	Image=NULL;
	imgsize=0;
	ShowMenu=0;
	SelectedItem=0;
	MenuX=0;
	MenuY=0;
	editing=0;
}

//destructor (should release all previously reserved memory)
CBitmapImage::~CBitmapImage(void)
{
	if (Image) free(Image);
}

//Copies relevant data from another object of the same type (this is used
//when a copy of an existing object is to be made)
int CBitmapImage::CopyFrom(CDrawing *Original)
{
	if (Original->IsSpecialDrawing!=52) return 0;
	CBitmapImage *org=(CBitmapImage*)Original->SpecialData;
	if (!org) return 0;

	ShowMenu=0;
	SelectedItem=0;
	MenuX=0;
	MenuY=0;
	editing=0;

	if (org->Image)
	{
		Image=(char*)malloc(org->imgsize);
		imgsize=org->imgsize;
		memcpy(Image,org->Image,org->imgsize);
	}
	else
	{
		Image=NULL;
		imgsize=0;
	}

	return 1;
}

//Paints the object interior (painting starts from X,Y coordinates - upper left corner)
#pragma optimize("s",on)
int CBitmapImage::Paint(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg)
{
	X+=min(Base->Items->X1,Base->Items->X2)/DRWZOOM*zoom/100+1;
	Y+=min((Base->Items+1)->Y1,(Base->Items+1)->Y2)/DRWZOOM*zoom/100+1;
	int Xlen=abs(Base->Items->X1-Base->Items->X2)/DRWZOOM*zoom/100;
	int Ylen=abs((Base->Items+1)->Y1-(Base->Items+1)->Y2)/DRWZOOM*zoom/100;	
	X+=Base->Items->LineWidth/DRWZOOM*zoom/100;
	Y+=Base->Items->LineWidth/DRWZOOM*zoom/100;
	Xlen-=Base->Items->LineWidth/DRWZOOM*zoom/200;
	Ylen-=Base->Items->LineWidth/DRWZOOM*zoom/200;
	MenuX=min(Base->Items->X1,Base->Items->X2)/DRWZOOM*ViewZoom/100+1+Base->Items->LineWidth/DRWZOOM*ViewZoom/100;
	MenuY=min((Base->Items+1)->Y1,(Base->Items+1)->Y2)/DRWZOOM*ViewZoom/100+1+Base->Items->LineWidth/DRWZOOM*ViewZoom/100;

	if (Image)
	{
		BITMAPINFO *binfo=(BITMAPINFO*)Image;
		BITMAPINFOHEADER *bhead=(BITMAPINFOHEADER*)Image;
		int colors=bhead->biClrUsed;
		if (bhead->biBitCount!=24) colors=1<<bhead->biBitCount;
		int tablesize=colors*sizeof(RGBQUAD);
		void *bits=Image+tablesize+sizeof(BITMAPINFOHEADER);
		StretchDIBits(DC->m_hDC,X,Y,Xlen,Ylen,0,0,bhead->biWidth,bhead->biHeight,bits,binfo,DIB_RGB_COLORS,SRCCOPY);
	}
	else
	{
		for (int ii=0;ii<8;ii++)
			for (int jj=0;jj<8;jj++)
				DC->FillSolidRect(X+ii*Xlen/8,Y+jj*Ylen/8,Xlen/8,Ylen/8,((ii+jj)%2)?RGB(255,255,255):((Xlen<30)?RGB(208,208,208):RGB(240,240,240)));
	}

	if ((ShowMenu) && (!ViewOnlyMode))
	{
		DC->SetTextAlign(TA_LEFT | TA_TOP);
		DC->SetBkColor(RGB(240,240,240));
		DC->SetBkMode(TRANSPARENT);
		DC->SelectObject(GetFontFromPool(4,0,0,13));
		char str1[30];
		char str2[30];
		char str3[30];
		char str4[30];
		CopyTranslatedString(str1,"Update", 5072,29);
		CopyTranslatedString(str2,"Edit",5070,29);
		CopyTranslatedString(str3,"Load",5071,29);
		CopyTranslatedString(str4,"Cancel",0,29);

		int sel=SelectedItem;
		DC->FillSolidRect(X,Y,45,26,RGB(240,240,240));
		DC->SetTextColor((sel==1)?BLUE_COLOR:0);
		DC->TextOut(X,Y,(editing)?str1:str2);
		if (sel==1) DC->TextOut(X+1,Y,(editing)?str1:str2);
		DC->SetTextColor((sel==2)?BLUE_COLOR:0);
		DC->TextOut(X,Y+12,(editing)?str4:str3);
		if (sel==2) DC->TextOut(X+1,Y+12,(editing)?str4:str3);
		BITMAPINFOHEADER *bhead=(BITMAPINFOHEADER*)Image;
		if (bhead)
		{
			if ((Base->Items+1)->Y2>(40*DRWZOOM))
				if ((abs(bhead->biWidth-(Base->Items->X2-Base->Items->X1-2*Base->Items->LineWidth)/DRWZOOM)>1) ||
					(abs(bhead->biHeight-((Base->Items+1)->Y2-(Base->Items+1)->Y1-2*Base->Items->LineWidth)/DRWZOOM)>1))
			{
				DC->FillSolidRect(X,Y+24,45,14,RGB(240,240,240));
				DC->SetTextColor((sel==3)?BLUE_COLOR:0);
				DC->TextOut(X,Y+24,"1:1",3);
				if (sel==3) DC->TextOut(X+1,Y+24,"1:1",3);
			}
		}
	}

	return 1;
}

#pragma optimize("s",on)
int CBitmapImage::MouseClick(int X, int Y)
{
	if (X!=0x7FFFFFFF)
	if (ShowMenu)
	{
		if (SelectedItem==1) //the EDIT or UPDATE was choosen from menu
		{
			if (editing) //update
			{
				editing=0;
				char filename[340];
				GetTempPath(340,filename);
				if (filename[strlen(filename)-1]!='\\') strcat(filename,"\\");
				strcat(filename,"MoM_bmp.bmp");
				LoadImageFromFile((CObject*)Base,filename);
			}
			else 
			{
				//create its empty image if does not exist already
				if (Image==NULL) LoadImageFromFile((CObject*)Base,NULL);
			
				if (SaveImageToFileForEditing((CObject*)Base))
				{
					for (int i=0;i<NumDocumentElements;i++)
						if ((TheDocument[i].Type==2) && (TheDocument[i].Object))
						{
							CDrawing *d=(CDrawing*)TheDocument[i].Object;
							if (d->IsSpecialDrawing==52) ((CBitmapImage*)(d->SpecialData))->editing=0;
						}
					editing=1;
				}
			}
		}
		if (SelectedItem==2) //the LOAD/CANCEL was choosen from menu
		{
			if (editing) //cancel
			{
				editing=0;
			}
			else //cancel
			{
				int i=0;
				char *filter;
				filter="BMP files|*.BMP|JPG files|*.jpg|PNG files|*.PNG|All files|*.*||\0";
				CFileDialog fd(TRUE,"bmp",NULL,OFN_HIDEREADONLY,filter,theApp.m_pMainWnd,0);
				if (fd.DoModal()==IDOK)
				{
					LoadImageFromFile((CObject*)Base,fd.m_pOFN->lpstrFile);		
					editing=0;
				}
			}
		}
		if (SelectedItem==3) //1:1 button was choosen
		{
			//doing stuff here
			BITMAPINFOHEADER *bhead=(BITMAPINFOHEADER*)Image;
			if (bhead)
			{
				int orgx=bhead->biWidth;
				int orgy=bhead->biHeight;

				(Base->Items+0)->X2=(Base->Items+0)->X1+orgx*DRWZOOM+(2*Base->Items->LineWidth);
				(Base->Items+1)->X1=(Base->Items+1)->X2=(Base->Items+0)->X2;
				(Base->Items+2)->X1=(Base->Items+1)->X2;

				(Base->Items+1)->Y2=(Base->Items+1)->Y1+orgy*DRWZOOM+(2*Base->Items->LineWidth);
				(Base->Items+2)->Y1=(Base->Items+2)->Y2=(Base->Items+1)->Y2;
				(Base->Items+3)->Y1=(Base->Items+2)->Y2;
				pMainView->RepaintTheView();
			}
		}
	}
	return 1;
}

#pragma optimize("s",on)
int CBitmapImage::MouseMove(CDC *DC,int X, int Y, UINT flags)
{
	if (Base->IsSelected==0) 
	{
		if (ShowMenu)
		{
			ShowMenu=0;
			return 1; //this wil redraw the bitmap object
			//pMainView->RepaintTheView();
		}
		else
			return 0; //no redraw neccessary
	}
	else
	{
		SelectedItem=0;
		if (X!=0x7FFFFFFF)
		if ((X*ViewZoom/100<45+MenuX) && (X>3) && (Y>3))
		{
			if (Y*ViewZoom/100<12+MenuY) SelectedItem=1; 
			else if (Y*ViewZoom/100<24+MenuY) SelectedItem=2; 
			else if (Y*ViewZoom/100<36+MenuY) 
			{
				BITMAPINFOHEADER *bhead=(BITMAPINFOHEADER*)Image;
				if (bhead)
				{
					if ((Base->Items+1)->Y2>(40*DRWZOOM))
					if ((abs(bhead->biWidth-(Base->Items->X2-Base->Items->X1)/DRWZOOM)>1) ||
						(abs(bhead->biHeight-((Base->Items+1)->Y2-(Base->Items+1)->Y1)/DRWZOOM)>1))
						SelectedItem=3;
				}
			}
		}
	}
	return 1;
}

#pragma optimize("s",on)
int CBitmapImage::XML_output(char * output, int num_tabs, char only_calculate)
{
	static char tmpstr[256];

	int repeater=0;

	unsigned char pd1=0,pd2=0,pd3=0;
	int i=0;
	int j=0;
	int xlen=imgsize;
	sprintf(tmpstr,"<bmp len=\"%d\" b=\"",xlen);
	int len=(int)strlen(tmpstr);
	if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}

	//because all images are 24-bit, we are using compresion over 3 bytes.
	while (i<xlen)
	{
		unsigned char d1=0,d2=0,d3=0;
		if (i<xlen) d1=(unsigned char)*(Image+i);
		if (i+1<xlen) d2=(unsigned char)*(Image+i+1);
		if (i+2<xlen) d3=(unsigned char)*(Image+i+2);
		if ((pd1==d1) && (pd2==d2) && (pd3==d3))
		{
			repeater++;
			if (repeater==1)
			{
				if (!only_calculate) {*output=99;output+=1;}
				len+=1;
				j+=1;
			}
			else 
			{
				if (!only_calculate) *(output-1)=(*(output-1))+1;
				if (repeater>25) repeater=0;
			}
		}
		else if ((d1==0xFF) && (d2==0xFF) && (d3==0xFF))
		{
			pd1=d1;pd2=d2;pd3=d3;
			repeater=0;

			//white color pixel - very common and deserves special code 33
			if (!only_calculate) {*output=33;output+=1;}
			len+=1;
			j+=1;
		}
		else if ((d1==0x00) && (d2==0x00) && (d3==0x00))
		{
			pd1=d1;pd2=d2;pd3=d3;
			repeater=0;

			//black color pixel - very common and deserves special code 127
			if (!only_calculate) {*output=127;output+=1;}
			len+=1;
			j+=1;
		}
		else
		{
			pd1=d1;pd2=d2;pd3=d3;
			repeater=0;
	
			unsigned char d4=0;
			d4=(d1>>6)|((d2>>4)&0x0C)|((d3>>2)&0x30);
			d1&=0x3F;d2&=0x3F;d3&=0x3F;
			d1+=35;d2+=35;d3+=35;d4+=35;
			if (d1==92) d1=125; //92=backslash
			if (d2==92) d2=125;
			if (d3==92) d3=125;
			if (d4==92) d4=125;

			sprintf(tmpstr,"%c%c%c%c",d1,d2,d3,d4);
			len+=4;
			if (!only_calculate) {strcpy(output,tmpstr);output+=4;}
			j+=4;
		}

		if (j>190) 
		{
			repeater=0;
			sprintf(tmpstr,"\" />\r\n<bmp b=\"");
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
			j=0;
		}
		i+=3;
	}
	sprintf(tmpstr,"\" />\r\n");
	len+=(int)strlen(tmpstr);
	if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}

	return len;
}

int bitmap_position_counter=0;
#pragma optimize("s",on)
char *CBitmapImage::XML_input(char * file)
{
	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;

	if (strncmp(file,"bmp",3)==0)
	{
		file+=3;
		static char attribute[64];
		static char value[256];
		do
		{
			file=mf->XML_read_attribute(attribute,value,file,256);
			if (file==NULL) return NULL;
			if ((strcmp(attribute,"len")==0) || (strcmp(attribute,"bmplen")==0))
			{
				Image=(char*)malloc(atoi(value)+16);
				imgsize=atoi(value);
				bitmap_position_counter=0;
			}
			if ((strcmp(attribute,"b")==0) || (strcmp(attribute,"bits")==0))
			{
				for (int i=0;i<(int)strlen(value);i++)
				{
					unsigned char c=value[i];if (c==125) c=92;
					if (c==126) //old file format version support (repeating a 3-byte once)
					{
						for (int ii=0;ii<3;ii++)
						{
							Image[bitmap_position_counter]=Image[bitmap_position_counter-3];
							bitmap_position_counter++;
						}
					}
					else if (c==33) //white pixel
					{
						for (int ii=0;ii<3;ii++)
						{
							Image[bitmap_position_counter]=(char)0xFF;
							bitmap_position_counter++;
						} 
					}
					else if (c==127) //black pixel
					{
						for (int ii=0;ii<3;ii++)
						{
							Image[bitmap_position_counter]=(char)0x00;
							bitmap_position_counter++;
						} 
					}
					else if (c>=99) //repeating 3-bytes one or several times
					{
						for (int jj=0;jj<c-98;jj++)
							for (int ii=0;ii<3;ii++)
							{
								Image[bitmap_position_counter]=Image[bitmap_position_counter-3];
								bitmap_position_counter++;
							}
					}
					else
					{
						c-=35;
						unsigned char c2=value[++i];if (c2==125) c2=92;
						unsigned char c3=value[++i];if (c3==125) c3=92;
						unsigned char c4=value[++i];if (c4==125) c4=92;
						c2-=35;c3-=35;c4-=35;


						unsigned char d1=c|(c4<<6);
						unsigned char d2=c2|((c4<<4)&0xC0);
						unsigned char d3=c3|((c4<<2)&0xC0);
						Image[bitmap_position_counter++]=d1;
						Image[bitmap_position_counter++]=d2;
						Image[bitmap_position_counter++]=d3;
					}
				}
			}
		} while (attribute[0]);
	}
	return file;
}

#pragma optimize("s",on)
int CBitmapImage::LoadImageFromFile(CObject *dwg, char *fname)
{
	//this function loads an bitmap from file and stories it into CDrawing object (SpecialData=52 -> bitmap image object)
	CImage img;
	CDrawing *d=(CDrawing*)dwg;

	
	int X,Y,bpp;
	CDC *DC=pMainView->GetDC();
	if (fname)
	{
		try
		{
			if (img.Load(fname)) {pMainView->ReleaseDC(DC);return 0;}
			X=img.GetWidth();
			Y=img.GetHeight();
			bpp=img.GetBPP();
		}
		catch(...)
		{
			pMainView->MessageBox("Cannot Load bitmap!","Math-o-mir",MB_ICONWARNING);
			return 0;
		}

	}
	else
	{
		X=max(d->Items->X1,d->Items->X2)/DRWZOOM;
		Y=max((d->Items+1)->Y1,(d->Items+1)->Y2)/DRWZOOM;
		bpp=GetDeviceCaps(DC->m_hDC,BITSPIXEL);
		bpp=min(bpp,24);
		
	}

	if (((CBitmapImage*)(d->SpecialData))->Image) 
		free(((CBitmapImage*)(d->SpecialData))->Image);

	CBitmap bmp;
	CDC dc;
	
	dc.CreateCompatibleDC(DC);
	bmp.CreateCompatibleBitmap(DC,X,Y);
	dc.SelectObject(bmp);
	dc.FillSolidRect(0,0,X,Y,RGB(255,255,255));
	if (fname) img.BitBlt(dc.m_hDC,0,0);
	LPBITMAPINFO m2=(LPBITMAPINFO)malloc(X*Y*3+1024);
	m2->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	m2->bmiHeader.biWidth=X;
	m2->bmiHeader.biHeight=Y;
	m2->bmiHeader.biPlanes=1;
	m2->bmiHeader.biBitCount=min(bpp,24);
	m2->bmiHeader.biCompression=BI_RGB;
	m2->bmiHeader.biSizeImage=0;
	int colors=0;
	if (m2->bmiHeader.biBitCount<24) colors=1<<m2->bmiHeader.biBitCount;
	int tablesize=colors*sizeof(RGBQUAD);
	GetDIBits(dc.m_hDC,bmp,0,Y,((char*)m2)+sizeof(BITMAPINFOHEADER)+tablesize,m2,DIB_RGB_COLORS);
	((CBitmapImage*)(d->SpecialData))->Image=(char*)m2;
	((CBitmapImage*)(d->SpecialData))->imgsize=m2->bmiHeader.biSizeImage+tablesize+sizeof(BITMAPINFOHEADER)+16;
	pMainView->ReleaseDC(DC);
	pMainView->RepaintTheView();
	
	return 1;
}

#pragma optimize("s",on)
int CBitmapImage::SaveImageToFileForEditing(CObject *dwg)
{
	CDrawing *d=(CDrawing*)dwg;
	if (((CBitmapImage*)(d->SpecialData))->Image==NULL) return 0;

	CImage img;
	BITMAPINFOHEADER *hdr=(BITMAPINFOHEADER*)((CBitmapImage*)(d->SpecialData))->Image;

	int X=hdr->biWidth;
	int Y=hdr->biHeight;

	int colors=0;
	if (hdr->biBitCount<24) colors=1<<hdr->biBitCount;
	int tablesize=colors*sizeof(RGBQUAD);
	char *bits=(char*)hdr+sizeof(BITMAPINFOHEADER)+tablesize;

	char filename[340];
	GetTempPath(340,filename);
	if (filename[strlen(filename)-1]!='\\') strcat(filename,"\\");
	strcat(filename,"MoM_bmp.bmp");

	FILE *fil;
	fil=fopen(filename,"w+b");
	if (fil==NULL) return 0;

	char buff[32];
	memset(buff,0,32);
	buff[0]='B';
	buff[1]='M';
	if (hdr->biSizeImage==0) hdr->biSizeImage=(hdr->biBitCount/8)*hdr->biHeight*hdr->biWidth;
	*((int*)&buff[2])=40+tablesize+hdr->biSizeImage;
	*((int*)&buff[10])=40+tablesize+14;
	fwrite(buff,14,1,fil);
	fwrite(hdr,40+tablesize+hdr->biSizeImage,1,fil);
	fclose(fil);

	int result=(int)ShellExecute(NULL,"edit",filename,NULL,NULL,SW_SHOWNORMAL);
	if (result<=32) return 0;

	return 1;
}
