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
#include ".\element.h"
#include "expression.h"
#include "Mathomir.h"
#include "mainfrm.h"
#include "math.h"
#include "popupmenu.h"
#include "MathomirDoc.h"
#include "MathomirView.h"

extern char XMLFileVersion; //defines XML file version when loading/saving files

extern CMathomirView *pMainView;
extern int PlusLevel;
extern int MulLevel;
extern int EqLevel;
extern unsigned char OperatorLevelTable[256];
#define GetOperatorLevel(x) ((int)OperatorLevelTable[(unsigned char)(x)])
int CalculateText(CDC *DC,char * text, char * font, short * spacing, short TheFontSize, char *IsHigh, char *IsLow,char IsText, char IsFirst,char VMods);
int PaintText(CDC *DC,int X, int Y,char *text, char *font, short *spacing, short TheFontSize,int IsBlue,int color,char isText,char VMods);
int IsCharacterHigh(char ch, char font);
int IsCharacterLow(char ch, char font);
extern int PrintRendering;
extern char TouchMouseMode;
extern CExpression* ClipboardExpression;
extern PopupMenu *Popup;
extern char DisableMultitouch;
CElement *SelectedTab; //holds the tab element that is mouse-selected for lenght adjustment
int SelectedTabX;

const struct
{
	unsigned char E[3];
} FontSizeForElementTypes[]=
{
	{0,  0,  0},  //0-empty element
	{23, 0,  0},  //1-variable
	{23,  0,  0},  //2-operator
	{32,23, 0},  //3-exponent
	{29,29, 0},  //4-fraction
	{32,23, 0},  //5-parentheses
	{32,23, 0},  //6-functions
	{32,23, 23},  //7-sigma/pi
	{29, 23, 0},  //8-root
	{32,23, 23},  //9-condition list
	{23, 23, 23},  //10-condition list
	{0,  0,  0},  //11-column separator
	{0,  0,  0},  //12-row separator
	{0,  0,  0},  //
};


#define macroFontSizeForType(x) (FontSizeForElementTypes[m_Type].E[x]*max(35,((CExpression*)(m_pPaternalExpression))->m_FontSize)/32)

//constructor of the CElement class
//the 'CElement' represents an element of expression (in expression 'A+B' elemens are: 'A', '+', and 'B')
short CElementInitType;
CObject *CElementInitPaternalExpression;
CElement::CElement()
{
	//most of initialization is already done in the overloaded 'new' operator
	m_Color=-1;
	m_Text=(((CExpression*)m_pPaternalExpression)->m_StartAsText)?1:0;
	m_VMods=0;
}

CElement::~CElement(void)
{
	SelectedTab=NULL;
	if ((m_Type==9) && (Expression2==NULL) && (Expression3==NULL) && (Data1[0]=='H'))
	{
		//deleting hyperlink
		try
		{
			if (*(char**)Data3)
			{
				free(*(char**)Data3);
			}
		}
		catch(...)
		{

		}
	}
	if (Expression3) {delete (CExpression*)Expression3;}
	if (Expression2) {delete (CExpression*)Expression2;}
	if (Expression1) {delete (CExpression*)Expression1;}
}

const struct
{
	char D3pos;
	char Resize;
} ElementMemoryReservations[]=
{
	{0,		0},			//type 0 - never use
	{24,	24+48-4},	//type 1 - variable (can have max 1 subexpression)
	{-10,	0},			//type 2 - operator (can have max 1 subexpression)
	{4,		4+24-4},	//type 3 - exponent (can have max 2 subexpressions)
	{-10,	0},			//type 4 - fraction (can have max 2 subexpressions)
	{-10,	0},			//type 5 - brackets (can have max 2 subexpressions)
	{-10,	24-4},		//type 6 - function (can have max 2 subexpressions)
	{4,		4+8-4},		//type 7 - symbol (can have max 3 subexpressions)
	{-10,	0},			//type 8 - root (can have max 2 subexpressions)
	{4,		4+8-4},		//type 9 - condition list (can have max 3 subexpressions)
	{4,		4+8-4},		//type 10 - condition list (can have max 3 subexpressions)
	{-10,		24-4},			//type 11 - column separator
	{-10,		24-4}			//type 12 - row separator
};

/*void CElement::operator delete(void *p)
{
	//HeapFree(ProcessHeap,0,p);  //this makes problems wile poltting functions - better to use new/delete to reserve memory
	::operator delete(p);
}*/
void *CElement::operator new(size_t size)
{
	char *p;
	size_t tmp=size+ElementMemoryReservations[CElementInitType].Resize;
	p=(char*)::operator new(tmp);memset(p,0,tmp/*sizeof(CElement)*/);
	//p=(char*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,tmp);  //this can make problems while plotting functions!!! better to use new/delete to reserve memeory

	((CElement*)p)->m_pPaternalExpression=CElementInitPaternalExpression;
	((CElement*)p)->m_Type=(char)CElementInitType;
	((CElement*)p)->Data3=(short*)((char*)&(((CElement*)p)->Data1)+ElementMemoryReservations[CElementInitType].D3pos);
	return p;	
}


char ArmedTextReposition=0;

//calculate size of an element (in pixels) and prepare it 
//for displaying on the screen (recursively it does the same for all subexpressions containted in this element)
void CElement::CalculateSize(CDC *DC,short int zoom, short int * length, short int * above, short int * below, short paternal_position, char HQR)
{
	HFONT hfont;
	int ActualSize=((CExpression*)m_pPaternalExpression)->GetActualFontSize(zoom); //from the PaternalExpression get actual font size (adjusted to zoom level)
	ParenthesesAbove=ParenthesesBelow=ActualSize/3;

	if (m_Type==1)  //variable / constant 
	{
		int armed_here=0;
		//Data1 holds characters (string, null-terminated)
		//Data2 holds font-formating data (bit0 - bold, bit1 - italic, bit2 - normal/reduced size, bits4-7 - facename)
		//Data3 will be filled with coordinates (in pixels) of every character - prepared for displaying

		//first calculate character positions and store it into Data3
		char IsHigh=0,IsLow=0,IsFirst=0;
		if (paternal_position==0) 
			IsFirst=1;
		else
		{
			tElementStruct *ts=((CExpression*)m_pPaternalExpression)->m_pElementList+paternal_position-1;
			if ((ts->Type==11) || (ts->Type==12) || ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF)))
				IsFirst=1;
		}
		*length=CalculateText(DC,Data1,Data2,Data3,ActualSize,&IsHigh,&IsLow,m_Text,IsFirst,m_VMods);

		if (paternal_position>0) 
		{
			// between math and text add some space
			CElement *elm2=(((CExpression*)m_pPaternalExpression)->m_pElementList+paternal_position-1)->pElementObject;
			if (elm2)
			{
				char pretext=0;
				if (elm2->m_Type==1) pretext=elm2->m_Text;
				if (((elm2->m_Type==2) && (elm2->Data1[0]==(char)0xFF)) ||
					(elm2->m_Type==11) || (elm2->m_Type==12)) pretext=m_Text;
				if (((pretext==0) && (this->m_Text!=0)) || ((pretext!=0) && (this->m_Text==0)))
				{
					int delta=(((CExpression*)m_pPaternalExpression)->m_MarginX*3-1)/4;
					char ch=this->Data1[0];
					if ((ch==',') || (ch=='.') || (ch==';') || (ch=='?') || (ch==':'))
						delta=-2*(((CExpression*)m_pPaternalExpression)->m_MarginX)/3;
					
					(((CExpression*)(this->m_pPaternalExpression))->m_pElementList+paternal_position)->X_pos+=delta;
				}
				if ((pretext) && (elm2->Expression1) && ((this->Data1[0]==',') || (this->Data1[0]=='.') || (this->Data1[0]==';'))) //make comma closer to an indexed word/variable (text mode only)
					(((CExpression*)(this->m_pPaternalExpression))->m_pElementList+paternal_position)->X_pos-=(((IsHighQualityRendering) && (ArmedTextReposition==0))?2:3)*ActualSize/12;

				if ((pretext) && ((this->m_Text==2) || (this->m_Text==3) || (ArmedTextReposition))) //babaluj2
				{
					//spliced words of text!
					
					//(((CExpression*)(this->m_pPaternalExpression))->m_pElementList+paternal_position)->X_pos-=((IsHighQualityRendering)?5:4)*((CExpression*)m_pPaternalExpression)->m_MarginX/5;
					(((CExpression*)(this->m_pPaternalExpression))->m_pElementList+paternal_position)->X_pos-=(((IsHighQualityRendering) && (ArmedTextReposition==0))?3:3)*ActualSize/16;

				}
				else if ((pretext) && (this->m_Text) && (elm2->m_Type==1))
				{
					//some more space between words of text
					(((CExpression*)(this->m_pPaternalExpression))->m_pElementList+paternal_position)->X_pos+=max(((CExpression*)m_pPaternalExpression)->m_MarginX*2-4,0)/3;
					if (((this->Data1[0]>='A') && (this->Data1[0]<='Z')) || (this->Data1[0]<0)) (((CExpression*)(this->m_pPaternalExpression))->m_pElementList+paternal_position)->X_pos+=((CExpression*)m_pPaternalExpression)->m_MarginX/4;
				}
			}
		}
		if (this->m_Text==4) {ArmedTextReposition=1;armed_here=1;}

		//calculate height of the text (depending if it has high (d,f,h...) and low (j,g...) characters)
		if (IsHigh>1) *above=5*ActualSize/12;
		else if (IsHigh==1) *above=ActualSize/4; 
		else *above=ActualSize/5;
		
		if (IsLow) *below=ActualSize/3;
		else *below=2*ActualSize/7;

		if (Expression1)
		{
			//if this variable has some index expression (subscripted text)
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			if (HQR)
			{
				E1_posX=*length+ActualSize/16;
				*length+=E1_length;
			}
			else
			{
				E1_posX=*length-ActualSize/10;
				*length+=E1_length-ActualSize/6;
			}

			//adjusting positions on character level
			int last=(int)strlen(Data1)-1;
			char ch=Data1[last];
			int Xrepos=0;
			if ((Data2[last]&0xE0)!=0x60)
			{
				//latin characters
				if ((ch=='F') || (ch=='P')) Xrepos=-ActualSize/8;
				if ((ch=='Y') || (ch=='T') || (ch=='r') || (ch=='V')) Xrepos=-ActualSize/8;
				if ((ch=='\'') || (ch=='*')) Xrepos=-ActualSize/8; 
			}
			else
			{
				//greek characters
				if ((ch=='G')) Xrepos=-ActualSize/8;
				if ((ch=='R') || (ch=='U')) Xrepos=-ActualSize/16;
			}
			if ((m_pPaternalExpression) && 
				(((CExpression*)m_pPaternalExpression)->m_pPaternalElement) &&
				(((CExpression*)m_pPaternalExpression)->m_pPaternalElement->m_Type==3))
				Xrepos+=ActualSize/24; //if this is within an exponent then move it a bit to the right (because exponent must be aligned)

			E1_posX+=Xrepos;(*length)+=Xrepos;

			int tt=ActualSize/15;
			int tt2=ActualSize/8;
			if (HQR) {tt=ActualSize/24;tt2=ActualSize/4;if (zoom>200) tt2=2*ActualSize/9;}
			E1_posY=tt+max(((CExpression*)Expression1)->GetActualFontSize(zoom)/2,E1_above+tt2)-ActualSize/8;
			if (*below<(E1_posY+E1_below)) *below=E1_posY+E1_below;
		}
		if (!armed_here) ArmedTextReposition=0;
		return;
	}
	ArmedTextReposition=0;

	if (m_Type==2)  //operator (with two operands. Like '+', '-', '/')
	{
		if (Data1[0]==(char)0xE3)
		{
			//special handling - arrow with expression above it
			if (Expression1)
			{
				((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			}
			E1_posX=ActualSize/4+ActualSize/16;
			E1_posY=-E1_below;
			*length=E1_length+ActualSize/2+ActualSize/4;
			*above=(short)max(ActualSize/3,E1_above-E1_posY);
			*below=(short)ActualSize/3;
			return;
		}

		//Data1 contains the actual operator
		//Data3 is filled with its X position
		//Data2 is filled with font data

		/*int is_index=0;
		if ((this->m_pPaternalExpression) && (((CExpression*)this->m_pPaternalExpression)->m_pPaternalElement) &&
				((((CExpression*)this->m_pPaternalExpression)->m_pPaternalElement)->m_Type==1))
				is_index=1;*/

		CSize cs;
		int tmp=ActualSize/15;

		char Font=0x60;
		Data2[1]=Data1[0];  //in most cases we will simply print the character
		Data2[2]=0; //terminating zero

		int ls; //left space
		int rs; //right space
		Data3[2]=-4*ActualSize/33; //Y position correction (to baseline)

		if ((Data1[0]=='-') || (Data1[0]=='+'))
		{
			Data3[1]=ActualSize;
			ls=rs=0;
		}
		else
		{
			Data3[1]=110*ActualSize/100; //font size
			ls=rs=tmp;
		}

		if ((Data1[0]!='+') && (Data1[0]!='-')) //if either plus or minus we will jump over -> faster
		{
			char ch=Data1[0];


			if (ch==(char)0xB4) // the 'x' operator (vector multiplication)
			{
				//Data3[2]-=ActualSize/20;
				Data3[1]=19*Data3[1]/20;
				rs/=16;
				ls/=16;
			}

			if (ch==9) //TAB (space)
			{
				if (Data1[3]==0)
					ls=rs=1+(ActualSize*4+7)/15; //standarde length of a tab character
				else if (Data1[3]==1)
					ls=rs=-(ActualSize+40)/80; //single space character
				else
					ls=rs=((unsigned char)Data1[3])*ActualSize/20;
			}
			if (ch==(char)0xFF) //line wrapper
			{
				Data2[1]=' ';
				*length=(short)(0);
				*above=(short)ActualSize/4;
				*below=(short)ActualSize/3;
				return;
			}

			if (ch==(char)0xF0) //D'Alambert operator (empty square)
			{
				Data3[2]=ActualSize/32;
				rs/=8;
			}
			if (ch==(char)0x9E) //triangle
			{
				rs=ls=-ActualSize/8;
				Data2[1]='M';
			}
			if (ch==(char)0x44) //delta (laplace)
			{
				rs=0;
			}
			if (ch==(char)0xD1) //nabla
			{
				rs/=8;
				CElement *prev=GetPreviousElement();
				if ((prev) && (prev->m_Type==2)) ls/=8;
			}

			if (ch=='P') //paralel operator
			{
				Data2[1]='J';
				ls=ls/2;
				rs=rs/2;
				Data3[2]=0;
			}
			if (ch=='p') //parpendicular operator
			{
				Data2[1]=0x5E;'J';
				ls=ls/2;
				rs=rs/2;
				Data3[2]/=2;
			}
			if (ch==0x7C) //vertical bar
			{
				ls=rs=-ActualSize/10;
			}
			if (ch=='#') 
			{
				rs=0;
			}
			if (ch==(char)0xD8)//not operator
			{
				Data3[2]=-ActualSize/6;
				Data3[1]=95*ActualSize/100;
				rs=-ActualSize/8;
			}
			if ((ch==(char)0xD9) || (ch==(char)0xDA)) //and and or operators
			{
				ls=rs=rs/2;
			}
			if (ch==(char)0xD0) //angle operator
			{
				Data3[2]=0; //Y position correction (to baseline)
				Data3[1]=ActualSize; //font size
				rs=-rs;
				ls=ls/2;
			}
			if (ch==(char)0xB2) //minus-plus operator
			{
				Data2[1]=(char)0xB1;
				Data3[2]/=2;
			}
			if (ch==(char)0xE2) //  |--> arrow
			{
				Data2[1]=(char)0xAE;
				Data3[2]/=2;
			}
			if (ch==3) //small circle (composition of functions)
			{
				Data2[1]=(char)0xB7;
				rs=rs/2;
				ls=ls/2;
			}
			if ((ch==(char)0xA0) || (ch==(char)0x9F))//approx. equal (equal with dot above) and 'equal with hat'
			{
				Data2[1]='=';
			}

			if (ch==(char)0x5B) //triple dot in triangle formation (upside down)
				Data2[1]=0x5C; //triple dot in triangle formation
			if ((ch==0x5B) || (ch==0x5C))
			{
				ls-=ActualSize/20;
				rs-=ActualSize/20;
			}
			if (ch==(char)0xBC) //triple dot
			{
				Data3[2]+=ActualSize/12;
			}
			if (ch==(char)0xD7) {ls=-tmp;rs=-tmp;} //small dot (multiplying)
			
			if ((ch=='/') || 
				(ch=='&') || 
				(ch==(char)0xA1) ||  //gama
				(ch==(char)0xA6) ||  //elongated f
				(ch==(char)0xD1) ||  //nabla
				(ch=='D'))			 //delta
				{Data3[2]=0;} 

			if ((ch==(char)0xD7) || (ch=='/'))
			{
				//check if the operator is between two measurement units, if yes condense a bit
				if ((paternal_position) && (paternal_position<((CExpression*)(m_pPaternalExpression))->m_NumElements-1))
				{
					tElementStruct *ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position-1;
					if ((ts->pElementObject) && (ts->pElementObject->IsMeasurementUnit()))
					{
						ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position+1;
						if ((ts->pElementObject) && (ts->pElementObject->IsMeasurementUnit()))
						{
							if (!HQR) Font=0x70;
							rs=ls=-ActualSize/8;
						}
					}
				}
			}

			if (ch=='*') {Data3[2]/=3;} 

			if ((ch==(char)0xA3) ||  //less or equal
				(ch==(char)0xB3) ||  //greater or equal
				(ch==(char)0xB1) ||  //plus minus
				(ch==(char)0xC4) ||  //encircled plus
				(ch==(char)0xC5))    //encircled x
				{Data3[2]/=2;} 

			{   //we are handling logical operators and similar ones
				if (ch=='m') {strcpy(&Data2[1],"mod");}
				if (ch=='a') {strcpy(&Data2[1],"and");}
				if (ch=='o') {strcpy(&Data2[1],"or"); }
				if (ch=='A') {strcpy(&Data2[1],"nand");}
				if (ch=='O') {strcpy(&Data2[1],"nor");}
				if (ch=='x') {strcpy(&Data2[1],"xor");}
				if (ch=='n') {strcpy(&Data2[1],"not");}
				if (ch=='r') {strcpy(&Data2[1],"rot");}
				if (Data2[2]!=0) {Font=0x20; Data3[1]=4*ActualSize/5; Data3[2]=0;}
				if (ch==']') {strcpy(&Data2[1],"J");rs=0;} // spacer for the complement operator '\'

			}

			if (ch=='1') {strcpy(&Data2[1],":=");}
			if (ch=='2') {strcpy(&Data2[1],"=:");}
			if (ch=='3') {Data2[1]=':';Data2[2]=(char)0xDB;Data2[3]=0;}
			
			if (ch==0x01) {strcpy(&Data2[1],"<<");	Font=0x60;		Data3[1]=ActualSize;}
			if (ch==0x02) {strcpy(&Data2[1],">>");	Font=0x60;		Data3[1]=ActualSize;}

			if ((ch==0x22) || (ch==0x24)) {Data3[1]=4*ActualSize/3;}  //'all' and 'exists'

			if ((ch==(char)0xAB) || 
				(ch==(char)0xAC) || 
				(ch==(char)0xAD) || 
				(ch==(char)0xAE) || 
				(ch==(char)0xAF)) 
				{rs=ls=-tmp;} //arrows (simple arrows)

			//the following operators must be aligned to left
			if ((ch=='!') || (ch==',') || (ch==';'))
			{
				Font=0x60;
				Data3[2]=0;
				rs=tmp;ls=0;

				if (ch=='!') 
				{
					Data3[1]=4*ActualSize/3; //make exclamation mark little bigger
					Data3[2]/=2;
				}

				//we would like to move this operator closer to the left (to be closer to the previous element)
				//but we first have to check what element was there before it (paternal_position contains position of this element in paternal expression)
				if ((m_pPaternalExpression) && (paternal_position>0))
				{
					tElementStruct *theElement;
					theElement= ((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position-1;
					if ((theElement->Type==1) || //variable
						(theElement->Type==4) || //rational number
						(theElement->Type==8))    //root
					{
						ls=-ActualSize/6;
					}
				}
			}
		}

		/*if (is_index)
		{
			//we render somewhat more dense within indexes (to keep indexes short and readable)
			if (rs>ActualSize/16) rs-=ActualSize/6; else if (rs>=0) rs-=ActualSize/8; else rs-=ActualSize/12;
			if (ls>ActualSize/16) ls-=ActualSize/6; else if (ls>=0) ls-=ActualSize/8; else ls-=ActualSize/12;
		}*/

		hfont=GetFontFromPool(Font,max(Data3[1],1));
		DC->SelectObject(hfont);
		Data2[0]=Font; //stored for painting
		cs=DC->GetTextExtent(&Data2[1]);if (cs.cx==0) cs.cx=ActualSize/10;
		Data3[0]=ls;
		*length=(short)(cs.cx+ls+rs);
		char IsHigh=IsCharacterHigh(Data2[1],Font);
		//char IsLow=((CExpression*)m_pPaternalExpression)->IsCharacterLow(Data2[1],Font);

		if (IsHigh)
			*above=(short)ActualSize/3; 
		else
			*above=(short)ActualSize/4;

		*below=(short)ActualSize/3;

		return;
	}
	
	*length=0;
	*above=0;
	*below=0;

	if (m_Type==3)  //power (superscript text)
	{
		Data3[0]=0;
		int is_base_unit=0;
		E1_posX=0;
		E1_posY=0;

		if (Expression1!=NULL)
		{
			//handling base
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			*length=E1_length;			
			*below=E1_below;
			*above=E1_above;
			ParenthesesAbove=((CExpression*)Expression1)->m_ParenthesesAbove;
			ParenthesesBelow=((CExpression*)Expression1)->m_ParenthesesBelow;

			//check if all elements in the base of this exponent function are measurement units
			if (!HQR)
			{
				is_base_unit=1;
				for (int i=0;i<((CExpression*)Expression1)->m_NumElements;i++)
				{
					tElementStruct *ts=((CExpression*)Expression1)->m_pElementList+i;
					if ((ts->Type==2) && ((ts->pElementObject->Data1[0]=='/') || (ts->pElementObject->Data1[0]==(char)0xD7))) continue;
					if (ts->pElementObject)
						if (ts->pElementObject->IsMeasurementUnit()) continue;
					is_base_unit=0;
					break;
				}
				if (is_base_unit) Data3[0]=1; //paint exponent in gray color (because this is measurement unit: m^2, kg^2...)
			}

			//if (CenterParentheses) E1_CenterParentheses(above,below);
		}
		if (Expression2!=NULL)
		{
			//handling power, first compute the size of exponent
			CExpression *Base=(CExpression*)Expression1;
			CExpression *Exp=(CExpression*)Expression2;
			Exp->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
			
			int ExponentPosXCorrection;
			int ExponentPosYCorrection;
			if (HQR)
			{
				ExponentPosXCorrection=-ActualSize/16;
				ExponentPosYCorrection=6*ActualSize/16;
			}
			else
			{
				ExponentPosXCorrection=ActualSize*2/9;
				ExponentPosYCorrection=6*ActualSize/12; 
				if (zoom>200) ExponentPosYCorrection-=ActualSize/20;
			}

			//fine adjusting, according to the character
			if ( (Base->m_NumElements==1) && (Base->m_DrawParentheses==0) &&
					(Base->m_pElementList->Type==1))
			{
				//we found that there is only one element in the power base 
				// and that this element is a variable and that no parentheses are drawn
				CElement* BaseElement=Base->m_pElementList->pElementObject;
				int last=(int)strlen(BaseElement->Data1)-1;
				char ch=BaseElement->Data1[last];
				char fnt=BaseElement->Data2[last];
				if ((fnt&0xE0)!=0x60)
				{
					//latin characters
					if ((ch=='L')) ExponentPosXCorrection+=ActualSize/7;
					if ((ch=='A'))  ExponentPosXCorrection+=ActualSize/14;
					if ((IsCharacterHigh(ch,fnt)) && (fnt&0x02) && (ch!='h') && (ch!='b') && (ch!='i') && (ch!='j')) 
					{
						//high italic characters
						if (ch=='f')
							ExponentPosXCorrection-=ActualSize/10; 
						else
							ExponentPosXCorrection-=ActualSize/16; 

					}
					
				}
				else
				{
					//greek characters
					//if ((ch=='G'))  ExponentPosXCorrection+=ActualSize/7;
					if ((ch=='D') || (ch=='L'))  ExponentPosXCorrection+=ActualSize/14;
				}
			}

			E2_posX=*length-ExponentPosXCorrection;
			*length+=E2_length-ExponentPosXCorrection-((HQR)?ActualSize/40:ActualSize/8);

			//in order to corectly position (y-coordinate) the exponent, we have to check
			//what is in the exponent base, especially the parenthese type of the base (if it has parentheses)

			//int ParentheseType=-1; //suppose it has no parentheses
			int DrawsParentheses=0;
			if (Base->m_DrawParentheses)
			{
				if (HQR)
				{
					E2_posX+=-ActualSize/20;
					*length+=-ActualSize/20;
				}
				else
				{
					E2_posX+=ActualSize/8;
					*length+=ActualSize/8;
				}
				//ParentheseType=Base->m_ParentheseHeightFactor;
				DrawsParentheses=1;
				if ((Base->m_ParentheseShape=='[') )
				{
					E2_posX+=Base->m_ParentheseWidth/3;
					*length+=Base->m_ParentheseWidth/3;
				}
				if ((Base->m_ParentheseShape=='/'))
				{
					E2_posX+=Base->m_ParentheseWidth/4;
					*length+=Base->m_ParentheseWidth/4;
				}
				if ((Base->m_ParentheseShape=='{') || (Base->m_ParentheseShape=='\\'))
				{
					E2_posX-=Base->m_ParentheseWidth/4;
					*length-=Base->m_ParentheseWidth/4;
				}
			}
			else
			{
				//the base expression doesn't display parentheses, but we want to check deeper into subexpressions
				if (Base->m_pElementList->Type==5)
				{
					E2_posX+=-ActualSize/20;
					*length+=-ActualSize/20;
					//ParentheseType=((CExpression*)(Base->m_pElementList->pElementObject->Expression1))->m_ParentheseHeightFactor;
					DrawsParentheses=1;
					if ((((CExpression*)(Base->m_pElementList->pElementObject->Expression1))->m_ParentheseShape=='[') ||
						(((CExpression*)(Base->m_pElementList->pElementObject->Expression1))->m_ParentheseShape=='/'))
					{
						E2_posX+=ActualSize/15;
						*length+=ActualSize/15;
					}
					if ((((CExpression*)(Base->m_pElementList->pElementObject->Expression1))->m_ParentheseShape=='{') ||
						(((CExpression*)(Base->m_pElementList->pElementObject->Expression1))->m_ParentheseShape=='\\'))
					{
						E2_posX-=ActualSize/20;
						*length-=ActualSize/20;
					}
				}
				if ((Base->m_pElementList->Type==4) && (Base->m_pElementList->pElementObject->Data1[0]==' '))
				{
					// a over b handling
					ExponentPosYCorrection+=ActualSize/6;
				}
			}
			if (!DrawsParentheses) //no parentheses
				E2_posY=E1_posY-E1_above+((!HQR)?ExponentPosYCorrection:(4*ExponentPosYCorrection/5))-E2_below;
			//else if (ParentheseType==0) //large parentheses (full height of expression)
			//	E2_posY=E1_posY-E1_above-E2_below+ActualSize/3;
			else  //medium parentheses (only to cover rational numbers)
				E2_posY=-ParenthesesAbove-E2_below+ExponentPosYCorrection-ActualSize/8+((HQR)?ActualSize/8:0);
			//else //small parentheses (always the size of font)
			//	E2_posY=-ActualSize/3-E2_below+ExponentPosYCorrection-ActualSize/6;



			//but in some special cases, the exponent position must be somewhat different.
			//take care about these special cases
			if (Expression1)
			{
				short bt=Base->m_pElementList->Type;
				if ( (Base->m_NumElements==1) && (Base->m_DrawParentheses==0) &&
					((bt==1) || (bt==5)))
				{
					//we found that there is only one element in the power base 
					// and that this element is a variable or parenthese
					// and that no parentheses are drawn at the base level
					CElement* BaseElement=Base->m_pElementList->pElementObject;
					if (((BaseElement->Expression1) && (bt==1)) ||
						((BaseElement->Expression2) && (bt==5)))
					{
						//this variable has defined index so we will change position of our exponent
						int MoveToRight;

						if (bt==1)
						{
							int skew=0;
							if ((Base->m_pElementList->pElementObject->Data2[0]&0x02)) skew=ActualSize/12;
							E2_posX=BaseElement->E1_posX+Base->m_pElementList->X_pos+skew+((CExpression*)BaseElement->Expression1)->m_pElementList->X_pos-((CExpression*)Expression2)->m_pElementList->X_pos;
							MoveToRight=BaseElement->E1_length-E2_length-skew;
							//
						}
						else
						{
							E2_posX=BaseElement->E2_posX+Base->m_pElementList->X_pos;
							MoveToRight=BaseElement->E2_length+Base->m_pElementList->X_pos-E2_length;
							if (!HQR) MoveToRight-=Base->m_MarginX;
						}
						if (MoveToRight<0) MoveToRight=0;


						//adjusting positions on character level
						int last=(int)strlen(BaseElement->Data1)-1;
						char ch=BaseElement->Data1[last];
						if ((BaseElement->Data2[last]&0xE0)!=0x60)
						{
							//latin characters
							if ((ch=='F') || (ch=='P')) {E2_posX+=ActualSize/8;}
							if ((ch=='Y') || (ch=='T') || (ch=='r') || (ch=='V')) {E2_posX+=ActualSize/16;}
						}
						else
						{
							//greek characters
							if ((ch=='G')) {E2_posX+=ActualSize/8;}
							if ((ch=='R') || (ch=='U')) {E2_posX+=ActualSize/16;}
						}
						*length=E2_posX+E2_length+MoveToRight-((HQR)?0:ActualSize/9); 
					}
				}
				
				if (Base->m_DrawParentheses==0)
				if ( ((Base->m_NumElements==1) && (Base->m_pElementList->Type==6)) ||
					((Base->m_NumElements==2) && (FrameSelections) && (Base->m_pElementList->Type==6) && ((Base->m_pElementList+1)->Type==1) && ((Base->m_pElementList+1)->pElementObject->Data1[0]==0)) ||
					((Base->m_NumElements==2) && (FrameSelections) && (Base->m_pElementList->Type==1) && ((Base->m_pElementList+1)->Type==6) && ((Base->m_pElementList+0)->pElementObject->Data1[0]==0)) )
				{
					int elpos=0;
					CElement* BaseElement=Base->m_pElementList->pElementObject;
					if ((Base->m_NumElements==2) && (Base->m_pElementList->Type==1))
					{
						elpos=1;
						BaseElement=(Base->m_pElementList+1)->pElementObject;
					}

					{
						//this function has one single argument, we are adding exponent next to function
						int index_length=0;
						char is_lim=0;
						if (BaseElement->Expression2) 
						{
							index_length=BaseElement->E2_length;
							if (strcmp(BaseElement->Data1,"lim")==0) 
							{
								index_length=0;
								int fname_end=(Base->m_pElementList+elpos)->Length-BaseElement->Data3[1];
								int index_end=BaseElement->E2_posX+BaseElement->E2_length;
								if (index_end>fname_end) index_length=index_end-fname_end;
								is_lim=1;
							}
						}

						int MoveToRight=E2_length-index_length-((HQR)?0:(ActualSize/6));
						if (MoveToRight<0) MoveToRight=0;

						E2_posX=(Base->m_pElementList+elpos)->Length-(Base->m_pElementList+elpos)->pElementObject->Data3[1];//+ActualSize/6;
						if (HQR) E2_posX+=ActualSize/8;
						if ((BaseElement->Expression2) && (!is_lim))
							E2_posX=(Base->m_pElementList+elpos)->X_pos+BaseElement->E2_posX;

						E2_posY=(HQR)?(-ActualSize/8-E2_below):(-ActualSize/32-E2_below);
						if (IsCharacterHigh(BaseElement->Data1[strlen(BaseElement->Data1)-1],BaseElement->Data2[0])==0)
							E2_posY+=ActualSize/10;
						E1_length+=MoveToRight;
						*length=E1_length-((CExpression*)m_pPaternalExpression)->m_MarginX;
						if (HQR) *length+=ActualSize/7;

						//we also need to adjust function element (it was calculated before)
						//its argument expression must be moved to the right

						if ((Base->m_NumElements==2) && (elpos==0)) (Base->m_pElementList+1)->X_pos+=3*MoveToRight/4;
						BaseElement->E1_posX+=MoveToRight;
						(Base->m_pElementList+elpos)->Length+=MoveToRight;
						Base->m_OverallLength+=MoveToRight;
					}
				}
			}
			if (*above<(-E2_posY+E2_above)) *above=-E2_posY+E2_above;
			if (ParenthesesAbove<-E2_posY) ParenthesesAbove=-E2_posY;
		}
		return;
	}

	if (m_Type==4)  //rational nubers
	{
		/*int ddform=0;
		if (Data1[0]=='d')
		{
			//special handling - check if this is a d/d() operator
			CExpression *num=(CExpression*)Expression1;
			CExpression *denom=(CExpression*)Expression2;
			if ((num) && (denom) && (num->m_pElementList->pElementObject))  
			if ((num->m_NumElements==1) ||
				((num->m_NumElements==2) && (num->m_pElementList->Type==1) && (num->m_pElementList->pElementObject->Data1[0]==0) && ((num->m_pElementList+1)->Type==6)) ||
				((num->m_NumElements==2) && ((num->m_pElementList+1)->Type==1) && ((num->m_pElementList+1)->pElementObject->Data1[0]==0) && (num->m_pElementList->Type==6)) )
			{
				int elpos=0;
				if ((num->m_NumElements>1) && ((num->m_pElementList+1)->Type==6)) elpos=1;
				CElement *numelement=(num->m_pElementList+elpos)->pElementObject;
				if (numelement->IsDifferential())
				if ((numelement->Expression1) && (((CExpression*)numelement->Expression1)->m_ParenthesesFlags&0x81))
				{				
					ddform=1;
					
				}
			}
		}*/

		if (Expression1!=NULL)
		{
			//handling upper expression
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			E1_posX=0;
			if (HQR) 
				E1_posY=-ActualSize/6-E1_below;
			else
				E1_posY=-ActualSize/8-E1_below;
			*above=-E1_posY+E1_above;
		}
		if (Expression2!=NULL)
		{
			//handling lower expression
			((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
			E2_posX=0;
			if (HQR) 
				E2_posY=ActualSize/6+E2_above;
			else
				E2_posY=ActualSize/8+E2_above;
			*below=E2_posY+E2_below;
		}

		if (E1_length>E2_length)
		{
			E2_posX=(E1_length-E2_length)/2;
			*length=E1_length;
		}
		else
		{
			E1_posX=(E2_length-E1_length)/2;
			*length=E2_length;
		}
		
		
		if (E2_length>E1_length) Data3[0]=E2_length; else Data3[0]=E1_length;
		
		if (HQR)
		{
			Data3[0]+=(ActualSize+8)/6;
			*length+=ActualSize/4;
			E1_posX+=ActualSize/8;
			E2_posX+=ActualSize/8;
		}



		if (Data1[0]=='/')
		{
			//we need to draw diagonal line

			int tt=0;
			if (HQR) tt=ActualSize/8; //

			E1_posX=2*tt/3;
			E2_posX=E1_length+ActualSize/12+2*tt;
			E1_posY=-E1_below+ActualSize/6-tt;
			E2_posY=E2_above-ActualSize/6+tt;
			*above=-E1_posY+E1_above;
			*below=E2_posY+E2_below;
			*length=E2_posX+E2_length+2*tt/3;
			Data3[0]=min(E2_above+ActualSize/8,E1_length)+tt;
			Data3[1]=Data3[0];Data3[0]/=2;
			Data3[2]=min(E1_below+ActualSize/8,E2_length)+tt;
			Data3[3]=Data3[2];Data3[2]/=2;
		}
	
		int ddform=0;
		if (Data1[0]=='d')
		{
			//special handling for d/d() operator
			CExpression *num=(CExpression*)Expression1;
			CExpression *denom=(CExpression*)Expression2;
			if ((num) && (denom) && (num->m_pElementList->pElementObject))  
			if ((num->m_NumElements==1) ||
				((num->m_NumElements==2) && (num->m_pElementList->Type==1) && (num->m_pElementList->pElementObject->Data1[0]==0) && ((num->m_pElementList+1)->Type==6)) ||
				((num->m_NumElements==2) && ((num->m_pElementList+1)->Type==1) && ((num->m_pElementList+1)->pElementObject->Data1[0]==0) && (num->m_pElementList->Type==6)) )
			{
				int elpos=0;
				if ((num->m_NumElements>1) && ((num->m_pElementList+1)->Type==6)) elpos=1;
				CElement *numelement=(num->m_pElementList+elpos)->pElementObject;
				if (numelement->IsDifferential())
				if ((numelement->Expression1) && (((CExpression*)numelement->Expression1)->m_ParenthesesFlags&0x81))
				{	
					ddform=1;
					//we have an fraction if form: d()/d() we will change the function below it
					int fnamelen=numelement->E1_posX;
					(num->m_pElementList+elpos)->Length-=fnamelen;
					numelement->E1_posX=0;					
					E1_length-=fnamelen;
					for (int i=elpos+1;i<num->m_NumElements;i++)
						(num->m_pElementList+i)->X_pos-=fnamelen;
					num->m_OverallLength-=fnamelen;
					numelement->Data3[2]=-E2_length/2-num->m_MarginX-ActualSize/6-elpos*ActualSize/6;
					numelement->Data3[3]=-ActualSize/5-ActualSize/3;

					if (numelement->Expression2)
					{
						numelement->Data3[2]-=ActualSize/4;
						numelement->E2_posX=numelement->Data3[2]+ActualSize/3+ActualSize/8;
						numelement->E2_posY=numelement->Data3[3]+ActualSize/5;
					}
					
					E1_posY=0;
					E1_posX=E2_length+ActualSize/8;
					E2_posX=0;
					*length=E1_posX+E1_length+((CExpression*)m_pPaternalExpression)->m_MarginX;
					*above=max(E1_above,2*ActualSize/3+ActualSize/6);
					*below=max(*below,E1_below);
					
					Data3[0]=E2_length;
				}
			}
		}
		if (Data1[0]==' ')
		{
			//a over b   (binom - no horizontal line, but has parentheses around)

			int tt=0;
			int tt2=0;
			if (HQR) {tt=ActualSize/14;} else tt2=-ActualSize/16;
			Data3[1]=ActualSize/5+(E1_below+E1_above+E2_below+E2_above)/32+tt2;  //parenthese width
			E1_posX+=Data3[1]-ActualSize/20-tt;
			E2_posX+=Data3[1]-ActualSize/20-tt;
			E1_posY=-E1_below-3*tt/2;
			E2_posY=E2_above+3*tt/2;
			*above=-E1_posY+E1_above;
			*below=E2_posY+E2_below;
			*length+=2*Data3[1]-ActualSize/10-11*tt/8;
			Data3[0]=*length;
			ParenthesesAbove=-E1_posY+E1_above-ActualSize/16+tt;
			ParenthesesBelow=E2_posY+E2_below-ActualSize/16+tt;
		}
		else if (ddform)
		{
			ParenthesesAbove=3*ActualSize/4;
			if (ParenthesesAbove<E1_above) ParenthesesAbove=E1_above;
			ParenthesesBelow=E2_posY+ActualSize/3;
		}
		else
		{
			ParenthesesAbove=-E1_posY+ActualSize/3;
			ParenthesesBelow=E2_posY+ActualSize/3;
		}
	}

	if (m_Type==5) //Parentheses
	{
		if (Expression1!=NULL)
		{
			//handling expression
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);

			E1_posX=0;
			E1_posY=0;
			*length=E1_length;
			

			ParenthesesAbove=((CExpression*)Expression1)->m_ParenthesesAbove;
			ParenthesesBelow=((CExpression*)Expression1)->m_ParenthesesBelow;
			*below=E1_below;
			*above=E1_above;
			//if (CenterParentheses) E1_CenterParentheses(above,below);
			
			if (Expression2!=NULL)
			{
				//if it has an index
				((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
				if (HQR)
				{
					int tt=ActualSize/24;
					if (((CExpression*)this->Expression1)->m_ParentheseShape=='[') 
						tt=ActualSize/16;
					E2_posX=*length+tt;
					*length+=E2_length-tt;
				}
				else
				{
					E2_posX=*length-ActualSize/16;
					*length+=E2_length-ActualSize/16;
				}

				//adjusting positions on character level
				E2_posY=ParenthesesBelow+E2_above-((HQR)?ActualSize/4:ActualSize/3);
				if (*below<(E2_posY+E2_below)) *below=E2_posY+E2_below;
			}
			
		}
	}

	if (m_Type==6) //function,  like: sin(...), log(...), user(...) but also d(x)
	{			   //these functions can have exponent at upper right corner function name
		CSize cs;
		char IsHigh=0;
		char IsLow=0;
		int len=0;
		Data1[24-1]=0;		  //Data1 contains function name
		char font=Data2[0];  //Data2[0] contains font information
		char *ch=Data1;
		while (*ch)
		{
			if (!IsHigh) if (IsCharacterHigh(*ch,font)) IsHigh=1;
			if (!IsLow)  if (IsCharacterLow(*ch,font)) IsLow=1;
			len++; //len will contain the length of the function name
			ch++;
		}

		//for very small fonts, fix the font facename to Arial (math-o-mir system font) 
		//for better readability (important for toolbox items)
		if (ActualSize<=11) font=((font&0xE0)!=0x60)?(char)0x80:(char)0x60;

		if ((m_VMods) || (Data1[strlen(Data1)-1]=='\''))
		{
			char xfont[24];
			short spacing[24];
			memset(xfont,font,24);
			char IsHigh,IsLow;
			cs.cx=CalculateText(DC,Data1,xfont,spacing,max(ActualSize,1),&IsHigh,&IsLow,0,1,m_VMods);
			Data3[0]=1;
		}
		else
		{
			hfont=GetFontFromPool(font,max(ActualSize,1)|0x8000);
			DC->SelectObject(hfont);
			cs=DC->GetTextExtent(Data1);
			Data3[0]=0;
		}
		if (cs.cx==0) cs.cx=ActualSize/10;

		//special handling if the last character in the name is 'f' character
		if ((Data1[len-1]=='f') && ((font&0xE0)!=(char)0x60)) cs.cx+=ActualSize/15;

		*length=(short)(cs.cx)+(((HQR) || ((Expression1) && ((CExpression*)Expression1)->m_DrawParentheses))?ActualSize/6:ActualSize/32);

		if (IsHigh>1) *above=(short)ActualSize/2;
		else if (IsHigh==1) *above=(short)2*ActualSize/5;  
		else *above=(short)4*ActualSize/11;

		if (IsLow) *below=(short)4*ActualSize/9; 
		else *below=(short)2*ActualSize/5;

		//the starting position of the function name (in pixels - x coordinate)
		Data3[2]=-ActualSize/20;
		*length+=Data3[2];

		//analyze the index (subscript text) if it exists
		if (Expression2)
		{
			((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);

			if (strcmp(&Data1[0],"lim")==0)  //special handling for limes function
			{
				if (HQR)
					E2_posY=E2_above+2*ActualSize/5;
				else
					E2_posY=E2_above+9*ActualSize/24;
				if (E2_length<(short)cs.cx)
				{
					E2_posX=((short)cs.cx-E2_length)/2;
				}
				else
				{
					E2_posX=0;
					*length+=(E2_length-(short)cs.cx);
					Data3[2]=(E2_length-(short)cs.cx)/2;
				}
			}
			else
			{
				if (HQR)
				{
					E2_posX=(short)cs.cx+ActualSize/16;
					*length+=E2_length+ActualSize/16;
				}
				else
				{
					E2_posX=(short)cs.cx-ActualSize/12;
					*length+=E2_length-ActualSize/6;
				}
				int tt=ActualSize/15;
				int tt2=ActualSize/8;
				if (HQR) {tt=ActualSize/24;tt2=ActualSize/4;if (zoom>200) tt2=2*ActualSize/9;}
				E2_posY=tt+max(((CExpression*)Expression2)->GetActualFontSize(zoom)/2,E2_above+tt2)-ActualSize/8;

				//E2_posY=((HQR)?ActualSize/24:ActualSize/15)+max(((CExpression*)Expression2)->GetActualFontSize(zoom)/2,E2_above);
			}
			if (*below<(E2_posY+E2_below)) *below=E2_posY+E2_below;
		}


		//now the function argument
		Data3[1]=0;
		Data3[3]=0;
		if (Expression1)
		{
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			if (*above<E1_above) *above=(short)E1_above;
			if (*below<E1_below) *below=(short)E1_below;
			ParenthesesAbove=((CExpression*)Expression1)->m_ParenthesesAbove;
			ParenthesesBelow=((CExpression*)Expression1)->m_ParenthesesBelow;
		
			*length-=ActualSize/16;
			E1_posX=*length;

			if (HQR) {E1_posX+=ActualSize/9;*length+=ActualSize/9;}
			E1_posY=0;
			if (this->IsDifferential()) 
			{
				if (HQR) {E1_posX-=ActualSize/8;*length-=ActualSize/8;} else {E1_posX-=ActualSize/16;*length-=ActualSize/16;}
			}
			*length+=E1_length;

			if (this->IsDifferential(1))
			{
				if (!HQR)
					*length-=((CExpression*)this->m_pPaternalExpression)->m_MarginX;
			}
			else 
			{
				if ((m_pPaternalExpression) && (paternal_position<((CExpression*)m_pPaternalExpression)->m_NumElements-1))
				{
					//search through paternal expression and find the next element in the expression
					//If the next element is not operator (like + or -) then we intentionaly make a little space are
					tElementStruct *ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position+1;
					char ch=0;
					if (ts->pElementObject) ch=ts->pElementObject->Data1[0];
					if ((ts->Type!=2) || (GetOperatorLevel(ch)>MulLevel))
					if ((ts->Type!=1) || (ch!=0))
					if (ts->Type!=6)
					{
						if (((((CExpression*)Expression1)->m_ParenthesesFlags)&0xFD)==0)
						{
							if ((((CExpression*)Expression1)->m_NumElements>1) ||
								(((CExpression*)Expression1)->m_pElementList->Type!=5))
							{
								*length+=ActualSize/12; //the space is added at the end to distinct function argument
								if (HQR) *length+=ActualSize/6;
							}
						}
					}
				}
				
			}

			Data3[1]=*length-(short)cs.cx-Data3[2]; //distance (in pixels) from function name to the end of element
													//to be used in position exponent calculation (see above: m_Type==3)
			Data3[3]=0; //this is adjusted by fraction in the case of d/d() operator
		}
	}


	if (m_Type==7) //operators like: Sigma; Pi; Integral
	{			   //these operators/functions have two sub-expressions along with the main expression

		int AddEndingSpace=0;

		if (Expression1)
		{
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			*above=(short)E1_above;
			*below=(short)E1_below;
			ParenthesesAbove=((CExpression*)Expression1)->m_ParenthesesAbove;
			ParenthesesBelow=((CExpression*)Expression1)->m_ParenthesesBelow;
			E1_posX=0;
			E1_posY=0;

			//if (CenterParentheses) E1_CenterParentheses(above,below);

			if (Data1[0]!='/') //we are not adding end space for right bars
			if ((m_pPaternalExpression) && (paternal_position<((CExpression*)m_pPaternalExpression)->m_NumElements-1))
			{
				//search through paternal expression and find the next element in the expression
				//If the next element is not operator (like + or -) then we intentionaly make a little space are
				tElementStruct *ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position+1;
				char ch=0;
				if (ts->pElementObject) ch=ts->pElementObject->Data1[0];
				if ((ts->Type!=2) || (GetOperatorLevel(ch)>=MulLevel))
				{
					if ((ts->Type!=1) || (!ts->pElementObject->m_Text))
					AddEndingSpace=ActualSize/4; 
				}
				else if ((Data1[0]=='I') || (Data1[0]=='O'))
				{
					CExpression *a=(CExpression*)Expression1;
					if (a->m_NumElements)
					{
						tElementStruct *t=a->m_pElementList+a->m_NumElements-1;
						if ((t->Type!=6) || (strcmp(t->pElementObject->Data1,"d")))
							AddEndingSpace=ActualSize*2/7; 
					}
				}
			}
		}

		//calculate the height of the symbol (three posible sizes, according to Data2[0])
		int HalfSymbolHeight;

		if ((Data1[0]=='S') || (Data1[0]=='P')) //sigma and pi
		{
			if (Data2[0]==0) //large symbol
				HalfSymbolHeight=ActualSize*36/30;
			else if (Data2[0]==1) //medium symbol
				HalfSymbolHeight=ActualSize*28/30;
			else //small 
			{
				HalfSymbolHeight=ActualSize*20/30;
				Data2[0]=2;
			}
		}
		else
		{
			if (Data2[0]==0) //large symbol (full height of expression)
				HalfSymbolHeight=max(min(E1_above-E1_posY,E1_below+E1_posY),ActualSize);
			else if (Data2[0]==1) //medium symbol (only to cover rational numbers)
				HalfSymbolHeight=max(min(ParenthesesAbove,ParenthesesBelow),ActualSize*3/4);
			else //small symbol (always the size of font)
			{
				HalfSymbolHeight=ActualSize*2/3;
				Data2[0]=2;
			}
		}
		if (HalfSymbolHeight<ActualSize/2) HalfSymbolHeight=ActualSize/2;
		

		CSize cs;
		Data1[1]=(char)0xE5;Data1[2]=0;
		if ((Data1[0]=='S')) Data1[1]=(char)0xE5;  //sigma
		if ((Data1[0]=='P')) Data1[1]=(char)0xD5;  //Pi
		if ((Data1[0]=='I')) Data1[1]='a';  //integral
		if ((Data1[0]=='O')) Data1[1]='a';  //circular integral
		if ((Data1[0]=='|') || (Data1[0]=='/')) Data1[1]='a';  //bar (left or right)
		if ((Data1[0]=='S') || (Data1[0]=='P'))
			hfont=GetFontFromPool(3,0,0,max(2*HalfSymbolHeight,1)); //always greek alphabet
		else
			hfont=GetFontFromPool(3,0,0,max(ActualSize/2+HalfSymbolHeight/3,1));
		DC->SelectObject(hfont);
		cs=DC->GetTextExtent(&Data1[1]);if (cs.cx==0) cs.cx=ActualSize/10;
		if ((IsHighQualityRendering)  && ((Data1[0]=='I') || (Data1[0]=='O'))) cs.cx=(cs.cx+2)*7/6;
		if ((IsHighQualityRendering)  && ((Data1[0]=='|') || (Data1[0]=='/'))) cs.cx=(cs.cx+1)*6/5;

		if (*above<HalfSymbolHeight) *above=(short)HalfSymbolHeight;
		if (*below<HalfSymbolHeight) *below=(short)HalfSymbolHeight;
		if (ParenthesesAbove<HalfSymbolHeight) ParenthesesAbove=HalfSymbolHeight;
		if (ParenthesesBelow<HalfSymbolHeight) ParenthesesBelow=HalfSymbolHeight;
		Data3[0]=(short)HalfSymbolHeight; //store the height of the symbol
		Data3[1]=(short)cs.cx;    //store the width of symbol (in pixels)
		if (Data2[2]==2) Data3[1]=Data3[1]*2; //double integral
		if (Data2[2]==3) Data3[1]=Data3[1]*5/2; //triple integral
		Data3[3]=(short)cs.cx;    //store the width of a single symbol


		int Ycorr=0;
		if (Data2[1]==1) //integration limits placed aside
		{
			Ycorr+=HalfSymbolHeight*10/100+ActualSize/4;
		}

		//analyze the upper index (sperscript text) 
		if (Expression2)
		{
			((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
			if ((Data1[0]=='S') || (Data1[0]=='P')) 
			{
				if (HQR)
					E2_posY=-HalfSymbolHeight*85/100-E2_below+Ycorr;
				else
					E2_posY=-HalfSymbolHeight*75/100-E2_below+Ycorr;
			}
			else
				E2_posY=-HalfSymbolHeight-E2_below+Ycorr-ActualSize/10;
			if (*above<(-E2_posY+E2_above)) *above=-E2_posY+E2_above;
			if (ParenthesesAbove<-E2_posY) ParenthesesAbove=-E2_posY;
		}
		else
		{
			E2_length=0;
			E2_above=0;
			E2_below=0;
		}
		//analyze the lower index (subscript text) 
		if (Expression3)
		{
			((CExpression*)Expression3)->CalculateSize(DC,zoom,&E3_length,&E3_above,&E3_below,HQR);
			if ((Data1[0]=='S') || (Data1[0]=='P')) 
			{
				if (HQR)
					E3_posY=+HalfSymbolHeight*83/100+E3_above-Ycorr;
				else
					E3_posY=+HalfSymbolHeight*75/100+E3_above-Ycorr;

			}
			else
				E3_posY=+HalfSymbolHeight+E3_above-Ycorr+ActualSize/24;
			if (*below<(E3_posY+E3_below)) *below=E3_posY+E3_below;
			if (ParenthesesBelow<E3_posY) ParenthesesBelow=E3_posY;

		}
		else
		{
			E3_length=0;
			E3_above=0;
			E3_below=0;
		}

		//finally, calculate X positions
		if (Data2[1]==1)
		{
			//integration limits placed aside
			int longest=(int)E2_length;
			if (E3_length>longest) longest=E3_length;
	
			if (Data1[0]=='/') //right bar
			{
				E1_posX=0;
				E2_posX=E1_length+(longest-E2_length);
				E3_posX=E1_length+(longest-E3_length);
				Data3[2]=E1_length+longest;
				*length=Data3[2]+Data3[1];
			}
			else
			{
				Data3[2]=0; //the horizontal position of the symbol
				E3_posX=E2_posX=Data3[1];  //Data3[1] contains the witdht of the symbol
				E1_posX=longest+ActualSize/10+Data3[1];
				if (HQR)
				{
					E2_posX-=Data3[3]/15;
					E3_posX-=Data3[3]/15;
					E1_posX-=Data3[3]/10;
				}
				*length=E1_posX+E1_length+AddEndingSpace;
				if ((Data1[0]!='S') && (Data1[0]!='P') && (Data1[0]!='|'))
				{
					int Move=Data3[3]/4;
					E3_posX-=Move;
					E2_posX+=Move;
				}
			}
		}
		else
		{
			//integration limits placed above/below
			int longest=(int)Data3[1];
			if (E2_length>longest) longest=E2_length;
			if (E3_length>longest) longest=E3_length;
			
			if (Data1[0]=='/')
			{
				E1_posX=0;
				E2_posX=(longest-E2_length)/2+E1_length;
				E3_posX=(longest-E3_length)/2+E1_length;
				Data3[2]=(longest-Data3[1])/2+E1_length;
				*length=E1_length+longest;
			}
			else
			{
				Data3[2]=(longest-Data3[1])/2; //the horizontal position of the symbol
			
				E2_posX=(longest-E2_length)/2;
				E3_posX=(longest-E3_length)/2;
				E1_posX=longest+((HQR)?(ActualSize/8):0);
				*length=E1_posX+E1_length+AddEndingSpace;
				if ((Data1[0]!='S') && (Data1[0]!='P') && (Data1[0]!='|'))
				{
					int Move=Data3[3]/4;
					E3_posX-=Move;
					if (E3_posX<0) {*length+=-E3_posX;E2_posX+=-E3_posX;E1_posX+=-E3_posX;Data3[2]+=-E3_posX;E3_posX=0;}
					E2_posX+=Move;
				}
			}
		}
	}

	if (m_Type==8)  //root
	{
		if (Expression1!=NULL)
		{
			//handling argument
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			if ((((CExpression*)Expression1)->m_NumElements==1) && (((CExpression*)Expression1)->m_pElementList->Type==0))
			{E1_above=ActualSize/3+ActualSize/20;E1_below=ActualSize/2-ActualSize/20;}
			E1_posX=0;
			E1_posY=ActualSize/24;
			*length=E1_length+2;
			*above=(short)E1_above+ActualSize/8+((HQR)?(ActualSize/16):0);
			*below=(short)(E1_posY+E1_below);
			ParenthesesAbove=*above-ActualSize/8;
			ParenthesesBelow=*below-ActualSize/8;
		}

		if (Expression2!=NULL)
		{
			//handling expression2
			((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
			if (E2_length<2*ActualSize/5) E2_length=2*ActualSize/5;
			E2_posX=0;
			E2_posY=-ActualSize/10-E2_below;
			if (*above<(-E2_posY+E2_above)) *above=-E2_posY+E2_above;
		}
		else
		{
			E2_posX=0;
			E2_posY=-ActualSize/6-ActualSize/5;
			E2_length=2*ActualSize/5; 
			E2_above=ActualSize/6;
			E2_below=ActualSize/6;
		}

		//if (CenterParentheses) E1_CenterParentheses(above,below);


		int tt=0;
		if (HQR) tt=ActualSize/12; 
		E1_above+=tt;
		E1_below+=tt;
		Data3[0]=ActualSize/5+(E1_above+E1_below)/20; //width of the symbol
		E1_posX=E2_length+Data3[0]+ActualSize/15+2*tt;
		*length=E1_posX+E1_length+2*tt;
	}

	if (m_Type==9) //Vertical line (with condition list), also an HTML element
	{
		if ((Expression2==NULL) && (Expression3==NULL))
		{
			//an HTML element (type 9 without Expression2 and Expression3 defined)
			//pointer to additional data (URL) is in Data3

			if (Expression1)
				((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);

			E1_posX=0;
			E1_posY=0;
			*length=E1_length+((Data1[0]=='H')?(ActualSize/2):(ActualSize/6));
			*above=max(E1_above,ActualSize/3);
			*below=max(E1_below,ActualSize/3);
		}
		else
		{
			//handling main expression
			((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
			E1_posX=0;
			E1_posY=0;
			ParenthesesAbove=((CExpression*)Expression1)->m_ParenthesesAbove;
			ParenthesesBelow=((CExpression*)Expression1)->m_ParenthesesBelow;
			if (HQR) E1_length+=ActualSize/16; else E1_length-=ActualSize/16;


			if (Expression2) ((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
			if (Expression3) ((CExpression*)Expression3)->CalculateSize(DC,zoom,&E3_length,&E3_above,&E3_below,HQR);

			//in order to corectly position (y-coordinate) the exponent, we have to check
			//what is in the exponent base, especially the parenthese type of the base (if it has parentheses)
			CExpression *Base=(CExpression*)Expression1;

			//small vertical line - only the size of font
			Data3[0]=ActualSize/3; //line height - above
			Data3[1]=ActualSize/3; //line height - below
			/*if (Base->m_ParentheseHeightFactor==0) //large vertical line (full height of expression)
			{
				if (Data3[0]<E1_above) Data3[0]=E1_above;
				if (Data3[1]<E1_below) Data3[1]=E1_below;
			}
			else if (Base->m_ParentheseHeightFactor=1) //medium vertical line (only to cover rational numbers)*/
			{
				if (Data3[0]<E1_above) Data3[0]=ParenthesesAbove;
				if (Data3[1]<E1_below) Data3[1]=ParenthesesBelow;
			}

			if (Data3[0]<E2_above+E2_below+ActualSize/26) Data3[0]=E2_above+E2_below+ActualSize/26;
			if (Data3[1]<E3_above+E3_below+ActualSize/26) Data3[1]=E3_above+E3_below+ActualSize/26;
			E2_posY=-Data3[0]+E2_above-ActualSize/10;
			E3_posY= Data3[1]-E3_below+ActualSize/10;
			if (HQR)
			{
				E2_posX=E1_length+ActualSize/6;
				E3_posX=E1_length+ActualSize/6;
				*length=E1_length+ActualSize/5+max(E2_length,E3_length);
			}
			else
			{
				E2_posX=E1_length+ActualSize/10;
				E3_posX=E1_length+ActualSize/10;
				*length=E1_length+ActualSize/10+max(E2_length,E3_length);
			}

			
			*above=max(-E2_posY+E2_above,E1_above);
			*below=max(E3_posY+E3_below,E1_below);
		}
	}

	if (m_Type==10) //Vertical line with condition list - as an element
	{
		E2_length=1;E2_above=0;E2_below=0; //set if Expression2 doesn't exist
		if (Expression1) ((CExpression*)Expression1)->CalculateSize(DC,zoom,&E1_length,&E1_above,&E1_below,HQR);
		if (Expression2) ((CExpression*)Expression2)->CalculateSize(DC,zoom,&E2_length,&E2_above,&E2_below,HQR);
		if (Expression3) ((CExpression*)Expression3)->CalculateSize(DC,zoom,&E3_length,&E3_above,&E3_below,HQR);
		int maxlen=max(E1_length,max(E2_length,E3_length));
		if (Data1[0]&0x01) *length=ActualSize/5; else *length=ActualSize/12;
		Data3[0]=*length; //left bar position
		int tmp=*length;
		*length+=maxlen;
		Data3[1]=*length; //right bar position
		if (Data1[0]&0x02) *length+=ActualSize/5; else *length+=ActualSize/12;

		if (Data2[0]==2) //right aligned
		{
			tmp+=maxlen;
			E1_posX=tmp-E1_length;
			E2_posX=tmp-E2_length;
			E3_posX=tmp-E3_length;
		} 
		else if (Data2[0]==1) //center aligned
		{
			E1_posX=tmp+(maxlen-E1_length)/2;
			E2_posX=tmp+(maxlen-E2_length)/2;
			E3_posX=tmp+(maxlen-E3_length)/2;
		}
		else //left aligned
		{
			E1_posX=tmp;
			E2_posX=tmp;
			E3_posX=tmp;
		}
		
		E2_posY=0;
		*above=E1_above+E1_below+E2_above;
		*below=E3_above+E3_below+E2_below;
		//we want simetrical height
		if (*above>*below) 
		{
			*below=*above; 
			E3_posY=*below-E3_below;
			E1_posY=-E1_below-E2_above;
		}
		else 
		{
			*above=*below;  
			E1_posY=-*above+E1_above;
			E3_posY=E2_below+E3_above;	
		}

		ParenthesesAbove=-E1_posY+((CExpression*)Expression1)->m_ParenthesesAbove;
		ParenthesesBelow=E3_posY+((CExpression*)Expression3)->m_ParenthesesBelow;
	}
	if ((m_Type==11) || (m_Type==12))
	{
		*above=ActualSize/4;
		*below=ActualSize/3;
		*length=0;
		E1_posX=E1_posY=0;
		E1_length=0;
	}
	return;
}

void CElement::CalculateSizeReadjust(short zoom, short *length, short *above, short *below,char align)
{ 
	this->ParenthesesAbove=(ParenthesesAbove*zoom+HQZMp)/HQZM;
	this->ParenthesesBelow=(ParenthesesBelow*zoom+HQZMp)/HQZM;

	int E2=0;
	int E3=0;
	if (this->m_Type==1)
	{
		int ok=0;
		int l=(int)strlen(Data1);

		if ((m_pPaternalExpression!=KeyboardEntryObject) || (this->m_Text))
		if (l>0)
		{
			//re-calculate text again for better rendering
			short as=((CExpression*)this->m_pPaternalExpression)->GetActualFontSize(zoom);
			char hi,lo;

			CDC *DC=pMainView->GetDC();

			int orig_extend=(Data3[l]*zoom+HQZMp)/HQZM;
			int max_extend=orig_extend+as/16+1;

			CElement *next=this->GetNextElement();
			CElement *previous=this->GetPreviousElement();
			char is_last_word=0;
			char is_first_word=0;
			if ((!next) || ((next->m_Type==11) || (next->m_Type==12) || ((next->m_Type==2) && (next->Data1[0]==(char)0xFF))))
				is_last_word=1;
			if ((!previous) || ((previous->m_Type==11) || (previous->m_Type==12) || ((previous->m_Type==2) && (previous->Data1[0]==(char)0xFF))))
				is_first_word=1;

			CalculateText(DC,Data1,Data2,Data3,as,&hi,&lo,this->m_Text,is_first_word,m_VMods);
			if (Data3[l]>max_extend)
			{
				int prev=DC->SetTextCharacterExtra(-1);
				CalculateText(DC,Data1,Data2,Data3,as,&hi,&lo,this->m_Text,is_first_word,m_VMods);
				if (Data3[l]>max_extend)
				{
					int prev=DC->SetTextCharacterExtra(-2);
					CalculateText(DC,Data1,Data2,Data3,as,&hi,&lo,this->m_Text,is_first_word,m_VMods);
				}
				DC->SetTextCharacterExtra(prev);
			}
			else if (Data3[l]<orig_extend-as/12)
			{
				int prev=DC->SetTextCharacterExtra(1);
				CalculateText(DC,Data1,Data2,Data3,as,&hi,&lo,this->m_Text,is_first_word,m_VMods);
				DC->SetTextCharacterExtra(prev);
				if (Data3[l]>max_extend) //if it is too long, revert to original
				{
					CalculateText(DC,Data1,Data2,Data3,as,&hi,&lo,this->m_Text,is_first_word,m_VMods);
				}
			}

			//centering the word if it is of different size than ideal
			int delta=orig_extend-Data3[l];
			if ((is_first_word) && (align=='l'))  //if left alignment, don't move the first words
				delta=0;
			else if ((is_last_word) && (align=='r')) //if right alignment, algin right words to the end
				delta=delta;
			else
				delta=delta/2;

			for (int i=0;i<=l;i++)
				Data3[i]+=delta;
			

			ok=1;
			pMainView->ReleaseDC(DC);
		}
		
		if (!ok)
			for (int i=0;i<=l;i++)
				Data3[i]=(Data3[i]*zoom+HQZMp)/HQZM;
	}
	else if (m_Type==2)
	{
		Data3[0]=(Data3[0]*zoom+HQZMp)/HQZM;
		Data3[1]=(Data3[1]*zoom+HQZMp)/HQZM;
		Data3[2]=(Data3[2]*zoom+HQZMp)/HQZM;
	}
	else if ((m_Type==3) || (m_Type==5))
	{
		E2=1;
	}
	else if ((m_Type==4) || (m_Type==7))
	{
		Data3[0]=(Data3[0]*zoom+HQZMp)/HQZM;
		Data3[1]=(Data3[1]*zoom+HQZMp)/HQZM;
		Data3[2]=(Data3[2]*zoom+HQZMp)/HQZM;
		Data3[3]=(Data3[3]*zoom+HQZMp)/HQZM;
		E2=1;
		if (m_Type==7) E3=1;
	}
	else if (m_Type==6)
	{
		Data3[2]=(Data3[2]*zoom+HQZMp)/HQZM;
		Data3[3]=(Data3[3]*zoom+HQZMp)/HQZM;
		E2=1;
	}
	else if (m_Type==8)
	{
		Data3[0]=(Data3[0]*zoom+HQZMp)/HQZM;
		E2=1;
	}
	else if ((m_Type==9) || (m_Type==10))
	{
		if ((m_Type==9) && (Expression2==NULL) && (Expression3==NULL) && ((Data1[0]=='H') || (Data1[0]=='L'))) //hyperlink or label
		{
			E2=E3=0;
		}
		else
		{
			E2=E3=1;
			Data3[0]=(Data3[0]*zoom+HQZMp)/HQZM;
			Data3[1]=(Data3[1]*zoom+HQZMp)/HQZM;
		}
	}

	E1_posX=(E1_posX*zoom+HQZMp)/HQZM;
	E1_posY=(E1_posY*zoom+HQZMp)/HQZM;
	E1_above=(E1_above*zoom+HQZMp)/HQZM;
	E1_below=(E1_below*zoom+HQZMp)/HQZM;
	E1_length=(E1_length*zoom+HQZMp)/HQZM;
	if (E2)
	{
		E2_posX=(E2_posX*zoom+HQZMp)/HQZM;
		E2_posY=(E2_posY*zoom+HQZMp)/HQZM;
		E2_above=(E2_above*zoom+HQZMp)/HQZM;
		E2_below=(E2_below*zoom+HQZMp)/HQZM;
		E2_length=(E2_length*zoom+HQZMp)/HQZM;
	}
	if (E3)
	{
		E3_posX=(E3_posX*zoom+HQZMp)/HQZM;
		E3_posY=(E3_posY*zoom+HQZMp)/HQZM;
		E3_above=(E3_above*zoom+HQZMp)/HQZM;
		E3_below=(E3_below*zoom+HQZMp)/HQZM;
		E3_length=(E3_length*zoom+HQZMp)/HQZM;
	}

	if (Expression1)
		((CExpression*)Expression1)->CalculateSizeReadjust(zoom,length,above,below);
	if (Expression2)
		((CExpression*)Expression2)->CalculateSizeReadjust(zoom,length,above,below);
	if (Expression3)
		((CExpression*)Expression3)->CalculateSizeReadjust(zoom,length,above,below);
}


//it is painting the Element into device context
//the element must be already prepared for painting (by calling 'CalculateSize' earlier)
//this function should be fast!
void CElement::PaintExpression(CDC * DC, short zoom, short X, short Y,char IsBlue,int ActualSize,RECT *ClipReg,int color)
{
	CMainFrame *mf;
	HFONT hfont;

	if (m_Color!=-1) color=ColorTable[m_Color];
	if (m_Type==1)  //variable / constant
	{
		if (((IsBlue) && (m_Text) && ((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint>0) && (!Popup->IsWindowVisible()))
		{
			//for plain text we are going to display insertion point also within the words (this differs
			//from math variables where a multi-letter variable would become 'touched' instead)
			int kk=((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint;
			if (kk<=1) kk=0;
			int paternal_pos=this->GetPaternalPosition();
			if ((((CExpression*)this->m_pPaternalExpression)->m_pElementList+paternal_pos)->IsSelected!=2)
			if (Data3[strlen(Data1)]>=kk)
			{
				IsBlue=0;
				if ((((CExpression*)m_pPaternalExpression)->m_IsKeyboardEntry!=paternal_pos+1) ||
					(Data3[((CExpression*)m_pPaternalExpression)->m_KeyboardCursorPos]!=kk))
				{
					int ActualSize2;
					if (IsHighQualityRendering) 
					{
						ActualSize2=ActualSize/4;
						if (ActualSize2==0) ActualSize2=1;
					}
					else 
						ActualSize2=ActualSize/3;
					int width=max((ActualSize+12)/16,2);
					int height=2*ActualSize2+ActualSize/16+ActualSize2/2;
					kk-=width/2;
					int Ypos=Y-ActualSize2-ActualSize/11+ActualSize/24-ActualSize2/6;
					if (IsDrawingMode)
						DC->FillSolidRect(X+kk+width/4,Ypos,width/2,height,PALE_RGB(GREEN_COLOR)); //PAINT THE INSERTION POINT
					else
						DC->FillSolidRect(X+kk,Ypos,width,height,GREEN_COLOR); //PAINT THE INSERTION POINT
				}
			}
		}

		if ((Data1[0]=='\'') && (Data1[1]==0) && (this->IsMeasurementUnit()==0)) //special handling for the apostrophe key (it is repositioned according to the previous element height)
		{
			int ll=GetPaternalPosition();
			if (ll>0)
			{
				Y-=(((CExpression*)this->m_pPaternalExpression)->m_pElementList+ll-1)->pElementObject->ParenthesesAbove-ActualSize/6;
				if ((((CExpression*)this->m_pPaternalExpression)->m_pElementList+ll-1)->Type>1)  
					X-=ActualSize/8;
				else
					X-=ActualSize/16;
			}
			else
				Y-=ActualSize/8;
			
		}

		PaintText(DC,X,Y,Data1,Data2,Data3,ActualSize,IsBlue,color,m_Text,m_VMods);
		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		return;
	}

	mf=(CMainFrame*)theApp.m_pMainWnd;

	if (m_Type==2)  //operator (Like: '+', '-', '/')
	{
		char oper_type=Data1[0];
		int clr=(IsBlue)?BLUE_COLOR:((Data2[0]&0x10)?PALE_RGB(color):color);

		if (oper_type&0x80)
		{
			if (oper_type==(char)0xE3) //arrow with expression above it
			{
				if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
				DC->FillSolidRect(X,Y-ActualSize/40+ActualSize/24,this->E1_length+ActualSize/2+ActualSize/6,max(1,ActualSize/20),clr);
				CBrush brush(clr);
				CPen pen(PS_SOLID,1,clr);
				HGDIOBJ op=DC->SelectObject(pen);
				HGDIOBJ ob=DC->SelectObject(brush);
				CPoint pt[4];
				pt[0].x=X+this->E1_length+ActualSize/2+ActualSize/4; pt[0].y=Y+ActualSize/24;
				pt[1].x=pt[0].x-ActualSize/4; pt[1].y=pt[0].y+ActualSize/5;
				pt[2].x=pt[0].x-ActualSize/8; pt[2].y=pt[0].y;
				pt[3].x=pt[1].x;              pt[3].y=pt[0].y-ActualSize/5;
				DC->Polygon(pt,4);
				DC->SelectObject(ob);
				DC->SelectObject(op);
				return;
			}

			if (oper_type==(char)0x9E) //triangle
			{
				DC->SelectObject(GetPenFromPool(max(1,Data3[1]/16),IsBlue,clr));
				int xx=X+Data3[0]+Data3[1]/12;
				int yy=Y+Data3[2]+Data3[1]/3+Data3[1]/16-((Data3[1]<14)?1:0);
				int h=Data3[1]/2+Data3[1]/16;
				int l=h;
				DC->MoveTo(xx,yy);
				DC->LineTo(xx+l/2,yy-h);
				DC->LineTo(xx+l,yy);
				DC->LineTo(xx,yy);
				CPoint pt[4];
				return;
			}
			if (oper_type==0x5B) //three dots in upside-down triangle formation
			{
				int h=(Data3[1]+3)/6;
				int r=2*(h+3)/7;
				DC->SelectObject(GetPenFromPool(max(1,Data3[1]/20),IsBlue,clr));	
				CBrush br;
				br.CreateSolidBrush((IsBlue)?BLUE_COLOR:clr);
				DC->SelectObject(br);
				Y-=Data3[1]/32;
				X+=h;

				if (r>3) 
				{
					DC->Ellipse(X,Y-h,X+r,Y-h+r);
					DC->Ellipse(X+2*h,Y-h,X+2*h+r,Y-h+r);
					DC->Ellipse(X+h,Y+h,X+h+r,Y+h+r);
				}
				else if ((r==2) || (r==3))
				{
					DC->Rectangle(X,Y-h,X+r,Y-h+r);
					DC->Rectangle(X+2*h,Y-h,X+2*h+r,Y-h+r);
					DC->Rectangle(X+h,Y+h,X+h+r,Y+h+r);
				}
				else
				{
					DC->SetPixelV(X,Y-h,(IsBlue)?BLUE_COLOR:clr);
					DC->SetPixelV(X+2*h,Y-h,(IsBlue)?BLUE_COLOR:clr);
					DC->SetPixelV(X+h,Y+h,(IsBlue)?BLUE_COLOR:clr);
				}

				return;
			} 
			if (oper_type==(char)0xB2) //minus plus opertor
			{
				if (Data3[1]<20) Data3[1]=120*Data3[1]/100;
				int corr=max(1,Data3[1]/20);
				if (corr&0x01) corr=1; else corr=0;
				DC->FillSolidRect(X+Data3[0],Y+Data3[1]/3-7*Data3[1]/14+Data3[2],max(1,10*Data3[1]/22)-corr,max(1,Data3[1]/20),clr);
				DC->FillSolidRect(X+Data3[0],Y+Data3[1]/3-3*Data3[1]/14+Data3[2]-max(1,Data3[1]/20)/2-corr,max(1,10*Data3[1]/22)-corr,max(1,Data3[1]/20),clr);			
				DC->FillSolidRect(X+Data3[0]+5*Data3[1]/22-max(1,Data3[1]/20)/2-corr,Y+Data3[1]/3-6*Data3[1]/14+Data3[2],max(1,Data3[1]/20),max(1,6*Data3[1]/14)-corr,clr);
				return;
			}
		}
		if (oper_type=='P') //paralel operator
		{
			DC->FillSolidRect(X+Data3[0]+Data3[1]/9,Y-Data3[1]/3+Data3[2],max(1,Data3[1]/15),2*Data3[1]/3,clr);
			DC->FillSolidRect(X+Data3[0]+4*Data3[1]/11,Y-Data3[1]/3+Data3[2],max(1,Data3[1]/15),2*Data3[1]/3,clr);
			return;
		}
		if (oper_type==0x03) //small circle (composition of functions)
		{
			DC->SelectObject(GetPenFromPool(max(1,Data3[1]/20),IsBlue,clr));			
			DC->Arc(X+Data3[0]+Data3[1]/16,
					Y-Data3[1]/4+Data3[1]/6,
					X+Data3[0]+Data3[1]/4+Data3[1]/16,
					Y+Data3[1]/6,
					0,0,1,0);
			return;
		} 
		if ((oper_type==9) && 
			((IsBlue) || (((CExpression*)this->m_pPaternalExpression)->m_Selection) || (this->m_pPaternalExpression==KeyboardEntryObject))) //the tab (space)
		{
			int ok=0;
			if (IsBlue) ok=1;

			CExpression *p=(CExpression*)this->m_pPaternalExpression;
			if ((p->m_Selection>0) && (p->m_Selection<=p->m_NumElements+1))
			{
				int sel=p->m_Selection;
				if ((sel<=p->m_NumElements) && ((p->m_pElementList+sel-1)->pElementObject==this))
					ok=1;
				if ((sel>1) && ((p->m_pElementList+sel-2)->pElementObject==this))
					ok=1;
			}
			if (KeyboardEntryObject==(CObject*)p)
			{
				if ((p->m_IsKeyboardEntry>0) && (p->m_IsKeyboardEntry<=p->m_NumElements))
				{
					int sel=p->m_IsKeyboardEntry;
					CElement *elm=(p->m_pElementList+sel-1)->pElementObject;
					if (elm->m_Type==1)
					{
						if ((sel<p->m_NumElements) && ((p->m_pElementList+sel)->pElementObject==this) && (p->m_KeyboardCursorPos>=(int)strlen(elm->Data1)))
							ok=1;
						if ((sel>1) && ((p->m_pElementList+sel-2)->pElementObject==this) && (p->m_KeyboardCursorPos==0))
							ok=1;
					}
				}
			}
			if (ok)
			{
				DC->SelectObject(GetPenFromPool(1,1,0));
				if ((Data1[3]==1)) 
				{
					DC->MoveTo(X-Data3[1]/16,Y-Data3[1]/20);
					DC->LineTo(X+Data3[1]/16,Y+Data3[1]/20);
					DC->LineTo(X-Data3[1]/16,Y+3*Data3[1]/20);
				}
				else
				{
					int len=Data3[0]*2+Data3[1]/16;
					int c=Data3[1]/4;
					int c2=(Data3[1]+28)/16;
					DC->MoveTo(X,Y);
					DC->LineTo(X+len-c2,Y);
					DC->LineTo(X+len-c-c2,Y-c);
					DC->MoveTo(X+len-c2,Y);
					DC->LineTo(X+len-c-c2,Y+c);
					if (SelectedTab!=this)
					{
						DC->MoveTo(X+len-c2,Y-c);
						DC->LineTo(X+len,Y-c);
						DC->LineTo(X+len,Y+c);
						DC->LineTo(X+len-c2,Y+c);
						DC->LineTo(X+len-c2,Y-c);
					}
					else
						DC->FillSolidRect(X+len-c2,Y-c,c2,2*c,BLUE_COLOR);
				}
			}
			return;
		}
		if (oper_type==']') //complement?
		{
			DC->SelectObject(GetPenFromPool(max(1,Data3[1]/16),IsBlue,clr));
			DC->MoveTo(X,Y-Data3[1]/4);
			DC->LineTo(X+4*Data3[1]/8,Y+Data3[1]/4);
			return;
		}

		hfont=GetFontFromPool(Data2[0],max(Data3[1],1));
		DC->SelectObject(hfont);
		DC->SetTextColor(clr);
		if ((ShadowSelection) && (IsBlue==1) && (ActualSize>10) && (ActualSize<40))
			DC->TextOut(X+Data3[0]+1,Y+Data3[1]/3+Data3[2],&Data2[1]);
		DC->TextOut(X+Data3[0],Y+Data3[1]/3+Data3[2],&Data2[1]);

		if (oper_type&0x80)
		{
			if (oper_type==(char)0xE2) // |--> operator
			{
				DC->TextOut(X+Data3[0],Y+Data3[1]/3+Data3[2],&Data2[1]);
				DC->FillSolidRect(X+Data3[0],Y+ActualSize/40+Data3[2],max(ActualSize/22,1),ActualSize/4+((ActualSize>15)?1:0),clr);
			}
			if (oper_type==(char)0xA0) //approx. equal (dot over equal)
			{
				int corr=max(0,Data3[1]/30);
				int Xx=X+Data3[0]+5*Data3[1]/23;
				int Yy=Y+Data3[1]/3+Data3[2]-6*Data3[1]/14;
				if (corr==0)
				{
					DC->SetPixel(Xx,Yy,clr);
					if (Data3[1]>=20) DC->SetPixel(Xx-1,Yy,clr);
				}
				else if (corr<2)
				{
					DC->FillSolidRect(Xx-corr,Yy-corr,2*corr,2*corr,clr);
				}
				else
				{
					DC->SelectObject(GetPenFromPool(max(1,Data3[1]/20),IsBlue,clr));
					DC->Ellipse(Xx-corr,Yy-corr,Xx+corr,Yy+corr);
				}
			}
			if (oper_type==(char)0x9F) //equal with hat
			{
				//int corr=max(0,Data3[1]/30);
				int Xx=X+Data3[0]+5*Data3[1]/24;
				int Yy=Y+Data3[1]/3+Data3[2]-14*Data3[1]/32;
				int pw=max(1,Data3[1]/24);
				DC->SelectObject(GetPenFromPool(pw,IsBlue,clr));
				int cc=max(1,Data3[1]/9);
				DC->MoveTo(Xx-cc,Yy);
				DC->LineTo(Xx,Yy-cc);
				if (pw>1) pw=0;
				DC->LineTo(Xx+cc+pw,Yy+pw);
			}
		}
		return;
	}

	if (m_Type==3)  //power (superscript text)
	{
		if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,(Data3[0])?PALE_RGB(color):color);
		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		return;
	}

	if (m_Type==4)  //rational numbers
	{
		int Len;
		if (E2_length>E1_length) Len=E2_length; else Len=E1_length;

		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);

		if (Data1[0]=='/')
		{
			DC->SelectObject(GetPenFromPool(max(ActualSize/12,1),(IsBlue)?1:0,color));
			DC->MoveTo(X+E1_posX+E1_length+(E2_posX-E1_posX-E1_length)/2+ActualSize/24-Data3[0],Y+Data3[1]);
			DC->LineTo(X+E1_posX+E1_length+(E2_posX-E1_posX-E1_length)/2-ActualSize/24+Data3[2],Y-Data3[3]);
		}
		else if (Data1[0]==' ')
		{
			//a over b
			((CExpression*)m_pPaternalExpression)->PaintParentheses(DC,zoom,X,Y-ParenthesesAbove,X+Data3[0],Y+ParenthesesBelow,Data3[1],'(',0,IsBlue,color);
		}
		else
		{
			//horizontal line
			int len=Data3[0];
			if (!IsHighQualityRendering)
				DC->FillSolidRect(X+ActualSize/16,Y-(ActualSize+0)/26,len-ActualSize/8,max((ActualSize+0)/13,1),(IsBlue)?BLUE_COLOR:color);
			else
				DC->FillSolidRect(X+ActualSize/16,Y-(ActualSize+0)/26,len,max((ActualSize+0)/13,1),(IsBlue)?BLUE_COLOR:color);
		}
		return;
	}

	if (m_Type==5) //parentheses
	{
		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);
		return;
	}

	if (m_Type==6) //function; like sin(...)
	{
		COLORREF thecolor=(IsBlue)?BLUE_COLOR:((color==0) && (IsHighQualityRendering==0))?RGB(0,(PrintRendering)?88:((ActualSize<20)?104:96),0):color;
		char font_params=Data2[0];
		if (ActualSize<=11) font_params=((font_params&0xE0)!=0x60)?(char)0x80:(char)0x60;

		if (Data3[0])
		{
			//general printout
			char isHigh,isLow;
			char font[24];
			memset(font,font_params,24);
			short spacing[24]; 			
			CalculateText(DC,Data1,font,spacing,max(ActualSize,1),&isHigh,&isLow,0,(char)(Data3[0]&0x02),m_VMods);
			PaintText(DC,X+Data3[2]+ActualSize/20,Y+Data3[3],Data1,font,spacing,max(ActualSize,1),IsBlue?0xFFFFFFFF:0,thecolor,0,m_VMods);
		}
		else
		{
			//fast printout
			DC->SetTextColor(thecolor);
			hfont=GetFontFromPool(font_params,max(ActualSize,1)|0x8000);
			DC->SelectObject(hfont);
			if ((ShadowSelection) && (IsBlue==1) && (ActualSize>10) && (ActualSize<40))
				DC->TextOut(X+Data3[2]+1,Y+ActualSize/3+Data3[3],Data1);
			DC->TextOut(X+Data3[2],Y+ActualSize/3+Data3[3],Data1);
		}

		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);
		return;
	}

	if (m_Type==7) //sigma, pi, integral
	{
		DC->SetTextColor((IsBlue)?BLUE_COLOR:color);
		if ((Data1[0]=='S') || (Data1[0]=='P'))
		{
			hfont=GetFontFromPool(3,0,0,max(Data3[0]*20/10,1));
			DC->SelectObject(hfont);
			DC->TextOut(X+Data3[2],Y+Data3[0]*20/30-Data3[0]/7,&Data1[1]);
		}
		else if ((Data1[0]=='|') || (Data1[0]=='/')) // bar
		{
			if (!IsHighQualityRendering)
			{
				DC->SelectObject(GetPenFromPool(max(Data3[3]/6+ActualSize/20,1),(IsBlue)?1:0,color));

				int X1,Y1,Y2;
				X1=X+Data3[2];  //x_placement
				Y1=Y-Data3[0];
				Y2=Y+Data3[0];
				int ww=4*Data3[3]/8;

				DC->MoveTo(X1+ww,Y1);
				DC->LineTo(X1+ww,Y2);
			}
			else
			{
				int X1,Y1,X2,Y2;

				int PenWidth=3*Data3[3]/4;
				int HalfHeight=Data3[0]*4;
				int HalfWidth;

				mf->StartMyPainting(DC,Data3[1]*4+2*PenWidth,HalfHeight,HalfHeight,color);

				X1=PenWidth;
				Y1=-HalfHeight;
				X2=Data3[3]*4-PenWidth;
				Y2=HalfHeight;
				HalfWidth=(X2-X1)/2;

				int ii;
				for (ii=-PenWidth/2;ii<=PenWidth/2;ii++)
				{
					mf->MyMoveTo(DC,X2-HalfWidth+ii,Y1+PenWidth);
					mf->MyLineTo(DC,X1+HalfWidth+ii,Y2-PenWidth,IsBlue);
				}

				mf->EndMyPainting(DC,X+Data3[2],Y);
			}
		}
		else //integrals
		{
			if (!IsHighQualityRendering)
			{
				int N=1,n;
				if (Data2[2]==2) N=2;
				if (Data2[2]==3) N=3;

				DC->SelectObject(GetPenFromPool(max(Data3[3]/8+ActualSize/20,1),(IsBlue)?1:0,color));
				DC->SelectObject(GetStockObject(WHITE_BRUSH));
				if (Data1[0]=='O') //circular integral
					DC->Ellipse(X+Data3[2],Y-Data3[3]/2,X+Data3[2]+Data3[1]-((N==1)?0:((N==2)?Data3[3]/3:Data3[3]/5)),Y+Data3[3]/2);

				for (n=0;n<N;n++)
				{
					int X1,Y1,Y2;
					X1=X+Data3[2]+n*Data3[1]/(N+1);  //x_placement
					Y1=Y-Data3[0];
					Y2=Y+Data3[0];
					int h=(Y2-Y1)/4;
					int w=(4*Data3[3])/9; // bilo /10
					int ww=(4*Data3[3])/8;

					DC->Arc(X1+ww,Y1,X1+ww+2*w,Y1+2*h,   X1+ww+10*w/8,Y1+0,X1+ww,Y1+h);
					DC->Arc(X1+ww-2*w,Y2-2*h,X1+ww,Y2,   X1+ww-10*w/8,Y2-0,X1+ww,Y2-h);
					DC->MoveTo(X1+ww,Y1+h-1);
					DC->LineTo(X1+ww,Y2-h);
				}
			}
			else
			{
				int X1,Y1,X2,Y2;

				int PenWidth=3*Data3[3]/4;if (Data3[3]>=12) PenWidth=3+Data3[3]/2; if (PenWidth<2) PenWidth=2;
				int HalfHeight=Data3[0]*4;
				int HalfWidth;

				int N=1;
				int n;
				if (Data2[2]==2) N=2;
				if (Data2[2]==3) N=3;

				mf->StartMyPainting(DC,Data3[1]*4+2*PenWidth,HalfHeight,HalfHeight,color);
				for (n=0;n<N;n++)
				{
					X1=PenWidth+n*Data3[1]*4/(N+1);
					Y1=-HalfHeight;
					X2=Data3[3]*4+n*Data3[1]*4/(N+1)-PenWidth;
					Y2=HalfHeight;
					HalfWidth=(X2-X1)/2;

					int ii;
					for (ii=-PenWidth/2;ii<=PenWidth/2;ii++)
					{
						mf->MyArc(DC,X2-HalfWidth+ii,Y1,X2+HalfWidth-ii,Y1+HalfHeight,  X2,Y1,X2-HalfWidth,Y1+HalfHeight/2,IsBlue);
						mf->MyArc(DC,X2-HalfWidth+ii,Y1+1,X2+HalfWidth-ii,Y1+HalfHeight,  X2,Y1+1,X2-HalfWidth,Y1+HalfHeight/2,IsBlue);

						mf->MyMoveTo(DC,X2-HalfWidth+ii,Y1+HalfHeight/2-1);
						mf->MyLineTo(DC,X1+HalfWidth+ii,Y2-HalfHeight/2+1,IsBlue);
						mf->MyArc(DC,X1-HalfWidth+ii,Y2-HalfHeight,X1+HalfWidth-ii,Y2,  X1,Y2,X1+HalfWidth,Y2-HalfHeight/2,IsBlue);
						mf->MyArc(DC,X1-HalfWidth+ii,Y2-HalfHeight,X1+HalfWidth-ii,Y2-1,  X1,Y2-1,X1+HalfWidth,Y2-HalfHeight/2,IsBlue);
						
						mf->MyArc(DC,X1-3*ii/4-PenWidth/2,Y2-3*PenWidth/4,X1+3*ii/4+PenWidth/2,Y2,  X1,Y2-2*PenWidth,X1,Y2,IsBlue);
						mf->MyArc(DC,X2-3*ii/4-PenWidth/2,Y1,X2+3*ii/4+PenWidth/2,Y1+3*PenWidth/4,  X2,Y1+2*PenWidth,X2,Y1,IsBlue);
					}
				}
				if (Data1[0]=='O') //circular integral
				{
					//int xx=(N*Data3[3]*4)/2;
					//int cxx=4*PenWidth/2+(N-1)*Data3[3]*2;
					int xx=Data3[1]*4/2;
					int cxx=/*4*PenWidth/2+*/Data3[1]*2-PenWidth;
					int cc=((N==1)?0:((N==2)?Data3[3]/3:Data3[3]/5))*2;
					cxx-=cc;
					xx-=cc;
					mf->MyCircle(DC,xx-cxx,Y1+HalfHeight-5*PenWidth/2,xx+cxx,Y1+HalfHeight+5*PenWidth/2,max(3*PenWidth/4,1),IsBlue);
				}

				mf->EndMyPainting(DC,X+Data3[2],Y);
			}
		}
		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		if (Expression3) ((CExpression*)Expression3)->PaintExpression(DC,zoom,X+E3_posX,Y+E3_posY,ClipReg,color);
		if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);
		return;
	}

	if (m_Type==8) //root
	{
		X-=ActualSize/20;
		if (Expression1!=NULL) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		if (Expression2!=NULL) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);
	
		if (1)//!IsHighQualityRendering)
		{
			int h=ActualSize;
			if (Expression1) h=this->E1_above+this->E1_below;

			int penwidth=(ActualSize+h/24)/5; 
			DC->SelectObject(GetPenFromPool(max(penwidth/3,1),(IsBlue)?1:0,color));

			{
				int before=0,after=0,down=0;
				if (IsHighQualityRendering) {before=ActualSize/20;after=ActualSize/8;down=ActualSize/20;}

				CRgn myrgn;
				int X1=X+ActualSize/10-before;
				int X2=X+E1_posX+E1_length+after;
				myrgn.CreateRectRgn(X1,Y-h,X2,Y+h);
				DC->SelectClipRgn(&myrgn);
				DC->MoveTo(X1,Y+E2_posY+E2_below+down);
				DC->LineTo(X+E2_length-ActualSize/10,Y+E2_posY+E2_below+down);
				DC->LineTo(X+E2_length+Data3[0]/2-ActualSize/20,Y+E1_posY+E1_below-ActualSize/12);
				int plafon=Y+E1_posY-E1_above-ActualSize/10+((IsHighQualityRendering)?ActualSize/16:0);
				DC->LineTo(X+E2_length+Data3[0]-((IsHighQualityRendering)?ActualSize/32:0),plafon);
				DC->LineTo(X2,plafon);
				DC->SelectClipRgn(NULL);
			}
		}

		return;
	}

	if (m_Type==9) //vertical line with condition list
	{

		if ((Expression2==NULL) && (Expression3==NULL))
		{
			//an HTML element
			if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,RGB(0,0,192));
			if (Data1[0]=='H')
			{
				//hyperlink
				DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),(IsBlue)?1:0,PALE_RGB(color)));
				int l1=ActualSize/3+ActualSize/20;
				int off=-ActualSize/10;
				int offx=-ActualSize/20;
				int l2=l1;
				X+=offx+this->E1_length;
				Y+=off;
				DC->MoveTo(X,Y);
				DC->LineTo(X+l1,Y);
				DC->LineTo(X+l1,Y+l1);
				DC->LineTo(X,Y+l1);
				DC->LineTo(X,Y);

				DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),(IsBlue)?1:0,RGB(255,0,255)));
				DC->MoveTo(X+l1/2,Y+l1/2);
				DC->LineTo(X+l1/2+l2,Y+l1/2-l2);
				DC->LineTo(X+l1/2+l2,Y+l1/2);
				DC->MoveTo(X+l1/2+l2,Y+l1/2-l2);
				DC->LineTo(X+l1/2,Y+l1/2-l2);
			}
			else
			{
				//label
				int tmp;
				if (IsHighQualityRendering) tmp=0; else tmp=ActualSize/6;
				int sz=max(1,ActualSize/4);
				DC->FillSolidRect(X+E1_length-tmp,Y-E1_above+ActualSize/6,sz,sz,(IsBlue)?BLUE_COLOR:RGB(255,0,255));
			}
		}
		else
		{
			if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
			if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);
			if (Expression3) ((CExpression*)Expression3)->PaintExpression(DC,zoom,X+E3_posX,Y+E3_posY,ClipReg,color);

			DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),(IsBlue)?1:0,color));
			DC->MoveTo(X+E1_length+ActualSize/20,Y-Data3[0]);
			DC->LineTo(X+E1_length+ActualSize/20,Y+Data3[1]);
		}
		return;
	}

	if (m_Type==10) //vertical line with condition list - as an element
	{

		if (Expression1) ((CExpression*)Expression1)->PaintExpression(DC,zoom,X+E1_posX,Y+E1_posY,ClipReg,color);
		if (Expression2) ((CExpression*)Expression2)->PaintExpression(DC,zoom,X+E2_posX,Y+E2_posY,ClipReg,color);
		if (Expression3) ((CExpression*)Expression3)->PaintExpression(DC,zoom,X+E3_posX,Y+E3_posY,ClipReg,color);

		DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),(IsBlue)?1:0,color));

		if (Data1[0]&0x01)
		{
			DC->MoveTo(X+Data3[0]/2,Y+E1_posY-E1_above);
			DC->LineTo(X+Data3[0]/2,Y+E3_posY+E3_below);
		}
		if (Data1[0]&0x02)
		{
			DC->MoveTo(X+Data3[1]+Data3[0]/2,Y+E1_posY-E1_above);
			DC->LineTo(X+Data3[1]+Data3[0]/2,Y+E3_posY+E3_below);
		}
		return;
	}

	if (m_Type==11)
	{
		if (IsBlue)
		{
			POINT p[5];
			DC->SelectObject(GetPenFromPool(1,1));
			p[0].x=X;					p[0].y=Y-ActualSize/5;
			p[1].x=X+ActualSize/6;	p[1].y=Y;
			p[2].x=p[0].x;								p[2].y=Y+ActualSize/5;
			p[3].x=p[0].x;								p[3].y=p[0].y;
			DC->Polyline(p,4);
		}
	}
	if (m_Type==12)
	{
		if (IsBlue)
		{
			POINT p[5];
			DC->SelectObject(GetPenFromPool(1,1));
			p[0].x=X;					p[0].y=Y-ActualSize/5;
			p[1].x=X+ActualSize/5;	p[1].y=p[0].y;
			p[2].x=X+ActualSize/10;	p[2].y=Y+ActualSize/5;
			p[3].x=p[0].x;								p[3].y=p[0].y;
			DC->Polyline(p,4);
		}
	}
}

int CElement::ContainsBlinkingCursor()
{
	if ((this->m_pPaternalExpression==KeyboardEntryObject) && (((CExpression*)this->m_pPaternalExpression)->m_IsKeyboardEntry==this->GetPaternalPosition()+1))
		return 1;
	int t=0;
	if (this->Expression1) 
		if (((CExpression*)Expression1)->ContainsBlinkingCursor()) return 1;
	if (this->Expression2) 
		if (((CExpression*)Expression2)->ContainsBlinkingCursor()) return 1;
	if (this->Expression3) 
		if (((CExpression*)Expression3)->ContainsBlinkingCursor()) return 1;
	return 0;
}


int CElement::IsDifferential(int only_nonparentheses)
{
	// checks if this element is a differential 

	if ((m_Type==6) && (Data1[1]==0) && (m_VMods==0))
	{
		if (this->Expression1==0) return 0;

		if ((((CExpression*)this->Expression1)->m_ParenthesesFlags&0x81) && (only_nonparentheses)) 
		{
			CExpression *e=(CExpression*)this->m_pPaternalExpression;
			if ((e) && (e->m_pPaternalElement) && (only_nonparentheses==2) &&
				(e->m_pPaternalElement->m_Type==4) &&
				(e->m_pPaternalElement->Data1[0]=='d'))
			{
				//this is special handling for the d/d element - we are acting as if there is no parentheses
			}
			else
				return 0;
		}
		if ((Data1[0]=='d') && ((Data2[0]&0xE0)!=0x60))	return 1;
		if ((Data1[0]==(char)0xB6) && ((Data2[0]&0xE0)==0x60)) return 2;
	}
	return 0;
}


//this function empties the object, only adding a simple data -> 'oper'
//the function is specifically used for easy creating of elements in toolboxes
//THE ELEMENTU MUST BE JUST-CREATED and empty deleted
void CElement::Empty(char oper)
{
	//determining wheter this is a text or math element
	if ((m_Type==1) && (this->m_pPaternalExpression) && (((CExpression*)m_pPaternalExpression)->m_StartAsText))
		m_Text=1;
	else
		m_Text=0;

	if (m_Type==1) //variable
	{
		Data1[0]=oper;
		if (oper==(char)0xB0) m_VMods=0x10; //convert to unit (degrees of arc)
		if (oper>=32) return; //for speed

		//special handling for exponent
		if (oper==1) strcpy(Data1,"1");
		if (oper==2) strcpy(Data1,"2");
		if (oper==3) strcpy(Data1,"3");
		//special handling for e and pi, triple dots
		if (oper==4) {strcpy(Data1,"e");Data2[0]=0x22;}
		if (oper==5) {strcpy(Data1,"p");Data2[0]=0x60;}
		if (oper==6) {strcpy(Data1,"H");Data2[0]=(char)0xE3;} //horizontal triple dot
		if (oper==7) {strcpy(Data1,"V");Data2[0]=(char)0xE3;} //vertical triple dot
		if (oper==8) {strcpy(Data1,"A");Data2[0]=(char)0xE3;} //angled triple dot
		if (oper==9) {strcpy(Data1,"U");Data2[0]=(char)0xE3;} //up-angled triple dot
		if (oper==13) {strcpy(Data1,"%");Data2[0]=(char)0xE3;} //permille
		//degrees centigrade
		if (oper==10) {Data1[0]=(char)0xB0;Data1[1]='C';Data1[2]=0;Data2[0]=0x70;Data2[1]=0x20;}
		if (oper==11) {Data1[0]=(char)0xA1;Data1[1]=0;Data2[0]=(char)0x60;} //Greek upercase upsilon (styled)
		//if (oper==12) {strcpy(Data1,"j");Data2[0]=(char)0x22;} //imaginary unit
		return;
	}

	if (m_Type==2) //operator
	{
		Data1[0]=oper;
		if (oper==(char)0xE3)
		{
			Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
			
		}
		return;
	}

	if (m_Type==3) //power
	{
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
		//((CExpression*)Expression1)->m_FontSizeHQ=FontSizeForTypeHQ(1);
		//((CExpression*)Expression2)->m_FontSizeHQ=FontSizeForTypeHQ(2);

		((CExpression*)Expression1)->m_ParenthesesFlags=2; //automatic
		//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
		if (oper!=0) //we have to add a variable into exponent
		{
			if (oper==4) //e^x
			{
				((CExpression*)Expression1)->InsertEmptyElement(0,1,'e');
				((CExpression*)Expression1)->m_pElementList->pElementObject->Data2[0]=0x22; //italic, serif
			}
			else if (oper==5) //10^x
			{
				((CExpression*)Expression1)->InsertEmptyElement(0,1,'1');
				strcpy(((CExpression*)Expression1)->m_pElementList->pElementObject->Data1,"10");
				((CExpression*)Expression1)->m_pElementList->pElementObject->Data2[0]=0x00;
				((CExpression*)Expression1)->m_pElementList->pElementObject->Data2[0]=0x00;
			}
			else
			{
				if (oper<=3) ((CExpression*)Expression2)->InsertEmptyElement(0,2,'-');
				((CExpression*)Expression2)->InsertEmptyElement(1,1,oper);
			}
		}
		return;
	}

	if (m_Type==4) //rational number
	{
		Data1[0]=oper;
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
		if (oper==0) return; //for speed

		/*if (oper==9)  // /square root
		{
			Data1[0]=0;
			((CExpression*)Expression2)->InsertEmptyElement(0,8,1);
		}*/
		if (oper==10) // /2
		{
			Data1[0]=0;
			((CExpression*)Expression2)->InsertEmptyElement(0,1,'2');
		}

		char func=23;
		if ((Data1[0]>4) && (Data1[0]<=8)) {func=24;Data1[0]-=4;}
		if (Data1[0]=='p') {func=24;Data1[0]='d';}
		if (Data1[0]==1)  //  d /dx
		{
			((CExpression*)Expression1)->InsertEmptyElement(0,6,func);
			((CExpression*)Expression2)->InsertEmptyElement(0,6,func);

		}		
		if ((Data1[0]==2) || (Data1[0]=='d'))  //  d() /dx
		{
			((CExpression*)Expression1)->InsertEmptyElement(0,6,func);
			if (((CExpression*)Expression1)->m_pElementList->pElementObject)
			{
				
				CExpression *E2=(CExpression*)(((CExpression*)Expression1)->m_pElementList->pElementObject->Expression1);
				E2->m_ParenthesesFlags|=0x1;
				E2->m_FontSize=E2->m_FontSize*10/9;
			}
			((CExpression*)Expression2)->InsertEmptyElement(0,6,func);
		}
		if (Data1[0]==3)  //  d2 /(dx)2
		{
			((CExpression*)Expression1)->InsertEmptyElement(0,3,'2');
			((CExpression*)Expression2)->InsertEmptyElement(0,3,'2');
			if (((CExpression*)Expression1)->m_pElementList->pElementObject)
			{
				CExpression *E3=(CExpression*)(((CExpression*)Expression1)->m_pElementList->pElementObject->Expression1);
				E3->InsertEmptyElement(0,6,func);
				CExpression *E4=(CExpression*)(((CExpression*)Expression2)->m_pElementList->pElementObject->Expression1);
				E4->InsertEmptyElement(0,6,func);
				E4->m_ParenthesesFlags|=0x01;
			}
		}
		if (Data1[0]==4)  //  d2()/(dx)2
		{
			((CExpression*)Expression1)->InsertEmptyElement(0,3,'2');
			((CExpression*)Expression2)->InsertEmptyElement(0,3,'2');
			if (((CExpression*)Expression1)->m_pElementList->pElementObject)
			{
				CExpression *E3=(CExpression*)(((CExpression*)Expression1)->m_pElementList->pElementObject->Expression1);
				E3->InsertEmptyElement(0,6,func);
				CExpression *E5=(CExpression*)(E3->m_pElementList->pElementObject->Expression1);
				E5->m_ParenthesesFlags|=0x01;
				CExpression *E4=(CExpression*)(((CExpression*)Expression2)->m_pElementList->pElementObject->Expression1);
				E4->InsertEmptyElement(0,6,func);
				E4->m_ParenthesesFlags|=0x01;
			}
		}
		return;
	}

	if (m_Type==5) //parenthese
	{
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		if ((oper&0x80)==0)
		{
			//standard parentheses
			((CExpression*)Expression1)->m_ParenthesesFlags=2; //automatic
			if (oper=='l') {oper='{';((CExpression*)Expression1)->m_ParenthesesFlags|=0x10;} //left-only curly bracket
			if (oper=='r') {oper='{';((CExpression*)Expression1)->m_ParenthesesFlags|=0x08;} //right-only curly braket
			((CExpression*)Expression1)->m_ParentheseShape=oper;
		}
		else
		{
			//matrix - rows and columns are in first 7 bits;
			((CExpression*)Expression1)->m_ParentheseShape='[';
			((CExpression*)Expression1)->m_ParenthesesFlags=2; //automatic
			//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;
			int columns=((oper>>4)&0x07);
			int rows=(oper&0x0F);
			((CExpression*)Expression1)->FindMatrixElement(rows,columns,1); //this builds the matrix
			((CExpression*)Expression1)->AdjustMatrix();
		}
		return;
	}

	if (m_Type==6) //function
	{
		Data1[0]=oper;
		Data1[1]=0;
		Data2[0]=0x20; //font face

		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		((CExpression*)Expression1)->m_ParenthesesFlags=2; //automatic
		//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;

		if (oper==1) strcpy(Data1,"sin");
		if (oper==2) strcpy(Data1,"cos");
		if (oper==3) strcpy(Data1,"tg");
		if (oper==26) strcpy(Data1,"tan");
		if (oper==4) strcpy(Data1,"cot");
		if (oper==27) strcpy(Data1,"sec");
		//if (oper==28) strcpy(Data1,"cosec"); //not used
		if (oper==29) strcpy(Data1,"csc");
		if (oper==30) strcpy(Data1,"arcsec");
		if (oper==31) strcpy(Data1,"arccsc");
		if (oper==5) strcpy(Data1,"arcsin");
		if (oper==6) strcpy(Data1,"arccos");
		if (oper==7) strcpy(Data1,"arctg");
		if (oper==8) strcpy(Data1,"arccot");
		if (oper==9) strcpy(Data1,"sh");
		if (oper==10) strcpy(Data1,"ch");
		if (oper==11) strcpy(Data1,"th");
		if (oper==12) strcpy(Data1,"coth");
		if (oper==13) strcpy(Data1,"arsh");
		if (oper==14) strcpy(Data1,"arch");
		if (oper==15) strcpy(Data1,"arth");
		if (oper==16) strcpy(Data1,"arcoth");
		if (oper==32) strcpy(Data1,"sech");
		if (oper==33) strcpy(Data1,"csch");
		if (oper==34) strcpy(Data1,"arsech");
		if (oper==35) strcpy(Data1,"arcsch");
		if (oper==36) strcpy(Data1,"func");
		if (oper==40) strcpy(Data1,"arctan");

		if (oper==17) strcpy(Data1,"ln");
		if (oper==18) //log10
		{
			strcpy(Data1,"log");
			Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
			((CExpression*)Expression2)->InsertEmptyElement(0,1,' ');
			strcpy(((CExpression*)Expression2)->m_pElementList->pElementObject->Data1,"10");
			((CExpression*)Expression2)->m_pElementList->pElementObject->Data2[0]=0;
			((CExpression*)Expression2)->m_pElementList->pElementObject->Data2[1]=0;
		}
		if (oper==19) strcpy(Data1,"log");
		if (oper==20) strcpy(Data1,"rot");
		if (oper==21) {Data1[0]=(char)0xD1;Data2[0]=0x60;} //nabla operator
		if (oper==22) {Data1[0]=(char)0x44;Data2[0]=0x60;} //delta operator
		if (oper==23) {Data1[0]=(char)'d'; Data2[0]=0x20;} //derivation sign
		if (oper==24) {Data1[0]=(char)0xB6;Data2[0]=0x60;} //partial derivation sign
		if (oper==25) 
		{
			strcpy(Data1,"lim"); 
			Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
		}
		if (oper==37) {Data1[0]=(char)'d';Data2[0]=0x60;} //dirac delta
		if (oper==38) {Data1[0]=(char)0xC2;Data2[0]=0x60;} //Real part
		if (oper==39) {Data1[0]=(char)0xC1;Data2[0]=0x60;} //Imaginary part
	}

	if (m_Type==7) //sigma, pi, integral, limes
	{
		Data1[0]=oper;
		Data1[1]=0;
		Data2[0]=1; //symbol height
		Data2[1]=0; //integration limits placement (0=above,below;  1=aside)
		Data2[2]=0; //for double and triple integrals (2 and 3)
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		//((CExpression*)Expression1)->m_FontSizeHQ=FontSizeForTypeHQ(1);

		((CExpression*)Expression1)->m_ParenthesesFlags=2;	//automatic	
		//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
		if ((oper=='I'-1) || (oper=='i'-1) || (oper=='O'-1) || (oper=='o'-2))
		{
			//adding dx
			Data1[0]+=1;
			((CExpression*)Expression1)->InsertEmptyElement(0,6,23); // dx
		}
		if ((oper=='I'-2) || (oper=='i'-2) || (oper=='O'-2) || (oper=='o'-2))
		{  
			//adding parentheses and dx
			Data1[0]+=2;
			((CExpression*)Expression1)->InsertEmptyElement(0,5,'(');
			((CExpression*)Expression1)->InsertEmptyElement(1,6,23);
		}
		if ((oper=='I'-3) || (oper=='i'-3) || (oper=='O'-3) || (oper=='o'-3))
		{  
			//double integral
			Data1[0]+=3;
			Data2[2]=2; 
		}
		if ((oper=='I'-4) || (oper=='i'-4) || (oper=='O'-4) || (oper=='o'-4))
		{  
			//triple integral
			Data1[0]+=4;
			Data2[2]=3; 
		}
		if ((oper=='|') || (oper=='/') || (oper<'a')) //lowercase letters represent non-determined integrals
		{
			Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
			//((CExpression*)Expression2)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			//((CExpression*)Expression2)->m_FontSizeHQ=FontSizeForTypeHQ(2);

			Expression3=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(3));
			//((CExpression*)Expression3)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			//((CExpression*)Expression3)->m_FontSizeHQ=FontSizeForTypeHQ(3);

		}
		Data1[0]=toupper(Data1[0]);

	}
	if (m_Type==8) //root
	{
		Data1[0]=oper;
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		((CExpression*)Expression1)->m_ParenthesesFlags=0;

		if (oper==9) //square root of 2
		{
			((CExpression*)Expression1)->InsertEmptyElement(0,1,'2');
		}
		else if (oper!=1)  //root with index
		{
			Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
			if (oper)
				((CExpression*)Expression2)->InsertEmptyElement(0,1,oper);
		}
	}
	if (m_Type==9) //vertical line with condition list
	{

		Data1[0]=oper;
		if ((oper=='H') || (oper=='L'))
		{
			//an HTML element
			Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
			((CExpression*)Expression1)->m_ParenthesesFlags=0;
			//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			((CExpression*)Expression1)->m_StartAsText=1;

			Expression2=Expression3=NULL;
			Data3[0]=Data3[1]=0;
		}
		else
		{
			Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
			((CExpression*)Expression1)->m_ParenthesesFlags=0;
			//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
			//((CExpression*)Expression2)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			Expression3=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(3));
			//((CExpression*)Expression3)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			//((CExpression*)Expression1)->m_FontSizeHQ=FontSizeForTypeHQ(1);
			//((CExpression*)Expression2)->m_FontSizeHQ=FontSizeForTypeHQ(2);
			//((CExpression*)Expression3)->m_FontSizeHQ=FontSizeForTypeHQ(3);
			((CExpression*)Expression2)->m_Alignment=1;
			((CExpression*)Expression3)->m_Alignment=1;
		}
	}
	if (m_Type==10) //vertical line with condition list - as an element
	{
		Data1[0]=oper;
		Data2[0]=0;
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(1));
		//((CExpression*)Expression1)->m_FontSizeHQ=FontSizeForTypeHQ(1);
		//((CExpression*)Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
		((CExpression*)Expression1)->m_Alignment=1;
		if ((oper&0x04)==0)
		{
			Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(2));
			//((CExpression*)Expression2)->m_FontSizeHQ=FontSizeForTypeHQ(2);
			//((CExpression*)Expression2)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			((CExpression*)Expression2)->m_Alignment=1;
		}
		Expression3=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,FontSizeForType(3));
		//((CExpression*)Expression3)->m_FontSizeHQ=FontSizeForTypeHQ(3);
		//((CExpression*)Expression3)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
		((CExpression*)Expression3)->m_Alignment=1;
	}
	if ((m_Type==11) || (m_Type==12))
	{
		memset(Data1,32,20);
		Data1[21]='?'; //marks that data is uninitialized
		Data1[20]=0;
	}
	return;
}

//this function deletes all data from the element (including subexpressions)
/*int CElement::Delete(void)
{
	if (Expression1) {delete (CExpression*)Expression1;Expression1=NULL;}
	if (Expression2) {delete (CExpression*)Expression2;Expression2=NULL;}
	if (Expression3) {delete (CExpression*)Expression3;Expression3=NULL;}

	m_Color=-1;
	return 1;
}*/

//copies some other element data over this one - the element must be of same type
//the 'this' element must be empty (no subexpressions)!
void CElement::CopyElement(const CElement* Element)
{
	memcpy(Data2,Element->Data2,ElementMemoryReservations[m_Type].Resize+24+4);
	m_Color=Element->m_Color;
	m_Text=Element->m_Text;
	m_VMods=Element->m_VMods;

	for (int i=0;i<3;i++)
	{
		CExpression *oexp;
		CExpression *exp;
		if (i==0) {oexp=(CExpression*)(Element->Expression1);}
		else if (i==1) {oexp=(CExpression*)(Element->Expression2);}
		else if (i==2) {oexp=(CExpression*)(Element->Expression3);}
		if (oexp)
		{
			//calculate font size for subexpression - font must be proportional between subexpression and paternal expression
			int FontSize=macroFontSizeForType(i);
			if ((Element->m_pPaternalExpression) && (FontSize) && (oexp->m_FontSize))
			{
				int NewFontSize=((CExpression*)(m_pPaternalExpression))->m_FontSize *
					(oexp->m_FontSize) /
					((CExpression*)(Element->m_pPaternalExpression))->m_FontSize;
				if ((abs(NewFontSize-FontSize)*100/FontSize)>2)
					FontSize=NewFontSize;
			}
			exp=new CExpression(this,(CExpression*)m_pPaternalExpression,FontSize);
			exp->CopyExpression(oexp,0);
			if (i==0) Expression1=(CObject*)exp;
			else if (i==1) Expression2=(CObject*)exp;
			else if (i==2) Expression3=(CObject*)exp;
		}
	}
	if ((m_Type==9) && (Expression2==NULL) && (Expression3==NULL) && (Data1[0]=='H') && (*(char**)Element->Data3))
	{
		//copy hyperlinks
		try
		{
			char *link=(char*)malloc(340);
			memcpy(link,*(char**)Element->Data3,340);
			*(char**)Data3=link;
		}
		catch(...)
		{

		}
	}
	return;
}

//when creating subexpressions, this function returns the size of font in subexpression
int CElement::FontSizeForType(int subexpression)
{
	return macroFontSizeForType(subexpression-1);
}

int CElement::GetPaternalPosition()
{
	CExpression *p=(CExpression*)this->m_pPaternalExpression;
	tElementStruct *ts=p->m_pElementList;
	for (int i=0;i<p->m_NumElements;i++,ts++)
		if (ts->pElementObject==this) 
			return i;
	return 0;
}

CElement *CElement::GetPreviousElement()
{
	CExpression *e=(CExpression*)m_pPaternalExpression;
	int n=e->m_NumElements;
	tElementStruct *ts=e->m_pElementList+1;
	for (int i=1;i<n;i++,ts++)
		if (ts->pElementObject==this) return (ts-1)->pElementObject;
	return NULL;
} 

CElement *CElement::GetNextElement()
{
	CExpression *e=(CExpression*)m_pPaternalExpression;
	int n=e->m_NumElements;
	tElementStruct *ts=e->m_pElementList;
	for (int i=0;i<n-1;i++,ts++)
		if (ts->pElementObject==this) return (ts+1)->pElementObject;
	return NULL;
}


//returns an object where cursor is pointing (it select this object and its subexpressions)
//function is used for touching object with mouse
//this function must be fast!
CObject *CElement::SelectAtPoint(CDC* DC, short zoom, short X, short Y, short* IsExpression,char *IsParenthese, short paternal_position)
{

	//special handling for the HTML link element (we will select the whole element so it can be clicked on it)
	if ((this->m_Type==9) && (this->Data1[0]=='H') && (Expression2==NULL) && (Expression3==NULL) && (Expression1) &&
		(((CExpression*)Expression1)->m_pElementList->Type))
	{
		*IsExpression=0;
		if (Expression1) ((CExpression*)Expression1)->SelectExpression(1);
		return (CObject*)this;
	}

	//first check if the cursor is pointng at any of subexpression
	if (Expression2)
	{
		int extends=0;
		int extends2=0;
		if ((this->m_Type==4) || (this->m_Type==7) || (this->m_Type==6) || (this->m_Type==10) || (this->m_Type==8) || (this->m_Type==5)) extends=5*((CExpression*)Expression2)->m_MarginX/2;
		if (this->m_Type==3) {extends2=((CExpression*)Expression1)->m_MarginX;extends=extends=((CExpression*)Expression2)->m_MarginX;}
		int ttt=0;
		if (IsHighQualityRendering) ttt=((CExpression*)Expression2)->GetActualFontSize(zoom)/12;
		if ((X>=E2_posX-ttt-extends) && (X<=E2_posX+E2_length+ttt+extends) &&
			(Y>E2_posY-E2_above) && (Y<E2_posY+E2_below-extends2))
		{
			CObject *obj;
			
			obj=((CExpression*)(Expression2))->SelectObjectAtPoint(DC,zoom,X-E2_posX,Y-E2_posY,IsExpression,IsParenthese);
			return obj;
		}
	}
	if (Expression3)
	{
		int extends=0;
		if ((this->m_Type==4) || (this->m_Type==7) || (this->m_Type==6) || (this->m_Type==3) || (this->m_Type==10) || (this->m_Type==8) || (this->m_Type==5)) extends=5*((CExpression*)Expression3)->m_MarginX/2;

		int ttt=0;
		if (IsHighQualityRendering) ttt=((CExpression*)Expression3)->GetActualFontSize(zoom)/12;

		if ((X>=E3_posX-ttt-extends) && (X<=E3_posX+E3_length+ttt+extends) &&
			(Y>E3_posY-E3_above) && (Y<E3_posY+E3_below))
		{
			CObject *obj;
			obj=((CExpression*)(Expression3))->SelectObjectAtPoint(DC,zoom,X-E3_posX,Y-E3_posY,IsExpression,IsParenthese);
			return obj;
		}
	}
	if (Expression1) //this is the last subexpression to be checked because in some cases it covers other subexpressions
	{
		int ttt=0;
		int ttt2=0;
		if (IsHighQualityRendering)
		{
			int as=((CExpression*)Expression1)->GetActualFontSize(zoom);
			if (m_Type==4) ttt=ttt2=as/8;
			if ((m_Type==10) || (m_Type==1)) ttt=ttt2=as/12;
			if ((m_Type==6) || (m_Type==8)) ttt=as/12;
			if (m_Type==7) {if (Data1[0]=='/') ttt2=as/12; else ttt=as/12;}
			if (m_Type==9) ttt2=as/12;
			//if (m_Type==5) ttt=ttt2=as/12;
		}
		int extends=0;
		int extends2=0;
		if ((this->m_Type==4) || (this->m_Type==10)) extends=5*((CExpression*)Expression1)->m_MarginX/2;
		if (this->m_Type==1) {extends2=((CExpression*)Expression1)->m_MarginX;extends=((CExpression*)Expression1)->m_MarginX;}

		if ((X>=E1_posX-ttt-extends) && (X<=E1_posX+E1_length+ttt2+extends) &&
			(Y>E1_posY-E1_above+extends2) && (Y<E1_posY+E1_below))
		{
			CObject *obj;
			obj=((CExpression*)(Expression1))->SelectObjectAtPoint(DC,zoom,X-E1_posX,Y-E1_posY,IsExpression,IsParenthese);
			return obj;
		}
	}

	//if not pointing at subexpression, then it is pointing at the element itself
	*IsExpression=0;
	*IsParenthese=0;

	//babaluj
	if ((this->m_Type==1) && (this->m_Text) && (strlen(this->Data1)>=1))
	{
		int ok=0;
		if (TouchMouseMode==0) ok=1;
		if ((((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint) && (TouchMouseMode==1)) ok=1;
		((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint=0;
		if (ClipboardExpression==NULL)
		if ((((GetKeyState(VK_SHIFT)&0xFFFE)==0) && (ok)) || (DisableMultitouch))
		{	
			if ((X<=Data3[1]/2) && (this->m_Text!=2))
			{
				((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint=1;
				return (CObject*)this;
			}
			int i=1;
			while (Data1[i])
			{
				if ((X>=((Data3[i-1]+Data3[i])/2)) && (X<((Data3[i+1]+Data3[i])/2))) 
				{
					((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint=Data3[i];
					return (CObject*)this;
				}
				i++;
			}
			if (this->m_Text!=4)
				((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint=Data3[i];
			else
				((CExpression*)m_pPaternalExpression)->m_InternalInsertionPoint=(i>1)?Data3[i-1]:1;
			return (CObject*)this;
		}
	}

	if ((GetKeyState(VK_MENU)&0xFFFE)) return NULL;
	

	SelectedTab=NULL;

	if ((GetKeyState(16)&0xFFFE)==0)  //shift not pressed
	if (TouchMouseMode==0)
	if ((m_Type==2) && (Data1[0]==9) && (Data1[3]!=1))
	{
		int len=Data3[0]*2+Data3[1]/16;
		int c2=(Data3[1]+28)/16;

		if (X>=len-c2-1)
		{
			//touching tab character head - to select it for lenght adjustment
			SelectedTab=this;
			POINT cursor;
			GetCursorPos(&cursor);
			pMainView->ScreenToClient(&cursor);
			SelectedTabX=cursor.x-X;
			*IsParenthese=1; //just to trigger repainting
		}
		else 
			*IsParenthese=0;
	}




	//pointing at the right half of some operators can select multiple elements
	if ((GetKeyState(16)&0xFFFE)==0)  //shift not pressed
	if (TouchMouseMode==0)
	if ((m_Type==2) && (X>(((CExpression*)m_pPaternalExpression)->m_pElementList+paternal_position)->Length/2))
	{
		char ch=Data1[0];

		//check if it is pointing at '+' or '-'
		if (GetOperatorLevel(ch)==PlusLevel)
		{
			//select all elements until another operator
			(((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position)->IsSelected=1;
			for (int jj=paternal_position+1;jj<((CExpression*)(m_pPaternalExpression))->m_NumElements;jj++)
			{
				tElementStruct *ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+jj;
				if ((ts->Type==1) && (ts->pElementObject->m_Text)) break;
				if ((ts->Type==2) && (GetOperatorLevel(ts->pElementObject->Data1[0])<=PlusLevel)) break;
				if ((ts->Type==11) || (ts->Type==12) || (ts->Type==9) || (ts->Type==10)) break;
				if (ts->Type==0) break;
				((CExpression*)(m_pPaternalExpression))->SelectElement(1,jj);
			}
			return (CObject*)(this->m_pPaternalExpression);	
		}

		//check if it is pointing =,<,>,<<,>>,<=,>=,...
		if ((GetOperatorLevel(ch)==EqLevel)	||
			(ch==(char)0xBA) || (ch==(char)0x40) || (ch==(char)0xB5) || (ch==(char)0x7E))			
		{
			//select all elements until '=' operator
			(((CExpression*)(m_pPaternalExpression))->m_pElementList+paternal_position)->IsSelected=1;
			for (int jj=paternal_position+1;jj<((CExpression*)(m_pPaternalExpression))->m_NumElements;jj++)
			{
				tElementStruct *ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+jj;
				if ((ts->Type==1) && (ts->pElementObject->m_Text)) break;
				if ((ts->Type==2) && (ts->pElementObject))
				{
					char ch=ts->pElementObject->Data1[0];
					if ((GetOperatorLevel(ch)<=EqLevel) ||
						(ch==(char)0xBA) || (ch==0x7E) || (ch==0x40) || (ch==(char)0xB5))
						break;
				}
				if ((ts->Type==11) || (ts->Type==12) || (ts->Type==9) || (ts->Type==10)) break;
				((CExpression*)(m_pPaternalExpression))->SelectElement(1,jj);
			}
			return (CObject*)(this->m_pPaternalExpression);
		}
	}

	if (ContainsBlinkingCursor()) return NULL;

	//selecting all subexpression
	if (Expression1) ((CExpression*)(Expression1))->SelectExpression(1);
	if (Expression2) ((CExpression*)(Expression2))->SelectExpression(1);
	if (Expression3) ((CExpression*)(Expression3))->SelectExpression(1);

	return (CObject*)this;
}

//XML_output, trasforms all relevant element data into XML string
//used for save (to disk) and copy (to clipboard)
//should not be too slow (copy/paste)
#pragma optimize("s",on)
int CElement::XML_output(char * output, int num_tabs, char only_calculate)
{
	int len=0;
	int tmp;
	short i;
	char *E1=NULL;
	char *E2=NULL;
	char *E3=NULL;

	static char tmpstr[136]; //we are using this functin recursivley, so take care not to use too much memory
	//static char tabs[17];

	if (num_tabs>16) num_tabs=16;
	//memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string

	if (!only_calculate) {memset(output,9,num_tabs);output+=num_tabs;}
	len+=num_tabs;

	if ((XMLFileVersion==1) || (m_Type>=7))
	{
		sprintf(tmpstr,"<elm tp=\"%d\" ",m_Type);
		tmp=(int)strlen(tmpstr);
		len+=tmp;
		if (!only_calculate) {strcpy(output,tmpstr);output+=tmp;}
	}
	else 
	{
		if (!only_calculate)
		{
			if (m_Type==1) strcpy(output,"<var ");
			if (m_Type==2) strcpy(output,"<opr ");
			if (m_Type==3) strcpy(output,"<pwr ");
			if (m_Type==4) strcpy(output,"<fra ");
			if (m_Type==5) strcpy(output,"<bra ");
			if (m_Type==6) strcpy(output,"<fun ");
			output+=5;
		}
		len+=5;
	}

	if (m_Color!=-1)
	{
		strcpy(tmpstr," color=\"");
		tmp=(short)strlen(tmpstr);
		len+=tmp;
		if (!only_calculate) {strcpy(output,tmpstr);output+=tmp;}

		itoa(m_Color,tmpstr,10);
		strcat (tmpstr,"\" ");
		tmp=(short)strlen(tmpstr);
		len+=tmp;
		if (!only_calculate) {strcpy(output,tmpstr);output+=tmp;}
	}

	static CElement *d_this=this;
	static CExpression *d_parent=(CExpression*)m_pPaternalExpression;
	static int d_cnt;

	//addidng decoration description (encircled, strikeout, underlined, overlined...)
	if (m_pPaternalExpression)
	{
		for (i=0;i<((CExpression*)(m_pPaternalExpression))->m_NumElements;i++)
		{
			d_cnt=i;
			tElementStruct *ts=((CExpression*)(m_pPaternalExpression))->m_pElementList+i;
			if (ts->pElementObject==this)
			{
				if (ts->Decoration) 
				{
					sprintf(tmpstr,"decor=\"%d\" ",ts->Decoration);
					tmp=(short)strlen(tmpstr);
					len+=tmp;
					if (!only_calculate) {strcpy(output,tmpstr);output+=tmp;}
				}
				break;
			}
		}
	}

	tmpstr[0]=0;
	if ((m_Type==1) || //variable
		(m_Type==6))   //function
	{
		short j;
		if (XMLFileVersion==1) {strcpy(tmpstr,"tx=\"");j=4;} else {strcpy(tmpstr,"t=\"");j=3;}
		tmp=(short)strlen(Data1);
		for (i=0;i<tmp;i++)
		{
			if ((Data1[i]<' ') || (Data1[i]>0x7E) || (Data1[i]=='\\') || (Data1[i]=='"'))
			{
				char ppp[4];
				sprintf(ppp,"\\%02X",(unsigned char)Data1[i]);
				memcpy(tmpstr+j,ppp,3);
				j+=3;
			}
			else
				tmpstr[j++]=Data1[i];
		}

		if (m_VMods)
		{
			memcpy(tmpstr+j,"\" mods=\"",8);
			j+=8;
			char ppp[4];
			sprintf(ppp,"%02X",(unsigned char)m_VMods);
			tmpstr[j++]=ppp[0];
			tmpstr[j++]=ppp[1];
		}

		int all_chars_same_font=1;
		if (XMLFileVersion==1) 
		{
			all_chars_same_font=0;
			memcpy(tmpstr+j,"\" fnt=\"",7);j+=7;
		} 
		else 
		{
			memcpy(tmpstr+j,"\" f=\"",5);j+=5;
			for (i=1;i<tmp;i++) if (Data2[i]!=Data2[0]) {all_chars_same_font=0;break;} //we check if all characters are of the same font type
		}
		for (i=0;i<tmp;i++)
		{
			char ppp[4];
			unsigned char dd=(unsigned char)Data2[i];
			if (XMLFileVersion==1) dd=(dd&0xE3)|(m_VMods&0x1C);
			sprintf(ppp,"%02X",(unsigned char)dd);
			tmpstr[j++]=ppp[0];
			tmpstr[j++]=ppp[1];
			if ((m_Type==6) || (all_chars_same_font)) break;
		}
		memcpy(tmpstr+j,"\"\0",2);

		if (((CExpression*)m_pPaternalExpression)->m_StartAsText)
		{
			if (m_Text==0)	strcat(tmpstr," mth=\"1\"");
		}
		else
		{
			if (m_Text!=0) strcat(tmpstr," mth=\"0\"");
		}
		if (m_Text==2) strcat(tmpstr," ttxt=\"2\"");
		if (m_Text==3) strcat(tmpstr," ttxt=\"3\"");
		if (m_Text==4) strcat(tmpstr," ttxt=\"4\"");

		if (m_Type==1)
		{
			if ((Data1[14]==0) && ((Data1[15]&0x7F)==126))
			{
				//adding the floating point value data (for numbers)
				strcat(tmpstr," float=\"");
				for (int ij=0;ij<8;ij++)
				{
					char bfff[10];
					sprintf(bfff,"%08X",Data1[16+ij]);
					strcat(tmpstr,bfff+6);
				}
				if (Data1[15]&0x80) strcat(tmpstr,"_"); else strcat(tmpstr," ");
				strcat(tmpstr,"\"");
			}

			if (Expression1) E1="i"; 
		}
		else
		{
			if (Expression1) E1="";
			if (Expression2) E2="i";
		}
	}

	if (m_Type==2)  //operator
	{
		if ((Data1[0]<' ') || (Data1[0]>0x7E) || (Data1[0]=='\\') || (Data1[0]=='\'') || (Data1[0]=='\"') || (Data1[0]=='<') || (Data1[0]=='>'))
		{
			if (XMLFileVersion==1) sprintf(tmpstr,"stp=\"\\%02X\"",(unsigned char)Data1[0]); else sprintf(tmpstr,"s=\"\\%02X\"",(unsigned char)Data1[0]);
		}
		else
		{
			if (XMLFileVersion==1) sprintf(tmpstr,"stp=\"%c\"",Data1[0]); else sprintf(tmpstr,"s=\"%c\"",Data1[0]);
		}
		if ((Data1[0]==9) && (Data1[3]!=0))
		{
			char tmps[32];
			sprintf(tmps," tablen=\"%d\"",Data1[3]);
			strcat(tmpstr,tmps);
		}
		if (Expression1) E1="upp";
	}

	if (m_Type==3)  //exponent (power)
	{
		strcpy(tmpstr,"");
		if (Expression1) E1="";
		if (Expression2) E2="e";
	}

	if (m_Type==4)  //fraction (rational number), a over b
	{
		if (Data1[0]=='/') strcpy(tmpstr,"stp=\"semi-fraction\"");
		else if (Data1[0]==' ') strcpy(tmpstr,"stp=\"a-over-b\"");
		else if (Data1[0]=='d') strcpy(tmpstr,"stp=\"dfrac\"");
		else strcpy(tmpstr,"stp=\"\"");
		if (Expression1) E1="n";
		if (Expression2) E2="d";
	}

	if (m_Type==5) //parentheses
	{
		if (Expression1) E1="";
		if (Expression2) E2="i";
	}

	if (m_Type==7) //sigma, pi, integral
	{
		if (Data1[0]=='S') strcpy(tmpstr,"stp=\"Sigma\"");
		else if (Data1[0]=='P') strcpy(tmpstr,"stp=\"Pi\"");
		else if (Data1[0]=='I') {if (XMLFileVersion==1) strcpy(tmpstr,"stp=\"Integral\""); else strcpy(tmpstr,"stp=\"Int\"");}
		else if (Data1[0]=='O') strcpy(tmpstr,"stp=\"Circular-integral\"");
		else sprintf(tmpstr,"stp=\"%c\"",Data1[0]);
	
		if (XMLFileVersion==1) strcat(tmpstr," symbol_height=\""); else strcat(tmpstr," sze=\"");
		char h[10];itoa(Data2[0],h,10);
		strcat(tmpstr,h);strcat(tmpstr,"\"");

		if (Data2[1]!=1) Data2[1]=0;
		if ((XMLFileVersion==1) || (Data2[1]))
		{
			strcat(tmpstr," limits_aside=\"");
			itoa(Data2[1],h,10);
			strcat(tmpstr,h);strcat(tmpstr,"\"");
		}

		if ((Data1[0]=='I') || (Data1[0]=='O'))
		{
			if ((Data2[2]!=2) && (Data2[2]!=3)) Data2[2]=1;
			if ((XMLFileVersion==1) || (Data2[2]!=1))
			{
				strcat(tmpstr," dimension=\"");
				itoa(Data2[2],h,10);
				strcat(tmpstr,h);strcat(tmpstr,"\"");
			}
		}

		if (Expression1) E1="";
		if (Expression2) E2="h";
		if (Expression3) E3="l";
	}
	if (m_Type==8) //root
	{
		if (Expression1) E1="";
		if (Expression2) E2="ndx";
	}
	if (m_Type==9) //condition list
	{
		if (Expression1) E1="";
		if (Expression2) E2="h";
		if (Expression3) E3="l";
		if ((Expression3==0) && (Expression2==0) && (Data1[0]=='L'))
			strcat(tmpstr,"label=\"1\"");
		if ((Expression3==0) && (Expression2==0) && (Data1[0]=='H'))
		{

			if ((*(char**)this->Data3)==NULL)
				strcat(tmpstr," URL=\"\"");
			else
			{
				char *url=*(char**)this->Data3;
				int tmp=(int)strlen(url);
				strcat(tmpstr," URL=\"");
				int j=(int)strlen(tmpstr);
				for (int i=0;i<tmp;i++)
					if ((url[i]<' ') || (url[i]>0x7E) || (url[i]=='\\') || (url[i]=='"'))
					{
						char ppp[4];
						sprintf(ppp,"\\%02X",(unsigned char)url[i]);
						memcpy(tmpstr+j,ppp,3);
						j+=3;
					}
					else
						tmpstr[j++]=url[i];
				tmpstr[j]=0;

				//strcat(tmpstr,*(char**)this->Data3);
				strcat(tmpstr,"\"");
			}			
		}
	}
	if (m_Type==10) //condition list, as element
	{
		strcpy(tmpstr,"");
		if (Data1[0]&0x01) strcat(tmpstr,"left_bar=\"1\""); else strcat(tmpstr,"left_bar=\"0\"");
		if (Data1[0]&0x02) strcat(tmpstr," right_bar=\"1\""); else strcat(tmpstr," right_bar=\"0\"");
		if ((Data2[0]<0) || (Data2[0]>2)) Data2[0]=0;
		strcat(tmpstr," align=\"");
		char tmp[10];
		itoa(Data2[0],tmp,10);
		strcat(tmpstr,tmp);
		strcat(tmpstr,"\"");
		if (Expression1) E1="h";
		if (Expression2) E2="m";
		if (Expression3) E3="l";
	}

	if (XMLFileVersion==1)
	{
		if (E1) {strcat(tmpstr," Exp1=\"");strcat(tmpstr,E1);strcat(tmpstr,"\"");}
		if (E2) {strcat(tmpstr," Exp2=\"");strcat(tmpstr,E2);strcat(tmpstr,"\"");}
		if (E3) {strcat(tmpstr," Exp3=\"");strcat(tmpstr,E3);strcat(tmpstr,"\"");}
	}
	else
	{
		if (E1) {strcat(tmpstr," E1=\"");strcat(tmpstr,E1);strcat(tmpstr,"\"");}
		if (E2) {strcat(tmpstr," E2=\"");strcat(tmpstr,E2);strcat(tmpstr,"\"");}
		if (E3) {strcat(tmpstr," E3=\"");strcat(tmpstr,E3);strcat(tmpstr,"\"");}
	}

	if ((Expression1==NULL) && (Expression2==NULL) && (Expression3==NULL))
		strcat(tmpstr," />\r\n");
	else
		strcat(tmpstr,">\r\n");
	tmp=(short)strlen(tmpstr);
	len+=tmp;
	if (!only_calculate) {strcpy(output,tmpstr);output+=tmp;}

	if (Expression1)
	{
		tmp=((CExpression*)(Expression1))->XML_output(output,num_tabs,only_calculate);
		len+=tmp;
		if (!only_calculate) output+=tmp;
	}
	if (Expression2)
	{
		tmp=((CExpression*)(Expression2))->XML_output(output,num_tabs,only_calculate);
		len+=tmp;
		if (!only_calculate) output+=tmp;
	}
	if (Expression3)
	{
		tmp=((CExpression*)(Expression3))->XML_output(output,num_tabs,only_calculate);
		len+=tmp;
		if (!only_calculate) output+=tmp;
	}

	if ((Expression1!=NULL) || (Expression2!=NULL) || (Expression3!=NULL))
	{
		if (!only_calculate) 
		{
			memset(output,9,num_tabs);output+=num_tabs;
			if ((XMLFileVersion==1) || (m_Type>=7))
			{
				strcpy(output,"</elm>\r\n");
				output+=8;
			}
			else
			{
				if (m_Type==1) strcpy(output,"</var>\r\n");
				if (m_Type==2) strcpy(output,"</opr>\r\n");
				if (m_Type==3) strcpy(output,"</pwr>\r\n");
				if (m_Type==4) strcpy(output,"</fra>\r\n");
				if (m_Type==5) strcpy(output,"</bra>\r\n");
				if (m_Type==6) strcpy(output,"</fun>\r\n");
				output+=8;
			}
		}
		len+=8+num_tabs;
		if (!only_calculate) {strcpy(output,tmpstr);output+=tmp;}
	}
	return len;
}

//loads element data from the XML string (used in load from disk and paste from clipboard)
//also used recursively - take care about stack memory usage
//should not be too slow (copy/paste)
#pragma optimize("s",on)
char * CElement::XML_input(char * file,void *element_struct)
{
	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;

	char hasE1=0;
	char hasE2=0;
	char hasE3=0;

	static char attribute[48];
	static char value[300];
	do
	{
		file=mf->XML_read_attribute(attribute,value,file,299);
		if (file==NULL) return NULL;
		if (strcmp(attribute,"color")==0) m_Color=atoi(value);
		if ((strcmp(attribute,"decor")==0) && (element_struct))
		{
			tElementStruct *ts=(tElementStruct*)element_struct;
			ts->Decoration=atoi(value);
		}
		if ((strcmp(attribute,"Exp1")==0) || (strcmp(attribute,"E1")==0)) hasE1=1;
		if ((strcmp(attribute,"Exp2")==0) || (strcmp(attribute,"E2")==0)) hasE2=1;
		if ((strcmp(attribute,"Exp3")==0) || (strcmp(attribute,"E3")==0)) hasE3=1;

		if ((m_Type==1) ||    //variable
			(m_Type==6))      //function
		{
			if ((strcmp(attribute,"tx")==0) || (strcmp(attribute,"t")==0))
			{
				int i=0;
				int j=0;
				while (value[i])
				{
					char ch=value[i];
					Data1[j++]=ch; if (j>=23) break;
					i++;
				}
				Data1[j++]=0;
			}

			if (strcmp(attribute,"mods")==0)
			{
				char tmp[3];
				tmp[0]=value[0];
				tmp[1]=value[1];
				tmp[2]=0;
				int tt=0;sscanf(tmp,"%X",&tt);
				m_VMods=(unsigned char)tt;
			}

			if ((strcmp(attribute,"fnt")==0) || (strcmp(attribute,"f")==0))
			{
				int i=0;
				int j=0;
				while (value[i])
				{
					char tmp[3];
					tmp[0]=value[i];
					tmp[1]=value[i+1];
					tmp[2]=0;
					int tt=0;sscanf(tmp,"%X",&tt);
					Data2[j]=(char)tt; if (j>=23) break;
					if (j==0) memset(Data2+1,*Data2,24-1);
					if ((XMLFileVersion==1) && (j==0)) m_VMods=tt&0x1C;
					Data2[j]&=0xE3;
					j++;
					i+=2;
				}
			}
			if (strcmp(attribute,"float")==0)
			{
				for (int ij=0;ij<8;ij++)
				{
					char bfff[3];
					int fvalue;
					bfff[0]=value[ij*2];
					bfff[1]=value[ij*2+1];
					bfff[2]=0;
					sscanf(bfff,"%X",&fvalue);
					Data1[16+ij]=(char)fvalue;
				}
				Data1[15]=126;if (value[16]=='_') Data1[15]|=0x80;
				Data1[14]=0;
			}

			if (strcmp(attribute,"mth")==0)
			{
				if (value[0]=='1')
				{
					if (((CExpression*)m_pPaternalExpression)->m_StartAsText==1)
						m_Text=0;
				}
				else
				{
					if (((CExpression*)m_pPaternalExpression)->m_StartAsText==0)
						m_Text=1;
				}
			}
			if (strcmp(attribute,"ttxt")==0)
			{
				if (value[0]=='2') m_Text=2;
				if (value[0]=='3') m_Text=3;
				if (value[0]=='4') m_Text=4;
			}
		}
		if (m_Type==2) //operator
		{
			if ((strcmp(attribute,"stp")==0) || (strcmp(attribute,"s")==0))
			{
				Data1[0]=value[0];
				if (Data1[0]==9) Data1[3]=0;
			}
			if ((strcmp(attribute,"tablen")==0) && (Data1[0]==9))
				Data1[3]=atoi(value);
		}

		if (m_Type==4) //fraction (rational number)
		{
			if (strcmp(attribute,"stp")==0)
			{
				if (toupper(value[0])=='S') Data1[0]='/';   //semi-fraction
				else if (toupper(value[0])=='A') Data1[0]=' '; //a-over-b
				else if (toupper(value[0])=='D') Data1[0]='d'; // d/d()
				else Data1[0]=0;
			}
		}
		if (m_Type==7) //sigma, pi, integral
		{
			if (strcmp(attribute,"stp")==0)
			{
				Data1[0]=toupper(value[0]);
				if (toupper(value[0])=='C') Data1[0]='O';
				Data2[2]=1; //default value
			}
			if ((strcmp(attribute,"symbol_height")==0) || (strcmp(attribute,"sze")==0))
			{
				Data2[0]=atoi(value);
			}
			if (strcmp(attribute,"dimension")==0)
			{
				Data2[2]=atoi(value);
			}
			if (strcmp(attribute,"limits_aside")==0)
			{
				Data2[1]=atoi(value);
			}
		}
		if (m_Type==9)
		{
			if (strcmp(attribute,"label")==0)
			{
				Data1[0]='L';
			}
			if (strcmp(attribute,"URL")==0)
			{
				Data1[0]='H';
				*(char**)Data3=NULL;
				if (value[0])
				{
					*(char**)Data3=(char*)malloc(300);
					strcpy(*(char**)Data3,value);
				}
			}
		}
		if (m_Type==10) //condition list as element
		{
			if (strcmp(attribute,"left_bar")==0) Data1[0]|=atoi(value);
			if (strcmp(attribute,"right_bar")==0) Data1[0]|=atoi(value)<<1;
			if (strcmp(attribute,"align")==0) Data2[0]=atoi(value);
		}

	} while (attribute[0]);

	if (hasE1) //Expression1
	{
		Expression1=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,100);
		file=((CExpression*)(Expression1))->XML_input(file);
		if (file==NULL)
		{
			delete ((CExpression*)(Expression1));
			return NULL;
		}
	}
	if (hasE2) //Expression2
	{
		Expression2=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,100);
		file=((CExpression*)(Expression2))->XML_input(file);
		if (file==NULL)
		{
			delete ((CExpression*)(Expression2));
			return NULL;
		}
	}
	if (hasE3) //Expression3
	{
		Expression3=(CObject*)new CExpression(this,(CExpression*)m_pPaternalExpression,100);
		file=((CExpression*)(Expression3))->XML_input(file);
		if (file==NULL)
		{
			delete ((CExpression*)(Expression3));
			return NULL;
		}
	}

	return file;
}
#pragma optimize("",on)

//calculates an "unique" checksum of this element
extern int CalcStructuralChecksumOnly;
int CElement::CalcChecksum()
{
	int c=m_Type*4096;
	if (!CalcStructuralChecksumOnly) c+=m_Color*16;
	c+=Data1[0];
	if (m_Type==2) return c; //we suppose that operators 'm_type==2' don't have subexpressions
	c+=Data2[0]*8;
	c+=m_VMods*16;

	if (Expression1) {c+=2*((CExpression*)Expression1)->CalcChecksum();}
	if (Expression2) {c+=8;c+=4*((CExpression*)Expression2)->CalcChecksum();}
	if (Expression3) {c+=64;c+=8*((CExpression*)Expression3)->CalcChecksum();}

	if (m_Type==1)
	{
		if (Data1[1]==0) return c;
		int i=1;
		while (Data1[i])
		{
			c+=(Data1[i]+Data2[i])^i;
			i++;if (i>23) return c;
		}
		return c;
	}
	else if (m_Type==6)
	{
		int i=1;
		while (Data1[i])
		{
			c+=Data1[i]^i;
			i++;if (i>23) return c;
		}
		return c;
	}
	else if (m_Type==7) 
	{
		c+=Data2[1]*16+Data2[2]*32;
		return c;
	}
	else if ((m_Type==11) || (m_Type==12))
	{
		c+=4*Data1[1]+8*Data1[2]+16*Data1[3]+Data1[10];
	}

	return c;
}


//two helper functions and macros for mathml output
#define OUTPUT(x) len+=MakeOutput(&output,tabs,only_calculate,x)
#pragma optimize("s",on)
int MakeOutput(char **output,char *tabs,char only_calculate,char *text1)
{
	char tmpstr[136];
	strcpy(tmpstr,tabs);
	strcat(tmpstr,text1);
	int tt=(int)strlen(tmpstr);
	if (!only_calculate) {strcpy(*output,tmpstr);(*output)+=tt;}
	return tt;
}
#define OUTPUT_EXPRESSION(x) len+=MakeExpressionOutput(&output,&tabs,num_tabs,only_calculate,output_type,x)
#pragma optimize("s",on)
int MakeExpressionOutput(char **output,char **tabs,int num_tabs,char only_calculate, char output_type,CObject *expression)
{
	if (expression==NULL) return 0;
	if (((CExpression*)expression)->m_pElementList->Type==0) return 0;
	int tt;
	if (output_type==3)
		tt=((CExpression*)(expression))->LaTeX_output(*output,only_calculate);
	else
	{
		//tt=((CExpression*)(expression))->MathML_output(*output,num_tabs+1,only_calculate,output_type);
		//memset(*tabs,9,num_tabs);*(*tabs+num_tabs)=0; //generating the tablist string
	}
	if (!only_calculate) (*output)+=tt;
	return tt;
}

/*
#pragma optimize("s",on)
int CElement::MathML_output(char * output, int num_tabs, char only_calculate, char output_type)
{
	int len=0;
	int tmp;
	short i;

	static char tmpstr[136]; //we are using this functin recursivley, so take care not to use too much memory
	static char tabs2[17];
	char *tabs=&tabs2[0];
	if (num_tabs>16) num_tabs=16;

	memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string


	if ((m_Type==1) || //variable
		(m_Type==6))   //function
	{
		//handling variables and functions
		//this is a bit complicated - variables and functions can have indexes and exponents
		char is_pure_number=0;
		char is_d=0;
		CExpression *is_exp=NULL;

		//preparation - determine the way it will be coded (if msub or munder must be used)
		if ((m_Type==1) && (this->Expression1==NULL))
		{
			double N;
			int prec;
			int k=0;
			for (k=0;k<((CExpression*)(m_pPaternalExpression))->m_NumElements;k++)
				if ((((CExpression*)(m_pPaternalExpression))->m_pElementList+k)->pElementObject==this) break;
			if (((CExpression*)(m_pPaternalExpression))->IsPureNumber(k,1,&N,&prec))
				is_pure_number=1;  //mn will be used
		}
		if (((m_Type==1) && (this->Expression1)) || ((m_Type==6) && (this->Expression2)))
			is_pure_number=2;  //msub will be used
		if ((m_Type==6) && (this->Expression1) && (this->Expression2) &&
			(strcmp(this->Data1,"lim")==0))
			is_pure_number=3; //munder will be used

		//check if this is 'd' function - the differential
		if ((m_Type==6) && (this->Expression1) && (strcmp(Data1,"d")==0) && ((Data2[0]&0xE0)!=60))
			is_d=1;


		//check if this function is powered
		if ((m_Type==6) && (this->m_pPaternalExpression) && (((CExpression*)(m_pPaternalExpression))->m_NumElements==1) &&
			(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement) && 
			(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement->m_Type==3) &&
			(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement->Expression1==m_pPaternalExpression))
		{
			is_exp=(CExpression*)(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement->Expression2);
		}

		//check if this variable is powered (and has index at the same time)
		if ((m_Type==1) && (Expression1) &&
			(this->m_pPaternalExpression) && (((CExpression*)(m_pPaternalExpression))->m_NumElements==1) &&
			(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement) && 
			(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement->m_Type==3) &&
			(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement->Expression1==m_pPaternalExpression))
		{
			is_exp=(CExpression*)(((CExpression*)(m_pPaternalExpression))->m_pPaternalElement->Expression2);
		}
		
		if (is_pure_number==2)
		{
			if (is_exp)	OUTPUT("<msubsup>\r\n"); else OUTPUT("<msub>\r\n");
		}
		else if (is_pure_number==3)
		{
			if (is_exp) OUTPUT("<msup>\r\n");
			OUTPUT("<munder>\r\n");
		}
		else if (is_exp)
		{
			OUTPUT("<msup>\r\n");
		}

		if (Data2[0]&0x1C) //some decorations exist above variable (dash, arrow, kappa)
		{
			OUTPUT("<mover>\r\n");
		}


		strcpy(tmpstr,(is_pure_number==1)?"<mn":(is_d)?"<mo":"<mi");
		if ((Data2[0]&0x3)==2) strcat(tmpstr," mathvariant=\"italic\">");
		else if ((Data2[0]&0x3)==1) strcat(tmpstr," mathvariant=\"bold\">");
		else if ((Data2[0]&0x3)==3) strcat(tmpstr," mathvariant=\"bold-italic\">");
		else if ((Data2[0]&0xE0)==0x40) strcat(tmpstr," mathvariant=\"monospace\">");
		else strcat(tmpstr,">");

		if (is_d)
			strcat(tmpstr,"&dd;");
		else
		{
			char ppp[10];
			tmp=(short)strlen(Data1);
			for (i=0;i<tmp;i++)
			{
				if ((Data1[i]<' ') || (Data1[i]>0x7E) || (Data1[i]=='\\'))
				{					
					sprintf(ppp,"&#x%02X;",(unsigned char)Data1[i]);
					strcat(tmpstr,ppp);
				}
				else if (((Data2[i]&0xE0)==0x60) && (Data1[i]>='A') && (Data1[i]<='z')) //for greek letters
				{
					//handling for greek letters
					//                        A    B     C     D     E     F     G     H     I     J     K     L     M     N     O     P     Q     R     S     T     U     V     W     X     Y     Z
					static int unicodes[32]={0x391,0x392,0x3A7,0x394,0x395,0x3A6,0x393,0x397,0x399,0x04A,0x39A,0x39B,0x39C,0x39D,0x39F,0x3A0,0x398,0x3A1,0x3A3,0x3A4,0x3A5,0x056,0x3A9,0x39E,0x3A8,0x396};
					int pos=(int)toupper(Data1[i])-'A';
					sprintf(ppp,"&#x%04X;",unicodes[pos]+((Data1[i]>='a')?32:0));
					strcat(tmpstr,ppp);
				}
				else
				{
					sprintf(ppp,"%c",(char)Data1[i]);
					strcat(tmpstr,ppp);
				}
			}
		}
		strcat(tmpstr,(is_pure_number==1)?"</mn>\r\n":(is_d)?"</mo>\r\n":"</mi>\r\n");
		OUTPUT(tmpstr);

		if (Data2[0]&0x1C)
		{
			//handling for over-decoration (arrow, dash,kappa)
			if ((Data2[0]&0x1C)==0x04) OUTPUT("<mo>&HorizontalLine;</mo>\r\n");
			if ((Data2[0]&0x1C)==0x08) OUTPUT("<mo>&RightArrow;</mo>\r\n");
			if ((Data2[0]&0x1C)==0x0C) OUTPUT("<mo>^</mo>\r\n");
			if ((Data2[0]&0x1C)==0x14) OUTPUT("<mo>&CenterDot;</mo>\r\n"); //da li je to OK? (prikaz varijabli s toèkom)
			OUTPUT("</mover>\r\n");
		}

		if ((is_pure_number==2) || (is_pure_number==3))
		{
			//index handling
			if (m_Type==1) OUTPUT_EXPRESSION(Expression1);
			if (m_Type==6) OUTPUT_EXPRESSION(Expression2);

			if (is_exp)
			{
				if (is_pure_number==3) 
				{
					OUTPUT("</munder>\r\n");
					OUTPUT_EXPRESSION((CObject*)is_exp);
					OUTPUT("</msup>");
				}
				else
				{
					OUTPUT_EXPRESSION((CObject*)is_exp);
					OUTPUT("</msubsup>\r\n");
				}
			}
			else
				OUTPUT((is_pure_number==2)?"</msub>\r\n":"</munder>\r\n");
		}
		else if (is_exp)
		{
			//handling exponent
			OUTPUT_EXPRESSION((CObject*)is_exp);
			OUTPUT("</msup>\r\n");
		}

		if (m_Type==6)
		{
			//function argument
			OUTPUT("<mo>&af;</mo>\r\n");
			OUTPUT_EXPRESSION(Expression1);
		}
	}

	if (m_Type==2)  //operator
	{
		char *str="";
		char fb=Data1[0];
		if (fb==(char)0xD7) str="&CenterDot;";
		if (fb==(char)0xB1) str="&PlusMinus;";
		if (fb==(char)0xB2) str="&MinusPlus;";
		if (fb=='m') str="mod";
		if (fb=='n') str="not";
		if (fb=='a') str="and";
		if (fb=='o') str="or";
		if (fb=='O') str="nor";
		if (fb=='x') str="xor";
		if (fb=='A') str="nand";
		if (fb=='P') str="&DoubleVerticalBar;";
		if (fb=='p') str="&RightFloor;";
		if (fb=='&') str="&#x26;";
		if (fb=='<') str="&lt;";		
		if (fb==(char)0xB9) str="&NotEqual;";
		if (fb==(char)0x01) str="&LessLess;";
		if (fb==(char)0x02) str="&GreaterGreater;";
		if (fb==(char)0xBB) str="&TildeTilde;";
		if (fb==(char)0xA3) str="&le;";
		if (fb==(char)0xB3) str="&ge;";
		if (fb==(char)0x40) str="&EqualTilde;";
		if (fb==(char)0x7E) str="&Tilde;";
		if (fb==(char)0xA0) str="&DotEqual;";
		if (fb==(char)0xB8) str="./.";  //what is this (dash with dots above and below)
		if (fb==(char)0xB5) str="&Proportional;";
		if (fb==(char)0xBA) str="&Congruent;";  //triple equal

		if (fb==(char)0xC5) str="&CirclePlus;"; 
		if (fb==(char)0xC4) str="&CircleTimes;"; 
		if (fb==(char)0x24) str="&Exists;";  
		if (fb==(char)0x22) str="&ForAll;";  
		if (fb==(char)0x27) str="&ReverseElement;"; 
		if (fb==(char)0xCE) str="&Element;"; 
		if (fb==(char)0xCF) str="&NotElement;"; 
		if (fb==(char)0xC7) str="&Intersection;"; 
		if (fb==(char)0xC8) str="&Union;";  
		if (fb==(char)0xC9) str="&Superset;"; 
		if (fb==(char)0xCA) str="&SupersetEqual;"; 
		if (fb==(char)0xCC) str="&Subset;";  
		if (fb==(char)0xCD) str="&SubsetEqual;";  
		if (fb==(char)0xCB) str="&NotSubset;";  

		if (fb==(char)0xB4) str="&Cross;";  
		if (fb==(char)0x44) str="&#x0394;"; //delta
		if (fb==(char)0xD1) str="div";  //nabla operator
		if (fb=='r') str="rot";   //rotor
		if (fb==(char)0xA1) str="Y"; //pisani Y
		if (fb==(char)0xA6) str="f"; //pisani f
		if (fb==(char)0xB7) str="&Diamond;"; //big dot
		if (fb==(char)0x03) str="&SmallCircle;"; //function composition???
		if (fb==(char)0xDE) str="&DoubleRightArrow;";  
		if (fb==(char)0xDC) str="&DoubleLeftArrow;";  
		if (fb==(char)0xDD) str="&DoubleUpArrow;";  
		if (fb==(char)0xDF) str="&DoubleDownArrow;";  
		if (fb==(char)0xDB) str="&DoubleLeftRightArrow;";  
		if (fb==(char)0xAE) str="&RightArrow;";  
		if (fb==(char)0xAC) str="&LeftArrow;";  
		if (fb==(char)0xAD) str="&UpArrow;";  
		if (fb==(char)0xAF) str="&DownArrow;";  
		if (fb==(char)0xAB) str="&LeftRightArrow;";  

		strcpy(tmpstr,"<mo>");
		if (*str==0) 
			{char ppp[2];ppp[0]=fb;ppp[1]=0;strcat(tmpstr,ppp);}
		else
			strcat(tmpstr,str);
		strcat(tmpstr,"</mo>\r\n");
		OUTPUT(tmpstr);
	}

	if (m_Type==3) //exponent (power) or
	{
		if ((Expression1) && (((CExpression*)Expression1)->m_NumElements==1) && 
			(((CExpression*)Expression1)->m_pElementList->Type==6))
		{
			//will not print exponent (this will be done when printing the function)
			OUTPUT_EXPRESSION(Expression1);
		}
		else if ((Expression1) && (((CExpression*)Expression1)->m_NumElements==1) && 
			(((CExpression*)Expression1)->m_pElementList->Type==1) && 
			(((CExpression*)Expression1)->m_pElementList->pElementObject->Expression1))
		{
			//will not print the exponent (this will be done when printing variable)
			OUTPUT_EXPRESSION(Expression1);
		}
		else
		{
			OUTPUT("<msup>\r\n");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("</msup>\r\n");
		}
	}

	if (m_Type==4) //fraction
	{
		if (Data1[0]==' ') //a over b
		{
			OUTPUT("<mfenced open=\"(\" close=\")\">\r\n");
			OUTPUT("<mtable><mtr><mtd>\r\n");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT("</mtd></mtr><mtr><mtd>\r\n");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("</mtd></mtr></mtable>\r\n");
			OUTPUT("</mfenced>\r\n");
		}
		else
		{
			OUTPUT("<mfrac>\r\n");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("</mfrac>\r\n");
		}
	}

	if (m_Type==5) //parentheses
	{
		if (Expression2)
		{
			OUTPUT("<msub>\r\n");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("</msub>\r\n");
		}
		else
		{
			OUTPUT_EXPRESSION(Expression1);	
		}
	}

	if (m_Type==7) //sigma, pi, integral
	{
		char *str;
		if (Data1[0]=='/')
		{
			OUTPUT_EXPRESSION(Expression1);
		}
		if ((Expression2) && (((CExpression*)Expression2)->m_pElementList->Type) && 
			(Expression3) && (((CExpression*)Expression3)->m_pElementList->Type))
		{
			OUTPUT((Data2[1])?"<msubsup>\r\n":"<munderover>\r\n");
			str=(Data2[1])?"</msubsup>\r\n":"</munderover>\r\n";
		}
		else if ((Expression3) && (((CExpression*)Expression3)->m_pElementList->Type)) 
		{
			OUTPUT((Data2[1])?"<msub>\r\n":"<munder>\r\n");
			str=(Data2[1])?"</msub>\r\n":"</munder>\r\n";
		}
		else if ((Expression2) && (((CExpression*)Expression2)->m_pElementList->Type)) 
		{
			OUTPUT((Data2[1])?"<msup>\r\n":"<mover>\r\n");
			str=(Data2[1])?"</msup>\r\n":"</mover>\r\n";
		}
		else str="";

		if (Data1[0]=='S') OUTPUT("<mo>&Sum;</mo>\r\n");
		if (Data1[0]=='P') OUTPUT("<mo>&Product;</mo>\r\n");
		if (Data1[0]=='I') OUTPUT((Data2[2]==3)?"<mo>&Integral;&Integral;&Integral;</mo>\r\n":(Data2[2]==2)?"<mo>&Integral;&Integral;</mo>\r\n":"<mo>&Integral;</mo>\r\n");
		if (Data1[0]=='O') OUTPUT((Data2[2]==2)?"<mo>&DoubleContourIntegral;</mo>\r\n":"<mo>&ContourIntegral;</mo>\r\n");
		if (Data1[0]=='|') OUTPUT("<mo>&VerticalLine;</mo>\r\n");

		OUTPUT_EXPRESSION(Expression3);
		OUTPUT_EXPRESSION(Expression2);	
		if (*str) OUTPUT(str);
		if (Data1[0]!='/') OUTPUT_EXPRESSION(Expression1);	
	}

	if (m_Type==8) //root
	{
		OUTPUT((Expression2)?"<mroot>\r\n":"<msqrt>\r\n");
		OUTPUT_EXPRESSION(Expression1);
		OUTPUT_EXPRESSION(Expression2);
		OUTPUT((Expression2)?"</mroot>\r\n":"</msqrt>\r\n");
	}


	if (m_Type==9) //condition list
	{
		OUTPUT_EXPRESSION(Expression1);

		if ((Expression2) || (Expression3))
		{
			OUTPUT("<mfenced open=\"|\" close=\" \">\r\n");
			OUTPUT("<mstyle mathsize=\"small\"><mtable><mtr><mtd>\r\n");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("</mtd></mtr><mtr><mtd>\r\n");
			OUTPUT_EXPRESSION(Expression3);
			OUTPUT("</mtd></mtr></mtable></mstyle>\r\n");
			OUTPUT("</mfenced>\r\n");
		}
	}

	if (m_Type==10)
	{
		if (((Expression1) && (((CExpression*)(Expression1))->m_pElementList->Type)) || 
			((Expression2) && (((CExpression*)(Expression2))->m_pElementList->Type)) || 
			((Expression3) && (((CExpression*)(Expression3))->m_pElementList->Type)))
		{
			strcpy(tmpstr,"<mfenced open=\"");
			if (Data1[0]&0x01) strcat(tmpstr,"|\" close=\""); else strcat(tmpstr," \" close=\"");
			if (Data1[0]&0x02) strcat(tmpstr,"|\">\r\n"); else strcat(tmpstr," \">\r\n");
			OUTPUT(tmpstr);

			strcpy(tmpstr,"<mstyle mathsize=\"small\">");
			if (Data2[0]==0) strcat(tmpstr,"<mtable columnalign=\"left\"><mtr><mtd>\r\n");
			else if (Data2[0]==2) strcat(tmpstr,"<mtable columnalign=\"right\"><mtr><mtd>\r\n");
			else strcat(tmpstr,"<mtable><mtr><mtd>\r\n");
			OUTPUT(tmpstr);

			OUTPUT_EXPRESSION(Expression1);
			OUTPUT("</mtd></mtr><mtr><mtd>\r\n");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("</mtd></mtr><mtr><mtd>\r\n");
			OUTPUT_EXPRESSION(Expression3);
			OUTPUT("</mtd></mtr></mtable></mstyle>\r\n");
			OUTPUT("</mfenced>\r\n");
		}
	}

	return len;
}
*/


#pragma optimize("s",on)
int CElement::LaTeX_output(char * output,  char only_calculate)
{
	char output_type=3;
	int num_tabs=0;
	int len=0;
	char *tabs="";

	static char tmpstr[136]; //we are using this functin recursivley, so take care not to use too much memory
	/*static char tabs2[17];
	char *tabs=&tabs2[0];
	if (num_tabs>16) num_tabs=16;

	memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string
	*/

	if ((m_Type==1) || (m_Type==6)) //variable or function
	{
		int is_squared_function=0;
		if (m_Type==6)
		{
			CExpression *p=(CExpression*)this->m_pPaternalExpression;
			if ((p->m_pPaternalElement) && (p->m_pPaternalElement->m_Type==3) &&
				((CObject*)p==p->m_pPaternalElement->Expression1) && (p->m_NumElements==1))
				is_squared_function=1;

		}
		if (m_VMods) //decorations (hat, dash, arrow, dot...)
		{
			//handling for over-decoration (arrow, dash,kappa)
			if (m_VMods==0x04) OUTPUT("\\bar{");
			if (m_VMods==0x08) OUTPUT("\\vec{");
			if (m_VMods==0x0C) OUTPUT("\\hat{");
			if (m_VMods==0x14) OUTPUT("\\dot{");
			if (m_VMods==0x18) OUTPUT("\\ddot{");
			if (m_VMods==0x1C) OUTPUT("\\check{");
			if (m_VMods==0x20) OUTPUT("\\dddot{");
			if (m_VMods==0x24) OUTPUT("\\tilde{");
		}
		char fnt=Data2[0]&0xE0;

		if ((Data2[0]!=(char)0xE3) && (fnt!=0x60))
			if (((Data2[0]&0x03)==1) || ((Data2[0]&0x03)==3)) //bold or bold-italic
				OUTPUT("\\mathbf{");

		int function_def=0;
		
		if (fnt==0x40) //monospaced font - as script
			OUTPUT("\\mathtt{");
		else if (m_Type==6)
		{
			function_def=1;
			if (strcmp(Data1,"sin")==0) OUTPUT("\\sin ");
			else if (strcmp(Data1,"cos")==0) OUTPUT("\\cos ");
			else if (strcmp(Data1,"arccos")==0) OUTPUT("\\arccos ");
			else if (strcmp(Data1,"arcsin")==0) OUTPUT("\\arcsin ");
			else if (strcmp(Data1,"cot")==0) OUTPUT("\\cot ");
			else if (strcmp(Data1,"arctan")==0) OUTPUT("\\arctan ");
			else if (strcmp(Data1,"arg")==0) OUTPUT("\\arg ");
			else if (strcmp(Data1,"exp")==0) OUTPUT("\\exp ");
			else if (strcmp(Data1,"sec")==0) OUTPUT("\\sec ");
			else if (strcmp(Data1,"csc")==0) OUTPUT("\\csc ");
			else if (strcmp(Data1,"lim")==0) OUTPUT("\\lim ");
			else if (strcmp(Data1,"ln")==0) OUTPUT("\\ln ");
			else if (strcmp(Data1,"log")==0) OUTPUT("\\log ");
			else if (strcmp(Data1,"tan")==0) OUTPUT("\\tan ");
			else if (strcmp(Data1,"log")==0) OUTPUT("\\log ");
			else if (strcmp(Data1,"min")==0) OUTPUT("\\min ");
			else if (strcmp(Data1,"max")==0) OUTPUT("\\max ");
			else if ((Data1[0]==(char)0xB6) && (fnt==0x60)) OUTPUT("\\partial ");
			else 
			{
				if (fnt==0x60)
					OUTPUT("\\, ");
				else
					OUTPUT("\\, \\mathrm{");
				function_def=0;
			}
		}

		if (function_def==0)
		{
			if (fnt==(char)0xE0)
			{
				if (Data1[0]=='H') OUTPUT("\\cdots ");
				if (Data1[0]=='V') OUTPUT("\\vdots ");
				if (Data1[0]=='A') OUTPUT("\\ddots ");
				if (Data1[0]=='U') OUTPUT("\\ddots ");
			}
			else if (fnt==0x60)
			{
				int ii=0;
				while (Data1[ii])
				{
					char *str=";";
					char c=Data1[ii];
					if (c=='\'') {OUTPUT("'");ii++;continue;}
					if (c=='*') {OUTPUT("^{*}");ii++;continue;}
					if (((c>='0') && (c<='9')) || (c=='.')) {char bf[8];bf[0]=c;bf[1]=0;OUTPUT(bf);ii++;continue;}
					char lower=1;
					if ((c&0x80)==0) if ((c<='Z') && (c>='A')) {c+=32;lower=0;}
					if (c=='a') str="alpha";
					if (c=='b') str="beta";
					if (c=='g') str="gamma";
					if (c=='d') str="delta";
					if (c=='e') str="epsilon";
					if (c=='z') str="zeta";
					if (c=='h') str="eta";
					if (c=='q') str="theta";
					if (c=='i') str="iota";
					if (c=='k') str="kappa";
					if (c=='l') str="lambda";
					if (c=='m') str="mu";
					if (c=='n') str="nu";
					if (c=='x') str="xi";
					if (c=='p') str="pi";
					if (c=='r') str="rho";
					if (c=='s') str="sigma";
					if (c=='t') str="tau";
					if (c=='u') str="upsilon";
					if (c=='f') str="phi";
					if (c=='c') str="chi";
					if (c=='y') str="psi";
					if (c=='w') str="omega";
					if (c==(char)0xA5) str="infty";
					if (c==(char)0xC0) str="aleph";
					if (c==(char)0xC1) str="Im";
					if (c==(char)0xC2) str="Re";
					if (c==(char)0xC3) str="wp";
					if (c==(char)0xC6) str="emptyset";
					if (c==(char)0xA1) str="mathcal{Y}";
					if (c==(char)0xA6) str="mathcal{F}";
					if (c==(char)0xD0) str="angle";
					if (c==(char)0xB0) str="mathcal{Y}";
					if (c==(char)0xA1) str=";";  //degeree ?????
					char bff[16];
					strcpy(bff,"\\");
					strcat(bff,str);
					strcat(bff," ");
					if (!lower) bff[1]-=32;
					OUTPUT(bff);
					ii++;
				}
			}
			else 
			{
				if (Data1[strlen(Data1)-1]=='*')
				{
					char bff[32];
					strcpy(bff,Data1);
					bff[strlen(bff)-1]=0;
					OUTPUT(bff);
					OUTPUT("^{*}");
				}
				else
					OUTPUT(Data1);
				OUTPUT(" ");
			}

			if (fnt==0x40) OUTPUT("}");
			else if (m_Type==6)
			{
				if (fnt!=0x60) OUTPUT("} ");
				if (this->Expression1)
				if ((this->Expression2==NULL) && 
					(((CExpression*)this->Expression1)->m_pElementList->Type==1) &&
					(((CExpression*)this->Expression1)->m_DrawParentheses==0) &&
					(!is_squared_function)) 
					OUTPUT("\\, ");
			}
		}
		
		if ((Data2[0]!=(char)0xE3) && (fnt!=0x60))
			if (((Data2[0]&0x03)==1) || ((Data2[0]&0x03)==3)) OUTPUT("}");

		if (m_VMods) OUTPUT("}"); //close if there was some decorations

		if (((Expression1)&& (m_Type==1)) || ((Expression2) && (m_Type==6)))
		{
			OUTPUT("_{");
			OUTPUT_EXPRESSION((m_Type==1)?Expression1:Expression2);
			OUTPUT("}");
		}

		if (m_Type==6)
		{
			if (is_squared_function)
			{
				//this is form of: func^2(x)
				OUTPUT("^{");
				OUTPUT_EXPRESSION(((CExpression*)(this->m_pPaternalExpression))->m_pPaternalElement->Expression2);
				OUTPUT("} ");

			}
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT(" \\, ");
		}
	}

	if (m_Type==2)  //operator
	{
		char *str="";
		char fb=Data1[0];
		if (fb==(char)0xD7) str="\\cdot";
		if (fb==(char)0xB1) str="\\pm";
		if (fb==(char)0xB2) str="\\mp";
		if (fb=='m') str="\\bmod";
		if (fb=='n') str="\\mathrm{not}";
		if (fb=='a') str="\\mathrm{and}";
		if (fb=='o') str="\\mathrm{or}";
		if (fb=='O') str="\\mathrm{nor}";
		if (fb=='x') str="\\mathrm{xor}";
		if (fb=='A') str="\\mathrm{nand}";
		if (fb=='P') str="\\parallel";
		if (fb=='p') str="\\perp";
		if (fb=='&') str="&";
		if (fb=='<') str="<";		
		if (fb==(char)0xB9) str="\\neq";
		if (fb==(char)0x01) str="\\ll";
		if (fb==(char)0x02) str="\\gg";
		if (fb==(char)0xBB) str="\\approx";
		if (fb==(char)0xA3) str="\\leq";
		if (fb==(char)0xB3) str="\\geq;";
		if (fb==(char)0x40) str="\\cong";
		if (fb==(char)0x7E) str="\\sim";
		if (fb==(char)0xA0) str="\\doteq";
		if (fb==(char)0xB8) str="\\div";  
		if (fb==(char)0xB5) str="\\propto";
		if (fb==(char)0xBA) str="\\equiv";  //triple equal
		if (fb==(char)0xBC) str="\\dots";  //...

		if (fb==(char)0xC5) str="\\oplus"; 
		if (fb==(char)0xC4) str="\\otimes"; 
		if (fb==(char)0x24) str="\\exists";  
		if (fb==(char)0x22) str="\\forall";  
		if (fb==(char)0x27) str="\\ni"; 
		if (fb==(char)0xCE) str="\\in"; 
		if (fb==(char)0xCF) str="\\notin"; 
		if (fb==(char)0xC7) str="\\cap"; 
		if (fb==(char)0xC8) str="\\cup";  
		if (fb==(char)0xC9) str="\\supset"; 
		if (fb==(char)0xCA) str="\\supseteq"; 
		if (fb==(char)0xCC) str="\\subset";  
		if (fb==(char)0xCD) str="\\subseteq";  
		if (fb==(char)0xCB) str="\\subset\\mid";  //not-subset?????

		if (fb==(char)0xB4) str="\\times";  
		if (fb==(char)0x44) str="\\Delta"; //delta
		if (fb==(char)0xD1) str="\\nabla";  //nabla operator
		if (fb=='r') str="\\mathrm{rot}";   //rotor
		if (fb=='*') str="\\ast";   //asterisk
		if (fb==(char)0xA1) str="\\mathcal{Y}"; //pisani Y
		if (fb==(char)0xA6) str="\\mathcal{f}"; //pisani f
		if (fb==(char)0xB7) str="\\bullet"; //big dot
		if (fb==(char)0x03) str="\\circ"; //function composition???
		if (fb==(char)0xDE) str="\\Rightarrow";  
		if (fb==(char)0xDC) str="\\Leftarrow";  
		if (fb==(char)0xDD) str="\\Uparrow";  
		if (fb==(char)0xDF) str="\\Downarrow";  
		if (fb==(char)0xDB) str="\\Leftrightarrow";  
		if (fb==(char)0xAE) str="\\rightarrow";  
		if (fb==(char)0xAC) str="\\leftarrow";  
		if (fb==(char)0xAD) str="\\uparrow";  
		if (fb==(char)0xAF) str="\\downarrow";  
		if (fb==(char)0xAB) str="\\leftrightarrow";  
		if (fb==(char)0xE2) str="\\mapsto";  

		if (str[0]==0) {char bff[5];bff[0]=Data1[0]; bff[1]=0;OUTPUT(bff);}
		else
		{

		OUTPUT(str);
		OUTPUT(" ");
		}
	}

	if (m_Type==3) //exponent (power) or
	{
		CExpression *p=(CExpression*)this->Expression1;
		if ((p->m_pElementList->Type==6) && (p->m_NumElements==1))
		{
			//this is form of: func^2(x)
			OUTPUT_EXPRESSION(Expression1);
		}
		else
		{
			OUTPUT("{");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT("}^{");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("}");
		}
	}

	if (m_Type==4) //fraction
	{
		if (Data1[0]==' ') OUTPUT("\\binom{");
		else OUTPUT("\\frac{");
		OUTPUT_EXPRESSION(Expression1);
		OUTPUT("}{");
		OUTPUT_EXPRESSION(Expression2);
		OUTPUT("}");
	}

	if (m_Type==5) //parentheses
	{
		OUTPUT("{");
		OUTPUT_EXPRESSION(Expression1);	
		OUTPUT("}");
		if (Expression2)
		{
			OUTPUT("_{");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("}");
		}
	}

	if (m_Type==7) //sigma, pi, integral
	{
		if (Data1[0]=='/')
		{
			//TODO provjeriti ovo - kako se radi right bar sa limitima
			OUTPUT("\\ ");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT("\\mid ");
		}
		if (Data1[0]=='S') OUTPUT("\\sum ");
		if (Data1[0]=='P') OUTPUT("\\prod ");
		if (Data1[0]=='I') OUTPUT((Data2[2]==3)?"\\iiint ":(Data2[2]==2)?"\\iint ":"\\int ");
		if (Data1[0]=='O') OUTPUT((Data2[2]==2)?"\\oiint ":"\\oint ");
		if (Data1[0]=='|') OUTPUT("\\mid "); //????? vertical line????
		if ((Expression3) && (((CExpression*)Expression3)->m_pElementList->Type))
		{
			OUTPUT("_{");
			OUTPUT_EXPRESSION(Expression3);
			OUTPUT("}");
		}
		if ((Expression2) && (((CExpression*)Expression2)->m_pElementList->Type))
		{
			OUTPUT("^{");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("}");
		}
		if (Data1[0]!='/')
		{
			OUTPUT_EXPRESSION(Expression1);
		}
		OUTPUT("\\; ");
	}

	if (m_Type==8) //root
	{
		CExpression *e=(CExpression*)Expression2;
		if ((e) && (e->m_NumElements) && (e->m_pElementList->Type))
		{
			OUTPUT("\\sqrt[");
			OUTPUT_EXPRESSION(Expression2);
			OUTPUT("]{");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT("}");
		}
		else
		{
			OUTPUT("\\sqrt{");
			OUTPUT_EXPRESSION(Expression1);
			OUTPUT("}");
		}
	}


	if (m_Type==9) //condition list
	{
		OUTPUT_EXPRESSION(Expression1);
		OUTPUT("\\left| ");
		OUTPUT("\\begin{array}{c}\r\n");
		OUTPUT_EXPRESSION(Expression2);
		OUTPUT("\\\\\r\n");
		OUTPUT_EXPRESSION(Expression3);
		OUTPUT("\\end{array} \\right. \r\n");
	}

	if (m_Type==10)
	{
		if (Data1[0]&0x01) OUTPUT("\\left| "); else OUTPUT("\\left. ");
		OUTPUT("\\begin{array}{c}\r\n");
		OUTPUT_EXPRESSION(Expression1);
		OUTPUT("\\\\\r\n");
		OUTPUT_EXPRESSION(Expression2);
		OUTPUT("\\\\\r\n");
		OUTPUT_EXPRESSION(Expression3);
		OUTPUT("\\end{array}\r\n");
		if (Data1[0]&0x02) OUTPUT("\\right| "); else OUTPUT("\\right. ");
	}

	return len;
}
#pragma optimize("",on)


//check if the given element is actually measurement unit (or its exponent)
int CElement::IsMeasurementUnit()
{
	if (m_Type==1) return (m_VMods==0x10)?1:0;
	if (m_Type==3)
	{
		CExpression *base=(CExpression*)Expression1;
		CExpression *exp=(CExpression*)Expression2;

		int only_units=1;
		tElementStruct *ts=base->m_pElementList;
		for (int i=0;i<base->m_NumElements;i++,ts++)
		{
			if ((ts->pElementObject) && (ts->pElementObject->IsMeasurementUnit())) continue;
			only_units=0;
			break;
		}
		if (only_units)
		{
			if (exp->m_NumElements>2) only_units=0;
			if (exp->m_NumElements==2)
			{
				if (exp->m_pElementList->Type!=2) only_units=0;
				else if ((exp->m_pElementList->pElementObject->Data1[0]!='+') && (exp->m_pElementList->pElementObject->Data1[0]!='-')) only_units=0;
				if ((exp->m_pElementList+1)->Type!=1) only_units=0;
				else if (((exp->m_pElementList+1)->pElementObject->Data1[0]<'0') || ((exp->m_pElementList+1)->pElementObject->Data1[0]>'9')) only_units=0;
			}
			if (exp->m_NumElements==1)
			{
				if ((exp->m_pElementList)->Type!=1) only_units=0;
				else if (((exp->m_pElementList)->pElementObject->Data1[0]<'0') || ((exp->m_pElementList)->pElementObject->Data1[0]>'9')) only_units=0;
			}
		}
		return only_units;
	}
	return 0;
}

int CElement::SetColor(int color)
{
	m_Color=color;
	
	if (m_Type<1) return 1;
	if (m_Type==11) return 1;
	if (m_Type==12) return 1;

	if (Expression1) ((CExpression*)Expression1)->SetColor(-1);
	if (Expression2) ((CExpression*)Expression2)->SetColor(-1);
	if (Expression3) ((CExpression*)Expression3)->SetColor(-1);
	return 1;

}