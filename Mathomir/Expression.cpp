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
#include "Mathomir.h"
#include "mainfrm.h"
#include "popupmenu.h"

#include "math.h"
#include "float.h"
#include "toolbox.h"
#include ".\expression.h"
#include "drawing.h"
#include "MathomirDoc.h"
#include "MathomirView.h"

extern CToolbox *Toolbox;
extern char XMLFileVersion; //defines XML file version when loading/saving files

extern char IsWindowOutOfFocus;
extern CElement *SelectedTab;
extern char QuickTypeUsed;
extern CMathomirView *pMainView;
extern int NumSelectedObjects;
extern int RepaintViewTimer;
extern int GuidlinesFirstPass;
bool CheckForGuidlines(int i,int AbsoluteX, int AbsoluteY, int show_range);
void PaintGuidlines(CDC *DC, int AbsoluteY);
void CheckForExtendedGuidlines(int AbsoluteY);
extern int EasycastListStart;
extern char TextControlboxMode;
unsigned char is_call_from_ploter;

extern int SelectedGuidelineType;
extern int GuidlineElement;
extern int GuidlineClearTimer;
extern int SizeNerbyList;
int IsCharacterHigh(char ch, char font);
int IsCharacterLow(char ch, char font);
int KeyboardIndexMode=0;
int KeyboardExponentMode=0;

CExpression *AutocompleteSource;
int AutocompletePoints;
int AutocompleteTriggered;
extern char TouchMouseMode;
extern char DisableMultitouch;
int CalcStructuralChecksumOnly; //for speed - as global variable (should be used with care in single thread)

const char *FunctionListString="arcsin,arccos,arctan,arccot,arctg,arcsec,arccsc,cosec,csc,sec,sin,cos,tan,log,ln,par,ctg,tg,cot,lim,arch,arsh,arth,arcoth,arsech,arcsch,sech,csch,sh,ch,th,coth,Re,Im,max,min,exp,arg,Res,sum,int,";

const struct LIST_OF_KNOWN_FUNCTIONS
{
	char type;
	char oper;
	char name[10];
} ListOfKnownFunctions[]={
	{6,23,"d"}, //differential
	{6,24,"par"}, //partial differential
	{6,24,"partial"}, //partial differential
	{4,'d',"dd"}, //d/d()
	{4,'p',"parpar"}, // parc/parc()
	{2,'a',"and"},
	{2,'o',"or"},
	{2,'m',"mod"},
	{2,'A',"nand"},
	{2,'O',"nor"},
	{2,'x',"xor"},
	{2,'n',"not"},
	//{6,0,"lim"},
	{4,0,"frac"},
	{8,1,"sqrt"},
	{8,0,"root"},
	{4,' ',"binom"},
	{2,(char)0xAE,"to"},
	{7,'I',"int"},
	{7,'I'-3,"iint"},
	{7,'I'-4,"iiint"},
	{7,'O',"oint"},
	{7,'O'-3,"oiint"},
	{7,'O'-4,"oiiint"},
	{7,'S',"sum"},

	{2,(char)0xB1,"pm"},
	{2,(char)0xB2,"mp"},
	{10,5,"case"},

	{2,9,"space"},
	{2,'*',"ast"},
	{7,'P',"prod"},
	{2,(char)0xB9,"neq"},
	{2,(char)0xD1,"nabla"},
	{2,(char)0xB4,"times"},
	{2,(char)0xD7,"cdot"},
	{2,(char)0xB7,"bullet"},
	{2,(char)0xB8,"div"},
	{2,(char)0xC5,"oplus"},
	{2,(char)0xC4,"otimes"},
	{2,(char)0xC8,"cup"},
	{2,(char)0xC7,"cap"},
	{2,(char)0x7E,"sim"},
	{2,(char)0x40,"cong"},
	{2,(char)0xBA,"equiv"},
	{2,(char)0xA0,"doteq"},
	{2,'P',"parallel"},
	{2,'p',"perp"},
	{2,(char)0xB5,"propto"},
	{2,(char)0xCC,"subset"},
	{2,(char)0xCD,"subseteq"},
	{2,(char)0xC9,"supset"},
	{2,(char)0xCA,"supseteq"},
	{2,0x7C,"mid"},
	{2,(char)0x5C,"therefore"},
	{2,(char)0x5B,"because"},
	{2,']',"setminus"},


	{2,(char)0xBB,"approx"},
	{2,(char)0xD0,"angle"},

	{1,(char)0xC0,"aleph"},
	{1,(char)0xC6,"emptyset"},
	{1,(char)0xA5,"infty"},
	{1,'a',"alpha"},
	{1,'b',"beta"},
	{1,'g',"gamma"},
	{1,'d',"delta"},
	{1,'e',"epsilon"},
	{1,'e',"ep"},
	{1,'z',"zeta"},
	{1,'h',"eta"},
	{1,'q',"theta"},
	{1,'i',"iota"},
	{1,'k',"kappa"},
	{1,'l',"lambda"},
	{1,'m',"mu"},
	{1,'n',"nu"},
	{1,'x',"xi"},

	{1,'p',"pi"},
	{1,'r',"rho"},
	{1,'s',"sigma"},
	{1,'t',"tau"},
	{1,'u',"upsilon"},
	{1,'u',"ups"},
	{1,'f',"phi"},
	{1,'c',"chi"},
	{1,'y',"psi"},
	{1,'w',"omega"},
	{1,'j',"varphi"},
	{1,'v',"varpi"},
	
	/*{1,4,"e"}, //natural number e
	{1,11,"i"}, //imaginary unit
	{1,12,"j"}, //imaginary unit*/

	{2,(char)0xA3,"leq"},
	{2,(char)0xB3,"geq"},
	{2,(char)0xCE,"elm"}, 
	{2,(char)0x27,"owns"}, 
	{2,(char)0xCF,"notin"},
	{2,(char)0x03,"circ"},
	{2,(char)0xE2,"mapsto"},
	{2,(char)0x24,"exists"},
	{2,(char)0x22,"forall"},
	{2,(char)0xF0,"square"},
	{2,(char)0x9E,"triangle"},

	//{5,'T',"text"},
	{5,'a',"bra"},
	{5,'k',"ket"},
	{5,'c',"ceil"},
	{5,'f',"floor"},
	{9,'H',"link"},
	{9,'L',"label"},

	{100,' ',"t"}, //tonne
	{100,' ',"g"}, //gram

	{100,' ',"m"}, //meter
	{100,' ',"s"}, //second
	{100,' ',"mol"}, //
	{100,' ',"cd"}, //candela
	{100,' ',"Hz"}, //hertz

	{100,' ',"A"}, //ampere
	{100,' ',"V"}, //volt
	{100,' ',"T"}, //tesla
	{100,' ',"Wb"}, //webber
	{100,' ',"F"}, //Faraday
	{100,' ',"C"}, //Coloumb
	{100,' ',"ohm"}, //Ohm
	{100,' ',"S"}, //Siemens
	{100,' ',"H"}, //Henry
	{100,' ',"eV"}, //electron-volt
	{100,' ',"lm"}, //lumen
	{100,' ',"lx"}, //lux
	{100,' ',"Bq"}, //Faraday
	{100,' ',"Gy"}, //Faraday
	{100,' ',"Sv"}, //Faraday
	{100,' ',"kat"}, //Faraday

	{100,' ',"J"}, //joul
	{100,' ',"N"}, //Newton
	{100,' ',"W"}, //watt
	{100,' ',"K"}, //kelvin
	{100,' ',"Pa"}, //pascal
	{100,' ',"Wh"}, //Watt-hour
	{101,' ',"$"}, //dollar sign n
	{101,' ',"€"}, //euro sign 

	{101,' ',"deg"}, //degreens
	{101,' ',"\'\'\'"}, //degreens (alternative way)
	{101,' ',"'"}, //minutes
	{101,' ',"''"}, //seconds
	{101,' ',"rad"}, //radians
	{101,' ',"sr"}, //steradians
	{101,' ',"L"}, //liter
	{101,' ',"degC"}, //celsius
	{101,' ',"\'\'\'C"}, //celsius (alternative way)
	{101,' ',"\'C"}, //celsius (alternative way)
	{101,' ',"lb"}, //pound
	{101,' ',"lbm"}, //pound
	{101,' ',"oz"}, //ounce
	{101,' ',"ton"}, //ton
	{101,' ',"mi"}, //mile
	{101,' ',"nmi"}, //nautical-mile
	{101,' ',"ft"}, //foot
	{101,' ',"in"}, //inch
	{101,' ',"yd"}, //yard
	{101,' ',"h"}, //hour
	{101,' ',"mn"}, //minute
	{101,' ',"day"}, //day
	{101,' ',"yr"}, //year
	{101,' ',"pt"}, //pint
	{101,' ',"qt"}, //quarat
	{101,' ',"gal"}, //galon
	{101,' ',"bbl"}, //barrel
	{101,' ',"dB"},  //decibel
	{101,' ',"ppm"},  //part per milion
	{101,' ',"mph"}, //miles per hour

	/*{101,' ',"mps"}, // m/s
	{101,' ',"radps"}, // rad/s
	{101,' ',"Apm"}, // A/m
	{101,' ',"kgpm"}, // kg/m
	{101,' ',"Vpm"}, //ounce*/
	{101,' ',"Nm"}, // Nm
	{101,' ',"Ns"}, // Ns
	{101,' ',"mph"}, // km/h

	{0,0,""}
};
const char *GetCommandFromCreationCode(short CreationCode);

unsigned char CursorBlinkState;
unsigned char OperatorLevelTable[256];
#define GetOperatorLevel(x) ((int)OperatorLevelTable[(unsigned char)(x)])
extern CExpression *TheKeyboardClipboard;
//#define PROFILE_ON

#ifdef PROFILE_ON

	typedef struct
	{
		unsigned int cnt;
		unsigned int time;
		int atime;
		unsigned int from[20];
	} tProfiler;

	struct
	{
		tProfiler OVERALL;
		tProfiler Compute;
		tProfiler FactorizeExpression;
		tProfiler Polynomize;
		tProfiler ExecuteComputation;
		tProfiler MultiplyElements;
		tProfiler CompareExpressions;
		tProfiler ComputeFraction;
		tProfiler ComputeRoot;
		tProfiler ComputeExponent;
		tProfiler ComputeLog;
		tProfiler DividePolynome;
		tProfiler ExtractVariables;
		tProfiler StrikeoutCommonFactors;		
		tProfiler StrikeoutRemove;
	} PROFILER;

	tProfiler *profiler_last_timed;
#define PROFILE_CNT(x) tProfiler *profilerid=NULL;((x)->cnt)++;
#define PROFILE_TIME(x) tProfiler *profilerid=x;DWORD profiler_start=GetTickCount();\
	if (profiler_last_timed) ((x)->from)[(((char*)profiler_last_timed-(char*)&PROFILER)/sizeof(tProfiler))]++;\
	((x)->cnt)++;\
	tProfiler *profiler_caller=profiler_last_timed;\
	profiler_last_timed=x;
#define xreturn(x) {tProfiler *tp=(tProfiler*)&PROFILER;int profilerretval=(x);\
	if (profilerid)\
	{\
		int delta=GetTickCount()-profiler_start;\
		profilerid->time+=delta;profilerid->atime+=delta;\
		if (profiler_caller) profiler_caller->atime-=delta;\
		profiler_last_timed=profiler_caller;\
	}\
	return profilerretval;}

#else
#define PROFILE_CNT(x)
#define PROFILE_TIME(x)
#define xreturn(x) return x
#endif

extern short CElementInitType; //passed through global variable for speed
extern CObject *CElementInitPaternalExpression; //passed through global variable for speed

extern CExpression* ClipboardExpression;
extern CExpression* prevClipboardExpression;
extern int PrintRendering;
extern PopupMenu *Popup;
extern RECT TheClientRect; 

const char *GetCommandFromCreationCode(short CreationCode)
{
	int i=0;
	while (ListOfKnownFunctions[i].name[0])
	{
		int type=ListOfKnownFunctions[i].type;
		if ((type*256+ListOfKnownFunctions[i].oper)==CreationCode) return ListOfKnownFunctions[i].name;
		i++;
	}
	return NULL;
}


//the CExpression represents an mathematical expression of type 'A+B*C/3'
//the CExpression contains list of its elements
//the CExpression has frame around its elements (all elements can be selected by pointing at the frame)
//note: an CExpression cannot be element of CExpression. Only CElement can be element of CExpression
//      however, and CElement can have up to three CExpression subelements
//must be very fast
CExpression::CExpression(CElement *PaternalElement, CExpression *PaternalExpression, short int FontSize)
{
	//paternal element and paternal expression cannot be changed later
	m_pPaternalElement=PaternalElement;
	m_pPaternalExpression=PaternalExpression;

	m_FontSize=FontSize; 
	m_ElementListReservations=0;
	m_pElementList=NULL;
	m_MatrixRows=NULL;
	m_MatrixColumns=NULL;
	m_StartAsText=0;

	//creating memory list for elements, adds an dummy object (type=0)
	Delete();
}

//must be very fast
CExpression::~CExpression(void)
{
	//special handling - if clipboard is deleted, send command to other windows that it must be deleted
	if (this==ClipboardExpression)
	{
		if (theApp.m_pMainWnd)
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

	if (KeyboardEntryObject==(CObject*)this) {KeyboardEntryObject=NULL;KeyboardEntryBaseObject=NULL;}

	if (m_MatrixRows) {HeapFree(ProcessHeap,0,m_MatrixRows);m_MatrixRows=NULL;}
	if (m_MatrixColumns) {HeapFree(ProcessHeap,0,m_MatrixColumns);m_MatrixColumns=NULL;}
	if (m_pElementList)
	{
		tElementStruct *theElement=m_pElementList+m_NumElements-1;
		for (;theElement>=m_pElementList;theElement--)
			if (theElement->pElementObject)
				delete theElement->pElementObject;

		HeapFree(ProcessHeap,0,m_pElementList);
	}
}

//this will insert an empty element into the expression
//this is primarely used in toolboxes (to create toolbox elements)
//shouf be fast
int CExpression::InsertEmptyElement(short position, short type,char Operator,int color)
{
	int ret;
	//if (type<0) return 0;
	if (position<0) return 0;
	if (position>m_NumElements) return 0;

	tElementStruct myElement;
	myElement.Above=0;
	myElement.Below=0;
	myElement.Length=0;
	myElement.IsSelected=0;
	myElement.X_pos=0;
	myElement.Y_pos=0;
	myElement.Type=(char)type;
	myElement.pElementObject=NULL;
	myElement.Decoration=0;

	if (type!=0)
	{
		//temporary creating an element
		CElementInitPaternalExpression=(CObject*)this;
		CElementInitType=type;
		myElement.pElementObject=new CElement(); 
		myElement.pElementObject->Empty(Operator);
		
		//determine the text mode (depending if there is a text variable just before)
		myElement.pElementObject->m_Text=DetermineInsertionPointType(position);
		myElement.pElementObject->m_Color=color;
	}

	ret=MoveElementInto(&myElement,position);
	return ret;
}




//calculates the size (in pixels) of the whole expression. It prepares the expression
//for displaying
//this function should be relatively fast.

void CExpression::CalculateSize(CDC *DC,short int zoom, short int * length, short int * above, short int * below,char HQR,char optimize_for_readability)
{
	
	try
	{
	int external=0;
	if ((HQR==-1) && ((zoom<=50) || (IsHighQualityRendering)) && (optimize_for_readability==0)) {external=zoom;zoom=HQZM;}



	int startHQR=-1;
	int endHQR=-1;

	if (HQR==-1) 
	{
		HQR=IsHighQualityRendering;

		if ((HQR) && (KeyboardEntryBaseObject) && (this->m_pPaternalExpression==NULL))
		{
			if (this==(CExpression*)KeyboardEntryBaseObject->Object)
			{
				int t=ContainsBlinkingCursor();
				if ((t>0) && (t<=m_NumElements))
				{
					if ((this->m_MaxNumColumns>1) || (this->m_MaxNumRows>1))
					{
						int fnd=0;
						startHQR=0;
						for (int i=0;i<m_NumElements;i++)
						{
							tElementStruct *ts=m_pElementList+i;
							if ((ts->Type==11) || (ts->Type==12)) {if (fnd==0) startHQR=i;if (fnd==1) {endHQR=i;fnd=2;}}
							if (i==t-1) fnd=1;
						}
						if (endHQR==-1) endHQR=m_NumElements;
					}
					else 
					{
						HQR=0;
						startHQR=0;
						endHQR=m_NumElements;
					}
				}
			}
		}
	}



	if (this==NULL) return;
	if (DC==NULL) return;
	if (zoom<5) zoom=5;
	if (zoom>5000) zoom=5000;




	int is_matrix=0;
	{
		tElementStruct *p=m_pElementList+1;
		for (int i=1;i<m_NumElements-1;i++,p++)
			if ((p->Type==11) || (p->Type==12)) {is_matrix=1;break;}
	}

	int ActualSize=GetActualFontSize(zoom);
	//mf=(CMainFrame*)theApp.m_pMainWnd;

	m_ParenthesesAbove=5*ActualSize/12;
	m_ParenthesesBelow=5*ActualSize/12;


	//deciding whether it is needed to paint parentheses around the expression
	//m_ParenthesesFlags is a bit mask, second bit tells if parentheses are automatic or not
	m_DrawParentheses=0;
	m_ParenthesesFlags&=0x7F;  //the most important bit will be calculated below

	if ((m_ParenthesesFlags&0x02) &&   //auto parentheses
		(m_pPaternalElement) &&
		(m_pPaternalElement->Expression1==(CObject*)this))
	{
		if (m_pPaternalElement->m_Type==3)  //exponent (power)
		{
			if (m_NumElements>2)
			{				
				m_ParenthesesFlags|=0x80; //force on
			}
			else if (m_NumElements==2)
			{
				tElementStruct *ts1=m_pElementList;
				tElementStruct *ts2=ts1+1;
				if (((ts1->Type==1) && (ts1->pElementObject->Data1[0]==0) && ((ts2->Type==6) || (ts2->Type==1) ||(ts2->Type==5))  ) ||
					((ts2->Type==1) && (ts2->pElementObject->Data1[0]==0) && ((ts1->Type==6) || (ts1->Type==1) ||(ts1->Type==5)) ))
				{
				}
				else
					m_ParenthesesFlags|=0x80; //force on
				if (FrameSelections==0) m_ParenthesesFlags|=0x80;
			}
			else if ((m_pElementList->Type!=1) && (m_pElementList->Type!=6) && 
					((m_pElementList->Type!=4) || (m_pElementList->pElementObject->Data1[0]!=' ')) && 
					(m_pElementList->Type!=0) && (m_pElementList->Type!=5))
					m_ParenthesesFlags|=0x80; //force on
		}

		if (m_pPaternalElement->m_Type==5) //parentheses
		{
			m_ParenthesesFlags|=0x80; //always force
		}

		if (m_pPaternalElement->m_Type==6)  //function
		{	
			if (m_pPaternalElement->IsDifferential())
			{
				if (m_NumElements>2) 
					m_ParenthesesFlags|=0x80; //force on
				else if (m_NumElements==2)
				{
					tElementStruct *ts1=m_pElementList;
					tElementStruct *ts2=ts1+1;
					if (((ts1->Type==1) && (ts1->pElementObject->Data1[0]==0)) ||
						((ts2->Type==1) && (ts2->pElementObject->Data1[0]==0)))
					{
					}
					else
						m_ParenthesesFlags|=0x80; //force on
					if (FrameSelections==0) m_ParenthesesFlags|=0x80;
				}
			
			}
			else
			{
				//go through element list, if found any operator other than multyplying dot'
				//and even if found some strange elements...
				int ii;
				char found_one_already=0;
				for (ii=0;ii<m_NumElements;ii++)
				{
					int Type=(m_pElementList+ii)->Type;
					if ((Type==2) /*&& ((m_pElementList+ii)->pElementObject->Data1[0]!=(char)0xD7)*/) break;
					if (Type>=6) break; 
					if (Type>2) //no two or more other elements
					{
						if (found_one_already) break;
						found_one_already=1;
					}
				}
				if (ii<m_NumElements)
					m_ParenthesesFlags|=0x80;
			}
		}

		if ((m_pPaternalElement->m_Type==7) && 
			(m_pPaternalElement->Data1[0]!='I') && //integral
			(m_pPaternalElement->Data1[0]!='O')) //circular integral
		{	
			//go through element list, if found any operator other tha...
			//and even if found some strange elements...
			int ii;
			for (ii=0;ii<m_NumElements;ii++)
			{
				int Type=(m_pElementList+ii)->Type;
				if ((Type==2) && ((m_pElementList+ii)->pElementObject->Data1[0]!=(char)0xD7)) break;
				if (Type>=9) break;
			}
			if (ii<m_NumElements)
				m_ParenthesesFlags|=0x80;
		}
	}

	//show parentheses if they are forced by user or by automatic algoritam (above)
	if (m_ParenthesesFlags&0x81)
	{
		m_DrawParentheses=m_ParentheseShape;
		if ((m_DrawParentheses!='[') && (m_DrawParentheses!='{') && (m_DrawParentheses!='|') &&
			(m_DrawParentheses!='/') && (m_DrawParentheses!='\\') &&
			(m_DrawParentheses!='<') && (m_DrawParentheses!='b') &&
			(m_DrawParentheses!='l') && (m_DrawParentheses!='r') &&
			(m_DrawParentheses!='a') && (m_DrawParentheses!='k') &&
			(m_DrawParentheses!='c') && (m_DrawParentheses!='f') &&
			(m_DrawParentheses!='x') && (m_DrawParentheses!='T')) 
			m_DrawParentheses='(';
	} 
	else
		m_ParenthesesSelected=0;

	//calculate margines
	int MarginX; 
	int MarginY; 
	int PrecisionMarginX;
	int PrecisionMarginY;
	if (HQR) 
		{MarginY=ActualSize/8;PrecisionMarginY=64*ActualSize/8;}
	else
		{MarginY=ActualSize/4;PrecisionMarginY=64*ActualSize/4;if (zoom>200) {MarginY=2*ActualSize/9;PrecisionMarginY=64*2*ActualSize/9;}}
	MarginX=(ActualSize+2)/5;
	PrecisionMarginX=64*(ActualSize+2)/5;
	int is_index=0;
	if ((m_pPaternalElement) && 
		((m_pPaternalElement->m_Type==1) ||
		((m_pPaternalElement->m_Type==5) && (m_pPaternalElement->Expression2==(CObject*)this)) ||
		((m_pPaternalElement->m_Type==6) && (m_pPaternalElement->Expression2==(CObject*)this))))
		is_index=1;

	//store margines for later usage
	m_MarginX=MarginX;
	m_MarginY=MarginY;


	//int xposerror=0;
	*length=MarginX;
	//xposerror=PrecisionMarginX-66*MarginX;

	if ((m_FontSize>250) && (m_pPaternalExpression==NULL) && ((m_ParenthesesFlags&0x81)==0) && (!is_matrix))
		*length=*length*250/m_FontSize;  //for better left alignent
	if ((HQR)&&(m_pPaternalExpression) && (!is_matrix)) {*length=MarginX/4;if (m_ParenthesesFlags&0x81) *length=(PrecisionMarginX+26)/256; }
	if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==3) && (this->m_pPaternalElement->Expression1==(CObject*)this) &&
		((this->m_ParenthesesFlags&0x81)==0)) 
	{
		//special handling for exponents - no insertion point in front of an exponent (for better rendering and simpler editing) //BABA
		if (this->m_pElementList->Type)
			*length=0; 
		else 
			*length=MarginX/2;
	}

	
	*above=0;
	*below=0;


	int contains_text=IsTextContained(-1);
	int Ycenterline=0,LastlineBelow=0;
	int TextlineStart=0;
	int TextBeginning=0;
	int Row=0;
	int Column=0;
	int MaxNumColumns=1;
	int MaxNumRows=1;
	if (m_MatrixRows)
		memset((void*)m_MatrixRows,0,sizeof(tMatrixRows)*50);
	if (m_MatrixColumns)
		memset((void*)m_MatrixColumns,0,sizeof(tMatrixColumns)*50);

	int textlinestartpos=*length;
	int LineXpos=textlinestartpos;
	
	int i;
	for (i=0;i<m_NumElements;i++)
	{

		tElementStruct *theElement;
		theElement=m_pElementList+i;

		if (theElement->Type==0)  //empty frame
		{
			theElement->X_pos=LineXpos;
			theElement->Y_pos=Ycenterline;
			theElement->Length=ActualSize/3;//ActualSize/3;
			theElement->Above=4*ActualSize/9;//ActualSize/4;
			theElement->Below=4*ActualSize/10;//ActualSize/3;
			if (HQR) {theElement->Above=7*ActualSize/18;theElement->Below=4*ActualSize/10;}
			if (m_StartAsText)
			{
				theElement->Length=ActualSize/2;
				theElement->Above=ActualSize/3;
			}
		}

		
		if ((theElement->Type>0) && (theElement->pElementObject))
		{
			char tmpHQR=HQR;
			if ((i>=startHQR) && (i<=endHQR)) tmpHQR=0;

			theElement->X_pos=LineXpos;
			theElement->Y_pos=Ycenterline;
			theElement->pElementObject->CalculateSize(DC,zoom,&(theElement->Length),&(theElement->Above),&(theElement->Below),i,tmpHQR);
			LineXpos=theElement->X_pos; //because the X_pos can change in previous call
		}

		if (is_index) 
			LineXpos+=theElement->Length+MarginX/2; //whthin index, we make more condensed rendering
		else
			LineXpos+=theElement->Length+MarginX;


		if (((theElement->Type!=2) || (theElement->pElementObject->Data1[0]!=(char)0xFF)) &&
			(theElement->Type!=12) && (theElement->Type!=11))
		if (LineXpos>*length) *length=LineXpos;

		if (((theElement->Type==2) && (theElement->pElementObject->Data1[0]==(char)0xFF)) || (i==m_NumElements-1) || (theElement->Type==11) || (theElement->Type==12))
		{
			int txAbove=0,txBelow=0;

			for (int j=TextlineStart;j<=i;j++)
			{
				tElementStruct *ts=m_pElementList+j;
				int delta_above=(contains_text)?ActualSize/8:0;
				int delta_below=delta_above;
				if (ts->Type==1)
				{
					delta_above=(contains_text)?-ActualSize/16:-ActualSize/16;
					if (ts->pElementObject->Expression1==0) 
						delta_below=(contains_text)?-ActualSize/16:-ActualSize/16;
					else
						delta_below=(contains_text)?ActualSize/3:0;
				}
				if (ts->Type==3)
				{
					if (contains_text) delta_above=ActualSize/4;
				}
				if (ts->Above-delta_above>txAbove) txAbove=ts->Above-delta_above;
				if (ts->Below-delta_below>txBelow) txBelow=ts->Below-delta_below; 
			}

			if (txAbove<2*txBelow/3) txAbove=2*txBelow/3;
			if (txBelow<2*txAbove/3) txBelow=2*txAbove/3;
			if (TextlineStart==TextBeginning) Ycenterline-=txAbove+7*ActualSize/16;
			Ycenterline+=max(1,LastlineBelow+txAbove+7*ActualSize/16);


			for (int j=TextlineStart;j<=i;j++)
			{
				tElementStruct *ts=m_pElementList+j;
				if (ts->Y_pos<Ycenterline) ts->Y_pos=Ycenterline;

				if (ts->Type==1) 
				{
					if (ts->Y_pos-ts->Above-MarginY<-*above) *above=ts->Above+MarginY;
					if (ts->pElementObject->Expression1==NULL)
					{
						if (ts->Y_pos+ts->Below+MarginY>*below) *below=ts->Y_pos+ts->Below+MarginY;
					}
					else
					{
						if (ts->Y_pos+ts->Below>*below) *below=ts->Y_pos+ts->Below;
					}
				}
				else 
				{
					if (ts->Y_pos-ts->Above<-*above) *above=ts->Above;
					if (ts->Y_pos+ts->Below>*below) *below=ts->Y_pos+ts->Below;
				}
				if (ts->pElementObject)
				{
					if (m_ParenthesesAbove<ts->pElementObject->ParenthesesAbove-ts->Y_pos) m_ParenthesesAbove=ts->pElementObject->ParenthesesAbove-ts->Y_pos;
					if (m_ParenthesesBelow<ts->pElementObject->ParenthesesBelow+ts->Y_pos) m_ParenthesesBelow=ts->pElementObject->ParenthesesBelow+ts->Y_pos;
				}
			}
			

			LastlineBelow=txBelow;
			if (theElement->Type==2) TextlineStart=i+1;
			LineXpos=textlinestartpos;
			//xposerror=0;
		}

		if ((theElement->Type==11) || (theElement->Type==12) || (i==m_NumElements-1))  //Matrix horizontal and vertical spacer
		{
			if (((this->m_pPaternalExpression) || (i<m_NumElements-1) || (Row) || (Column))
				&& (TextlineStart!=TextBeginning))
			{
				//center align text verticaly (if not simple text at document level)
				int delta=(*below-*above)/2;
				for (int j=TextBeginning;j<=i;j++)
					(m_pElementList+j)->Y_pos-=delta;
				*below-=delta;
				*above+=delta;
				m_ParenthesesAbove+=delta;
				m_ParenthesesBelow-=delta;
			}

			//aligning text center-left-right
			int LineStart=TextBeginning;
			for (int j=TextBeginning;j<=i;j++)
			{
				if ((((m_pElementList+j)->Type==2) && ((m_pElementList+j)->pElementObject->Data1[0]==(char)0xFF)) || (j==i))
				{
					int EndX=(m_pElementList+j)->X_pos+(m_pElementList+j)->Length;
					if (j)
					if ((((m_pElementList+j)->Type==2) && ((m_pElementList+j)->pElementObject->Data1[0]==(char)0xFF)) ||
						((m_pElementList+j)->Type==11) ||
						((m_pElementList+j)->Type==12))
						EndX=(m_pElementList+j-1)->X_pos+(m_pElementList+j-1)->Length;
					int align=this->m_Alignment;
					int delta=0;
					tCellAttributes attrib;
					if (GetCellAttributes(Row,Column,&attrib))
					{
						if (*attrib.alignment=='l') align=1;
						if (*attrib.alignment=='c') align=0;
						if (*attrib.alignment=='r') align=2;
					}
					if (align==0) delta=(*length-EndX-MarginX)/2; //center alignmet
					if (align==2) delta=*length-EndX-MarginX; //right alignment
					for (int k=LineStart;k<=j;k++)
						(m_pElementList+k)->X_pos+=delta;
					LineStart=j+1;
				}
			}

			if ((theElement->Type==11) || (theElement->Type==12))
			{
				if (m_MatrixRows==NULL) m_MatrixRows=(tMatrixRows*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,50*sizeof(tMatrixRows));
				if (m_MatrixColumns==NULL) m_MatrixColumns=(tMatrixColumns*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,50*sizeof(tMatrixColumns));
			
				if ((HQR) && (m_pPaternalExpression) && (!is_matrix)) 
				{
					if (m_ParenthesesFlags&0x81) 
						*length-=MarginX-(PrecisionMarginX+26)/256;
					else
						*length-=MarginX-MarginX/4;
				}

				//if (HQR) *length-=MarginX;
				if (m_MatrixRows[Row].above<*above) m_MatrixRows[Row].above=*above;
				if (m_MatrixRows[Row].below<*below) m_MatrixRows[Row].below=*below;
				if (m_MatrixColumns[Column].length<*length) m_MatrixColumns[Column].length=*length;
				*length=MarginX;
				if ((HQR)&&(m_pPaternalExpression)&&(!is_matrix)) {*length=MarginX/4;if (m_ParenthesesFlags&0x81) *length=(PrecisionMarginX+26)/256;}

				*above=0;*below=0;
				TextlineStart=i+1;
				TextBeginning=i+1;
				Ycenterline=0;
				textlinestartpos=*length;
				LineXpos=textlinestartpos;
				//xposerror=0;
				LastlineBelow=0;
				//Column++;
				if (theElement->Type==12)
				{
					if (Column>MaxNumColumns) MaxNumColumns=Column;
					if (i<m_NumElements-1)
					{
						Column=0;
						Row++;
					}
				}
				else
					Column++;
			}
		}
	}

	if ((HQR) && (m_pPaternalExpression) && (!is_matrix)) 
	{
		if (m_ParenthesesFlags&0x81) 
			*length-=MarginX-(PrecisionMarginX+26)/256;
		else
			*length-=MarginX-MarginX/4;
	}



	/*{
	if (xposerror>64) (*length)++;
	if (xposerror<-64) if ((*length)>0) (*length)--;
	}*/

	if ((Column==0) && (Row==0)) goto CalculateSize_DrawParentheses;

	//the following part of code is only for matrices

	//if (HQR) *length-=MarginX;
	if (m_MatrixRows[Row].above<*above) m_MatrixRows[Row].above=*above;
	if (m_MatrixRows[Row].below<*below) m_MatrixRows[Row].below=*below;
	if (m_MatrixColumns[Column].length<*length) m_MatrixColumns[Column].length=*length;
	if (Column+1>MaxNumColumns) MaxNumColumns=Column+1;
	if (Row+1>MaxNumRows) MaxNumRows=Row+1;

	//okay, now we have calculated size of every element, width of
	//every column,height of every row and number of columns and rows.
	//now continue placing elements into matrix

	Row=0;Column=0;
	int l=MarginX,a=0,b=0;
	int ColumnStart=0;
	int ColumnStartX=MarginX/2;//(HQR)?0:MarginX/2;
	int MatrixHeight=0;
	
	int ggg;
	if (HQR)
		ggg=6*(((zoom>200)?(128*ActualSize/9):(16*ActualSize))-64*MarginY-3)/4;
	else
		ggg=5*(PrecisionMarginY-64*MarginY)/2;
	int ttt=0;
	int ttt2=0;
	for (i=0;i<MaxNumRows;i++) 
	{
		ttt2+=ggg;
		int ccc=(ttt2-ttt)/64;
		m_MatrixRows[i].above+=ccc;
		m_MatrixRows[i].below+=ccc;
		ttt+=ccc*64;
		
		MatrixHeight+=m_MatrixRows[i].above+m_MatrixRows[i].below;
	}

	MatrixHeight+=(MaxNumRows+1)*MarginY;

	int RowStart=0;
	int RowStartY=-(MatrixHeight/2)+m_MatrixRows[0].above+MarginY;
	if (MaxNumRows==1) 
	{
		RowStartY=0;
	}

	m_MaxNumRows=MaxNumRows;
	m_MaxNumColumns=MaxNumColumns;

	for (int i=0;i<m_MaxNumColumns;i++)
		m_MatrixColumns[i].length+=m_MarginX;

	*length=0;
	*above=0;
	*below=0;


	char *prevcellalign=NULL;
	for (i=0;i<=m_NumElements;i++)
	{
		tElementStruct *theElement;
		theElement=m_pElementList+i;

		if ((i==m_NumElements) || (theElement->Type==11) || ((theElement->Type==12) && (i<m_NumElements-1)))
		{
			int j;
			l-=2*MarginX;
			int Delta;
			int align=m_Alignment;
			char *cellalign=NULL;
			if (i<m_NumElements) 
			{
				cellalign=&(theElement->pElementObject->Data1[10]);
				if (theElement->Type==11) prevcellalign=&(theElement->pElementObject->Data1[11]); else prevcellalign=&(theElement->pElementObject->Data1[12]);
			}
			else
				cellalign=prevcellalign;
			if (cellalign)
			{
				if (*cellalign=='l') align=1;
				if (*cellalign=='c') align=0;
				if (*cellalign=='r') align=2;
			}

			if (align==1) Delta=5*MarginX/4;								        //left alignment
			else if (align==2) Delta=(m_MatrixColumns[Column].length-l)-5*MarginX/4;   //right alignment
			else Delta=(m_MatrixColumns[Column].length-l)/2;					//center alignment
			int increment=-MarginX;
			if (HQR) 
			{
				//if (Column==0) increment+=MarginX/2;
				//if (Column==m_MaxNumColumns-1) increment-=MarginX/2;
			}
			int hh=ColumnStartX-(m_pElementList+ColumnStart)->X_pos;
			for (j=ColumnStart;j<i;j++)
			{
				tElementStruct *ts=m_pElementList+j;
				ts->X_pos+=ColumnStartX+Delta+increment;
			}
			m_MatrixColumns[Column].x=ColumnStartX;

			if (i<m_NumElements)
			{
				theElement->X_pos+=ColumnStartX+Delta+increment;
				theElement->Length=0;
				if (theElement->Type==11) ColumnStartX+=(HQR)?(1*MarginX):MarginX;
			}
			ColumnStartX+=m_MatrixColumns[Column].length;
			if (*length<ColumnStartX) *length=ColumnStartX;
			ColumnStart=i+1;
			l=MarginX;
			Column++;
			if ((theElement->Type==11) && (i<m_NumElements)) continue;
		}

		if ((i==m_NumElements) || ((theElement->Type==12) && (i<m_NumElements-1)))
		{
			int j;
			int Delta=0;m_MarginY;
			int hh=RowStartY-(m_pElementList+RowStart)->Y_pos;
			for (j=RowStart;j<i;j++)
			{
				tElementStruct *ts=m_pElementList+j;
				ts->Y_pos+=RowStartY;//hh+Delta;
				if (ts->Type==11) {ts->Above=m_MatrixRows[Row].above;ts->Below=m_MatrixRows[Row].below;}
			}
			if (*above<m_MatrixRows[Row].above-RowStartY) *above=m_MatrixRows[Row].above-RowStartY;
			if (*below<m_MatrixRows[Row].below+RowStartY) *below=m_MatrixRows[Row].below+RowStartY;

			m_MatrixRows[Row].y=RowStartY;

			if (i<m_NumElements)
			{
				theElement->Length=0;
				theElement->Y_pos+=RowStartY;
				theElement->Above=m_MatrixRows[Row].above;
				theElement->Below=m_MatrixRows[Row].below;
				RowStartY+=m_MatrixRows[Row].below+MarginY;
				RowStart=i+1;
				ColumnStartX=MarginX/2;//(HQR)?0:MarginX/2;
				Column=0;
				a=0;b=0;
				Row++;
				RowStartY+=m_MatrixRows[Row].above;
				continue;
			}
			else break;
		}

		if ((theElement->Type!=12) && (theElement->Type!=11) &&
			((theElement->Type!=2) || (theElement->pElementObject->Data1[0]!=(char)0xFF)))
		if (l<MarginX+theElement->X_pos+theElement->Length) l=MarginX+theElement->X_pos+theElement->Length;

		if (theElement->Type==1)
		{
			if (theElement->pElementObject->Expression2==NULL)
			{
				if (a<theElement->Above+MarginY) a=theElement->Above+MarginY;
			}
			else
			{
				if (a<theElement->Above) a=theElement->Above;
			}
			if ((theElement->pElementObject->Expression1==NULL) && (theElement->pElementObject->Expression3==NULL))
			{
				if (b<theElement->Below+MarginY) b=theElement->Below+MarginY;
			}
			else
			{
				if (b<theElement->Below) b=theElement->Below;
			}
		}
		else
		{
			if (a<theElement->Above) a=theElement->Above;
			if (b<theElement->Below) b=theElement->Below;
		}
	}
	

	if ((MaxNumColumns>1) || (MaxNumRows>1))
	{
		if (!HQR)
		{
			*above+=2*MarginY/3;
			*below+=2*MarginY/3;
			
		}
		*length+=MarginX/2;
		m_ParenthesesAbove=*above;
		m_ParenthesesBelow=*below;
	}

CalculateSize_DrawParentheses:

	if (m_DrawParentheses) 
	{
		//calculate parenthese height
		if (((m_ParenthesesFlags&0x04)==0) && (m_DrawParentheses!='b') && (m_DrawParentheses!='x'))
		{
			//for vertical (standard) parentheses
			/*if (m_ParentheseHeightFactor==0) //large - to cover full height of expression
			{
				m_ParenthesesAbove=*above;
				m_ParenthesesBelow=*below;
			}
			else if (m_ParentheseHeightFactor==1) // medium - to cover only rational numbers (but no margin, exponent and indexes)
			{
				//just use the above calculated size
			} else if (m_ParentheseHeightFactor==2) //forced small parentheses - only the font size
			{
				m_ParenthesesAbove=(9*ActualSize/24);
				m_ParenthesesBelow=(9*ActualSize/24);
				if (m_MaxNumRows>1)
				{
					m_ParenthesesAbove+=-m_MatrixRows[0].y;
					m_ParenthesesBelow+=m_MatrixRows[m_MaxNumRows-1].y;
				}
			}*/

			int tmmp=(int)sqrt(((double)(m_ParenthesesAbove+m_ParenthesesBelow))/ActualSize);
			m_ParentheseWidth=ActualSize/5+tmmp;
			if (m_ParentheseShape=='{') m_ParentheseWidth=ActualSize/4+tmmp;
			if ((m_ParentheseShape=='/') || (m_ParentheseShape=='<')) m_ParentheseWidth=ActualSize/3+2*tmmp;
			if (HQR)
			{
				if (m_ParentheseShape=='(')
					m_ParentheseWidth+=ActualSize/9;
				else
					m_ParentheseWidth+=(ActualSize/20)*2;
				m_ParenthesesAbove+=(ActualSize/16);
				m_ParenthesesBelow+=(ActualSize/16);
			}
		}
		else
		{
			//for horizontal parentheses
			m_ParentheseWidth=ActualSize/5+(*length)/80;
			//if (!m_DontAddParentheseWidth)
			{
				if (!(m_ParenthesesFlags&0x08))
					*above+=m_ParentheseWidth;
				if (!(m_ParenthesesFlags&0x10))
					*below+=m_ParentheseWidth;	
			}
		}

		if (*above<m_ParenthesesAbove) *above=m_ParenthesesAbove;
		if (*below<m_ParenthesesBelow) *below=m_ParenthesesBelow;

		if (((m_ParenthesesFlags&0x04)==0) || (m_DrawParentheses=='b') || 
			(m_DrawParentheses=='x'))
		{
			int ii;
			int deltax=m_ParentheseWidth;
			if ((HQR) && (m_pPaternalExpression==NULL) && (!is_matrix)) deltax-=ActualSize/8;
			if ((!(m_ParenthesesFlags&0x08)) || (m_DrawParentheses=='b') || (m_DrawParentheses=='x'))
			{
				
				for (ii=0;ii<m_NumElements;ii++)
				{
					tElementStruct *ts=m_pElementList+ii;
					ts->X_pos+=deltax;
				}
				if (m_MatrixColumns)
					for (ii=0;ii<m_MaxNumColumns;ii++)
						m_MatrixColumns[ii].x+=deltax;
				*length+=deltax;
			}
			if ((!(m_ParenthesesFlags&0x10)) || (m_DrawParentheses=='b') || (m_DrawParentheses=='x'))
				*length+=deltax;

			

			//if ((HQR==0) && (IsHighQualityRendering))
			//	m_ParentheseWidth+=ActualSize/10;
		}
		if ((HQR) && (m_DrawParentheses)) m_DrawParentheses|=0x80;
	}


	if ( (m_pPaternalExpression==NULL) && (m_MatrixRows))
	{
		int shift=-m_MatrixRows[0].y;//*above-1*ActualSize/3-MarginY;
		for (int i=0;i<m_NumElements;i++)
		{
			tElementStruct *ts=m_pElementList+i;
			ts->Y_pos+=shift;
		}
		for (int i=0;i<this->m_MaxNumRows;i++)
		{
			m_MatrixRows[i].y+=shift;
		}
		this->m_ParenthesesAbove-=shift;
		this->m_ParenthesesBelow+=shift;

		*below+=shift;
		*above-=shift;
	}


	if ((m_IsVertical) && (HQR))
	{
		*above+=ActualSize/8;
		*below+=ActualSize/8;
	}

	m_OverallLength=*length; //store for future use
	m_OverallAbove=*above;
	m_OverallBelow=*below;


	if ((m_IsVertical) && (m_pPaternalExpression==NULL))
	{
		int l=*length;
		*length=*above+*below;
		*above=(unsigned short)(l/2);
		*below=(unsigned short)(l-l/2);
	}



	if (external)
	{
		CalculateSizeReadjust(external,length,above,below);
	}

	}
	catch(...)
	{
		FatalErrorHandling();
	}


	return;
}


void CExpression::CalculateSizeReadjust(short zoom, short *length, short *above, short *below)
{
	*length=((*length)*zoom+HQZMp)/HQZM;
	*above=((*above)*zoom+HQZMp)/HQZM;
	*below=((*below)*zoom+HQZMp)/HQZM;

	if (this->m_MatrixColumns)
	for (int i=0;i<this->m_MaxNumColumns;i++)
	{
		m_MatrixColumns[i].length=(m_MatrixColumns[i].length*zoom+HQZMp)/HQZM;
		m_MatrixColumns[i].x=(m_MatrixColumns[i].x*zoom+HQZMp)/HQZM;
	}
	if (this->m_MatrixRows)
	for (int i=0;i<this->m_MaxNumRows;i++)
	{
		m_MatrixRows[i].above=(m_MatrixRows[i].above*zoom+HQZMp)/HQZM;
		m_MatrixRows[i].below=(m_MatrixRows[i].below*zoom+HQZMp)/HQZM;
		m_MatrixRows[i].y=(m_MatrixRows[i].y*zoom+HQZMp)/HQZM;
	}
	this->m_MarginX=(this->m_MarginX*zoom+HQZMp)/HQZM;
	this->m_MarginY=(this->m_MarginY*zoom+HQZMp)/HQZM;
	this->m_OverallAbove=(m_OverallAbove*zoom+HQZMp)/HQZM;
	this->m_OverallBelow=(m_OverallBelow*zoom+HQZMp)/HQZM;
	this->m_OverallLength=(m_OverallLength*zoom+HQZMp)/HQZM;
	this->m_ParenthesesAbove=(m_ParenthesesAbove*zoom+HQZMp)/HQZM;
	this->m_ParenthesesBelow=(m_ParenthesesBelow*zoom+HQZMp)/HQZM;
	this->m_ParentheseWidth=(m_ParentheseWidth*zoom+HQZMp)/HQZM;
	
	tElementStruct *ts=this->m_pElementList;
	int row=0;
	int col=0;
	for (int i=0;i<m_NumElements;i++,ts++)
	{
		ts->Above=(ts->Above*zoom+HQZMp)/HQZM;
		ts->Below=(ts->Below*zoom+HQZMp)/HQZM;
		ts->Length=(ts->Length*zoom+HQZMp)/HQZM;
		ts->X_pos=(ts->X_pos*zoom+HQZMp)/HQZM;
		ts->Y_pos=(ts->Y_pos*zoom+HQZMp)/HQZM;
		if ((i>0) && (ts->Type==1) && ((ts-1)->Type==1))
		{
			//handling for text that is joined left (like coma, dot ...)
			CElement *el=ts->pElementObject;
			if ((el->m_Text==2) || (el->m_Text==3))
			{
				CElement *pel=(ts-1)->pElementObject;
				ts->X_pos=(ts-1)->X_pos+pel->Data3[strlen(pel->Data1)]+(m_MarginX/8);
			}
		}
		short l,a,b;
		tCellAttributes attr;
		if (((this->m_MaxNumColumns==1) && (this->m_MaxNumRows==1)) || 
			(this->GetCellAttributes(row,col,&attr)==0))
			attr.alignment=((this->m_Alignment==0)?"c":((this->m_Alignment==1)?"l":"r"));

		if (ts->pElementObject)
			((CElement*)ts->pElementObject)->CalculateSizeReadjust(zoom,&l,&a,&b,*attr.alignment);

		if (ts->Type==11) col++;
		if (ts->Type==12) {col=0;row++;}

	}
}


CBitmap *cursor_bitmap;
//paints the expression - the expression must be already prepared for displaying (by call to CalculateSize)
//this function must be fast!!
void CExpression::PaintExpression(CDC * DC, short zoom, short X, short Y,RECT *ClipReg, int color)
{
	try
	{
	if (zoom<5) zoom=5;
	if ((X<-8000) || (Y<-8000)) return;

	int ActualSize=GetActualFontSize(zoom);

	//determine the color of expression
	if (m_Color!=-1) 
		color=ColorTable[m_Color];
	else if ((m_IsHeadline) && (color==0) && (m_pPaternalExpression==NULL))
		color=RGB(0,128,128);

	//painting yellow shadow (for autocomplete-soruce objects) or blue shadow (when typing indexes or exponents
	if ((AutocompleteSource==this) || ((KeyboardEntryObject==(CObject*)this) && ((KeyboardIndexMode)||(KeyboardExponentMode))))
	if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
	{
		int l,a,b;
		a=this->m_OverallAbove;
		b=this->m_OverallBelow;
		l=this->m_OverallLength-2*m_MarginX;
		int is_empty=0;
		if (AutocompleteSource)
		{
			if (AutocompleteTriggered) is_empty=1;
			int fade=min(a+b,l)/2;
			fade=min(fade,6);
			for (int ii=0;ii<fade+1;ii++)
			{
				int clr;
				
				if (is_empty)
					clr=RGB(255,240+(fade-ii)*2,160+(fade-ii)*8);
				else 
					clr=RGB(255,248+(fade-ii)*1,204+(fade-ii)*3);
				int x=X+ii+m_MarginX/2;
				int y=Y-a-1+ii;
				int len=l-2*ii+m_MarginX;
				int height=a+b+2-2*ii;
				if (ii<fade)
				{
					DC->FillSolidRect(x,y,len,1,clr);
					DC->FillSolidRect(x+len-1,y,1,height,clr);
					DC->FillSolidRect(x,y+height-1,len,1,clr);
					DC->FillSolidRect(x,y,1,height,clr);
				}
				else
					DC->FillSolidRect(x,y,len,height,clr);
			}
			if (!is_empty)
			{
				int w=max(ActualSize/12,1);
				DC->FillSolidRect(X+m_MarginX,Y+b-w,l,w,RGB(255,248,128));
			}
		}
		else
		{
			DC->FillSolidRect(X+m_MarginX,Y-a-1,l,a+b+2,SHADOW_BLUE_COLOR);
		}
	}

	int ActualSize2; 
	if (IsHighQualityRendering) 
	{
		ActualSize2=ActualSize/4;
		if (ActualSize2==0) ActualSize2=1;
	}
	else 
		ActualSize2=ActualSize/3;


	CBitmap *cb=NULL;
	CDC *orgDC=NULL;
	int orgX=X;
	int orgY=Y;
	if (m_pPaternalExpression==NULL)
	{
		if (m_IsVertical)
		{
			orgDC=DC;
			DC=new CDC();
			CDC *ldc=theApp.m_pMainWnd->GetDC();
			DC->CreateCompatibleDC(ldc/*orgDC*/); //IMPORTANT - what DC should be used? Printer or screen? For doPDF it works with screen DC.
			theApp.m_pMainWnd->ReleaseDC(ldc);
			cb=new CBitmap();
			cb->CreateCompatibleBitmap(orgDC,m_OverallLength,m_OverallAbove+m_OverallBelow);
			DC->SelectObject(cb);
			DC->FillSolidRect(0,0,m_OverallLength,m_OverallAbove+m_OverallBelow,RGB(255,255,255));
			X=0;
			Y=m_OverallAbove;
			ClipReg=NULL;
		}

		//we are going to set some DC properties
		DC->SetBkMode(TRANSPARENT);
		DC->SetTextAlign(TA_BASELINE);
	}
	else if (FrameSelections)
	{
		//paint frame around selection ( if is selected or is keyboard entry mode)
		if ((m_Selection==1) || (m_Selection==m_NumElements+1) || 
			((m_IsKeyboardEntry==1) && (m_KeyboardCursorPos==0) && ((m_pElementList+m_IsKeyboardEntry-1)->Type==1)) || 
			((m_IsKeyboardEntry==m_NumElements) && (m_NumElements) && ((m_pElementList+m_IsKeyboardEntry-1)->Type==1) && ((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1[m_KeyboardCursorPos]==0)))
		{
			//finding paternal object, if exists
			int is_pater_object_selected=0;
			tElementStruct *ts=NULL;
			CExpression *exp=this;
			int corr=0;

			int is_cursor_at_end=0;
			if ((m_NumElements==m_IsKeyboardEntry) && (m_IsKeyboardEntry>0) && (m_KeyboardCursorPos>=0) && (m_KeyboardCursorPos<=23))
				if ((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1[m_KeyboardCursorPos]==0) is_cursor_at_end=1;

			if ((this->m_pPaternalElement) && (this->m_pPaternalElement->IsDifferential(1)) && (this->m_pPaternalExpression->m_pPaternalElement))
			{
				//we are transferring to the parent object of the differential //BABA2
				exp=this->m_pPaternalExpression;
				tElementStruct *ts=exp->GetElementStruct(this->m_pPaternalElement);
				if (ts)
				{
					corr=ts->X_pos+ts->pElementObject->E1_posX;
					if (ts-exp->m_pElementList<exp->m_NumElements-1) is_cursor_at_end=0;
				}
			}
		
			ts=exp->m_pPaternalExpression->m_pElementList+exp->m_pPaternalElement->GetPaternalPosition();
			if (ts->IsSelected) is_pater_object_selected=1;
			
			int Selection=exp->m_Selection;
			int IsKeyboardEntry=exp->m_IsKeyboardEntry;
			int NumElements=exp->m_NumElements;
			int KeyboardCursorPos=exp->m_KeyboardCursorPos;
			char ParenthesesFlags=exp->m_ParenthesesFlags;
			tElementStruct *pElementList=exp->m_pElementList;

			if ((is_pater_object_selected==0) && (ts) && 
				((!exp->m_DrawParentheses) || (ParenthesesFlags&0x18)) && (ts->pElementObject->Expression1==((CObject*)exp)))
			{
				if (((ts->Type==3) && ((Selection==1) || ((IsKeyboardEntry==1) && (KeyboardCursorPos==0)))) ||
					((ts->Type==3) && (NumElements<=2) && (pElementList->Type==6) && ((Selection==NumElements+1) || ((IsKeyboardEntry==NumElements) && ((pElementList+IsKeyboardEntry-1)->pElementObject->Data1[0]==0)))) ||
					((ts->Type==6) && (ts->pElementObject->IsDifferential(1)==0) && ((Selection==NumElements+1) || (is_cursor_at_end))) ||
					((ts->Type==4) && (ts->pElementObject->Data1[0]=='d') && ((Selection==NumElements+1) || (is_cursor_at_end))) ||
					((ts->Type==7) && (ts->pElementObject->Data1[0]!='/') && ((Selection==NumElements+1) || (is_cursor_at_end))) ||
					((ts->Type==7) && (ts->pElementObject->Data1[0]=='/') && ((Selection==1) || ((IsKeyboardEntry==1) && (KeyboardCursorPos==0)))) ||
					((ts->Type==5) && (ParenthesesFlags&0x10) && ((Selection==NumElements+1) || (is_cursor_at_end))) ||
					((ts->Type==5) && (ParenthesesFlags&0x08) && ((Selection==1) || ((IsKeyboardEntry==1) && (KeyboardCursorPos==0)))) ||
					((ts->Type==9) && ((Selection==1) || ((IsKeyboardEntry==1) && (KeyboardCursorPos==0)))) )
				{
					int X1,Y1,X2,Y2;

					CPen pen(PS_DOT,1,RGB(176,176,240));
					DC->SelectObject(pen);
					
					//now we are ploting the rectangle frame around this expression
					X1=X-corr-ActualSize/10;
					Y1=Y-exp->m_OverallAbove-ActualSize/16;
					X2=X+exp->m_OverallLength-corr+((IsHighQualityRendering)?ActualSize/8:0);
					Y2=Y+exp->m_OverallBelow;
					int cc=zoom/200+1;
					X1+=cc;X2-=cc;
					Y1+=cc;Y2-=cc;
					int X3=X2;
					int X4=X2;
					int Y3=Y1;
					int Y4=Y1;

					if (ts->Type==3)
					{
						X3=min(X-ts->pElementObject->E1_posX+ts->pElementObject->E2_posX,X3);
						X4=min(X3+ts->pElementObject->E2_length,X4);
						Y3=max(Y-ts->pElementObject->E1_posY+ts->pElementObject->E2_posY+ts->pElementObject->E2_below,Y3);
						if (X2<=X4) Y4=Y3;
						X3+=cc;X4+=cc;
						Y3+=cc;Y4+=cc;
					}

					DC->MoveTo(X2,Y2);
					DC->LineTo(X1,Y2);
					DC->LineTo(X1,Y1);
					DC->LineTo(X3,Y1);
					DC->LineTo(X3,Y3);
					DC->LineTo(X4,Y3);
					DC->LineTo(X4,Y4);
					DC->LineTo(X2,Y4);
					DC->LineTo(X2,Y2);
					DC->SelectObject(GetStockObject(BLACK_PEN));
				}
			}
		}
	}


	//the main loop for painting
	char LastDecoration=m_pElementList->Decoration;
	short LastDecorationElement=0;
	tElementStruct *theElement=m_pElementList;
	for (int i=0;i<m_NumElements;i++,theElement++)
	{
		if ((((theElement->IsSelected) || ((theElement->Type==5) && (((CExpression*)theElement->pElementObject->Expression1)->m_ParenthesesSelected)))
			&& ((TouchMouseMode>1) || ((GetKeyState(VK_SHIFT)&0xFFFE) && (DisableMultitouch==0)))) || 
			(theElement->IsSelected==2))//painting blue-hue behind selected elements
		{
			int a=theElement->Above;
			int b=theElement->Below;
			int l=theElement->Length;
			if (a<ActualSize/2) a=ActualSize/2;
			if (b<ActualSize/2) b=ActualSize/2;
			l+=m_MarginX;
			DC->FillSolidRect(X+theElement->X_pos-m_MarginX/2,Y+theElement->Y_pos-a,l,a+b,RGB(240,240,255));
		}

		if (theElement->Type==0)  //empty frame
		{	
			if (!PrintRendering)
			{
				if ((m_StartAsText) && (i==0) && (m_MaxNumRows==1) && (m_MaxNumColumns==1) && 
					((this->m_pPaternalElement==NULL) || (this->m_pPaternalElement->m_Type==5)))
				{
					DC->SelectObject(GetFontFromPool(1,0,1,ActualSize));
					DC->SetTextColor((theElement->IsSelected)?BLUE_COLOR:color);
					DC->TextOut(X+theElement->X_pos,Y+theElement->Y_pos+ActualSize/3,"T");
				}
				else
				{
					int paint=1;
					if ((this->m_pPaternalElement) && 
						(this->m_pPaternalElement->m_Type==7) && 
						(theElement->IsSelected==0) && (this->m_Selection==0) &&
						(this->m_pPaternalElement->Expression2==(CObject*)this) &&
						(this->m_pElementList->Type==0) && (this->m_NumElements==1) && 
						(this->m_pPaternalElement->Expression3!=KeyboardEntryObject) &&
						(((CExpression*)this->m_pPaternalElement->Expression3)->m_pElementList->Type))
						paint=0; //for signa or integral signs, no upper limit will be desplayed if empty
					if (paint) 
					{
						POINT p[5];
						int CLR=PALE_RGB(color);
						DC->SelectObject(GetPenFromPool((ActualSize<9)?1:0,(theElement->IsSelected)?1:0,CLR));
						p[0].x=X+theElement->X_pos;						p[0].y=Y+theElement->Y_pos-3*theElement->Above/5-ActualSize/20;
						p[1].x=X+theElement->X_pos+theElement->Length;	p[1].y=p[0].y;
						p[2].x=p[1].x;									p[2].y=Y+theElement->Y_pos+7*theElement->Below/8-ActualSize/20;
						p[3].x=p[0].x;									p[3].y=p[2].y;
						p[4].x=p[0].x;									p[4].y=p[0].y;
						DC->Polyline(p,5);
					}
				}
			}
		}
		else //if (theElement->Type>0)
		{
			if ((ClipReg==NULL) ||
				((X+theElement->X_pos<=ClipReg->right+32) && (X+theElement->X_pos+theElement->Length>=ClipReg->left-32) &&
				(Y+theElement->Y_pos-theElement->Above<=ClipReg->bottom+32) && (Y+theElement->Y_pos+theElement->Below>=ClipReg->top-32)))
			{
				theElement->pElementObject->PaintExpression(DC,zoom,X+theElement->X_pos,Y+theElement->Y_pos,theElement->IsSelected,ActualSize,ClipReg,color);
			}
		}

		//go paint decoration
		if (theElement->Decoration!=LastDecoration)
		{
			if (LastDecoration)
				PaintDecoration(DC,zoom,X,Y,LastDecorationElement,i,LastDecoration,color);
			LastDecoration=theElement->Decoration;
			LastDecorationElement=i;
		}

		//paint insertion point
		if (m_Selection)
		{
			int Xpos;
			int Ypos=Y+theElement->Y_pos-ActualSize2-ActualSize/11+ActualSize/24-ActualSize2/6-1;
			int width=max((ActualSize+12)/16,2);
			int height=(5*ActualSize2+1)/2+ActualSize/16+1;
			int do_green;

			if ((i==m_NumElements-1) && (m_Selection-1==m_NumElements))
			{
				//the last one
				do_green=this->DetermineInsertionPointType(m_NumElements);
				Xpos=X+theElement->X_pos+theElement->Length+((do_green)?10*m_MarginX/8:m_MarginX*2/3)-width/2;// /*-ActualSize/20*/-((ActualSize<=20)?1:0);
				
				if (theElement->pElementObject)
				if ((theElement)->pElementObject->Data2[0]&0x02) Xpos+=(ActualSize+8)/32; //if the previous character is italic, then move insertion point position a bit to the right
				
				if (IsDrawingMode)
					DC->FillSolidRect(Xpos+width/4,Ypos,width/2,height,PALE_RGB((do_green)?GREEN_COLOR:BLUE_COLOR));
				else
					DC->FillSolidRect(Xpos,Ypos,width,height,(do_green)?GREEN_COLOR:BLUE_COLOR);
			}
			else if (m_Selection-1==i)
			{
				//all the others
				do_green=this->DetermineInsertionPointType(m_Selection-1);
				if ((i==0) || (((theElement-1)->Type==2) && ((theElement-1)->pElementObject->Data1[0]==(char)0xFF)) ||
					((theElement-1)->Type==11) || ((theElement-1)->Type==12))
				{
					//first in the line or matrix/table cell
					Xpos=X+theElement->X_pos-((do_green)?m_MarginX:m_MarginX*2/3)-width/2;
					if (Xpos<X-width/2) Xpos=X-width/2;
				}
				else if ((theElement->Type==11) || (theElement->Type==12) || ((theElement->Type==2) && (theElement->pElementObject->Data1[0]==(char)0xFF)))
				{
					//the last in the line or matrix/table cell
					Xpos=X+(theElement-1)->X_pos+(theElement-1)->Length+((do_green)?10*m_MarginX/8:m_MarginX*2/3)-width/2;
				}
				else
				{
					int delta=width/2;//ActualSize/20;
					Xpos=X+(theElement->X_pos+(theElement-1)->X_pos+(theElement-1)->Length)/2-width/2;
					if ((theElement-1)->pElementObject->Data2[0]&0x02) Xpos+=(ActualSize+8)/32; //if the previous character is italic, then move insertion point position a bit to the right
				}
				if (IsDrawingMode)
					DC->FillSolidRect(Xpos+width/4,Ypos,width/2,height,PALE_RGB((do_green)?GREEN_COLOR:BLUE_COLOR));
				else
					DC->FillSolidRect(Xpos,Ypos,width,height,(do_green)?GREEN_COLOR:BLUE_COLOR);
			}
		}

		//paint cursor (the blinking cursor used in keyboard entry mode)
		if (m_IsKeyboardEntry-1==i)
		{
			int CPos=0;
			if (theElement->pElementObject) CPos=theElement->pElementObject->Data3[m_KeyboardCursorPos];
			if (((m_KeyboardCursorPos>0) && (theElement->pElementObject->Data2[m_KeyboardCursorPos-1]&0x02)) ||
				((m_KeyboardCursorPos==0) && (m_IsKeyboardEntry>1) && ((theElement-1)->pElementObject->Data2[0]&0x02))) 
				CPos+=(ActualSize+8)/32;
			
			if (theElement->Type==6)
			{
				CSize cz;
				cz=DC->GetTextExtent(theElement->pElementObject->Data1,m_KeyboardCursorPos);
				CPos=cz.cx;
			}
			int do_green;
			if (theElement->Type==1)
				do_green=theElement->pElementObject->m_Text;
			else
				do_green=this->DetermineInsertionPointType(m_IsKeyboardEntry);

			if ((do_green!=TextControlboxMode) && ((IsDrawingMode==0) || (TextControlboxMode!=-1)))
			{
				//repainting the text controlbox
				if (TextControlboxMode==-1) {Toolbox->AdjustPosition();Toolbox->InvalidateRect(NULL,0);} else Toolbox->PaintTextcontrolbox(NULL);
				TextControlboxMode=do_green;
			}

			int hhh=2*ActualSize2+2+ActualSize/10;
			int pY=Y+theElement->Y_pos-ActualSize2-1-ActualSize/10;
			if (IsHighQualityRendering) {hhh+=ActualSize/12;}
			int www=max(2*ActualSize/5,1)-ActualSize/32;
			int pX=X+theElement->X_pos+CPos-2*ActualSize/10+ActualSize/32;
			if ((theElement->Type==1) && (theElement->pElementObject->Data1[0]==0) && (!IsWindowOutOfFocus))
			{
				//adds some hue (blue/green) behind the blinking cursor when the variable is empty
				CDC tempdc;
				tempdc.CreateCompatibleDC(DC);
				tempdc.SelectObject(cursor_bitmap);
				if (cursor_bitmap==NULL)
				{
					cursor_bitmap=new CBitmap();
					cursor_bitmap->CreateCompatibleBitmap(DC,130,260);
				}
				tempdc.SelectObject(cursor_bitmap);
				
				tempdc.FillSolidRect(0,0,www,hhh,do_green?RGB(228,255,228):RGB(240,240,255));
				tempdc.TransparentBlt(0,0,www,hhh,DC,pX,pY,www,hhh,RGB(255,255,255));
				DC->BitBlt(pX,pY, www,hhh,&tempdc,0,0,SRCCOPY);
			}

			if ((CursorBlinkState) || (IsWindowOutOfFocus))
			{
				int cw=max((ActualSize+12)/16,2);
				int clr=(do_green)?(GREEN_COLOR):(BLUE_COLOR);
				if (IsWindowOutOfFocus) clr=PALE_RGB(PALE_RGB(clr));
				DC->FillSolidRect(X+theElement->X_pos+CPos-cw/2,
								  pY,
								  cw,
								  hhh,clr);
			}
			if ((IsTextContained(i,1)) && (m_MaxNumColumns==1) && (m_MaxNumRows==1))
			{
				//painting a small sign below the cursor to inform the user that the Enter key would wrap the line
				DWORD clr=PALE_RGB((do_green)?(GREEN_COLOR):(BLUE_COLOR));
				int width=max(ActualSize/24,1);
				int ttt=0;if (IsHighQualityRendering) ttt=ActualSize/8;
				DC->FillSolidRect(X+theElement->X_pos+CPos+ActualSize2/4,
						  Y+theElement->Y_pos+ActualSize2+ActualSize2/6+ttt,
						  width,ActualSize2/3,clr);
				DC->FillSolidRect(X+theElement->X_pos+CPos+ActualSize2/4-ActualSize2/2,
						  Y+theElement->Y_pos+ActualSize2+ActualSize2/6+ActualSize2/3+ttt,
						  ActualSize2/2+max(ActualSize/24,1),width,clr);
			}

			if ((m_NumElements==1)&&(m_pElementList->Type==1)&&(m_pElementList->pElementObject->Data1[0]==0)&&
				(m_pPaternalExpression==NULL) && ((this->m_ParenthesesFlags&0x81)==0))
			{
				//paint frame around blinking cursor when the expression is empty
				DWORD clr=(do_green)?RGB(132,216,132):RGB(132,132,255);
				if (IsWindowOutOfFocus) clr=PALE_RGB(clr);

				CPen pen(PS_DOT,1,clr);
				DC->SelectObject(pen);
				int tmp=(IsHighQualityRendering)?-1:1;
				int x1=pX;
				int y1=pY+hhh-1;
				int x2=pX+www-1;
				int y2=pY;

				DC->MoveTo(x1,y1);
				DC->LineTo(x2,y1);
				DC->MoveTo(x2,y2);
				DC->LineTo(x2,y1);
				DC->MoveTo(x2,y2);
				DC->LineTo(x1,y2);
				DC->LineTo(x1,y1);
				DC->SetPixel(x1,y1,clr);
				DC->SetPixel(x1,y2,clr);
				DC->SetPixel(x2,y1,clr);
				DC->SetPixel(x2,y2,clr);
			}
		}
	}

	//go paint decoration
	if (LastDecoration)
	{
		PaintDecoration(DC,zoom,X,Y,LastDecorationElement,m_NumElements,LastDecoration,color);
	}

	//matrix insertion points 
	//also drawing table border lines
	if ((m_MatrixColumns) && (m_MatrixRows))
	{
		int Xpositions[90];
		int Ypositions[90];

		for (int i=0;i<=m_MaxNumColumns;i++)
		{
			int xx;
			if (i<m_MaxNumColumns)
			{
				int lx;
				if (i>0)
					lx=m_MatrixColumns[i-1].x+m_MatrixColumns[i-1].length;
				else
					lx=m_MatrixColumns[i].x-m_MarginX;
				xx=X+(m_MatrixColumns[i].x+lx)/2; 
			}
			else 
				xx=X+m_MatrixColumns[m_MaxNumColumns-1].x+m_MatrixColumns[m_MaxNumColumns-1].length+m_MarginX/2;
			Xpositions[i]=xx-ActualSize/32;
		}
		
		for (int i=0;i<=m_MaxNumRows;i++)
		{
			int yy;
			if (i<m_MaxNumRows) 
			{
				int ly;
				if (i==0)
					ly=m_MatrixRows[i].y-m_MatrixRows[i].above-m_MarginY;
				else
					ly=m_MatrixRows[i-1].y+m_MatrixRows[i-1].below;
				yy=Y+(m_MatrixRows[i].y-m_MatrixRows[i].above+ly)/2; 
			}
			else 
				yy=Y+m_MatrixRows[m_MaxNumRows-1].y+m_MatrixRows[m_MaxNumRows-1].below+m_MarginY/2;
			Ypositions[i]=yy-ActualSize/32;
		}

		for (int i=0;i<=m_MaxNumColumns;i++)
			for (int j=0;j<=m_MaxNumRows;j++)
			{
				int xx=Xpositions[i];
				int yy=Ypositions[j];
				int lenx=(i<m_MaxNumColumns)?Xpositions[i+1]-xx:0;
				int leny=(j<m_MaxNumRows)?Ypositions[j+1]-yy:0;
				int alternate_data=0;
				if ((this->m_IsPointerHover) ||(this==(CExpression*)KeyboardEntryObject))
				{
					DC->FillSolidRect(xx,yy,lenx,max(1,ActualSize/24),RGB(240,240,240)); //painting tiny lines
					DC->FillSolidRect(xx,yy,max(1,ActualSize/24),leny,RGB(240,240,240));
				}

				if ((i<m_MaxNumColumns) || (j<m_MaxNumRows))
				{
					tCellAttributes attrib;
					char horiz=0,vert=0;
					if ((i<m_MaxNumColumns) && (j<m_MaxNumRows))
					{
						if (this->GetCellAttributes(j,i,&attrib))
						{
							horiz=*attrib.top_border;
							vert=*attrib.left_border;
						}
					}
					else if (i==m_MaxNumColumns)
					{
						if (this->GetCellAttributes(j,i-1,&attrib))
						{
							horiz=*attrib.top_border;
							vert=*attrib.right_border;
						}
					}
					else if (j==m_MaxNumRows)
					{
						if (this->GetCellAttributes(j-1,i,&attrib))
						{
							horiz=*attrib.bottom_border;
							vert=*attrib.left_border;
						}
					}
					
					if (horiz=='-') DC->FillSolidRect(xx,yy,lenx,max(1,ActualSize/24),RGB(0,0,0));
					if (horiz=='=') {DC->FillSolidRect(xx,yy-(ActualSize+8)/24,lenx,max(1,ActualSize/24),RGB(0,0,0));DC->FillSolidRect(xx,yy+(ActualSize+8)/24,lenx,max(1,ActualSize/24),RGB(0,0,0));}
					if (vert=='-') DC->FillSolidRect(xx,yy,max(1,ActualSize/24),leny,RGB(0,0,0));
					if (vert=='=') {DC->FillSolidRect(xx-(ActualSize+8)/24,yy,max(1,ActualSize/24),leny,RGB(0,0,0));DC->FillSolidRect(xx+(ActualSize+8)/24,yy,max(1,ActualSize/24),leny,RGB(0,0,0));}
				}
				
				//painting the selected line in blue color
				if ((m_IsRowInsertion) && (m_RowSelection==j))
					DC->FillSolidRect(xx,yy,lenx,max(1,ActualSize/24),BLUE_COLOR);
				if ((m_IsColumnInsertion) && (m_ColumnSelection==i))
					DC->FillSolidRect(xx,yy,max(1,ActualSize/24),leny,BLUE_COLOR);
			}
		if (m_IsMatrixElementSelected)
		{
			//painting light-blue frame around a matrix cell when the whole cell is selected
			int row=0,column=0;
			int painted=0;
			int lw=max(1,ActualSize/24);
			for (int i=0;i<m_NumElements;i++)
			{
				tElementStruct *ts=m_pElementList+i;
				if (ts->Type==11) {painted=0;column++;}
				else if (ts->Type==12) {painted=0;column=0;row++;}
				else if (ts->IsSelected)
				{
					if (!painted)
					{
						int width=max(1,(ActualSize+12)/16);
						DC->FillSolidRect(Xpositions[column]+lw,Ypositions[row]+lw,Xpositions[column+1]-Xpositions[column]-lw,width,SHADOW_BLUE_COLOR2);
						DC->FillSolidRect(Xpositions[column]+lw,Ypositions[row+1]-width,Xpositions[column+1]-Xpositions[column]-lw,width,SHADOW_BLUE_COLOR2);
						DC->FillSolidRect(Xpositions[column]+lw,Ypositions[row]+lw,width,Ypositions[row+1]-Ypositions[row]-lw,SHADOW_BLUE_COLOR2);
						DC->FillSolidRect(Xpositions[column+1]-width,Ypositions[row]+lw,width,Ypositions[row+1]-Ypositions[row]-lw,SHADOW_BLUE_COLOR2);
						painted=1;
					}
				}
			}
		}
	}

	//go paint parentheses if needed
	if (m_DrawParentheses)
	{
		char IsBlue=m_ParenthesesSelected;

		if ((m_ParenthesesFlags&0x04) || (m_DrawParentheses=='b'))
		{
			PaintHorizontalParentheses(DC,zoom,X,Y-m_OverallAbove,X+m_OverallLength,Y+m_OverallBelow,m_ParentheseWidth,m_DrawParentheses,m_ParenthesesFlags,IsBlue,color);
		}
		else
		{	
			int increase=0;
			if (m_OverallAbove)
			{
				increase=m_OverallLength*m_MarginX/4/(m_OverallAbove+m_OverallBelow);
				if (increase>m_MarginX) increase=m_MarginX;
			}
			if ((ShadowSelection) && (IsBlue==1) && (ActualSize>10) && (ActualSize<40))
				PaintParentheses(DC,zoom,X+1,Y-m_ParenthesesAbove-increase/5,X+m_OverallLength+1,Y+m_ParenthesesBelow+increase/2,m_ParentheseWidth,m_DrawParentheses,m_ParenthesesFlags,IsBlue,color);
			PaintParentheses(DC,zoom,X,Y-m_ParenthesesAbove-increase/5,X+m_OverallLength,Y+m_ParenthesesBelow+increase/2,m_ParentheseWidth,m_DrawParentheses,m_ParenthesesFlags,IsBlue,color);
		}
	}

	if ((m_IsVertical) && (m_pPaternalExpression==NULL))
	{
		//TODO: it doesn't want to print vertical text into doPDF (PDF creator)

		//roatating the image to vertical position
		int fast_method=1;
		int bits_per_pixel=orgDC->GetDeviceCaps(BITSPIXEL);
		if (bits_per_pixel%8) fast_method=0;
		int bytes_per_pixel=bits_per_pixel/8;
		if (PrintRendering) fast_method=0;
		unsigned char *bits=NULL;
		if (fast_method)
				bits=(unsigned char*)HeapAlloc(ProcessHeap,0,(m_OverallAbove+m_OverallBelow)*m_OverallLength*bytes_per_pixel);
		if (!bits)
			fast_method=0;
		else
			if (!cb->GetBitmapBits((m_OverallAbove+m_OverallBelow)*m_OverallLength*bytes_per_pixel,bits)) 
			{
				fast_method=0;
				HeapFree(ProcessHeap,0,bits);
			}

		if (fast_method)
		{
			unsigned char *bits2=(unsigned char*)HeapAlloc(ProcessHeap,0,(m_OverallAbove+m_OverallBelow)*m_OverallLength*bytes_per_pixel);
			if (bits2)
			{
				int line_size2=bytes_per_pixel*(m_OverallAbove+m_OverallBelow);
				//if (line_size2%2) line_size2++; //ist this true - mst be rounded to 16 bits?
				int line_size=bytes_per_pixel*m_OverallLength;
				//if (line_size%2) line_size++; //is this true - must be rounded to 16 bits?

				unsigned char *src=bits;
				unsigned char *dst=bits2+(m_OverallLength-1)*line_size2;

				if (bytes_per_pixel==4)
					for (int i=0;i<m_OverallAbove+m_OverallBelow;i++,dst+=m_OverallLength*line_size2+bytes_per_pixel,src=bits+i*line_size)
						for (int j=0;j<m_OverallLength;j++,src+=bytes_per_pixel,dst-=line_size2)
							*((unsigned int*)dst)=*((unsigned int*)src);
				if (bytes_per_pixel==3)
					for (int i=0;i<m_OverallAbove+m_OverallBelow;i++,dst+=m_OverallLength*line_size2+bytes_per_pixel,src=bits+i*line_size)
						for (int j=0;j<m_OverallLength;j++,src+=bytes_per_pixel,dst-=line_size2)
						{
							*((unsigned short*)dst)=*((unsigned short*)src);
							*((unsigned char*)(dst+2))=*((unsigned char*)(src+2));
						}
				if (bytes_per_pixel==2)
					for (int i=0;i<m_OverallAbove+m_OverallBelow;i++,dst+=m_OverallLength*line_size2+bytes_per_pixel,src=bits+i*line_size)
						for (int j=0;j<m_OverallLength;j++,src+=bytes_per_pixel,dst-=line_size2)
							*((unsigned short*)dst)=*((unsigned short*)src);
				if (bytes_per_pixel==1)
					for (int i=0;i<m_OverallAbove+m_OverallBelow;i++,dst+=m_OverallLength*line_size2+bytes_per_pixel,src=bits+i*line_size)
						for (int j=0;j<m_OverallLength;j++,src+=bytes_per_pixel,dst-=line_size2)
							*((unsigned char*)dst)=*((unsigned char*)src);

				
				CBitmap cb2;
				cb2.CreateCompatibleBitmap(orgDC,(m_OverallAbove+m_OverallBelow),m_OverallLength);
				cb2.SetBitmapBits((m_OverallAbove+m_OverallBelow)*m_OverallLength*bytes_per_pixel,bits2);
				DC->SelectObject(cb2);
				orgDC->TransparentBlt(orgX,orgY-m_OverallLength/2,m_OverallAbove+m_OverallBelow,m_OverallLength,DC,0,0,m_OverallAbove+m_OverallBelow,m_OverallLength,RGB(255,255,255));
				HeapFree(ProcessHeap,0,bits2);
			}
			HeapFree(ProcessHeap,0,bits);
		}
		else
		{
			//unsuported bits-per-pixel resolution - do it the slow way - pixel by pixel
			orgY-=m_OverallLength/2;
			for (int i=0;i<m_OverallAbove+m_OverallBelow;i++)
			{
				int tmp=m_OverallLength-1;
				for (int j=0;j<m_OverallLength;j++)
				{
					int clr=DC->GetPixel(tmp,i);
					if (clr!=RGB(255,255,255))
					{
						orgDC->SetPixelV(orgX,orgY+j,clr);
					}
					tmp--;
				}
				orgX++;
			}
		}
		DC->DeleteDC();
		delete DC;
		cb->DeleteObject();
		delete cb;
	}
	}
	catch(...)
	{
		FatalErrorHandling();
	}
}

short CExpression::GetActualFontSize(short zoom)
{
	return (short)(((int)zoom*(int)m_FontSize*20+3000)/10000);
}


int CExpression::PaintParentheses(CDC * DC, short zoom, short X1, short Y1, short X2, short Y2, short ParentheseWidth, char Type,short data,char IsBlue,int color)
{
	short tmp;
	int HQPenWidth=0;
	int PenWidth;
	int ActualSize=GetActualFontSize(zoom);

	if (X2<X1) {tmp=X2;X2=X1;X1=tmp;}
	if (Y2<Y1) {tmp=Y2;Y2=Y1;Y1=tmp;}

	char HQR=IsHighQualityRendering;
	if (HQR)
	{
		if ((Type&0x80)==0) ParentheseWidth=11*ParentheseWidth/8; //when expression is edited in presentation mode
		Y1+=ActualSize/20;
		Y2-=ActualSize/20;
		ParentheseWidth-=2*(ActualSize/20);

		Type=Type&0x7F;
		if (Type=='{') HQPenWidth=7*ParentheseWidth/8;
		else if ((Type=='/') || (Type=='<')) HQPenWidth=ParentheseWidth*5/8;
		else HQPenWidth=9*ParentheseWidth/8;
		if (HQPenWidth<1) HQPenWidth=1;
	}

	Type=Type&0x7F;
	if (Type=='{') PenWidth=7*ParentheseWidth/24;
	else if ((Type=='/') || (Type=='<')) PenWidth=ParentheseWidth*5/24;
	else PenWidth=ParentheseWidth/3;
	if (PenWidth<1) PenWidth=1;

	int PaintLeft=1;
	int PaintRight=1;
	if (data&0x08) PaintLeft=0;
	if (data&0x10) PaintRight=0;

	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;
	HPEN pen=GetPenFromPool(PenWidth,IsBlue,color);
	DC->SelectObject(pen);

	if ((Type=='(') || (Type=='l') || (Type=='r'))
	{
		ActualSize=3*ActualSize/2;
		if ((Y2-Y1)<ActualSize) ActualSize=(Y2-Y1);

		if ((!HQR) || ((ActualSize<18) && (IsHalftoneRendering==0)))
		{
			int half=ActualSize/2;
			int bit=ParentheseWidth/4;
			X1+=bit;
			X2-=bit;
			ParentheseWidth-=bit;
			if ((PaintLeft) && ((Type=='(') || (Type=='l')))
			{
				if (Y2-Y1==ActualSize)
					DC->Arc(X1,Y1,X1+ParentheseWidth*2,Y2,  X1+ParentheseWidth,Y1,X1+ParentheseWidth+1,Y2+100);
				else
				{
					DC->Arc(X1,Y1,X1+ParentheseWidth*2,Y1+ActualSize,  X1+ParentheseWidth,Y1,X1,Y1+half);
					DC->MoveTo(X1,Y1+half-1);
					DC->LineTo(X1,Y2-half);
					DC->Arc(X1,Y2-ActualSize,X1+ParentheseWidth*2,Y2,  X1,Y2-half,X1+ParentheseWidth+1,Y2+100);
				}
			}
			if ((PaintRight)&& ((Type=='(') || (Type=='r')))
			{
				if (Y2-Y1==ActualSize)
					DC->Arc(X2-ParentheseWidth*2,Y1,X2,Y2,X2-ParentheseWidth+1,Y2+100,X2-ParentheseWidth-1,Y1-100);
				else
				{
					DC->Arc(X2-ParentheseWidth*2,Y1,X2,Y1+ActualSize,  X2,Y1+half,X2-ParentheseWidth-1,Y1-100);
					DC->MoveTo(X2,Y1+half-1);
					DC->LineTo(X2,Y2-half);
					DC->Arc(X2-ParentheseWidth*2,Y2-ActualSize,X2,Y2,  X2-ParentheseWidth+1,Y2+100,X2,Y2-half);
				}
			}
			if (Type=='(') return 1;
		}
		else
		{
			int H=(Y2-Y1)*4;
			int i;
			mf->StartMyPainting(DC,ParentheseWidth*4,0,(Y2-Y1)*4,color);
			for (i=0;i<=HQPenWidth;i++)
			{
				mf->MyArc(DC,i,0,ParentheseWidth*8-i,ActualSize*4,  ParentheseWidth*4,0,0,ActualSize*2,IsBlue);
				mf->MyMoveTo(DC,i,ActualSize*2-1);
				mf->MyLineTo(DC,i,H-ActualSize*2,IsBlue);
				mf->MyArc(DC,i,H-ActualSize*4,ParentheseWidth*8-i,H, 0,H-ActualSize*2,ParentheseWidth*4,H,IsBlue);
			}
			mf->MySetPixel(DC,ParentheseWidth*4,H,IsBlue);
			if ((PaintLeft) && ((Type=='(') || (Type=='l'))) mf->EndMyPainting(DC,X1,Y1);
			if ((PaintRight) && ((Type=='(') || (Type=='r'))) mf->EndMyPainting(DC,X2-ParentheseWidth,Y1,0,1);
		}
	}

	if ((Type=='[') || (Type=='l') || (Type=='r') || (Type=='c') || (Type=='f'))
	{
		if ((PaintLeft) && ((Type=='[') || (Type=='r') || (Type=='c') || (Type=='f')))
		{				
			if (Type!='f') DC->FillSolidRect(X1+PenWidth/2,Y1,ParentheseWidth,PenWidth,(IsBlue)?BLUE_COLOR:color);
			DC->FillSolidRect(X1+PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
			if (Type!='c') DC->FillSolidRect(X1+PenWidth/2,Y2-PenWidth,ParentheseWidth,PenWidth,(IsBlue)?BLUE_COLOR:color);
		}
		if ((PaintRight) && ((Type=='[') || (Type=='l') || (Type=='c') || (Type=='f')))
		{
			if (Type!='f') DC->FillSolidRect(X2-ParentheseWidth-PenWidth/2,Y1,ParentheseWidth,PenWidth,(IsBlue)?BLUE_COLOR:color);
			DC->FillSolidRect(X2-3*PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
			if (Type!='c') DC->FillSolidRect(X2-ParentheseWidth-PenWidth/2,Y2-PenWidth,ParentheseWidth,PenWidth,(IsBlue)?BLUE_COLOR:color);
		}
		return 1;
	}

	if (Type=='{')
	{
		short height2=(Y2-Y1)/2;
		ActualSize=2*ActualSize;
		if (3*(Y2-Y1)/2<ActualSize) ActualSize=3*(Y2-Y1)/2;
		if (!HQR)
		{
			if (height2<ParentheseWidth*2)
			{
				int bit=2*ParentheseWidth/7;
				ParentheseWidth-=bit;
				X2-=bit;
				X1+=bit;
			}
			if (PaintLeft)
			{
				DC->MoveTo(X1+ParentheseWidth,Y1);
				DC->LineTo(X1+ParentheseWidth/2,Y1+ParentheseWidth/2);
				DC->LineTo(X1+ParentheseWidth/2,Y1+height2-ParentheseWidth/2);
				DC->LineTo(X1,Y1+height2);
				DC->LineTo(X1+ParentheseWidth/2,Y2-height2+ParentheseWidth/2);
				DC->LineTo(X1+ParentheseWidth/2,Y2-ParentheseWidth/2);
				DC->LineTo(X1+ParentheseWidth,Y2);
			}
			if (PaintRight)
			{
				DC->MoveTo(X2-ParentheseWidth,Y1);
				DC->LineTo(X2-ParentheseWidth/2,Y1+ParentheseWidth/2);
				DC->LineTo(X2-ParentheseWidth/2,Y1+height2-ParentheseWidth/2);
				DC->LineTo(X2,Y1+height2);
				DC->LineTo(X2-ParentheseWidth/2,Y2-height2+ParentheseWidth/2);
				DC->LineTo(X2-ParentheseWidth/2,Y2-ParentheseWidth/2);
				DC->LineTo(X2-ParentheseWidth,Y2);
			}
			return 1;
		}
		else
		{
			int H=(Y2-Y1)*4;
			int i;
			
			mf->StartMyPainting(DC,ParentheseWidth*4,0,(Y2-Y1)*4,color);
			for (i=0;i<=HQPenWidth;i++)
			{
				mf->MyArc(DC,ParentheseWidth*2-HQPenWidth/2+i, 0,
						3*ParentheseWidth*2+HQPenWidth/2-i,4*ActualSize/3,  
						ParentheseWidth*4,0,
						ParentheseWidth*2,4*ActualSize/6,IsBlue);
				mf->MyMoveTo(DC,ParentheseWidth*2-HQPenWidth/2+i,4*ActualSize/6-1);
				mf->MyLineTo(DC,ParentheseWidth*2-HQPenWidth/2+i,H/2-4*ActualSize/6,IsBlue);
				mf->MyArc(DC,-ParentheseWidth*2+HQPenWidth/2-i,H/2-4*ActualSize/3,
						ParentheseWidth*2-HQPenWidth/2+i,H/2,  
						0, H/2,
						ParentheseWidth*2, H/2-4*ActualSize/6,IsBlue);

				mf->MyArc(DC,-ParentheseWidth*2+HQPenWidth/2-i,H/2,
						ParentheseWidth*2-HQPenWidth/2+i,H/2+4*ActualSize/3,  
						ParentheseWidth*2, H/2+4*ActualSize/6,
						0, H/2,IsBlue);
				mf->MyMoveTo(DC,ParentheseWidth*2-HQPenWidth/2+i,H/2+4*ActualSize/6);
				mf->MyLineTo(DC,ParentheseWidth*2-HQPenWidth/2+i,H-4*ActualSize/6,IsBlue);
				mf->MyArc(DC,ParentheseWidth*2-HQPenWidth/2+i,H-4*ActualSize/3, 
						3*ParentheseWidth*2+HQPenWidth/2-i,H,
						ParentheseWidth*2, H-4*ActualSize/6,
						ParentheseWidth*4, H, IsBlue);
			}
			if (PaintLeft) mf->EndMyPainting(DC,X1,Y1);
			if (PaintRight) mf->EndMyPainting(DC,X2-ParentheseWidth,Y1,0,1);
		}
	}

	if ((Type=='|') || (Type=='a') || (Type=='k'))
	{
		if ((PaintLeft) && (Type!='a'))
		{
			DC->FillSolidRect(X1+ParentheseWidth/2-PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
		}
		if ((PaintRight) && (Type!='k'))
		{
			DC->FillSolidRect(X2-ParentheseWidth/2-PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
		}
		if (Type=='|') return 1;
	}
	
	if (Type=='/')
	{
		if (PaintLeft)
		{
			DC->MoveTo(X1+9*ParentheseWidth/10,Y1);
			DC->LineTo(X1,Y2);
		}
		if (PaintRight)
		{
			DC->MoveTo(X2,Y1);
			DC->LineTo(X2-9*ParentheseWidth/10,Y2);
		}
		return 1;
	}

	if (Type=='\\')  //absolute value || ||
	{
		if (PaintLeft)
		{
			DC->FillSolidRect(X1+ParentheseWidth/4-PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
			DC->FillSolidRect(X1+3*ParentheseWidth/4-PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
		}
		if (PaintRight)
		{
			DC->FillSolidRect(X2-ParentheseWidth/4-PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
			DC->FillSolidRect(X2-3*ParentheseWidth/4-PenWidth/2,Y1,PenWidth,Y2-Y1,(IsBlue)?BLUE_COLOR:color);
		}
		return 1;
	}

	if ((Type=='<')  || (Type=='a') || (Type=='k'))
	{
		if ((PaintLeft) && (Type!='k'))
		{
			DC->MoveTo(X1+9*ParentheseWidth/10,Y1);
			DC->LineTo(X1+ParentheseWidth/10,(Y1+Y2)/2);
			DC->LineTo(X1+9*ParentheseWidth/10,Y2);
		}

		if ((PaintRight) && (Type!='a'))
		{
			DC->MoveTo(X2-9*ParentheseWidth/10,Y1);
			DC->LineTo(X2-ParentheseWidth/10,(Y1+Y2)/2);
			DC->LineTo(X2-9*ParentheseWidth/10,Y2);
		}
		return 1;
	}

	if ((Type=='b') || ((Type=='T') && (IsBlue)))  //box or text
	{
		int tt=0;if (IsHighQualityRendering) tt=5*this->m_MarginY/2;

		if (PaintLeft)
		{
			DC->MoveTo(X1+ParentheseWidth/10,Y2-ParentheseWidth/10+3*tt/2);
			DC->LineTo(X1+ParentheseWidth/10,Y1+ParentheseWidth/10-tt);
			DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth/10-tt);
		}
		if (PaintRight)
		{
			DC->MoveTo(X1+ParentheseWidth/10,Y2-ParentheseWidth/10+3*tt/2);
			DC->LineTo(X2-ParentheseWidth/10,Y2-ParentheseWidth/10+3*tt/2);
			DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth/10-tt);
		}
		return 1;
	}

	if (Type=='x')  //crossed
	{
		if (PaintLeft)
		{
			DC->MoveTo(X1+ParentheseWidth/10,Y1+ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y2-ParentheseWidth/10);
		}
		if (PaintRight)
		{
			DC->MoveTo(X1+ParentheseWidth/10,Y2-ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth/10);
		}
		return 1;
	}
	return 0;
}

void CExpression::SelectElement(char Select,int position)
{
	//Select=0 - deselection
	//Select=1 - standard selection
	//Select=2 - keyboard selection (when element is selected by SHIFT+arrow key)

	tElementStruct *theElement=m_pElementList+position;
	theElement->IsSelected=Select;
	if (theElement->Type>0)
	{
		CElement *object=(CElement*)theElement->pElementObject;
		if (object)
		{
			if (Select)
			{
				if (object->Expression1) ((CExpression*)(object->Expression1))->SelectExpression(1);
				if (object->Expression2) ((CExpression*)(object->Expression2))->SelectExpression(1);
				if (object->Expression3) ((CExpression*)(object->Expression3))->SelectExpression(1);
			}
			else
			{
				if (object->Expression1) ((CExpression*)(object->Expression1))->DeselectExpression();
				if (object->Expression2) ((CExpression*)(object->Expression2))->DeselectExpression();
				if (object->Expression3) ((CExpression*)(object->Expression3))->DeselectExpression();

			}
		}
	}
}

//this one selects/deseclects the whole expression (together with its subelements)
void CExpression::SelectExpression(char Select)
{
	//Select=0 - deselection
	//Select=1 - standard selection
	//Select=2 - standard selection without parentheses
	//Select=3 - special selection (no shadowing when painted - used for ClipboardExpression)

	m_ParenthesesSelected=Select;
	if (Select==2) {m_ParenthesesSelected=0;Select=1;}
	m_Selection=(Select)?0x7FFF:0;
	if (Select==0)
		m_IsPointerHover=0;
	m_IsRowInsertion=0;
	m_IsColumnInsertion=0;
	m_IsMatrixElementSelected=0;
	m_InternalInsertionPoint=0;

	tElementStruct *theElement=m_pElementList;
	for (int i=0;i<m_NumElements;i++,theElement++)
	{
		theElement->IsSelected=Select;
		if (theElement->Type>0) 
		{
			CElement *object=(CElement*)theElement->pElementObject;
			if (object)
			{
				if (object->Expression1) ((CExpression*)(object->Expression1))->SelectExpression(Select);
				if (object->Expression2) ((CExpression*)(object->Expression2))->SelectExpression(Select);
				if (object->Expression3) ((CExpression*)(object->Expression3))->SelectExpression(Select);
			}
		}
	}
}

//this is a redundant function - for speed
void CExpression::DeselectExpressionExceptKeyboardSelection(void)
{
	if (!KeyboardEntryObject) DeselectExpression();
	m_ParenthesesSelected=0;
	m_Selection=0;
	m_IsRowInsertion=0;
	m_IsColumnInsertion=0;
	m_IsMatrixElementSelected=0;
	m_IsPointerHover=0;

	tElementStruct *theElement=m_pElementList;
	for (int i=m_NumElements-1;i>=0;i--,theElement++)
	if ((theElement->IsSelected!=2) || /*(!KeyboardEntryObject) ||*/
		((this!=(CExpression*)KeyboardEntryObject) && (this!=((CExpression*)KeyboardEntryObject)->m_pPaternalExpression)))
	{
		theElement->IsSelected=0;
		if (theElement->Type>0)
		{
			CElement *object=(CElement*)theElement->pElementObject;
			if (object)
			{
				if (object->Expression1) ((CExpression*)(object->Expression1))->DeselectExpressionExceptKeyboardSelection();
				if (object->Expression2) ((CExpression*)(object->Expression2))->DeselectExpressionExceptKeyboardSelection();
				if (object->Expression3) ((CExpression*)(object->Expression3))->DeselectExpressionExceptKeyboardSelection();
			}
		}
	}
}

void CExpression::DeselectExpression()
{
	m_ParenthesesSelected=0;
	m_Selection=0;
	m_IsRowInsertion=0;
	m_IsColumnInsertion=0;
	m_IsMatrixElementSelected=0;
	m_IsPointerHover=0;

	tElementStruct *theElement=m_pElementList;
	for (int i=m_NumElements-1;i>=0;i--,theElement++)
	{
		theElement->IsSelected=0;
		if (theElement->Type>0)
		{
			CElement *object=(CElement*)theElement->pElementObject;
			if (object)
			{
				if (object->Expression1) ((CExpression*)(object->Expression1))->DeselectExpression();
				if (object->Expression2) ((CExpression*)(object->Expression2))->DeselectExpression();
				if (object->Expression3) ((CExpression*)(object->Expression3))->DeselectExpression();
			}
		}
	}
}

//selects and returns object the cursor is pointing at
//it work recursively (can return its subelements)
CObject* CExpression::SelectObjectAtPoint(CDC* DC, short zoom, short X, short Y,short *IsExpression, char *IsParenthese,char ForceInsertionPoints)
{
	int i;
	if ((DC==NULL) || (X<-5000) || (X>30000) || (Y<-5000) || (Y>15000) || (IsExpression==NULL)) return NULL;

	if ((m_IsVertical) && (m_pPaternalExpression==NULL)) 
	{
		short tmp=X;
		X=m_OverallLength/2-Y;
		Y=tmp-m_OverallAbove;
	}

	m_RowSelection=0;
	m_ColumnSelection=0;
	m_IsRowInsertion=0;
	m_IsColumnInsertion=0;

	if (zoom<5) zoom=5;
	if (zoom>5000) zoom=5000;
	int ActualSize=this->GetActualFontSize(zoom);

	int startItem=-1; //start item of an matrix cell
	int column=0,row=0;
	this->m_IsPointerHover=1;
	*IsParenthese=0;

	int insertion_points_only=0; //if only insertion points are to be touched (for example if ALT key is held down)
	int no_insertion_points=0; //if insertion points are not to be touched (for example if SHIFT key is held down)
	int delta=0; //used in insertion_points_only mode to allow easier selection of an insertion point)
	if (((GetKeyState(VK_MENU)&0xFFFE)) || (ForceInsertionPoints==2) || (QuickTypeUsed)) {insertion_points_only=1;delta=2*m_MarginX/3;}
	else if (((GetKeyState(VK_SHIFT)&0xFFFE)) || (TouchMouseMode)) {if (ForceInsertionPoints==0) no_insertion_points=1;}

	//check if the pointer is pointing at any element or insertion point inside the Expression
	for (i=0;i<=m_NumElements;i++)
	{
		tElementStruct *theElement=m_pElementList+i;
		if (startItem==-1) startItem=i;

		//handling matrices
		if ((!insertion_points_only) && (m_MatrixRows) && (m_MatrixColumns))
		if ((i==m_NumElements) || (theElement->Type==11) || (theElement->Type==12))
		{
			int Ybaseline=m_MatrixRows[row].y;
			int startX=m_MatrixColumns[column].x;
			int endX=m_MatrixColumns[column].x+m_MatrixColumns[column].length;
			int maxAbove=-m_MatrixRows[row].above+Ybaseline;
			int maxBelow=m_MatrixRows[row].below+Ybaseline;

			if (startItem==i) endX=startX;

			int t1=m_OverallLength-((m_DrawParentheses)?m_ParentheseWidth:0);
			int t2=((m_DrawParentheses)?m_ParentheseWidth:0);
			if ((GetKeyState(VK_SHIFT)&0xFFFE)==0)
			if (TouchMouseMode==0)
			if ( ((X>=endX) && (X<t1) && (i==m_NumElements) && (Y>m_OverallBelow-ActualSize/6)) ||
				 /*((X>=endX) && (X<t1) && (row==0) && (column==m_MaxNumColumns-1) && (Y<-m_OverallAbove+ActualSize/6)) ||*/
				 ((X<=startX) && (X>t2) && (column==0) && (row==m_MaxNumRows-1) && (Y>m_OverallBelow-ActualSize/6)) 
				 /*((X<=startX) && (X>t2) && (column==0) && (row==0) && (Y<-m_OverallAbove+ActualSize/6))*/)
			{
				//the cursor is pointing at the corner of the expression - select the whole expression
				if (ContainsBlinkingCursor()) return NULL;
				SelectExpression(2);
				m_Selection=0x7FFF;
				*IsExpression=0x7FFF;
				return (CObject*)this;
			}

			if ((m_MaxNumRows==1) && (m_MaxNumColumns==1))
			{
				maxAbove=-m_OverallAbove;
				maxBelow=m_OverallBelow;
			}

			if ((TouchMouseMode==0) && ((GetKeyState(VK_SHIFT)&0xFFFE)==0))
			if ((m_MaxNumRows>1) || (m_MaxNumColumns>1))
			{
				//check if the mouse is pointing at column insert point
				if ((Y>=maxAbove) && (Y<=maxBelow))
				{
					int lx;
					if (column>0) lx=m_MatrixColumns[column-1].x+m_MatrixColumns[column-1].length;
					else lx=m_MatrixColumns[column].x-m_MarginX;

					if ((X<m_MatrixColumns[column].x) && (X>lx))
					{
						m_RowSelection=row;
						m_ColumnSelection=column;
						m_IsColumnInsertion=1;
						m_IsRowInsertion=0;
						m_Selection=0;
						*IsExpression=i+1;
						return(CObject*)this;
					}
					if (column==m_MaxNumColumns-1)
					if ((X>m_MatrixColumns[column].x+m_MatrixColumns[column].length) && (X<m_MatrixColumns[column].x+m_MatrixColumns[column].length+m_MarginX))
					{
						m_RowSelection=row;
						m_ColumnSelection=column+1;
						m_IsColumnInsertion=1;
						m_IsRowInsertion=0;
						m_Selection=0;
						*IsExpression=i+1;
						return(CObject*)this;
					}
				}

				//check if the mouse is pointing at row insert point
				if ((X>m_MatrixColumns[column].x) && (X<m_MatrixColumns[column].x+m_MatrixColumns[column].length))
				{
					int ly;
					if (row==m_MaxNumRows-1) ly=maxBelow+m_MarginY;
					else ly=m_MatrixRows[row+1].y-m_MatrixRows[row+1].above;

					if ((Y>maxBelow+m_MarginY/4) && (Y<=ly-m_MarginY/4)) 
					{
						m_RowSelection=row+1;
						m_ColumnSelection=column;
						m_IsColumnInsertion=0;
						m_IsRowInsertion=1;
						m_Selection=0;
						*IsExpression=i+1;
						return(CObject*)this;
					}

					if (row==0) ly=maxAbove-m_MarginY;
					else ly=m_MatrixRows[row-1].y+m_MatrixRows[row-1].below;

					if ((Y<maxAbove-m_MarginY/4) && (Y>=ly+m_MarginY/4)) 
					{
						m_RowSelection=row;
						m_ColumnSelection=column;
						m_IsColumnInsertion=0;
						m_IsRowInsertion=1;
						m_Selection=0;
						*IsExpression=i+1;
						return(CObject*)this;
					}
				}
			}

			if ((X>startX) && (X<endX))
			{
				if (((Y<=maxBelow+m_MarginY/4) && (Y>=maxBelow-ActualSize/8)) ||
					((Y>=maxAbove-m_MarginY/4) && (Y<=maxAbove+ActualSize/8)))
				{
					if ((startItem==0) && (i==m_NumElements))
					{
						//all items are being selected
						if (ContainsBlinkingCursor()) return NULL;
						SelectExpression(2); //no parentheses included in selection
						m_Selection=0x7FFF;
						*IsExpression=0x7FFF;
						return (CObject*)this;
					}
					//select the whole matrix element
					SelectMatrixElement(row,column);
					if ((ClipboardExpression) && ((ClipboardExpression->m_MaxNumColumns>1) || (ClipboardExpression->m_MaxNumRows>1)))
					{
						//there is some matrix/vector in clipboard expression that we are carying with the cursor
						//adjust selection according to its size so that cell-replacement can be done
						int ii,jj;
						for (ii=row;ii<row+ClipboardExpression->m_MaxNumRows;ii++)
							for (jj=column;jj<column+ClipboardExpression->m_MaxNumColumns;jj++)
								SelectMatrixElement(ii,jj);
					}
					m_Selection=0;
					*IsExpression=-startItem;
					return (CObject*)this;
				}
			}
			column++;
			startItem=-1;
			if (theElement->Type==12) {column=0;row++;}
		}
		
		//checking if particular elements are pointed-at (touched)
		if (i<m_NumElements)
		{
			int is_ok=0;
			if ((Y>=theElement->Y_pos-theElement->Above-ActualSize/20) && (Y<=theElement->Y_pos+theElement->Below+ActualSize/20))
			{
				int ldelta=delta;
				int rdelta=delta;
				if (theElement->Type==3) rdelta=ActualSize/8;
				if ((Y>theElement->Y_pos+theElement->Below-ActualSize/2) || (Y<=theElement->Y_pos-theElement->Above+ActualSize/2))
				{
					if (theElement->Type==3) rdelta=0;
					if (theElement->Type==7) {if (theElement->pElementObject->Data1[0]!='/') ldelta=0; else rdelta=0;}
					if (theElement->Type==9) rdelta=0;
					if (theElement->Type==4) rdelta=0;
				}
				if ((X>theElement->X_pos+ldelta) && (X<theElement->X_pos+theElement->Length-rdelta)) 
				{
					is_ok=1;
				}

				if (is_ok==0)
				{
					//in some cases (variable with indexes or exponents) allow non-rectangle regions (so that index/exponent end insertion point can be touched)
					if ((theElement->Type==3) && (theElement->pElementObject->Expression1))
					{
						CExpression* base=(CExpression*)theElement->pElementObject->Expression1;
						int has_index=0;
						if (base->m_DrawParentheses==0)
						{
							if (((base->m_pElementList+base->m_NumElements-1)->Type==1) && ((base->m_pElementList+base->m_NumElements-1)->pElementObject->Expression1)) 
								has_index=1;
							if (((base->m_pElementList+base->m_NumElements-1)->Type==5) && ((base->m_pElementList+base->m_NumElements-1)->pElementObject->Expression2)) 
								has_index=1;
						}
						if ((Y<=theElement->Y_pos-2*this->m_MarginY/3) && (X>theElement->X_pos+ldelta) && (X<theElement->X_pos+theElement->Length+2*this->m_MarginX/3-rdelta))
							is_ok=1;
						if (has_index)
						if ((Y>theElement->Y_pos+this->m_MarginY) && (X>theElement->X_pos+ldelta) && (X<theElement->X_pos+theElement->Length+2*this->m_MarginX/3-rdelta))
							is_ok=1;
					}
					if ((theElement->Type==1) && (theElement->pElementObject->Expression1))
					{
						//special handling for indexes
						if ((Y>theElement->Y_pos+this->m_MarginY) && (X>theElement->X_pos+delta) && (X<theElement->X_pos+theElement->Length+2*this->m_MarginX/3))
							is_ok=1;
					}
				}
			}

			if (is_ok)
			{
				if ((theElement->Type==0) || (theElement->Type==11) || (theElement->Type==12))
				{
					//this is only an empty frame (placeholder)
					//we will pretend exactly as if it is clicked at insertion point just before this element
					m_Selection=i+1;
					*IsExpression=i+1;
					return (CObject*)this;
				}
				
				//an element is being pointed at - we will call its 'selectAtPoint' function to recursively determine which final element is being touched
				CObject *obj=NULL;
				if (theElement->pElementObject)
				{
					obj=theElement->pElementObject->SelectAtPoint(DC,zoom,X-theElement->X_pos,Y-theElement->Y_pos,IsExpression,IsParenthese,i);
					if (obj==NULL)
					{
						m_Selection=0;
						*IsExpression=0;
						return NULL;
					}
				}
				m_Selection=0;
				if (obj==(CObject*)theElement->pElementObject)
					theElement->IsSelected=1;
				return obj;
			}
		}
	}

	SelectedTab=0;

	if (!insertion_points_only)
	{
		//check if it is pointing at the parenthese 
		if (m_DrawParentheses) 
		{
			int parenthese_width_top=0;
			int parenthese_width_bottom=0;
			int parenthese_width_right=0;
			int parenthese_width_left=0;
			char parenthese_shape=m_DrawParentheses; //this contains the shape of parenth. or 0 if no parentheses are to be displayed
			if (m_ParenthesesFlags&0x04) //if parentheses are horizontal
			{
				if ((m_ParenthesesFlags&0x08)==0)
					parenthese_width_top=m_ParentheseWidth;
				if ((m_ParenthesesFlags&0x10)==0)
					parenthese_width_bottom=m_ParentheseWidth;
			}
			else
			{
				if ((m_ParenthesesFlags&0x08)==0)
					parenthese_width_left=m_ParentheseWidth;
				if ((m_ParenthesesFlags&0x10)==0)
					parenthese_width_right=m_ParentheseWidth;
			}
			if ((parenthese_shape=='x') || (parenthese_shape=='b')) //for box or crossed
			{
				parenthese_width_top=parenthese_width_bottom=parenthese_width_left=parenthese_width_right=m_ParentheseWidth;
			}
			
			if (IsHighQualityRendering) {parenthese_width_left=max(1,3*parenthese_width_left/4);parenthese_width_right=max(1,3*parenthese_width_right/4);}
			if ((X<parenthese_width_left) || (X>m_OverallLength-parenthese_width_right) ||
				(Y<-m_OverallAbove+parenthese_width_top) || (Y>m_OverallBelow-parenthese_width_bottom))
			{
				if (ContainsBlinkingCursor()) return NULL;
				m_ParenthesesSelected=1;
				SelectExpression(1);
				m_Selection=0x7FFF;
				*IsExpression=0x7FFF;
				*IsParenthese=1;
				return (CObject*)this;
			}
		}


		//then check if it is pointing at the expression edge (selecting all elements in the Expression)
		if ((TouchMouseMode==0) && (m_MaxNumRows==1) && ((GetKeyState(VK_SHIFT)&0xFFFE)==0))
		{
			//int maxAbove=m_OverallAbove-ActualSize/6;
			int maxBelow=m_OverallBelow-ActualSize/6;
			//if (IsHighQualityRendering) {maxAbove+=ActualSize/16;maxBelow+=ActualSize/16;}
			int maxXb;
			//int minXa;
			//int maxXa;
			if (m_NumElements)
				maxXb=max((m_pElementList+m_NumElements-1)->X_pos+(m_pElementList+m_NumElements-1)->Length,ActualSize);
			/*minXa=min(m_pElementList->X_pos,this->m_OverallLength-ActualSize);
			maxXa=this->m_OverallLength;
			for (int i=0;i<m_NumElements;i++) //for multi-line expressions, the uppermost-line length is determined
			{
				tElementStruct *ts=m_pElementList+i;
				if ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF))
				{
					maxXa=ts->X_pos+ts->Length;
					break;
				}
			}*/

			if (/*((Y<=-maxAbove) && (X<=maxXa) && (X>=minXa)) ||*/ ((Y>=maxBelow) && (X<=maxXb)))
			{
				if (ContainsBlinkingCursor()) return NULL;
				SelectExpression(2);
				m_Selection=0x7FFF;
				*IsExpression=0x7FFF;
				return (CObject*)this;
			}
		}
	}


	if (no_insertion_points) return NULL;

	//Finally check if it is pointing between elements (insertion point)
	if (m_NumElements==0) 
	{
		//an empty expression (should not be) 
		m_Selection=1;
		*IsExpression=1;
		return (CObject*)this;
	}
	
	//checking the very last insertion point
	{
		tElementStruct *theElement;
		theElement=m_pElementList+m_NumElements-1;

		int xHighLimit=theElement->X_pos+theElement->Length+5*m_MarginX/2+delta;
		
		if ((this->m_MaxNumColumns>1) || (this->m_MaxNumRows>1))
		{
			xHighLimit=min(xHighLimit,this->m_MatrixColumns[this->m_MaxNumColumns-1].x+this->m_MatrixColumns[this->m_MaxNumColumns-1].length);
		}

		if ((X>=theElement->X_pos+theElement->Length-delta)  && (X<=xHighLimit) 
			&& (Y>theElement->Y_pos-ActualSize/2) && (Y<theElement->Y_pos+ActualSize/2))
		if ((this->m_IsKeyboardEntry!=m_NumElements) || (theElement->pElementObject->Data1[0]))
		{
			if (theElement->Type==0) //the dummy object can be only pointed before
			{
				m_Selection=m_NumElements;
				*IsExpression=m_NumElements;
				return (CObject*)this;
			}
			
			//if (!insertion_points_only)
			if (ClipboardExpression==NULL)
			if ((theElement->Type==1) && (theElement->pElementObject->m_Text) && (X<theElement->X_pos+theElement->Length+m_MarginX*2/3))
			{
				CObject *obj=(theElement->pElementObject)->SelectAtPoint(DC,zoom,X-theElement->X_pos,Y,IsExpression,IsParenthese,1);
				if (obj) theElement->IsSelected=1;
				return obj;
			}

			m_Selection=m_NumElements+1;
			*IsExpression=m_NumElements+1;

			return (CObject*)this;
		}
	}

	//checking the very first insertion point
	{
		tElementStruct *theElement;
		theElement=m_pElementList;
		int xLowLimit=theElement->X_pos-5*m_MarginX/2;
		if ((this->m_MaxNumColumns>1) || (this->m_MaxNumRows>1))
		{
			xLowLimit=max(xLowLimit,0);
		}
		if ((X<=theElement->X_pos+delta) && (X>xLowLimit) &&
			(Y>theElement->Y_pos-ActualSize/2) && (Y<theElement->Y_pos+ActualSize/2))
			if ((this->m_IsKeyboardEntry!=1) || (theElement->pElementObject->Data1[0]))
		{
			//check the space just before an element
			//if (!insertion_points_only)
			if (ClipboardExpression==NULL)
			if ((theElement->Type==1) && (theElement->pElementObject->m_Text) && (X>theElement->X_pos-m_MarginX/2))
			{
				//babaluj
				CObject *obj=(theElement->pElementObject)->SelectAtPoint(DC,zoom,X-theElement->X_pos,Y,IsExpression,IsParenthese,1);
				if (obj) theElement->IsSelected=1;
				return obj;
			}

			m_Selection=1;
			*IsExpression=1;

			return (CObject*)this;
		}
	}

	//checking all other insertion points
	{
		int row=0;
		int col=0;
		for (i=1;i<m_NumElements;i++)
		{
			tElementStruct *theElement;
			tElementStruct *prevElement;
			theElement=m_pElementList+i;
			prevElement=m_pElementList+i-1;
			int after_word=0;
			if (prevElement->Type==1) after_word=1;
	

			int extends=0;
			int extends2=0;
			if (((prevElement->Type==2) && (prevElement->pElementObject->Data1[0]==(char)0xFF))  || //newline
				(prevElement->Type==11) || (prevElement->Type==12))
				extends=3*m_MarginX/2;
			if (i<m_NumElements-1)
			{
				tElementStruct *postElement=m_pElementList+i+1;
				if (((theElement->Type==2) && (theElement->pElementObject->Data1[0]==(char)0xFF)) || //newline
					(theElement->Type==11) || (theElement->Type==12))
					extends2=3*m_MarginX/2;
			}


			int xHighLimit=theElement->X_pos+delta+extends2;
			int xLowLimit=(after_word)?(prevElement->X_pos+prevElement->Length-delta-extends):(theElement->X_pos-m_MarginX-delta-extends);
			if ((this->m_MaxNumColumns>1) || (this->m_MaxNumRows>1))
			{
				xHighLimit=min(xHighLimit,this->m_MatrixColumns[col].x+this->m_MatrixColumns[col].length);
				xLowLimit=max(xLowLimit,this->m_MatrixColumns[col].x);
			}
			if (theElement->Type==11) col++;
			if (theElement->Type==12) {col=0;row++;}
			


			if ((X<=xHighLimit) && (X>=xLowLimit) 
				 && (Y>theElement->Y_pos-ActualSize/2) && (Y<theElement->Y_pos+ActualSize/2))
			if ((this->m_IsKeyboardEntry!=i+1) || (theElement->pElementObject->Data1[0]))
			if ((this->m_IsKeyboardEntry!=i) || ((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1[0]))
			{
				if (ClipboardExpression==NULL)
				if ((prevElement->Type==1) && (prevElement->pElementObject->m_Text) && (X<=theElement->X_pos-m_MarginX/2-m_MarginX/8))
				{
					//babaluj
					CObject *obj=(prevElement->pElementObject)->SelectAtPoint(DC,zoom,X-prevElement->X_pos,Y,IsExpression,IsParenthese,1);
					if (obj) prevElement->IsSelected=1;
					return obj;
				}
				//if (!insertion_points_only)
				if (ClipboardExpression==NULL)
				if ((theElement->Type==1) && (theElement->pElementObject->m_Text) && (X>theElement->X_pos-m_MarginX/2-m_MarginX/8))
				{
					//babaluj
					CObject *obj=(theElement->pElementObject)->SelectAtPoint(DC,zoom,X-theElement->X_pos,Y,IsExpression,IsParenthese,1);
					if (obj) theElement->IsSelected=1;
					return obj;
				}
				//check the space just before an element
				if (((theElement->Type==11) || (theElement->Type==12)) && (prevElement->Type==0))
				{
					m_Selection=i;
					*IsExpression=i;
					return (CObject*)this;
				}
				m_Selection=i+1;
				*IsExpression=i+1;
				return (CObject*)this;
			}
		}
	}

	return NULL;
}


void CExpression::AddToStackClipboard(int add_forcefully)
{
	//check if this is a very simple expression - in this case we dont't add it to the clipboard
	//(because it is easier to type it than to paste it)

	if (add_forcefully==0)
	{
		if (GetKeyState(VK_CONTROL)&0xFFFE)return; //we do not remember keyboard accelerator generated objects
		if (this->m_NumElements==1)
		{
			if (this->m_pElementList->Type==0) return;

			if (this->m_pElementList->Type==1)
			{
				if (strlen(this->m_pElementList->pElementObject->Data1)==1)
				{
					char ch=this->m_pElementList->pElementObject->Data1[0];
					if ((ch>=0) && (ch<='z')) return;
				}
			}
			if (this->m_pElementList->Type==2)
			{
				char ch=this->m_pElementList->pElementObject->Data1[0];
				if ((ch=='+') || (ch=='-') || (ch=='/') || (ch==(char)0xD7) || (ch=='>') || (ch=='<') || (ch=='=')) return;
			}
		}
		if (this->m_NumElements>25) return; //we are not going to add too long objects either
	}
	//int i;
	//check if we need to add the expression into the stack-clipboard
	/*CalcStructuralChecksumOnly=1;
	unsigned int chksm=this->CalcChecksum();
	for (i=0;i<5;i++)
	{
		if ((prevClipboardExpression[i]))
		{
			int chksm2=prevClipboardExpression[i]->CalcChecksum();
			if (chksm==chksm2)
			{
				//we find the same object already in the stack-clipboard list - we are just reordering
				CExpression *temp=prevClipboardExpression[i];
				for (int j=i;j>0;j--)
					prevClipboardExpression[j]=prevClipboardExpression[j-1];
				prevClipboardExpression[0]=temp;
				break;
			}
		}
	}
	CalcStructuralChecksumOnly=0;*/


	//if (i==5)
	{
		if (prevClipboardExpression) delete prevClipboardExpression;
		prevClipboardExpression=new CExpression(NULL,NULL,100);
		prevClipboardExpression->CopyExpression(this,0,0,0);
	}
		
}

// Copies all data from original expression into this one
int CopyExpressionSpecialFlag=0;
extern int LastTakenChecksum;
int CExpression::CopyExpression(const CExpression* Original,const char OnlySelected,const char selection_type,const char update_clipboard)
{
	//first delete all data in this expression if not empty
	if ((m_NumElements!=1) || (m_pElementList->Type!=0))
	{
		Delete();
	}

	//check if original is valid
	if (Original==NULL) return 0;
	if (Original->m_NumElements<0) return 0;
	if (Original->m_NumElements>0x7FFF) return 0;

	if (OnlySelected==2) CopyExpressionSpecialFlag=1;
	
	//now copy data from original
	int i,j=0;
	const int num_elements=Original->m_NumElements;

	if (OnlySelected!=1)
	{
		tElementStruct *theElement=Original->m_pElementList;
		for (i=0;i<num_elements;i++,theElement++)
		{
			if (!InsertElement(theElement,j++)) 
			{
				//some unexpected error (return)
				if (OnlySelected==2) CopyExpressionSpecialFlag=0;
				return 0;
			}
		}
	}
	else
	{
		int lastrow=-1,lastcol=1;
		int row=0,column=0;
		char tmp_data[24];
		tmp_data[0]=0;
		int data_copied=0;
		int is_matrix=0;
		tElementStruct *theElement=Original->m_pElementList;
		for (i=0;i<num_elements;i++,theElement++)
		{			
			if (theElement->Type==11) {column++;if ((data_copied) && (!tmp_data[0])) memcpy(tmp_data,theElement->pElementObject->Data1,24);if (OnlySelected==1) theElement->IsSelected=0;}
			if (theElement->Type==12) {row++;column=0;if ((data_copied) && (!tmp_data[0])) memcpy(tmp_data,theElement->pElementObject->Data1,24);if (OnlySelected==1) theElement->IsSelected=0;}
			if ((theElement->IsSelected==selection_type) || (OnlySelected!=1))
			{
				if (OnlySelected==1)
				{
					if ((lastrow!=-1) && (lastcol!=-1))
					{
						if (lastrow!=row) {is_matrix=1;InsertEmptyElement(j,12,0);memcpy((m_pElementList+j)->pElementObject->Data1,tmp_data,24);data_copied=0;tmp_data[0]=0;j++;}
						else if (lastcol!=column) {is_matrix=1;InsertEmptyElement(j,11,0);	memcpy((m_pElementList+j)->pElementObject->Data1,tmp_data,24);data_copied=0;tmp_data[0]=0;j++;}
					}
				}
				if ((OnlySelected!=1) || (theElement->Type!=1) || (theElement->pElementObject->Data1[0]))
				{
					data_copied=1;
					if (!InsertElement(theElement,j)) 
					{
						//some unexpected error (return)
						if (OnlySelected==2) CopyExpressionSpecialFlag=0;
						return 0;
					}
					j++;
				}
				lastrow=row;
				lastcol=column;
			}
		}
		if (is_matrix)
		{
			if (lastrow!=row) {is_matrix=1;InsertEmptyElement(j,12,0);memcpy((m_pElementList+j)->pElementObject->Data1,tmp_data,24);data_copied=0;tmp_data[0]=0;j++;}
			else if (lastcol!=column) {is_matrix=1;InsertEmptyElement(j,11,0);	memcpy((m_pElementList+j)->pElementObject->Data1,tmp_data,24);data_copied=0;tmp_data[0]=0;j++;}

		}
	}
	m_Selection=0;

	m_StartAsText=Original->m_StartAsText;
	m_IsHeadline=Original->m_IsHeadline;
	m_Color=Original->m_Color;
	m_IsComputed=Original->m_IsComputed;
	m_Alignment=Original->m_Alignment;
	m_ParentheseShape=Original->m_ParentheseShape;
	m_ParenthesesFlags=Original->m_ParenthesesFlags;
	if (m_pPaternalExpression) m_IsVertical=0; else m_IsVertical=Original->m_IsVertical;

	if (OnlySelected==1)
	{
		if (!(Original->m_ParenthesesSelected)) m_ParenthesesFlags&=0xFE;  //clear show parentheses flag
		if ((Original->m_DrawParentheses) && (Original->m_ParenthesesSelected)) m_ParenthesesFlags|=0x01; //set show parentheses flag
	}
	if (CopyExpressionSpecialFlag)
	{
		m_IsKeyboardEntry=Original->m_IsKeyboardEntry;
		m_KeyboardCursorPos=Original->m_KeyboardCursorPos;
	}
	if (OnlySelected==2) CopyExpressionSpecialFlag=0;

	if ((Original->m_MaxNumRows>1) || (Original->m_MaxNumColumns>1))
	{
		//we must call calculate size in order to generate valid matrix/table parameters
		//AdjustMatrix();
		CDC *DC=pMainView->GetDC();
		short l,a,b;
		this->CalculateSize(DC,ViewZoom,&l,&a,&b);
		pMainView->ReleaseDC(DC);
	}

	//special handling - if copied into clipboard, copy also into windows clipboard
	if ((this==ClipboardExpression) && (update_clipboard))
	{
		CopyToWindowsClipboard();
	}

	return 1;
}


//this function deletes all exprssion contents (including its subelements)
//it then creates one single dummy element of the expression (empty frame)
void CExpression::Delete(void)
{
	if (KeyboardEntryObject==(CObject*)this) {KeyboardEntryObject=NULL;KeyboardEntryBaseObject=NULL;}

	m_Selection=0;
	m_ModeDefinedAt=0;
	m_IsPointerHover=0;
	m_IsMatrixElementSelected=0;

	if (m_MatrixRows) {HeapFree(ProcessHeap,0,m_MatrixRows);m_MatrixRows=NULL;}
	if (m_MatrixColumns) {HeapFree(ProcessHeap,0,m_MatrixColumns);m_MatrixColumns=NULL;}

	if (m_pElementList==NULL)
	{
		m_ElementListReservations=1;
		m_pElementList=(tElementStruct*)HeapAlloc(ProcessHeap,0,sizeof(tElementStruct));
		if (m_pElementList==NULL) {AfxMessageBox("Cannot reserve memory(3)",MB_OK,NULL);return;}
	}
	else
	{
		tElementStruct *theElement=m_pElementList+m_NumElements-1;
		for (;theElement>=m_pElementList;theElement--)
			if (theElement->pElementObject)
				delete theElement->pElementObject;
	}
	m_NumElements=1;

	//create an empty frame object
	m_pElementList->Type=0;
	m_pElementList->IsSelected=0;
	m_pElementList->pElementObject=NULL;
	m_pElementList->Decoration=0;

	m_ParentheseShape='(';
	m_ParenthesesFlags=0;
	m_ParenthesesSelected=0;
	m_IsKeyboardEntry=0;
	m_KeyboardCursorPos=0;
	m_RowSelection=0;
	m_ColumnSelection=0;
	m_IsColumnInsertion=0;
	m_IsRowInsertion=0;
	m_Alignment=0;
	m_MaxNumRows=1;
	m_MaxNumColumns=1;
	m_IsComputed=0;
	m_IsVertical=0;
	m_Color=-1;
	m_IsHeadline=0;
	m_DrawParentheses=0;

	return;
}


//this function inserts the given element at the given position into this expression
//if neccessary, te function expands memory list (note: the copy of original object is made and inserted)
int CExpression::InsertElement(const tElementStruct* Element, int position)
{
	if (Element==NULL) return 0;
	if (position<0) return 0;
	if (position>m_NumElements) return 0;
	//if (Element->Type<0) return 0;
	if (m_NumElements>0x7FFF-2) return 0; //0x7FFF-2 elements is the maximum for an expression

	CExpression *ccc=this;
	do
	{
		ccc->m_IsComputed=0;
		ccc=ccc->m_pPaternalExpression;
	} while (ccc);

	if ((m_NumElements>=position) && (position>0) && ((m_pElementList+position-1)->Type==0)
		&& (Element->Type!=11) && (Element->Type!=12)) position--;

	if ((m_NumElements>position) && ((m_pElementList+position)->Type==0))
	{
		//only the 'empty frame' is part of expression - will be replaced
		tElementStruct *ts=m_pElementList+position;
		ts->Type=Element->Type;
		ts->IsSelected=0;
		ts->Decoration=Element->Decoration;
		ts->X_pos=Element->X_pos;
		ts->Y_pos=Element->Y_pos;
		ts->Length=Element->Length;
		ts->Above=Element->Above;
		ts->Below=Element->Below;
		if ((Element->Type>0) /*&& (Element->Type!=11) && (Element->Type!=12)*/)
		{
			CElementInitPaternalExpression=(CObject*)this;
			CElementInitType=Element->Type;
			ts->pElementObject=new CElement();
			ts->pElementObject->CopyElement(Element->pElementObject);
		}
		else
			ts->pElementObject=NULL;
		return 1;
	}


	//expanding the element list
	m_NumElements++;
	if (m_NumElements>m_ElementListReservations)
	{
		if (m_ElementListReservations==1) m_ElementListReservations+=3; else m_ElementListReservations+=6;
		m_pElementList=(tElementStruct*)HeapReAlloc(ProcessHeap,0,(void*)m_pElementList,m_ElementListReservations*sizeof(tElementStruct));
		if (m_pElementList==NULL) {AfxMessageBox("Cannot alloc memory!!!",MB_OK,0);return 0;}
	}

	//shifting data
	tElementStruct *ts=m_pElementList+position;
	memmove(ts+1,ts,(m_NumElements-position-1)*sizeof(tElementStruct));


	//creating the element
	ts->Type=Element->Type;
	ts->IsSelected=0;
	ts->Decoration=Element->Decoration;
	ts->X_pos=Element->X_pos;
	ts->Y_pos=Element->Y_pos;
	ts->Length=Element->Length;
	ts->Above=Element->Above;
	ts->Below=Element->Below;
	if ((Element->Type>0) /*&& (Element->Type!=11) && (Element->Type!=12)*/ && (Element->pElementObject))
	{
		CElementInitPaternalExpression=(CObject*)this;
		CElementInitType=Element->Type;
		ts->pElementObject=new CElement();
		ts->pElementObject->CopyElement(Element->pElementObject);
	}
	else
		ts->pElementObject=NULL;
	return 1;
}


//this function moves the given element at the given position into this expression
//if neccessary, te function expands memory list. (note - the original element object is inserted, not it's copy)
int CExpression::MoveElementInto(const tElementStruct* Element, int position)
{
	if (Element==NULL) return 0;
	if (position<0) return 0;
	if (position>m_NumElements) return 0;
	if (Element->Type<0) return 0;
	if (m_NumElements>0x7FFF-2) return 0; //0x7FFF-2 elements is the maximum for an expression

	CExpression *ccc=this;
	do
	{
		ccc->m_IsComputed=0;
		ccc=ccc->m_pPaternalExpression;
	} while (ccc);

	if ((m_NumElements>=position) && (position>0) && ((m_pElementList+position-1)->Type==0)
		&& (Element->Type!=11) && (Element->Type!=12)) position--;

	if ((m_NumElements>position) && ((m_pElementList+position)->Type==0))
	{
		//only the 'empty frame' is part of expression - will be replaced
		tElementStruct *ts=m_pElementList+position;
		ts->Type=Element->Type;
		ts->IsSelected=0;
		ts->Decoration=Element->Decoration;
		ts->X_pos=Element->X_pos;
		ts->Y_pos=Element->Y_pos;
		ts->Length=Element->Length;
		ts->Above=Element->Above;
		ts->Below=Element->Below;
		if (Element->Type>0)
			ts->pElementObject=Element->pElementObject;
		else
			ts->pElementObject=NULL;
		return 1;
	}

	//expanding the element list
	m_NumElements++;
	if (m_NumElements>m_ElementListReservations)
	{
		if (m_ElementListReservations==1) m_ElementListReservations+=3; else m_ElementListReservations+=6;
		m_pElementList=(tElementStruct*)HeapReAlloc(ProcessHeap,0,(void*)m_pElementList,m_ElementListReservations*sizeof(tElementStruct));
		if (m_pElementList==NULL) {AfxMessageBox("Cannot alloc memory!!!",MB_OK,0);return 0;}
	}

	//moving data
	tElementStruct *ts=m_pElementList+position;
	memmove(ts+1,ts,(m_NumElements-position-1)*sizeof(tElementStruct));

	//creating the element
	ts->Type=Element->Type;
	ts->IsSelected=0;
	ts->Decoration=Element->Decoration;
	ts->X_pos=Element->X_pos;
	ts->Y_pos=Element->Y_pos;
	ts->Length=Element->Length;
	ts->Above=Element->Above;
	ts->Below=Element->Below;
	if (Element->Type>0)
		ts->pElementObject=Element->pElementObject;
	else
		ts->pElementObject=NULL;
	return 1;
}

/*
//This functions moves elements from one expression to another expression (elements are moved, not copied)
//TODO  -  unfinished
int CExpression::MovePortionInto(int position, CExpression *Original, int source_pos, int num_elements)
{
	int i;

	CExpression *ccc=this;
	do
	{
		ccc->m_IsComputed=0;
		ccc=ccc->m_pPaternalExpression;
	} while (ccc);

	if ((m_NumElements>=position) && (position>0) && ((m_pElementList+position-1)->Type==0)
		&& ((Original->m_pElementList+source_pos)->Type!=11) && ((Original->m_pElementList+source_pos)->Type!=12)) position--;

	tElementStruct *ts;
	if ((m_NumElements>position) && ((m_pElementList+position)->Type==0))
	{
		//expanding the element list
		m_NumElements+=num_elements-1;
		if (m_NumElements>m_ElementListReservations)
		{
			m_ElementListReservations=m_NumElements;
			m_pElementList=(tElementStruct*)HeapReAlloc(ProcessHeap,0,(void*)m_pElementList,m_ElementListReservations*sizeof(tElementStruct));
			if (m_pElementList==NULL) {AfxMessageBox("Cannot alloc memory (6)!!!",MB_OK,0);return 0;}
		}
		ts=m_pElementList+position;
		memmove(ts+num_elements-1,ts,(m_NumElements-position-2)*sizeof(tElementStruct));
	}
	else
	{
		//expanding the element list
		m_NumElements+=num_elements;
		if (m_NumElements>m_ElementListReservations)
		{
			m_ElementListReservations=m_NumElements;
			m_pElementList=(tElementStruct*)HeapReAlloc(ProcessHeap,0,(void*)m_pElementList,m_ElementListReservations*sizeof(tElementStruct));
			if (m_pElementList==NULL) {AfxMessageBox("Cannot alloc memory (7)!!!",MB_OK,0);return 0;}
		}
		ts=m_pElementList+position;
		memmove(ts+num_elements,ts,(m_NumElements-position-1)*sizeof(tElementStruct));
	}

	//moving elements
	for (i=0;i<num_elements;i++)
	{
		tElementStruct *Element=Original->m_pElementList+source_pos+i;
		ts->Type=Element->Type;
		ts->IsSelected=0;
		ts->Decoration=Element->Decoration;
		ts->X_pos=Element->X_pos;
		ts->Y_pos=Element->Y_pos;
		ts->Length=Element->Length;
		ts->Above=Element->Above;
		ts->Below=Element->Below;
		if (Element->Type>0)
			ts->pElementObject=Element->pElementObject;
		else
			ts->pElementObject=NULL;
		ts++;
	}

	
	tElementStruct *theElement;
	theElement=Original->m_pElementList+source_pos;

	//then reduce the element list
	memmove(theElement,theElement+num_elements,(Original->m_NumElements-source_pos-num_elements)*sizeof(tElementStruct));

	Original->m_NumElements-=num_elements;
	
	ccc=Original;
	do
	{
		ccc->m_IsComputed=0;
		ccc=ccc->m_pPaternalExpression;
	} while (ccc);
	

	if (Original->m_NumElements==0)
	{
		//if emptied, we will add a dummy 'empty frame'
		Original->m_pElementList->Type=0;
		Original->m_pElementList->IsSelected=0;
		Original->m_pElementList->pElementObject=NULL;
		Original->m_pElementList->X_pos=0;
		Original->m_pElementList->Y_pos=0;
		Original->m_pElementList->Length=0;
		Original->m_pElementList->Below=0;
		Original->m_pElementList->Above=0;

		Original->m_NumElements=1;
	}

	return 1;
}
*/

//this function copies original somewhere into the equation - the position
//to copy at, is specified in coordinates X,Y (in pixels). This is used
//to copy for clipboard, at mouse click.
CExpression *CExpression::CopyAtPoint(CDC* DC, short zoom, short X, short Y, CExpression* Original,int keyboard_select)
{
	if (zoom<5) zoom=5;
	if (zoom>5000) zoom=5000;
	if (X<-5000) return 0;
	if (X>30000) return 0;
	if (Y<-5000) return 0;
	if (Y>15000) return 0;
	if (Original==NULL) return 0;
	if (Original->m_NumElements<0) return 0;
	if (Original->m_NumElements>0x7FFF) return 0;

	//first, find the grand paternal expression
	CExpression *expr=this;
	while (expr->m_pPaternalExpression) expr=expr->m_pPaternalExpression;

	//now find the paternal expression of all selected expressions
	expr=expr->AdjustSelection(keyboard_select);

	//if found no selection, then continue using provided coordinates
	if (expr==NULL)
	{
		if (DC==NULL) 
		{
			expr=this;
		}
		else
		{
			short IsExpression;
			char IsParenthese;
			CObject *obj=SelectObjectAtPoint(DC,zoom,X,Y,&IsExpression,&IsParenthese);
			if (obj==NULL) return 0;
			if (IsExpression==0) return 0;
			expr=(CExpression*)obj;
		}
	}

	if (expr->m_Selection==0x7FFF)
	{
		//the whole expression being selected for replacement

		if (expr->m_DrawParentheses==0)
		{
			//if the destination expression has no parentheses, then just copy
			expr->CopyExpression(Original,0);
			return expr;
		}

		if (expr->m_ParenthesesSelected==0) //no parentheses are to be replaced
		{
			//the destination has parentheses, but are not selected for replacement
			if (Original->m_DrawParentheses)
			{
				//the original has parentheses, we will add these also
				expr->CopyExpression(Original,0);
				return expr;
			}

			//int p1=expr->m_ParentheseData;
			//int p2=expr->m_ParentheseHeightFactor;
			int p3=expr->m_ParentheseShape;
			int p4=expr->m_ParenthesesFlags;
			int tmp=expr->m_ParenthesesSelected;
			expr->CopyExpression(Original,0);
			//expr->m_ParentheseData=p1;
			//expr->m_ParentheseHeightFactor=p2;
			expr->m_ParentheseShape=(char)p3;
			expr->m_ParenthesesFlags=p4;
			return expr;
		}
		else
		{
			//parentheses are to be replaced, and original has no parentheses
			if ((Original->m_DrawParentheses==0) && (expr->m_pPaternalExpression) && 
				(expr->m_pPaternalElement) && (expr->m_pPaternalElement->m_Type==5) && 
				(Original->m_MaxNumRows==1) && (Original->m_MaxNumColumns==1))
			{
				//we are puting something over parentheses, we need to kill existing parentheses
				int i;
				for (i=0;i<expr->m_pPaternalExpression->m_NumElements;i++)
				{
					tElementStruct *ts=(expr->m_pPaternalExpression->m_pElementList)+i;
					if ((ts->Type==5) && (ts->pElementObject->Expression1==(CObject*)expr))
					{
						int j;
						for (j=0;j<Original->m_NumElements;j++)
						{
							tElementStruct *tts=Original->m_pElementList+j;
							expr->m_pPaternalExpression->InsertElement(tts,i+j+1);
						}
						expr->m_pPaternalExpression->DeleteElement(i);
						break;
					}
				}
				return expr;
			}

			//the original has parentheses, just copy
			expr->CopyExpression(Original,0);
			return expr;
		}
		return expr;
	}
	else if (expr->m_Selection)
	{
		int pos=expr->m_Selection-1;
		int IsExpressionEmpty=0;
		if (expr->m_NumElements==0) IsExpressionEmpty=1;
		if ((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) IsExpressionEmpty=1;
		//inserting into expression
		int OriginalIsText=Original->IsTextContained(-1);
		//int ThisIsText=expr->IsTextContained(pos);
		int ThisIsText=expr->DetermineInsertionPointType(pos);
		if ((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) ThisIsText=OriginalIsText;

		if ((Original->m_DrawParentheses) /*|| ((OriginalIsText!=ThisIsText) && (ThisIsText==0))*/ || 
			((!IsExpressionEmpty) && ((Original->m_MaxNumColumns>1) || (Original->m_MaxNumRows>1))))
		{
			//the original has parentheses (or is matrix/vector), we will add these also
			expr->InsertEmptyElement(pos,5,0);
			expr=(CExpression*)(((expr->m_pElementList+pos)->pElementObject)->Expression1);
			expr->CopyExpression(Original,0);
			/*if ((Original->m_DrawParentheses==0) && (OriginalIsText)) 
			{
				expr->m_DrawParentheses=expr->m_ParentheseShape='/';
				expr->m_ParenthesesFlags=0x01;
			}*/
			return expr;
		}

		//the original has no parentheses, so we will insert just here
		int i;
		for (i=0;i<Original->m_NumElements;i++)
		{
			int ret;
			tElementStruct *theElement;
			theElement=Original->m_pElementList+i;
			if ((theElement->Type==2) && (theElement->pElementObject->Data1[0]==(char)0xFF) && (expr->m_MaxNumRows==1) && (expr->m_MaxNumColumns==1))
			{
				//expr->m_IsText=1;
				expr->m_Alignment=1;
			}
			ret=expr->InsertElement(theElement,pos+i);

			if (ret==0) return 0;
		}
		return expr;
	}
	else
	{
		//we replace over all selected objects
		int i;
		int first_found=-1;
		int row=0,column=0;
		int first_found_row=-1;
		int first_found_column=-1;

		if (expr->m_IsColumnInsertion)
		{
			//column insertion
			int i;
			for (i=0;i<Original->m_MaxNumColumns;i++)
				InsertMatrixColumn(expr->m_ColumnSelection);

			expr->m_IsMatrixElementSelected=1;
			first_found=expr->FindMatrixElement(expr->m_RowSelection,expr->m_ColumnSelection,1);
			if (first_found==-1) return 0;
			first_found_row=expr->m_RowSelection;
			first_found_column=expr->m_ColumnSelection;
		}
		else if (expr->m_IsRowInsertion)
		{
			//row insertion
			int i;
			for (i=0;i<Original->m_MaxNumRows;i++)
				InsertMatrixRow(expr->m_RowSelection);

			expr->m_IsMatrixElementSelected=1;
			first_found=expr->FindMatrixElement(expr->m_RowSelection,expr->m_ColumnSelection,1);
			if (first_found==-1) return 0;
			first_found_row=expr->m_RowSelection;
			first_found_column=expr->m_ColumnSelection;
		}
		else
		{
			//overwrite (first delete content of existing cells - selected ones will be overwriten)
			for (i=0;i<expr->m_NumElements;i++)
			{
				tElementStruct *ts=expr->m_pElementList+i;
				if (ts->Type==11) {ts->IsSelected=0;column++;}
				if (ts->Type==12) {ts->IsSelected=0;column=0;row++;}
				if (ts->IsSelected)
				{
					if (first_found==-1) {first_found=i;first_found_row=row;first_found_column=column;}
					expr->DeleteElement(i);
					i--;
				}
			}
		}
		if (first_found==-1) {first_found=0;first_found_column=0;first_found_row=0;}

		if ((Original->m_DrawParentheses) || 
			(((Original->m_MaxNumColumns>1) || (Original->m_MaxNumRows>1)) && (!(expr->m_IsMatrixElementSelected))))
		{
			//the original has parentheses, we will add these also
			expr->InsertEmptyElement(first_found,5,0);
			expr=(CExpression*)(((expr->m_pElementList+first_found)->pElementObject)->Expression1);
			expr->CopyExpression(Original,0);
			return expr;
		}
		row=0;column=0;
		if (expr->m_IsMatrixElementSelected)
		{
			//we have to paste over matrix elements
			first_found=expr->FindMatrixElement(first_found_row,first_found_column,1);
			for (i=0;i<Original->m_NumElements;i++)
			{
				tElementStruct *ts=Original->m_pElementList+i;
				if (ts->Type==11) 
				{
					tCellAttributes att;
					if (Original->GetCellAttributes(row,column,&att))
						SetCellAttributes(first_found_row+row,first_found_column+column,*att.alignment,*att.top_border,*att.bottom_border,*att.left_border,*att.right_border);
					column++;
					first_found=expr->FindMatrixElement(first_found_row+row,first_found_column+column,1);
					continue;
				}
				if (ts->Type==12) 
				{
					tCellAttributes att;
					if (Original->GetCellAttributes(row,column,&att))
						SetCellAttributes(first_found_row+row,first_found_column+column,*att.alignment,*att.top_border,*att.bottom_border,*att.left_border,*att.right_border);
					if (i<Original->m_NumElements-1)
					{
						row++;column=0;
						first_found=expr->FindMatrixElement(first_found_row+row,first_found_column+column,1);
					}
					continue;
				}
				if (first_found==-1) return 0;
				if ((expr->InsertElement(ts,first_found))==0) return 0;
				first_found++;
			}
			expr->AdjustMatrix();
			return expr;
		}

		for (i=0;i<Original->m_NumElements;i++)
		{
			tElementStruct *ts=Original->m_pElementList+i;
			if ((expr->InsertElement(ts,first_found+i))==0) return 0;
		}
		return expr;
	}



	return 0;
}

int CExpression::ContainsBlinkingCursor()
{
	if ((this->m_IsKeyboardEntry) && (KeyboardEntryObject==(CObject*)this)) return m_IsKeyboardEntry;

	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if (ts->pElementObject)
		{
			int t=ts->pElementObject->ContainsBlinkingCursor();
			if (t) return i+1;
		}
	}
	return 0;
}


CElement *CExpression::DecodeInternalInsertionPoint() //babaluj
{
	int ns=0;
	int fp=-1;
	for (int i=0;i<m_NumElements;i++)
		if ((m_pElementList+i)->IsSelected) {ns++;fp=i;}

	if ((ns==1) && ((m_pElementList+fp)->Type==1) && ((m_pElementList+fp)->pElementObject->m_Text) && (this->m_InternalInsertionPoint))
		return (m_pElementList+fp)->pElementObject;
	else
		return NULL;
}

//deletes an element of the expression
int CExpression::DeleteElement(short position)
{
	if (position>=m_NumElements) return 0;
	if (position<0) return 0;

	//first delete element itself
	tElementStruct *theElement;
	theElement=m_pElementList+position;
	if ((theElement->Type>0) && (theElement->pElementObject))
	{
		delete theElement->pElementObject;
	}

	//then reduce the element list
	memmove(theElement,theElement+1,(m_NumElements-position-1)*sizeof(tElementStruct));

	m_NumElements--;
	
	CExpression *ccc=this;
	do
	{
		ccc->m_IsComputed=0;
		ccc=ccc->m_pPaternalExpression;
	} while (ccc);
	

	if (m_NumElements==0)
	{
		//if emptied, we will add a dummy 'empty frame'
		m_pElementList->Type=0;
		m_pElementList->IsSelected=0;
		m_pElementList->pElementObject=NULL;
		m_pElementList->X_pos=0;
		m_pElementList->Y_pos=0;
		m_pElementList->Length=0;
		m_pElementList->Below=0;
		m_pElementList->Above=0;

		m_NumElements=1;
	}

	return 1;
}

CExpression *CExpression::GetLabel()
{
	//search if there is Label contained in this expression
	tElementStruct *ts=m_pElementList;
	for (int i=0;i<m_NumElements;i++,ts++)
	if (ts->pElementObject)
	{
		CElement *el=ts->pElementObject;
		if ((ts->Type==9) && (el->Data1[0]=='L') && (el->Expression2==NULL) && (el->Expression3==NULL) && (el->Expression1))
		{
			//label element found - now return its expression
			return (CExpression*)el->Expression1;
		}
		CExpression *found;
		if (el->Expression1) {found=((CExpression*)el->Expression1)->GetLabel();if (found) return found;}
		if (el->Expression2) {found=((CExpression*)el->Expression2)->GetLabel();if (found) return found;}
		if (el->Expression3) {found=((CExpression*)el->Expression3)->GetLabel();if (found) return found;}
		
	}
	return NULL;
}

//this function checks if all selected elements into the equation have one common parent
//If not, the selection is expanded, and the parent expression is returned
//NULL is returned if nothing is selected
//this function must be fast!
CExpression *CExpression::AdjustSelection(char keyboard_select)
{
	//now go recursively
	CExpression* retval=NULL; //retval contains the parent expression of all selected items
	short i;
	short row=0,column=0;
	short FoundSelectedObjectRow=32767;
	short FoundSelectedObjectColumn=32767;
	short FoundSelectedObjectRow2=-1;
	short FoundSelectedObjectColumn2=-1;
	short FoundSelectedObject=32767;
	short FoundSelectedObject2=-1;
	
	tElementStruct *theElement=m_pElementList;
	for (i=0;i<m_NumElements;i++,theElement++)
	{
		if (theElement->Type==11) column++;
		else if (theElement->Type==12) {row++;column=0;}
		else 
		{
			for (int j=0;j<4;j++) //there can be three subexpressions in an element plus the element itself
			{
				CExpression *ret=NULL;
				if ((j==0) && (theElement->IsSelected)) ret=this;
				else if (theElement->pElementObject)
				{
					if ((j==1) && (theElement->pElementObject->Expression1)) ret=((CExpression*)theElement->pElementObject->Expression1)->AdjustSelection(keyboard_select);
					if ((j==2) && (theElement->pElementObject->Expression2)) ret=((CExpression*)theElement->pElementObject->Expression2)->AdjustSelection(keyboard_select);
					if ((j==3) && (theElement->pElementObject->Expression3)) ret=((CExpression*)theElement->pElementObject->Expression3)->AdjustSelection(keyboard_select);
				}
				if (ret)
				{					
					if (!retval) retval=ret; else retval=this;
					FoundSelectedObjectRow=min(FoundSelectedObjectRow,row);
					FoundSelectedObjectColumn=min(FoundSelectedObjectColumn,column);
					FoundSelectedObjectRow2=max(FoundSelectedObjectRow2,row);
					FoundSelectedObjectColumn2=max(FoundSelectedObjectColumn2,column);
					FoundSelectedObject=min(FoundSelectedObject,i);
					FoundSelectedObject2=max(FoundSelectedObject2,i);
				}			
			} //for (all subexpression in the element object)
		} // if (the element has element object)
	} //for (all elements in expression)

	if (retval!=this) return retval;
	
	if ((FoundSelectedObjectRow==FoundSelectedObjectRow2) && (FoundSelectedObjectColumn==FoundSelectedObjectColumn2))
	{
		if (FoundSelectedObject<=i)
			for (int jj=FoundSelectedObject;jj<=FoundSelectedObject2;jj++)
			{
				if ((m_pElementList+jj)->IsSelected==0)
					(m_pElementList+jj)->IsSelected=keyboard_select;
				CElement *elm=(m_pElementList+jj)->pElementObject;
				if (elm)
				{
					if (elm->Expression1!=NULL) ((CExpression*)(elm->Expression1))->SelectExpression(1);
					if (elm->Expression2!=NULL) ((CExpression*)(elm->Expression2))->SelectExpression(1);
					if (elm->Expression3!=NULL) ((CExpression*)(elm->Expression3))->SelectExpression(1);
				}
			}
	}
	else
	{
		for(int ii=FoundSelectedObjectRow;ii<=FoundSelectedObjectRow2;ii++)
			for (int jj=FoundSelectedObjectColumn;jj<=FoundSelectedObjectColumn2;jj++)
			{
				SelectMatrixElement(ii,jj,keyboard_select);
			}
	}		

	return retval;
}

//extern int DisableAutocomplete;
char lastkeystrokes[12];
int lastkeystrokeselm[12];
char prevEmptyBoxType=-1;
CExpression *previouslyEditedExpression;
//called when it is needed to start keyboard entry
#pragma optimize("s",on)
int CExpression::KeyboardStart(CDC* DC, short zoom)
{
	if (DC==NULL) return 0;
	if (zoom<5) zoom=5;
	if (zoom>5000) zoom=5000;
	//m_Selection tels us where to start keyboard entry
	prevEmptyBoxType=-1;

	if (((m_IsRowInsertion) || (m_IsColumnInsertion)) && (m_Selection==0))
	{
		if ((m_IsColumnInsertion) && (m_RowSelection<=m_MaxNumRows) && (m_ColumnSelection<=m_MaxNumColumns) && (m_MaxNumColumns<50))
		{
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("insert column",20300);
			InsertMatrixColumn(m_ColumnSelection);
			AdjustMatrix();
			m_Selection=FindMatrixElement(m_RowSelection,m_ColumnSelection,1)+1;
		}
		if ((m_IsRowInsertion) && (m_RowSelection<=m_MaxNumRows) && (m_ColumnSelection<=m_MaxNumColumns) && (m_MaxNumRows<50))
		{
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("row spacer",20301);
			InsertMatrixRow(m_RowSelection);
			AdjustMatrix();
			m_Selection=FindMatrixElement(m_RowSelection,m_ColumnSelection,1)+1;
		}
	}

	//unselect any selection
	int found_selection=0;
	tDocumentStruct *ds=TheDocument;
	for (int i=0;i<NumDocumentElements;i++,ds++)
		if (ds->MovingDotState==3) 
		{
			ds->MovingDotState=0;found_selection=1;
			if (ds->Object)
			{
				if (ds->Type==1) ((CExpression*)ds->Object)->DeselectExpression();
				if (ds->Type==2) ((CDrawing*)ds->Object)->SelectDrawing(0);
			}
		}
	if (found_selection) {NumSelectedObjects=0;pMainView->InvalidateRect(NULL,0);pMainView->UpdateWindow();}
	


	CElement *elm=this->DecodeInternalInsertionPoint();
	if (elm)
	{
		m_IsKeyboardEntry=elm->GetPaternalPosition()+1;
		int i;
		int t=this->GetActualFontSize(zoom)/16;
		int tt=m_InternalInsertionPoint;
		if (tt==1) tt=0;
		for (i=0;i<(int)strlen(elm->Data1);i++)
			if (elm->Data3[i]+t>=tt) break;
		m_KeyboardCursorPos=i;
		short l,a,b;
		CalculateSize(DC,zoom,&l,&a,&b);
		previouslyEditedExpression=NULL;
	}
	else
	{
		if (m_Selection<0) return 0;
		if (m_Selection==0x7FFF) return 0;
		if (m_Selection>m_NumElements+1) return 0;

		int start_pos=m_Selection;
		if (start_pos==0) start_pos=1;
		//We are going to create an empty variable
		if (!InsertEmptyElement(start_pos-1,1,0)) return 0;
		//recalculate
		short l,a,b;
		CalculateSize(DC,zoom,&l,&a,&b);
		m_IsKeyboardEntry=start_pos;
		m_KeyboardCursorPos=0;
		previouslyEditedExpression=NULL;
	}

	m_ModeDefinedAt=0;

	//deleting clipboard
	if (ClipboardExpression)
	{
		delete ClipboardExpression;
		ClipboardExpression=0;
	}

	if (KeyboardEntryObject)
	{
		HCURSOR hc=theApp.LoadCursor(IDC_CURSOR_TEXT);
		SetCursor(hc);
	
	}	
	if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

	AutocompleteSource=NULL;
	Autocomplete(0);

	return 1;
}

/*
void CExpression::StoreBackspaceStorage()
{
	//call this function when temporary backspace storage needs to be transfered to the stack-clipboard

	if (BackspaceStorage)
	{
		int N1=BackspaceStorage->m_NumElements-this->m_NumElements;
		int any_deleted=0;

		for (int i=0;i<m_IsKeyboardEntry-1;i++,any_deleted=1)
			BackspaceStorage->DeleteElement(0);

		int N2=BackspaceStorage->m_NumElements-N1-1;
		for (int i=0;i<N2;i++,any_deleted=1)
			BackspaceStorage->DeleteElement(N1+1);

		for (int i=0;i<BackspaceStorage->m_NumElements;i++)
			if (((BackspaceStorage->m_pElementList+i)->Type==1) && ((BackspaceStorage->m_pElementList+i)->pElementObject->Data1[0]==0))
			{
				BackspaceStorage->DeleteElement(i);
				i--;
			}
		if (any_deleted)
			BackspaceStorage->m_ParenthesesFlags=0;

		BackspaceStorage->AddToStackClipboard(0);
		delete BackspaceStorage;
		BackspaceStorage=NULL;
		BackspaceStorageOriginal=NULL;
	}
}
*/

#pragma optimize("s",on)
int CExpression::KeyboardStop(void)
{
	static char is_inside;
	if (KeyboardEntryBaseObject)
	if (KeyboardEntryBaseObject-TheDocument<NumDocumentElements)
	if (KeyboardEntryBaseObject->Type==1)
	{
		//if ((KeyboardEntryObject) && (BackspaceStorage)) ((CExpression*)KeyboardEntryObject)->StoreBackspaceStorage();

		CExpression *e=((CExpression*)KeyboardEntryBaseObject->Object);
		if ((e->m_NumElements>1) && 
			((e->m_pElementList+e->m_NumElements-1)->Type==2) && 
			(((e->m_pElementList+e->m_NumElements-1)->pElementObject->Data1[0]==(char)0xFF)))
				e->DeleteElement(e->m_NumElements-1); //remove the last element if it is the newline element
	}

	//m_IsKeyboardEntry contains ordinal number (with offset +1) of the element that 
	//is being edited. The edited element can only be variable (type==1)
	if ((m_IsKeyboardEntry<=0) || (m_IsKeyboardEntry>m_NumElements)) {AdjustMatrix();return 0;}

	if (this==(CExpression*)KeyboardEntryObject)
	{
		//we cannot call DeselectExpression because we only need to deselect the keyboard selections
		for (int i=0;i<m_NumElements;i++) 
			if ((m_pElementList+i)->IsSelected==2)
				(m_pElementList+i)->IsSelected=0;
	}

	tElementStruct *theElement;
	theElement=m_pElementList+m_IsKeyboardEntry-1;

	if (theElement->pElementObject==NULL) {m_IsKeyboardEntry=0;return 0;} //element not defined
	if (theElement->Type!=1) {m_IsKeyboardEntry=0;return 0;} //not variable

	if (!is_inside)
	if ((KeyboardEntryBaseObject) && (KeyboardEntryObject) &&(theElement->pElementObject) && (theElement->pElementObject->m_Text==0) && ((theElement->pElementObject->Data1[0]=='\'') || (theElement->pElementObject->Data1[0]=='\\')))
	{
		if ((m_KeyboardCursorPos==strlen(theElement->pElementObject->Data1)) && (m_KeyboardCursorPos>1))
		{
			//resolving multiletter variables (start with apostrophe) and commands (start with backslash)
			CDC *DC=pMainView->GetDC();
			this->ResolveKnownFunctions(DC,ViewZoom,' ',0,0,0,theElement);
			pMainView->ReleaseDC(DC);
			is_inside=1;
			int rval=((CExpression*)KeyboardEntryObject)->KeyboardStop();
			is_inside=0;
			return rval;
		}
	}

	CElement *EditedVariable=theElement->pElementObject;
	bool IsEditedVariableEmpty=(strlen(EditedVariable->Data1)==0)?1:0;

	if (IsEditedVariableEmpty)
	{
		DeleteElement(m_IsKeyboardEntry-1);
		if (m_Selection>m_IsKeyboardEntry)  m_Selection--;
	}

	m_IsKeyboardEntry=0;
	m_KeyboardCursorPos=0;
	AdjustMatrix();


	if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject-TheDocument<NumDocumentElements))
	{
		if (KeyboardEntryBaseObject->Type==1)
		{
			
			short l,a,b;
			CDC *DC=pMainView->GetDC();
			((CExpression*)KeyboardEntryBaseObject->Object)->CalculateSize(DC,ViewZoom,&l,&a,&b);
			pMainView->ReleaseDC(DC);
		}
	}
	return 1;
}

extern int IsALTDown;
extern int IsSHIFTALTDown;
extern int IsSHIFTDown;
//extern char IsSHIFTJustPressed;

CExpression *TempPopupExpression;
#pragma optimize("s",on)
int CExpression::KeyboardPopupClosed(int user_param,int exit_code)
{
	if (TempPopupExpression==NULL) return 0;

	if (user_param==0) // the ?? sequence was used to open popup menu
	{
		int pos=m_IsKeyboardEntry-1;
		if ((pos) && (exit_code))
		{
			if (TempPopupExpression->m_pElementList->Type)
			for (int ii=0;ii<TempPopupExpression->m_NumElements;ii++)
				InsertElement(TempPopupExpression->m_pElementList+ii,pos++);
		}
		if (((m_pElementList+pos)->Type!=1) || ((m_pElementList+pos)->pElementObject->Data1[0]))
			InsertEmptyElement(pos,1,0,Toolbox->GetFormattingColor());
		m_IsKeyboardEntry=pos+1;
		short l,a,b;
		CDC *DC=pMainView->GetDC();
		this->CalculateSize(DC,ViewZoom,&l,&a,&b);
		pMainView->ReleaseDC(DC);
		pMainView->ScrollCursorIntoView();
	}
	else //enter after keyboard selection was used to open popup menu
	{
		if (TempPopupExpression->m_pElementList->Type)
		{
			int tmp=m_NumElements; 
			for (int ii=0;ii<tmp;ii++)
				DeleteElement(0);
			for (int ii=0;ii<TempPopupExpression->m_NumElements;ii++)
				InsertElement(TempPopupExpression->m_pElementList+ii,ii);

			tmp=user_param;
			if (tmp>0)
			{
				this->InsertEmptyElement(tmp-1,1,0,Toolbox->GetFormattingColor());
				m_IsKeyboardEntry=tmp;
			}
			else if (tmp<0)
			{
				tmp=-tmp;			
				tmp=m_NumElements-tmp+1;
				if ((tmp>=0) && (tmp<=this->m_NumElements))
				{
					this->InsertEmptyElement(tmp,1,0,Toolbox->GetFormattingColor());
					m_IsKeyboardEntry=tmp+1;
				}
			}
			m_KeyboardCursorPos=0;
		}
	}
	delete TempPopupExpression;
	TempPopupExpression=NULL;
	return 1;
}

//removes variable if empty or replaces it with the dummy
int CExpression::RemoveEmptyVariable(CDC *DC, tElementStruct *theElement, char no_recomputation)
{
	if (theElement->pElementObject->Data1[0]==0) 
	{
		CExpression *parent=NULL;
		if (theElement->pElementObject) parent= (CExpression*)theElement->pElementObject->m_pPaternalExpression;

		int prev_type=11;
		int aft_type=11;
		if (m_IsKeyboardEntry>1) prev_type=(m_pElementList+m_IsKeyboardEntry-2)->Type;
		if (m_IsKeyboardEntry<m_NumElements) aft_type=(m_pElementList+m_IsKeyboardEntry)->Type;
		if (((prev_type==11) || (prev_type==12)) && ((aft_type==11) || (aft_type==12)))
		{
			DeleteElement(m_IsKeyboardEntry-1);
			InsertEmptyElement(m_IsKeyboardEntry-1,0,0);  //turn it into dummy object

			if (parent)
			{
				short l,a,b;
				parent->CalculateSize(DC,ViewZoom,&l,&a,&b);
			}
			return 0;
		}
		else
		{
			DeleteElement(m_IsKeyboardEntry-1);
			if (!no_recomputation)
			if (parent)
			{
				short l,a,b;
				if (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;
				parent->CalculateSize(DC,ViewZoom,&l,&a,&b);
			}
			return 1;
		}
	}
	return 0;
}

char InhibitAllKeyHandling=0;

//this function searches above and suggest the length of the TAB spacer (to make it aligned with the tab spacer in the previous row)
//returns the number of tabs inserted
int CExpression::InsertTabSpacer(int position, char stop_on_equality)
{
	if (position<0) return 0;
	if (position>m_NumElements) return 0;

	int orig_position=position;

	if (InhibitAllKeyHandling)
	{
		InsertEmptyElement(position,2,9);
		return 1;
	}
	//if the tab is not at the beginning of a line, then we suggest a standard tab len=0
	if ((position!=0) &&
		((m_pElementList+position-1)->Type!=11) &&
		((m_pElementList+position-1)->Type!=12) &&
		(((m_pElementList+position-1)->Type!=2) || ((m_pElementList+position-1)->pElementObject->Data1[0]!=(char)0xFF)))
	{
		InsertEmptyElement(position,2,9);
		return 1;
	}

	CExpression *exp=this;

	if (position) 
		position--;
	else if ((this->m_MaxNumColumns==1) && (this->m_MaxNumRows==1) && (KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
	{
		//we are at the beginning of an object - check if there is another object just above us
		int mindelta=0x7FFFFFFF;
		int ffound=-1;
		for (int i=0;i<NumDocumentElements;i++)
			if (KeyboardEntryBaseObject!=TheDocument+i)
			{
				if (TheDocument[i].absolute_X==KeyboardEntryBaseObject->absolute_X)
				{
					int delta=KeyboardEntryBaseObject->absolute_Y-KeyboardEntryBaseObject->Above-(TheDocument[i].absolute_Y+TheDocument[i].Below);
					if ((delta>0) && (delta<mindelta)) {mindelta=delta;ffound=i;}
				}
			}
			if ((ffound!=-1) && (mindelta<this->m_FontSize/3))
		{
			//we found an object just above our own object
			exp=(CExpression*)TheDocument[ffound].Object;
			position=exp->m_NumElements;
			
		}

	}

	//the tab is going to be placed at the beginning of a line, search line above it and determine the tab length
	int found_equality=0;
	while ((position>0) &&
		((exp->m_pElementList+position-1)->Type!=11) &&
		((exp->m_pElementList+position-1)->Type!=12) &&
		(((exp->m_pElementList+position-1)->Type!=2) || ((exp->m_pElementList+position-1)->pElementObject->Data1[0]!=(char)0xFF)))
	{
		if (((exp->m_pElementList+position)->Type==2) && ((position<orig_position-2) || (exp!=this)))
		{
			char cc=(exp->m_pElementList+position)->pElementObject->Data1[0];
			if ((cc=='=') || (cc=='<') || (cc=='>') || (cc==(char)0xB9) || (cc==(char)0xA3) || (cc==(char)0xB3) || (cc==1) || (cc==2)) found_equality=position+1;
		}
		position--;
	}

	if ((exp->m_pElementList+position)->Type==2)
	{
		//if the very first operator in the line is equality, then we are not aligning to equality (no tab insertion)
		char cc=(exp->m_pElementList+position)->pElementObject->Data1[0];
		if ((cc=='=') || (cc=='<') || (cc=='>') || (cc==(char)0xB9) || (cc==(char)0xA3) || (cc==(char)0xB3) || (cc==1) || (cc==2)) found_equality=0;
	}

	int cnt=0;
	while (((exp->m_pElementList+position)->Type==2) && ((exp->m_pElementList+position)->pElementObject->Data1[0]==9))
	{
		InsertEmptyElement(position,1,0,Toolbox->GetFormattingColor());DeleteElement(position); //dirty trick to prevent memory re-reservation during next instruction
		InsertElement(exp->m_pElementList+position,orig_position);
		position++;
		orig_position++;
		cnt++;
		
	}
	if ((cnt==0) && (stop_on_equality) && (found_equality))
	{
		found_equality--;
		int ActualSize=this->GetActualFontSize(ViewZoom);
		int x=(exp->m_pElementList+found_equality)->X_pos*10/ActualSize-4;
		if (x<2) x=2;
	
		while (x>0)
		{
			int tt=x;
			if (tt>255) tt=255;
			InsertEmptyElement(orig_position,2,9);
			(this->m_pElementList+orig_position)->pElementObject->Data1[3]=(char)tt;
			orig_position++;
			x-=tt;
			x-=3;
			cnt++;
		}
		
	}
	if ((cnt==0) && (stop_on_equality==0))
	{
		//inserting standard tab
		InsertEmptyElement(orig_position,2,9);
		return 1;
	}
	return cnt;
}


char InhibitParentheseMerging=0;
extern char IsSpacebarOnlyHit;

#pragma optimize("s",on)
int CExpression::ResolveKnownFunctions(CDC* DC, short zoom, UINT nChar, UINT nRptCnt, UINT nFlags,int fcolor, tElementStruct *theElement)
{
	char IsText=0;
	if (theElement->pElementObject) IsText=theElement->pElementObject->m_Text;

	if (theElement->Type!=1) return 0; //crashes without this line - strange
	if (InhibitAllKeyHandling) return 0;

	if ((theElement->pElementObject->Data1[0]=='\'')  &&
		(theElement->pElementObject->Data1[1]>'\'') &&
		(IsText==0))
	{	
		//special handling - if a variable begins with the ' then, the ' is deleted
		//this is used in simple and very simple variable mode to write down long variable names
		int meas=0;
		if (theElement->pElementObject->IsMeasurementUnit()) meas=1;
		memmove(theElement->pElementObject->Data1,theElement->pElementObject->Data1+1,23);
		memmove(theElement->pElementObject->Data2,theElement->pElementObject->Data2+1,23);
		if (meas) theElement->pElementObject->m_VMods=0x10;
		m_KeyboardCursorPos--;
		return 1;
	}




	//predefined commands
	if (strnicmp(theElement->pElementObject->Data1,"\\big",5)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->ChangeFontSize((float)(1.2));
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\bbig",6)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->ChangeFontSize((float)(1.2*1.2*1.2));
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\small",7)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		if (m_FontSize>50) this->ChangeFontSize((float)(1/1.2));
		return 2;
	}

	//headlines \h1, \h2, \h3
	if ((strnicmp(theElement->pElementObject->Data1,"\\h",2)==0) &&
		(theElement->pElementObject->Data1[2]>='1') &&
		(theElement->pElementObject->Data1[2]<='3') &&
		(theElement->pElementObject->Data1[3]==0))
	{
		theElement->pElementObject->Data1[0]=0;
		float fontsize=(float)(250.0/(float)this->m_FontSize);
		int size=theElement->pElementObject->Data1[2];
		if (size=='2') fontsize=(float)(207.4/(float)this->m_FontSize);
		if (size=='3') fontsize=(float)(144.0/(float)this->m_FontSize);
		fontsize=fontsize*(float)DefaultFontSize/(float)100.0;
		this->m_KeyboardCursorPos=0;
		this->ChangeFontSize(fontsize);
		theElement->pElementObject->m_Text=1;
		this->m_ModeDefinedAt=0x4001;
		this->m_IsHeadline=size-'0';
		Toolbox->InvalidateRect(NULL,0);
		return 2;
	}
	
	if (strnicmp(theElement->pElementObject->Data1,"\\red",5)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->m_Color=1;
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\green",7)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->m_Color=2;
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\blue",6)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->m_Color=3;
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\gray",6)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->m_Color=4;
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\black",7)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		this->m_Color=0;
		return 2;
	}
	if (strnicmp(theElement->pElementObject->Data1,"\\vert",5)==0)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		CExpression *parent=this;
		while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;
		parent->m_IsVertical=(parent->m_IsVertical)?0:1;
		return 2;
	}
	/*if ((strnicmp(theElement->pElementObject->Data1,"\\text",5)==0)
		&& (this->m_NumElements==1) && (this->m_pPaternalExpression==NULL)) //this is special handling for empty only (otherwise from table)
	{
		theElement->pElementObject->Data1[0]=0;
		this->m_KeyboardCursorPos=0;
		theElement->pElementObject->m_Text=1;
		Toolbox->InvalidateRect(NULL,0);
		this->m_ModeDefinedAt=0x4001;
		return 2;
	}*/

	if ((strcmp(theElement->pElementObject->Data1,"\\dx")==0) ||
		(strcmp(theElement->pElementObject->Data1,"\\dy")==0) ||
		(strcmp(theElement->pElementObject->Data1,"\\dz")==0) ||
		(strcmp(theElement->pElementObject->Data1,"\\dt")==0))
	{
		char t=theElement->pElementObject->Data1[2];
		char f;
		if (Toolbox->m_FontModeSelection==0) f=Toolbox->GetUniformFormatting(); else f=Toolbox->GetMixedFormatting(t,0);
		DeleteElement(m_IsKeyboardEntry-1);
		InsertEmptyElement(m_IsKeyboardEntry-1,6,23,fcolor);
		CExpression *b=(CExpression*)((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1);
		b->InsertEmptyElement(0,1,t,fcolor);
		b->m_pElementList->pElementObject->Data2[0]=f;
		InsertEmptyElement(m_IsKeyboardEntry,1,0,fcolor);
		m_IsKeyboardEntry++;
		m_KeyboardCursorPos=0;
		return 1;
	}
	if ((strcmp(theElement->pElementObject->Data1,"\\ddx")==0) ||
		(strcmp(theElement->pElementObject->Data1,"\\ddy")==0) ||
		(strcmp(theElement->pElementObject->Data1,"\\ddz")==0) ||
		(strcmp(theElement->pElementObject->Data1,"\\ddt")==0))
	{
		char t=theElement->pElementObject->Data1[3];
		char f;
		if (Toolbox->m_FontModeSelection==0) f=Toolbox->GetUniformFormatting(); else f=Toolbox->GetMixedFormatting(t,0);

		DeleteElement(m_IsKeyboardEntry-1);
		InsertEmptyElement(m_IsKeyboardEntry-1,4,'d',fcolor);
		CExpression *b=(CExpression*)((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression2);
		b=(CExpression*)b->m_pElementList->pElementObject->Expression1;
		b->InsertEmptyElement(0,1,t,fcolor);
		b->m_pElementList->pElementObject->Data2[0]=f;
		b=(CExpression*)((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1);
		b=(CExpression*)b->m_pElementList->pElementObject->Expression1;
		KeyboardRefocus(b,0);
		return 2;
	}
	if ((strnicmp(theElement->pElementObject->Data1,"\\func",5)==0) &&
		(strnicmp(theElement->pElementObject->Data1,"\\function",9)!=0))
	{
		char tmp[25];
		memcpy(tmp,theElement->pElementObject->Data1,24);
		tmp[23]=0;
		char greek=0;if ((theElement->pElementObject->Data2[5]&0xE0)==0x60) greek=1;

		{
			//check if the command is in form \funcGreek['] (examples: \funcEpsilon, \funcpi'')
			int jj=(int)strlen(tmp)-1;
			while ((jj>0) && (tmp[jj]=='\'')) jj--;
			jj-=4;
			int ii=0;

			if (jj>1)
			while (ListOfKnownFunctions[ii].type)
			{
				if ((strnicmp(ListOfKnownFunctions[ii].name,tmp+5,jj)==0) && (ListOfKnownFunctions[ii].type==1))
				{
					int x=(int)strlen(tmp)-jj-5;
					if (tmp[5]<='Z') tmp[5]=ListOfKnownFunctions[ii].oper-32; else tmp[5]=ListOfKnownFunctions[ii].oper;
					memset(tmp+6,0,24-6);
					memset(tmp+6,'\'',x);
					greek=1;
					break;
				}
				ii++;
			}
		}
		CExpression *b=((CExpression*)theElement->pElementObject->m_pPaternalExpression);
		int i;
		for (i=0;i<b->m_NumElements;i++)
			if ((b->m_pElementList+i)==theElement) break;
		b->DeleteElement(i);
		b->InsertEmptyElement(i,6,'f',fcolor);
		if ((tmp[5]==0) || (tmp[5]=='\''))
		{
			memmove(tmp+5,tmp+1,8);tmp[13]=0;
		}
		else if (greek) (b->m_pElementList+i)->pElementObject->Data2[0]=0x60;
		strcpy((b->m_pElementList+i)->pElementObject->Data1,tmp+5);
		CExpression *arg=(CExpression*)(b->m_pElementList+i)->pElementObject->Expression1;
		if ((nChar!='(') && (nChar!='[') && (nChar!='{'))
			if (strlen((b->m_pElementList+i)->pElementObject->Data1)<=1) 
			{
				arg->m_ParenthesesFlags=1;
			}
		KeyboardRefocus(arg,0);
		arg->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
		return 2;
		
	}


	if (strnicmp(theElement->pElementObject->Data1,"\\unit",5)==0)
	{
		char tmp[25];
		memcpy(tmp,theElement->pElementObject->Data1,24);
		tmp[23]=0;
		CExpression *b=((CExpression*)theElement->pElementObject->m_pPaternalExpression);
		int i;
		for (i=0;i<b->m_NumElements;i++)
			if ((b->m_pElementList+i)==theElement) break;
		b->DeleteElement(i);
		b->InsertEmptyElement(i,1,'u',fcolor);
		if (tmp[5]==0)
		{
			memmove(tmp+5,tmp+1,8);tmp[13]=0;
		}
		strcpy((b->m_pElementList+i)->pElementObject->Data1,tmp+5);
		//((b->m_pElementList+i)->pElementObject->Data2[0])&=0xE3;
		((b->m_pElementList+i)->pElementObject->m_VMods)=0x10;

		i++;
		InsertEmptyElement(i,1,0,fcolor);
		this->m_IsKeyboardEntry=i+1;
		this->m_KeyboardCursorPos=0;
		this->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
		return 2;
	}

	//comands from list
	int ii=0;
	if (theElement->pElementObject->Data1[0]=='\\')
	if ((theElement->pElementObject->Data2[1]&0xE0)!=0x60)
	while (ListOfKnownFunctions[ii].type)
	{
		char cch=theElement->pElementObject->Data1[1];
		char tmpdata[24];
		memcpy(tmpdata,theElement->pElementObject->Data1+1,23);		
		int num_markings=0;
		int ll=(int)strlen(tmpdata)-1;
		if (tmpdata[0]!='\'')
		{
			while ((ll>0)  && (tmpdata[ll]=='\'')) {ll--;num_markings++;}
			tmpdata[ll+1]=0;
		}

		if (((stricmp(ListOfKnownFunctions[ii].name,tmpdata)==0) && (ListOfKnownFunctions[ii].type==1) && (ll>1)) ||
			(strcmp(ListOfKnownFunctions[ii].name,theElement->pElementObject->Data1+1)==0) ||
			((strcmp(ListOfKnownFunctions[ii].name,theElement->pElementObject->Data1+1)==0) && (ListOfKnownFunctions[ii].type>=100)) ||
			((stricmp(ListOfKnownFunctions[ii].name,theElement->pElementObject->Data1+1)==0) && (ListOfKnownFunctions[ii].type<100) && (ll>1)) ||
			((strcmp(ListOfKnownFunctions[ii].name,theElement->pElementObject->Data1+2)==0) &&
				(ListOfKnownFunctions[ii].type==100) &&
				((cch=='p') || (cch=='n') || (cch=='u') || (cch=='m') || (cch=='c') || (cch=='d') ||
				((cch=='h') || (cch=='k') || (cch=='M') || (cch=='G') || (cch=='T')))))
		{
			if ((ListOfKnownFunctions[ii].type==100) || 
				(ListOfKnownFunctions[ii].type==101))
			{
				if (nChar=='(') goto resolve_known_functions_nounit;
				if (nChar=='_') goto resolve_known_functions_nounit;
			}
			if (ListOfKnownFunctions[ii].type==100)
			{
				//we don't allow all prefiexes in all cases
				if ((strcmp(ListOfKnownFunctions[ii].name,"t")==0) && //tonne (never with smaller prefixes)
					((cch=='p') || (cch=='n') || (cch=='u') || (cch=='m') || (cch=='c') || (cch=='d')))
					goto resolve_known_functions_nounit;
				if (((strcmp(ListOfKnownFunctions[ii].name,"m")) && (strcmp(ListOfKnownFunctions[ii].name,"g"))) &&
					((cch=='c') || (cch=='d')))
					goto resolve_known_functions_nounit;
			}

			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("command entry",20302);
			//we found known function

			DeleteElement(m_IsKeyboardEntry-1);

			if ((ListOfKnownFunctions[ii].type==100) || 
				(ListOfKnownFunctions[ii].type==101))
			{
				//physical units
				InsertEmptyElement(m_IsKeyboardEntry-1,1,'u',fcolor);
				tElementStruct *tsss=m_pElementList+m_IsKeyboardEntry-1;
				if (strcmp(tmpdata+strlen(tmpdata)-3,"ohm")==0)
				{
					tmpdata[strlen(tmpdata)-3]=1;
					tmpdata[strlen(tmpdata)-2]=0;
				}
				if ((strcmp(tmpdata,"deg")==0) || (strcmp(tmpdata,"\'\'\'")==0))
				{
					tmpdata[0]=(char)0xB0;
					tmpdata[1]=0;
				}
				if ((strcmp(tmpdata,"degC")==0) || (strcmp(tmpdata,"\'\'\'C")==0) || (strcmp(tmpdata,"\'C")==0)) //degree celsius
				{
					tmpdata[0]=(char)0xB0;
					tmpdata[1]='C';
					tmpdata[2]=0;
				}

				if (strcmp(tmpdata,"mn")==0) //minute
				{
					strcpy(tmpdata,"min");
				}

				if (strcmp(tmpdata,"Nm")==0) // Nm
				{
					strcpy(tsss->pElementObject->Data1,"N");
					tsss->pElementObject->m_VMods=0x10;
					InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
					tsss=m_pElementList+m_IsKeyboardEntry-1;
					strcpy(tmpdata,"m");
				}
				if (strcmp(tmpdata,"Ns")==0) // Ns
				{
					strcpy(tsss->pElementObject->Data1,"N");
					tsss->pElementObject->m_VMods=0x10;
					InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
					tsss=m_pElementList+m_IsKeyboardEntry-1;
					strcpy(tmpdata,"s");
				}
				{
					for (int iii=0;iii<(int)strlen(tmpdata)+1;iii++)
					{
						char chc=tmpdata[iii];
						unsigned char fnt=0x00;
						if (chc==1) {chc='W';fnt=0x60;} //ohm->omega
						if (chc==0xB0) {chc=(char)0xB0;fnt=(unsigned char)0x70;} //degree

						if ((ListOfKnownFunctions[ii].type==100) && (iii==0))
						{
							if (chc=='u') {chc='m';fnt=(unsigned char)0x70;} //greek micro
						}
						tsss->pElementObject->Data1[iii]=chc;
						tsss->pElementObject->Data2[iii]=fnt;
					}
					tsss->pElementObject->m_VMods=0x10;
				}
				m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
			}
			else
			{
				InsertEmptyElement(m_IsKeyboardEntry-1,ListOfKnownFunctions[ii].type,ListOfKnownFunctions[ii].oper,fcolor);
				theElement=m_pElementList+m_IsKeyboardEntry-1;
				if ((ListOfKnownFunctions[ii].type==1) && (tmpdata[1]))
				{
					//greek symbols
					if ((cch>='A') && (cch<='Z') && (ListOfKnownFunctions[ii].oper>0)) theElement->pElementObject->Data1[0]-=32; //greek symbols upercase and lowercase handling
					if ((cch=='U') && (ListOfKnownFunctions[ii].oper=='u')) theElement->pElementObject->Data1[0]=(char)0xA1;//special handling for uppercase upsilon (varinant of Y)
					UINT rv=0;
					if (ListOfKnownFunctions[ii].oper&0x80) rv=0x60; //special characters like aleph0, N, R, emptyset, infinity, approx
					else if (Toolbox->m_FontModeSelection==1) rv=(Toolbox->GetMixedFormatting(theElement->pElementObject->Data1[0],1)&0xFFFFFF1F)|0x60;
					else rv=(Toolbox->GetUniformFormatting()&0xFFFFFF1F)|0x60;
					theElement->pElementObject->Data2[0]=rv&0xFF;
					theElement->pElementObject->m_VMods=(rv>>16)&0xFF;
					for (int jj=0;jj<num_markings;jj++)
					{
						theElement->pElementObject->Data1[jj+1]='\'';
						theElement->pElementObject->Data2[jj+1]=0;
					}
					theElement->pElementObject->Data1[num_markings+1]=0;
				}
			}
			if (ListOfKnownFunctions[ii].type==2) //an operator
			{
				InsertEmptyElement(m_IsKeyboardEntry,1,0,fcolor);
				(m_pElementList+m_IsKeyboardEntry)->pElementObject->m_Text=IsText;
				m_IsKeyboardEntry++;
				m_KeyboardCursorPos=0;
			}
			if ((ListOfKnownFunctions[ii].type==7) || //symbol (integral, sum, pi)
				(ListOfKnownFunctions[ii].type==6) || //functon
				(ListOfKnownFunctions[ii].type==8) || //root
				(ListOfKnownFunctions[ii].type==10) || //condition list
				(ListOfKnownFunctions[ii].type==9) || //hyperlink, label, condition list
				(ListOfKnownFunctions[ii].type==5) || //parentheses
				(ListOfKnownFunctions[ii].type==4))   //fraction
			{
				//m_IsKeyboardEntry=0;
				CExpression *NextObject=((CExpression*)(theElement->pElementObject->Expression1));
				if ((NextObject->m_pElementList->Type==6) && (ListOfKnownFunctions[ii].type==4))
					NextObject=((CExpression*)(NextObject->m_pElementList->pElementObject->Expression1));
				if ((theElement->Type==8) && (theElement->pElementObject->Expression2) && (((CExpression*)theElement->pElementObject->Expression2)->m_pElementList->Type==0))
					NextObject=((CExpression*)(theElement->pElementObject->Expression2)); //for \root we jump into expression2

				KeyboardRefocus(NextObject,0);
				/*NextObject->InsertEmptyElement(0,1,0,fcolor);
				NextObject->m_IsKeyboardEntry=1;
				NextObject->m_KeyboardCursorPos=0;
				KeyboardEntryObject=(CObject*)NextObject;*/
				((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
				return 2;
			}
			KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
			return 2;
		}
		ii++;
	}
resolve_known_functions_nounit:
	if (theElement->pElementObject->Data1[0]=='\\')
	{
		
		unsigned char ccc=theElement->pElementObject->Data1[1];
		
		if ((ccc>='@') && (ccc!='{') && (ccc!='}') && (ccc!='[') && (ccc!=']') && (ccc!='_') && (ccc!='|'))
		{
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("command entry",20302);
			char tmpdata[24];
			memcpy(tmpdata,theElement->pElementObject->Data1+1,23);
			int greek=0;if ((theElement->pElementObject->Data2[1]&0xE0)==0x60) greek=1;
			DeleteElement(m_IsKeyboardEntry-1);
			if (strcmp(tmpdata,"lim")==0)
				InsertEmptyElement(m_IsKeyboardEntry-1,6,25,fcolor);
			else
			{
				InsertEmptyElement(m_IsKeyboardEntry-1,6,'X',fcolor);	
				if (greek) ((m_pElementList+m_IsKeyboardEntry-1))->pElementObject->Data2[0]=0x60;
			}
			theElement=m_pElementList+m_IsKeyboardEntry-1;

			memcpy(theElement->pElementObject->Data1,tmpdata,23);
			KeyboardRefocus((CExpression*)theElement->pElementObject->Expression1,0);
			if ((tmpdata[1]==0) || ((tmpdata[strlen(tmpdata)-1]<='9')))
			{
				//adding parentheses over single-letter functions or those whose name ends with a number ;
				((CExpression*)theElement->pElementObject->Expression1)->m_ParenthesesFlags=1;
				if ((nChar=='(') || (nChar=='[') || (nChar=='{'))
				{
					((CExpression*)theElement->pElementObject->Expression1)->m_ParentheseShape=nChar;
					nChar=6; //don't allow adding another parenthese
				}
			}

			((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
			return 2;
		}
	}
	return 0;
}

//changes keyboard input focus to new object
#pragma optimize("s",on)
int CExpression::KeyboardRefocus(CExpression* new_focus, int position)
{
	if (new_focus==NULL) return 0;
	if (new_focus->m_pPaternalElement==0)
			KeyboardIndexMode=0;	
	else
	{
		if ((new_focus->m_pPaternalElement->m_Type==1) && ((CObject*)new_focus->m_pPaternalExpression==new_focus->m_pPaternalElement->Expression1)) KeyboardIndexMode=0;
		if (((new_focus->m_pPaternalElement->m_Type==5) || (new_focus->m_pPaternalElement->m_Type==6)) && ((CObject*)new_focus->m_pPaternalExpression==new_focus->m_pPaternalElement->Expression2)) KeyboardIndexMode=0;
	}
	if ((new_focus->m_pPaternalElement==0) || (new_focus->m_pPaternalElement->m_Type!=3))
			KeyboardExponentMode=0;

	m_IsKeyboardEntry=0;
	m_KeyboardCursorPos=0;
	if (position>new_focus->m_NumElements) position=new_focus->m_NumElements;
	if ((position>0) && ((new_focus->m_pElementList+position-1)->Type==0)) position--;
	new_focus->InsertEmptyElement(position,1,0,Toolbox->GetFormattingColor());
	new_focus->m_IsKeyboardEntry=position+1;
	new_focus->m_KeyboardCursorPos=0;
	KeyboardEntryObject=(CObject*)new_focus;
	CDC *mdc=Toolbox->GetDC();
	if ((!IsDrawingMode) && (KeyboardEntryBaseObject)) Toolbox->PaintTextcontrolbox(mdc);
	Toolbox->ReleaseDC(mdc);

	return 1;
}

/*int CExpression::FuseDifferentials() //BABA2
{
	//check if there is any differential opperator and fuse it with its argument

	tElementStruct *ts=this->m_pElementList;
	for (int i=0;i<this->m_NumElements-1;i++,ts++)
	if (ts->Type==6)
	{
		CElement *elm=ts->pElementObject;
		if (elm->Expression1==NULL)
		if (elm->IsDifferential())
		{
			//now add the next element into the function argument
			CExpression *arg=new CExpression(elm,this,elm->FontSizeForType(1));
			elm->Expression1=(CObject*)arg;
			arg->InsertElement(ts+1,0);
			this->DeleteElement(i+1);
			if ((KeyboardEntryObject==(CObject*)this) && (m_IsKeyboardEntry>i))
				m_IsKeyboardEntry--;
		}
	}

	//now fuse cases of d^2
	if ((this->m_NumElements==1) && (this->m_pElementList->Type==6) && (this->m_pElementList->pElementObject) &&
		(this->m_pElementList->pElementObject->Expression1==NULL) &&
		(this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==3) && (this->m_pPaternalElement->Expression1==(CObject*)this))
	{
		CElement *elm=this->m_pElementList->pElementObject;
		if (elm->IsDifferential())
		{
			CExpression *p=this->m_pPaternalExpression;
			for (int i=0;i<p->m_NumElements;i++)
			if ((p->m_pElementList+i)->pElementObject==this->m_pPaternalElement)
			{
				//now add the next element into the function argument
				CExpression *arg=new CExpression(elm,this,elm->FontSizeForType(1));
				elm->Expression1=(CObject*)arg;
				arg->InsertElement(p->m_pElementList+i+1,0);
				p->DeleteElement(i+1);
				if ((KeyboardEntryObject==(CObject*)p) && (p->m_IsKeyboardEntry>i))
					p->m_IsKeyboardEntry--;				
			}
		}
	}

	//now fuse cases of d/d
	if ((this->m_NumElements==1) && (this->m_pElementList->Type==6) && (this->m_pElementList->pElementObject) &&
		(this->m_pElementList->pElementObject->Expression1==NULL) &&
		(this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==4) && (this->m_pPaternalElement->Expression1==(CObject*)this))
	{
		CElement *elm=this->m_pElementList->pElementObject;
		if (elm->IsDifferential())
		{
			CExpression *p=this->m_pPaternalExpression;
			for (int i=0;i<p->m_NumElements;i++)
			if ((p->m_pElementList+i)->pElementObject==this->m_pPaternalElement)
			{
				//now add the next element into the function argument
				CExpression *arg=new CExpression(elm,this,elm->FontSizeForType(1));
				elm->Expression1=(CObject*)arg;
				arg->InsertElement(p->m_pElementList+i+1,0);
				p->DeleteElement(i+1);
				if ((KeyboardEntryObject==(CObject*)p) && (p->m_IsKeyboardEntry>i))
					p->m_IsKeyboardEntry--;				
			}
		}
	}

	ts=this->m_pElementList;
	for (int i=0;i<this->m_NumElements-1;i++,ts++)
	{
		CElement *elm=ts->pElementObject;
		if (elm)
		{
			if (elm->Expression1) ((CExpression*)elm->Expression1)->FuseDifferentials();
			if (elm->Expression2) ((CExpression*)elm->Expression2)->FuseDifferentials();
			if (elm->Expression3) ((CExpression*)elm->Expression3)->FuseDifferentials();
		}
	}
	return 1;
}*/

extern struct TOOLBOX_FONT_FORMATING ToolboxFontFormating;


CExpression *LastEditedExpression;

//char prevtypingmode=-1;
//char prevtypingmodetimer=0;
//char ExponentAcceleratorUsed=0;
//char IndexAcceleratorUsed=0;
char KeyboardEntryString[32];
int KeyboardEntryStringLen=0;
//int DisableAutocomplete=0;
DWORD KeyboardEntryStringTime;


//int IndexModeWasUsed=0;
//int ExponentModeWasUsed=0;
int KeyboardCursorWasAttached=0;
int KeyboardCursorIsAttached=0;
unsigned int lastkeystroketime;
unsigned int prevkeystroketime;
int DotAutomaticallyAdded=0;
extern char SpacebarPressedRecently;

CExpression *LastCursorPositionExpression;
int LastCursorPositionPosition;
int LastCursorPositionCursorPos;
tDocumentStruct *LastCursorPositionBaseObject;
char LastCursorPositionKey;

char IsDoubleStrokeConversionUsed;
//this function receives messages when a key is hit (handling of keyboard)
#pragma optimize("s",on)
int CExpression::KeyboardKeyHit(CDC* DC, short zoom, UINT nChar, UINT nRptCnt, UINT nFlags,int fcolor,char is_external)
{
	//******************************************************************************
	//this function receives already prepared key codes (in 'nChar')
	//Key codes are prepared in CMathomirView and are assembled from WM_CHAR, WM_KEYDOWN, WM_SYSKEYDOWN and WM_TIMER
	//Some virtual key codes ( 1<nChar<9 ... 14<nChar<29 ) can also be received - see CMathomirView for details
	//******************************************************************************

	//error handling
	if (!m_IsKeyboardEntry) return 0;
	if (m_IsKeyboardEntry==0x7FFF) {m_IsKeyboardEntry=0;return 0;}
	if (m_IsKeyboardEntry<0) {m_IsKeyboardEntry=0;return 0;}
	if (m_IsKeyboardEntry>m_NumElements) {m_IsKeyboardEntry=0;return 0;}
	if (m_KeyboardCursorPos<0) {m_IsKeyboardEntry=0;return 0;}
	if (m_KeyboardCursorPos>23) m_KeyboardCursorPos=23;
	

	//m_IsKeyboardEntry contains ordinal number (with offset +1) of the element that 
	//is being edited. The edited element can only be variable (type==1)
	tElementStruct *theElement;
	theElement=m_pElementList+m_IsKeyboardEntry-1;	

	//some more error handling
	if (theElement->pElementObject==NULL) {m_IsKeyboardEntry=0;return 0;} //element not defined
	if ((theElement->Type!=1) && (theElement->Type!=6)) {m_IsKeyboardEntry=0;return 0;} //not variable

	//determining the keyboard mode: math (for editing formulas) or text (for editing plain text)
	char IsText=0;
	if ((theElement->Type==1) && ((theElement->pElementObject->m_Text) || ((SpacebarPressedRecently) && (is_external==2)))) IsText=1;

	//if pure number has its double IEEE value coded inside, just delete it
	if ((theElement->pElementObject->Data1[14]==0) && ((theElement->pElementObject->Data1[15]&0x7F)==126)) theElement->pElementObject->Data1[15]=0;

	//prepare some frequently used data
	CElement *EditedVariable=theElement->pElementObject;
	bool IsEditedVariableEmpty=(strlen(EditedVariable->Data1)==0)?1:0;

	//int fcolor=Toolbox->GetFormattingColor();

	//int IsALTDown=((GetKeyState(VK_MENU)&0xFFFE))?1:0;

	if (((GetKeyState(VK_SPACE)&0xFFFE)==0) && (KeyboardIndexMode)) //ending the index mode (space is released)
	{
		KeyboardIndexMode=0;
		unsigned char c1=lastkeystrokes[0];
		if ((c1!=' ') && (c1!='+') && (c1!='-') && (c1!='/') && (c1!='*') && (c1!='\\'))
		if ((m_pPaternalElement) && 
			(((m_pPaternalElement->m_Type==1) && (m_pPaternalElement->Expression1==(CObject*)this)) ||
			(((m_pPaternalElement->m_Type==6) || (m_pPaternalElement->m_Type==5)) && (m_pPaternalElement->Expression2==(CObject*)this))))
		{
			int is=IsSHIFTDown;IsSHIFTDown=0;
			int ia=IsALTDown;IsALTDown=0;
			this->KeyboardKeyHit(DC,zoom,0x0D,nRptCnt,0,0,1);
			IsSHIFTDown=is;
			IsALTDown=ia;
			//IndexAcceleratorUsed=1;
		}
		//IndexModeWasUsed=1;
		
		((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,1);
		((CExpression*)KeyboardEntryObject)->Autocomplete(0);
		return (AutocompleteSource)?2:1;
	}

	if (((GetKeyState(VK_MENU)&0xFFFE)==0) && (KeyboardExponentMode)) //ending the exponent mode (ALT is released)
	{
		KeyboardExponentMode=0;

		unsigned char c1=lastkeystrokes[0];
		if ((c1!=' ') && (c1!='+') && (c1!='-') && (c1!='/') && (c1!='*') && (c1!='\\'))
		if ((m_pPaternalElement) && 
			((m_pPaternalElement->m_Type==3) && (m_pPaternalElement->Expression2==(CObject*)this)))
		{
			int is=IsSHIFTDown;IsSHIFTDown=0;
			int ia=IsALTDown;IsALTDown=0;
			this->KeyboardKeyHit(DC,zoom,0x0D,nRptCnt,0,0,1);
			IsSHIFTDown=is;
			IsALTDown=ia;
			//ExponentAcceleratorUsed=1;

		}
		//ExponentModeWasUsed=1;
		((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,1);
		((CExpression*)KeyboardEntryObject)->Autocomplete(0);
		return (AutocompleteSource)?2:1;
	}

	if (((KeyboardExponentMode) && (this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==3) && (this->m_pPaternalElement->Expression2==(CObject*)this)))
	{
		//to make typing of the '+' sign easier with the Alt key (no need to hold the shift on some keyboards)
		unsigned char ch=(nChar)&0xFF;
		if (ch=='=') nChar='+';
	}

	if ((theElement->Type==1) && (theElement->pElementObject) && (theElement->pElementObject->Data1[0]==0) && (theElement->pElementObject->m_Text>1))
		theElement->pElementObject->m_Text=1; //if the variable is empty, then force normal text (no splicing left or right)

	if ((nChar&0xFF)==6) //WM_TIMER message (sent as character code 6)
	{
		//periodically check if there is anything in the clipboard to be inserted at the keyboard cursor positon
		if ((ClipboardExpression) && (ClipboardExpression->m_NumElements>0) && (m_IsKeyboardEntry) && ((GetKeyState(VK_LBUTTON)&0xFFFE)==0))
		{
			//we are going to copy the clipboard at the current cursor position

			LastCursorPositionExpression=NULL;

			//first resolve known function, if users used any
			if (ResolveKnownFunctions(DC,zoom,nChar,nRptCnt,nFlags,fcolor,theElement))
			{
				IsSpacebarOnlyHit=0;
				if (!m_IsKeyboardEntry) return 1;
				theElement=m_pElementList+m_IsKeyboardEntry-1;
				EditedVariable=theElement->pElementObject;
				IsEditedVariableEmpty=(strlen(EditedVariable->Data1)==0)?1:0;
			}

			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("insert into",20303);

			theElement=KeyboardSplitVariable();
			if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;

			//special handling for tekst mode
			if ((this->m_pPaternalExpression==NULL) && (this->m_NumElements==1) &&
				(this->m_pElementList->Type==1) && (m_IsKeyboardEntry==1) && (m_KeyboardCursorPos==0))
			{
				if ((ClipboardExpression->m_NumElements==1) &&
					(ClipboardExpression->m_pElementList->Type==5) &&
					(((CExpression*)(ClipboardExpression->m_pElementList->pElementObject->Expression1))->m_StartAsText))
				{
					m_StartAsText=1; //convert the expression in text mode
					m_Alignment=1;
					m_pElementList->pElementObject->m_Text=1;

					//clears clipboard
					delete ClipboardExpression;
					ClipboardExpression=NULL;
					return 1;
				}
			}

			if (IsEditedVariableEmpty)
				DeleteElement(m_IsKeyboardEntry-1);
			else
				m_IsKeyboardEntry++;

			
			if (!ClipboardExpression) return 1;
			//just check if there is an empty variable at the end - delete it (we don't want empty variables to propagate)
			tElementStruct *tss=ClipboardExpression->m_pElementList+ClipboardExpression->m_NumElements-1;
			if ((tss->Type==1) && ((tss->pElementObject->Data1[0]==0) || (tss->pElementObject->Data1[0]==32)))
				ClipboardExpression->DeleteElement(ClipboardExpression->m_NumElements-1);
			

			//now we check if there is a keyboard selection that can be inserted
			//into clipboard element before pasting (implanting)
			if ((ClipboardExpression->m_NumElements==1) && 
				(ClipboardExpression->m_pElementList->pElementObject))
			{
				CExpression *ex=(CExpression*)ClipboardExpression->m_pElementList->pElementObject->Expression1;
				if ((ex) && (ex->m_NumElements==1) && (ex->m_pElementList->Type==0))
				{
					for (int jj=0;jj<m_NumElements;jj++)
					{
						if ((m_pElementList+jj)->IsSelected==2)
						{
							ex->InsertElement(m_pElementList+jj,ex->m_NumElements);
							DeleteElement(jj);
							if (m_IsKeyboardEntry-1>jj) m_IsKeyboardEntry--;
							jj--;
						}
					}		
				}
			}


			int NumAdded=0;
			if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
				((CExpression*)(KeyboardEntryBaseObject->Object))->DeselectExpression();

			m_Selection=m_IsKeyboardEntry;
			m_IsRowInsertion=0;
			m_IsColumnInsertion=0;

			int tmp=m_NumElements; 
			if ((tmp==1) && (m_pElementList->Type==0)) tmp=0;
			CopyAtPoint(NULL,ViewZoom,-1,-1,ClipboardExpression);
			if (ClipboardExpression) ClipboardExpression->AddToStackClipboard(0);
			m_Selection=0;
			int newtmp=m_NumElements;
			if ((newtmp==1) && (m_pElementList->Type==0)) newtmp=0;

			m_IsKeyboardEntry+=newtmp-tmp;
			NumAdded=newtmp-tmp;
 
			for (int oo=0;oo<NumAdded;oo++)
			{
				int trt=m_IsKeyboardEntry-NumAdded-1+oo;
				if ((trt<0) || (trt>m_NumElements)) break;

				theElement=m_pElementList+trt;

				if ((theElement->pElementObject) && (theElement->pElementObject->m_Color==-1))
					theElement->pElementObject->m_Color=fcolor;
				
				if ((ClipboardExpression->m_pElementList+oo)->Type!=1)
				{
					if ((theElement->pElementObject) && (theElement->pElementObject->Expression1))
					{
						CExpression *e=(CExpression*)(theElement->pElementObject->Expression1);

						if ((theElement->Type==8) && (theElement->pElementObject->Expression2) &&
							(((CExpression*)theElement->pElementObject->Expression2)->m_pElementList->Type==0))
							e=(CExpression*)(theElement->pElementObject->Expression2); //special handling for root symbol
						
						if ((e->m_pElementList->Type==0) ||
							((e->m_pElementList->Type==5) && (((CExpression*)e->m_pElementList->pElementObject->Expression1)->m_pElementList->Type==0)) ||
							((e->m_pElementList->pElementObject->IsDifferential()) &&
							(((CExpression*)e->m_pElementList->pElementObject->Expression1)->m_NumElements==1) && 
							(((CExpression*)e->m_pElementList->pElementObject->Expression1)->m_pElementList->Type==0)))
						{
							//the inserted object has expresson 1 empty (only dummy elemnt inside, or and empty 'd' element)
							//we will continue keyboard input on that expression
							CExpression *newone=e;
							if ((e->m_pElementList->Type==6) && (e->m_pPaternalElement) && (e->m_pPaternalElement->m_Type==4))
								newone=(CExpression*)(e->m_pElementList->pElementObject->Expression1);
							if (e->m_pElementList->Type==5)
								newone=(CExpression*)(e->m_pElementList->pElementObject->Expression1);

							m_Selection=0;
							KeyboardRefocus(newone,0);
							
							delete ClipboardExpression; //clears clipboard
							ClipboardExpression=NULL;
							return 1;
						}
				
					}
				}
			}

			//opens new variable for editing

			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
			if (m_IsKeyboardEntry<=m_NumElements)
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
			m_KeyboardCursorPos=0;
			IsEditedVariableEmpty=1;

			//clears clipboard
			delete ClipboardExpression;
			ClipboardExpression=NULL;
		}


		//we are changing cursor state (the message is received approx every 300ms)
		if ((CursorBlinkState) && (GetTickCount()-lastkeystroketime>750))
			CursorBlinkState=0;
		else
			CursorBlinkState=1;
		return 1;
	}

	if ((IsEditedVariableEmpty) && (theElement->Type==1))
	{
		theElement->pElementObject->m_Color=fcolor;
	}
	
	if (is_external)
	{ 
		CursorBlinkState=1;
	}

	if ((is_external) && (nChar==10)) //changing to the uppercase
	{
		if (IsEditedVariableEmpty) return 1;
		if (theElement->Type!=1) return 1;
		int len=(int)m_KeyboardCursorPos-1;//(int)strlen(theElement->pElementObject->Data1)-1;
		if (len<0) return 1;
		if (lastkeystrokes[0]==theElement->pElementObject->Data1[len]) 
		{
			lastkeystrokes[0]=toupper(lastkeystrokes[0]);
		}
		if (KeyboardEntryStringLen>0)
			if (KeyboardEntryString[KeyboardEntryStringLen-1]==theElement->pElementObject->Data1[len]) 
				KeyboardEntryString[KeyboardEntryStringLen-1]=toupper(KeyboardEntryString[KeyboardEntryStringLen-1]);

		theElement->pElementObject->Data1[len]=toupper(theElement->pElementObject->Data1[len]);

		nFlags=Toolbox->GetUppercaseFormatting(theElement->pElementObject->Data1[len],theElement->pElementObject->Data2[len]);
		theElement->pElementObject->Data2[len]=nFlags&0xFF;
		theElement->pElementObject->m_VMods=(nFlags>>16)&0xFF;

		return 1;
	}

	//
	//heuristicly determine if we are typing text into the math box (if yes, we will convert the math box into text box)
	//
	if ((!IsText) && (is_external) && (KeyboardEntryBaseObject))
	{
		//detect if we are typing into index (typing into index is normaly used in Very Simple Variable Mode)
	
		CExpression *exp=this;

		{
			//all keypresses are stored into the KeyboardEntryString array
			if (LastEditedExpression!=exp)
			{
				KeyboardEntryString[0]=0;
				KeyboardEntryStringLen=-1;
			}
			
			if ((IsEditedVariableEmpty) && (this->m_NumElements==1) && 
				(this->m_pPaternalExpression==NULL) && (KeyboardEntryBaseObject->Object==(CObject*)this))
			{
				KeyboardEntryStringLen=0;
				KeyboardEntryStringTime=GetTickCount();
			}
			LastEditedExpression=exp;



			if ((KeyboardEntryStringLen>=0) && (KeyboardEntryStringLen<31))
			{
				if ((KeyboardEntryStringLen) && (nChar==8))
					KeyboardEntryStringLen--; //backspace support
				else 
				{
					if ((KeyboardEntryStringLen==0)  || (KeyboardEntryString[KeyboardEntryStringLen-1]!=' ') || (nChar!=' '))
					{
						KeyboardEntryString[KeyboardEntryStringLen++]=nChar;
						if (KeyboardExponentMode) KeyboardEntryString[KeyboardEntryStringLen++]=0; //this will disable transformation
					}
				}
			}

			if ((KeyboardEntryStringLen>0) && (KeyboardEntryStringLen<30))
			{
				int is_text=1;
				int has_space=0;
				int has_nonspace=0;
				for (int i=0;i<KeyboardEntryStringLen;i++)
				{
					unsigned char ch=(unsigned char)KeyboardEntryString[i];

					if ((ch=='\'') && (i==0)) is_text=0;
					if (ch<32) is_text=0;
					if ((ch=='_') || (ch=='^') || (ch=='\\')) is_text=0;
					if (ch==32) has_space++;
					if (i>0)
					{
						unsigned char ch2=(unsigned char)KeyboardEntryString[i-1];
						if ((ch=='\'') && (ch2==' ')) is_text=0;
						if ((ch!=32) && (ch2!=32) && ((ch<'0') || (ch>'9'))) has_nonspace++;
						if ((ch=='x') && ((ch2=='y') || (ch2=='z'))) is_text=0;
						if ((ch=='y') && ((ch2=='x') || (ch2=='z'))) is_text=0;
						if ((ch=='z') && ((ch2=='x') || (ch2=='y'))) is_text=0;
					}
				}

				for (int i=0;i<exp->m_IsKeyboardEntry;i++)
					if ((exp->m_pElementList+i)->Type!=1) 
					{
						if ((exp->m_pElementList+i)->Type==2)
						{
							char cch=(exp->m_pElementList+i)->pElementObject->Data1[0];
							if ((cch=='-') && (i==0)) continue; //dash is allowed at the beginning
							if ((cch==',') && (i!=0)) continue; //coma is allowd inside
						}
						is_text=0;
						break;
					}
					else
					{
						if (((exp->m_pElementList+i)->pElementObject->m_VMods)==0x10) {is_text=0;break;} //a measurement unit
					}
		
				int isOK=0;
				if ((KeyboardEntryStringLen==2) && (KeyboardEntryString[1]==' ') && (KeyboardEntryString[0]=='-')) isOK=1;
				if ((KeyboardEntryStringLen==1) && (KeyboardEntryString[0]=='\"')) isOK=1;
			
				if (IsSimpleVariableMode)
						if (((has_space) || (has_nonspace>5)) && (has_nonspace>2) && (has_nonspace/2+1>has_space) && (KeyboardEntryStringLen>3)) isOK=1;
				if (!IsSimpleVariableMode)
						if ((has_space>1) && (has_nonspace>2) && (has_nonspace/2+1>has_space) && (KeyboardEntryStringLen>3)) isOK=1;

				int typerate=(GetTickCount()-KeyboardEntryStringTime)/KeyboardEntryStringLen;
				if ((is_text) && (isOK)&& (typerate<300) && (nChar!='.'))
				{
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("text conversion",20314);

					tDocumentStruct *bo=KeyboardEntryBaseObject;
					while (exp->m_pElementList->Type) exp->DeleteElement(0);
					KeyboardEntryBaseObject=bo;

					exp->InsertEmptyElement(0,1,0,fcolor);
					exp->m_pElementList->pElementObject->m_Text=1;
					

					for (int i=0;i<KeyboardEntryStringLen;i++)
					{
						char ch=KeyboardEntryString[i];
						if (ch==' ') 
						{
							exp->InsertEmptyElement(exp->m_NumElements,1,0,fcolor);
							(exp->m_pElementList+exp->m_NumElements-1)->pElementObject->m_Text=1;
							continue;
						}
						int l=(int)strlen((exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Data1);
						(exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Data1[l]=ch;
						(exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Data1[l+1]=0;
						(exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Data2[l]=//KeyboardEntryFonts[i];
							Toolbox->GetUniformFormatting();
					}
					exp->m_IsKeyboardEntry=exp->m_NumElements;
					exp->m_KeyboardCursorPos=(int)strlen((exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Data1);
					KeyboardEntryStringLen=-1;
					KeyboardEntryObject=(CObject*)exp;

					IsText=1;
					m_StartAsText=1;
					m_ModeDefinedAt=1;
					if (IsText) m_ModeDefinedAt|=0x4000;
					Toolbox->KeyboardHit(6,0); //just to make the toolbox aware that the text mode is activated
					fcolor=Toolbox->GetFormattingColor();
					for (int i=0;i<exp->m_NumElements;i++)
						if ((exp->m_pElementList+i)->pElementObject) (exp->m_pElementList+i)->pElementObject->m_Color=fcolor;

					
					//m_ModeDefinedAt=0;

					IsSpacebarOnlyHit=0;
					goto show_keymode_label;

					
					return 1;
				}
			}
		}
	}


	/*if (BackspaceStorage)
	{
		if (this!=BackspaceStorageOriginal)
		{
			delete BackspaceStorage;
			BackspaceStorage=NULL;
		}
		else if (((nChar&0xFF)!=0x08) && ((nChar&0xFF)!=0x07))
		{
			StoreBackspaceStorage();
		}
			
	}*/

	//if ((nChar&0xFF)!=0x08) {ExponentAcceleratorUsed=0;IndexAcceleratorUsed=0;}

	if (is_external==1) //when quick-typing then is_external==2 (we don't want to destroy the lastkeystrokes with quick typing)
	{ 
		if (previouslyEditedExpression!=this)
		{
			//memset(lastkeystrokes,0,10);
			memmove(lastkeystrokes+1,lastkeystrokes,10);
			lastkeystrokes[0]=(char)0xFF; //special flag that the focus was changed to another expression
			KeyboardCursorWasAttached=0;
			KeyboardCursorIsAttached=0;
		}
		previouslyEditedExpression=this;
	}
	if (is_external)
	{
		lastkeystroketime=prevkeystroketime;
		prevkeystroketime=GetTickCount();
		if ((nChar&0xFF)==0x08)
		{
			if (!IsSHIFTDown)
			{
				memmove(lastkeystrokes,lastkeystrokes+1,10);
				memmove(lastkeystrokeselm,lastkeystrokeselm+1,10*sizeof(int));
			}
		}
		else if (((nChar&0xFF)!=' ') || (lastkeystrokes[0]!=' '))
		{
			memmove(lastkeystrokes+1,lastkeystrokes,10);
			memmove(lastkeystrokeselm+1,lastkeystrokeselm,10*sizeof(int));
			/*for (int ii=10;ii>=0;ii--) 
			{
				lastkeystrokes[ii+1]=lastkeystrokes[ii];
				lastkeystrokeselm[ii+1]=lastkeystrokeselm[ii];
			}*/

			lastkeystrokes[0]=(nChar&0xFF);
			lastkeystrokeselm[0]=(int)(theElement-m_pElementList);
		}
	}

	//starting the index mode (Space+letter)
	//if (!IsText)
	{
		char cc=nChar&0xFF;

		if ((GetKeyState(VK_SPACE)&0xFFFE) && (cc!=' ') && (cc!='^') && (cc!='_') && (cc!=4) && (cc!=5) && (cc!=2) && (cc!=3) && (cc!=0x0D) && (KeyboardIndexMode==0))
		{
			//the spacebar shuld be held for some time because we don't want accidental triggering
			for (int ii=0;ii<12;ii++)
			{
				Sleep(10);
				if ((GetAsyncKeyState(VK_SPACE)&0xFFFE)==0) break;
				if ((!IsText) && (ii>5)) break;
			}

			if ((GetAsyncKeyState(VK_SPACE)&0xFFFE))
			{
				KeyboardIndexMode=1;
				if (lastkeystrokes[1]==' ') 
				{
					lastkeystrokes[1]='_';
				}

				this->KeyboardKeyHit(DC,zoom,'_',nRptCnt,0,0,0);
				nFlags=Toolbox->KeyboardHit(nChar,nFlags);
				previouslyEditedExpression=(CExpression*)KeyboardEntryObject;
				return ((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
			}
		}
	}
	
	//starting the exponent mode (ALT+letter)
	if ((IsALTDown)  && ((nChar&0xFF)!='^') && ((nChar&0xFF)!='_') && ((nChar&0xFF)!=' ') && ((nChar&0xFF)!=0x0D) && ((nChar&0xFF)!=',') && ((nChar&0xFF)!='.') && ((nChar&0xFF)>=32) && (KeyboardExponentMode==0))
	{
		KeyboardExponentMode=1;
		if (lastkeystrokes[1]==' ') 
		{
			lastkeystrokes[1]='^';
		}

		//special handling for the 'e' base (changing the format to italic, serif)
		int prev=0;
		if ((m_KeyboardCursorPos==0) && (m_IsKeyboardEntry>1) && (theElement->Type==1) && (theElement->pElementObject->Data1[0]==0)) {theElement--;prev=1;}
		if ((m_KeyboardCursorPos==1-prev) && (theElement->Type==1) && (theElement->pElementObject->Data1[1]==0) && (theElement->pElementObject->Data1[0]=='e') && ((theElement->pElementObject->Data2[0]&0xE0)!=0x60))
		{
			theElement->pElementObject->Data2[0]=0x22;
		}
		
		this->KeyboardKeyHit(DC,zoom,'^',nRptCnt,0,0,0);
		previouslyEditedExpression=(CExpression*)KeyboardEntryObject;
		return ((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
	}

/*
	if (IndexModeWasUsed==1)
	{
		IndexModeWasUsed=2;
	}
	else
		IndexModeWasUsed=0;

	if (ExponentModeWasUsed==1)
	{
		ExponentModeWasUsed=2;
	}
	else
		ExponentModeWasUsed=0;
*/


	if (is_external)
	{
	if (!IsText)
	{
		KeyboardCursorWasAttached=KeyboardCursorIsAttached;
		if ((theElement->Type==1) && (theElement->pElementObject->Data1[0]) && ((nChar&0xFF)!=0x08) &&
			(theElement->pElementObject) &&
			(theElement->pElementObject->Data1[m_KeyboardCursorPos]==0) && (m_KeyboardCursorPos) &&
			((this->m_pPaternalElement==0) || (this->m_pPaternalElement->m_Type!=1)))
		{
			KeyboardCursorIsAttached=1; //the keyboard cursor is on the rightmost position of the variable and we are typing another letter
		}
		else 
			KeyboardCursorIsAttached=0;
	}
	else
		KeyboardCursorWasAttached=KeyboardCursorIsAttached=0;
	}

	//removes autocomplete selection if the spacebar is pressed (two times usually) to enable creating triple-dot symbols and .number entries
	/*if (((nChar&0xFF)==' ') && (m_KeyboardCursorPos==0) && (theElement->Type==1) && 
		(theElement->pElementObject->Data1[0]==0) && (!DisableAutocomplete) && (is_external))
	{
		AutocompleteSource=NULL;
		DisableAutocomplete=1;
	}
	else 
		DisableAutocomplete=0;*/

	if ((nChar&0xFF)==27) //ESC key - immediately exits the keyboard-entry mode
	{
		//Deletes the edited variable if empty
		if (IsEditedVariableEmpty) 
		{
			DeleteElement(m_IsKeyboardEntry-1);
		}
		m_IsKeyboardEntry=0; 
		DeselectExpression();
		return 0;
	}


	//if not clicked at any of cursor moving keys, then we delete prevEmptyBoxType 
	if (((nChar!=0x05) && (nChar!=0x04) && (nChar!=0x03) && (nChar!=0x02) && (is_external)) /*|| (m_NumElements>1) || (m_KeyboardCursorPos>0)*/) 
		prevEmptyBoxType=-1;


	static char SaveObjectMoving;
	if ((KeyboardEntryBaseObject) && ((GetKeyState(VK_SPACE)&0xFFFE)))
	{
		//using SpaceBar+ArrowKey moves the edited equation in very fine steps
		int cc=nChar&0xFF;
		if ((cc>=2) && (cc<=5))
		{
			if (SaveObjectMoving) {((CMainFrame*)theApp.m_pMainWnd)->UndoSave("move",20402);}
			if (cc==0x04) KeyboardEntryBaseObject->absolute_X-=4;
			if (cc==0x05) KeyboardEntryBaseObject->absolute_X+=4;
			if (cc==0x02) KeyboardEntryBaseObject->absolute_Y+=4;
			if (cc==0x03) KeyboardEntryBaseObject->absolute_Y-=4;
			SaveObjectMoving=0;
			pMainView->InvalidateRect(NULL,0);
			pMainView->UpdateWindow();
			return 1;
		
		}
	}
	SaveObjectMoving=1;



	//handling cursor movement - specifically the situation when the oposite arrow key is pressed
	//and then the cursor must be returned exactly where it was before
	{
		char c=nChar&0x0FF;
		if (((c==0x04) || (c==0x05) || (c==0x02) || (c==0x03)) && //left,right,down,up arrow key
			 (!IsALTDown) && 
			 (!IsSHIFTDown) && 
			 (!IsSHIFTALTDown) && 
			 ((GetKeyState(VK_SPACE)&0xFFFE)==0) &&
			 (KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
			 
		{
			if ((LastCursorPositionExpression) && (LastCursorPositionBaseObject) && (LastCursorPositionPosition))
			if (((c==0x04) && (LastCursorPositionKey==0x05)) ||
				((c==0x05) && (LastCursorPositionKey==0x04)) ||
				((c==0x02) && (LastCursorPositionKey==0x03)) ||
				((c==0x03) && (LastCursorPositionKey==0x02)))
			{
				if (IsEditedVariableEmpty)
					RemoveEmptyVariable(DC,theElement);
				m_IsKeyboardEntry=0;
				m_KeyboardCursorPos=0;
				if (LastCursorPositionPosition>LastCursorPositionExpression->m_NumElements+11) LastCursorPositionPosition=LastCursorPositionExpression->m_NumElements+1;
				if ((LastCursorPositionPosition>1) && ((LastCursorPositionExpression->m_pElementList+LastCursorPositionPosition-2)->Type==0)) LastCursorPositionPosition--;
				if (LastCursorPositionCursorPos>=0)
				{
					LastCursorPositionExpression->m_IsKeyboardEntry=LastCursorPositionPosition;
					LastCursorPositionExpression->m_KeyboardCursorPos=LastCursorPositionCursorPos;
				}
				else
				{
					LastCursorPositionExpression->InsertEmptyElement(LastCursorPositionPosition-1,1,0,Toolbox->GetFormattingColor());
					LastCursorPositionExpression->m_IsKeyboardEntry=LastCursorPositionPosition;
					LastCursorPositionExpression->m_KeyboardCursorPos=0;
				}
				KeyboardEntryObject=(CObject*)LastCursorPositionExpression;
				KeyboardEntryBaseObject=LastCursorPositionBaseObject;
				LastCursorPositionExpression=NULL;
				CDC *mdc=Toolbox->GetDC();
				if ((!IsDrawingMode) && (KeyboardEntryBaseObject)) Toolbox->PaintTextcontrolbox(mdc);
				Toolbox->ReleaseDC(mdc);
				return 2;
			}

			if ((m_NumElements>1) || (m_pElementList->Type>1) || (m_pPaternalExpression) || 
				((m_pElementList->Type==1) && (m_pElementList->pElementObject->Data1[0]!=0)))
			{
				if (IsEditedVariableEmpty)
					LastCursorPositionCursorPos=-1;
				else
					LastCursorPositionCursorPos=this->m_KeyboardCursorPos;
				LastCursorPositionExpression=this;
				LastCursorPositionPosition=m_IsKeyboardEntry;
				LastCursorPositionBaseObject=KeyboardEntryBaseObject;
				LastCursorPositionKey=c;
			}
			else
				LastCursorPositionExpression=NULL;
		}
		else
			LastCursorPositionExpression=NULL;
	}



	
	if (!IsALTDown)
	if ((nChar&0xFF)==0x04)  //LEFT cursor arrow key - moves cursor to the left
	{
		DotAutomaticallyAdded=0;
		m_ModeDefinedAt=0;

		if (IsSHIFTDown)
		{
			//select to the left			
			if (m_KeyboardCursorPos) 
			{
				InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
			}
			theElement=KeyboardSplitVariable();			
			
			this->DeselectExpressionExceptKeyboardSelection();


			//now, determine matrix rows and columns of selection
			int row=0,column=0;
			int minc=0x7FFF,minr=0x7FFF,maxc=0,maxr=0;
			int cursor_row=0,cursor_column=0;
			int is_matrix=0;
			for (int i=0;i<m_NumElements;i++)
			{
				tElementStruct *ts=m_pElementList+i;
				if (ts->Type==11) column++;
				else if (ts->Type==12) {column=0;row++;}
				else if ((ts->IsSelected==2) || (i==m_IsKeyboardEntry-1))
				{
					if (column<minc) minc=column;
					if (column>maxc) maxc=column;
					if (row<minr) minr=row;
					if (row>maxr) maxr=row;
				}
				if (i==m_IsKeyboardEntry-1) {cursor_row=row;cursor_column=column;}
			}
			if ((maxr>minr) || (maxc>minc)) is_matrix=1;


			if (!is_matrix)
			{
				int ii=m_IsKeyboardEntry;
				if ((m_IsKeyboardEntry==1) || ((m_pElementList+m_IsKeyboardEntry-2)->IsSelected!=2))
					for (;ii<m_NumElements;ii++)
					{
						if ((m_pElementList+ii)->IsSelected!=2) 
						{
							break;
						}
					}
				if (ii==m_IsKeyboardEntry)
				{
					int has_columns=0;
					for (ii=m_IsKeyboardEntry-2;ii>=0;ii--)
					{
						tElementStruct *ts=m_pElementList+ii;
						if (ts->Type==12) /*if (m_IsText) continue; else*/ break;
						if (ts->Type==11) {has_columns=1;continue;}
						if (ts->IsSelected!=2) break;
					}
					if (ii>=0) if ((m_pElementList+ii)->Type!=12) SelectElement(2,ii);
					if (has_columns) SelectElement(2,m_IsKeyboardEntry-1);

					AdjustSelection(2);
					if (m_IsMatrixElementSelected)
					{
						int ii;
						for (ii=m_IsKeyboardEntry-1;ii<m_NumElements;ii++)
						{
							if (((m_pElementList+ii)->Type==11) ||((m_pElementList+ii)->Type==12))
								break;
						}
						ii--;
						DeleteElement(m_IsKeyboardEntry-1);
						InsertEmptyElement(ii,1,0,fcolor);
						m_IsKeyboardEntry=ii+1;
					}
				}
				else
				{
					SelectElement(0,ii-1);
				}
			}
			else
			{
				if ((cursor_column==minc) && (minc<maxc))
				{
					for (int ii=0;ii<m_MaxNumRows;ii++)
					{
						int k=FindMatrixElement(ii,maxc,0);
						if (k>=0)
						for (int jj=k;jj<m_NumElements;jj++)
						{
							SelectElement(0,jj);
							if ((m_pElementList+jj)->Type==11) break;
							if ((m_pElementList+jj)->Type==12) break;
						}
					}
				}
				else
				{
					minc--;if (minc<0) minc=0;
					int k=FindMatrixElement(minr,minc,0);
					if (k<0) k=0;
					SelectElement(2,k);
					AdjustSelection(2);
				}
			}
			if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

			return 1;
		}

		if (m_KeyboardCursorPos>0)
		{

			m_KeyboardCursorPos--;
			DeselectExpression();
			if ((m_KeyboardCursorPos==0) && (theElement->Type==1) && ((theElement->pElementObject->m_Text==2) || (theElement->pElementObject->m_Text==3)))
			{
				//for variables that are left-spliced continue
			}
			else
				return 1;
		}


		//cursor fall out of the edited variable. If it was empty, delete it
		RemoveEmptyVariable(DC,theElement);

		DeselectExpression();

		if ((m_IsKeyboardEntry==2) && (m_NumElements==1) && (m_pElementList->Type!=5) &&
			(m_pPaternalElement) && (m_pPaternalElement->m_Type==3) && (m_pPaternalElement->Expression1==(CObject*)this) &&
			((this->m_ParenthesesFlags&0x81)==0))
		{
			//jumping over the first insertion point in an exponent base - //BABA
			m_IsKeyboardEntry=0;

		}
		
		if ((m_IsKeyboardEntry<=1) || (/*(IsText==0) && */(m_pPaternalExpression==NULL) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==12)))
		{
			
			if (m_pPaternalExpression==NULL)
			{
				if (KeyboardEntryBaseObject==NULL) return 1;
				if (KeyboardEntryBaseObject->Type==2) return 1; //hand-drawing, plotter

				//we were editing the main line (bottom level expression);

				int candidate=-1;
				int candidate_x=-100000;
				int absX=KeyboardEntryBaseObject->absolute_X;
				int absY=KeyboardEntryBaseObject->absolute_Y+(m_pElementList+m_IsKeyboardEntry-1)->Y_pos*100/ViewZoom;
				int dist=3*GRID;
				if ((m_NumElements==0) || (m_pElementList->Type==0)) dist=2*GRID;

				tDocumentStruct *ds=TheDocument;
				for (int i=0;i<NumDocumentElements;i++,ds++)
					if ((ds->Type==1) && (ds->Object) && 
						(ds->absolute_Y-ds->Above<absY) &&
						(ds->absolute_Y+ds->Below>absY) &&
						(ds->absolute_X+ds->Length<absX) &&
						(ds->absolute_X+ds->Length>candidate_x) && 
						(ds->absolute_X+ds->Length+dist>absX))
					{
						candidate_x=ds->absolute_X+ds->Length;
						candidate=i;
					}
				if (candidate>=0)
				{
					//found a new equation - set the cursor to it
					CExpression *c;
					c=(CExpression*)TheDocument[candidate].Object;
					if ((c->m_MaxNumRows<=1) ||
						(c->KeyboardStartAt(TheDocument[candidate].Length*ViewZoom/100,(absY-TheDocument[candidate].absolute_Y)*ViewZoom/100,-2)==0))
						KeyboardRefocus(c,c->m_NumElements);
					else
						m_IsKeyboardEntry=0;
					KeyboardEntryBaseObject=TheDocument+candidate;
					return 2; //this forces repaint
				}

				//we are going to create new object
				{
					int y_pos=absY;//KeyboardEntryBaseObject->absolute_Y;
					int x_pos=((KeyboardEntryBaseObject->absolute_X+3-GRID*(((m_NumElements==0) || (m_pElementList->Type==0))?1:2))/GRID)*GRID-3;

					GuidlinesFirstPass=1;
					GuidlineElement=-1;SizeNerbyList=0;
					int range=KeyboardEntryBaseObject->absolute_X-x_pos+1;
					for (int i=0;i<NumDocumentElements;i++)
					{
						tDocumentStruct *ds=TheDocument+i;
						if (ds!=KeyboardEntryBaseObject)
							if (CheckForGuidlines(i,x_pos+range/2,y_pos,range/2+1))
								GuidlineElement=i;
					}
					int make_text=0;
					//if ((this->m_NumElements==1) && (this->m_pElementList->Type==0))
					{
						if (prevEmptyBoxType==-1)
							make_text=IsText; //if we are 'moving' and empty box, then we will inherit its math/text type
						else
							make_text=prevEmptyBoxType;
					}
					prevEmptyBoxType=make_text;
					if (GuidlineElement>=0)
					{
						if ((TheDocument[GuidlineElement].absolute_X>=x_pos) &&
							(TheDocument[GuidlineElement].absolute_X<KeyboardEntryBaseObject->absolute_X))
						{
							x_pos=TheDocument[GuidlineElement].absolute_X;
							CheckForExtendedGuidlines(y_pos);
							PaintGuidlines(DC,y_pos);
							if (TheDocument[GuidlineElement].absolute_Y<-100) make_text=SelectedGuidelineType;

							GuidlineClearTimer=2;
						}
						else
							GuidlineElement=-1;
					}


					if (x_pos<10)
					{
						if (m_pElementList->Type!=1) {InsertEmptyElement(0,1,0,fcolor);m_KeyboardCursorPos=0;}
						return 1;
					}
					AddDocumentObject(1,x_pos,y_pos);
					TheDocument[NumDocumentElements-1].Object=(CObject*)new CExpression(NULL,NULL,DefaultFontSize);
					((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_StartAsText=make_text;
					if (make_text) ((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_Alignment=1;
					KeyboardEntryBaseObject=TheDocument+NumDocumentElements-1;
					KeyboardRefocus((CExpression*)TheDocument[NumDocumentElements-1].Object,0);
					return 2;
				}
				return 1;
			}

			//we fall out of the expression, we will try to move cursor at some other expression (to the left)
			CExpression *NextEditedObject=NULL;
			CElement *EL=m_pPaternalElement;
			int d1=EL->E1_posX+EL->E1_length*5/6;
			int d2=EL->E2_posX+EL->E2_length*5/6;
			int d3=EL->E3_posX+EL->E3_length*5/6;

			if ((m_pPaternalElement->Expression1==(CObject*)this) && (m_pPaternalElement->m_Type==1))
			{
				//index of a variable - moving the cursor at the end of the variable
				tElementStruct *ts=m_pPaternalExpression->GetElementStruct(m_pPaternalElement);
				if (ts)
				{
					m_pPaternalExpression->m_IsKeyboardEntry=(int)(ts-m_pPaternalExpression->m_pElementList)+1;
					m_pPaternalExpression->m_KeyboardCursorPos=(int)strlen(m_pPaternalElement->Data1);
					m_IsKeyboardEntry=0; 
					KeyboardEntryObject=(CObject*)m_pPaternalExpression;
					return 2;
				}
			}

			if (m_pPaternalElement->Expression2==(CObject*)this)
			{
				d1=EL->E2_posX-d1;
				d3=EL->E2_posX-d3;
				if (EL->Expression1) if ((d1>=0) && (abs(EL->E2_posY-EL->E1_posY)<=d1)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression1;
				if (EL->Expression3) if ((d3>=0) && (abs(EL->E2_posY-EL->E3_posY)<=d3)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression3;
			}
			if (m_pPaternalElement->Expression3==(CObject*)this)
			{
				d2=EL->E3_posX-d2;
				d1=EL->E3_posX-d1;
				if (EL->Expression2) if ((d2>=0) && (abs(EL->E3_posY-EL->E2_posY)<=d2)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression2;
				if (EL->Expression1) if ((d1>=0) && (abs(EL->E3_posY-EL->E1_posY)<=d1)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression1;
			}

			if (NextEditedObject)
			{
				//we found an expression inside the same object that is positioned to left
				KeyboardRefocus(NextEditedObject,NextEditedObject->m_NumElements);
				return 1;				
			}
			else
			{
				//we will continue at lower level expression
				//first find the currently edited object in its parent object list
				int ii;
				CExpression *parent=m_pPaternalExpression;
				CElement *pelement=m_pPaternalElement;
				if ((parent) && (pelement) && ((pelement->m_Type==6) || (pelement->m_Type==5)) && (pelement->Expression1==(CObject*)this) &&
					(parent->m_pPaternalElement) && (parent->m_pPaternalElement->m_Type==3) && (parent->m_NumElements==1) &&
					(parent->m_pPaternalElement->Expression1==(CObject*)parent))
				{
					//exponent over function! - we are jumping all the way out of the exponent element -//BABA
					pelement=parent->m_pPaternalElement;
					parent=parent->m_pPaternalExpression;
				}
				for (ii=0;ii<parent->m_NumElements;ii++)
					if ((parent->m_pElementList+ii)->pElementObject==pelement) break;
				if (ii==parent->m_NumElements) return 0;
				KeyboardRefocus(parent,ii);
				return 1;				
			}
		}

		m_IsKeyboardEntry--;
		theElement=m_pElementList+m_IsKeyboardEntry-1;
		if (theElement->Type==1)
		{
			//if this is another variable then go into EDIT mode
			m_KeyboardCursorPos=(short)strlen(theElement->pElementObject->Data1);
			if ((!IsText) || 
				(theElement->pElementObject->m_Text==4))
				m_KeyboardCursorPos--;
			return 1;
		}
		else if (theElement->Type==11)
		{
			//if this is just spacer (tab) then jump over it
			int tmp=m_NumElements;
			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
			if (tmp==m_NumElements) m_IsKeyboardEntry--;
			m_KeyboardCursorPos=0;
			return 1;
		}
		else if (theElement->Type==4)
		{
			CExpression *newpos;
			if (theElement->pElementObject->E1_length+2*this->m_MarginX>=theElement->pElementObject->E2_length)
				newpos=(CExpression*)theElement->pElementObject->Expression1;
			else
				newpos=(CExpression*)theElement->pElementObject->Expression2;
			KeyboardRefocus(newpos,newpos->m_NumElements);
			return 1;
		}
		else
		{
			// first find X and Y position of the cursor (in the topmost expression)
			int X;
			int Y;
			X=theElement->X_pos+theElement->Length;
			Y=theElement->Y_pos;
			if ((theElement->Type==12) && (m_IsKeyboardEntry<m_NumElements))
			{
				X=(theElement+1)->X_pos;
				Y=(theElement+1)->Y_pos;
			}
			
			//then find our best candidate
			if (IsText==0)
			{
				if (KeyboardStartAt(X-1,Y,-2)==0)
				{
					m_IsKeyboardEntry++;
					if (m_IsKeyboardEntry<m_NumElements) if ((theElement+1)->Type==0) m_IsKeyboardEntry++;
					InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				}			
				return 1;
			}
			else
			{
				//this is the text box, but
				//the element to the left is not variable, we will check if it has 
				//Expression1 defined. If yes we will edit it. (Expression2 and 3 will not be tested)
				if ((theElement->pElementObject) && (theElement->pElementObject->Expression1))
				{
					CExpression *NextEditedObject=(CExpression*)theElement->pElementObject->Expression1;
					KeyboardRefocus(NextEditedObject,NextEditedObject->m_NumElements);
					return 1;				
				}

				//Nothing, jump over this one and create new empty variable
				int tmp=m_NumElements;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				if (tmp==m_NumElements) m_IsKeyboardEntry--;
				m_KeyboardCursorPos=0;
				if (m_MaxNumColumns==1) AdjustMatrix();
			
				return 1;
			}
		}
	}

 
	if (!IsALTDown)
	if ((nChar&0xFF)==0x05)  //RIGHT cursor arrow key - moves cursor to the right
	{
		DotAutomaticallyAdded=0;
		m_ModeDefinedAt=0;
		if (IsSHIFTDown)
		{
			//select to the left
						
			if (m_KeyboardCursorPos)
			{
				if (m_KeyboardCursorPos==(int)strlen(theElement->pElementObject->Data1)) m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
			}			
			theElement=KeyboardSplitVariable();

			this->DeselectExpressionExceptKeyboardSelection();

			//now, determine matrix rows and columns of selection
			int row=0,column=0;
			int minc=0x7FFF,minr=0x7FFF,maxc=0,maxr=0;
			int cursor_row=0,cursor_column=0;
			int is_matrix=0;
			for (int i=0;i<m_NumElements;i++)
			{
				tElementStruct *ts=m_pElementList+i;
				if (ts->Type==11) column++;
				else if (ts->Type==12) {column=0;row++;}
				else if ((ts->IsSelected==2) || (i==m_IsKeyboardEntry-1))
				{
					if (column<minc) minc=column;
					if (column>maxc) maxc=column;
					if (row<minr) minr=row;
					if (row>maxr) maxr=row;
				}
				if (i==m_IsKeyboardEntry-1) {cursor_row=row;cursor_column=column;}
			}
			if ((maxr>minr) || (maxc>minc)) is_matrix=1;

			if ((!is_matrix))
			{
				int ii=m_IsKeyboardEntry-2;			
				if ((m_IsKeyboardEntry==m_NumElements) || ((m_pElementList+m_IsKeyboardEntry)->IsSelected!=2))
					for (ii;ii>=0;ii--)
					{
						if ((m_pElementList+ii)->IsSelected!=2) break;
					}

				if (ii==m_IsKeyboardEntry-2)
				{
					int has_columns=0;
					for (ii=m_IsKeyboardEntry;ii<this->m_NumElements;ii++)
					{
						tElementStruct *ts=m_pElementList+ii;
						if (ts->Type==12) /*if (m_IsText) continue; else*/ break;
						if (ts->Type==11) {has_columns=1;continue;}
						if (ts->IsSelected!=2) break;
					}
					if (ii<this->m_NumElements) if ((m_pElementList+ii)->Type!=12) SelectElement(2,ii);
					if (has_columns) SelectElement(2,m_IsKeyboardEntry-1);

					AdjustSelection(2);
					if (m_IsMatrixElementSelected)
					{
						int ii;
						for (ii=m_IsKeyboardEntry-1;ii>=0;ii--)
						{
							if (((m_pElementList+ii)->Type==11) ||((m_pElementList+ii)->Type==12))
								break;
						}
						ii++;
						DeleteElement(m_IsKeyboardEntry-1);
						InsertEmptyElement(ii,1,0,fcolor);
						m_IsKeyboardEntry=ii+1;
					}
				}
				else
				{
					SelectElement(0,ii+1);
				}
			}
			else			
			{
				if ((cursor_column==maxc) && (maxc>minc))
				{
					for (int ii=0;ii<m_MaxNumRows;ii++)
					{
						int k=FindMatrixElement(ii,minc,0);
						if (k>=0)
						for (int jj=k;jj<m_NumElements;jj++)
						{
							SelectElement(0,jj);
							if ((m_pElementList+jj)->Type==11) break;
							if ((m_pElementList+jj)->Type==12) break;
						}
					}
				}
				else
				{
					maxc++;if (maxc>=m_MaxNumColumns) maxc=m_MaxNumColumns-1;
					int k=FindMatrixElement(minr,maxc,0);
					if (k<0) k=0;
					SelectElement(2,k);
					AdjustSelection(2);
				}
			}
			if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

			return 1;
		}
		if (theElement->Type==6) //if we are editing function name
		{
			m_KeyboardCursorPos=0;
			m_IsKeyboardEntry++;
			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
			return 1;
		}
		if (m_KeyboardCursorPos<(short)strlen(theElement->pElementObject->Data1))
		{
			m_KeyboardCursorPos++;
			DeselectExpression();
			if ((m_KeyboardCursorPos==(short)strlen(theElement->pElementObject->Data1)) && (theElement->Type==1) && (theElement->pElementObject->m_Text==4))
			{
				//for variables that are right-spliced continue
			}
			else
				return 1;
		}

		int tmpret=ResolveKnownFunctions(DC,zoom,nChar,nRptCnt,nFlags,fcolor,theElement);
		if (tmpret) IsSpacebarOnlyHit=0;
		if (tmpret==2) return 1; //editing is continued at other expression
		if (tmpret==1) 
		{
			theElement=m_pElementList+m_IsKeyboardEntry-1;
			EditedVariable=theElement->pElementObject;
			IsEditedVariableEmpty=(strlen(EditedVariable->Data1)==0)?1:0;
		}

		//delete this variable if empty
		int tmp=m_NumElements;
		if (IsEditedVariableEmpty) 
		{
			if (RemoveEmptyVariable(DC,theElement)) tmp--; else m_IsKeyboardEntry++;
		}
		else
			m_IsKeyboardEntry++;

		DeselectExpression();


		

		if ((m_IsKeyboardEntry>tmp) || (/*(IsText==0) &&*/ (m_pPaternalExpression==NULL) && ((m_pElementList+m_IsKeyboardEntry-1)->Type==12)))
		{
			m_IsKeyboardEntry--;

			if (m_pPaternalExpression==NULL)
			{
				//we were editing the main line (bottom level expression)
				//int X=(parent->m_pElementList+parent->m_NumElements-1)->X_pos+(parent->m_pElementList+parent->m_NumElements-1)->Length;
				//int Y=0;
				if (KeyboardEntryBaseObject->Type==2) return 1;

				int candidate=-1;
				int candidate_x=100000;
				int absX=KeyboardEntryBaseObject->Length+KeyboardEntryBaseObject->absolute_X;
				int absY=KeyboardEntryBaseObject->absolute_Y+(m_pElementList+m_IsKeyboardEntry-1)->Y_pos*100/ViewZoom;
				int dist=3*GRID;
				if ((m_NumElements==0) || (m_pElementList->Type==0)) dist=2*GRID;

				tDocumentStruct *ds=TheDocument;
				for (int i=0;i<NumDocumentElements;i++,ds++)
					if ((ds->Type==1) && (ds->Object) && 
						(ds->absolute_Y-ds->Above<absY) &&
						(ds->absolute_Y+ds->Below>absY) &&
						(ds->absolute_X>absX) &&
						(ds->absolute_X<candidate_x) && 
						(ds->absolute_X-dist<absX))
					{
						candidate_x=ds->absolute_X;
						candidate=i;
					}
				if (candidate>=0)
				{
					//found a new equation - try to set the cursor to it
					CExpression *c;
					c=(CExpression*)TheDocument[candidate].Object;
					if ((c->m_MaxNumRows<=1) ||
						(c->KeyboardStartAt(0,(absY-TheDocument[candidate].absolute_Y)*ViewZoom/100,2)==0))
						KeyboardRefocus(c,0);
					else
						m_IsKeyboardEntry=0;
					KeyboardEntryBaseObject=TheDocument+candidate;
					return 2; //this forces repaint
				}

				//we are going to create new object
				{
					int prev_x_pos=KeyboardEntryBaseObject->absolute_X;
					if (m_pElementList->Type) 
						prev_x_pos+=KeyboardEntryBaseObject->Length;

					int y_pos=absY;
					int x_pos=((KeyboardEntryBaseObject->absolute_X+KeyboardEntryBaseObject->Length+GRID*(((m_NumElements==0) || (m_pElementList->Type==0))?1:2))/GRID)*GRID-3;


					GuidlinesFirstPass=1;
					GuidlineElement=-1;SizeNerbyList=0;
					int range=x_pos-prev_x_pos+1;

					for (int i=0;i<NumDocumentElements;i++)
					{
						tDocumentStruct *ds=TheDocument+i;
						if (ds!=KeyboardEntryBaseObject)
							if (CheckForGuidlines(i,(x_pos+prev_x_pos)/2,y_pos,range/2))
								GuidlineElement=i;
					}
					int make_text=0;
					//if ((this->m_NumElements==1) && (this->m_pElementList->Type==0))
					{
						if (prevEmptyBoxType==-1)
							make_text=IsText; //if we are 'moving' and empty box, then we will inherit its math/text type
						else
							make_text=prevEmptyBoxType;
					}
					prevEmptyBoxType=make_text;
					if (GuidlineElement>=0)
					{
						if ((TheDocument[GuidlineElement].absolute_X<=x_pos) &&
							(TheDocument[GuidlineElement].absolute_X>KeyboardEntryBaseObject->absolute_X))
						{
							x_pos=TheDocument[GuidlineElement].absolute_X;
							CheckForExtendedGuidlines(y_pos);
							PaintGuidlines(DC,y_pos);
							if (TheDocument[GuidlineElement].absolute_Y<-100) make_text=SelectedGuidelineType;
							GuidlineClearTimer=2;
						}
						else
							GuidlineElement=-1;
					}

					AddDocumentObject(1,x_pos,y_pos);
					TheDocument[NumDocumentElements-1].Object=(CObject*)new CExpression(NULL,NULL,DefaultFontSize);
					((CExpression*)TheDocument[NumDocumentElements-1].Object)->m_StartAsText=make_text;
					if (make_text) ((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_Alignment=1;

					KeyboardEntryBaseObject=TheDocument+NumDocumentElements-1;
					KeyboardRefocus((CExpression*)TheDocument[NumDocumentElements-1].Object,0);
					return 2;
				}
				return 1;
			}

			//we fall out of the expression, we will try to move cursor at some other expression (to the right)
			CExpression *NextEditedObject=NULL;
			CElement *EL=m_pPaternalElement;

			if (m_pPaternalElement->Expression2==(CObject*)this)
			{
				if (EL->Expression1) if ((EL->E1_posX)>(EL->E2_posX+EL->E2_length)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression1;
				if (EL->Expression3) if ((EL->E3_posX)>(EL->E2_posX+EL->E2_length)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression3;
			}
			if (m_pPaternalElement->Expression3==(CObject*)this)
			{
				if (EL->Expression2) if ((EL->E2_posX)>(EL->E3_posX+EL->E3_length)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression2;
				if (EL->Expression1) if ((EL->E1_posX)>(EL->E3_posX+EL->E3_length)) NextEditedObject=(CExpression*)m_pPaternalElement->Expression1;
			}

			if (NextEditedObject)
			{
				//we found an expression inside the same object that is positioned to left
				KeyboardRefocus(NextEditedObject,0);
				return 1;				
			}
			else
			{
				//we will continue at lower level expression
				CExpression *parent=m_pPaternalExpression;
				CElement *pelement=m_pPaternalElement;

				if ((pelement->m_Type==6) && (pelement->Expression1==(CObject*)this) &&
					(parent->m_pPaternalElement) && (parent->m_pPaternalElement->m_Type==3) && (parent->m_NumElements==1) &&
					(parent->m_pPaternalElement->Expression1==(CObject*)parent))
				{
					//function with exponent!!! - special handling as we don't want to jump over insertion point of the exponent function //BABA
					pelement=parent->m_pPaternalElement;
					parent=parent->m_pPaternalExpression;
				}

				//first find the currently edited object in its parent object list
				int ii;
				
				for (ii=0;ii<parent->m_NumElements;ii++)
					if ((parent->m_pElementList+ii)->pElementObject==pelement) break;
				if (ii==parent->m_NumElements) return 0;

				KeyboardRefocus(parent,ii+1);
				return 1;				
			}
		}

		theElement=m_pElementList+m_IsKeyboardEntry-1;
		if (theElement->Type==1)
		{
			//if this is another variable then go into EDIT mode
			if ((IsText==0) || 
				(theElement->pElementObject->m_Text==2) || (theElement->pElementObject->m_Text==3))
				m_KeyboardCursorPos=1; else m_KeyboardCursorPos=0;
			return 1;
		}
		else if (theElement->Type==11)
		{
			//if the next element is spacer (tab) jump over it
			m_IsKeyboardEntry++;
			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
			m_KeyboardCursorPos=0;
			return 1;
		}
		else if (theElement->Type==4) //fractions - we are always entering the numerator or the denominator
		{
			CExpression *newpos;
			if (theElement->pElementObject->E1_length+2*this->m_MarginX>=theElement->pElementObject->E2_length)
				newpos=(CExpression*)theElement->pElementObject->Expression1;
			else
				newpos=(CExpression*)theElement->pElementObject->Expression2;
			KeyboardRefocus(newpos,0);
			return 1;
				
		}
		else
		{
			// first find X and Y position of the cursor (in the topmost expression)
			int X;
			int Y;
			X=theElement->X_pos;
			Y=theElement->Y_pos;

			//then find our best candidate
			if (IsText==0)
			{
				if (KeyboardStartAt(X+1,Y,2)==0)
				{
					if (m_IsKeyboardEntry>1) if ((theElement-1)->Type==0) m_IsKeyboardEntry--;
					InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				}

				return 1;
			}
			else
			{
				//the element to the right is not variable, we will check if it has 
				//Expression1 defined. If yes we will edit it. (Expression2 and 3 will not be tested)
				if ((theElement->pElementObject) && (theElement->pElementObject->Expression1))
				{
					CExpression *NextEditedObject=(CExpression*)theElement->pElementObject->Expression1;
					int pos=0;
					if ((theElement->Type==3) && (NextEditedObject->m_pElementList->Type) &&
						(theElement->pElementObject->Expression1) && ((((CExpression*)theElement->pElementObject->Expression1)->m_ParenthesesFlags&0x81)==0)) pos=1; //jumping over the frist insertion point in an exponent - //BABA
					KeyboardRefocus(NextEditedObject,pos);
					return 1;				
				}

				//jump over this one and create new empty variable
				m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				return 1;
			}
		}

	}


	if ((((nChar&0xFF)==3) || ((nChar&0xFF)==2)) && (IsSHIFTDown)) //SHIFT+UP/DOWN key
	{
		DotAutomaticallyAdded=0;
		if (m_NumElements==1)
		{
			//togles vertical/horizontal orientattion of equation
			CExpression *parent=this;
			while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;
			if (parent)
			{
				parent->m_IsVertical=(parent->m_IsVertical)?0:1; // change the orientation by the up or down key
			}
			if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
			return 1;
		}
		else
		{
			//check if there is a line wraper in front or below somewhwre
			int wraper_found=0;
			if (nChar==3)
				for (int i=m_IsKeyboardEntry-1;i>=0;i--)
				{
					if (((m_pElementList+i)->Type==2) && ((m_pElementList+i)->pElementObject->Data1[0]==(char)0xFF)) {wraper_found=1; break;}
					if (((m_pElementList+i)->Type==11) || ((m_pElementList+i)->Type==12)) break;
				}
			else
				for (int i=m_IsKeyboardEntry;i<m_NumElements;i++)
				{
					if (((m_pElementList+i)->Type==2) && ((m_pElementList+i)->pElementObject->Data1[0]==(char)0xFF)) {wraper_found=1; break;}
					if (((m_pElementList+i)->Type==11) || ((m_pElementList+i)->Type==12)) break;
				}

			if (wraper_found)
			{
				if (m_KeyboardCursorPos==0) KeyboardSplitVariable();
				int X,Y;
				int dir=1;
				short l,a,b;
				this->CalculateSize(DC,zoom,&l,&a,&b);
				GetKeyboardCursorPos(&X,&Y);
					if (nChar==2) Y++; else {Y--;dir=-1;}
				int oldpos=m_IsKeyboardEntry;
				int oldnum=m_NumElements;
				if (this->KeyboardStartAt(X,Y,dir,1))
				{
					if (m_IsKeyboardEntry>oldpos)
					{
						for (int i=oldpos;i<=m_IsKeyboardEntry;i++)
							this->SelectElement(2,i-1);
						if ((m_pElementList+oldpos-1)->pElementObject->Data1[0]==0)
						{DeleteElement(oldpos-1);m_IsKeyboardEntry--;}
					}
					if (m_IsKeyboardEntry<oldpos)
					{
						if (m_NumElements>oldnum) oldpos++;
						for (int i=m_IsKeyboardEntry;i<=oldpos;i++)
							this->SelectElement(2,i-1);
						if ((m_pElementList+oldpos-1)->pElementObject->Data1[0]==0)
							DeleteElement(oldpos-1);
					}
				}
				if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

				return 1;
			}

			if (m_KeyboardCursorPos)
			{
				if (m_KeyboardCursorPos==(int)strlen(theElement->pElementObject->Data1)) m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
			}			
			theElement=KeyboardSplitVariable();

			DeselectExpressionExceptKeyboardSelection();
			
			int row=0,column=0;
			int minc=0x7FFF,minr=0x7FFF,maxc=0,maxr=0;
			int cursor_row=0,cursor_column=0;
			for (int i=0;i<m_NumElements;i++)
			{
				tElementStruct *ts=m_pElementList+i;
				if (ts->Type==11) column++;
				else if (ts->Type==12) {column=0;row++;}
				else if ((ts->IsSelected==2) || (i==m_IsKeyboardEntry-1))
				{
					if (column<minc) minc=column;
					if (column>maxc) maxc=column;
					if (row<minr) minr=row;
					if (row>maxr) maxr=row;
				}
				if (i==m_IsKeyboardEntry-1) {cursor_row=row;cursor_column=column;}
			}
			if ((minc>maxc) || (minr>maxr)) 
			{
				SelectMatrixElement(cursor_row,cursor_column,2);
				if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

				return 1;
			}

			if ((cursor_row==minr) && (maxr>cursor_row) && ((nChar&0xFF)==3)) //up key
			{
				//removes from selection
				int k=FindMatrixElement(maxr,0,0);
				if (k>=0)
				while (k<m_NumElements)
					SelectElement(0,k++);
				if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

				return 1;
			}
			if ((cursor_row==maxr) && (minr<cursor_row) && ((nChar&0xFF)==2)) //down key
			{
				//removes from selection
				int k=FindMatrixElement(minr+1,0,0);
				k--;
				while (k>=0)
					SelectElement(0,k--);
				if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

				return 1;
			}
			if ((nChar&0xFF)==3) minr--; //up key
			if ((nChar&0xFF)==2) maxr++; //down key
			
			if (minr<0) minr=0;
			if (maxr>m_MaxNumRows-1) maxr=m_MaxNumRows-1;

			if ((minr!=maxr))
			{
				int k=FindMatrixElement(minr,minc,0);
				if (k<0) k=0;
				SelectElement(2,k);
				k=FindMatrixElement(maxr,maxc,0);
				if (k<0) k=0;
				SelectElement(2,k);
				AdjustSelection(2);
			}
			if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
			
			return 1;
		}
	}

	//WORKING WITH KEYBOARD SELECTIONS  (keyboard implanting and others)

	{
		int mi,mx;
		int found=0;
		for (mi=0;mi<=m_IsKeyboardEntry-2;mi++)
			if ((this->m_pElementList+mi)->IsSelected==2) {found=1;break;}
		if (!found) mi=m_IsKeyboardEntry;

		for (mx=m_NumElements-1;mx>=m_IsKeyboardEntry;mx--)
			if ((this->m_pElementList+mx)->IsSelected==2) {found=2;break;}
		if (found!=2) mx=m_IsKeyboardEntry-2;
				
		if (found)
		{			
			char ch=toupper(nChar&0xFF);

			if (ch==13)
			{
				//the ENTER key opens pop-up menu
				TempPopupExpression=new CExpression(NULL,NULL,100);
				TempPopupExpression->CopyExpression(this,0);
				TempPopupExpression->m_IsMatrixElementSelected=0;
				TempPopupExpression->m_Selection=0;

				for (int ii=mi;ii<=mx;ii++)
					TempPopupExpression->SelectElement(1,ii);
				TempPopupExpression->DeleteElement(m_IsKeyboardEntry-1);
				short l,a,b;
				TempPopupExpression->CalculateSize(DC,ViewZoom,&l,&a,&b);

				EasycastListStart=0;
				if ((mi<mx) || 
					((mi==mx) && ((TempPopupExpression->m_pElementList+mi)->Type!=0) && ((TempPopupExpression->m_pElementList+mi)->Type!=11) && ((TempPopupExpression->m_pElementList+mi)->Type!=12)))
				{
					if (m_IsKeyboardEntry-1<mx) Popup->ShowPopupMenu(TempPopupExpression,theApp.m_pMainWnd,3,m_IsKeyboardEntry);
					if (m_IsKeyboardEntry-1>mi) Popup->ShowPopupMenu(TempPopupExpression,theApp.m_pMainWnd,3,m_IsKeyboardEntry-m_NumElements-1);				
				}
				return 1;
			}
			
			if ((ch==0x07) || (ch==0x08))
			{
				//backspace and DEL will delete the contents
				if (ch==0x07)
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
				else
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);

				/*if (BackspaceStorage==NULL) 
				{
					BackspaceStorage=new CExpression(NULL,NULL,100);
					BackspaceStorage->CopyExpression(this,0,0,0);
					BackspaceStorageOriginal=this;
				}*/

				if (IsEditedVariableEmpty)
					DeleteElement(m_IsKeyboardEntry-1);

				mi=0;
				for (int i=0;i<m_NumElements;i++)
				{
					if ((m_pElementList+i)->IsSelected==2) {mi=i;break;}
				}
				if (mi>=m_NumElements) mi=m_NumElements-1;
				DeleteSelection(2);
				if (mi) if ((m_pElementList+mi-1)->Type==0) mi--;
				if (mi>m_NumElements) mi=m_NumElements;
				m_IsKeyboardEntry=mi+1;				
				InsertEmptyElement(mi,1,0,fcolor);
				m_KeyboardCursorPos=0;		
				if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
				return 1;
			}

			if ((ch=='B') || (ch=='I') || //bold, italic
				(ch=='U') || (ch=='S') || (ch=='O') || //underline, strikeout, overline
				(ch=='R') || (ch=='G') || // red, green
				(ch=='N')) //normalization
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("font formatting",20218);			
				for (int ii=mi;ii<=mx;ii++)
				{
					tElementStruct *ts=m_pElementList+ii;
					if (ts->pElementObject)
					{
						if ((ts->Type==1) && (ts->pElementObject))
						{
							for (int jj=0;jj<24;jj++)
							{
								if (ch=='B') ts->pElementObject->Data2[jj]|=0x01;
								if (ch=='I') ts->pElementObject->Data2[jj]|=0x02;
								if (ch=='N') ts->pElementObject->Data2[jj]&=0xFC;
							}
						}
						if (ch=='R') ts->pElementObject->m_Color=1;
						if (ch=='G') ts->pElementObject->m_Color=2;
						if (ch=='N') ts->pElementObject->m_Color=-1;
					}
					if (ch=='O') ts->Decoration=4;
					if (ch=='U') ts->Decoration=3;
					if (ch=='S') ts->Decoration=1;
					if (ch=='N') ts->Decoration=0;
				}				
				this->DeselectExpression();
				if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
				return 1;
			}

			if ((ch=='/') || (ch=='C') || (ch=='#') || (ch=='`') || ((ch>='0') && (ch<='9')) || (ch=='\\') ||
				(ch=='(') || (ch=='[') || (ch=='{') || (ch=='|') || (ch=='}'))
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("keyboard implanting",20306);			
				if (IsEditedVariableEmpty)
				{
					DeleteElement(m_IsKeyboardEntry-1);
					if (m_IsKeyboardEntry-1<=mx) mx--;
					if (m_IsKeyboardEntry-1<=mi) mi--;
				}

				if (ch=='C')
				{
					InsertEmptyElement(mx+1,9,0,fcolor);
					CExpression *base=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression1);
					CExpression *low=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression3);
					for (int ii=mi;ii<=mx;ii++)
					{
						if ((m_pElementList+mi)->pElementObject)
							(m_pElementList+mi)->pElementObject->m_Text=0;
						base->InsertElement(m_pElementList+mi,base->m_NumElements);
						DeleteElement(mi);
					}
					KeyboardRefocus(low,0);
					if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
					return 2;
				}
				if ((ch=='/') || (ch=='#') || (ch=='`'))
				{
					InsertEmptyElement(mx+1,4,0,fcolor);
					CExpression *num=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression1);
					CExpression *denom=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression2);
					for (int ii=mi;ii<=mx;ii++)
					{
						if ((m_pElementList+mi)->pElementObject)
							(m_pElementList+mi)->pElementObject->m_Text=0;
						num->InsertElement(m_pElementList+mi,num->m_NumElements);
						DeleteElement(mi);
					}
					KeyboardRefocus(denom,0);
					if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

					return 2;
				}
				if ((ch>='0') && (ch<='9'))
				{
					InsertEmptyElement(mx+1,3,ch,fcolor);
					CExpression *num=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression1);
					for (int ii=mi;ii<=mx;ii++)
					{
						if ((m_pElementList+mi)->pElementObject)
							(m_pElementList+mi)->pElementObject->m_Text=0;
						num->InsertElement(m_pElementList+mi,num->m_NumElements);
						DeleteElement(mi);
					}
					m_IsKeyboardEntry=mi+2;
					InsertEmptyElement(mi+1,1,0,fcolor);
					m_KeyboardCursorPos=0;
					if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

					return 1;
				}
				
				if ((ch=='\\'))
				{
					InsertEmptyElement(mx+1,6,36,fcolor);
					CExpression *num=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression1);
					for (int ii=mi;ii<=mx;ii++)
					{
						if ((m_pElementList+mi)->pElementObject)
							(m_pElementList+mi)->pElementObject->m_Text=0;
						num->InsertElement(m_pElementList+mi,num->m_NumElements);
						DeleteElement(mi);
					}
					m_IsKeyboardEntry=mi+1;
					m_KeyboardCursorPos=0;
					if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

					return 1;
				}
				if ((ch=='(') || (ch=='[') || (ch=='{') || (ch=='|') || (ch=='}'))
				{
					InsertEmptyElement(mx+1,5,(ch=='}')?'{':ch,fcolor);
					CExpression *num=(CExpression*)((m_pElementList+mx+1)->pElementObject->Expression1);
					for (int ii=mi;ii<=mx;ii++)
					{
						if ((m_pElementList+mi)->pElementObject)
							(m_pElementList+mi)->pElementObject->m_Text=0;
						num->InsertElement(m_pElementList+mi,num->m_NumElements);
						DeleteElement(mi);
					}
					if (ch=='}')
					{
						num->m_Alignment=2;
						num->m_ParenthesesFlags|=0x08;
					}

					m_IsKeyboardEntry=mi+2;
					InsertEmptyElement(mi+1,1,0,fcolor);
					m_KeyboardCursorPos=0;
					if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

					return 1;
				}
			}
		}
	}

	if (((nChar&0xFF)==0x0D) && (!IsSHIFTDown)) //ENTER key - continues edit at different expression 
	{		
		if ((GetKeyState(VK_SPACE)&0xFFFE) && (is_external))
		{
			//Spacebar + Enter changes the typing mode (math/text)
			goto toggle_keymode;
		}

		DotAutomaticallyAdded=0;
		if (ResolveKnownFunctions(DC,zoom,' ',nRptCnt,nFlags,fcolor,theElement))
		{
			IsSpacebarOnlyHit=0;
			return 1;
		}

		if (((IsTextContained(m_IsKeyboardEntry-1,0)) && (this->m_pPaternalElement==NULL) && (this->m_MaxNumColumns==1) && (this->m_MaxNumRows==1))
			|| ((IsALTDown) && (!KeyboardExponentMode) && (!KeyboardIndexMode)))
		{
			//inserting line-wrap 

			//special handling for text boxes - if two consecutive enter keys, it will
			//split textbox (or create new one just below)
			if (((m_pElementList+m_IsKeyboardEntry-1)->Type==1) && (m_KeyboardCursorPos==0) /*&& (m_MaxNumColumns==1) && (m_MaxNumRows==1)*/ &&
				((m_IsKeyboardEntry==1) || 
				(((m_pElementList+m_IsKeyboardEntry-2)->Type==2) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]==(char)0xFF)) ||
				((m_pElementList+m_IsKeyboardEntry-2)->Type==11) ||
				((m_pElementList+m_IsKeyboardEntry-2)->Type==12)))
			{
				//if the cursor is at the beginning of a paragraph

				if ((this->m_pPaternalExpression) || (m_MaxNumColumns>1) || (m_MaxNumRows>1))
				{
					//if this text box is nested into an equation, handle it differently
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("Enter key",20307);
					theElement=KeyboardSplitVariable();

					//deletes the last line wraper if it is found at the end of expression
					if ((m_IsKeyboardEntry==m_NumElements) && (m_NumElements>2))
						if (((m_pElementList+m_NumElements-2)->Type==2) && ((m_pElementList+m_NumElements-2)->pElementObject->Data1[0]==(char)0xFF))
						{
							DeleteElement(m_NumElements-2);
							m_IsKeyboardEntry--;
						}
				}
				else
				{
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("Enter key",20307); //text checked in the backspace handling
					theElement=KeyboardSplitVariable();
					if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;
					if (KeyboardEntryBaseObject==NULL) return 2;
					if (KeyboardEntryBaseObject->Type==2) return 2;

					if (m_IsKeyboardEntry==1)
					{
						KeyboardEntryBaseObject->absolute_Y+=m_FontSize/5;
						KeyboardEntryBaseObject->MovingDotState|=0x80;
						((CMainFrame*)(theApp.m_pMainWnd))->RearangeObjects(m_FontSize/5);			
						return 2;
					}
					else if (m_IsKeyboardEntry>2)
					{
						CExpression *tmp=new CExpression(NULL,NULL,100);
						for (int i=m_IsKeyboardEntry;i<m_NumElements;i++)
							tmp->InsertElement(this->m_pElementList+i,i-m_IsKeyboardEntry);
						//while ((tmp->m_pElementList->Type==2) && (tmp->m_pElementList->pElementObject->Data1[0]==(char)0xFF)) tmp->DeleteElement(0);

						while (m_NumElements>m_IsKeyboardEntry-2)
							DeleteElement(m_IsKeyboardEntry-2);
						m_IsKeyboardEntry=0;
						this->KeyboardInsertNewEquation(DC,zoom,nChar,(tmp->m_pElementList->Type)?tmp:NULL,IsText);
						delete tmp;
					}
					return 1;
				}
			}
			else 
			{
				//if the cursor is not at the beginning of a paragraph 

				if (IsEditedVariableEmpty) 
					{RemoveEmptyVariable(DC,theElement);m_IsKeyboardEntry--;}
				else if ((m_IsKeyboardEntry>1) || (m_KeyboardCursorPos>0))
				{
					if ((m_KeyboardCursorPos>0) && (m_KeyboardCursorPos<(int)strlen(theElement->pElementObject->Data1)))
					{
						this->KeyboardSplitVariable();
					}

					if (m_IsKeyboardEntry>0)
					{
						theElement=m_pElementList+m_IsKeyboardEntry-1;
						if (theElement->Type==1)
						if ((int)strlen(theElement->pElementObject->Data1)>m_KeyboardCursorPos) m_IsKeyboardEntry--;
					}
				}
				int found_break=0;
				for (int i=0;i<m_NumElements;i++)
					if (((m_pElementList+i)->Type==11) || ((m_pElementList+i)->Type==12) || 
						(((m_pElementList+i)->Type==2) && ((m_pElementList+i)->pElementObject->Data1[0]==(char)0xFF)))
						found_break=1;
				if (!found_break) m_Alignment=1;
				if ((!found_break) && (IsTextContained(-1))) {m_Alignment=1;m_StartAsText=1;}
				InsertEmptyElement(m_IsKeyboardEntry,2,(char)0xFF,fcolor);
				InsertEmptyElement(m_IsKeyboardEntry+1,1,0,fcolor);
				m_IsKeyboardEntry+=2;
				m_KeyboardCursorPos=0;
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
				return 1;
			}
		}

		if (IsEditedVariableEmpty)
			{RemoveEmptyVariable(DC,theElement);m_IsKeyboardEntry--;}
	
		DeselectExpression();
		
		

		if ((m_MaxNumColumns>1) || (m_MaxNumRows>1))
		{
			//jump into the next column/row
			for (int i=m_IsKeyboardEntry;i<m_NumElements;i++)
			{
				tElementStruct *ts=m_pElementList+i;
				if ((ts->Type==11) || ((ts->Type==12) && (i<m_NumElements-1)))
				{
					//first delete the newline if there is nothing in the last line of text
					if (m_IsKeyboardEntry)
					{
						tElementStruct *ts=m_pElementList+m_IsKeyboardEntry-1;
						if ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF) && 
							(((ts+1)->Type==11) || ((ts+1)->Type==12)))
						{
							DeleteElement(m_IsKeyboardEntry-1);
							i--;
						}
					}

					InsertEmptyElement(i+1,1,0,fcolor);
					m_IsKeyboardEntry=i+2;
					m_KeyboardCursorPos=0;
					return 1;
				}
			}
		}
		
		//exit if we were editing the main line (bottom level expression)
		if ((m_pPaternalElement==NULL) || (m_pPaternalExpression==NULL)) 
		{
			if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
			{
				InsertEmptyElement(m_NumElements,1,0,fcolor);
				m_IsKeyboardEntry=m_NumElements;
				m_KeyboardCursorPos=0;
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("Enter key",20307); //text checked in the backspace handling
				DeleteElement(m_NumElements-1);
				if ((this->m_MaxNumColumns>1) || (this->m_MaxNumRows>1)) this->AdjustMatrix();

				m_IsKeyboardEntry=0;
				this->KeyboardInsertNewEquation(DC,zoom,nChar,0,IsText);
				return 1;
			}
			
			return 0;
		}

		int prevkeyboardentry=m_IsKeyboardEntry;
		m_IsKeyboardEntry=0;

		CExpression *NextEditedObject=NULL;
		if (m_pPaternalElement->m_Type==4) //fractions
		{
			if ((m_pPaternalElement->Expression1==(CObject*)this) && (((CExpression*)(m_pPaternalElement->Expression1))->m_pElementList->Type))
			{
				CExpression *e2=((CExpression*)m_pPaternalElement->Expression2);
				if (e2->m_NumElements==1)
				{
					if (e2->m_pElementList->Type==0) NextEditedObject=(CExpression*)m_pPaternalElement->Expression2;
					else if (e2->m_pElementList->Type==6)
					{
						CElement *el=e2->m_pElementList->pElementObject;
						if ((el->Expression1) && (((CExpression*)el->Expression1)->m_pElementList->Type==0))
							if (el->IsDifferential())
								NextEditedObject=(CExpression*)el->Expression1;
					}
				}
			}
			if ((m_pPaternalElement->Expression2==(CObject*)this) &&
				(((CExpression*)(m_pPaternalElement->Expression2))->m_pElementList->Type) &&
				(((CExpression*)(m_pPaternalElement->Expression1))->m_pElementList->Type==0))
					NextEditedObject=(CExpression*)m_pPaternalElement->Expression1;
		}
		if (m_pPaternalElement->m_Type==3) //exponent
		{
			if (m_pPaternalElement->Expression2==(CObject*)this)
				if ((((CExpression*)(m_pPaternalElement->Expression1))->m_pElementList->Type==6) &&
					(((CExpression*)(m_pPaternalElement->Expression1))->m_NumElements==1))
				{
					CExpression *tmp=(CExpression*)(((CExpression*)(m_pPaternalElement->Expression1))->m_pElementList->pElementObject->Expression1);
					if ((tmp) && (tmp->m_pElementList->Type==0))
						NextEditedObject=tmp;
				}
		}
		if ((m_pPaternalElement->m_Type==6) || //functions
			(m_pPaternalElement->m_Type==8)) //root
		{
			if (m_pPaternalElement->Expression1)
			{
				if (m_pPaternalElement->Expression2==(CObject*)this)
					if (((CExpression*)(m_pPaternalElement->Expression1))->m_pElementList->Type==0)
						NextEditedObject=(CExpression*)m_pPaternalElement->Expression1;
			}
		}

		if  (m_pPaternalElement->m_Type==9) //condition
		{
			if (m_pPaternalElement->Expression2==(CObject*)this)
				NextEditedObject=(CExpression*)m_pPaternalElement->Expression3;
		}
		if (m_pPaternalElement->m_Type==7) //sigma, pi , integral
		{
			if ((m_pPaternalElement->Expression3==(CObject*)this) &&
				(((CExpression*)(m_pPaternalElement->Expression3))->m_pElementList->Type) &&
				//(((CExpression*)(m_pPaternalElement->Expression3))->m_NumElements>1) &&
				(((CExpression*)(m_pPaternalElement->Expression2))->m_pElementList->Type==0))
				NextEditedObject=(CExpression*)m_pPaternalElement->Expression2;  //from upper to lower limit (if lower limit not defined)
			if ((m_pPaternalElement->Expression2==(CObject*)this) &&
				(((CExpression*)(m_pPaternalElement->Expression2))->m_pElementList->Type) &&
				(((CExpression*)(m_pPaternalElement->Expression3))->m_pElementList->Type==0))
				NextEditedObject=(CExpression*)m_pPaternalElement->Expression3; //from lower to upper limit (if upper limit not defined)
			if ((NextEditedObject==0) && (m_pPaternalElement->Expression1!=(CObject*)this))
			{
				//from any limit into the function argument, if the argument is empty (or contains 'd' or 'partial' empty function)
				CExpression *e=(CExpression*)m_pPaternalElement->Expression1;
				char is_empty=0;
				if (e->m_NumElements)
				{
					if (e->m_pElementList->Type==0) 
						is_empty=1;
					else if (e->m_pElementList->Type==6)
					{

					if (e->m_pElementList->pElementObject->IsDifferential())
						if ((((CExpression*)e->m_pElementList->pElementObject->Expression1)->m_NumElements==1) && 
							(((CExpression*)e->m_pElementList->pElementObject->Expression1)->m_pElementList->Type==0))
							is_empty=1;
					}
				}
					
				if (is_empty) NextEditedObject=e;
			}
			if (NextEditedObject==0)
			{
				//we will put the cursor into the 'd' element if it is empty
				CExpression *e=(CExpression*)m_pPaternalElement->Expression1;
				if ((e->m_NumElements) && ((e->m_pElementList+e->m_NumElements-1)->Type==6) && (prevkeyboardentry<e->m_NumElements))
				{
					CElement *el=(e->m_pElementList+e->m_NumElements-1)->pElementObject;
					if (el)
					{
						CExpression *e2=(CExpression*)el->Expression1;
						if (el->IsDifferential())
							if ((e2->m_NumElements==1) && (e2->m_pElementList->Type==0))
								NextEditedObject=e2;
					}
				}
			}
		}
		if (m_pPaternalElement->m_Type==10) //conditions
		{
			if (m_pPaternalElement->Expression1==(CObject*)this)
			{
				NextEditedObject=(CExpression*)m_pPaternalElement->Expression2;
				if (NextEditedObject==NULL) NextEditedObject=(CExpression*)m_pPaternalElement->Expression3;
			}
			if (m_pPaternalElement->Expression2==(CObject*)this)
				NextEditedObject=(CExpression*)m_pPaternalElement->Expression3;
		}

		if (NextEditedObject)
		{
			//we will continue editing next expression of the same element
			int delete_var_index=0;
			if ((this->m_pElementList->Type==0) && (this->m_NumElements<2) && 
				 ((m_pPaternalElement->m_Type==6) && ((CObject*)this==m_pPaternalElement->Expression2)))
			{
				delete_var_index=2;
			}

			KeyboardRefocus(NextEditedObject,0);
			if ((delete_var_index) && (NextEditedObject==(CExpression*)this->m_pPaternalElement->Expression1))
			{
				CExpression *exp=(CExpression*)this->m_pPaternalElement->Expression2;
				this->m_pPaternalElement->Expression2=NULL;
				delete exp;
			}

			return 1;				
		}
		else 
		{
			//there are no other expressions in the same element,
			//so we are continuing at the parent expression, just behind this element.

			//first find the currently edited object in its parent object list
			int ii;
			CExpression *parent=m_pPaternalExpression;
			CElement *pelement=m_pPaternalElement;

			int jump_one_step_further=0;
			
			if ((is_external) && (pelement->IsDifferential(2))) 
			{
				jump_one_step_further=1; //BABA2
			}
			else if ((is_external) && (pelement->m_Type==1) && (parent->m_pPaternalElement) && (parent->m_pPaternalElement->IsDifferential(2)))
			{
				//we were editing index or exponent of a variable inside a differential
				jump_one_step_further=1;
			}


			if ((pelement->m_Type==6) && (pelement->Expression1==(CObject*)this) &&
				(parent->m_pPaternalElement) && (parent->m_pPaternalElement->m_Type==3) && (parent->m_NumElements==1) &&
				(parent->m_pPaternalElement->Expression1==(CObject*)parent))
			{
				//exponent over function! - we are jumping all the way out of the exponent element -//BABA
				pelement=parent->m_pPaternalElement;
				parent=parent->m_pPaternalExpression;
			}


			ii=pelement->GetPaternalPosition();
			/*for (ii=0;ii<parent->m_NumElements;ii++)
				if ((parent->m_pElementList+ii)->pElementObject==pelement) break;
			if (ii==parent->m_NumElements) return 0;*/


			if ((pelement->m_Type==3) && (this==(CExpression*)(pelement->Expression2)) && (this->m_NumElements==1) && (this->m_pElementList->Type==0))
			{
				//we are removing exponent because it is empty
				CExpression *base=(CExpression*)pelement->Expression1;
				if ((base->m_NumElements>1) || (base->m_MaxNumRows>1) || (base->m_MaxNumColumns>1))
				{
					parent->InsertEmptyElement(ii,5,'(',fcolor);
					((CExpression*)((parent->m_pElementList+ii)->pElementObject->Expression1))->CopyExpression(base,0,0,0);
					KeyboardRefocus(parent,ii+1);
					parent->DeleteElement(ii+2);
					return 1;
				}
				else if ((base->m_NumElements==1) && (base->m_pElementList->Type))
				{
					parent->InsertElement(base->m_pElementList,ii);
					KeyboardRefocus(parent,ii+1);
					parent->DeleteElement(ii+2);
					return 1;
				}
			}

			int delete_var_index=0;
			if ((this->m_pElementList->Type==0) && (this->m_NumElements<2) && 
				(((m_pPaternalElement->m_Type==1) && ((CObject*)this==m_pPaternalElement->Expression1)) ||
				 ((m_pPaternalElement->m_Type==6) && ((CObject*)this==m_pPaternalElement->Expression2)) ||
				 ((m_pPaternalElement->m_Type==6) && ((CObject*)this==m_pPaternalElement->Expression1)) ||
				 ((m_pPaternalElement->m_Type==5) && ((CObject*)this==m_pPaternalElement->Expression2))))
			{
				if ((m_pPaternalExpression->m_pPaternalElement) && (m_pPaternalExpression->m_pPaternalElement->m_Type==4) &&
					(m_pPaternalExpression->m_pPaternalElement->Data1[0]=='d'))
					delete_var_index=0;
				else
					delete_var_index=((CObject*)this==m_pPaternalElement->Expression1)?1:2;
			}

			KeyboardRefocus(parent,ii+1);
			if ((delete_var_index) && (parent->m_IsKeyboardEntry>1))
			{
				pelement=m_pPaternalElement;
				if (delete_var_index==1)
				{
					delete (CExpression*)(pelement->Expression1);
					pelement->Expression1=NULL;
				}
				else
				{
					delete (CExpression*)(pelement->Expression2);
					pelement->Expression2=NULL;
				}

			}
			if (jump_one_step_further) ((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);

			return 1;				
		}	
	}
	
	if ((nChar&0xFF)==14) //home key
	{
		DotAutomaticallyAdded=0;
		m_ModeDefinedAt=0;
		if (IsSHIFTDown)
		{
			//selecting elements from the beginning
			theElement=KeyboardSplitVariable();
			if ((m_KeyboardCursorPos>0) && (m_KeyboardCursorPos==strlen(theElement->pElementObject->Data1)))
			{
				InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
			}
			this->DeselectExpression();
			int ii;
			for (ii=m_IsKeyboardEntry-1;ii>=0;ii--)
			{
				if (((m_pElementList+ii)->Type==11) ||
					((m_pElementList+ii)->Type==12) ||
					(((m_pElementList+ii)->Type==2) && ((m_pElementList+ii)->pElementObject->Data1[0]==(char)0xFF)))
					break;
				SelectElement(2,ii);
			}
			if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
			return 1;
		}

		if (ResolveKnownFunctions(DC,ViewZoom,nChar,nRptCnt,nFlags,fcolor,theElement)==2) 
		{
			IsSpacebarOnlyHit=0;
			return 1;
		}
		RemoveEmptyVariable(DC,theElement);

		CExpression *e=this;
		int startpos=m_IsKeyboardEntry-2;
		while (e)
		{
			for (int i=startpos;i>0;i--)
			{
				tElementStruct *ts=e->m_pElementList+i;
				if ((ts->Type==12) || (ts->Type==11) || ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF)))
				{
					KeyboardRefocus(e,i+1);
					return 1;
				}
			}
			if (e->m_pPaternalElement) startpos=((CElement*)e->m_pPaternalElement)->GetPaternalPosition();
			if (e->m_pPaternalExpression) 
				e=e->m_pPaternalExpression;
			else
			{
				KeyboardRefocus(e,0);
				e=NULL;
			}
		}
		return 1;
	}

	if ((nChar&0xFF)==15) //end key
	{
		DotAutomaticallyAdded=0;
		m_ModeDefinedAt=0;
		if (IsSHIFTDown)
		{
			//selecting elements to the end
			theElement=KeyboardSplitVariable();
			if ((m_KeyboardCursorPos>0) && (m_KeyboardCursorPos==strlen(theElement->pElementObject->Data1)))
			{
				InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
			}
			this->DeselectExpression();
			int ii;
			for (ii=m_IsKeyboardEntry;ii<m_NumElements;ii++)
			{
				if (((m_pElementList+ii)->Type==11) ||
					((m_pElementList+ii)->Type==12) ||
					(((m_pElementList+ii)->Type==2) && ((m_pElementList+ii)->pElementObject->Data1[0]==(char)0xFF)))
					break;
				SelectElement(2,ii);
			}
			if ((UseToolbar)  && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();
			return 1;
		}
		if (ResolveKnownFunctions(DC,ViewZoom,nChar,nRptCnt,nFlags,fcolor,theElement)==2) 
		{
			IsSpacebarOnlyHit=0;
			return 1;
		}
		RemoveEmptyVariable(DC,theElement);


		CExpression *e=this;
		int startpos=m_IsKeyboardEntry-1;
		while (e)
		{
			for (int i=startpos;i<e->m_NumElements;i++)
			{
				tElementStruct *ts=e->m_pElementList+i;
				if ((ts->Type==12) || (ts->Type==11) || ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF)))
				{
					KeyboardRefocus(e,i);
					return 1;
				}
			}
			if (e->m_pPaternalElement) startpos=((CElement*)e->m_pPaternalElement)->GetPaternalPosition()+1;
			if (e->m_pPaternalExpression) 
				e=e->m_pPaternalExpression;
			else
			{
				KeyboardRefocus(e,e->m_NumElements);
				e=NULL;
			}
		}
		return 1;
	}

	if ((nChar&0xFF)==0x08) //BACKSPACE - delete the previous character or object
	{
		//special handling - will execute the Undo operation if the backspace is used after the Enter key that creates a new object
		if ((m_IsKeyboardEntry==1) && (m_pElementList->Type==1) && 
			((m_NumElements==1)) && (m_pPaternalExpression==NULL) &&
			(m_KeyboardCursorPos==0) && ((m_ParenthesesFlags&0x01)==0))
		{
			//if the Enter key was last used, then we execute the undo (return 88)
			if (((CMainFrame*)(theApp.m_pMainWnd))->UndoCheckText("Enter key",20307)) return 88;
		}
		
		//splicing of variables when the Shift+Backspace is used
		if ((m_KeyboardCursorPos>0) && (IsSHIFTDown)) 
		{
			if ((m_IsKeyboardEntry>1) && (m_pElementList[m_IsKeyboardEntry-2].Type==1) && 
				(m_pElementList[m_IsKeyboardEntry-2].pElementObject->Expression1==NULL))
			{
				CElement *el=m_pElementList[m_IsKeyboardEntry-2].pElementObject;
				int l=(int)strlen(theElement->pElementObject->Data1);
				int x=(int)strlen(el->Data1);
				if ((x>0) && (l+x<24))
				{
					char c1=el->Data1[0];
					char c2=el->Data1[l-1];
					if (((c1<'0') || (c1>'9')) && ((c2<'0') || (c2>'9')))
					{
						memmove(theElement->pElementObject->Data1+x,theElement->pElementObject->Data1,l);
						memcpy(theElement->pElementObject->Data1,el->Data1,x);
						memmove(theElement->pElementObject->Data2+x,theElement->pElementObject->Data2,l);
						memcpy(theElement->pElementObject->Data2,el->Data2,x);
						el->Data1[l+x]=0;

						m_IsKeyboardEntry--;
						m_KeyboardCursorPos+=x;
						DeleteElement(m_IsKeyboardEntry-1);
					}
				}
			}
			return 1;
		}

		/*if (BackspaceStorage==NULL) 
		{
			BackspaceStorage=new CExpression(NULL,NULL,100);
			BackspaceStorage->CopyExpression(this,0,0,0);
			BackspaceStorageOriginal=this;
		}*/

		//deleting characters if the cursor is not at the very first position in the variable
		if (m_KeyboardCursorPos>0)
		{
			if ((lastkeystrokes[1]!=0x08) && (KeyboardEntryBaseObject))
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);

			memmove(theElement->pElementObject->Data1+m_KeyboardCursorPos-1,theElement->pElementObject->Data1+m_KeyboardCursorPos,24-m_KeyboardCursorPos);
			if (theElement->Type==1)
				memmove(theElement->pElementObject->Data2+m_KeyboardCursorPos-1,theElement->pElementObject->Data2+m_KeyboardCursorPos,24-m_KeyboardCursorPos);
			m_KeyboardCursorPos--;

			//check some additional handlings if the variable name is completely erased
			if ((m_KeyboardCursorPos==0) && (theElement->pElementObject->Data1[0]==0))
			{
				//delete the index of the variable if it exists
				if (((this->m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1) &&
					((this->m_pElementList+m_IsKeyboardEntry-1)->Type==1))
				{
					delete (CExpression*)((this->m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1);
					(this->m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1=NULL;
				}

				//deletes the multiplying dot if it was automatically added in front of a number  
				if ((DotAutomaticallyAdded) && (m_IsKeyboardEntry>2))
				{
					CElement *el=(this->m_pElementList+m_IsKeyboardEntry-2)->pElementObject;
					if ((el->m_Type==2) && (el->Data1[0]==(char)0xD7))
					{
						this->DeleteElement(m_IsKeyboardEntry-2);
						m_IsKeyboardEntry--;
						theElement=m_pElementList+m_IsKeyboardEntry-1;
						DotAutomaticallyAdded=0;
					}
				}
				
				//deletes the decoration, if it existed over the variable
				theElement->Decoration=0;
				theElement->pElementObject->m_VMods=0;
			}

			//special handling - if we typed one variable then immediately deleting it (by backspace), the following lines
			//will ensure that the cursor is placed at the end of the previous variable (not in an empty variable)
			//(this is if we accidentaly typed a letter, after backspace everything is as it was before we typed the accidental letter)
			if ((m_KeyboardCursorPos==0) && (KeyboardCursorWasAttached) && (m_IsKeyboardEntry>1) && ((theElement-1)->Type==1))
			{
				m_KeyboardCursorPos=(int)strlen((theElement-1)->pElementObject->Data1);
				m_IsKeyboardEntry--;
				DeleteElement(m_IsKeyboardEntry);
				return 1;
			}

			if ((m_KeyboardCursorPos==0) && ((theElement->pElementObject->m_Text==2) || (theElement->pElementObject->m_Text==3)))
			{
				//for left-spliced elements we continue.
			}
			else //for others we return
				return 1; 
		}

		DotAutomaticallyAdded=0;

		//the cursor fall out of variable, delete the variable if empty
		if ((IsEditedVariableEmpty) && (m_IsKeyboardEntry>1) && (IsText==0))
			DeleteElement(m_IsKeyboardEntry-1);

		//if the cursor fall out of expression, we do nothing - we never go beyound the current expression
		//the only exeption is that we delete element of some types (function , sigma...) from Expresson1
		if (m_IsKeyboardEntry<=1) 
		{
			//if not empty do nothing
			if ((m_NumElements>1) || (m_pElementList->Type!=1) || (m_pElementList->pElementObject->Data1[0])) return 1;

			//deleting of parentheses from the root object, if parentheses exist
			if (m_pPaternalElement==NULL) 
			{
				if (this->m_Selection==0x7FFF)
				{
					this->m_ParenthesesFlags=0;
					this->m_ParentheseShape=0;
					this->DeselectExpression();
				}
				else
					this->SelectExpression(1);
				return 1;
			}

			//this is an empty non-root expression (sbexpression of an element)
			int i=m_pPaternalElement->GetPaternalPosition();
			int type=m_pPaternalElement->m_Type;
			
			//check if this is an index expression of variable/function/parenheses - if yes, we delete the index expression
			if (((type==1) && (m_pPaternalElement->Expression1==(CObject*)this)) || 
				(((type==5) || (type==6)) && (m_pPaternalElement->Expression2==(CObject*)this)))
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);

				CExpression *NextEditedObject=m_pPaternalExpression;
				m_IsKeyboardEntry=0;
				if (type==1)
				{
					NextEditedObject->m_IsKeyboardEntry=i+1;
					NextEditedObject->m_KeyboardCursorPos=(short)strlen(m_pPaternalElement->Data1);							
				}
				else
				{
					NextEditedObject->m_IsKeyboardEntry=i+2;
					NextEditedObject->InsertEmptyElement(i+1,1,0,fcolor);
					NextEditedObject->m_KeyboardCursorPos=0;
				}
				KeyboardEntryObject=(CObject*)NextEditedObject;
				CExpression *tmp;
				if (type==1)
				{
					tmp=((CExpression*)(m_pPaternalElement->Expression1));
					m_pPaternalElement->Expression1=NULL;
				}
				else
				{
					tmp=((CExpression*)(m_pPaternalElement->Expression2));
					m_pPaternalElement->Expression2=NULL;
				}
				delete tmp; //deleting it as the last operation!
				if (lastkeystrokes[0]=='_')
				{
					memmove(lastkeystrokes,lastkeystrokes+1,10);
					previouslyEditedExpression=(CExpression*)KeyboardEntryObject;
				}
				return 1;
			}

			//we will undo if the last operation was conversion to a function (this is used
			//when we accidently convert to a function to return everything back to the original state)
			if ((type==3) || (type==6))
			if (((CMainFrame*)(theApp.m_pMainWnd))->UndoCheckText("convert to function",20315))
			{
				return 88;
			}
				

			//if this is exponent expression, then we will delete the exponent
			if ((type==3) && (m_pPaternalElement->Expression2==(CObject*)this)) 
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);

				CExpression *Base=(CExpression*)m_pPaternalElement->Expression1;
				CExpression *NextEditedObject=m_pPaternalExpression;
				tDocumentStruct *tmp=KeyboardEntryBaseObject;
				if (Base->m_ParenthesesFlags&0x80)
				{
					NextEditedObject->InsertEmptyElement(i+1,5,Base->m_ParentheseShape,fcolor);
					for (int jj=0;jj<Base->m_NumElements;jj++)
						((CExpression*)((NextEditedObject->m_pElementList+i+1)->pElementObject->Expression1))->InsertElement(Base->m_pElementList+jj,jj);
					NextEditedObject->InsertEmptyElement(i+2,1,0,fcolor);
					NextEditedObject->DeleteElement(i);
					NextEditedObject->m_IsKeyboardEntry=i+2;
				}
				else
				{
					int jj;
					for (jj=0;jj<Base->m_NumElements;jj++)
						NextEditedObject->InsertElement(Base->m_pElementList+jj,i+jj+1);
					NextEditedObject->InsertEmptyElement(i+jj+1,1,0,fcolor);
					NextEditedObject->DeleteElement(i);
					NextEditedObject->m_IsKeyboardEntry=i+jj+1;
				}
				NextEditedObject->m_KeyboardCursorPos=0;
				KeyboardEntryObject=(CObject*)NextEditedObject;
				KeyboardEntryBaseObject=tmp;
				return 2; //this will repaint
			}

			//we can delete elements from inside (two-step) for functions, sigma/integral, root, exponents and parentheses
			if ((type!=6) && (type!=7) && (type!=8) && (type!=3) && (type!=5)) return 1;
			if (m_pPaternalElement->Expression1!=(CObject*)this) return 1;
			if ((m_pPaternalExpression->m_pElementList+i)->IsSelected)
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);

				CExpression *NextEditedObject=m_pPaternalExpression;
				KeyboardRefocus(NextEditedObject,i);
				NextEditedObject->DeleteElement(i+1);
				return 1;
			}
			else
			{
				m_pPaternalExpression->DeselectExpressionExceptKeyboardSelection();
				m_pPaternalExpression->SelectElement(2,i);
				return 1;
			}
				
			return 1;
		}

		//okay, we deleted an element in the middle of expression,
		//now we will have to check the element just before
		m_IsKeyboardEntry--;
		theElement=m_pElementList+m_IsKeyboardEntry-1;
		if (theElement->Type==1)
		{
			//if the variable has a very simple index then delete it, if the
			//index is more complex, then move the cursor into it
			CExpression *index=(CExpression*)theElement->pElementObject->Expression1;
			if (index) 
			{
				if ((index->m_NumElements==1) && (index->m_pElementList->Type==1) && (strlen(index->m_pElementList->pElementObject->Data1)<=2))
				{
					delete index;
					theElement->pElementObject->Expression1=NULL;
					m_KeyboardCursorPos=(int)strlen(theElement->pElementObject->Data1);
				}
				else
				{
					KeyboardRefocus(index,index->m_NumElements);
				}
				return 1;
			}

			//text variables can be spliced and so we have special handling
			if (IsText)
			{
				m_IsKeyboardEntry++; //to save proper undo state
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);

				int len=(int)strlen((theElement+1)->pElementObject->Data1);
				int firsts=23-len;
				if ((int)strlen(theElement->pElementObject->Data1)<=firsts) 
					firsts=(int)strlen(theElement->pElementObject->Data1);
				else if (((theElement+1)->Type==1) && (theElement->Type==1) && (theElement->pElementObject->m_Text))
				{
					(theElement+1)->pElementObject->m_Text=2;
					m_IsKeyboardEntry--;
					m_KeyboardCursorPos=(int)strlen(theElement->pElementObject->Data1);
					return 1;
				}


				if ((theElement->pElementObject->m_Text==4) && (firsts>0)) firsts--;
				memcpy(theElement->pElementObject->Data1+firsts,(theElement+1)->pElementObject->Data1,len);
				if (theElement->Type==1)
					memcpy(theElement->pElementObject->Data2+firsts,(theElement+1)->pElementObject->Data2,len);
				theElement->pElementObject->Data1[firsts+len]=0;
				DeleteElement(m_IsKeyboardEntry-1);
				m_IsKeyboardEntry--;
				m_KeyboardCursorPos=firsts;
				return 1;
			}

			//finally, we will delete the last character of the variable
			m_KeyboardCursorPos=(short)strlen(theElement->pElementObject->Data1);
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);
			if (IsSHIFTDown) return 1; //if Shift+Backspace, just move the cursor at the end of the previous variable
			if ((m_KeyboardCursorPos>0) && (m_KeyboardCursorPos<24))
				theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=0;
			if (m_KeyboardCursorPos>0) m_KeyboardCursorPos--;
			if (m_KeyboardCursorPos==0)	{theElement->Decoration=0;theElement->pElementObject->m_VMods=0;}
			return 1;
		}
		else
		{
			//this another element (not a variable) - we generally use two-step deletion (first select, then delete).

			//special handling for simple exponents - a simple exponent is immediately deleted (no two-step deletion)
			if (theElement->Type==3)
			{
				CExpression *exp=(CExpression*)(theElement->pElementObject->Expression2);
				if (((exp) && (exp->m_NumElements==1) && (exp->m_pElementList->Type<=2)) ||
					((exp) && (exp->m_NumElements==2) && (exp->m_pElementList->Type<=2) && ((exp->m_pElementList+1)->Type<=2)))
				{
					int i=m_IsKeyboardEntry;
					CExpression *exp=(CExpression*)theElement->pElementObject->Expression2;
					CExpression *Base=(CExpression*)theElement->pElementObject->Expression1;
					if (Base->m_NumElements==1)
					{
						InsertElement(Base->m_pElementList,i);
						InsertEmptyElement(i+1,1,0,fcolor);
						if (Base->m_pElementList->Type==1)
							(m_pElementList+i+1)->pElementObject->m_Text=IsText;
						DeleteElement(i-1);
						m_IsKeyboardEntry=i+1;
						m_KeyboardCursorPos=0;
						return 1; //this will repaint
					}
				}
			}

			//special handling for math operators - math operators are immediately deleted (no two-step deletion)
			if ((theElement->Type==2) || 
				(KeyboardEntryBaseObject==NULL) || (KeyboardEntryBaseObject->Type==2)) //for the quick-type
			{
				if (IsText)
				{
					m_IsKeyboardEntry++; 
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);
					{
						if ((theElement->pElementObject->Data1[0]==(char)0xFF) && 
							(lastkeystrokes[0]!=' ') && (m_IsKeyboardEntry>2) &&
							((this->m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1[0]==0))
						{
							//if the last entry was newline, then delete the newline and move the cursor
							//at the end of the previous row
							DeleteElement(m_IsKeyboardEntry-1);
							m_IsKeyboardEntry--;
							DeleteElement(m_IsKeyboardEntry-1);
							m_IsKeyboardEntry--;
							m_KeyboardCursorPos=(short)strlen((this->m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1);
						}
						else
						{
							m_IsKeyboardEntry--;
							DeleteElement(m_IsKeyboardEntry-1);
						}
					}
					return 1;
				}

				m_IsKeyboardEntry++; //temprarily return to previous situation in order to save Undo
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("backspace key",20305);
				DeleteElement(m_IsKeyboardEntry-1);
				m_IsKeyboardEntry--;

				DeleteElement(m_IsKeyboardEntry-1);

				if ((m_IsKeyboardEntry>=2) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==0)) m_IsKeyboardEntry--; //what????

				if ((KeyboardCursorWasAttached) && (m_IsKeyboardEntry>1) && ((theElement-1)->Type==1))
				{
					//this part of code ensures to return everyting to the previous situation if we accidentally typed an operator and then 
					//corrected it with the backspace
					m_KeyboardCursorPos=(int)strlen((theElement-1)->pElementObject->Data1);
					m_IsKeyboardEntry--;
					return 1;
				}
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
				m_KeyboardCursorPos=0;
				return 1;
			}
			else
			{
				DeselectExpressionExceptKeyboardSelection();
				if (((theElement->Type!=11) && (theElement->Type!=12)) || (this->m_MaxNumRows==1)) SelectElement(2,m_IsKeyboardEntry-1);
				m_IsKeyboardEntry++;
				if (IsText==0)
				{
					InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
					m_KeyboardCursorPos=0;
				}
				return 1;
			}
		}
	}

	DotAutomaticallyAdded=0;

	if ((nChar&0xFF)==0x07) //DEL key - delete the character just after
	{
		/*if (BackspaceStorage==NULL) 
		{
			BackspaceStorage=new CExpression(NULL,NULL,100);
			BackspaceStorage->CopyExpression(this,0,0,0);
			BackspaceStorageOriginal=this;
		}*/

		//if we are in the empty function argument then we delete the function argument box (as if we hit the Enter key)
		if ((IsEditedVariableEmpty) && (this->m_NumElements==1) && 
			(this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==6) && (this->m_pPaternalElement->Expression1==(CObject*)this))
		{
			int pos=this->m_pPaternalElement->GetPaternalPosition();
			this->m_pPaternalExpression->InsertEmptyElement(pos+1,1,0,-1);
			KeyboardRefocus(this->m_pPaternalExpression,pos+1);

			this->m_pPaternalElement->Expression1=NULL;
			delete ((CExpression*)(this->m_pPaternalElement->Expression1));
			return 1;
		}

		//delete the character to the right of cursor, if there is one
		if (m_KeyboardCursorPos<(short)strlen(theElement->pElementObject->Data1))
		{			
			if ((lastkeystrokes[1]!=0x07) && (KeyboardEntryBaseObject))
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
			memmove(theElement->pElementObject->Data1+m_KeyboardCursorPos,theElement->pElementObject->Data1+m_KeyboardCursorPos+1,23-m_KeyboardCursorPos);
			if (theElement->Type==1)
			{
				memmove(theElement->pElementObject->Data2+m_KeyboardCursorPos,theElement->pElementObject->Data2+m_KeyboardCursorPos+1,23-m_KeyboardCursorPos);
				//we will delete index of variable if needed			
				if ((theElement->pElementObject->Data1[0]==0) && 
					(theElement->pElementObject->Expression1))
				{
					delete (CExpression*)(theElement->pElementObject->Expression1);
					theElement->pElementObject->Expression1=NULL;
				}
				if (theElement->pElementObject->Data1[0]==0)
					theElement->Decoration=0;
			}
			return 1;
		}


		//delete this variable if empty
		if ((IsEditedVariableEmpty) && (m_IsKeyboardEntry<m_NumElements) && (IsText==0))
			DeleteElement(m_IsKeyboardEntry-1);
		else
			m_IsKeyboardEntry++;

		if (m_IsKeyboardEntry>m_NumElements) 
		{
			//nothing else to delete in this expression...
			//check if the expression is completely empty, and if we can delete its paternal element
			m_IsKeyboardEntry=m_NumElements;
			if (m_NumElements==1)
			if ((m_pElementList->Type==0) ||
				((m_pElementList->Type==1) && (m_pElementList->pElementObject->Data1[0]==0)))
			{
				if ((this->m_pPaternalElement) && 
					(this->m_pPaternalElement->Expression1==(CObject*)this) &&
					(this->m_pPaternalElement->m_Type!=4) && (this->m_pPaternalElement->m_Type!=1))
				{
					CExpression *parent=this->m_pPaternalExpression;
					int i;
					for (i=0;i<parent->m_NumElements;i++)
					{
						tElementStruct *ts=parent->m_pElementList+i;
						if (ts->pElementObject==this->m_pPaternalElement)
						{							
							if (ts->IsSelected==2)
							{	
								((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
								m_IsKeyboardEntry=0;
								KeyboardEntryObject=(CObject*)parent;							
								parent->InsertEmptyElement(i,1,0,fcolor);
								parent->m_IsKeyboardEntry=i+1;
								parent->m_KeyboardCursorPos=0;
								parent->DeleteElement(i+1);
							}
							else
								this->m_pPaternalExpression->SelectElement(2,i);
							return 1;
						}
					}
				}
			}
			return 1;
		}


		//okay, there are elements to the right, check them
		theElement=m_pElementList+m_IsKeyboardEntry-1;
		if (theElement->Type==1)
		{
			//if this is another variable then go into EDIT mode
			//and delete the first character

			if (IsText)
			{
				//in the case of text box, join two words
				m_IsKeyboardEntry--;
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
				int len=(int)strlen(theElement->pElementObject->Data1);
				int lasts=23-m_KeyboardCursorPos;
				if (len<lasts) 
					lasts=len;
				else if ((theElement->Type==1) && (theElement->pElementObject->m_Text))
				{
					theElement->pElementObject->m_Text=2;
					m_IsKeyboardEntry++;
					m_KeyboardCursorPos=0;
					return 1;
				}
				if ((lasts>0) && ((theElement->pElementObject->m_Text==2) || (theElement->pElementObject->m_Text==3))) lasts--;
				theElement--;
				memcpy(theElement->pElementObject->Data1+m_KeyboardCursorPos,(theElement+1)->pElementObject->Data1+len-lasts,lasts);
				if (theElement->Type==1)
					memcpy(theElement->pElementObject->Data2+m_KeyboardCursorPos,(theElement+1)->pElementObject->Data2+len-lasts,lasts);
				theElement->pElementObject->Data1[m_KeyboardCursorPos+lasts]=0;
				DeleteElement(m_IsKeyboardEntry);
				return 1;
			}
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
			m_KeyboardCursorPos=0;
			return KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
		}
		else
		{
			//this is another type (not variable) of element. 
			//Select it.
			if ((theElement->Type==2) ||
				(KeyboardEntryBaseObject==NULL) || (KeyboardEntryBaseObject->Type==2))  //for the quick type
			{
				if (IsText)
				{
					m_IsKeyboardEntry--;
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
					DeleteElement(m_IsKeyboardEntry);
					return 1;
				}
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("del key",20304);
				DeleteElement(m_IsKeyboardEntry-1);

				DeleteElement(m_IsKeyboardEntry-1);
				if ((m_IsKeyboardEntry>=2) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==0))	
					m_IsKeyboardEntry--;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				return 1;
			}
			else
			{
				DeselectExpressionExceptKeyboardSelection();
				if (((theElement->Type!=11) && (theElement->Type!=12)) || (this->m_MaxNumRows==1)) SelectElement(2,m_IsKeyboardEntry-1);
				if (IsText==0)
				{
					InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
					m_KeyboardCursorPos=0;
				}
				else
				{
					m_IsKeyboardEntry--;
				}
				return 1;
			}
		}
	}

	
	if (this->m_pPaternalExpression)
		this->m_pPaternalExpression->DeselectExpression();
	else
		DeselectExpression(); // we have to delete everything because of the DEL and BACKSPACE keys (these keys delete selected objects)


	{
		// by adequate keystroke it is possible to create equation elements by keyboard input
		// these keystrokes are handled here

		char ch=nChar&0xFF;

		char ch_number=0;
		if (((ch>='0') && (ch<='9')) || (ch=='.') || 
			((ch==',') && (UseCommaAsDecimal) && (m_KeyboardCursorPos==strlen(theElement->pElementObject->Data1)) && (m_KeyboardCursorPos))) ch_number=1;
		//check if user entered the name of any known function (sin cos...)
		//if yes, accept that function
		if ((ch<'A') || (ch>'z') || ((ch<'a') && (ch>'Z'))) //not a letter	
		if (ch!='\'')
		if ((ch&0x80)==0)
		if (((ch<'0') || (ch>'9')) || (KeyboardExponentMode))  //not a number
		{
			//in the case we are entering non-letter and non-number
			//then check if we are entering in the variable index, exit it
			if ((!UseComplexIndexes) && (ch!=3) && (ch!=2) && (ch!='@') && (ch!='.') && (ch!='_') && ((ch!=',') || (!UseCommaAsDecimal)) && (ch!='\\'))
			if ((m_IsKeyboardEntry>1) || (m_KeyboardCursorPos>0) || (ch==' '))
			if (!KeyboardIndexMode)
			if ((this->m_pPaternalElement) && 
				(((this->m_pPaternalElement->m_Type==1) && (this->m_pPaternalElement->Expression1==(CObject*)this)) ||
					((this->m_pPaternalElement->m_Type==5) && (this->m_pPaternalElement->Expression2==(CObject*)this)) ||
					((this->m_pPaternalElement->m_Type==6) && (this->m_pPaternalElement->Expression2==(CObject*)this) && (strcmp(this->m_pPaternalElement->Data1,"lim")) )))
			{
				int iii;
				for (iii=0;iii<this->m_pPaternalExpression->m_NumElements;iii++)
				{
					tElementStruct *theElement2=this->m_pPaternalExpression->m_pElementList+iii;
					if (theElement2->pElementObject==this->m_pPaternalElement)
						break;
				}
				if ((iii<this->m_pPaternalExpression->m_NumElements))
				{
					if ((IsEditedVariableEmpty) && (m_NumElements==1))
					{
						//we are deleting the index
						DeleteElement(0);
					}

					int tmpret=ResolveKnownFunctions(DC,zoom,nChar,nRptCnt,nFlags,fcolor,theElement);
					/*if (tmpret) */IsSpacebarOnlyHit=0;
					if (tmpret==2) return 1; //editing is continued at other expression
					theElement=m_pElementList+m_IsKeyboardEntry-1;
					if ((theElement->Type==1) && (theElement->pElementObject->Data1[0]==0))
						DeleteElement(m_IsKeyboardEntry-1);
					iii++;
					m_IsKeyboardEntry=0;
					if (lastkeystrokes[0]==32) lastkeystrokes[0]=0x0D; //just because the spacebar would prevent 'mighty dot' operation
					if ((this->m_pPaternalElement->m_Type==6) &&
						(((CExpression*)(this->m_pPaternalElement->Expression1))->m_pElementList->Type==0))
					{
						CExpression *arg=(CExpression*)((this->m_pPaternalElement)->Expression1);
						KeyboardRefocus(arg,0);
						((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
						return 1;
					}
					((CExpression*)(this->m_pPaternalExpression))->InsertEmptyElement(iii,1,0,fcolor);
					((CExpression*)(this->m_pPaternalExpression))->m_IsKeyboardEntry=iii+1;
					((CExpression*)(this->m_pPaternalExpression))->m_KeyboardCursorPos=0;
					KeyboardEntryObject=(CObject*)(this->m_pPaternalExpression);
					((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
					
					return 1;
				}
			}

			//check the variable we are currently editing. if it is one of known functions then make it so.
			int tmpret=ResolveKnownFunctions(DC,zoom,(nChar==13)?32:nChar,nRptCnt,nFlags,fcolor,theElement);
			if (tmpret) IsSpacebarOnlyHit=0;
			if (tmpret==2) return 1; //editing is continued at other expression
			if (tmpret==1) 
			{
				theElement=m_pElementList+m_IsKeyboardEntry-1;
				EditedVariable=theElement->pElementObject;
				IsEditedVariableEmpty=(strlen(EditedVariable->Data1)==0)?1:0;
			}
		}
		

		if ((ch=='\\')) //special handling for the backslash
		{
			theElement=KeyboardSplitVariable();
			if (m_KeyboardCursorPos==0)
				IsEditedVariableEmpty=1;
			else if ((m_KeyboardCursorPos==1) && (theElement->pElementObject->Data1[0]=='\\'))
			{
				if (m_KeyboardCursorPos==1) 
				{
					DeleteElement(m_IsKeyboardEntry-1);
					m_IsKeyboardEntry--;
				}
				else 
					theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=0;
				m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
				goto toggle_keymode;
				
				IsText=(IsText)?0:1;
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
				goto show_keymode_label;

				//doube backslash - we are starting the new equation just below
				/*DeleteElement(m_IsKeyboardEntry-1);
				m_IsKeyboardEntry=0;
				this->KeyboardInsertNewEquation(DC,zoom,nChar,0,IsText);
				return 1;*/
			}
			else
			{
				m_IsKeyboardEntry++;
				m_KeyboardCursorPos=0;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				theElement=m_pElementList+m_IsKeyboardEntry-1;
				IsEditedVariableEmpty=1;
			}
		}
		if (/*(IsText==0) &&*/ (m_KeyboardCursorPos==1) && (theElement->pElementObject->Data1[0]=='\\') &&
			((ch=='@') || (ch=='*') || (ch=='#') || (ch=='\"')))
		{
				//special handling of express commands
				DeleteElement(m_IsKeyboardEntry-1);				
				if (ch=='@') InsertEmptyElement(m_IsKeyboardEntry-1,1,'@',fcolor);
				if (ch=='*') InsertEmptyElement(m_IsKeyboardEntry-1,2,'*',fcolor);
				if (ch=='#') InsertEmptyElement(m_IsKeyboardEntry-1,2,'#',fcolor);
				if (ch=='\"') InsertEmptyElement(m_IsKeyboardEntry-1,1,'\"',fcolor);
				m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				return 1;
		}

		if (IsText==0)
		if ((ch==')') || (ch==']') || (ch=='}') || (ch=='>')) //exits the parentheses element
		{
			if ((m_KeyboardCursorPos==1) && (theElement->pElementObject->Data1[0]=='\\'))
			{
				//handling \} command (inserting right curly bracked)
				DeleteElement(m_IsKeyboardEntry-1);				
				char chx=')';if (ch!='(') chx=ch-2;
				InsertEmptyElement(m_IsKeyboardEntry-1,5,chx,fcolor);
				CExpression *e=(CExpression*)(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1;
				e->m_ParenthesesFlags|=0x08; //no right bracket
				e->m_Alignment=2;

				for (int i=0;i<m_IsKeyboardEntry-1;i++)
					e->InsertElement(m_pElementList+i,e->m_NumElements);
				for (int i=0;i<m_IsKeyboardEntry-1;i++)
					DeleteElement(0);

				m_IsKeyboardEntry=2;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				return 1;
			}

		
			if ((ch==']') && (lastkeystrokes[1]=='<') && (m_IsKeyboardEntry>1) && ((theElement-1)->Type==2) && ((theElement-1)->pElementObject->Data1[0]=='<'))
			{
				//support for < | parentheses (bra)
				DeleteElement(m_IsKeyboardEntry-2);
				InsertEmptyElement(m_IsKeyboardEntry-2,5,'a',fcolor);
				CExpression *e=(CExpression*)((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Expression1);
				KeyboardRefocus(e,0);
				return 1;

			}
			if ((this->m_DrawParentheses) && (lastkeystrokes[2]==this->m_ParentheseShape) && (lastkeystrokes[1]==-1))
			{
				if ((this->m_ParentheseShape=='(') && (ch==']'))
				{
					this->m_ParentheseShape='l'; //support for ( ] parentheses
					return 1;
				}

				if ((this->m_ParentheseShape=='[') && (ch==')'))
				{
					this->m_ParentheseShape='r'; //support for [  ) parentheses
					return 1;
				}
				if ((this->m_ParentheseShape=='|') && (ch=='>'))
				{
					this->m_ParentheseShape='k'; //support for | > parentheses
					return 1;
				}
				if ((this->m_ParentheseShape=='[') && (ch=='>'))
				{
					this->m_ParentheseShape='k'; //support for | > parentheses
					return 1;
				}
			}

			if (ch!='>')
			{
				if ((m_pPaternalElement) && (m_pPaternalExpression))
				{

					//check if this expression has parentheses
					CExpression *exp=this;
					do
					{
						if (((exp->m_DrawParentheses=='(') && (ch==')')) ||
							((exp->m_DrawParentheses=='[') && (ch==']')) ||
							((exp->m_DrawParentheses=='{') && (ch=='}')))
						{
							break;
						}
						exp=exp->m_pPaternalExpression;
					} while (exp);//(exp->m_pPaternalExpression);

					//finding the paternal element in its paternal expression
					
					if (exp==NULL) exp=this;
					if (exp->m_pPaternalExpression==NULL) exp=this;
					CExpression *parent=exp->m_pPaternalExpression;
					CElement *pelement=exp->m_pPaternalElement;
					if ((pelement->m_Type==6) && (pelement->Expression1==(CObject*)this) &&
						(parent->m_pPaternalElement) && (parent->m_pPaternalElement->m_Type==3) && (parent->m_NumElements==1) &&
						(parent->m_pPaternalElement->Expression1==(CObject*)parent))
					{
						//exponent over function! - we are jumping all the way out of the exponent element -//BABA
						pelement=parent->m_pPaternalElement;
						parent=parent->m_pPaternalExpression;
					}
					for (int ii=0;ii<parent->m_NumElements;ii++)
					{
						tElementStruct *ts=parent->m_pElementList+ii;
					
						if (ts->pElementObject==pelement)
						//if ((ts->pElementObject) && 
						//	((ts->pElementObject->Expression1==(CObject*)exp) ||
						//	(ts->pElementObject->Expression2==(CObject*)exp) ||
						//	(ts->pElementObject->Expression3==(CObject*)exp)))
						{			
							if (IsEditedVariableEmpty) 	DeleteElement(m_IsKeyboardEntry-1);
							KeyboardRefocus(parent,ii+1);
							return 1;
						}
					}
				}
				else if ((m_KeyboardCursorPos==1) && (ch==')') && (theElement->pElementObject->Data1[0]>32))
				{
					//handling entries like: a), b).... 
					theElement->pElementObject->Data1[1]=ch;
					theElement->pElementObject->Data1[2]=0;
					InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
					m_KeyboardCursorPos=0;

				}
				return 1;
				
			}
		}

		if (IsText==0)
		if ((ch=='(') || (ch=='[') || (ch=='{') || (ch=='|') || (ch=='<')) //inserts the parentheses element (type=5)
		{
			//spliting the current variable if neccessary - the new element will be placed in between
			int single_side=0;
			if ((m_KeyboardCursorPos==1) && (theElement->pElementObject->Data1[0]=='\\'))
			{
				single_side=1; //commands like \{  \[   \<  (adding left side parenthese only)
			}
			if ((ch!='<') || (single_side))
			{
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("parentheses entry",20308);
			
				if (!single_side)
				{
					if (theElement->Type==6)
					{
						//if we were editing function name (keyboard implanting)
						CExpression *arg=(CExpression*)(theElement->pElementObject->Expression1);
						arg->m_ParenthesesFlags|=0x01;
						arg->m_ParentheseShape=ch;
						//arg->m_ParentheseHeightFactor=1;
						
						m_IsKeyboardEntry++;
						InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
						m_KeyboardCursorPos=0;
						return 1;				
					}
					if (((CExpression*)(KeyboardEntryObject))->m_pPaternalElement)
					{
						CElement *elm=((CExpression*)(KeyboardEntryObject))->m_pPaternalElement;
						if ((elm->m_Type==6) && ((CObject*)this==elm->Expression1))
						{
							//we are in the argument of function
							if (!InhibitParentheseMerging)
							if ((m_NumElements==1) && (IsEditedVariableEmpty) && (lastkeystrokes[1]!=' ') &&
								(((m_ParenthesesFlags&0x01)==0) || (m_ParentheseShape<ch)))
							{
								//if the first key pressed inside function argument is open parenthese
								//then enable parentheses for this function
								m_ParenthesesFlags|=0x01;
								m_ParentheseShape=ch;
								//m_ParentheseHeightFactor=1;
								return 1;
							}					
						}
						if ((elm->m_Type==5) && ((CObject*)this==elm->Expression1) && 
							(((CExpression*)elm->Expression1)->m_ParentheseShape=='|') &&
							(lastkeystrokes[2]=='|') && (lastkeystrokes[1]==-1) && (ch=='|'))
						{
							((CExpression*)elm->Expression1)->m_ParentheseShape='\\'; //double-bar
							return 1;
						}
					}
					if ((m_IsKeyboardEntry>1) && ((theElement-1)->Type==6) && ((theElement-1)->pElementObject->Expression1==NULL) && (ch=='('))
					{
						//there is a function without argument in front of cursor, we are generating argument now and make parentheses visible
						if (IsEditedVariableEmpty) DeleteElement(m_IsKeyboardEntry-1);
						CExpression *e;
						e=new CExpression((theElement-1)->pElementObject,this,this->m_FontSize);
						(theElement-1)->pElementObject->Expression1=(CObject*)e;
						e->m_ParenthesesFlags|=0x01;
						e->m_ParentheseShape=ch;
						//e->m_ParentheseHeightFactor=1;
						KeyboardRefocus(e,0);
						return 1;
					}
				}

				if (single_side) 
					IsEditedVariableEmpty=1;
				else
				{
					theElement=KeyboardSplitVariable();
					if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;
				}

				if (IsEditedVariableEmpty) 
					DeleteElement(m_IsKeyboardEntry-1);
				else
					m_IsKeyboardEntry++;

				InsertEmptyElement(m_IsKeyboardEntry-1,5,ch,fcolor);
				theElement=m_pElementList+m_IsKeyboardEntry-1;
				if (single_side)
				{
					CExpression *e=(CExpression*)theElement->pElementObject->Expression1;
					e->m_ParenthesesFlags|=0x10; //no right bracket
					e->m_Alignment=1;
				}

				KeyboardRefocus(((CExpression*)(theElement->pElementObject->Expression1)),0);
				return 1;
			}
			
		}


		//handling ALT+, (adds vector, dot and dash over a variable)
		if ((IsALTDown) && ((ch==',') || (ch=='.')) && (!KeyboardExponentMode))
		{
			int addnew=1;
			if (theElement->pElementObject->Data1[0]==0)  
				if (m_IsKeyboardEntry>1) {theElement--;addnew=0;} else return 1;
			if (theElement->Type==1)
			{
				unsigned char *f=&(theElement->pElementObject->m_VMods);
				unsigned char format=*f;
				if (ch==',')
				{
					if (format==0x04) *f=0x0C;  //hat
					else if (format==0x08) *f=0x04;  //dash
					else *f=0x08;	 //vector	
				}
				else
				{
					if (format==0x18) *f=0x20; //triple double dot
					else if (format==0x14) *f=0x18; //double dot
					else *f=0x14;  //dot
				}

				if (addnew)
				{
					InsertEmptyElement(m_IsKeyboardEntry,1,0,fcolor);
					m_IsKeyboardEntry++;
					m_KeyboardCursorPos=0;
				}
				return 1;
			}
		}

		//handling cases where user enters sequences like ^* , ^^  (to write variables like: x* or x^)
		if ((ch=='*')  || (ch=='\"') || (ch==94) || (ch=='~'))
		{
			CElement* pelem=(CElement*)this->m_pPaternalElement;
			if ((this->m_NumElements==1) && (m_KeyboardCursorPos==0) && (IsEditedVariableEmpty))
			if ((pelem) && (pelem->m_Type==3) && (pelem->Expression2==(CObject*)this))
			if (( ((CExpression*)pelem->Expression1)->m_NumElements==1) && 
				(((CExpression*)pelem->Expression1)->m_pElementList->Type==1))
			{
				//this handles sequence ^*	(putting the star)
				int i;
				CExpression *baseline=(CExpression*)pelem->m_pPaternalExpression;
				for (i=0;i<baseline->m_NumElements;i++)
				{
					if ((baseline->m_pElementList+i)->pElementObject==pelem)
					{						
						baseline->InsertElement(((CExpression*)pelem->Expression1)->m_pElementList,i);

						int len=(int)strlen((baseline->m_pElementList+i)->pElementObject->Data1);
						char tx=baseline->DetermineInsertionPointType(i);
						if (len>22) len=22;
						if ((ch=='*') || (ch=='\"'))
						{
							(baseline->m_pElementList+i)->pElementObject->Data1[len]=nChar&0xFF;
							(baseline->m_pElementList+i)->pElementObject->Data2[len]=0;
							len++;
							(baseline->m_pElementList+i)->pElementObject->Data1[len]=0;
						}
						else 
						{
							if (ch==94) 
								(baseline->m_pElementList+i)->pElementObject->m_VMods=0x0C; //hat
							else
								(baseline->m_pElementList+i)->pElementObject->m_VMods=0x24; //tilde
						}
						i++;
						KeyboardRefocus(baseline,i);
						(baseline->m_pElementList+i)->pElementObject->m_Text=tx;
						baseline->DeleteElement(i+1);
						return 1;
					}
				}						
			}
		}


		//ALT+something - placing into exponent (or index)		
		
		/*if (IsALTDown)
		{			
			if (1)//((ch_number) || (ch==',') || (ch=='+') || (ch=='-'))
			{
				theElement=KeyboardSplitVariable();
				if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;
				if (IsEditedVariableEmpty) 
				{
					DeleteElement(m_IsKeyboardEntry-1);
					m_IsKeyboardEntry--;
					theElement=m_pElementList+m_IsKeyboardEntry-1;
				}

				CExpression *ToAdd=NULL;
				if ((m_IsKeyboardEntry>0) && ((theElement)->Type==2) &&  
					((m_IsKeyboardEntry==0) || ((theElement-1)->Type!=2)))
				{
					//there is an operator just before (remember it and delete it)
					ToAdd=new CExpression(NULL,NULL,100);
					ToAdd->InsertElement(theElement,0);
					DeleteElement(m_IsKeyboardEntry-1);
					m_IsKeyboardEntry--;
				}

				CExpression *exp=NULL;
				if (m_IsKeyboardEntry<=0)
				{
					if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==6) &&
						(((CObject*)this==this->m_pPaternalElement->Expression1) || (IsSimpleVariableMode)))
					{
						//we are in function argument - powering the function
						int ii;
						for (ii=0;ii<this->m_pPaternalExpression->m_NumElements;ii++)
							if ((this->m_pPaternalExpression->m_pElementList+ii)->pElementObject==this->m_pPaternalElement) break;
						CExpression *base=this->m_pPaternalExpression;						
						if ((ii!=0) || (base->m_NumElements!=1) || (base->m_pPaternalElement==0) ||
							(base->m_pPaternalElement->m_Type!=3))
						{

							tElementStruct ts;
							ts=*(base->m_pElementList+ii);
							
							CElementInitPaternalExpression=(CObject*)this->m_pPaternalExpression;
							CElementInitType=3;
							CElement *exp=new CElement();
							exp->Empty(0);

							(base->m_pElementList+ii)->pElementObject=exp;
							(base->m_pElementList+ii)->Type=3;

							CExpression *newbase=(CExpression*)((base->m_pElementList+ii)->pElementObject->Expression1);
							if (IsSHIFTALTDown)
							{
								CExpression *newexp=(CExpression*)((base->m_pElementList+ii)->pElementObject->Expression2);
								newexp->InsertEmptyElement(0,2,'-',fcolor);
							}
							*(newbase->m_pElementList)=ts;
						
							this->m_pPaternalExpression=newbase;
							this->m_pPaternalElement->m_pPaternalExpression=(CObject*)newbase;
						}
						exp=(CExpression*)this->m_pPaternalExpression;
						if (exp)
						{
							exp=(CExpression*)exp->m_pPaternalElement->Expression2;
							m_IsKeyboardEntry=0;
							goto keyboardkeyhit_addtoexponent;
						}
						InsertEmptyElement(0,1,0,fcolor);
						m_IsKeyboardEntry=1;
						m_KeyboardCursorPos=0;
						if (ToAdd) delete ToAdd;
						return 1;
					}
					m_IsKeyboardEntry=1;
					m_KeyboardCursorPos=0;
					InsertEmptyElement(0,1,0,fcolor);
					if (ToAdd) delete ToAdd;
					return 1;
				}

				theElement=m_pElementList+m_IsKeyboardEntry-1;

				if (theElement->Type!=2)
				{
					if (theElement->Type!=3)
					{
						//we are going to add an 'exponent' element here
						tElementStruct* theElement2;
						InsertEmptyElement(m_IsKeyboardEntry-1,3,0,fcolor); //insert power element
						theElement2=m_pElementList+m_IsKeyboardEntry-1; //power element
						theElement=m_pElementList+m_IsKeyboardEntry; //non-operator element
						if (theElement2->pElementObject->Expression1)
						{
							if (theElement->pElementObject) theElement->pElementObject->m_Text=0;
							((CExpression*)(theElement2->pElementObject->Expression1))->InsertElement(theElement,0);
							this->DeleteElement(m_IsKeyboardEntry);
							if ((IsSHIFTALTDown) && (ch_number))
							{
								((CExpression*)(theElement2->pElementObject->Expression2))->InsertEmptyElement(0,2,'-',fcolor);
							}
							ExponentAcceleratorUsed=1;
						}
					}

					tElementStruct* theElement2;
					theElement2=m_pElementList+m_IsKeyboardEntry-1; //power element
					exp=((CExpression*)theElement2->pElementObject->Expression2);

keyboardkeyhit_addtoexponent:
					if (ToAdd) exp->InsertElement(ToAdd->m_pElementList,exp->m_NumElements);
					char palt=IsALTDown;IsALTDown=0;
					KeyboardEntryObject=(CObject*)exp;
					if ((exp->m_pElementList+exp->m_NumElements-1)->Type==1)
					{
						exp->m_IsKeyboardEntry=exp->m_NumElements;
						exp->m_KeyboardCursorPos=(int)strlen((exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Data1);
					}
					else
					{
						exp->InsertEmptyElement(exp->m_NumElements,1,0,fcolor);
						exp->m_KeyboardCursorPos=0;
						exp->m_IsKeyboardEntry=exp->m_NumElements;
					}
					int tmpeau=ExponentAcceleratorUsed;
					exp->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags);
					ExponentAcceleratorUsed=tmpeau;
					IsALTDown=palt;
					exp->KeyboardStop();

					exp->m_IsKeyboardEntry=0;
					KeyboardEntryObject=(CObject*)this;
				}
				if (ToAdd) delete ToAdd;
				m_IsKeyboardEntry++;
				m_KeyboardCursorPos=0;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				return 1;
			}
		}*/

		//inserting Greek symbols - by using double-stroke combination
		char cnvOk=1;
		char isdelta=0;
		if ((theElement->pElementObject->Data1[0]=='D') && 
			((theElement->pElementObject->Data2[0]&0xE0)==0x60) && 
			(m_KeyboardCursorPos==2))
			isdelta=1;

		if (InhibitAllKeyHandling) cnvOk=0;
		if (lastkeystrokes[1]!=(nChar&0xFF)) cnvOk=0;
		if ((m_KeyboardCursorPos!=1) && (!isdelta)) cnvOk=0;
		if ((m_KeyboardCursorPos!=2) && (isdelta)) cnvOk=0;
		if (theElement->Type!=1) cnvOk=0;
		if (theElement->pElementObject->Data1[m_KeyboardCursorPos]!=0) cnvOk=0;
		if (((theElement->pElementObject->m_VMods)==0x10) && (nChar!='u')) cnvOk=0; //if it is a measurement unit


		if (cnvOk)
		{
			unsigned char ch=nChar&0xFF;
			int now=GetTickCount();
			unsigned int ttt=750; 
			if (IsHighQualityRendering) ttt+=70;
			if (IsHalftoneRendering) ttt+=70;
			if (ViewZoom>280) ttt+=(ttt-200)/3;

			char is_in_index=0;
			if ((this->m_pPaternalElement) && 
				(((this->m_pPaternalElement->m_Type==1) && (this==(CExpression*)this->m_pPaternalElement->Expression1)) ||
				((this->m_pPaternalElement->m_Type==6) && (this==(CExpression*)this->m_pPaternalElement->Expression2)) ||
				((this->m_pPaternalElement->m_Type==5) && (this==(CExpression*)this->m_pPaternalElement->Expression2))))
				is_in_index=1;

			if (ch!='o')
			if (((ch>='A') && (ch<='Z')) || ((ch>='a') && (ch<='z')))
			if (((theElement->pElementObject->Data1[m_KeyboardCursorPos-1]==ch)) ||
				((ch=='u') && (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='m') && (IsDoubleStrokeConversionUsed==2)))
			{
					if ((ch=='R') || (ch=='N') || (ch=='C') || (ch=='Z') || (ch=='K'))
					{
						if (!is_in_index)
						if (theElement->pElementObject->Data2[0]!=0x21)
						{
							if (((now-lastkeystroketime)<ttt) ) 
							{
								IsDoubleStrokeConversionUsed=1;
								theElement->pElementObject->Data2[0]=0x21;
								if (ch=='K') {theElement->pElementObject->Data1[0]='Q';}
								return 1;
							}
						}
						else if ((IsSimpleVariableMode==0) || ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==1)))
						{
							//we will revert back to latin if the key is hit the third time (only in indexes or in general variable mode
							lastkeystrokes[0]=0;
							int rv=Toolbox->KeyboardHit(nChar,(IsSHIFTDown)?0x02:0x00);
							theElement->pElementObject->Data2[0]=rv&0xFF;
							theElement->pElementObject->m_VMods=(rv>>16)&&0xFF;
							//continuing
						}
					}
					else if ((ch=='A') || (ch=='E') || (ch=='I') || (ch=='T') || (ch=='O') || (ch=='U') || (ch=='M') || (ch=='B') || (ch=='H'))
					{
						if (!is_in_index)
						if (!IsText)
						if (((now-lastkeystroketime)<ttt))
						{
							unsigned char oper=0;
							if (!is_in_index)
							{
								if (ch=='H') oper=0xD1; //nabla
								if (ch=='B') oper=0xB7; //bullet
								if (ch=='U') oper=0xC8; //union
								if (ch=='M') oper='|'; //mid
							}
							if (ch=='A') oper=0x22; //for all
							if (ch=='E') oper=0x24; //exists
							if (ch=='I') oper='P'; //parralel
							if (ch=='T') oper='p'; //parepndicular
							if (ch=='O') oper=0x03; //circle (composition)
							
							if (oper)
							{
								IsDoubleStrokeConversionUsed=1;
								DeleteElement(m_IsKeyboardEntry-1);
								InsertEmptyElement(m_IsKeyboardEntry-1,2,oper,fcolor);
								m_IsKeyboardEntry++;
								m_KeyboardCursorPos=0;
								InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
								return 1;
							}
							
						}
					}
					else if ((theElement->pElementObject->Data2[m_KeyboardCursorPos-1]&0xE0)!=0x60)
					{
						if ((now-lastkeystroketime)<ttt)
						{
							IsDoubleStrokeConversionUsed=1;
							if (nChar=='u') {nChar='m';theElement->pElementObject->Data1[m_KeyboardCursorPos-1]='m';IsDoubleStrokeConversionUsed=2;}
							int rv=Toolbox->KeyboardHit(nChar,(IsSHIFTDown)?0x12:0x10);
							if ((theElement->pElementObject->m_VMods)==0x10) rv=(0x10<<16)|0x60;
							theElement->pElementObject->Data2[m_KeyboardCursorPos-1]=rv&0xFF;
							theElement->pElementObject->m_VMods=(rv>>16)&0xFF;
							return 1;
						}
					}
					else  if ((IsSimpleVariableMode==0) || (IsText) || ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==1)))
					{
						//we will revert back to latin if the key is hit the third time (only in indexes or in general variable mode
						lastkeystrokes[0]=0;
						int rv=Toolbox->KeyboardHit(nChar,(IsSHIFTDown)?0x02:0x00);
						theElement->pElementObject->Data2[0]=rv&0xFF;
						theElement->pElementObject->m_VMods=(rv>>16)&0xFF;
						if ((IsDoubleStrokeConversionUsed==2) && (nChar=='u')) theElement->pElementObject->Data1[0]='u';
						//continuing
					}
					
			}
		}

		if ((ch==',') && ((KeyboardExponentMode) || (KeyboardIndexMode)))
			Autocomplete(0);

		if ((AutocompleteSource) && (ch==',') && (m_KeyboardCursorPos==0))
		{

			//ACTIVATES autocompletion 
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("autocomplete",20316);

			lastkeystrokes[0]=0;
			while ((this->m_pElementList->Type) || (this->m_NumElements>1)) this->DeleteElement(0);
			for (int i=0;i<AutocompleteSource->m_NumElements;i++)
				this->InsertElement(AutocompleteSource->m_pElementList+i,this->m_NumElements);

			this->InsertEmptyElement(m_NumElements,1,0,fcolor);
			this->m_IsKeyboardEntry=m_NumElements;
			this->m_KeyboardCursorPos=0;

			IsSHIFTDown=0;
			return KeyboardKeyHit(DC,zoom,0x0D,0,0,0,0);
		}


		unsigned short unicode;
		unsigned char chh=(nChar)&0xFF;
		//if (!MultiByteToWideChar(CP_THREAD_ACP,0,(LPCSTR)&chh,1,(LPWSTR)&unicode,1)) unicode=0;
		if (!MultiByteToWideChar(GetACP(),0,(LPCSTR)&chh,1,(LPWSTR)&unicode,1)) unicode=0;

		if (IsText==0)
		if (((ch=='+') || (ch=='-') || (ch=='=')) && (lastkeystrokes[1]==ch) && (m_IsKeyboardEntry>1) && ((this->m_pElementList+m_IsKeyboardEntry-2)->Type==2) && ((this->m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]==ch))
		{
			//handling ++ and -- and == sequences
			if (m_pPaternalElement==NULL)
			{
				//inserting new line
				int found_break=0;
				if ((this->m_MaxNumColumns==1) && (this->m_MaxNumRows==1))
				for (int i=0;i<m_NumElements;i++)
				{
					if (((m_pElementList+i)->Type==2) && ((m_pElementList+i)->pElementObject->Data1[0]==(char)0xFF))
					{
						found_break=1;
					}
				}
				if (!found_break) m_Alignment=1;

				int is_newline=0;
				if ((m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==2) &&
					((m_IsKeyboardEntry==2) || (((m_pElementList+m_IsKeyboardEntry-3)->Type==2) && 
					((m_pElementList+m_IsKeyboardEntry-3)->pElementObject->Data1[0]==(char)0xFF))))
				{
					char c=(m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0];
					if (((c=='=') || (c=='+') || (c=='-')) && (c==ch))
						is_newline=1;
				}

				if (is_newline)
				{
					//if we are at the line beginning, then we are not generating new line - just add a tab spacer
					DeleteElement(m_IsKeyboardEntry-2);
					m_IsKeyboardEntry--;
				}
				else
				{
					//we are generating the newline
					InsertEmptyElement(m_IsKeyboardEntry-1,2,(char)0xFF,fcolor);
					m_IsKeyboardEntry++;
				}

				if ((this->m_MaxNumColumns==1) && (this->m_MaxNumRows==1))
					m_IsKeyboardEntry+=this->InsertTabSpacer(m_IsKeyboardEntry-1,1);
			}
			else
			{
				//existing the element
				DeleteElement(m_IsKeyboardEntry-2);
				m_IsKeyboardEntry--;
				int sh=IsSHIFTDown;
				int sa=IsSHIFTALTDown;
				int al=IsALTDown;
				IsSHIFTDown=IsSHIFTALTDown=IsALTDown=0;
				KeyboardKeyHit(DC,zoom,0x0D,nRptCnt,0,0,0);
				IsSHIFTDown=sh;
				IsALTDown=al;
				IsSHIFTALTDown=sa;
				((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,ch,nRptCnt,nFlags,fcolor,0);
				return 1;
			}
		}

		/*if ((ch==',') && (lastkeystrokes[1]==ch))
		{
			//double comma stroke changes the text/math typing mode
			if ((this->m_KeyboardCursorPos>0) && (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]==','))
			{
				if (m_KeyboardCursorPos==1) 
				{
					DeleteElement(m_IsKeyboardEntry-1);
					m_IsKeyboardEntry--;
				}
				else 
					theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=0;
				m_IsKeyboardEntry++;
				InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				m_KeyboardCursorPos=0;
				IsEditedVariableEmpty=1;
				goto toggle_keymode;
				
				IsText=(IsText)?0:1;
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
				goto show_keymode_label;
			}
			else if ((m_IsKeyboardEntry>1) && ((theElement-1)->Type==2) && ((theElement-1)->pElementObject->Data1[0]==',') && (IsEditedVariableEmpty))
			{
				DeleteElement(m_IsKeyboardEntry-2);
				m_IsKeyboardEntry--;
				goto toggle_keymode;
				IsText=(IsText)?0:1;
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
				goto show_keymode_label;
				
			}
		}*/

		if (((m_IsKeyboardEntry>1) || (m_KeyboardCursorPos>1)) && (ch=='?') && (IsText==0) && 
			(KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1)) //to prevent opening when drawing box command line or function plotter line
		{
			//opening the popup menu and starting the computation

			//if '?' character is entered, we will check for two successive '?'

			char prev=lastkeystrokes[1];
			if ((ch=='?') && (prev=='?'))
			{
				if ((m_KeyboardCursorPos>0) && (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='?'))
					theElement->pElementObject->Data1[--m_KeyboardCursorPos]=0;

				if (m_KeyboardCursorPos)
				{
					m_IsKeyboardEntry++;
					m_KeyboardCursorPos=0;
					InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				}

				//adding the '=' operator
				if ((m_IsKeyboardEntry==1) ||
					((m_pElementList+m_IsKeyboardEntry-2)->Type!=2) ||
					((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]!='='))
				{
					InsertEmptyElement(m_IsKeyboardEntry-1,2,'=',fcolor);
					m_IsKeyboardEntry++;
					theElement=m_pElementList+m_IsKeyboardEntry-1;
				}

				//InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
				TempPopupExpression=new CExpression(NULL,NULL,100);
				int ii=m_IsKeyboardEntry-3;
				while (ii>=0)
				{
					tElementStruct *ts=m_pElementList+ii;
					if (ts->Type==2)
					{
						char ch2=ts->pElementObject->Data1[0];
						if (GetOperatorLevel(ch2)<=GetOperatorLevel('=')) break;
					}
					if ((ts->Type==11) || (ts->Type==12)) break;
					TempPopupExpression->InsertElement(ts,0);
					ii--;
				}
				if (TempPopupExpression->m_pElementList->Type)
				{
					short l,a,b;
					TempPopupExpression->CalculateSize(DC,ViewZoom,&l,&a,&b);						
					Popup->ShowPopupMenu(TempPopupExpression,theApp.m_pMainWnd,3,0);
				}
				else
				{
					delete TempPopupExpression;
					TempPopupExpression=NULL;
				}									
				return 1;																	
			}
		}

		
		//handling operator entry (including space bar)
		if ((InhibitAllKeyHandling==0) || (ch==' '))
		if ((IsText==0) || (ch==' ') || (ch=='*'))
		if ((ch=='+') || (ch=='-') || (ch=='*') || ((ch==',') && (!ch_number)) || (ch==';') ||
			(ch=='/') || (ch=='<') || (ch=='>') || (ch=='=') || (ch==':') || (ch=='`') || 
			(ch=='&') || (ch=='|') || (ch==' ') || (unicode==(unsigned char)'÷') || (unicode==(unsigned char)'×') || (unicode==(unsigned char)0xAC) || (ch=='!') ||
			(ch=='~') || (ch=='#') || 
			((ch>=2) && (ch<=5) && (IsALTDown))  //arrows
			)
		{			

			int first=0;
			int was_spacebaronlyhit=IsSpacebarOnlyHit;
			if (m_IsKeyboardEntry<=1) first=1;
			else if ((m_pElementList+m_IsKeyboardEntry-2)->Type!=0) 
			{
				CElement *elm=(m_pElementList+m_IsKeyboardEntry-2)->pElementObject;
				if ((elm) && ((elm->m_Type==11) || (elm->m_Type==12)))
					first=1;
			}
			if ((ch==' ') && (IsEditedVariableEmpty==0)) IsSpacebarOnlyHit=0;
			if ((!IsALTDown) && (!IsSHIFTALTDown) && (nChar==' ') && (first==0)  && (IsEditedVariableEmpty) && (IsSpacebarOnlyHit==10) /* && (lastkeystrokes[1]==' ')*/)  //space adds small spacing
			{
				//inserting small spacer
				InsertEmptyElement(m_IsKeyboardEntry-1,2,9,fcolor);
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1[3]=1;
				m_IsKeyboardEntry++;
				return 1;
			}

			if ((m_pPaternalElement) && (m_pPaternalElement->IsDifferential(1)))
			{
				//inside differentials, any operator automatically exits the differential argument //BABA2
				int ii=0;
				for (ii=0;ii<m_NumElements;ii++) 
					if (((m_pElementList+ii)->Type!=2) && ((ii!=m_IsKeyboardEntry-1) || (m_KeyboardCursorPos>0))) break;
				if (ii<m_NumElements)
				if ((m_IsKeyboardEntry==m_NumElements) && (m_KeyboardCursorPos==strlen(theElement->pElementObject->Data1)))
				if ((m_NumElements>1) || (m_KeyboardCursorPos>0))
				{
					int prevsh=IsSHIFTDown;IsSHIFTDown=0;
					int preval=IsALTDown;IsALTDown=0;
					KeyboardKeyHit(DC,zoom,0x0D,0,0,0,0);
					IsSHIFTDown=prevsh;
					IsALTDown=preval;
					((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
					return 1;
				}
				
			}
			if (theElement->Type==6)
			{
			
				//DA LI SE OVO IKAD KORISTI???
				//if we are editing function name, any operator will jump into the 
				//function argument
				if (((CExpression*)theElement->pElementObject->Expression1))
				if (((CExpression*)theElement->pElementObject->Expression1)->m_pElementList->Type==0)
				{
					KeyboardRefocus((CExpression*)theElement->pElementObject->Expression1,0);
					/*this->m_IsKeyboardEntry=0;
					KeyboardEntryObject=theElement->pElementObject->Expression1;
					((CExpression*)KeyboardEntryObject)->InsertEmptyElement(0,1,0,fcolor);
					((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry=1;
					((CExpression*)KeyboardEntryObject)->m_KeyboardCursorPos=0;*/
					return ((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0);
				}
			}


			//spliting the current variable if neccessary - the new element will be placed in between
			if (ch!=' ')
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("operator entry",20309);

			if (ch==3) ch=(char)0xAD; //up arrow
			if (ch==2) ch=(char)0xAF; //down arrow
			if (ch==4) ch=(char)0xAC; //left arrow
			if (ch==5) ch=(char)0xAE; //right arrow

			//if the ' ' is typed after comma - turn the comma into operator (when comma is allowed as decimal separator)
			if ((ch==' ') && (theElement->Type==1) && (!IsText))
			{
				int len=(int)strlen(theElement->pElementObject->Data1);
				if ((len>0) && (theElement->pElementObject->Data1[len-1]==','))
				{
					(theElement)->pElementObject->Data1[len-1]=0;
					ch=',';
				}
			}


			theElement=KeyboardSplitVariable();
			if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;

			if (IsEditedVariableEmpty) 
				DeleteElement(m_IsKeyboardEntry-1);
			else
			{
				m_IsKeyboardEntry++;
			}

			if ((ch=='`') || (ch=='#')) //grave accent or hash sign (inserting simple fraction)
			{
				//check if the cursor is in the empty denominator
				int paternal_pos=0;
				if ((m_pPaternalExpression) && (this->m_NumElements==1) && (this->m_pElementList->Type==0))
					for (paternal_pos=0;paternal_pos<m_pPaternalExpression->m_NumElements;paternal_pos++)
						if ((m_pPaternalExpression->m_pElementList+paternal_pos)->pElementObject==m_pPaternalElement)
							break;
				if (paternal_pos>0)
				if ((m_pPaternalElement) && (m_pPaternalElement->m_Type==4) && (m_pPaternalElement->Data1[0]==0)  && (m_pPaternalElement->Expression2==(CObject*)this))
				{
					//suck-in the object into the numerator
					
					tElementStruct *ts1=m_pPaternalExpression->m_pElementList+paternal_pos-1;
					CExpression *num=(CExpression*)m_pPaternalElement->Expression1;
					if ((ts1->Type!=11) && (ts1->Type!=12))
					{
						num->InsertElement(ts1,0);
						m_pPaternalExpression->DeleteElement(paternal_pos-1);
						
						InsertEmptyElement(0,1,0,fcolor);
						m_KeyboardCursorPos=0;
						m_IsKeyboardEntry=1;
					}
					else 
						InsertEmptyElement(0,1,0,fcolor);
					return 1;
				}
				if ((lastkeystrokes[1]=='`') || (lastkeystrokes[1]=='#'))
				{
					InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
					m_KeyboardCursorPos=0;
					return 1;
				}

				InsertEmptyElement(m_IsKeyboardEntry-1,4,0,fcolor);
				CExpression *nom=((CExpression*)(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression1);
				if ((m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->Type!=2) && ((m_pElementList+m_IsKeyboardEntry-2)->Type!=12) && ((m_pElementList+m_IsKeyboardEntry-2)->Type!=11))
				{
					CExpression *denom=((CExpression*)(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Expression2);
					nom->InsertElement(m_pElementList+m_IsKeyboardEntry-2,0);
					DeleteElement(m_IsKeyboardEntry-2);
					KeyboardRefocus(denom,0);
				}
				else
					KeyboardRefocus(nom,0);
				return 1;
			}


			if (ch!=' ')
			{
				if (m_IsKeyboardEntry>1)
				{
					tElementStruct *ts=m_pElementList+m_IsKeyboardEntry-2;
					if (ts->Type==2)
					{
						char *prev=ts->pElementObject->Data1;
						if ((*prev==(char)0xDE) && (ch==(char)0xAE))
						{
							*prev=(char)0xE2; //this converts double right arrow to |--> arrow after third ALT+right keypress
							ch=-1;
						}
						char pprev=*prev;
						if (pprev==(char)0xAD) pprev=3; //up arrow
						if (pprev==(char)0xAF) pprev=2; //down arrow
						if (pprev==(char)0xAC) pprev=4; //left arrow
						if (pprev==(char)0xAE) pprev=5; //right arrow
						if (pprev==lastkeystrokes[1])
						{
							
							if ((ch==(char)0xAD) || (ch==(char)0xAF) || (ch==(char)0xAC) || (ch==(char)0xAE))
							{
								//if arrow character is entered, we will check for two successive arrows, and change this to one double arrow

								if (*prev==ch) {*prev+=0x30;ch=-1;}						
							}
							
							if (ch==(char)'/') 
							{
								//if '/' character is entered, we will check for two successive '/'
								//two successive slashes, //, produce true fraction
								if (*prev=='/')
								{
									DeleteElement(m_IsKeyboardEntry-2);
									InsertEmptyElement(m_IsKeyboardEntry-2,4,0,fcolor);
									//ts=(m_pElementList+m_IsKeyboardEntry-2);
									KeyboardRefocus((CExpression*)ts->pElementObject->Expression1,0);
									/*m_IsKeyboardEntry=0;
									((CExpression*)(ts->pElementObject->Expression1))->InsertEmptyElement(0,1,0,fcolor);
									KeyboardEntryObject=ts->pElementObject->Expression1;
									((CExpression*)(ts->pElementObject->Expression1))->m_IsKeyboardEntry=1;
									((CExpression*)(ts->pElementObject->Expression1))->m_KeyboardCursorPos=0;*/
									return 1;							
								}
							}
							
							if ((ch=='=') || (ch=='<') || (ch=='>') || (ch=='+') || (ch=='-') || (ch==':') || (ch=='~'))
							{
								//if equals '=' sign, check operators before, if this is '<' or '>' then...						
								if ((ch=='=') && (*prev=='<')) {*prev=(char)0xA3;ch=-1;}    //<=
								else if ((ch=='=') && (*prev=='>'))	{*prev=(char)0xB3;ch=-1;} //>=
								else if ((ch=='<') && (*prev=='<'))	{*prev=(char)0x01;ch=-1;} // <<
								else if ((ch=='>') && (*prev=='>'))	{*prev=(char)0x02;ch=-1;} // >>
								else if ((ch=='>') && (*prev=='-'))	{*prev=(char)0xAE;ch=-1;} // ->
								else if ((ch=='>') && (*prev=='+'))	{*prev=(char)0xE2;ch=-1;} // |->
								else if ((ch=='+') && (*prev=='-'))	{*prev=(char)0xB2;ch=-1;} // -+
								else if ((ch=='-') && (*prev=='+'))	{*prev=(char)0xB1;ch=-1;} // +-
								else if ((ch=='~') && (*prev=='~'))	{*prev=(char)0xBB;ch=-1;} // ~~ (approx equal)
								else if ((ch=='=') && (*prev==':'))	{*prev=(char)'1';ch=-1;} // :=	
								else if ((ch==':') && (*prev=='='))	{*prev=(char)'2';ch=-1;} // =:		
								else if ((ch=='>') && (*prev=='='))	{*prev=(char)0xDE;ch=-1;} // =>		
								else if ((ch=='=') && (*prev=='/'))	{*prev=(char)0xB9;ch=-1;} // not equal
								else if ((ch=='=') && (*prev=='~')) {*prev=(char)0x40;ch=-1;} //approx equal 
								else if ((ch=='=') && (*prev=='-')) {*prev=(char)0xBA;ch=-1;} //(triple equal) - equivalent

								else if ((ch=='>') && (*prev=='<'))
								{
									DeleteElement(m_IsKeyboardEntry-1);
									DeleteElement(m_IsKeyboardEntry-2);
									InsertEmptyElement(m_IsKeyboardEntry-2,5,'<',fcolor);
									CExpression *a=(CExpression*)(this->m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Expression1;
									KeyboardRefocus(a,0);
									return 1;
								}

							}
						}
					}
				}
				
				if (unicode==(unsigned char)0xD7) ch=(char)0xB4; //cross multiplying sign
				if (unicode==(unsigned char)0xF7) ch=(char)0xB8; //division sign (dash with two dots)
				if (unicode==(unsigned char)0xAC) ch=(char)0xD8; //not sign

				if (ch=='*') 
				{
					if ((lastkeystrokes[1]=='*') && (lastkeystrokes[2]=='*') && (m_IsKeyboardEntry>2))
					{
						//triple dot ***
						DeleteElement(m_IsKeyboardEntry-2);
						DeleteElement(m_IsKeyboardEntry-3);
						m_IsKeyboardEntry-=1;
						InsertEmptyElement(m_IsKeyboardEntry-2,1,6,fcolor);
						InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
						m_KeyboardCursorPos=0;
						return 1;
					}

					short t=0;
					if (m_IsKeyboardEntry>1) t=(m_pElementList+m_IsKeyboardEntry-2)->Type;
					if (((m_IsKeyboardEntry==1) && (m_NumElements==1)) ||
						((IsText) && 
						((m_IsKeyboardEntry==1) || (t==12) || (t==11) || 
						((t==2) && (m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]==9)) ||
						((t==2) && (m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]==(char)0xFF)) )))
					{
						if ((!m_pPaternalElement) || (m_pPaternalElement->m_Type!=3))
						{
							if (!IsText)
							{
								IsText=1;
								if (m_pElementList->Type==1) m_pElementList->pElementObject->m_Text=1;
								m_ModeDefinedAt=1;
								m_StartAsText=1;
								m_ModeDefinedAt|=0x4000;
								InsertEmptyElement(m_IsKeyboardEntry-1,2,(char)0xB7,fcolor);
								m_IsKeyboardEntry++;
								InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
								(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=1;
								m_KeyboardCursorPos=0;
								goto show_keymode_label;
								
							}
							ch=(char)0xB7; //the big dot (for bulleted lists)
						}
					}
					else
						ch=(char)0xD7; //the star character will be changed to small dot (multiply)
				}

				//if (ch=='#') ch=(char)0xB9; //non-equal
				if (ch!=-1)
				{
					if ((ch==',') && (m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==1) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->m_Text))
					{
						//special handling for coma - convert it to text if it is typed just after a text
						InsertEmptyElement(m_IsKeyboardEntry-1,1,',',fcolor);
						m_IsKeyboardEntry++;

					}
					else
					{
						InsertEmptyElement(m_IsKeyboardEntry-1,2,ch,fcolor);
						m_IsKeyboardEntry++;
					}
				}
			}


			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
			(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;

			m_KeyboardCursorPos=0;
			int is_section_start=0;
			int is_section_end=0;
			int is_element_empty=0;
			tElementStruct *te=m_pElementList+m_IsKeyboardEntry-1;
			
			if (m_pPaternalExpression==NULL)
			{
				if ((m_IsKeyboardEntry==1) || ((m_IsKeyboardEntry>1) &&
					(((te-1)->Type==11) || ((te-1)->Type==12))))  is_section_start=1;
				if ((m_IsKeyboardEntry==m_NumElements) || ((m_IsKeyboardEntry<m_NumElements) &&
					(((te+1)->Type==11) || ((te+1)->Type==12)))) is_section_end=1;
				if ((te->pElementObject) && (te->pElementObject->Data1[0]==0)) is_element_empty=1;
			}

			if (is_external)
			if (ch==' ')
			if (((IsALTDown) && (!KeyboardExponentMode)) ||  //Alt+Space
				((is_element_empty) && (is_section_start) && (is_section_end) && (IsSpacebarOnlyHit==10)))
			{
toggle_keymode:
				if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Object) && (KeyboardEntryBaseObject->Type==1))
				{
					(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText=(IsText)?0:1;
					if (m_NumElements==1) m_StartAsText=IsText;

					m_ModeDefinedAt=(m_IsKeyboardEntry&0x3FFF);
					if (IsText) m_ModeDefinedAt|=0x4000;
				}

show_keymode_label:
				
				if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Object) && (KeyboardEntryBaseObject->Type==1))
				{
					CExpression *parent=(CExpression*)KeyboardEntryBaseObject->Object;
					int x,y;
					short l,a,b;
					parent->CalculateSize(DC,ViewZoom,&l,&a,&b);
					parent->GetKeyboardCursorPos(&x,&y);
					int ycord=(KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Below-ViewY+2)*ViewZoom/100+1;
					
					CDC *mdc=Toolbox->GetDC();
					Toolbox->PaintTextcontrolbox(mdc);
					Toolbox->ReleaseDC(mdc);
					DisplayShortText(
						(IsText)?"T":"S",
						(KeyboardEntryBaseObject->absolute_X-ViewX)*ViewZoom/100+x-8,
						ycord,7999,(IsText)?3:2);
					
				}
			}
			return 1;
		}

		if ((!IsText) && (ch=='.') && (lastkeystrokes[1]!=' ') && (m_KeyboardCursorPos==0) && (theElement->pElementObject->Data1[0]==0))
		{
			//IndexModeWasUsed=0;
			if ((m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==1) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]>='A'))
			{
				DeleteElement(m_IsKeyboardEntry-1);
				m_IsKeyboardEntry--;
				theElement=this->m_pElementList+m_IsKeyboardEntry-1;
				m_KeyboardCursorPos=(int)(strlen(theElement->pElementObject->Data1));
			}
		}
		if ((!IsText) && (ch=='.') && (lastkeystrokes[1]!=' ') && (theElement->pElementObject->Data1[m_KeyboardCursorPos]==0) && 
			(m_KeyboardCursorPos>0) && ((theElement->pElementObject->m_VMods)!=0x10) &&
			((theElement->pElementObject->Data1[0]>='A') || ((theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='.') &&
			(this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==1))))
		{ 
		
			CExpression *base_exp=this;
			tElementStruct *base_elm=theElement;

			if (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='.') {m_KeyboardCursorPos--;theElement->pElementObject->Data1[m_KeyboardCursorPos]=0;}

			//first check if we are in variable index
			if ((this->m_pPaternalElement) && 
				(m_pPaternalElement->m_Type==1) && 
				(m_pPaternalElement->Expression1==(CObject*)this))
			{
				base_exp=this->m_pPaternalExpression;
				int ii;
				for (ii=0;ii<base_exp->m_NumElements;ii++)
					if ((base_exp->m_pElementList+ii)->pElementObject==this->m_pPaternalElement) break;
				if (ii==base_exp->m_NumElements) return 1;
				base_elm=base_exp->m_pElementList+ii;
			}
		
			{
				
				int start_ord;
				int end_ord;
				
				int func_type=0;
				char funcname[32];
				unsigned char funcfont[32];
				CExpression *index=NULL;
				funcname[0]=0;

			
				start_ord=(int)(base_elm-base_exp->m_pElementList);
				end_ord=start_ord;
				strcpy(funcname,base_elm->pElementObject->Data1);
				for (int ii=0;ii<(int)strlen(funcname);ii++)
				{
					unsigned char ff=base_elm->pElementObject->Data2[ii];
					if ((ff&0xE0)!=0x60) ff=0x20; else ff=ff&0xFC;
					funcfont[ii]=ff;
				}
				index=(CExpression*)base_elm->pElementObject->Expression1;

				if (IsSimpleVariableMode)
				{
					int ok=0;
					//examine keys last used
					int xx=1;
					if (lastkeystrokes[2]=='_') xx=3;
					if (lastkeystrokes[3]=='_') xx=4;
					if (lastkeystrokes[4]=='_') xx=5;

					int pos1=0;
					int pos2=0;

					while (FunctionListString[pos1])
					{
						if (FunctionListString[pos1]==',')
						{
							char tmpbuff[20];
							int jj=0;
							for (int ii=pos1-1;ii>=pos2;ii--)
								tmpbuff[jj++]=FunctionListString[ii];

							if (strncmp(lastkeystrokes+xx,tmpbuff,jj)==0) 
							{
								start_ord=lastkeystrokeselm[jj+xx-2];
								memcpy(funcname,&FunctionListString[pos2],jj);
								funcname[jj]=0;
								memset(funcfont,0x20,jj);
								ok=1;
								
								break;
							}
							pos2=pos1+1;
						}
						pos1++;
					}
				}

				if (strcmp(funcname,"lim")==0) func_type=2;
				if (strcmp(funcname,"par")==0) {funcname[0]=(char)0xB6;funcname[1]=0;funcfont[0]=0x60;}
				if ((strcmp(funcname,"e")==0) && ((funcfont[0]&0xE0)!=0x60) && (index==NULL)) func_type=1;
				if ((strcmp(funcname,"i")==0) && ((funcfont[0]&0xE0)!=0x60) && (index==NULL)) func_type=4;
				if ((strcmp(funcname,"j")==0) && ((funcfont[0]&0xE0)!=0x60) && (index==NULL)) func_type=4;

				if ((strcmp(funcname,"sum")==0) && (index==NULL)) {func_type=3;}
				if ((strcmp(funcname,"int")==0) && (index==NULL)) {func_type=3;}
				
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("convert to function",20315); //text checked in the backspace handling

				if (func_type==1) //an exponent function e^
					base_exp->InsertEmptyElement(start_ord,3,4,fcolor);
				else if (func_type==2) //the limes 
					base_exp->InsertEmptyElement(start_ord,6,25,fcolor);
				else if (func_type==3) //sumation sign (sigma)
					base_exp->InsertEmptyElement(start_ord,7,funcname[0],fcolor);
				else if (func_type==4) //imaginary units i and j
				{
					if ((base_exp->m_pElementList+start_ord)->pElementObject->Data2[0]==0x22)
					{
						base_exp->InsertEmptyElement(start_ord,6,0,fcolor);
						func_type=0;
						
					}
					else
					{
						(base_exp->m_pElementList+start_ord)->pElementObject->Data2[0]=0x22; //seriph, italc
						end_ord--;
						//lastkeystrokes[0]=' ';
					}
				}
				else
				{
					base_exp->InsertEmptyElement(start_ord,6,0,fcolor);
				}
				CElement *elm=(base_exp->m_pElementList+start_ord)->pElementObject;

				if (func_type==0)
				{
					strcpy(elm->Data1,funcname);
					memcpy(elm->Data2,funcfont,strlen(funcname));
					char add_parentheses=0;
					if (funcname[1]==0) add_parentheses=1;
					if (funcname[1]=='\'') add_parentheses=1;
					if ((funcname[1]==0) && (funcname[0]=='d') && ((elm->Data2[0]&0xE0)!=0x60)) add_parentheses=0;
					if ((funcname[1]==0) && (funcname[0]==(char)0xB6) && ((elm->Data2[0]&0xE0)==0x60)) add_parentheses=0;
					if (add_parentheses)
					{
						if (elm->Expression1)
						{
							//automatically adds parentheses to single-letter functions
							((CExpression*)elm->Expression1)->m_ParenthesesFlags|=0x01;
							((CExpression*)elm->Expression1)->m_ParentheseShape='(';
							//((CExpression*)elm->Expression1)->m_ParentheseHeightFactor=1;//DefaultParentheseType;
						}
					}
				}
				if (index)
				{
					if (elm->Expression2==NULL)
						elm->Expression2=(CObject*)new CExpression(elm,base_exp,elm->FontSizeForType(2));
					((CExpression*)elm->Expression2)->CopyExpression(index,0);
				}

				if (IsSimpleVariableMode)
				if (base_exp->m_NumElements>start_ord+1)
				if (((base_exp->m_pElementList+start_ord+1)->Type==6) && (base_exp->m_pElementList+start_ord+1)->pElementObject->IsDifferential(1))
				{
					//handling differentials and then functions, for example: d<dot>sin<dot>
					//without this handling it would not be recognized that this is a differential of sinus functon
					CExpression *e=(CExpression*)(base_exp->m_pElementList+start_ord+1)->pElementObject->Expression1;
					if (e)
					{
						e->DeleteElement(0);
						e->InsertElement(base_exp->m_pElementList+start_ord,0);
						elm=e->m_pElementList->pElementObject;
						base_exp->DeleteElement(start_ord);
						end_ord--;
					}
				}

				{

					if (func_type==4)
					{
						//KeyboardRefocus(this,m_IsKeyboardEntry);
					}
					else if ((func_type==0) || (func_type>2))
						KeyboardRefocus((CExpression*)elm->Expression1,0);
					else
						KeyboardRefocus((CExpression*)elm->Expression2,0);
				}

				for (int i=start_ord;i<=end_ord;i++)
				{
					base_exp->DeleteElement(start_ord+1);
				}
				return 1;
			}
			
			
		}
		


		//if ((GetKeyState(VK_SPACE)&0xFFFE)==0)
		if (ch_number) //entering numbers
		{
			if (FixFontForNumbers)
			{
				if (m_KeyboardCursorPos<=0) 
					nFlags=0;
				else
					nFlags=theElement->pElementObject->Data2[m_KeyboardCursorPos-1]&0x07;
			}

			if ((!IsText) && (ch=='.') && (m_KeyboardCursorPos>=2) &&
				(theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='.') &&
				(theElement->pElementObject->Data1[m_KeyboardCursorPos-2]=='.'))
			{
				//handling triple dot entry
				theElement=KeyboardSplitVariable();
				
				if (m_KeyboardCursorPos<=2) 
					DeleteElement(--m_IsKeyboardEntry);
				else
					theElement->pElementObject->Data1[m_KeyboardCursorPos-2]=0;
				m_KeyboardCursorPos=0;
				InsertEmptyElement(m_IsKeyboardEntry++,2,(char)0xBC,fcolor);
				(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data2[0]=0x60;
				InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
				return 1;				
			}

			if ((!IsText) && (ch!=',') && (ch!='.') && (m_KeyboardCursorPos==1) && (theElement->pElementObject->Data1[0]=='.'))
			{
				//if the number begins with dot '.', then add leading zero
				memmove(theElement->pElementObject->Data1+1,theElement->pElementObject->Data1,23);
				memmove(theElement->pElementObject->Data2+1,theElement->pElementObject->Data2,23);
				theElement->pElementObject->Data1[0]='0';
				theElement->pElementObject->Data2[0]=(nFlags)&0xFF;
				theElement->pElementObject->m_VMods=(nFlags>>16)&0xFF;
				m_KeyboardCursorPos++;
				
				if ((nChar!=',') || ((UseCommaAsDecimal) && (lastkeystrokes[1]!=' ')))
					goto keyboardkeyhit_add_dot;
			}
			if ((!IsText) && ((nChar!=',') || ((UseCommaAsDecimal) && (lastkeystrokes[1]!=' '))) && (m_KeyboardCursorPos==0))
			{
				//new number is started, check if the previous element is also a number, then 
				//add a multiplying dot in between
keyboardkeyhit_add_dot:
				//the command line of the drawing box is excluded
				if ((KeyboardEntryBaseObject==NULL) || (KeyboardEntryBaseObject->Type!=2) || 
					(((CDrawing*)(KeyboardEntryBaseObject->Object))->IsSpecialDrawing!=50)) 
				if (m_IsKeyboardEntry>1)
				{
					theElement=m_pElementList+m_IsKeyboardEntry-2;
					int is_number=1;
					if (theElement->Type==2) is_number=0; 
					if ((theElement->Type==11) || (theElement->Type==12)) is_number=0;
					if ((theElement->Type==1) && (theElement->pElementObject))
					{
						int ln=(int)strlen(theElement->pElementObject->Data1);
						char ch=theElement->pElementObject->Data1[ln-1];
						if (((theElement->pElementObject->Data2[ln-1])&0xE3)==0xE3) is_number=0; //triple-dots (special characters)
						if (ch<'0') is_number=0; //care is taken that 10\deg 20\' 30'' can be written without dots inside
						if ((ch>'9') && (ch<'@')) is_number=0;
						if (theElement->pElementObject->m_Text) is_number=0;
					}
					if (is_number)
					{					
						if (((lastkeystrokes[1]==' ') || (m_KeyboardCursorPos>0) || (theElement->pElementObject->Expression1)) && (nChar!='.'))
						{
							//adding a multiplying dot between previous number and this number
							DotAutomaticallyAdded=1;
							InsertEmptyElement(m_IsKeyboardEntry-1,2,(char)0xD7,fcolor); //multiplying dot
							m_IsKeyboardEntry++;
						}
						else if ((lastkeystrokes[1]!=' ') && (theElement->Type==1))
						{
							//splicing this number to the previous number
							if (strlen((theElement+1)->pElementObject->Data1)==0)
								DeleteElement(m_IsKeyboardEntry-1);

							m_IsKeyboardEntry--;
							m_KeyboardCursorPos=(short)strlen(theElement->pElementObject->Data1);
						}
						
					}
					theElement=m_pElementList+m_IsKeyboardEntry-1;
				}
			}
		}

		

		
		if (ch==3) //up cursor key
		{

			m_ModeDefinedAt=0;

			// first find X and Y position of the cursor 
			int at_beginning=0;
			if (((m_IsKeyboardEntry==1) || ((m_pElementList+m_IsKeyboardEntry-2)->Type==12)) &&
				(m_KeyboardCursorPos==0) &&
				(this->m_pPaternalExpression==NULL)) at_beginning=1;

			CExpression *parent=this;
			while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;

			int X,Y;
			parent->GetKeyboardCursorPos(&X,&Y);

			//then find our best candidate
			short tmp_entry=m_IsKeyboardEntry;
			RemoveEmptyVariable(DC,theElement,1);

			if ((m_IsKeyboardEntry>1) && ((m_pElementList+m_IsKeyboardEntry-2)->Type==3) && (m_KeyboardCursorPos==0))
			{
				//special handling if there is an exponent before
				CElement *el=(m_pElementList+m_IsKeyboardEntry-2)->pElementObject;
				if ((el) && (el->Expression1) && (el->Expression2))
				{
					CExpression *base=(CExpression*)el->Expression1;
					if ((base->m_NumElements>1) || (base->m_pElementList->Type!=6))
					{
						CExpression *exp=(CExpression*)el->Expression2;
						KeyboardRefocus(exp,exp->m_NumElements);
						return 1;
					}
				}
			}

			if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==7) && (this->m_pPaternalElement->Expression1==(CObject*)this))
			{
				//special handling for sigma/product/integral with limits defined - force limit editing
				int a=1,b=0;
				if (this->m_pPaternalElement->Data1[0]=='/') // integration limit bar (the function is on the left side)
				{
					a=this->m_NumElements;b=(int)strlen((this->m_pElementList+a-1)->pElementObject->Data1);
				}
				if ((m_IsKeyboardEntry==a) && (m_KeyboardCursorPos==b) && (this->m_pPaternalElement->Expression2))
				{
					KeyboardRefocus((CExpression*)this->m_pPaternalElement->Expression2,((CExpression*)(this->m_pPaternalElement->Expression2))->m_NumElements);
					return 1;
				}
			}
			if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==3) && (this->m_pPaternalElement->Expression1==(CObject*)this) &&
				(m_IsKeyboardEntry>=m_NumElements))
			{
				//special handling for the exponent - go into exponent
				KeyboardRefocus((CExpression*)this->m_pPaternalElement->Expression2,0);
				return 1;
			}
			
			if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==1) && (this->m_pPaternalElement->Expression1==(CObject*)this) &&
				(m_IsKeyboardEntry==1) && (m_KeyboardCursorPos==0))
			{
				//special handling for variable indexes - moving the cursor at the end of the variable
				//KeyboardRefocus((CExpression*)this->m_pPaternalExpression,this->m_pPaternalElement->GetPaternalPosition());
				KeyboardEntryObject=(CObject*)this->m_pPaternalExpression;
				this->m_pPaternalExpression->m_IsKeyboardEntry=this->m_pPaternalElement->GetPaternalPosition()+1;
				this->m_pPaternalExpression->m_KeyboardCursorPos=(short)strlen(this->m_pPaternalElement->Data1);
				m_IsKeyboardEntry=0;
				return 1;
			}

			if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==8) && (this->m_pPaternalElement->Expression1==(CObject*)this) &&
				(m_IsKeyboardEntry==1) && (m_KeyboardCursorPos==0) && (this->m_pPaternalElement->Expression2))
			{
				//special handling for root
				KeyboardRefocus((CExpression*)this->m_pPaternalElement->Expression2,((CExpression*)this->m_pPaternalElement->Expression2)->m_NumElements);
				return 1;
			}
			m_IsKeyboardEntry=0;
		
			if (parent->KeyboardStartAt(X,Y-(m_FontSize/20)*ViewZoom/100,-1,1)==0)
			{
				//we found no new keyboard position in thcurrente same equation
				//try other equations (just above the  cursor position)

				int candidate=-1;
				int candidate_y=-100000;
				int absX=X*100/ViewZoom+KeyboardEntryBaseObject->absolute_X;
				int dist=2*GRID;
				if ((m_NumElements==0) || (m_pElementList->Type==0)) dist=3*GRID/2;

				tDocumentStruct *ds=TheDocument;
				for (int i=0;i<NumDocumentElements;i++,ds++)
					if ((ds->Type==1) && (ds->Object) && 
						(ds->absolute_X+ds->Length>absX) &&
						(ds->absolute_X<absX) &&
						(ds->absolute_Y<KeyboardEntryBaseObject->absolute_Y) &&
						(ds->absolute_Y>candidate_y) && 
						(ds->absolute_Y+ds->Below+dist>KeyboardEntryBaseObject->absolute_Y-KeyboardEntryBaseObject->Above))
					{
						candidate_y=ds->absolute_Y;
						candidate=i;
					}
				if (candidate>=0)
				{
					//found a new equation - try to set the cursor to it
					CExpression *c=(CExpression*)TheDocument[candidate].Object;
					if (c->KeyboardStartAt(X-(TheDocument[candidate].absolute_X-KeyboardEntryBaseObject->absolute_X)*ViewZoom/100,
						(TheDocument[candidate].Below)*ViewZoom/100,-1,2))
					{
						//done - change keyboard focus and return
						short l,a,b;
						parent->CalculateSize(DC,ViewZoom,&l,&a,&b);

						m_IsKeyboardEntry=0;
						KeyboardEntryBaseObject=TheDocument+candidate;
						return 2; //this forces repaint
					}
				}

				//we are going to create new object
				{
					int make_text=0;
					if ((this->m_NumElements==1) && (this->m_pElementList->Type==0))
					{
						make_text=IsText; //if we are 'moving' and empty box, then we will inherit its math/text type
					}
					else
					{
						if (prevEmptyBoxType!=-1) make_text=prevEmptyBoxType;
					}
					prevEmptyBoxType=make_text;

					int y_pos=((KeyboardEntryBaseObject->absolute_Y-KeyboardEntryBaseObject->Above-GRID)/GRID)*GRID;
					int x_pos=absX-DefaultFontSize/25;
					if (at_beginning) x_pos=KeyboardEntryBaseObject->absolute_X;
					if (y_pos<10)
					{
						m_IsKeyboardEntry=tmp_entry;
						InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
						m_KeyboardCursorPos=0;
						return 1;
					}

					AddDocumentObject(1,x_pos,y_pos);
					TheDocument[NumDocumentElements-1].Object=(CObject*)new CExpression(NULL,NULL,DefaultFontSize);
					((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_StartAsText=make_text;
					if (make_text) ((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_Alignment=1;
					KeyboardEntryBaseObject=TheDocument+NumDocumentElements-1;
					KeyboardRefocus((CExpression*)TheDocument[NumDocumentElements-1].Object,0);
					if (IsHighQualityRendering) pMainView->RepaintTheView();
					return 2;
				}
			}

			return 1;
		}

		if (ch==2) //down cursor key
		{
			//we will move the cursor down, either inside the same euquation object
			//or we will step otside (new empty object will be created) or step into 
			//another object
			m_ModeDefinedAt=0;

			//first check if we are at the very beginning of an object (not to change x position when stepping outside)
			int at_beginning=0;
			if (((m_IsKeyboardEntry==1) || ((m_pElementList+m_IsKeyboardEntry-2)->Type==12)) &&
				(m_KeyboardCursorPos==0) &&
				(this->m_pPaternalExpression==NULL)) at_beginning=1;

			CExpression *parent=this;
			while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;

			// first find X and Y position of the cursor 
			int X,Y;
			parent->GetKeyboardCursorPos(&X,&Y);

			RemoveEmptyVariable(DC,theElement,1);
			if ((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==7) && (this->m_pPaternalElement->Expression1==(CObject*)this))
			{
				//special handling for sigma/product/integral with limits defined - force limit editing
				int a=1,b=0;
				if (this->m_pPaternalElement->Data1[0]=='/') // integration limit bar (the function is on the left side)
				{
					a=this->m_NumElements;b=(int)strlen((this->m_pElementList+a-1)->pElementObject->Data1);
				}
				if ((m_IsKeyboardEntry==a) && (m_KeyboardCursorPos==b) && (this->m_pPaternalElement->Expression3))
				{
					KeyboardRefocus((CExpression*)this->m_pPaternalElement->Expression3,((CExpression*)(this->m_pPaternalElement->Expression3))->m_NumElements);
					return 1;
				}
			}			
			m_IsKeyboardEntry=0;

			//now check if we can move cursor inside the same equation
			if (parent->KeyboardStartAt(X,Y+1+m_MarginY/8,1,1)==0)
			if (KeyboardEntryBaseObject)
			{
				//we found no new keyboard position in the same equation
				//try other equations (just below the current cursor position)

				int candidate=-1;
				int candidate_y=0x7FFFFFFF;
				int absX=X*100/ViewZoom+KeyboardEntryBaseObject->absolute_X;
				int dist=2*GRID;
				if ((m_NumElements==0) || (m_pElementList->Type==0)) dist=3*GRID/2;

				tDocumentStruct *ds=TheDocument;
				for (int i=0;i<NumDocumentElements;i++,ds++)
					if ((ds->Type==1) && (ds->Object) && 
						(ds->absolute_X+ds->Length>absX) &&
						(ds->absolute_X<absX) &&
						(ds->absolute_Y>KeyboardEntryBaseObject->absolute_Y) &&
						(ds->absolute_Y<candidate_y) &&
						(ds->absolute_Y-ds->Above-dist<KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Below))
					{
						candidate_y=ds->absolute_Y;
						candidate=i;
					}
				if (candidate>=0)
				{
					//found a new equation - try to set the cursor to it
					CExpression *c;
					c=(CExpression*)TheDocument[candidate].Object;
					if (c->KeyboardStartAt(X-(TheDocument[candidate].absolute_X-KeyboardEntryBaseObject->absolute_X)*ViewZoom/100,
						(-TheDocument[candidate].Above)*ViewZoom/100,1,2))
					{
						//if (this->m_IsText) AdjustMatrix();

						//done - change keyboard focus and return
						short l,a,b;
						parent->CalculateSize(DC,ViewZoom,&l,&a,&b);

						m_IsKeyboardEntry=0;
						KeyboardEntryBaseObject=TheDocument+candidate;
						return 2; //this forces repaint
					}
				}

				//we are going to create new object
				{
					int make_text=0;
					if ((this->m_NumElements==1) && (this->m_pElementList->Type==0))
					{
						make_text=IsText; //if we are 'moving' and empty box, then we will inherit its math/text type
					}
						else
					{
						if (prevEmptyBoxType!=-1) make_text=prevEmptyBoxType;
					}
					prevEmptyBoxType=make_text;
					

					int y_pos=((KeyboardEntryBaseObject->absolute_Y+KeyboardEntryBaseObject->Below+3*GRID/2)/GRID)*GRID;
					int x_pos=absX-DefaultFontSize/25;
					if (at_beginning) x_pos=KeyboardEntryBaseObject->absolute_X;

					AddDocumentObject(1,x_pos,y_pos);
					TheDocument[NumDocumentElements-1].Object=(CObject*)new CExpression(NULL,NULL,DefaultFontSize);
					((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_StartAsText=make_text;
					if (make_text) ((CExpression*)(TheDocument[NumDocumentElements-1].Object))->m_Alignment=1;
					KeyboardEntryBaseObject=TheDocument+NumDocumentElements-1;
					KeyboardRefocus((CExpression*)TheDocument[NumDocumentElements-1].Object,0);
					if (IsHighQualityRendering) pMainView->RepaintTheView();
					return 2;
				}
			}

			return 1;
		}



		//determining wether double-quotes should be handled as exponent-insert (always in math) or as a quotes (in text, sometimes)
		int use_quotes_as_exponent=0;
		if (InhibitAllKeyHandling==0)
		if (ch=='\"')
		{
			if (IsText==0) use_quotes_as_exponent=1;
			else if (theElement->Type==1)
			{
				if ((m_KeyboardCursorPos>0) && (m_KeyboardCursorPos<2))  //only single-character variables 
				if (theElement->pElementObject->Data1[0]==lastkeystrokes[1])
				{
					use_quotes_as_exponent=1;
					

					int num_opens=0; //check the difference between opened and closed doble-quote signs
					for (int ii=m_IsKeyboardEntry-1;ii>=0;ii--)
					{
						tElementStruct *ts=m_pElementList+ii;
						if ((ts->Type==11) || (ts->Type==12)) break;
						if ((ts->Type==1) && (ts->pElementObject->Data1[0]))
						{
							for (int j=0;j<(int)strlen(ts->pElementObject->Data1);j++)
								if (ts->pElementObject->Data1[j]=='\"') num_opens++;
						}
					}
					if (num_opens&0x01) use_quotes_as_exponent=0;
				}
			}
		}

		if (InhibitAllKeyHandling==0)
		if ((ch=='^') || (use_quotes_as_exponent)) //kappa key (converting to power)
		{
			//the powering is only possible if 
			// - there are suitable symbols (non-operator) just before cursor or 
			// - there is a single operator and then the suitable symbol
			// - we are at the very beginning of the function argument (or there is only one single operator before)
			if ((m_KeyboardCursorPos) || 
				((m_IsKeyboardEntry>1) && ((theElement-1)->Type!=2) && ((theElement-1)->pElementObject)) ||
				((m_IsKeyboardEntry>2) && ((theElement-1)->Type==2) && ((theElement-2)->Type!=2) && ((theElement-2)->pElementObject)) ||
				((this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==6) && (this->m_pPaternalElement->Expression1==(CObject*)this) && ((m_IsKeyboardEntry==1) || ((m_IsKeyboardEntry==2) && ((theElement-1)->Type==2)))) )
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("exponent entry",20310);
			else 
				return 1;

			theElement=KeyboardSplitVariable();
			if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;
			if (IsEditedVariableEmpty)
			{
				DeleteElement(m_IsKeyboardEntry-1);
				m_IsKeyboardEntry--;
			}

			CExpression *Baseline=this;
			int Basepos=m_IsKeyboardEntry-1;

			if ((m_IsKeyboardEntry==0) || (m_IsKeyboardEntry==1) && (m_pElementList->Type==2))
			{
				//we are in function argument - powering the function (move pointers outside the function)
				Basepos=this->m_pPaternalElement->GetPaternalPosition();
				Baseline=this->m_pPaternalExpression;

				//check if we are already inside an exponential element (if so, also move pointers outside it)
				if ((Baseline->m_NumElements==1) && (Baseline->m_pPaternalElement) && (Baseline->m_pPaternalElement->m_Type==3))
				{
					Basepos=Baseline->m_pPaternalElement->GetPaternalPosition();
					Baseline=Baseline->m_pPaternalExpression;
				}
				if ((m_IsKeyboardEntry==1) && (m_pElementList->Type==2))
				{
					Basepos++;
					Baseline->InsertElement(m_pElementList,Basepos);
					this->DeleteElement(0);
				}
			}

			if ((IsALTDown) && ((Baseline->m_pElementList+Basepos)->pElementObject) && ((Baseline->m_pElementList+Basepos)->Type==3))
			{
				//we are only adding to the existing exponent
				CExpression *exp=(CExpression*)((Baseline->m_pElementList+Basepos)->pElementObject->Expression2);
				KeyboardRefocus(exp,exp->m_NumElements);
				return 1;
			}

			int move_operator=0;
			if ((Baseline->m_pElementList+Basepos)->Type==2) {Basepos--;move_operator=1;} //the operator will be moved into the exponent

			Baseline->InsertEmptyElement(Basepos,3,0,fcolor); //insert power element
			tElementStruct *theElement2=Baseline->m_pElementList+Basepos; //power element

			theElement=Baseline->m_pElementList+Basepos+1; //non-operator element
			if (theElement2->pElementObject->Expression1)
			{
				if (theElement->pElementObject) theElement->pElementObject->m_Text=0;
				((CExpression*)(theElement2->pElementObject->Expression1))->InsertElement(theElement,0);
				if (move_operator)
					((CExpression*)(theElement2->pElementObject->Expression2))->InsertElement(theElement+1,0); //copy the operator into exponent

				KeyboardRefocus((CExpression*)theElement2->pElementObject->Expression2,move_operator);
				Baseline->DeleteElement(Basepos+1);
				if (move_operator) Baseline->DeleteElement(Basepos+1); //delete the operator

				return 1;
			}

			//we do nothing
			return 1;
		}


		if (InhibitAllKeyHandling==0)
		if (ch=='_') //underline key (editing index of an variable, function or parenthese)
		{
			if (((m_IsKeyboardEntry==1) || ((m_IsKeyboardEntry==2) && (this->m_pElementList->Type==2))) && 
				(m_KeyboardCursorPos==0) &&
				(this->m_pPaternalElement) && (this->m_pPaternalElement->Expression1==(CObject*)this) &&
				((this->m_pPaternalElement->m_Type==6) || (this->m_pPaternalElement->m_Type==7)) )
			{

				// the underscore was hit while in function argument 
				// (either as the first character, or second character just after an operator)
				if (m_IsKeyboardEntry==2)
					((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index entry",20311);

				
				if (IsEditedVariableEmpty) DeleteElement(m_IsKeyboardEntry-1);

				CElement *elm=this->m_pPaternalElement;
				if (elm->Expression2==NULL)
				{
					elm->Expression2=(CObject*)new CExpression(elm,(CExpression*)elm->m_pPaternalExpression,elm->FontSizeForType(2));

					//((CExpression*)(elm->Expression2))->m_FontSizeHQ=elm->FontSizeForTypeHQ(2);
				}
				if ((elm->Expression3==NULL) && (this->m_pPaternalElement->m_Type==7))
				{
					elm->Expression3=(CObject*)new CExpression(elm,(CExpression*)elm->m_pPaternalExpression,elm->FontSizeForType(3));
					//((CExpression*)(elm->Expression3))->m_FontSizeHQ=elm->FontSizeForTypeHQ(3);
				}					
				
				CExpression *ee=(CExpression*)((this->m_pPaternalElement->m_Type==6)?elm->Expression2:elm->Expression3);

				int d=ee->m_NumElements;
				if (ee->m_pElementList->Type==0) d=0;
				if (m_IsKeyboardEntry==2) {ee->InsertElement(this->m_pElementList,d);d++;DeleteElement(0);}
				KeyboardRefocus(ee,d);
				return 1;
			}

			if ((m_KeyboardCursorPos) || 
				((m_IsKeyboardEntry>1) && 
				(((m_pElementList+m_IsKeyboardEntry-2)->Type==1) ||
				(((m_pElementList+m_IsKeyboardEntry-2)->Type==3) && (((CExpression*)(m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Expression1)->m_NumElements==1) && ((((CExpression*)(m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Expression1)->m_pElementList->Type==1) ||(((CExpression*)(m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Expression1)->m_pElementList->Type==6))) ||
				((m_pElementList+m_IsKeyboardEntry-2)->Type==5) ||
				(((m_pElementList+m_IsKeyboardEntry-2)->Type==2) && (m_IsKeyboardEntry>2) && (((m_pElementList+m_IsKeyboardEntry-3)->Type==1) || ((m_pElementList+m_IsKeyboardEntry-3)->Type==6) || ((m_pElementList+m_IsKeyboardEntry-3)->Type==5))) ||
				((m_pElementList+m_IsKeyboardEntry-2)->Type==6)) ))
				((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("index entry",20311);
			else if ((m_NumElements==1) &&
				(m_KeyboardCursorPos==0) && 
				(m_pElementList->pElementObject->Data1[0]==0) &&
				(m_pPaternalElement) && 
				(((CElement*)m_pPaternalElement)->Expression1==(CObject*)this) &&
				((((CElement*)m_pPaternalElement)->m_Type==1) /*|| (((CElement*)m_pPaternalElement)->m_Type==6)*/))
			{
				//the underline is hit at the beginning of index exponent for variable
				//add the underline character to the variable
				CExpression *exp=(CExpression*)m_pPaternalExpression;
				tElementStruct *ts=exp->GetElementStruct(this->m_pPaternalElement);
				if ((ts) && ((ts->Type==1) || (ts->Type==6)))
				{
					int j=(int)(ts-exp->m_pElementList);
					int len=(int)strlen(ts->pElementObject->Data1);
					if (len<22)
					{
						strcat(ts->pElementObject->Data1,"_");
						if (ts->pElementObject->Expression1)
						{
							KeyboardEntryObject=(CObject*)exp;
							exp->m_IsKeyboardEntry=j+1;
							exp->m_KeyboardCursorPos=len+1;
							m_IsKeyboardEntry=0;
							m_KeyboardCursorPos=0;
							delete (CExpression*)ts->pElementObject->Expression1;
							ts->pElementObject->Expression1=0;

							return 2;
						}
					}
				}
			}
			else
				return 1;

			theElement=KeyboardSplitVariable();
			if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;
			if (IsEditedVariableEmpty)
			{
				DeleteElement(m_IsKeyboardEntry-1);
				m_IsKeyboardEntry--;
			}
			if (m_IsKeyboardEntry>0)
			{
				int operator_flag=0;
				theElement=m_pElementList+m_IsKeyboardEntry-1;
				if (theElement->Type==2) {theElement--;operator_flag=1;}

				if (theElement->pElementObject->IsDifferential(1))
				{
					theElement=((CExpression*)theElement->pElementObject->Expression1)->m_pElementList;
				}
				else if (theElement->Type==3) 
				{
					theElement=((CExpression*)theElement->pElementObject->Expression1)->m_pElementList;
					if (theElement->pElementObject->IsDifferential(1))
						theElement=((CExpression*)theElement->pElementObject->Expression1)->m_pElementList;
				}

				short typ=theElement->Type;
				if ((typ==1) || (typ==6) || (typ==5)) //only the variable and function and parentheses can have index
				{
					CExpression *index;
					int expr;
					if (typ==1)
						{expr=1;index=(CExpression*)theElement->pElementObject->Expression1;}
					else
						{expr=2;index=(CExpression*)theElement->pElementObject->Expression2;}

					if (!index)
					{
						//it has no index expression, we will create it now
						index=new CExpression(theElement->pElementObject,this,theElement->pElementObject->FontSizeForType(expr));
						//index->m_FontSizeHQ=theElement->pElementObject->FontSizeForTypeHQ(expr);
						if (typ==1)
							theElement->pElementObject->Expression1=(CObject*)index;
						else
							theElement->pElementObject->Expression2=(CObject*)index;

						index->m_pPaternalElement=theElement->pElementObject;
						index->m_pPaternalExpression=(CExpression*)theElement->pElementObject->m_pPaternalExpression;
					}

					if (operator_flag) 
					{
						index->InsertElement(m_pElementList+m_IsKeyboardEntry-1,index->m_NumElements);
						DeleteElement(m_IsKeyboardEntry-1);
					}
					KeyboardRefocus(index,index->m_NumElements);
					//if ((index->m_NumElements==1) && (index->m_pElementList->Type==1) && (index->m_IsKeyboardEntry==1) && (index->m_KeyboardCursorPos==0))
					//{
					//	index->m_pElementList->pElementObject->m_Text=IsText;
					//}
					return 1;
				}
			}
			
			
			//do nothing
			return 1;
		}
	}


	if (nChar==9) //the TAB key
	{
		if (!IsSHIFTDown)
		{
			//just ad a spacer character (the tab)
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("spacer",20312);
			theElement=KeyboardSplitVariable();
			if (m_KeyboardCursorPos==0) DeleteElement(--m_IsKeyboardEntry);
			m_IsKeyboardEntry+=this->InsertTabSpacer(m_IsKeyboardEntry,0);

			//InsertEmptyElement(m_IsKeyboardEntry++,2,9,fcolor);
			//(m_pElementList+m_IsKeyboardEntry-1)->IsSelected=1; //we are selecting it just to display shortly the character
			m_IsKeyboardEntry++;
			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
			m_KeyboardCursorPos=0;
			return 1;
		}

		//m_IsText=0;

		//add new column to the matrix by inserting the column spacer
		((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("column spacer",20300);
		theElement=KeyboardSplitVariable();
		if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;

		if (IsEditedVariableEmpty)
			if (RemoveEmptyVariable(DC,theElement)) m_IsKeyboardEntry--;

		//check if the cursor is at the first position in the cell
		char is_first=0;
		if ((m_IsKeyboardEntry==0) || ((m_pElementList+m_IsKeyboardEntry-1)->Type==11) || ((m_pElementList+m_IsKeyboardEntry-1)->Type==12))
			is_first=1;

		//determine row and cell where we are
		int crow=0,ccolumn=0;
		int ii;
		for (ii=0;ii<m_IsKeyboardEntry;ii++)
		{
			if ((m_pElementList+ii)->Type==11) ccolumn++;
			if ((m_pElementList+ii)->Type==12) {ccolumn=0;crow++;}
		}
		if (!is_first) ccolumn++;

		InsertMatrixColumn(ccolumn);

		AdjustMatrix();
		m_IsKeyboardEntry=FindMatrixElement(crow,ccolumn,1)+1;
		InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
		m_KeyboardCursorPos=0;
		return 1;
	
	}

	//handles the Enter key for adding new row into matrix or into text box
	if ((nChar==0x0D) && ((IsSHIFTDown)))
	{
		//if ((IsALTDown) && (m_MaxNumColumns==1)) {if (m_IsText==0) {m_Alignment=1;m_IsText=1;}}

		//add new line by inserting the row spacer
		((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("row spacer",20301);
		
		theElement=KeyboardSplitVariable();
		if (m_KeyboardCursorPos==0) IsEditedVariableEmpty=1;
		if (IsEditedVariableEmpty)
			if (RemoveEmptyVariable(DC,theElement)) m_IsKeyboardEntry--;

		//check if the cursor is at the first position in the cell
		char is_first=0;
		if ((m_IsKeyboardEntry==0) || ((m_pElementList+m_IsKeyboardEntry-1)->Type==11) || ((m_pElementList+m_IsKeyboardEntry-1)->Type==12))
			is_first=1;

		//determine row and cell where we are
		int crow=0,ccolumn=0;
		int ii;
		for (ii=0;ii<m_IsKeyboardEntry;ii++)
		{
			if ((m_pElementList+ii)->Type==11) ccolumn++;
			if ((m_pElementList+ii)->Type==12) {ccolumn=0;crow++;}
		}
		if (!is_first) crow++;

		
		char append=0;
		if (crow>=this->m_MaxNumRows) append=1;
		InsertMatrixRow(crow);
		AdjustMatrix();

		if (!is_first)
		{
			int i=m_IsKeyboardEntry;
			int j=FindMatrixElement(crow,ccolumn,1);
			if (j<0) j=0;
			while (((m_pElementList+i)->Type!=12) && (i<m_NumElements))
			{
				tElementStruct *ts=m_pElementList+i;
				if ((ts->Type==12) || (ts->Type==11)) break;
				InsertElement(ts,j);
				DeleteElement(i);
			}
		}

		m_IsKeyboardEntry=FindMatrixElement(crow,(append)?0:ccolumn,1)+1;

		InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
		(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=IsText;
		this->m_ModeDefinedAt=m_IsKeyboardEntry+(IsText<<14);
		m_KeyboardCursorPos=0;
		return 2;
	}


	char veryfirstch=theElement->pElementObject->Data1[0];


	//this part of code handles keyboard depending on what variable mode is selected
	//in 'general variable mode' it allows mult-letter variables, in 'simple variable mode'
	//it allows variables with indexes, in 'very simple variable mode' only single-letter 
	//variables.
	if (!IsText)
	if ((veryfirstch) && (veryfirstch!='\\') && (veryfirstch!='\''))
	if (nChar!='\'')
	if ((this->m_pPaternalElement==NULL) || 
		((this->m_pPaternalElement->m_Type!=1) && 
		((this->m_pPaternalElement->m_Type!=6) || (this->m_pPaternalElement->Expression2!=(CObject*)this)) &&
		((this->m_pPaternalElement->m_Type!=5) || (this->m_pPaternalElement->Expression2!=(CObject*)this))))
	{
		//this is an expression (non-index of variable)
		int previousnumber=0;
		int previouschar=0;
		int thisnumber=0;
		int thischar=0;
		char prevch=0;
		if (m_KeyboardCursorPos>0) 
			prevch=theElement->pElementObject->Data1[m_KeyboardCursorPos-1];
		else
		{
			//if we are entering the very first character
			char fc=theElement->pElementObject->Data1[0];
			if (((fc>='0') && (fc<='9')) || fc=='.') {previousnumber=1;prevch=fc;}
		}
		if ((prevch>='0') && (prevch<='9')) previousnumber=1;
		else if ((prevch=='.') && (m_KeyboardCursorPos>1)) previousnumber=1;
		else if ((prevch==',') && (UseCommaAsDecimal)) previousnumber=1;
		else if ((prevch>='@') && (prevch<='Z')) previouschar=1;
		else if ((prevch>='a') && (prevch<='z')) previouschar=1;
		else if (prevch&0x80) previouschar=1;
		else if ((prevch<0) || (prevch=='\'') || (prevch=='*')) previouschar=1;

		if ((nChar>='0') && (nChar<='9')) thisnumber=1;
		else if (nChar=='.') thisnumber=1;
		else if ((nChar==',') && (UseCommaAsDecimal)) thisnumber=1;
		else if ((nChar>='@') && (nChar<='Z')) thischar=1;
		else if ((nChar>='a') && (nChar<='z')) thischar=1;
		else if (nChar&0x80) thischar=1;
		else if (prevch<0) thischar=1;

		int varmode=IsSimpleVariableMode;
		if (theElement->Type==6) varmode=0; //entering function names
		if (theElement->pElementObject->IsMeasurementUnit()) varmode=0;
		if (theElement->pElementObject->Data1[m_KeyboardCursorPos]!=0) varmode=0;

		if ((m_KeyboardCursorPos==1) && (prevch=='D') && ((theElement->pElementObject->Data2[0]&0xE0)==0x60)) {thischar=0;thisnumber=1;}

		//the folowing line is special handling for drawing box command line - forces general variable mode
		if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==2) &&
			(((CDrawing*)(KeyboardEntryBaseObject->Object))->IsSpecialDrawing==50)) varmode=0;
		
		if (varmode)
		{
			//very simple variable mode
			if (((previousnumber) && (thischar)) || ((previouschar) && (!thisnumber)) || (prevch==0))
			{
				//adds spaces between varibles and/or numbers
				char pshft=IsSHIFTDown;IsSHIFTDown=0;
				char palt=IsALTDown;IsALTDown=0;
				KeyboardKeyHit(DC,zoom,' ',nRptCnt,nFlags,fcolor,0);
				IsSHIFTDown=pshft;
				IsALTDown=palt;
				return ((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,zoom,nChar,nRptCnt,nFlags,fcolor,0); //napravljeno da se omoguci umetanje toèke u 'char * broj' (12.05.2011)
			}
		}
		else 
		{
			//general variable mode
			if ((previousnumber) && (thischar)) 
			{
				//handles adding a space between number and variable
				char pshft=IsSHIFTDown;IsSHIFTDown=0;
				char palt=IsALTDown;IsALTDown=0;
				KeyboardKeyHit(DC,zoom,' ',nRptCnt,nFlags,fcolor,0);
				IsSHIFTDown=pshft;
				IsALTDown=palt;
				theElement=m_pElementList+m_IsKeyboardEntry-1;
			}
		}
	}


	//finally, make space and add character to the variable
	{
		int i;
		unsigned char ch=nChar&0xFF;

		if (InhibitAllKeyHandling==0)
		{
			if (ch=='@') {ch=0xA5;nFlags=0x60;} //the '@' key will be translated to 'infinite' sign
			if (ch=='\'') nFlags=0; //the ' character is always writen with first font

			if ((ch=='%') && (m_KeyboardCursorPos) && (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='%'))
			{
				//changes percentage into permille sign (when percentage typed twice) - to be checked
				theElement->pElementObject->Data2[m_KeyboardCursorPos-1]=(char)0xE3;
				return 1;
			}
		}

		if (m_KeyboardCursorPos==23)
		{
			InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
			m_KeyboardCursorPos=0;
			theElement=m_pElementList+m_IsKeyboardEntry-1;
			if (IsText)
			{
				theElement->pElementObject->m_Text=2;
			}
		}

		//putting the characters into index if the space bar is held down
		/*if ((ch!=' ') && (!IsText) && (IsSimpleVariableMode!=1) && (GetKeyState(VK_SPACE)&0xFFFE))
		if ((m_IsKeyboardEntry>1) || 
			(m_IsKeyboardEntry==1) && (this->m_pPaternalElement) && (this->m_pPaternalElement->m_Type==6) ||
			(m_KeyboardCursorPos>0))
		{
			int nodel=0;
			tElementStruct *prevelm=NULL;
			if ((m_IsKeyboardEntry==1) && (m_KeyboardCursorPos==0))
			{
				for (int ii=0;ii<this->m_pPaternalExpression->m_NumElements;ii++)
					if ((this->m_pPaternalExpression->m_pElementList+ii)->pElementObject==this->m_pPaternalElement)
					{
						prevelm=this->m_pPaternalExpression->m_pElementList+ii;
						break;
					}
			}
			else if (m_KeyboardCursorPos>0)
			{
				prevelm=m_pElementList+m_IsKeyboardEntry-1;
				nodel=1;
			}
			else
				prevelm=m_pElementList+m_IsKeyboardEntry-2;

			if (prevelm)
			if ((prevelm->Type==1) || (prevelm->Type==6) || (prevelm->Type==5))
			{
				CExpression **exp=(CExpression**)&(prevelm->pElementObject->Expression1);
				if (prevelm->Type!=1) exp=(CExpression**)&(prevelm->pElementObject->Expression2);
				if (*exp==NULL)
				{
					*exp=new CExpression(prevelm->pElementObject,this,prevelm->pElementObject->FontSizeForType((prevelm->Type==1)?1:2));
					(*exp)->m_FontSizeHQ=prevelm->pElementObject->FontSizeForTypeHQ((prevelm->Type==1)?1:2);
				}
				CExpression *indexexp=*exp;
				if (indexexp->m_pElementList->Type==0) indexexp->InsertEmptyElement(0,1,0,fcolor);
				CElement *lastone=(indexexp->m_pElementList+indexexp->m_NumElements-1)->pElementObject;
				if ((lastone) && (lastone->m_Type==1))
				{
					int len=(int)strlen(lastone->Data1);
					if (len<23)
					{
					lastone->Data1[len]=ch;
					lastone->Data2[len]=nFlags;

					if ((prevelm->Type==1) && (!nodel))
					{
						//special handling for variables - we need this because otherwise the dot '.'will not convert the variable into function
						DeleteElement(m_IsKeyboardEntry-1);
						m_IsKeyboardEntry--;
						m_KeyboardCursorPos=(int)(strlen(prevelm->pElementObject->Data1));
					}
					return 1;
					}
				}
			}
		}*/

		if ((m_KeyboardCursorPos==0) && (theElement->pElementObject->Data1[0]==0))
		{
			((CMainFrame*)(theApp.m_pMainWnd))->UndoSave("variable entry",20313);
		}

		if (theElement->Type==1)
		{
			char pch=theElement->pElementObject->Data1[0];
			
			if ((ch>='0') &&(ch<='9'))
			if ((m_KeyboardCursorPos==1) && (theElement->pElementObject->Data1[1]==0) && (theElement->pElementObject->m_Text==2) && 
				((theElement->pElementObject->Data1[0]=='.') || ((theElement->pElementObject->Data1[0]==',') && (UseCommaAsDecimal))) &&
				(m_IsKeyboardEntry>1) && ((theElement-1)->Type==1) && ((theElement-1)->pElementObject->m_Text) && ((theElement-1)->pElementObject->Data1[0]<='9') && ((theElement-1)->pElementObject->Data1[0]>='0'))
			{
				//typing decimal numbers in text mode - the decimal separator does not separate words - we are splicing
				{
					int ln=(int)strlen((theElement-1)->pElementObject->Data1);
					if (ln<22)
					{
						(theElement-1)->pElementObject->Data1[ln]=theElement->pElementObject->Data1[0];
						(theElement-1)->pElementObject->Data1[ln+1]=0;
						if (((ch>='0') &&(ch<='9')) || (ch=='.'))
						{
							theElement--;
							m_KeyboardCursorPos=ln+1;
							DeleteElement(m_IsKeyboardEntry-1);
							m_IsKeyboardEntry--;
						}
						/*else
						{
							theElement->pElementObject->Data1[0]=0;
							if (theElement->pElementObject->m_Text) theElement->pElementObject->m_Text=1;
							m_KeyboardCursorPos=0;
						}*/
					}
				}
			}
			if ((IsText) && (m_KeyboardCursorPos) && (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]!='.') && (theElement->pElementObject->Data1[m_KeyboardCursorPos]==0) &&
				((ch==',') || (ch=='.') || (ch==';') || (ch==':') || (ch=='!') || (ch=='?') || (ch==')') || (ch=='}') || (ch==']') || (ch=='\"')))
			{
				InsertEmptyElement(m_IsKeyboardEntry,1,0,fcolor);
				m_IsKeyboardEntry++;
				m_KeyboardCursorPos=0;
				theElement=m_pElementList+m_IsKeyboardEntry-1; //babaluj2
				theElement->pElementObject->m_Text=2;
			}
			if ((IsText) && (((m_KeyboardCursorPos==0) && (theElement->pElementObject->Data1[0])) || (m_KeyboardCursorPos==1)))
			{
				char pch=theElement->pElementObject->Data1[0];
				if (m_KeyboardCursorPos==0) 
				{
					pch=ch;
					if ((pch=='(') || (pch=='{') || (pch=='[') || (pch=='\"'))
					{
						InsertEmptyElement(m_IsKeyboardEntry-1,1,pch,fcolor);
						(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data2[0]=nFlags&0xFF;
						(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_VMods=(nFlags>>16)&0xFF;
						(m_pElementList+m_IsKeyboardEntry-1)->pElementObject->m_Text=4;
						m_IsKeyboardEntry++;
						m_KeyboardCursorPos=0;
						theElement=m_pElementList+m_IsKeyboardEntry-1; //babaluj2
						return 1;

					}
				}
				else
				{
					if ((pch=='(') || (pch=='{') || (pch=='[') || (pch=='\"'))
					{
						theElement->pElementObject->m_Text=4;
						m_IsKeyboardEntry++;
						InsertEmptyElement(m_IsKeyboardEntry-1,1,0,fcolor);
						m_KeyboardCursorPos=0;
						theElement=m_pElementList+m_IsKeyboardEntry-1; //babaluj2
					}
				}
			}

			if ((ch>='A') && (ch<='Z') || (ch>='a'))
			if ((theElement->pElementObject->Data1[m_KeyboardCursorPos]==0) && 
				(m_KeyboardCursorPos==1) && 
				(IsDoubleStrokeConversionUsed) &&
				(((theElement->pElementObject->Data2[0]&0xE0)==0x60) || ((theElement->pElementObject->Data2[0]==0x21))) && 
				(lastkeystrokes[1]==lastkeystrokes[2]))
			{
				if (((theElement->pElementObject->Data1[0]>='a') && (theElement->pElementObject->Data1[0]<='z') && (ch<='Z')) ||
					((theElement->pElementObject->Data1[0]=='m') && (((theElement->pElementObject->Data2[0])&0xE0)==0x60) && ((ch=='g') || (ch=='m') || (ch=='s'))) ||
					((theElement->pElementObject->Data1[0]=='D') && (((theElement->pElementObject->Data2[0])&0xE0)==0x60)))
				{
					//in some cases we don't want to revert from greek back to double latting even if the typing continues
					// - to write uP or uC (microporcessor, microcontroller) where lowercase greek is in front of uppercase lating
					// - to wrtie um, us or ug (micrometter microsecond microgram)
				}
				else
				{
					//if double letter was turned into Greek symbol (or special symbol letter), but the typing continues then we revert back
					if ((IsDoubleStrokeConversionUsed==2) && (theElement->pElementObject->Data1[0]=='m')) theElement->pElementObject->Data1[0]='u';
					theElement->pElementObject->Data2[0]=nFlags&0xFF;
					theElement->pElementObject->m_VMods=(nFlags>>16)&0xFF;
					theElement->pElementObject->Data2[1]=(nFlags&0xFF);
					theElement->pElementObject->m_VMods=(nFlags>>16)&0xFF;
					theElement->pElementObject->Data1[1]=theElement->pElementObject->Data1[0];
					m_KeyboardCursorPos++;
				}
			}
			IsDoubleStrokeConversionUsed=0;

			if ((theElement->pElementObject->IsMeasurementUnit()) && (((ch>='0') && (ch<='9')) || (ch=='.')))
			{
				//if a number is to be entered behind a measurement unit - separate it
				this->InsertEmptyElement(m_IsKeyboardEntry,1,0,fcolor);
				m_IsKeyboardEntry++;
				m_KeyboardCursorPos=0;
				theElement=this->m_pElementList+m_IsKeyboardEntry-1;
			}
			
			if ((m_KeyboardCursorPos>0) && (IsText==0) && (theElement->pElementObject->Data1[0]>='0') && (theElement->pElementObject->Data1[0]<='9') && (ch=='\''))
			{
				//typing the apostrophe after a number -> arc seconds and arc minutes
				if (theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=='.')
				{
					//if apostrophe is typed after dot - we will remove the dot (10.')
					theElement->pElementObject->Data1[m_KeyboardCursorPos-1]=0;
				}
		
				//convert the apostrophe into measurement unit 10'
				this->InsertEmptyElement(m_IsKeyboardEntry,1,'.',fcolor); //we use trick - as if the dot is typen erlier
				m_IsKeyboardEntry++;
				m_KeyboardCursorPos=1;
				theElement=this->m_pElementList+m_IsKeyboardEntry-1;
			}

			if ((IsText==0) &&
				((ch>='A')||(ch=='\'')) && (m_KeyboardCursorPos==1) && 
				(theElement->pElementObject) && (theElement->pElementObject->m_Text<2) &&
				(theElement->pElementObject->Data1[0]=='.') && 
				(theElement->pElementObject->Data1[1]==0))
			{
				//if the dot (period) was typen frist, then convert to measurement unit
				nFlags=(0x10<<16);
				m_KeyboardCursorPos=0;
				theElement->pElementObject->Data1[0]=0;
				theElement->pElementObject->m_VMods=0x10;
			}
			memmove(theElement->pElementObject->Data1+m_KeyboardCursorPos+1,theElement->pElementObject->Data1+m_KeyboardCursorPos,23-m_KeyboardCursorPos);
			memmove(theElement->pElementObject->Data2+m_KeyboardCursorPos+1,theElement->pElementObject->Data2+m_KeyboardCursorPos,23-m_KeyboardCursorPos);
			


			theElement->pElementObject->Data1[23]=0;
			theElement->pElementObject->Data1[m_KeyboardCursorPos]=ch;
			if (((theElement->pElementObject->m_VMods)==0x10) && (m_KeyboardCursorPos>0))
			{
				//if the variable is a measurement unit, then change everything to measurement unit font
				nFlags=0x00;
			}
			


			theElement->pElementObject->Data2[m_KeyboardCursorPos]=nFlags&0xFF; 
			if ((nFlags&0xFFFF00000) || (m_KeyboardCursorPos==0)) theElement->pElementObject->m_VMods=(nFlags>>16)&0xFF; 
			m_KeyboardCursorPos++;
			if ((m_IsKeyboardEntry>1) && /*(IsText==0) &&*/
				(theElement->pElementObject->Data1[0]!='\\') && 
				((theElement->pElementObject->Data1[0]>'9') || (theElement->pElementObject->Data1[0]=='\'')) &&
				((theElement->pElementObject->Data2[0]&0xF0)!=0x60))
			{
				//we are going set it as a measurement unit if it is typed behind an existing measurement unit
				CElement *el=(CElement*)((m_pElementList+m_IsKeyboardEntry-2)->pElementObject);
				int ellen=(int)strlen(el->Data1);
			
				if (((el->IsMeasurementUnit()) && (IsText==0)) || //if previous element is a measurement unit
					((el->m_Type==1) && (el->Data1[ellen-1]=='.') && (el->Data1[0]>='0') && (el->Data1[0]<='9') && (lastkeystrokes[1]!=' ')) || //or a previous element is a number that finishes with dot
					((el->m_Type==2) && ((el->Data1[0]==(char)0xD7) || (el->Data1[0]=='/')) && (m_IsKeyboardEntry>2) && ((m_pElementList+m_IsKeyboardEntry-3)->pElementObject->IsMeasurementUnit()))) //or if a previous element is * or / and the element before it is also a measurement unit
				{
					if (el->Data1[ellen-1]=='.') el->Data1[ellen-1]=0;
					for (int kk=0;((kk<(int)strlen(theElement->pElementObject->Data1)) && (kk<24));kk++)
					{
						/*if ((theElement->pElementObject->Data2[kk]&0xE0)==0x60)
							theElement->pElementObject->Data2[kk]=0x70;
						else
							theElement->pElementObject->Data2[kk]=0x10;*/
						theElement->pElementObject->Data2[kk]=0;
						theElement->pElementObject->m_VMods=0x10;
					}
				}
			}
			

			int xlen=(int)strlen(theElement->pElementObject->Data1);
			if ((m_KeyboardCursorPos==xlen) && (theElement->pElementObject->IsMeasurementUnit()))
			{
				//turn 'u' into micro sign when typing measurement units
				if (theElement->pElementObject->Data1[0]=='u')
				{
					int kk=0;
					while (ListOfKnownFunctions[kk].type)
					{
						if ((ListOfKnownFunctions[kk].type==100) && (strcmp(ListOfKnownFunctions[kk].name,theElement->pElementObject->Data1+1)==0))
						{
							theElement->pElementObject->Data2[0]=0x60;
							theElement->pElementObject->m_VMods=0x10;
							theElement->pElementObject->Data1[0]='m';
							break;
						}
						kk++;
					}
				}
				//change 'ohm' into ohm sign
				if ((xlen>=3) && (strcmp(theElement->pElementObject->Data1+xlen-3,"ohm")==0))
				{
					theElement->pElementObject->Data1[xlen-3]='W';
					theElement->pElementObject->Data2[xlen-3]=0x60;
					theElement->pElementObject->m_VMods=0x10;
					theElement->pElementObject->Data1[xlen-2]=0;
					xlen-=2;
					m_KeyboardCursorPos-=2;
				}
				//change 'deg' into the degree sign (also tripple apostrophe)
				if ((xlen==3) && ((strcmp(theElement->pElementObject->Data1,"deg")==0) || (strcmp(theElement->pElementObject->Data1,"\'\'\'")==0)))
				{
					theElement->pElementObject->Data1[0]=(char)0xB0;
					theElement->pElementObject->Data2[0]=0x60;
					theElement->pElementObject->m_VMods=0x10;
					theElement->pElementObject->Data1[1]=0;
					xlen=1;
					m_KeyboardCursorPos=1;
				}
				
				//change 'C into degree Celsius
				if ((xlen==2) && (strcmp(theElement->pElementObject->Data1,"\'C")==0))
				{
					theElement->pElementObject->Data1[0]=(char)0xB0;
					theElement->pElementObject->Data2[0]=0x60;
					theElement->pElementObject->Data1[1]='C';
					theElement->pElementObject->m_VMods=0x10;
					theElement->pElementObject->Data1[2]=0;
					xlen=2;
					m_KeyboardCursorPos=2;
				}

				//change the 'Nm' into newton-metre
				if ((xlen>=2) && (strcmp(theElement->pElementObject->Data1+xlen-2,"Nm")==0))
				{
					theElement->pElementObject->Data1[xlen-1]=0;
					InsertEmptyElement(m_IsKeyboardEntry,1,'m',fcolor);
					m_IsKeyboardEntry++;
					m_KeyboardCursorPos=1;
					theElement=this->m_pElementList+m_IsKeyboardEntry-1;
					theElement->pElementObject->m_VMods=0x10;
					xlen=1;
				}
				//change the 'Ns' into newton-second
				if ((xlen>=2) && (strcmp(theElement->pElementObject->Data1+xlen-2,"Ns")==0))
				{
					theElement->pElementObject->Data1[xlen-1]=0;
					InsertEmptyElement(m_IsKeyboardEntry,1,'s',fcolor);
					m_IsKeyboardEntry++;
					m_KeyboardCursorPos=1;
					theElement=this->m_pElementList+m_IsKeyboardEntry-1;
					theElement->pElementObject->m_VMods=0x10;
					xlen=1;
				}
			}
		}
		else if (((ch>='A') && (ch<'z')) || (ch=='\'') || (ch>=128) || ((ch>='0') && (ch<='9') && (m_KeyboardCursorPos>0)))
		{
			//typing function names (not a variable)
			for (i=23;i>m_KeyboardCursorPos;i--)
			{
				theElement->pElementObject->Data1[i]=0;
			}
			theElement->pElementObject->Data1[23]=0;
			theElement->pElementObject->Data1[m_KeyboardCursorPos]=ch;
			m_KeyboardCursorPos++;
		}
	}


	/*if (m_KeyboardCursorPos>22)
	{
		InsertEmptyElement(m_IsKeyboardEntry++,1,0,fcolor);
		
		m_KeyboardCursorPos=0;
	}*/


	return 1;
}

// only to be called from KeyboardKeyHit function!!
// This function will split the currently edited variable in two parts
// depending on the cursor position, it will return pointer to of two parts;
#pragma optimize("s",on)
tElementStruct* CExpression::KeyboardSplitVariable(void)
{
	if (m_IsKeyboardEntry<1) return NULL;
	if (m_IsKeyboardEntry>m_NumElements) return NULL;

	tElementStruct* theElement=m_pElementList+m_IsKeyboardEntry-1;

	if (theElement->pElementObject)
		if (m_KeyboardCursorPos<(short)strlen(theElement->pElementObject->Data1))
		{
			InsertEmptyElement(m_IsKeyboardEntry-1,1,0,Toolbox->GetFormattingColor());
			theElement=m_pElementList+m_IsKeyboardEntry-1;
			tElementStruct *theElement2=m_pElementList+m_IsKeyboardEntry;
			if (theElement2->pElementObject)
			{
				memcpy(theElement->pElementObject->Data1,theElement2->pElementObject->Data1,24);
				memcpy(theElement->pElementObject->Data2,theElement2->pElementObject->Data2,24);
				theElement->pElementObject->Data1[m_KeyboardCursorPos]=0;
				memmove(theElement2->pElementObject->Data1,theElement2->pElementObject->Data1+m_KeyboardCursorPos,24-m_KeyboardCursorPos);
				memmove(theElement2->pElementObject->Data2,theElement2->pElementObject->Data2+m_KeyboardCursorPos,24-m_KeyboardCursorPos);
			}
		}

	return theElement;
}

//this is used for quick-type functionality (text entry without text entry mode)
//the expression must be prepared (m_Selection)...
CElement *prevQuickTypeElement;
#pragma optimize("s",on)
int CExpression::KeyboardQuickType(CDC* DC, short zoom, UINT nChar, UINT nRepCnt, UINT nFlags,int fcolor, int *x, int *y)
{
	if (ViewOnlyMode) return 0;
	int retval=0;
	((CMainFrame*)(theApp.m_pMainWnd))->UndoDisableSaving();
	CExpression *tmp_clipboard_storage=ClipboardExpression;
	ClipboardExpression=NULL;

	KeyboardStart(DC,ViewZoom); //starts the keyboard entry at the m_Selection position
	KeyboardEntryObject=(CObject*)this;

	KeyboardExponentMode=KeyboardIndexMode=0;
	if (m_IsKeyboardEntry>=2)
	{
		//if the entered chaacter should be append to the just-to-left variable 
		tElementStruct *ts=m_pElementList+m_IsKeyboardEntry-2;
		if ((ts->pElementObject==prevQuickTypeElement) && (ts->Type==1))
		{
			DeleteElement(m_IsKeyboardEntry-1);
			m_IsKeyboardEntry--;
			m_KeyboardCursorPos=(int)strlen(ts->pElementObject->Data1);
		}
	
	}

	//adds the character
	CExpression *newFocus;
	if (KeyboardKeyHit(DC,ViewZoom,nChar,nRepCnt,nFlags,fcolor,2)==0) 
	{
		this->m_Selection=0;
		this->m_IsKeyboardEntry=0;
		goto keyboardquicktype_end;
	}
	newFocus=(CExpression*)KeyboardEntryObject; //the keyboard focus my change;
	if (newFocus==NULL) goto keyboardquicktype_end;
	if (newFocus->m_IsKeyboardEntry<1) goto keyboardquicktype_end;

	if ((KeyboardExponentMode) || (KeyboardIndexMode))
	{
		if (newFocus->KeyboardKeyHit(DC,ViewZoom,0x0D,nRepCnt,nFlags,fcolor,1)==0) goto keyboardquicktype_end;
		newFocus=(CExpression*)KeyboardEntryObject; //the keyboard focus my change;
		if (newFocus==NULL) goto keyboardquicktype_end;
		if (newFocus->m_IsKeyboardEntry<1) goto keyboardquicktype_end;
	}

	prevQuickTypeElement=(newFocus->m_pElementList+newFocus->m_IsKeyboardEntry-1)->pElementObject;
	if (nChar==' ') prevQuickTypeElement=NULL;

	CExpression *parent=newFocus;
	while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;	
	short l,a,b;
	parent->CalculateSize(DC,ViewZoom,&l,&a,&b);
	*x=0;
	*y=0;



	if (parent->GetKeyboardCursorPos(x,y))
	{
		int tt=newFocus->GetActualFontSize(ViewZoom);

		if ((newFocus->m_pElementList+newFocus->m_IsKeyboardEntry-1)->pElementObject->Data1[0]==0) 
		{
			//spacebar
			if (tt<20) *x-=1;
			*x-=tt/48;
		}
		else /*if (newFocus->m_IsKeyboardEntry<newFocus->m_NumElements)*/
			*x+=tt/20;
		
		
		
		*y+=newFocus->GetActualFontSize(ViewZoom)/8;
		retval=1;
	}


	if ((m_NumElements>2)&& (m_IsKeyboardEntry==m_NumElements) &&
		((m_pElementList+m_IsKeyboardEntry-1)->Type==1) &&
		((m_pElementList+m_IsKeyboardEntry-1)->pElementObject->Data1[0]==0) &&
		((m_pElementList+m_IsKeyboardEntry-2)->Type==2) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]==(char)0xFF) && 
		(m_StartAsText))
	{
		//the last element was empty and in the new line - convert it to the null element
		DeleteElement(m_IsKeyboardEntry-1);
		InsertEmptyElement(m_IsKeyboardEntry-1,0,0,Toolbox->GetFormattingColor());
		*x+=MovingDotSize;
	}


	//finish the keyboard entry mode
	newFocus->KeyboardStop();
keyboardquicktype_end:
	KeyboardEntryObject=NULL;
	KeyboardEntryBaseObject=NULL;
	ClipboardExpression=tmp_clipboard_storage;
	((CMainFrame*)(theApp.m_pMainWnd))->UndoEnableSaving();

	return retval;
}
#pragma optimize("",on)

//Depending on the CalculateOnly flag, this function does two things
// - it calculates inter-character spacing (X coordinates) of characters in the string
// - it displays the text
// 'text' is the string (character array) that must be displayed/calculated
// 'font' is the array of font descriptions (every character has its own font description)
// 'spacing' is input/output array of X coordinates for characters
// returns overall length of the string
// THIS FUNCTION MUST BE FAST!
int CalculateText(CDC *DC,char * text, char * font, short * spacing, short TheFontSize, char *IsHigh, char *IsLow,char IsText, char IsFirst, char VMods)
{
	if (TheFontSize<4)
	{
		// very small text (tiles view) is only shown as a shadow area
		int len=(int)strlen(text);
		if (len==0) return 1;
		if (len>=2) memset(spacing,0,len-2);
		spacing[len]=spacing[len-1]=len*2/3+1;
		return spacing[len];
	}

	int retval,shift2;
	if (VMods==0x10)//*IsMeasurementUnit
	{
		//if this is unit, make condesed writing
		retval=shift2=-TheFontSize/14;
		if ((*text==(char)0xB0) || (*text=='\'')) //degrees or minutes must be shifted a lot to the right
			retval=-TheFontSize/8;
	}
	else
	{
		shift2=-TheFontSize/21;
		if ((IsFirst) || (IsText))
			retval=shift2=0; 
		else
			retval=-TheFontSize/20;
	}

	int fff;
	*IsLow=0;
	if (VMods&0xEF) {*IsHigh=1;fff=2;} else {*IsHigh=0;fff=1;} //need to set some decoration (dash, arrow, kappa...)
	char string[2];
	string[1]=0;
	char ch=-1;
	char ft=-1;
	
	while (*text)
	{
		CSize cs;
		if (*font==(char)0xE3) 
		{
			//special characters - drawn by GDI functions (not from fonts)
			cs.cx=2*TheFontSize/3; 
			if (*text=='%') cs.cx=TheFontSize;
		}
		else
		{
			if (ft!=((*font)&0xE3))
			{
				HFONT hfont=GetFontFromPool(*font,max(TheFontSize,2)|0x8000);
				DC->SelectObject(hfont);
			}

			//determining the character widht (cs.cx)
			if (*text=='\'') 
				cs.cx=TheFontSize/5;
			else
			{
				*string=*text;
				cs=DC->GetTextExtent(string);
				if (*(text+1)=='\'') 
				{
					if (*font&0x02) cs.cx+=TheFontSize/16; //for italic fonts
					if (*text=='f') 
					{
						cs.cx+=TheFontSize/8; //for the 'f' add some more
					}
				}
				else if  (*(text+1)=='*')
				{
					if (*text=='f') 
					{
						cs.cx+=TheFontSize/8; //for the 'f' add some more
					}
				}
			}

			//special conditions
			if (*text=='.') retval-=TheFontSize/20;
			if (ch=='.') retval-=TheFontSize/20;
			if (ft==*font)
			{
				//VA or AV combinations
				if ((ft&0xE0)!=0x60) //non-greek alphabet
				{
					if (((ch=='V') || (ch=='W') || (ch=='Y')) && (*text=='A')) retval-=TheFontSize/15;
					if ((ch=='A') && ((*text=='V') || (*text=='W') || (*text=='Y'))) retval-=TheFontSize/15;
				}
				else
				{
					if (((ch=='D') || (ch=='L') || (ch=='A')) && (*text=='Y')) retval-=TheFontSize/15;
					if ((ch=='Y') && ((*text=='D') || (*text=='L') || (*text=='A'))) retval-=TheFontSize/15;
				}
			}
			if ((*font&0xE2)==0x62) 
			{
				//italic greek leters should be moved to left a bit
				short dd=TheFontSize/12;
				retval-=dd;cs.cx+=dd;
			} 
		}

		if ((*IsHigh<2) && (IsCharacterHigh(*text,*font))) *IsHigh=fff;
		if ((*IsLow==0) && (IsCharacterLow(*text,*font))) *IsLow=1;
		*spacing=retval;

		retval+=(short)cs.cx;
		spacing++;
		ch=*text;
		ft=*font;
		font++;
		text++;
	}
	retval+=shift2;
	*spacing=retval;

	return retval;
}

//Paints the text - the text must be earlier prepared for painting by call to CalculateText
//THIS FUNCTION MUST BE VERY FAST!
int PaintText(CDC *DC,int X, int Y,char *text, char *font, short *spacing, short TheFontSize,int IsBlue,int color,char IsText,char VMods)
{
	if ((*text)==0) return 0;
	if (TheFontSize<4)
	{
		int len=(int)strlen(text);
		DC->FillSolidRect(X,Y,spacing[len-1],TheFontSize,(IsBlue)?BLUE_COLOR:PALE_RGB(color));
		return 1;
	}

	const int ftmp=TheFontSize/3;
	char prevFont=font[0];
	if ((text[1]==0) && (VMods==0)/* && (text[0]!='\'') */&& (prevFont!=(char)0xE3) && (text[15]!=(char)(126+128)))
	{
		//if there is only one character to be printed, and there is no need for special
		//decorations, then we can do it way faster.
		DC->SelectObject(GetFontFromPool(prevFont,TheFontSize|0x8000));
		if (!IsBlue)
		{
			DC->SetTextColor(color);
		}
		else
		{
			DC->SetTextColor(BLUE_COLOR);
			if ((ShadowSelection) && (TheFontSize>10) && (TheFontSize<40))
				DC->TextOut(X+spacing[0]+1,Y+ftmp,text,1);
		}
		DC->TextOut(X+spacing[0],Y+ftmp,text,1);
		return 1;
	}

	if (IsBlue) {DC->SetTextColor(BLUE_COLOR);}
	else if ((IsText) || (IsHighQualityRendering) || (VMods!=0x10)) DC->SetTextColor(color);
	else {DC->SetTextColor(PALE_RGB(color));}  //unit of measurement

	char HaveHighCharacters=0;
	int j=0;
	int prev_j=0;
	
	while (1)
	{
		if ((text[j]==0) || (prevFont!=font[j]) || (text[j]=='\'') || ((j>0) && (text[j-1]=='\'')))
		{
			if (prevFont==(char)0xE3)
			{
				CBrush *brush=NULL;

				//special handling for special characters (font 0xE3)
				if (IsBlue)
				{
					brush=new CBrush(BLUE_COLOR);
					DC->SelectObject(GetPenFromPool(1,1,0));
					DC->SelectObject(brush);					
				}
				else
				{
					brush=new CBrush(color);
					DC->SelectObject(brush);
					DC->SelectObject(GetPenFromPool(1,0,color));
				}

				for (int ii=prev_j;ii<j;ii++)
				{
					int r=max((TheFontSize+12)/12,1);
					int xii=X+spacing[ii];

					if (text[ii]=='H')  //horizontal triple dot
					{
						DC->Ellipse(xii,Y,xii+r,Y+r);
						DC->Ellipse(xii+2*TheFontSize/6-TheFontSize/16,Y,xii+2*TheFontSize/6+r-TheFontSize/16,Y+r);
						DC->Ellipse(xii+2*ftmp-r,Y,xii+2*ftmp,Y+r);
					}
					if (text[ii]=='V')  //vertical triple dot
					{
						DC->Ellipse(xii+ftmp,Y-ftmp,xii+ftmp+r,Y-ftmp+r);
						DC->Ellipse(xii+ftmp,Y-TheFontSize/16,xii+ftmp+r,Y+r-TheFontSize/16);
						DC->Ellipse(xii+ftmp,Y+ftmp-r,xii+ftmp+r,Y+ftmp);
					}
					if (text[ii]=='A')  //angled triple dot
					{
						DC->Ellipse(xii,Y-ftmp,xii+r,Y-ftmp+r);
						DC->Ellipse(xii+ftmp-TheFontSize/16,Y-TheFontSize/16,xii+ftmp+r-TheFontSize/16,Y+r-TheFontSize/16);
						DC->Ellipse(xii+2*ftmp-r,Y+ftmp-r,xii+2*ftmp,Y+ftmp);
					}
					if (text[ii]=='U')  //up-angled triple dot
					{
						DC->Ellipse(xii,Y+ftmp-r,xii+r,Y+ftmp);
						DC->Ellipse(xii+ftmp-TheFontSize/16,Y-TheFontSize/16,xii+ftmp+r-TheFontSize/16,Y+r-TheFontSize/16);
						DC->Ellipse(xii+2*ftmp-r,Y-ftmp,xii+2*ftmp,Y-ftmp+r);
					}
					if (text[ii]=='%') //permille
					{
						int r=max((TheFontSize+2)/4,1);
						DC->SelectObject(GetPenFromPool(max(1,TheFontSize/14),IsBlue,color));
						DC->Arc(xii,Y+ftmp-2*r-r/2,xii+r,Y+ftmp-1*r-r/8,-100,-100,-100,-101);
						DC->Arc(xii+1*r+r/4,Y+ftmp-r-r/2+r/8,xii+2*r+r/4,Y+ftmp,-100,-100,-100,-101);
						DC->Arc(xii+2*r+r/2,Y+ftmp-r-r/2+r/8,xii+3*r+r/2,Y+ftmp,-100,-100,-100,-101);
						DC->MoveTo(xii+r/4,Y+ftmp);
						DC->LineTo(xii+r+r,Y+ftmp-2*r-r/2);
					}
				}
				if (brush) delete brush;
			}
			else
			{
				int ggg[24];
				int k,l=0;
				for (k=prev_j;k<j;k++)
					ggg[l++]=spacing[k+1]-spacing[k];

				HFONT hfont;
				int aa;
				if (*(text+prev_j)=='\'') 
				{
					aa=TheFontSize/8;
					hfont=GetFontFromPool(0,max(TheFontSize,2)|0x8000);
				}
				else
				{
					aa=0;
					hfont=GetFontFromPool(prevFont,max(TheFontSize,2)|0x8000);
				}

				DC->SelectObject(hfont);
				if ((ShadowSelection) && (IsBlue) && (TheFontSize>10) && (TheFontSize<40))
					DC->ExtTextOut(X+spacing[prev_j]+1,Y+ftmp-aa,0,NULL,text+prev_j,j-prev_j,ggg);
				DC->ExtTextOut(X+spacing[prev_j],Y+ftmp-aa,0,NULL,text+prev_j,j-prev_j,ggg);
			}
			if (text[j]==0) break;
			prev_j=j;
			prevFont=font[j];
		}
		if ((HaveHighCharacters==0) && (VMods) && (IsCharacterHigh(text[j],font[j]))) 
			HaveHighCharacters=1;
		j++;
	}
	
	if (((text[14]==0) && (text[15]==(char)(126+128)) && (TheFontSize>13) && (j>0))
		|| (VMods==0x24))
	{
		//PAINTING A TILDAE SIGN (below the last digit if this is a rounded number, or above a symbol)
		int Y1=Y+ftmp+((TheFontSize>=20)?TheFontSize/20:1);
		int Y2=Y1+TheFontSize/10;
		int Y3=Y1+TheFontSize/20;
		int X1,X3;
		int col;

		if (VMods==0x24)
		{
			col=color;
			if (IsBlue) col=BLUE_COLOR;
			int dd=TheFontSize/2+TheFontSize/4;
			if (HaveHighCharacters) dd+=TheFontSize/10;
			Y1-=dd;
			Y2-=dd;
			Y3-=dd;
			X1=X+spacing[0]+TheFontSize/20;
			X3=X+spacing[j]-TheFontSize/20;
			if (font[0]&0x02) //italic font
			{
				X1+=TheFontSize/12;
				X3+=TheFontSize/12;
			}
			if (X3-X1<TheFontSize/4)
			{
				int d=TheFontSize/4-(X3-X1);
				X1-=d/2;
				X3+=d/2;
			}
			if (X3-X1>TheFontSize/2)
			{
				int d=(X3-X1)-TheFontSize/2;
				X1+=d/2;
				X3-=d/2;
			}
		}
		else
		{
			col=PALE_RGB(PALE_RGB(color));
			if (IsBlue) col=PALE_RGB(BLUE_COLOR); 
			X1=X+spacing[j-1]+TheFontSize/20;
			X3=X+spacing[j]-TheFontSize/20;
		}


		if (X3-X1<16)
		{
			int n=(X3-X1-3)/2;
			int ij=0;
			DC->SetPixel(X1+ij,Y3,col);ij++;
			for (int ijj=0;ijj<n;ijj++) {DC->SetPixel(X1+ij,Y3-1,col);ij++;}
			DC->SetPixel(X1+ij,Y3,col);ij++;
			for (int ijj=0;ijj<n;ijj++) {DC->SetPixel(X1+ij,Y3+1,col);ij++;}
			DC->SetPixel(X1+ij,Y3,col);
		}
		else
		{
			DC->SelectObject(GetPenFromPool(max(TheFontSize/24,1),0,col));
			int X2=(X1+X3)/2;
			int Y4=Y3+TheFontSize/48;
			int Y5=Y3;//-TheFontSize/132;
			DC->Arc(X1,Y2,X2,Y1,X2,Y3,X1,Y4);
			DC->Arc(X2,Y2,X3,Y1,X2,Y3,X3,Y5);
		}
	}

	if (VMods) //if we have to draw dash/arrow/...
	{
		int OverallLength=spacing[j];
		int corry=0;
		int corrx=0;
		int linesize=TheFontSize/15;if (linesize<1) linesize=1;
		if (HaveHighCharacters) corry=TheFontSize/8; else corry=-TheFontSize/12;
		if (font[0]&0x02) corrx+=TheFontSize/11; //italic font

		DC->SelectObject(GetPenFromPool(linesize,IsBlue,color));
		if ((VMods)==0x04) //dash
		{
			DC->MoveTo(X+corrx,Y-ftmp-corry);	
			DC->LineTo(X+OverallLength+corrx,Y-ftmp-corry);
		}
		if ((VMods)==0x08) //arrow
		{
			int arrow_len=9*TheFontSize/24;
			if (7*OverallLength/8>arrow_len) arrow_len=7*OverallLength/8;
			if (arrow_len>9*TheFontSize/8) arrow_len=9*TheFontSize/8;
			corrx+=(OverallLength-arrow_len)/2;
			POINT p[9];
			X=X+corrx;
			linesize=TheFontSize/15;
			Y=Y-corry-9*TheFontSize/24-linesize/2;

			if (((linesize%2)==0) && (linesize>0))
			{
				p[0].x=X;								p[0].y=Y-linesize/2;
				p[1].x=X+arrow_len-TheFontSize/10;		p[1].y=Y-linesize/2;
				p[2].x=p[1].x-TheFontSize/12;			p[2].y=Y-TheFontSize/10;
				p[3].x=X+arrow_len;						p[3].y=Y;
				p[4].x=p[2].x;							p[4].y=Y+TheFontSize/10;
				p[5].x=p[1].x;							p[5].y=Y+linesize/2-1;
				p[6].x=X;								p[6].y=Y+linesize/2-1;
			}
			else
			{
				p[0].x=X;								p[0].y=Y-linesize/2;
				p[1].x=X+arrow_len-TheFontSize/10;		p[1].y=Y-linesize/2;
				p[2].x=p[1].x-TheFontSize/12;			p[2].y=Y-TheFontSize/10;
				p[3].x=X+arrow_len;						p[3].y=Y;
				p[4].x=p[2].x;							p[4].y=Y+TheFontSize/10;
				p[5].x=p[1].x;							p[5].y=Y+linesize/2;
				p[6].x=X;								p[6].y=Y+linesize/2;
			}
			CBrush *blue=new CBrush((IsBlue)?BLUE_COLOR:color);
			DC->SelectObject(blue);
			DC->SelectObject(GetPenFromPool(1,IsBlue,color));
			DC->Polygon((LPPOINT)&p,7);
			delete blue;

		}
		if ((VMods)==0x0C) //hat
		{
			int arrow_len=ftmp;
			if (3*OverallLength/4>arrow_len) arrow_len=3*OverallLength/4;
			if (arrow_len>TheFontSize) arrow_len=TheFontSize;
			corrx+=(OverallLength-arrow_len)/2;
			Y+=linesize/2;
			DC->MoveTo(X+corrx,Y-ftmp-corry);	
			DC->LineTo(X+arrow_len/2+corrx,Y-ftmp-corry-TheFontSize/8);
			DC->LineTo(X+arrow_len+corrx,Y-ftmp-corry);
			DC->SetPixel(X+arrow_len+corrx,Y-ftmp-corry,(IsBlue)?BLUE_COLOR:0);

		}
		if ((VMods)==0x1C) //hacek
		{
			int arrow_len=TheFontSize/4;
			if (OverallLength/2>arrow_len) arrow_len=OverallLength/2;
			if (arrow_len>TheFontSize/2) arrow_len=TheFontSize/2;
			corrx+=(OverallLength-arrow_len)/2;
			corry-=TheFontSize/24;
			Y+=linesize/2;
			DC->MoveTo(X+corrx,Y-ftmp-corry-TheFontSize/8);	
			DC->LineTo(X+arrow_len/2+corrx,Y-ftmp-corry);
			DC->LineTo(X+arrow_len+corrx,Y-ftmp-corry-TheFontSize/8);
			DC->SetPixel(X+arrow_len+corrx,Y-ftmp-corry-TheFontSize/8,(IsBlue)?BLUE_COLOR:0);

		}
		if (((VMods)==0x14) || ((VMods)==0x18) || (VMods)==0x20) //dot, double dot, triple dot
		{
			int XX=X+OverallLength/2+corrx+TheFontSize/24;
			int YY=Y-ftmp-corry;
			int z=max((TheFontSize+20)/24,1);
			
			int offcenter=0;
			if ((VMods)==0x18) {offcenter=max(2,TheFontSize/10);if (TheFontSize<=12) offcenter=1;}
			if ((VMods)==0x20) {offcenter=max(2,TheFontSize/6);if (TheFontSize<=12) offcenter=2;}

			if ((TheFontSize>=20) && (TheFontSize<30))
			{
				DC->SetPixel(XX+offcenter,YY-1,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
				DC->SetPixel(XX+offcenter,YY-2,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
				DC->SetPixel(XX+offcenter-1,YY-1,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
				DC->SetPixel(XX+offcenter-1,YY-2,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
				if (((VMods)==0x18) || (VMods==0x20))
				{
					DC->SetPixel(XX-offcenter,YY-1,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
					DC->SetPixel(XX-offcenter,YY-2,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
					DC->SetPixel(XX-offcenter-1,YY-1,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
					DC->SetPixel(XX-offcenter-1,YY-2,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
				}
				if ((VMods)==0x20) 
				{
					DC->SetPixel(XX,YY-1,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
					DC->SetPixel(XX,YY-2,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
					DC->SetPixel(XX-1,YY-1,(IsBlue)?BLUE_COLOR:RGB(64,64,64));
					DC->SetPixel(XX-1,YY-2,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
				}
			}
			else if ((TheFontSize>8) && (TheFontSize<20))
			{
				DC->SetPixel(XX+offcenter-1,YY,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
				DC->SetPixel(XX+offcenter-1,YY-1,(IsBlue)?BLUE_COLOR:RGB(128,128,128));
				if (((VMods)==0x18) || (VMods==0x20))
				{
					DC->SetPixel(XX-offcenter-1,YY,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
					DC->SetPixel(XX-offcenter-1,YY-1,(IsBlue)?BLUE_COLOR:RGB(128,128,128));
				}
				if (VMods==0x20)
				{
					DC->SetPixel(XX-1,YY,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
					DC->SetPixel(XX-1,YY-1,(IsBlue)?BLUE_COLOR:RGB(128,128,128));
				}
				if (((VMods)!=0x18) && (VMods!=0x20)) 
				if (TheFontSize>15)
				{
					DC->SetPixel(XX+offcenter,YY,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
					DC->SetPixel(XX+offcenter,YY-1,(IsBlue)?BLUE_COLOR:RGB(128,128,128));
					{
						DC->SetPixel(XX-offcenter,YY,(IsBlue)?BLUE_COLOR:RGB(0,0,0));
						DC->SetPixel(XX-offcenter,YY-1,(IsBlue)?BLUE_COLOR:RGB(128,128,128));
					}
				}
			}
			else
			{
				CBrush *blue=new CBrush((IsBlue)?BLUE_COLOR:color);
				DC->SelectObject(blue);			
				DC->Ellipse(XX-z/2+offcenter,YY-z/2,XX+(z+1)/2+offcenter,YY+(z+1)/2);
				if (((VMods)==0x18) || (VMods==0x20))
				{
					DC->Ellipse(XX-z/2-offcenter,YY-z/2,XX+(z+1)/2-offcenter,YY+(z+1)/2);
				}
				if (VMods==0x20)
					DC->Ellipse(XX-z/2,YY-z/2,XX+(z+1)/2,YY+(z+1)/2);

				delete blue;
			}
		}
	}
	return 1;
}


int CExpression::Autocomplete(int is_internal)
{
	if (is_internal)
	{
		int i;
		int Points=0;

		if ((this->m_NumElements==1) && (this->m_pElementList->Type==2) || (this->m_pElementList->Type==0)) 
			return 1; //some pointless math line (single operator - this happens sometimes with exponents or indexes)
		
		CExpression *org=(CExpression*)KeyboardEntryObject;

		if (org==this) return 1;
		
		//we must check if the KeyboardEntryObject is conatained in 
		//this expression, if yes we should not try (avoid self-reference)
		CExpression *e=org;
		while (e)
		{
			if (e==this) goto autocomplete_doall;
			if ((e->m_pPaternalElement) && (e->m_pPaternalElement==this->m_pPaternalElement)) goto autocomplete_doall;
			e=e->m_pPaternalExpression;
		}
		e=this;
		while (e)
		{
			if (e==org) goto autocomplete_doall;
			if ((e->m_pPaternalElement) && (e->m_pPaternalElement==org->m_pPaternalElement)) goto autocomplete_doall;
			e=e->m_pPaternalExpression;
		}
		
		int num_obj=org->m_NumElements;
		if (num_obj)
		if (((org->m_pElementList+num_obj-1)->Type==1) && 
			((org->m_pElementList+num_obj-1)->pElementObject->Data1[0]==0))
			num_obj--; //if the last element of is an empty varaible, don't take it into the account

		if (num_obj)
		if (((org->m_pElementList+num_obj-1)->Type==2) && 
			((org->m_pElementList+num_obj-1)->pElementObject->Data1[0]==','))
			num_obj--; //if the last element of is comma, don't take it into the account (this enables double-comma triggering)

		//check if all elements are equal (up to the number of typed elements in org)
		i=0;
		if (num_obj<m_NumElements)
			for (;i<num_obj;i++)
			{
				if (i>=m_NumElements) break;
				tElementStruct *es1=org->m_pElementList+i;
				tElementStruct *es2=m_pElementList+i;
				if (es1!=es2) 
					if (this->CompareElement(es2,es1)==0) break;
			}

		if (i==num_obj)
		{
			//we will add points
			CExpression *a=org;
			CExpression *b=this;
			char decrease_points=0;
			if ((org->m_pPaternalElement) && (this->m_pPaternalElement))
			{
				//if one of objects hast parentheses around, then we will move to levelo lower (outside parentheses)
				if ((a->m_pPaternalElement->m_Type==5) && (a->m_pPaternalElement->Expression2==NULL) &&
					(b->m_pPaternalElement->m_Type!=5))
				{
					a=a->m_pPaternalExpression;
					decrease_points=1;
				}
				if ((b->m_pPaternalElement->m_Type==5) && (b->m_pPaternalElement->Expression2==NULL) &&
					(a->m_pPaternalElement->m_Type!=5))
				{
					b=b->m_pPaternalExpression;
					decrease_points=1;
				}
			}

			if ((a->m_pPaternalElement) && (b->m_pPaternalElement) && (a->m_pPaternalElement!=b->m_pPaternalElement))
			{

				if (a->m_pPaternalElement->m_Type==b->m_pPaternalElement->m_Type)
				{
					if (((b->m_pPaternalElement->m_Type==1) || (b->m_pPaternalElement->m_Type==6)) && 
						(strcmp(a->m_pPaternalElement->Data1,b->m_pPaternalElement->Data1)==0))
						Points+=10;//add one point if the name of string/function is the same
					if (((b->m_pPaternalElement->m_Type==4) || (b->m_pPaternalElement->m_Type==7) || (b->m_pPaternalElement->m_Type==8)) && 
						(a->m_pPaternalElement->Data1[0]==b->m_pPaternalElement->Data1[0]))
					{
						Points+=10;
					}
					if ((b->m_pPaternalElement->m_Type==3) && (b->m_pPaternalElement->Expression2==(CObject*)b) && (a->m_pPaternalElement->Expression2==(CObject*)a))
					{
						//if exponent bases are equal, then we can copy exponent
						CExpression *base=((CExpression*)(b->m_pPaternalElement->Expression1));
						CExpression *base2=((CExpression*)(a->m_pPaternalElement->Expression1));
						if (base->IsSuitableForComputation())
						if (base2->IsSuitableForComputation())
						if (base->CompareExpressions(0,-1,base2,0,-1))
							Points+=10;
					}
					if (b->m_pPaternalElement->m_Type==5)
						Points+=10;
					if (Points)
					{
						if ((a->m_pPaternalElement->Expression1==(CObject*)a) && (b->m_pPaternalElement->Expression1!=(CObject*)b)) Points=0;
						if ((a->m_pPaternalElement->Expression2==(CObject*)a) && (b->m_pPaternalElement->Expression2!=(CObject*)b)) Points=0;
						if ((a->m_pPaternalElement->Expression3==(CObject*)a) && (b->m_pPaternalElement->Expression3!=(CObject*)b)) Points=0;
					}
					if (Points)
					{
						//if depth is the same, add some more points
						int depth1=0;
						int depth2=0;
						int f=2;
						CExpression *p1=a;
						while (p1->m_pPaternalExpression)
						{
							depth1+=(p1->m_pPaternalElement->m_Type*f);
							if (p1->m_pPaternalElement->Expression1==(CObject*)p1) depth1+=10;
							if (p1->m_pPaternalElement->Expression2==(CObject*)p1) depth1+=20;
							if (p1->m_pPaternalElement->Expression3==(CObject*)p1) depth1+=30;
							p1=p1->m_pPaternalExpression;
						}
						f=2;
						p1=b;
						while (p1->m_pPaternalExpression)
						{
							depth2+=(p1->m_pPaternalElement->m_Type*f);
							if (p1->m_pPaternalElement->Expression1==(CObject*)p1) depth2+=10;
							if (p1->m_pPaternalElement->Expression2==(CObject*)p1) depth2+=20;
							if (p1->m_pPaternalElement->Expression3==(CObject*)p1) depth2+=30;
							p1=p1->m_pPaternalExpression;
						}
						if (depth1==depth2) Points+=8;
					}
					if (Points)
					{
						//check what is in front of - if the same, add some more points
						CElement *e1=a->m_pPaternalElement->GetPreviousElement();
						CElement *e2=b->m_pPaternalElement->GetPreviousElement();
						if ((e1==NULL) && (e2==NULL)) 
							Points+=6;
						else if ((e1) && (e2) && (e1!=e2))
						{
							tElementStruct *ts1=a->m_pPaternalExpression->GetElementStruct(e1);
							tElementStruct *ts2=b->m_pPaternalExpression->GetElementStruct(e2);
							if ((ts1) && (ts2) && (b->CompareElement(ts1,ts2))) Points+=8;
						}
					}
					if (Points)
					{
						//check what follows - if the same, add some more points
						CElement *e1=a->m_pPaternalElement->GetNextElement();
						CElement *e2=b->m_pPaternalElement->GetNextElement();
						if ((e1==NULL) && (e2==NULL)) 
							Points+=4;
						else if ((e1) && (e2) && (e1!=e2))
						{
							tElementStruct *ts1=a->m_pPaternalExpression->GetElementStruct(e1);
							tElementStruct *ts2=b->m_pPaternalExpression->GetElementStruct(e2);
							if ((ts1) && (ts2) && (b->CompareElement(ts1,ts2))) Points+=6;
						}
					}
					//check if other expressions in the same object are the same - if yes, add some more points
					if ((a->m_pPaternalElement->Expression1!=(CObject*)a) &&
						(b->m_pPaternalElement->Expression1!=(CObject*)b) &&
						(a->m_pPaternalElement->Expression1) && (b->m_pPaternalElement->Expression1))
					{
						int ttmp=CalcStructuralChecksumOnly;
						CalcStructuralChecksumOnly=1;
						if (((CExpression*)a->m_pPaternalElement->Expression1)->CalcChecksum()==((CExpression*)b->m_pPaternalElement->Expression1)->CalcChecksum())
							Points+=10;
						else 
							if ((((CExpression*)a->m_pPaternalElement->Expression1)->IsSuitableForComputation()) &&
								(((CExpression*)b->m_pPaternalElement->Expression1)->IsSuitableForComputation()) &&
								(((CExpression*)a->m_pPaternalElement->Expression1)->CompareExpressions(0,-1,(CExpression*)b->m_pPaternalElement->Expression1,0,-1)))
								Points+=5;
						CalcStructuralChecksumOnly=ttmp;
					}

					if ((a->m_pPaternalElement->Expression2!=(CObject*)a) &&
						(b->m_pPaternalElement->Expression2!=(CObject*)b) &&
						(a->m_pPaternalElement->Expression2) && (b->m_pPaternalElement->Expression2))
					{
						int ttmp=CalcStructuralChecksumOnly;
						CalcStructuralChecksumOnly=1;
						if (((CExpression*)a->m_pPaternalElement->Expression2)->CalcChecksum()==((CExpression*)b->m_pPaternalElement->Expression2)->CalcChecksum())
							Points+=10;
						else 
							if ((((CExpression*)a->m_pPaternalElement->Expression2)->IsSuitableForComputation()) &&
								(((CExpression*)b->m_pPaternalElement->Expression2)->IsSuitableForComputation()) &&
								(((CExpression*)a->m_pPaternalElement->Expression2)->CompareExpressions(0,-1,(CExpression*)b->m_pPaternalElement->Expression2,0,-1)))
								Points+=5;
						CalcStructuralChecksumOnly=ttmp;
					}

					if ((a->m_pPaternalElement->Expression3!=(CObject*)a) &&
						(b->m_pPaternalElement->Expression3!=(CObject*)b) &&
						(a->m_pPaternalElement->Expression3) && (b->m_pPaternalElement->Expression3))
					{
						int ttmp=CalcStructuralChecksumOnly;
						CalcStructuralChecksumOnly=1;
						if (((CExpression*)a->m_pPaternalElement->Expression3)->CalcChecksum()==((CExpression*)b->m_pPaternalElement->Expression3)->CalcChecksum())
							Points+=10;
						else 
							if ((((CExpression*)a->m_pPaternalElement->Expression3)->IsSuitableForComputation()) &&
								(((CExpression*)b->m_pPaternalElement->Expression3)->IsSuitableForComputation()) &&
								(((CExpression*)a->m_pPaternalElement->Expression3)->CompareExpressions(0,-1,(CExpression*)b->m_pPaternalElement->Expression3,0,-1)))
								Points+=5;
						CalcStructuralChecksumOnly=ttmp;
					}
				}
				if (decrease_points) Points=Points*3/4;
			}
			if ((Points) && (Points>=AutocompletePoints) && (this->m_pElementList->pElementObject))
			{
				if ((org->m_IsKeyboardEntry<=org->m_NumElements) &&
					((org->m_pElementList+org->m_IsKeyboardEntry-1)->Type==1) && 
					((org->m_pElementList+org->m_IsKeyboardEntry-1)->pElementObject->Data1[0]==0))
					AutocompleteTriggered=1;
				else
					AutocompleteTriggered=0;

				AutocompletePoints=Points;
				AutocompleteSource=this;
				
			}
			
		}
autocomplete_doall:
		for (i=0;i<m_NumElements;i++)
		{
			tElementStruct *ts=m_pElementList+i;
			if (ts->pElementObject)
			{
				if (ts->pElementObject->Expression1) ((CExpression*)ts->pElementObject->Expression1)->Autocomplete(1);
				if (ts->pElementObject->Expression2) ((CExpression*)ts->pElementObject->Expression2)->Autocomplete(1);
				if (ts->pElementObject->Expression3) ((CExpression*)ts->pElementObject->Expression3)->Autocomplete(1);

			}
		}
		return 1;
	}

	AutocompleteSource=NULL;
	AutocompletePoints=0;
	AutocompleteTriggered=0;

	if (KeyboardEntryObject==NULL) return 0;
	if (KeyboardEntryBaseObject==NULL) return 0;
	if (m_IsKeyboardEntry<=0) return 0;
	if (m_NumElements>m_IsKeyboardEntry) return 0;
	if ((m_pElementList+m_IsKeyboardEntry-1)->Type!=1) return 0;
	if (m_NumElements>m_IsKeyboardEntry+1) return 0;

	
	int X=KeyboardEntryBaseObject->absolute_X;
	int Y=KeyboardEntryBaseObject->absolute_Y;
	int L=KeyboardEntryBaseObject->Length;

	for (int i=0;i<NumDocumentElements;i++)
	{
		tDocumentStruct *ds=TheDocument+i;
		if ((ds->Type==1) && (ds->absolute_X+ds->Length>X-100) && (ds->absolute_X<X+L+100) &&
			(ds->absolute_Y<=Y+20) && (ds->absolute_Y>Y-200))
		{
			((CExpression*)ds->Object)->Autocomplete(1);
		}
	}

	if (AutocompleteSource)
	{
		//found something
	
	}
	return 1;
}

tElementStruct *CExpression::GetElementStruct(CElement *element)
{
	tElementStruct *ts=m_pElementList;
	for (int i=0;i<m_NumElements;i++,ts++)
	{
		if (ts->pElementObject==element) return ts;
	}
	return NULL;
}

int CExpression::SetCellAttributes(int row, int column,char align, char top, char bottom, char left, char right)
{
	tCellAttributes att;

	if (row>=m_MaxNumRows) return 0;
	if (column>=m_MaxNumColumns) return 0;
	if ((m_MaxNumRows==1) && (m_MaxNumColumns==1)) return 0;


	if (GetCellAttributes(row,column,&att))
	{
		*att.alignment=align;
		*att.top_border=top;
		*att.bottom_border=bottom;
		*att.left_border=left;
		*att.right_border=right;
		*att.data_known=' ';
		if ((column<m_MaxNumColumns-1) && (GetCellAttributes(row,column+1,&att)))
			*att.left_border=right;
		if ((row<m_MaxNumRows-1) && (GetCellAttributes(row+1,column,&att)))
			*att.top_border=bottom;

		return 1;
	}
	else 
		return 0;
}

/*int CExpression::GetCellAttributes(int row, int column,char *align, char *top, char *bottom, char *left, char *right)
{
	tCellAttributes att;
	*align=0;
	*top=*bottom=*left=*right=0;

	if (row>=m_MaxNumRows) return 0;
	if (column>=m_MaxNumColumns) return 0;
	if ((m_MaxNumRows==1) && (m_MaxNumColumns==1)) return 0;

	if (GetCellAttributes(row,column,&att))
	{
		*align=*att.alignment;
		*top=*att.top_border;
		*bottom=*att.bottom_border;
		*left=*att.left_border;
		*right=*att.right_border;
		return 1;
	}
	else 
		return 0;
}
*/


//returns pointers to cell attributes (collected inside the tCellAttributes structure
//The 'alternative_data' can be requested (1=data for cell to the right, 2=data for cell below)
int  CExpression::GetCellAttributes(int row, int column,tCellAttributes *attributes)
{
	if (row>=m_MaxNumRows) return 0;
	if (column>=m_MaxNumColumns) return 0;
	if ((m_MaxNumRows==1) && (m_MaxNumColumns==1)) return 0;

	int elm=FindMatrixElement(row,column,0); //getting the first element within the cell
	if (elm<0) elm=0;
	int k;
	for (k=elm;k<m_NumElements;k++)
	{
		if (((m_pElementList+k)->Type==11) || ((m_pElementList+k)->Type==12)) break;
	}
	if (k==m_NumElements) return 0; //not found

	/*if (k==m_NumElements)
	{
		if ((elm>0) && (alternative_data==0))
		{
			k=elm-1;
			if ((m_pElementList+k)->Type==11)
			{
				attributes->top_border=&((m_pElementList+k)->pElementObject->Data1[4]);
				attributes->right_border=&((m_pElementList+k)->pElementObject->Data1[5]);
				attributes->bottom_border=&((m_pElementList+k)->pElementObject->Data1[6]);
				attributes->left_border=&((m_pElementList+k)->pElementObject->Data1[1]);
				attributes->alignment=&((m_pElementList+k)->pElementObject->Data1[11]);
				attributes->data_known=&((m_pElementList+k)->pElementObject->Data1[21]);
				return 1;
			}
			if ((m_pElementList+k)->Type==12)
			{
				attributes->top_border=&((m_pElementList+k)->pElementObject->Data1[2]);
				attributes->right_border=&((m_pElementList+k)->pElementObject->Data1[7]);
				attributes->bottom_border=&((m_pElementList+k)->pElementObject->Data1[8]);
				attributes->left_border=&((m_pElementList+k)->pElementObject->Data1[9]);
				attributes->alignment=&((m_pElementList+k)->pElementObject->Data1[12]);
				attributes->data_known=&((m_pElementList+k)->pElementObject->Data1[21]);
				return 1;
			}
		}
		return 0;
	}*/
		
	attributes->top_border=&((m_pElementList+k)->pElementObject->Data1[0]);
	attributes->right_border=&((m_pElementList+k)->pElementObject->Data1[1]);
	attributes->bottom_border=&((m_pElementList+k)->pElementObject->Data1[2]);
	attributes->left_border=&((m_pElementList+k)->pElementObject->Data1[3]);
	attributes->alignment=&((m_pElementList+k)->pElementObject->Data1[10]);
	attributes->data_known=&((m_pElementList+k)->pElementObject->Data1[21]);

	/*if (alternative_data==1)
	{
		attributes->top_border=&((m_pElementList+k)->pElementObject->Data1[4]);
		attributes->right_border=&((m_pElementList+k)->pElementObject->Data1[5]);
		attributes->bottom_border=&((m_pElementList+k)->pElementObject->Data1[6]);
		attributes->alignment=&((m_pElementList+k)->pElementObject->Data1[11]);

		attributes->left_border=&((m_pElementList+k)->pElementObject->Data1[1]);
	}
	if (alternative_data==2)
	{
		attributes->right_border=&((m_pElementList+k)->pElementObject->Data1[7]);
		attributes->bottom_border=&((m_pElementList+k)->pElementObject->Data1[8]);
		attributes->left_border=&((m_pElementList+k)->pElementObject->Data1[9]);
		attributes->alignment=&((m_pElementList+k)->pElementObject->Data1[12]);

		attributes->top_border=&((m_pElementList+k)->pElementObject->Data1[2]);
	}*/
	return 1;
}

//this text makes tekst wraping of the text box to the given 'width' (absolute coordinates)
//if rewrap_all is set, then the texti is completely rewraped, otherwise only lines longer than 'width'
int CExpression::AutowrapText(CDC *DC,int width,int rewrap_all)
{
	if (this->IsTextContained(-1)==0) return 0;
	if (this->m_MaxNumColumns>1) return 0;
	if (width<ViewZoom) return 0;

	int startelm=0;

autowraptext_start:
	short l,a,b;
	this->CalculateSize(DC,ViewZoom,&l,&a,&b);
	if ((l<width) && (rewrap_all==0))
	{

		return 0;
	}
	int linestart=startelm;
	int ip_type=1; //text type insertion point
	for (int i=startelm;i<this->m_NumElements;i++)
	{
		tElementStruct *ts=this->m_pElementList+i;
		if (i==linestart) continue; //we never do wraping at the beginning of a line
		
		if ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF) && (ts->X_pos+ts->Length<width) && (rewrap_all) && (i<m_NumElements-1))
		{
			//this line is too short, we will check if new elements can be added from below

			tElementStruct *ts2=this->m_pElementList+i+1;
			int ok_to_wrap=1;
			if ((ts2->Type==1) && (ts2->pElementObject->Data1[1]==0))
			{
				char ch=ts2->pElementObject->Data1[0];
				if ((ch=='-') || (ch=='+') || (ch=='#')) ok_to_wrap=0; //if starts with minus character it can be a list
			}

			if (ts2->Type==2)
			{
				char ch=ts2->pElementObject->Data1[0];
				if ((ch==9) || (ch==(char)0xB7)) ok_to_wrap=0; //if starts with a tab or bullet
			}
			if ((ts2->Type==11) || (ts2->Type==12) || (ts2->Type==0)) ok_to_wrap=0;

			if ((ok_to_wrap) && (ts->X_pos+ts2->Length<width))
			{
				this->DeleteElement(i);
				if (m_IsKeyboardEntry>i) m_IsKeyboardEntry--;
				goto autowraptext_start;
			}
		}		
		if ((ts->Type==2)&& (ts->pElementObject->Data1[0]==(char)0xFF)) {linestart=i;continue;}

		//determining the type of insertion point (math or text - only text can be wraped)
		//if (DetermineInsertionPointType(i)!=1) continue;
		

		//handling spliced text
		int wpos=ts->X_pos+ts->Length;
		if ((ts->Type==1) && (ts->pElementObject->m_Text==4) && (i<m_NumElements-1) && ((ts+1)->Type==1))
		{
			wpos=(ts+1)->X_pos+(ts+1)->Length;
			if ((i<m_NumElements-2) && ((ts+2)->Type==1) && ((ts+2)->pElementObject->m_Text==2))
				wpos=(ts+2)->X_pos+(ts+2)->Length;
		}
		if ((ts->Type==1) && (i<m_NumElements-1) && ((ts+1)->Type==1) && ((ts+1)->pElementObject->m_Text==2))
			wpos=(ts+1)->X_pos+(ts+1)->Length;


		if (wpos>width)
		{
			//this line is too long, we are going to wrap the text before this line

			//first find the beginning of the math section (if there is math section)
			int old=i;
			while (i>0)
			{
				if ((i>1) && ((m_pElementList+i-1)->Type==1) && (m_pElementList+i-1)->pElementObject->m_Text) break;
				if (DetermineInsertionPointType(i)==1)
				{
					if ((m_pElementList+i)->Type==1)
					{
						char ch=(m_pElementList+i)->pElementObject->Data1[0];
						if ((ch==',') || (ch=='.') || (ch==';') || (ch==':') || (ch=='!') || (ch=='?')) {i--;continue;}
					}
					if (((m_pElementList+i)->Type==2) && ((m_pElementList+i)->pElementObject->Data1[0]==(char)0xFF))
						{i++;break;} //we reached beginning of the row (no wraping possible=
					break;
				}
				i--;
			}

			if ((i>0) && (((m_pElementList+i-1)->Type!=2) || ((m_pElementList+i-1)->pElementObject->Data1[0]!=(char)0xFF)))
			{
				//first we check if the line is unwraped until now - if yet unwraped, we adjust alignment
				{
					int j;
					for (j=0;j<m_NumElements;j++)
						if (((m_pElementList+j)->Type==2) && ((m_pElementList+j)->pElementObject->Data1[0]==(char)0xFF))
							break;
					if (j==m_NumElements) this->m_Alignment=1;
				}

				//first we insert the row separator here
				InsertEmptyElement(i,2,(char)0xFF);
				if (m_IsKeyboardEntry>i) m_IsKeyboardEntry++;
				startelm=i+1;
				//then we search until the end of the line and check if we can delete this row separator
				int j;
				for (j=i+1;j<m_NumElements;j++)
				{
					tElementStruct *ts2=this->m_pElementList+j;
					if ((ts2->Type==2) && (ts2->pElementObject->Data1[0]==(char)0xFF)) {;break;}
				}
				if (j<m_NumElements-1)
				{
					tElementStruct *ts2=this->m_pElementList+j+1;
					int ok_to_wrap=1;
					if ((ts2->Type==1) && (ts2->pElementObject->Data1[1]==0))
					{
						char ch=ts2->pElementObject->Data1[0];
						if ((ch<'0')) ok_to_wrap=0;
					}
					if (ts2->Type==2)
					{
						char ch=ts2->pElementObject->Data1[0];
						if ((ch==9) || (ch==(char)0xB7)) ok_to_wrap=0;

					}
					if ((ts2->Type==11) || (ts2->Type==12) || (ts2->Type==0)) ok_to_wrap=0;
					if (ok_to_wrap) 
					{
						this->DeleteElement(j);
						if (m_IsKeyboardEntry>j) m_IsKeyboardEntry--;
					}
				}
				goto autowraptext_start;
			}
			i=old;
		}

		
	}

	//if (m_IsText==0) {m_IsText=1;this->m_Alignment=1;}

	return 1;
}

#pragma optimize("s",on)
int CExpression::XML_output(char * output, int num_tabs, char only_calculate)
{
	int len=0;
	static char tmpstr[136];
	static char tabs[17];

	if (num_tabs>16) num_tabs=16;

	int i;
	memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string

	strcpy(tmpstr,tabs);
	if (XMLFileVersion==1) strcat(tmpstr,"<expr fnt_h=\""); else strcat(tmpstr,"<ex fh=\"");


	char tmp[8];
	itoa(m_FontSize,tmp,10);
	strcat(tmpstr,tmp);
	//if (m_FontSize!=m_FontSizeHQ)
	//{
	//	strcat(tmpstr,"\" fnthq_h=\"");
	//	itoa(m_FontSizeHQ,tmp,10);
	//	strcat(tmpstr,tmp);
	//}
	int old_version_text_decode=0;
	char prevStartAsText=m_StartAsText;
	if (XMLFileVersion==1)
	{
	if ((m_MaxNumRows==1) && (m_MaxNumColumns==1))
	{
		for (int ii=0;ii<m_NumElements;ii++)
			if (((m_pElementList+ii)->Type==2) && ((m_pElementList+ii)->pElementObject->Data1[0]==(char)0xFF)) 
			{
				old_version_text_decode=1;
				m_StartAsText=1;
				strcat(tmpstr,"\" txt=\"1");
				break;
			}
	}
	}
	
	if (m_StartAsText)
	{
		strcat(tmpstr,"\" stxt=\"");
		itoa(prevStartAsText,tmp,10);
		strcat(tmpstr,tmp);
	}
	if (m_IsHeadline)
	{
		strcat(tmpstr,"\" hed=\"");
		itoa(m_IsHeadline,tmp,10);
		strcat(tmpstr,tmp);
	}
	if (m_IsVertical)
	{
		strcat(tmpstr,"\" vert=\"");
		itoa(m_IsVertical,tmp,10);
		strcat(tmpstr,tmp);
	}
	if (m_Color!=-1)
	{
		if (XMLFileVersion==1) strcat(tmpstr,"\" color=\""); else strcat(tmpstr,"\" clr=\"");
		itoa(m_Color,tmp,10);
		strcat(tmpstr,tmp);
	}
	
	if (m_Alignment!=0)
	{
		strcat(tmpstr,"\" alig=\"");
		itoa(m_Alignment,tmp,10);
		strcat(tmpstr,tmp);
	}
	if ((m_ParenthesesFlags&0x1F)!=0)
	{
		if (XMLFileVersion==1) strcat(tmpstr,"\" brack=\""); else strcat(tmpstr,"\" br=\"");
		itoa(m_ParenthesesFlags&0x1F,tmp,10);
		strcat(tmpstr,tmp);
	}
	strcat(tmpstr,"\"");

	if ((m_DrawParentheses) && (m_ParentheseShape!='('))
	{
		if (XMLFileVersion==1) strcat(tmpstr," b_shape=\""); else strcat(tmpstr," shp=\""); 
		if (m_ParentheseShape=='\\')
			sprintf(tmp,"\\5C");
		else
			sprintf(tmp,"%c",m_ParentheseShape);
		strcat(tmpstr,tmp);
		strcat(tmpstr,"\"");
	}
	strcat(tmpstr,">\r\n");
	len+=(int)strlen(tmpstr);
	if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}

	//special handling for backward compatibility with file format 1.x (matrices/tables do not have the last row separator)
	if ((XMLFileVersion==1) && ((m_MaxNumRows>1) || (m_MaxNumColumns>1)) && ((m_pElementList+m_NumElements-1)->Type==12))
	{
		char top,right,bottom,left,align;
		top=(m_pElementList+m_NumElements-1)->pElementObject->Data1[0];
		right=(m_pElementList+m_NumElements-1)->pElementObject->Data1[1];
		bottom=(m_pElementList+m_NumElements-1)->pElementObject->Data1[2];
		left=(m_pElementList+m_NumElements-1)->pElementObject->Data1[3];
		align=(m_pElementList+m_NumElements-1)->pElementObject->Data1[10];
		for (int jj=m_NumElements-2;jj>=0;jj--)
		{
			tElementStruct *ts=m_pElementList+jj;
			if (ts->Type==11) 
			{
				ts->pElementObject->Data1[4]=top;
				ts->pElementObject->Data1[5]=right;
				ts->pElementObject->Data1[6]=bottom;
				ts->pElementObject->Data1[1]=left;
				ts->pElementObject->Data1[11]=align;
				break;
			}
			else if (ts->Type==12)
			{
				ts->pElementObject->Data1[2]=top;
				ts->pElementObject->Data1[7]=right;
				ts->pElementObject->Data1[8]=bottom;
				ts->pElementObject->Data1[9]=right;
				ts->pElementObject->Data1[12]=align;
				break;
			}
		}
	}

	for (i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if (ts->Type==11)
		{
			ts->pElementObject->Data1[20]=0;
			if (strcmp(ts->pElementObject->Data1,"                    "))
				sprintf(tmpstr,"%s<col_sep data=\"%s\" />\r\n",tabs,ts->pElementObject->Data1);
			else
				sprintf(tmpstr,"%s<col_sep />\r\n",tabs);
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
		}
		else if (ts->Type==12)
		{
			if ((XMLFileVersion>1) || (i<m_NumElements-1)) //for file format version 1.x, we never store the last row separator
			{
				ts->pElementObject->Data1[20]=0;
				if (strcmp(ts->pElementObject->Data1,"                    "))
					sprintf(tmpstr,"%s<row_sep data=\"%s\" />\r\n",tabs,ts->pElementObject->Data1);
				else
					sprintf(tmpstr,"%s<row_sep />\r\n",tabs);
				len+=(int)strlen(tmpstr);
				if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
			}
		}
		else if ((old_version_text_decode) && (ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF))
		{
			//for backward compatibility - we are storing simple text boxes this way
			strcpy(tmpstr,tabs);
			if (XMLFileVersion==1) strcat(tmpstr,"<row_sep />\r\n"); else strcat(tmpstr,"<wrap />\r\n");
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
		}
		else if ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF) && (XMLFileVersion>1))
		{
			strcpy(tmpstr,tabs);
			strcat(tmpstr,"<wrap />\r\n");
			len+=(int)strlen(tmpstr);
			if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}
		}
		else if (ts->pElementObject)
		{
			int tt=ts->pElementObject->XML_output(output,num_tabs+1,only_calculate);
			len+=tt;
			if (!only_calculate) output+=tt;
			
		}
	}

	memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string
	strcpy(tmpstr,tabs);
	if (XMLFileVersion==1) strcat(tmpstr,"</expr>\r\n"); else strcat(tmpstr,"</ex>\r\n");
	len+=(int)strlen(tmpstr);
	if (!only_calculate) {strcpy(output,tmpstr);output+=strlen(tmpstr);}

	m_StartAsText=prevStartAsText;
	return len;
}

#pragma optimize("s",on)
char * CExpression::XML_input(char * file)
{
	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;

	file=mf->XML_search("",file);
	if (file==NULL) return NULL;
	if (strncmp(file,"expr",4)==0) file+=4;
	else if (strncmp(file,"ex",2)==0) file+=2;
	else return 0;

	int realStartAsText=0;
	int old_version_text_decode=0;
	static char attribute[64];
	static char value[256];
	do
	{
		file=mf->XML_read_attribute(attribute,value,file,256);
		if (file==NULL) return NULL;
		if ((strcmp(attribute,"fnt_h")==0) || (strcmp(attribute,"fh")==0)) m_FontSize=atoi(value);
		if (strcmp(attribute,"stxt")==0) {realStartAsText=atoi(value);if (old_version_text_decode==0) m_StartAsText=realStartAsText;}
		if (strcmp(attribute,"txt")==0) {m_StartAsText=1;old_version_text_decode=1;}
		if (strcmp(attribute,"hed")==0) m_IsHeadline=atoi(value);
		if (strcmp(attribute,"vert")==0) m_IsVertical=atoi(value);
		if ((strcmp(attribute,"color")==0) || (strcmp(attribute,"clr")==0)) m_Color=atoi(value);
		if (strcmp(attribute,"alig")==0) m_Alignment=atoi(value);
		if ((strcmp(attribute,"brack")==0) || (strcmp(attribute,"br")==0)) m_ParenthesesFlags=atoi(value);
		if ((strcmp(attribute,"b_shape")==0) || (strcmp(attribute,"shp")==0)) m_ParentheseShape=value[0];
		if (strcmp(attribute,"b_horiz")==0) m_ParenthesesFlags|=(atoi(value)<<2);
		if (strcmp(attribute,"b_noleft")==0) m_ParenthesesFlags|=(atoi(value)<<3);
		if (strcmp(attribute,"b_noright")==0) m_ParenthesesFlags|=(atoi(value)<<4);
		if (strcmp(attribute,"b_data")==0) m_ParenthesesFlags|=(atoi(value)<<2);

	} while (attribute[0]);

	int found_anything=0;
	int is_matrix=0;
	while (1)
	{
		file=mf->XML_search("",file);  //search anything
		if (file==NULL) return NULL;
		if (strncmp(file,"/ex",3)==0) //either /expr or /ex tags will close expression
		{
			if (!found_anything) InsertEmptyElement(0,0,0);
			if ((is_matrix) /*&& ((m_MaxNumRows>1) || (m_MaxNumColumns>1))*/)
			{
				if ((XMLFileVersion==1) && ((m_pElementList+m_NumElements-1)->Type!=12))
				{
					//math-o-mir 1.x file format, we must find matrix cell attributes in former type 11/12 element
					char align,top,bottom,left,right;
					align=top=bottom=left=right=' ';
					int jj;
					for (jj=m_NumElements-1;jj>=0;jj--)
					{
						tElementStruct *ts=m_pElementList+jj;
						if (ts->Type==11) 
						{
							top=(ts->pElementObject->Data1[4]);
							right=(ts->pElementObject->Data1[5]);
							bottom=(ts->pElementObject->Data1[6]);
							left=(ts->pElementObject->Data1[1]);
							align=(ts->pElementObject->Data1[11]);
							break;
						}
						else if (ts->Type==12)
						{
							top=(ts->pElementObject->Data1[2]);
							right=(ts->pElementObject->Data1[7]);
							bottom=(ts->pElementObject->Data1[8]);
							left=(ts->pElementObject->Data1[9]);
							align=(ts->pElementObject->Data1[12]);
							break;
						}
					}
					InsertEmptyElement(m_NumElements,12,0);
					(m_pElementList+m_NumElements-1)->pElementObject->Data1[0]=top;
					(m_pElementList+m_NumElements-1)->pElementObject->Data1[1]=right;
					(m_pElementList+m_NumElements-1)->pElementObject->Data1[2]=bottom;
					(m_pElementList+m_NumElements-1)->pElementObject->Data1[3]=left;
					(m_pElementList+m_NumElements-1)->pElementObject->Data1[10]=align;
				}
				AdjustMatrix();
			}
			if (old_version_text_decode) m_StartAsText=realStartAsText;
			if (file[3]=='p') return file+5; else return file+3; //'/expr' or '/ex'
		}
		if (strncmp(file,"col_sep",7)==0) 
		{
			char buff[24];
			char attrib[24];
			file+=7;
			mf->XML_read_attribute(attrib,buff,file,20);
			InsertEmptyElement(m_NumElements,11,0);
			if (strcmp(attrib,"data")==0)
			{
				memcpy((m_pElementList+m_NumElements-1)->pElementObject->Data1,buff,20);
				(m_pElementList+m_NumElements-1)->pElementObject->Data1[21]=0;
			}
			is_matrix=1;
			found_anything=1;
			old_version_text_decode=0;
		}
		if (strncmp(file,"row_sep",7)==0) 
		{
			char buff[24];
			char attrib[24];
			file+=7;
			mf->XML_read_attribute(attrib,buff,file,20);
			if (old_version_text_decode)
			{
				InsertEmptyElement(m_NumElements,2,(char)0xFF);
			}
			else
			{
				InsertEmptyElement(m_NumElements,12,0);
				
				if (strcmp(attrib,"data")==0)
				{
					memcpy((m_pElementList+m_NumElements-1)->pElementObject->Data1,buff,20);
					(m_pElementList+m_NumElements-1)->pElementObject->Data1[21]=0;

				}
				is_matrix=1;
			}
			
			found_anything=1;
		}
		if (strncmp(file,"wrap",4)==0)
		{
			file+=4;
			InsertEmptyElement(m_NumElements,2,(char)0xFF);
			found_anything=1;
		}
		if ((strncmp(file,"elm",3)==0) ||
			(strncmp(file,"var",3)==0) ||
			(strncmp(file,"opr",3)==0) ||
			(strncmp(file,"pwr",3)==0) ||
			(strncmp(file,"fra",3)==0) ||
			(strncmp(file,"fun",3)==0) ||
			(strncmp(file,"bra",3)==0)) // 'elm' or several special elements (variable, operator, exponent, fraction, function, parentheses)  
		{
			char tt=*file; if (tt=='f') tt=*(file+1);
			file+=3;
			if (tt=='e') //'elm' to represents a general element (type 3...12) - we need to read the element type
			{
				file=mf->XML_read_attribute(attribute,value,file,256);
				if (file==NULL) return NULL;
			}
			if ((tt!='e') || (strcmp(attribute,"tp")==0))
			{
				tElementStruct element;
				element.IsSelected=0;
				element.Above=element.Below=element.Length=0;
				element.X_pos=0;
				element.Y_pos=0;
				element.Decoration=0;
				CElementInitPaternalExpression=(CObject*)this;
				if (tt=='v') CElementInitType=1; //variable
				if (tt=='o') CElementInitType=2; //operator
				if (tt=='p') CElementInitType=3; //exponent (power)
				if (tt=='r') CElementInitType=4; //fraction
				if (tt=='u') CElementInitType=6; //function
				if (tt=='b') CElementInitType=5; //bracket
				if (tt=='e') CElementInitType=atoi(value); //other elements (root, symbol, ...)
				element.pElementObject=new CElement();
				file=element.pElementObject->XML_input(file,&element);

				if (file==NULL) 
				{
					delete element.pElementObject;
					return NULL;
				}
				element.Type=element.pElementObject->m_Type;
				//InsertElement(&element,m_NumElements);
				MoveElementInto(&element,m_NumElements);
				//delete element.pElementObject;
			}
			found_anything=1;

		}
	}


	return NULL;
}


//output_type==1 - presenttion mathml
//output_type==2 - content mathml
/*#pragma optimize("s",on)
int CExpression::MathML_output(char * output, int num_tabs, char only_calculate,char output_type)
{
	int len=0;
	static char tmpstr[136];
	static char tabs[17];
	if (num_tabs>16) num_tabs=16;

	memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string

	if ((m_Color>-1) && (m_Color<=4))
	{
		//color of the expression - if m_Color==-1, then the color is inherited or default
		strcpy(tmpstr,tabs);
		strcat(tmpstr,"<mstyle mathcolor=\"");
		strcat(tmpstr,(m_Color==1)?"red":(m_Color==2)?"green":(m_Color==3)?"blue":(m_Color==4)?"gray":"black");
		strcat(tmpstr,"\">\r\n");
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}

	if ((m_DrawParentheses) && ((m_ParentheseData&0x01)==0)) //parentheses
	{
		//handling parentheses (horizontal parentheses not currently supported)
		char tmp1[64];
		tmp1[0]=0;

		strcpy(tmpstr,tabs);
		if (m_ParentheseShape=='(')  {strcpy(tmp1,"<mfenced open=\"(\" close=\")\">\r\n");}
		if (m_ParentheseShape=='[')  {strcpy(tmp1,"<mfenced open=\"[\" close=\"]\">\r\n");}
		if (m_ParentheseShape=='{')  {strcpy(tmp1,"<mfenced open=\"{\" close=\"}\">\r\n");}
		if (m_ParentheseShape=='|')  {strcpy(tmp1,"<mfenced open=\"|\" close=\"|\">\r\n");}
		if (m_ParentheseShape=='/')  {strcpy(tmp1,"<mfenced open=\"/\" close=\"/\">\r\n");}
		if (m_ParentheseShape=='\\') {strcpy(tmp1,"<mfenced open=\"\\\" close=\"\\\">\r\n");}
		if (m_ParentheseShape=='<')  {strcpy(tmp1,"<mfenced open=\"<\" close=\">\">\r\n");}
		if (m_ParentheseShape=='r')  {strcpy(tmp1,"<mfenced open=\"[\" close=\")\">\r\n");}
		if (m_ParentheseShape=='l')  {strcpy(tmp1,"<mfenced open=\"(\" close=\"]\">\r\n");}

		if (m_ParentheseData&0x02) tmp1[15]=' '; //parenthese excluded
		if (m_ParentheseData&0x04) tmp1[25]=' '; //parenthese excluded
		if (m_ParentheseShape=='b')  {strcpy(tmp1,"<menclose notation=\"box\">\r\n");}
		if (m_ParentheseShape=='x')  {strcpy(tmp1,"<menclose notation=\"updiagonalstrike downdiagonalstrike\">\r\n");}
		strcat(tmpstr,tmp1);
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}

	//the <mrow> is always present around an expression
	strcpy(tmpstr,tabs);
	strcat(tmpstr,"<mrow>\r\n");
	{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}


	//the main loop - parsing the expression at the level of matrix
	int pos=0;
	int is_table=0;
	while (1)
	{
		char et,p;
		int l=GetElementLen(pos,m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);

		if ((pos>0) || (l<m_NumElements))
		{
			//handling if there is matrix
			if (pos==0) 
			{
				is_table=1;
				strcpy(tmpstr,tabs);
				if (m_Alignment==1) strcat(tmpstr,"<mtable columnalign=\"left\">\r\n");
				else if (m_Alignment==2) strcat(tmpstr,"<mtable columnalign=\"right\">\r\n");
				else strcat(tmpstr,"<mtable>\r\n");
				strcat(tmpstr,tabs);
				strcat(tmpstr,"<mtr>\r\n");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
			if (et==(char)0xFE)
			{
				strcpy(tmpstr,tabs);
				strcat(tmpstr,"</mtr><mtr>\r\n");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
			
			strcpy(tmpstr,tabs);
			strcat(tmpstr,"<mtd>\r\n");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}

			if (l-p>1)
			{
				strcpy(tmpstr,tabs);
				strcat(tmpstr,"<mrow>\r\n");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
		}

		//going, element-by-element (take care about decoration and clor also)
		int prev_type=2;
		int last_decor=0;
		int last_decor2=0;
		for (int k=pos+p;k<=pos+l-1;k++)
		{
			int curd=(m_pElementList+k)->Decoration;
			if ((m_pElementList+k)->pElementObject) curd+=32*((m_pElementList+k)->pElementObject->m_Color+1);
			if (((m_pElementList+k)->Type==1) && ((m_pElementList+k)->pElementObject->m_Text)) curd+=1024;

			if (curd!=last_decor)
			{
				if (last_decor/1024)
				{
					strcpy(tmpstr,"</mtext>\r\n");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				if ((last_decor&0x3FF)/32)
				{
					strcpy(tmpstr,tabs);
					strcat(tmpstr,"</mstyle>\r\n");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				if ((last_decor%32) && (last_decor2!=(curd&0x1F)))
				{
					strcpy(tmpstr,tabs);
					strcat(tmpstr,"</menclose>\r\n");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}

				if ((curd%32) && (last_decor2!=(curd&0x1F)))
				{
					strcpy(tmpstr,tabs);
					strcat(tmpstr,"<menclose notation=\"");
					int mc=(m_pElementList+k)->Decoration;
					if (mc==1) strcat(tmpstr,"updiagonalstrike");
					if (mc==2) strcat(tmpstr,"circle");
					if (mc==3) strcat(tmpstr,"bottom");
					if (mc==4) strcat(tmpstr,"top");
					strcat(tmpstr,"\">\r\n");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				if ((curd&0x3FF)/32)
				{
					strcpy(tmpstr,tabs);
					int mc=(m_pElementList+k)->pElementObject->m_Color;
					if (mc==0) 	strcat(tmpstr,"<mstyle mathcolor=\"black\">\r\n");
					else if (mc==1) strcat(tmpstr,"<mstyle mathcolor=\"red\">\r\n");
					else if (mc==2) strcat(tmpstr,"<mstyle mathcolor=\"green\">\r\n");
					else if (mc==3) strcat(tmpstr,"<mstyle mathcolor=\"blue\">\r\n");
					else if (mc==4) strcat(tmpstr,"<mstyle mathcolor=\"gray\">\r\n");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}

				if (curd/1024)
				{
					strcpy(tmpstr,tabs);
					strcat(tmpstr,"<mtext>");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				last_decor2=curd&0x1F;
				last_decor=curd;
				
			}
			if ((m_pElementList+k)->pElementObject)
			{
				if (((m_pElementList+k)->Type==1) && ((m_pElementList+k)->pElementObject->m_Text))
				{
					strcpy(tmpstr,(m_pElementList+k)->pElementObject->Data1);
					strcat(tmpstr," ");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				else
				{
					if ((prev_type!=2) && ((m_pElementList+k)->Type!=2))
					{
						strcpy(tmpstr,tabs);
						strcat(tmpstr,"<mo>&it;</mo>\r\n");  //&InvisibleTimes; - added between two elements if there is no other operator
						{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
					}
					
					int tt=((CElement*)((m_pElementList+k)->pElementObject))->MathML_output(output,num_tabs,only_calculate,output_type);
					memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string
					len+=tt;if (!only_calculate) output+=tt;
				}
				prev_type=(m_pElementList+k)->Type;
			}
		}
		if (last_decor/1024)
		{
			strcpy(tmpstr,"</mtext>\r\n");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}
		if ((last_decor&0x3FF)/32)
		{
			strcpy(tmpstr,tabs);
			strcat(tmpstr,"</mstyle>\r\n");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}
		if (last_decor%32)
		{
			strcpy(tmpstr,tabs);
			strcat(tmpstr,"</menclose>\r\n");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}
		if ((pos>0) || (l<m_NumElements))
		{
			if (l-p>1)
			{
				strcpy(tmpstr,tabs);
				strcat(tmpstr,"</mrow>\r\n");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}

			strcpy(tmpstr,tabs);
			strcat(tmpstr,"</mtd>\r\n");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}
		pos+=l;
		if (pos>m_NumElements-1) 
		{
			if (is_table)
			{
				strcpy(tmpstr,tabs);
				strcat(tmpstr,"</mtr>\r\n");
				strcat(tmpstr,tabs);
				strcat(tmpstr,"</mtable>\r\n");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
			break;
		}
	}

	


	strcpy(tmpstr,tabs);
	strcat(tmpstr,"</mrow>\r\n");
	{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}

	if ((m_DrawParentheses) && ((m_ParentheseData&0x01)==0))
	{
		strcpy(tmpstr,tabs);
		if ((m_ParentheseShape=='b') || (m_ParentheseShape=='x')) strcat(tmpstr,"</menclose>\r\n");
		else strcat(tmpstr,"</mfenced>\r\n");
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}

	if ((m_Color>-1) && (m_Color<=4))
	{
		strcpy(tmpstr,tabs);
		strcat(tmpstr,"</mstyle>\r\n");
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}
	
	return len;
}
*/

#pragma optimize("s",on)
int CExpression::LaTeX_output(char * output, char only_calculate)
{
	int len=0;
	static char tmpstr[136];
	char *tabs="";
	char output_type=3;
	int num_tabs=0;
	//static char tabs[17];
	//if (num_tabs>16) num_tabs=16;

	//memset(tabs,9,num_tabs);tabs[num_tabs]=0; //generating the tablist string

	/*if ((m_Color>-1) && (m_Color<=4))
	{
		//color of the expression - if m_Color==-1, then the color is inherited or default
	}*/

	if ((m_DrawParentheses) && ((m_ParenthesesFlags&0x04)==0)) //parentheses (not horizontal)
	{
		//handling parentheses (horizontal parentheses not currently supported)
		char tmp1[48];
		tmp1[0]=0;

		strcpy(tmpstr,tabs);
		if (m_ParenthesesFlags&0x08) strcpy(tmp1,"\\left. ");
		else
		{
			if (m_ParentheseShape=='(')  {strcpy(tmp1,"\\left( ");}
			if (m_ParentheseShape=='[')  {strcpy(tmp1,"\\left[ ");}
			if (m_ParentheseShape=='{')  {strcpy(tmp1,"\\left\\{ ");}
			if (m_ParentheseShape=='|')  {strcpy(tmp1,"\\left| ");}
			if (m_ParentheseShape=='/')  {strcpy(tmp1,"\\left/ ");}
			if (m_ParentheseShape=='\\') {strcpy(tmp1,"\\left| \\left| ");}
			if (m_ParentheseShape=='<')  {strcpy(tmp1,"\\left< ");}
			if (m_ParentheseShape=='r')  {strcpy(tmp1,"\\left[ ");}
			if (m_ParentheseShape=='l')  {strcpy(tmp1,"\\left( ");}
			if (m_ParentheseShape=='a')  {strcpy(tmp1,"\\left< ");}
			if (m_ParentheseShape=='k')  {strcpy(tmp1,"\\left| ");}
			if (m_ParentheseShape=='b')  {strcpy(tmp1,"\\left[ ");} //boxing???
			if (m_ParentheseShape=='l')  {strcpy(tmp1,"\\left[ ");} //strikeout???
		}
		strcat(tmpstr,tmp1);
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}


	//the main loop - parsing the expression at the level of matrix
	int pos=0;
	int is_table=0;
	while (1)
	{
		char et,p;
		int l=GetElementLen(pos,m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);

		if ((pos>0) || (l<m_NumElements))
		{
			//handling if there is matrix
			if (pos==0) 
			{
				is_table=1;
				strcpy(tmpstr,"\\begin{array}{");

				for (int ii=0;ii<this->m_MaxNumColumns;ii++)
				{
					if (m_Alignment==1) strcat(tmpstr,"l");
					else if (m_Alignment==2) strcat(tmpstr,"r");
					else strcat(tmpstr,"c");
				}

				strcat(tmpstr,"}");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
			else if (et==(char)0xFE)
			{
				strcpy(tmpstr,"\\\\\r\n");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
			else
			{
				strcpy(tmpstr," & ");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
		}

		//going, element-by-element (take care about decoration and clor also)
		int prev_type=2;
		int last_decor=0;
		int last_decor2=0;
		for (int k=pos+p;k<=pos+l-1;k++)
		{
			int curd=(m_pElementList+k)->Decoration;
			//if ((m_pElementList+k)->pElementObject) curd+=32*((m_pElementList+k)->pElementObject->m_Color+1);
			if (((m_pElementList+k)->Type==1) && ((m_pElementList+k)->pElementObject->m_Text)) curd+=1024;

			if (curd!=last_decor)
			{
				if (last_decor/1024)
				{
					strcpy(tmpstr,"}");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				if ((last_decor&0x3FF)/32)
				{
					strcpy(tmpstr,"}");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				if ((last_decor%32) && (last_decor2!=(curd&0x1F)))
				{
					strcpy(tmpstr,"}");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}

				if ((curd%32) && (last_decor2!=(curd&0x1F)))
				{
					int mc=(m_pElementList+k)->Decoration;
					if (mc==1) strcpy(tmpstr,"\\underline{"); //strikeout
					if (mc==2) strcpy(tmpstr,"\\underline{");  //encircled
					if (mc==3) strcpy(tmpstr,"\\underline{"); // underline
					if (mc==4) strcpy(tmpstr,"\\overline{");  //overline
					if (mc==5) strcpy(tmpstr,"\\underbrace{");  //overline

					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				/*if ((curd&0x3FF)/32)
				{
					strcpy(tmpstr,tabs);
					int mc=(m_pElementList+k)->pElementObject->m_Color;
					if (mc==0) 	strcat(tmpstr,"<mstyle mathcolor=\"black\">\r\n");
					else if (mc==1) strcat(tmpstr,"<mstyle mathcolor=\"red\">\r\n");
					else if (mc==2) strcat(tmpstr,"<mstyle mathcolor=\"green\">\r\n");
					else if (mc==3) strcat(tmpstr,"<mstyle mathcolor=\"blue\">\r\n");
					else if (mc==4) strcat(tmpstr,"<mstyle mathcolor=\"gray\">\r\n");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}*/

				if (curd/1024)
				{
					strcpy(tmpstr,"\\text{");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				last_decor2=curd&0x1F;
				last_decor=curd;
				
			}
			if ((m_pElementList+k)->pElementObject)
			{
				if (((m_pElementList+k)->Type==1) && ((m_pElementList+k)->pElementObject->m_Text))
				{
					strcpy(tmpstr,(m_pElementList+k)->pElementObject->Data1);
					strcat(tmpstr," ");
					{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
				}
				else
				{
					int tt=((CElement*)((m_pElementList+k)->pElementObject))->LaTeX_output(output,only_calculate);
					len+=tt;if (!only_calculate) output+=tt;
				}
				prev_type=(m_pElementList+k)->Type;
			}
		}
		if (last_decor/1024)
		{
			strcpy(tmpstr,"}");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}
		if ((last_decor&0x3FF)/32)
		{
			strcpy(tmpstr,"}");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}
		if (last_decor%32)
		{
			strcpy(tmpstr,"}");
			{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
		}

		pos+=l;
		if (pos>m_NumElements-1) 
		{
			if (is_table)
			{
				strcpy(tmpstr,"\\end{array} ");
				{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
			}
			break;
		}
	}

	
	if ((m_DrawParentheses) && ((m_ParenthesesFlags&0x04)==0)) //parentheses (not horizontal)
	{
		//handling parentheses (horizontal parentheses not currently supported)
		char tmp1[48];
		tmp1[0]=0;

		strcpy(tmpstr,tabs);
		if (m_ParenthesesFlags&0x10) strcpy(tmp1,"\\right. ");
		else
		{
			if (m_ParentheseShape=='(')  {strcpy(tmp1,"\\right) ");}
			if (m_ParentheseShape=='[')  {strcpy(tmp1,"\\right] ");}
			if (m_ParentheseShape=='{')  {strcpy(tmp1,"\\right\\} ");}
			if (m_ParentheseShape=='|')  {strcpy(tmp1,"\\right| ");}
			if (m_ParentheseShape=='/')  {strcpy(tmp1,"\\right/ ");}
			if (m_ParentheseShape=='\\') {strcpy(tmp1,"\\right| \\right| ");}
			if (m_ParentheseShape=='<')  {strcpy(tmp1,"\\right> ");}
			if (m_ParentheseShape=='r')  {strcpy(tmp1,"\\right) ");}
			if (m_ParentheseShape=='l')  {strcpy(tmp1,"\\right] ");}
			if (m_ParentheseShape=='a')  {strcpy(tmp1,"\\right| ");}
			if (m_ParentheseShape=='k')  {strcpy(tmp1,"\\right> ");}
			if (m_ParentheseShape=='b')  {strcpy(tmp1,"\\right] ");} //boxing???
			if (m_ParentheseShape=='l')  {strcpy(tmp1,"\\right] ");} //strikeout???
		}
		strcat(tmpstr,tmp1);
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}

	/*if ((m_Color>-1) && (m_Color<=4))
	{
		strcpy(tmpstr,tabs);
		strcat(tmpstr,"</mstyle>\r\n");
		{int tt=(int)strlen(tmpstr);len+=tt;if (!only_calculate) {strcpy(output,tmpstr);output+=tt;}}
	}*/
	
	return len;
}

// tests if the given character is high or not (heght above baseline)
//THIS FUNCTION MUST BE VERY FAST;
#pragma optimize("",on)
int IsCharacterHigh(char ch, char font)
{
	if ((ch<'a') && (ch>0))
	{
		if ((ch!='.') && (ch!=',') && (ch!='_') && (ch!='-') && (ch!='+') && (ch!=';') && (ch!=':') &&
			(ch!='*') && (ch!='<') && (ch!='>') && (ch!='=') && (ch!=' '))
			return 1; 
	}
	else if ((ch<='z') && (ch>0))
	{
		if ((font&0xE0)==0x60) //greek font
		{
			if ((ch=='b') || (ch=='d') || (ch=='f') || (ch=='l') ||
				(ch=='q') || (ch=='x') || (ch=='z')) return 1;
		}
		else
		{
			if ((ch=='b') || (ch=='d') || (ch=='f') || (ch=='h') ||
				(ch=='i') || (ch=='j') || (ch=='k') || (ch=='l') ||
				(ch=='t')) return 1;
		}
	}
	else
	{
		if ((font&0xE0)==0x60)
		{
			if ((ch==(char)0xA3) || (ch==(char)0xA5) || (ch==(char)0xAB) || (ch==(char)0xAC) ||
				(ch==(char)0xAE) || (ch==(char)0xB1) || (ch==(char)0xB3) || (ch==(char)0xB4) ||
				(ch==(char)0xB5) || (ch==(char)0xB7) || (ch==(char)0xB8) || (ch==(char)0xB9) ||
				(ch==(char)0xBA) || (ch==(char)0xBB) || (ch==(char)0xBC) || (ch==(char)0xC4) ||
				(ch==(char)0xC5) || (ch==(char)0xCE) || (ch==(char)0xCF) || (ch==(char)0xD7) ||
				(ch==(char)0xDB) || (ch==(char)0xDC) || (ch==(char)0xDE)) return 0;
		}
		return 1;
	}

	return 0;
}

// tests if the given character goes below the font baseline
//THIS FUNCTION MUST BE VERY FAST;
int IsCharacterLow(char ch,char font)
{
	if ((ch<'a') && (ch>0))
	{
		if ((ch==',') || (ch==';') || ((ch=='V') && ((font&0xE0)==0x60))) return 1;
	}
	else if ((ch<='z') && (ch>0))
	{
		if ((font&0xE0)==0x60) //greek font
		{
			if ((ch=='r') || (ch=='z') || (ch=='f') || (ch=='g') ||
				(ch=='h') || (ch=='j') || (ch=='y') || (ch=='x') ||
				(ch=='c') || (ch=='b') || (ch=='m')) return 1;
		}
		else
		{
			if ((ch=='g') || (ch=='j') || (ch=='q') || (ch=='y')) return 1;
		}
	}
	else
		if (((font&0xE0)==0x60) && ((ch==(char)0xA6) || (ch==(char)0xC3))) return 1;

	return 0;
}

int CExpression::CalcChecksum()
{
	if ((m_NumElements==1) && (m_pElementList->Type==5)) //special () =()	
		return ((CExpression*)(m_pElementList->pElementObject->Expression1))->CalcChecksum();
		
	int c=m_NumElements;
	c+=8*(m_ParenthesesFlags&0x1F);
	c+=16*m_ParentheseShape;
	//c+=32*m_ParentheseData;

	if (!CalcStructuralChecksumOnly) 
	{
		c+=64*(m_FontSize/*+m_FontSizeHQ*/);
		//c+=128*m_ParentheseHeightFactor;
		c+=256*(m_Color+4*m_StartAsText+8*m_IsVertical);
		c+=768*m_Alignment;
		if (m_pPaternalExpression) c+=500;
		c+=m_IsKeyboardEntry+m_KeyboardCursorPos;
		c+=m_InternalInsertionPoint;
	}

	//loop partily unwinded for speed
	if (m_NumElements)
	{
		int i;
		tElementStruct *ts=m_pElementList;
		for (i=1;i<=m_NumElements;i++,ts++)
		{
			if (ts->pElementObject) c+=(ts->Decoration+ts->pElementObject->CalcChecksum())^i;
			else c+=(ts->Decoration)^i;
		}				
	}

	return c;
}


//this function determines whether the insertion point at given position is of math or text type
int CExpression::DetermineInsertionPointType(int position)
{
	int retval=0;
	if (position>m_NumElements) position=m_NumElements;
	if (position<0) position=0;
	if (KeyboardEntryBaseObject==NULL) 
		m_ModeDefinedAt=0;
	if ((position<m_NumElements) && (this==(CExpression*)KeyboardEntryObject) && (position==m_IsKeyboardEntry-1) &&
		((this->m_pElementList+position)->Type==1) && ((this->m_pElementList+position)->pElementObject->Data1[0]=='\\')) return 1; //command entry is always uniform
 
	int defposss=(m_ModeDefinedAt&0x3FFF)-1;  //position at which the mode is explicitly defined (by m_ModeDefinedAt variable)
	if ((defposss==position)) return (char)(m_ModeDefinedAt>>14);
	if ((defposss<position) && (defposss>=0)) 
	{
		int i;
		for (i=min(position,m_NumElements-1);i>=defposss;i--)
			if ((m_pElementList+i)->Type==12) break;
		if (i<defposss) return (char)(m_ModeDefinedAt>>14);
	}

	if ((position==0) && (position<m_NumElements))
		if ((m_pElementList)->Type==1) return (m_pElementList)->pElementObject->m_Text;

	while (position>0)
	{
		tElementStruct *ts=m_pElementList+position-1;

		if (defposss==position) return (char)(m_ModeDefinedAt>>14);
		if ((ts->Type==12) || (ts->Type==11))
		{
			if (position<m_NumElements) 
			{
				if ((ts+1)->Type==1) return (ts+1)->pElementObject->m_Text;
			}
			return this->m_StartAsText;
		}

		int irrelevant_object=0;
		if ((ts->Type==2) && (ts->pElementObject->Data1[0]==9)) irrelevant_object=1;
		if ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xB7)) irrelevant_object=1; //bullet
		if ((ts->Type==2) && (ts->pElementObject->Data1[0]==(char)0xFF)) irrelevant_object=1;
		if ((ts->Type!=1) && (defposss!=-1) && (defposss<position)) irrelevant_object=1;

		if (ts->pElementObject)
		{
			if (!irrelevant_object)
			{
				//we found an relevant object, determine its type and the type of the object to the right of it
				int type1=0;
				int type2=0;
				if (position>0)
				{
					if ((m_pElementList+position-1)->Type==1) 
						type1=(m_pElementList+position-1)->pElementObject->m_Text;
					else 
						type1=0;
				}
				if (position<m_NumElements)
				{
					if ((m_pElementList+position)->Type==1) 
						type2=(m_pElementList+position)->pElementObject->m_Text;
					else 
						type2=0;
				}
				if (position==0) type1=type2;
				if (position==m_NumElements) type2=type1;
				if ((type1) || (type2)) return 1; else return 0;
			}
		}
		position--;
	}

	return m_StartAsText;
}

CObject *CExpression::KeyboardFindEntryPos()
{
	//searches for an expression that has m_IsKeyboardEntry flag set

	if (m_IsKeyboardEntry) return (CObject*)this;
	int i;
	for (i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if (ts->pElementObject)
		{
			CObject *ret=NULL;
			if (ts->pElementObject->Expression1) ret=((CExpression*)(ts->pElementObject->Expression1))->KeyboardFindEntryPos();
			if (ret) return ret;
			if (ts->pElementObject->Expression2) ret=((CExpression*)(ts->pElementObject->Expression2))->KeyboardFindEntryPos();
			if (ret) return ret;
			if (ts->pElementObject->Expression3) ret=((CExpression*)(ts->pElementObject->Expression3))->KeyboardFindEntryPos();
			if (ret) return ret;
		}
	}
	return NULL;
}


//inserts new equation just below the current expression
#pragma optimize("s",on)
int CExpression::KeyboardInsertNewEquation(CDC *DC, short zoom, UINT nChar, CExpression*orig, int TypingMode)
{
	//find the root parent of this equation, and then check if it equals to edited object
	CExpression *parent=this;
	while (parent->m_pPaternalExpression) parent=parent->m_pPaternalExpression;	
	if ((!KeyboardEntryBaseObject) || (KeyboardEntryBaseObject->Object!=(CObject*)parent)) 
		return 0; //strage error



	//turn off keyboard entry mode for this expression
	m_IsKeyboardEntry=0;

	//create the new element in the main document
	tDocumentStruct *ds;
	tDocumentStruct *org_ds=KeyboardEntryBaseObject;

	short l,a,b;
	parent->CalculateSize(DC,100,&l,&a,&b);
	int zed=parent->m_FontSize;

	/*if (!parent->m_IsText)
	{
		zed-=parent->m_FontSize/6;
	}*/

	int delta, delta2;

	if (IsShowGrid)
	{
		delta=b+(zed)/4;
		delta2=b+(zed)/4;

		delta=(delta/GRID+1)*GRID;
		delta2=(delta2/GRID+1)*GRID;
	}
	else
	{
		delta=b+(zed)/4;
		delta2=b+(zed)/4;
	}
	
	int fs=((CExpression*)(org_ds->Object))->m_FontSize;if ((((CExpression*)(org_ds->Object))->m_IsHeadline) || (((CExpression*)(org_ds->Object))->m_FontSize>170)) fs=DefaultFontSize;
	//int fsh=((CExpression*)(org_ds->Object))->m_FontSizeHQ;if ((((CExpression*)(org_ds->Object))->m_IsHeadline) || (((CExpression*)(org_ds->Object))->m_FontSize>170)) fsh=DefaultFontSize;
	int clr=((CExpression*)(org_ds->Object))->m_Color;
	int istxt=((CExpression*)(org_ds->Object))->m_StartAsText;
	int algn=((CExpression*)(org_ds->Object))->m_Alignment;

	if ((this->m_NumElements==1) && 
		(this->m_pPaternalExpression==NULL) &&
		((this->m_pElementList->Type==0) || ((this->m_pElementList->Type==1) && (this->m_pElementList->pElementObject->Data1[0]==0))))
	{
		ds=org_ds;
		ds->absolute_Y+=delta;
		((CExpression*)(ds->Object))->Delete();
	}
	else
	{
		int i;
		for (i=0;i<NumDocumentElements;i++)
			if (TheDocument+i==KeyboardEntryBaseObject) break;
		AddDocumentObject(1,org_ds->absolute_X,org_ds->absolute_Y+delta);
		KeyboardEntryBaseObject=org_ds=TheDocument+i;
		ds=TheDocument+NumDocumentElements-1;
		ds->Object=(CObject*)new CExpression(NULL,NULL,fs);
		((CExpression*)(org_ds->Object))->CalculateSize(DC,zoom,&l,&a,&b);
		org_ds->Above=(short)((int)a*100/(int)ViewZoom);
		org_ds->Below=(short)((int)b*100/(int)ViewZoom); 
		org_ds->Length=(short)((int)l*100/(int)ViewZoom);
	}

	((CExpression*)(ds->Object))->m_StartAsText=0;
	if (orig)
	{
		((CExpression*)(ds->Object))->m_StartAsText=(char)istxt; //if the object was non-empty then inherit paragraph formatting

		for (int i=0;i<orig->m_NumElements;i++)
			((CExpression*)(ds->Object))->InsertElement(orig->m_pElementList+i,i);
	}
	//((CExpression*)(ds->Object))->m_FontSizeHQ=fsh;
	((CExpression*)(ds->Object))->m_Color=(char)clr;
	((CExpression*)(ds->Object))->m_Alignment=(char)algn;
	((CExpression*)(ds->Object))->InsertEmptyElement(0,1,0,Toolbox->GetFormattingColor());
	((CExpression*)(ds->Object))->m_pElementList->pElementObject->m_Text=(char)TypingMode;
	((CExpression*)(ds->Object))->m_ModeDefinedAt=1+(TypingMode<<14);
	((CExpression*)(ds->Object))->CalculateSize(DC,ViewZoom,&l,&a,&b);
	ds->Length=org_ds->Length*6/5; //temporary - needed when calling RearangeObjects
	ds->Above=(short)((int)a*100/(int)ViewZoom);
	ds->Below=(short)((int)b*100/(int)ViewZoom);  
	ds->MovingDotState=(char)0x80;

	((CMainFrame*)(theApp.m_pMainWnd))->RearangeObjects(delta/*ds->Below+ds->Above+fs/10*/);

	parent->CalculateSize(DC,ViewZoom,&l,&a,&b);

	ds->Length=(short)((int)l*100/(int)ViewZoom);
	ds->Below=0x7FFF;  //this will force recalcuation and repainting
	
	KeyboardEntryObject=ds->Object;
	KeyboardEntryBaseObject=ds;
	((CExpression*)(ds->Object))->m_IsKeyboardEntry=1;
	((CExpression*)(ds->Object))->m_KeyboardCursorPos=0;
	


	if ((ds->absolute_X+ds->Length+20)>ViewMaxX) ViewMaxX=ds->absolute_X+ds->Length+20;
	if ((ds->absolute_Y+ds->Below+PaperHeight)>ViewMaxY) ViewMaxY=ds->absolute_Y+ds->Below+PaperHeight;

	if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();

	return 1;
}
#pragma optimize("",on)
//copies expression data into windows clipboard
//should be fast
int CExpression::CopyToWindowsClipboard(void)
{
#ifdef TEACHER_VERSION
	if (TheFileType=='r') return 0;
#endif
	if (theApp.m_pMainWnd->OpenClipboard())
	{
		char tmp[64];
		int len=XML_output(tmp,0,1);
		if ((len>0) && (len<1000000))
		{
			EmptyClipboard();
			HANDLE hmem=GlobalAlloc(GMEM_ZEROINIT,len+256);
			LPVOID pntr=GlobalLock(hmem);
			int *control=(int*)pntr;
			int *checksum=control+1;
			char *data=(char*)(checksum+1);
			*control=0xAABBCC11;
			LastTakenChecksum=*checksum=CalcChecksum()+rand();

			XML_output(data,0,0);
			GlobalUnlock(hmem);
			UINT format=RegisterClipboardFormat("MATHOMIR_EXPR");
			HANDLE ret=SetClipboardData(format,hmem);
			CloseClipboard();
		}
	}
	return 0;
}

#pragma optimize("s",on)
int CExpression::PaintHorizontalParentheses(CDC * DC, short zoom, short X1, short Y1, short X2, short Y2, short ParentheseWidth, char Type,short data,char IsBlue,int color)
{
	char HQR=IsHighQualityRendering;
	//if (((Type&0x80)==0) && (HQR)) ParentheseWidth=11*ParentheseWidth/8;
	Type=Type&0x7F;

	int PaintTop=1;
	int PaintBottom=1;
	if (data&0x08) PaintTop=0;
	if (data&0x10) PaintBottom=0;

	short tmp;
	if (X2<X1) {tmp=X2;X2=X1;X1=tmp;}
	if (Y2<Y1) {tmp=Y2;Y2=Y1;Y1=tmp;}
	short height3=(Y2-Y1)/3;
	short height2=(Y2-Y1)/2;
	int ActualSize=GetActualFontSize(zoom);

	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;
	int PenWidth=ParentheseWidth/3;if (PenWidth<1) PenWidth=1;
	HPEN pen=GetPenFromPool(PenWidth,IsBlue,color);
	DC->SelectObject(pen);

	if ((Type=='(') || (Type=='r') || (Type=='l'))
	{
		ActualSize=3*ActualSize/2;
		if ((X2-X1)<ActualSize) ActualSize=(X2-X1);
		if (!IsHighQualityRendering)
		{
			if ((PaintTop) && ((Type=='(') || (Type=='l')))
			{
			DC->Arc(X1,Y1,X1+ActualSize,Y1+ParentheseWidth*2,  X1+ActualSize/2,Y1,X1,Y1+ParentheseWidth);
			DC->MoveTo(X1+ActualSize/2,Y1);
			DC->LineTo(X2-ActualSize/2,Y1);
			DC->Arc(X2-ActualSize,Y1,X2,Y1+ParentheseWidth*2,  X2,Y1+ParentheseWidth,X2-ActualSize/2,Y1);
			}
			if ((PaintBottom) && ((Type=='(') || (Type=='r')))
			{
			DC->Arc(X1,Y2-ParentheseWidth*2,X1+ActualSize,Y2,  X1,Y2-ParentheseWidth,X1+ActualSize/2,Y2);
			DC->MoveTo(X1+ActualSize/2,Y2);
			DC->LineTo(X2-ActualSize/2,Y2);
			DC->Arc(X2-ActualSize,Y2-ParentheseWidth*2,X2,Y2,  X2-ActualSize/2,Y2,X2,Y2-ParentheseWidth);
			}
			if (Type=='(') return 1;
		}
		else
		{
			int PenWidth=ParentheseWidth;
			int H=(X2-X1)*4;
			int i;

			//if ((PaintTop) && ((Type=='(') || (Type=='l')))
			{
				mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
				for (i=0;i<=PenWidth;i++)
				{
					mf->MyArc(DC,0,i,ActualSize*4,ParentheseWidth*8-i,   ActualSize*2,0,0,ParentheseWidth*4,IsBlue);
					mf->MyMoveTo(DC,ActualSize*2,i);
					mf->MyLineTo(DC,H-ActualSize*2,i,IsBlue);
					mf->MyArc(DC,H-ActualSize*4,i,H,ParentheseWidth*8-i, H,ParentheseWidth*4,H-ActualSize*2,0,IsBlue);
				}
				if ((PaintTop) && ((Type=='(') || (Type=='l')))mf->EndMyPainting(DC,X1,Y1);
				if ((PaintBottom) && ((Type=='(') || (Type=='r'))) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth,0,2);
				/*mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
				for (i=0;i<=PenWidth;i++)
				{
					mf->MyArc(DC,0,i-ParentheseWidth*4,ActualSize*4,ParentheseWidth*4-i,  0,0,ActualSize*2,ParentheseWidth*4,IsBlue);
					mf->MyMoveTo(DC,ActualSize*2,ParentheseWidth*4-i);
					mf->MyLineTo(DC,H-ActualSize*2,ParentheseWidth*4-i,IsBlue);
					mf->MyArc(DC,H-ActualSize*4,i-ParentheseWidth*4,H,ParentheseWidth*4-i, H-ActualSize*2,ParentheseWidth*4,H,0,IsBlue);
				}
				mf->EndMyPainting(DC,X1,Y2-ParentheseWidth);*/
			}
		}
	}

	if ((Type=='[') || (Type=='r') || (Type=='l'))
	{
		//if (!IsHighQualityRendering)
		{
			if ((PaintTop) && ((Type=='[') || (Type=='r')))
			{
				DC->MoveTo(X1+PenWidth/2,Y1+ParentheseWidth);
				DC->LineTo(X1+PenWidth/2,Y1+PenWidth);
				DC->LineTo(X2-PenWidth/2,Y1+PenWidth);
				DC->LineTo(X2-PenWidth/2,Y1+ParentheseWidth);
			}
			if ((PaintBottom) && ((Type=='[') || (Type=='l')))
			{
				DC->MoveTo(X1+PenWidth/2,Y2-ParentheseWidth);
				DC->LineTo(X1+PenWidth/2,Y2-PenWidth);
				DC->LineTo(X2-PenWidth/2,Y2-PenWidth);
				DC->LineTo(X2-PenWidth/2,Y2-ParentheseWidth);
			}
			return 1;
		}
		/*else
		{
			int PenWidth=ParentheseWidth;
			int H=(X2-X1)*4;
			//if ((PaintTop) && ((Type=='[') || (Type=='r')))
			{
				mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
				for (int i=0;i<=PenWidth;i++)
				{
					POINT points[6];
					points[0].x=i;  points[0].y=ParentheseWidth*4;
					points[1].x=i;		points[1].y=i+PenWidth/2;
					points[2].x=H-i;		points[2].y=i+PenWidth/2;
					points[3].x=H-i;  points[3].y=ParentheseWidth*4;
					mf->MyPolyline(DC,points,4,1,IsBlue);
				}
				if ((PaintTop) && ((Type=='[') || (Type=='r')))mf->EndMyPainting(DC,X1,Y1);
				if ((PaintBottom) && ((Type=='[') || (Type=='l'))) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth,0,2);
			}
		}*/
	}

	if (Type=='{')
	{
		ActualSize=2*ActualSize;
		if (3*(X2-X1)/2<ActualSize) ActualSize=3*(X2-X1)/2;
		height2=(X2-X1)/2;
		if (!IsHighQualityRendering)
		{
			HPEN pen=GetPenFromPool(max(7*PenWidth/8,1),IsBlue,color);
			DC->SelectObject(pen);

			if (PaintTop)
			{
			DC->MoveTo(X1,Y1+ParentheseWidth);
			DC->LineTo(X1+ParentheseWidth/2,Y1+ParentheseWidth/2);
			DC->LineTo(X1+height2-ParentheseWidth/2,Y1+ParentheseWidth/2);
			DC->LineTo(X1+height2,Y1);
			DC->LineTo(X2-height2+ParentheseWidth/2,Y1+ParentheseWidth/2);
			DC->LineTo(X2-ParentheseWidth/2,Y1+ParentheseWidth/2);
			DC->LineTo(X2,Y1+ParentheseWidth);
			}

			if (PaintBottom)
			{
			DC->MoveTo(X1,Y2-ParentheseWidth);
			DC->LineTo(X1+ParentheseWidth/2,Y2-ParentheseWidth/2);
			DC->LineTo(X1+height2-ParentheseWidth/2,Y2-ParentheseWidth/2);
			DC->LineTo(X1+height2,Y2);
			DC->LineTo(X2-height2+ParentheseWidth/2,Y2-ParentheseWidth/2);
			DC->LineTo(X2-ParentheseWidth/2,Y2-ParentheseWidth/2);
			DC->LineTo(X2,Y2-ParentheseWidth);
			}
			return 1;
		}
		int PenWidth=3*ParentheseWidth/4;
		int H=(X2-X1)*4;
		int i;
		{
			mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
			for (i=0;i<=PenWidth;i++)
			{
				mf->MyArc(DC,0,ParentheseWidth*2-PenWidth/2+i, 
						4*ActualSize/3,3*ParentheseWidth*2+PenWidth/2-i,  					
						4*ActualSize/6,ParentheseWidth*2,
						0,ParentheseWidth*4,IsBlue);
				mf->MyMoveTo(DC,4*ActualSize/6-1,ParentheseWidth*2-PenWidth/2+i);
				mf->MyLineTo(DC,H/2-4*ActualSize/6,ParentheseWidth*2-PenWidth/2+i,IsBlue);
				mf->MyArc(DC,H/2-4*ActualSize/3,-ParentheseWidth*2+PenWidth/2-i,
						H/2,ParentheseWidth*2-PenWidth/2+i,  					 
						H/2-4*ActualSize/6,ParentheseWidth*2,
						H/2,0, IsBlue);

				mf->MyArc(DC,H/2,-ParentheseWidth*2+PenWidth/2-i,
						H/2+4*ActualSize/3,ParentheseWidth*2-PenWidth/2+i,  					
						H/2,0,
						H/2+4*ActualSize/6,ParentheseWidth*2,  IsBlue);
				mf->MyMoveTo(DC,H/2+4*ActualSize/6,ParentheseWidth*2-PenWidth/2+i);
				mf->MyLineTo(DC,H-4*ActualSize/6,ParentheseWidth*2-PenWidth/2+i,IsBlue);
				mf->MyArc(DC,H-4*ActualSize/3, ParentheseWidth*2-PenWidth/2+i,
						H,3*ParentheseWidth*2+PenWidth/2-i,					 
						H,ParentheseWidth*4,
						H-4*ActualSize/6,ParentheseWidth*2,  IsBlue);
			}
			if (PaintTop) mf->EndMyPainting(DC,X1,Y1);
			if (PaintBottom) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth,0,2);
			/*mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
			for (i=0;i<=PenWidth;i++)
			{
				mf->MyArc(DC,0,ParentheseWidth*2+PenWidth/2-i,
						4*ActualSize/3,-ParentheseWidth*2-PenWidth/2+i,  
						0,0,		
						4*ActualSize/6,ParentheseWidth*2,IsBlue);
				mf->MyMoveTo(DC,4*ActualSize/6-1,ParentheseWidth*2+PenWidth/2-i);
				mf->MyLineTo(DC,H/2-4*ActualSize/6,ParentheseWidth*2+PenWidth/2-i,IsBlue);
				mf->MyArc(DC,H/2-4*ActualSize/3,3*ParentheseWidth*2-PenWidth/2+i,
						H/2,ParentheseWidth*2+PenWidth/2-i,   
						H/2,ParentheseWidth*4,
						H/2-4*ActualSize/6, ParentheseWidth*2, IsBlue);

				mf->MyArc(DC,H/2,3*ParentheseWidth*2-PenWidth/2+i,
						H/2+4*ActualSize/3,ParentheseWidth*2+PenWidth/2-i,  					 
						H/2+4*ActualSize/6,ParentheseWidth*2,
						H/2,ParentheseWidth*4, IsBlue);
				mf->MyMoveTo(DC,H/2+4*ActualSize/6,ParentheseWidth*2+PenWidth/2-i);
				mf->MyLineTo(DC,H-4*ActualSize/6,ParentheseWidth*2+PenWidth/2-i,IsBlue);
				mf->MyArc(DC,H-4*ActualSize/3,ParentheseWidth*2+PenWidth/2-i, 
						H,-ParentheseWidth*2-PenWidth/2+i,
						H-4*ActualSize/6, ParentheseWidth*2,
						H,0, IsBlue);
			}
			mf->EndMyPainting(DC,X1,Y2-ParentheseWidth);*/
		}
	}

	if ((Type=='|') || (Type=='a') || (Type=='k'))
	{
		//if (!IsHighQualityRendering)
		{
			if ((PaintTop) && (Type!='a'))
			{
			DC->MoveTo(X1,Y1+ParentheseWidth/2);
			DC->LineTo(X2,Y1+ParentheseWidth/2);
			}

			if ((PaintBottom) && (Type!='k'))
			{
			DC->MoveTo(X1,Y2-ParentheseWidth/2);
			DC->LineTo(X2,Y2-ParentheseWidth/2);
			}
			if (Type=='|') return 1;
		}

		/*int PenWidth=6*ParentheseWidth/5;
		int H=(X2-X1)*4;
		POINT points[6];

		//if (PaintTop)
		{
			mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
			points[0].x=0;  points[0].y=ParentheseWidth*2;
			points[1].x=H;  points[1].y=ParentheseWidth*2;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			if (PaintTop) mf->EndMyPainting(DC,X1,Y1);
			if (PaintBottom) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth,0,2);
		}*/
	}
	
	if (Type=='/')
	{
		//if (!IsHighQualityRendering)
		{
			if (PaintTop)
			{
			DC->MoveTo(X1,Y1+9*ParentheseWidth/10);
			DC->LineTo(X2,Y1);
			}

			if (PaintBottom)
			{
			DC->MoveTo(X1,Y2);
			DC->LineTo(X2,Y2-9*ParentheseWidth/10);
			}

			return 1;
		}
		/*int PenWidth=ParentheseWidth;
		int H=(X2-X1)*4;
		POINT points[6];

		//if (PaintTop)
		{
			mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
			points[0].x=0;  points[0].y=-PenWidth+ParentheseWidth*4;
			points[1].x=H;  points[1].y=PenWidth;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			if (PaintTop) mf->EndMyPainting(DC,X1,Y1);
			if (PaintBottom) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth);
			
		}*/
	}


	if (Type=='\\') // absolute value
	{
		//if (!IsHighQualityRendering)
		{
			HPEN pen=GetPenFromPool(max(2*PenWidth/3,1),IsBlue,color);
			DC->SelectObject(pen);
			if (PaintTop)
			{
			DC->MoveTo(X1,Y1+ParentheseWidth/4);
			DC->LineTo(X2,Y1+ParentheseWidth/4);
			DC->MoveTo(X1,Y1+3*ParentheseWidth/4);
			DC->LineTo(X2,Y1+3*ParentheseWidth/4);
			}

			if (PaintBottom)
			{
			DC->MoveTo(X1,Y2-ParentheseWidth/4);
			DC->LineTo(X2,Y2-ParentheseWidth/4);
			DC->MoveTo(X1,Y2-3*ParentheseWidth/4);
			DC->LineTo(X2,Y2-3*ParentheseWidth/4);
			}
			return 1;
		}

		/*int PenWidth=ParentheseWidth;
		int H=(X2-X1)*4;
		POINT points[6];

		//if (PaintTop)
		{
			mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
			points[0].x=0;  points[0].y=ParentheseWidth;
			points[1].x=H;  points[1].y=ParentheseWidth;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			points[0].x=0;  points[0].y=3*ParentheseWidth;
			points[1].x=H;  points[1].y=3*ParentheseWidth;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			if (PaintTop) mf->EndMyPainting(DC,X1,Y1);
			if (PaintBottom) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth,0,2);
		}*/

	}

	if ((Type=='<') || (Type=='a') || (Type=='k'))
	{
		//if (!IsHighQualityRendering)
		{
			if ((PaintTop) && (Type!='k'))
			{
			DC->MoveTo(X1,Y1+9*ParentheseWidth/10);
			DC->LineTo((X1+X2)/2,Y1+ParentheseWidth/10);
			DC->LineTo(X2,Y1+9*ParentheseWidth/10);
			}

			if ((PaintBottom) && (Type!='a'))
			{
			DC->MoveTo(X1,Y2-9*ParentheseWidth/10);
			DC->LineTo((X1+X2)/2,Y2-ParentheseWidth/10);
			DC->LineTo(X2,Y2-9*ParentheseWidth/10);
			}
			return 1;
		}
		/*int PenWidth=ParentheseWidth;
		int H=(X2-X1)*4;
		POINT points[6];

		//if (PaintTop)
		{
			mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
			points[0].x=0;  points[0].y=-PenWidth+ParentheseWidth*4;
			points[1].x=H/2;  points[1].y=PenWidth;
			points[2].x=H;  points[2].y=-PenWidth+ParentheseWidth*4;
			mf->MyPolyline(DC,points,3,PenWidth,IsBlue);
			if (PaintTop) mf->EndMyPainting(DC,X1,Y1);
			if (PaintBottom) mf->EndMyPainting(DC,X1,Y2-ParentheseWidth,0,2);

		}*/
	}

	if (Type=='b')  //box
	{
		//if (!IsHighQualityRendering)
		{
			if ((PaintTop) && (PaintBottom))
			{
			DC->MoveTo(X1+ParentheseWidth/10,Y1+ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y2-ParentheseWidth/10);
			DC->LineTo(X1+ParentheseWidth/10,Y2-ParentheseWidth/10);
			DC->LineTo(X1+ParentheseWidth/10,Y1+ParentheseWidth/10);
			}
			else if (PaintTop)
			{
			DC->MoveTo(X1+ParentheseWidth/10,Y1+ParentheseWidth+ActualSize);
			DC->LineTo(X1+ParentheseWidth/10,Y1+ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth+ActualSize);
			} 
			else if (PaintBottom)
			{
			DC->MoveTo(X1+ParentheseWidth/10,Y2-ParentheseWidth-ActualSize);
			DC->LineTo(X1+ParentheseWidth/10,Y2-ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y2-ParentheseWidth/10);
			DC->LineTo(X2-ParentheseWidth/10,Y2-ParentheseWidth-ActualSize);
			}
			return 1;
		}
		/*int PenWidth=ParentheseWidth;

		POINT points[6];
		if (PaintTop)
		{
		mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
		points[0].x=PenWidth;  points[0].y=PenWidth;
		points[1].x=(X2-X1)*4-PenWidth;  points[1].y=PenWidth;
		mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
		mf->EndMyPainting(DC,X1,Y1);

		if (!PaintBottom)
		{
			mf->StartMyPainting(DC,ParentheseWidth*4,0,(ParentheseWidth+ActualSize)*4,color);
			points[0].x=PenWidth;  points[0].y=PenWidth;
			points[1].x=PenWidth;  points[1].y=(ParentheseWidth+ActualSize)*4;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			mf->EndMyPainting(DC,X1,Y1);
			mf->StartMyPainting(DC,ParentheseWidth*4,0,(ParentheseWidth+ActualSize)*4,color);
			points[0].x=ParentheseWidth*4-PenWidth;  points[0].y=PenWidth;
			points[1].x=ParentheseWidth*4-PenWidth;  points[1].y=(ParentheseWidth+ActualSize)*4;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			mf->EndMyPainting(DC,X2-ParentheseWidth,Y1);
		}
		}
		if (PaintBottom)
		{
		mf->StartMyPainting(DC,(X2-X1)*4,0,ParentheseWidth*4,color);
		points[0].x=PenWidth;  points[0].y=4*ParentheseWidth-PenWidth;
		points[1].x=(X2-X1)*4-PenWidth;  points[1].y=4*ParentheseWidth-PenWidth;
		mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
		mf->EndMyPainting(DC,X1,Y2-ParentheseWidth);
		if (!PaintTop)
		{
			mf->StartMyPainting(DC,ParentheseWidth*4,0,(ParentheseWidth+ActualSize)*4,color);
			points[0].x=PenWidth;  points[0].y=0;
			points[1].x=PenWidth;  points[1].y=(ParentheseWidth+ActualSize)*4-PenWidth;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			mf->EndMyPainting(DC,X1,Y2-ParentheseWidth-ActualSize);
			mf->StartMyPainting(DC,ParentheseWidth*4,0,(ParentheseWidth+ActualSize)*4,color);
			points[0].x=ParentheseWidth*4-PenWidth;  points[0].y=0;
			points[1].x=ParentheseWidth*4-PenWidth;  points[1].y=(ParentheseWidth+ActualSize)*4-PenWidth;
			mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
			mf->EndMyPainting(DC,X2-ParentheseWidth,Y2-ParentheseWidth-ActualSize);
		}
		}

		if ((PaintTop) && (PaintBottom))
		{
		mf->StartMyPainting(DC,ParentheseWidth*4,0,(Y2-Y1)*4,color);
		points[0].x=PenWidth;  points[0].y=PenWidth;
		points[1].x=PenWidth;  points[1].y=(Y2-Y1)*4-PenWidth;
		mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
		mf->EndMyPainting(DC,X1,Y1);

		mf->StartMyPainting(DC,ParentheseWidth*4,0,(Y2-Y1)*4,color);
		points[0].x=ParentheseWidth*4-PenWidth;  points[0].y=PenWidth;
		points[1].x=ParentheseWidth*4-PenWidth;  points[1].y=(Y2-Y1)*4-PenWidth;
		mf->MyPolyline(DC,points,2,PenWidth,IsBlue);
		mf->EndMyPainting(DC,X2-ParentheseWidth,Y1);
		}*/

	}

	if (Type=='x')  //crossed
	{
		if (1/*!IsHighQualityRendering*/)  //only non-high-quality is proveded because lot of memory is consumet for large pictures
		{
			if (PaintTop)
			{
				DC->MoveTo(X1+ParentheseWidth/10,Y1+ParentheseWidth/10);
				DC->LineTo(X2-ParentheseWidth/10,Y2-ParentheseWidth/10);
			}
			if (PaintBottom)
			{
				DC->MoveTo(X1+ParentheseWidth/10,Y2-ParentheseWidth/10);
				DC->LineTo(X2-ParentheseWidth/10,Y1+ParentheseWidth/10);
			}
			return 1;
		}
	}
	return 0;
}
#pragma optimize("",on)

//the following function determines wether the expression contains text or math around the given position
//If the position=-1 then the whole expression is considered.
//Returns 1 if it contains text.
int CExpression::IsTextContained(int position, char unmark_at_line_start)
{
	if (position>m_NumElements-1) position=m_NumElements-1;
	if (position<-1) position=-1;
	if ((position==-1) && (m_pElementList->Type==0)) return this->m_StartAsText;
	if ((position>=0) && ((m_pElementList+position)->Type==0)) return this->m_StartAsText;

	int from=0;
	int to=m_NumElements-1;
	if (position>=0)
	{
		from=position;while((from>=0) && ((m_pElementList+from)->Type!=12) && ((m_pElementList+from)->Type!=11)) from--;
		to=position;while((to<m_NumElements) && ((m_pElementList+to)->Type!=12) && ((m_pElementList+to)->Type!=11)) to++;
		from++;
		to--;
		if (from<0) from=0;
		if (to>m_NumElements-1) to=m_NumElements-1;
	}

	//check if the keyboard entry is active and the cursor stands at the beginning of a line
	if (unmark_at_line_start)
	{
		//if the 'unmark_at_line_start' flag is set then we don't mark the object as text if the
		//cursor is at the beginning of the line
		if (((CObject*)this==KeyboardEntryObject) && (KeyboardEntryBaseObject))
		{
			if ((m_IsKeyboardEntry>=1) && (m_IsKeyboardEntry<=m_NumElements))
			{
				if (((m_pElementList+m_IsKeyboardEntry-1)->Type==1) && (m_KeyboardCursorPos==0))
				{
					if (m_IsKeyboardEntry==1) return 0;
					
					if ((m_IsKeyboardEntry>=2) && 
						(((m_pElementList+m_IsKeyboardEntry-2)->Type==12) || ((m_pElementList+m_IsKeyboardEntry-2)->Type==11)) ||
						(((m_pElementList+m_IsKeyboardEntry-2)->Type==2) && ((m_pElementList+m_IsKeyboardEntry-2)->pElementObject->Data1[0]==(char)0xFF)))
					{
						return 0;
					}
				}
			}
		}

		//also if there is anything selected
		for (int i=from;i<=to;i++)
		{
			if ((m_pElementList+i)->IsSelected==2) return 0;
		}

		//also if we are not in the main equation line
		if (m_pPaternalElement)
		{
			if ((m_pPaternalElement->Expression2==(CObject*)this) || (m_pPaternalElement->Expression3==(CObject*)this))
				return 0;
			if (m_pPaternalElement->m_Type==4) return 0;
		}
	}

	//we determine number of text and math elements;
	int text=0;
	int math=0;
	for (int i=from;i<=to;i++)
	{
		tElementStruct *ts=m_pElementList+i;

		if (ts->pElementObject==NULL) continue;
		if ((ts->Type==1) && (ts->pElementObject->Data1[0]==0)) continue;
		if ((ts->Type==11) || (ts->Type==12)) return 0; //a table cannot be considered as a text object
		if (ts->pElementObject->m_Text) text++; else if (ts->pElementObject->m_Type!=2) math++;
		if (i==0) {text*=3;math*=3;} //the first member has more weight
	}

	if (text==math) return m_StartAsText;
	if (text>math) return 1; else return 0;
}

// select element of the matris at (row,column)
int CExpression::SelectMatrixElement(int row, int column,char select_type)
{
	int r=0,c=0;
	int found_anything=0;
	tElementStruct *ts=m_pElementList;
	m_InternalInsertionPoint=0;
	for (int i=0;i<m_NumElements;i++,ts++)
	{
		if (ts->Type==11) c++;
		if (ts->Type==12) {r++;c=0;}

		if ((r==row) && (c==column))
		{
			found_anything=1;
			m_IsMatrixElementSelected=1;
			if ((ts->Type!=12) && (ts->Type!=11)) 
			{
				SelectElement(select_type,i);
			}
		}
	}
	return found_anything;
}

int CExpression::InsertMatrixColumn(int position)
{
	if (position>=50) return 0;
	if (position>this->m_MaxNumColumns) return 0;

	if ((m_pElementList+m_NumElements-1)->Type!=12)
		InsertEmptyElement(m_NumElements,12,0);

	int col=0;
	int separator_inserted=0;
	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((col==position) && (!separator_inserted))
		{
			separator_inserted=1;
			InsertEmptyElement(i,11,0);
			InsertEmptyElement(i,0,0);
			i+=1;
			continue;
		}

		if (ts->Type==11)
		{
			col++;
		}
		if (ts->Type==12) 
		{
			col++;
			if ((col==position) && (!separator_inserted))
			{
				ts->Type=11;
				InsertEmptyElement(i+1,12,0);
				InsertEmptyElement(i+1,0,0);
				i+=2;
			}
			col=0;
			separator_inserted=0;
		}
	}
	m_MaxNumColumns++;

	if (position==0)
	{
		for (int j=0;j<m_MaxNumRows;j++)
		{
			tCellAttributes attr;
			if (GetCellAttributes(j,1,&attr))
				SetCellAttributes(j,0,*attr.alignment,*attr.top_border,*attr.bottom_border,*attr.left_border,*attr.left_border);
		}
	}
	else
	{
		for (int j=0;j<m_MaxNumRows;j++)
		{
			tCellAttributes attr;
			if (GetCellAttributes(j,position-1,&attr))
				SetCellAttributes(j,position,*attr.alignment,*attr.top_border,*attr.bottom_border,*attr.right_border,*attr.right_border);
		}
	}

	return 1;
}

int CExpression::InsertMatrixRow(int position)
{
	if (position>=50) return 0;
	if (position>this->m_MaxNumRows) return 0;

	if ((m_pElementList+m_NumElements-1)->Type!=12)
		InsertEmptyElement(m_NumElements,12,0);


	if (position==0)
	{
		InsertEmptyElement(0,12,0);
		InsertEmptyElement(0,0,0);
		for (int j=0;j<m_MaxNumColumns-1;j++)
		{
			InsertEmptyElement(0,11,0);
			InsertEmptyElement(0,0,0);
		}
		m_MaxNumRows++;
		for (int j=0;j<m_MaxNumColumns;j++)
		{
			tCellAttributes attr;
			if (GetCellAttributes(1,j,&attr))
				SetCellAttributes(0,j,*attr.alignment,*attr.top_border,*attr.top_border,*attr.left_border,*attr.right_border);
		}
		return 1;
	}

	int row=0;
	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if (ts->Type==12) 
		{
			row++;
			if (position==row)
			{
				InsertEmptyElement(i+1,12,0);
				InsertEmptyElement(i+1,0,0);
				for (int j=0;j<m_MaxNumColumns-1;j++)
				{
					InsertEmptyElement(i+1,11,0);
					InsertEmptyElement(i+1,0,0);
				}
				m_MaxNumRows++;
				for (int j=0;j<m_MaxNumColumns;j++)
				{
					tCellAttributes attr;
					if (GetCellAttributes(row-1,j,&attr))
						SetCellAttributes(row,j,*attr.alignment,*attr.bottom_border,*attr.bottom_border,*attr.left_border,*attr.right_border);
				}
				return 1;
			}
		}
	}
	return 0;
}

// returns the postion of the first element at (row,column), also expands matrix if neccessary
int CExpression::FindMatrixElement(int row, int column,int expand_flag)
{
	int r=0,c=0;
	int i;
	tElementStruct *ts=m_pElementList;
	for (i=0;i<m_NumElements;i++,ts++)
	{
		if ((r==row) && (c==column)) 
			return i;
		if (ts->Type==11) 
			c++;
		if (ts->Type==12)
		{
			//if (c!=m_MaxNumColumns-1)
			//	break; //irregular matrix - we must do it slowly and correct the matrix structure
			c=0;
			r++;
		}
	}

	//we did not find the cell
	//if we are not alowed to expand the matrix, then return error ('cell not found')
	if (!expand_flag) 
		return -1;

	//we ensure the matrix is regular
	AdjustMatrix();

	//check if we need to add more columns
	for (i=m_MaxNumColumns;i<=column;i++)
		InsertMatrixColumn(i);

	//check if we need to add more rows
	for (i=m_MaxNumRows;i<=row;i++)
		InsertMatrixRow(i);

	//recursive call to return the cell element
	if  ((column<m_MaxNumColumns) && (row<m_MaxNumRows))
		return FindMatrixElement(row,column,0);

	//something went wrong
	return -1;
}

//adujust rows and columns of mattrix (it fills rows with empty elements if neccessary)
int CExpression::AdjustMatrix(void)
{
	//finding the actual matrix size (by counting row and column separators)
	int i;
	int row,column;
	m_MaxNumColumns=m_MaxNumRows=1;
	column=1;
	tElementStruct *ts=m_pElementList;
	for (i=0;i<m_NumElements;i++,ts++)
	{
		if (ts->Type==11) 
			column++;
		if (ts->Type==12)
		{
			if (column>m_MaxNumColumns) 
				m_MaxNumColumns=column;
			if (i<m_NumElements-1)
				m_MaxNumRows++;
			column=1;
		}
	}

	//check if this is a matrix at all
	if ((m_MaxNumColumns==1) && (m_MaxNumRows==1))
	{
		//if this is not a matrix, then it should not end with type 12 or type 11 element
		if (((m_pElementList+m_NumElements-1)->Type==12) ||
			((m_pElementList+m_NumElements-1)->Type==11))
			DeleteElement(m_NumElements-1);
		return 0;
	}

	//make sure the matrix ends with row separator (element of type 12)
	if ((m_pElementList+m_NumElements-1)->Type!=12) 
		InsertEmptyElement(m_NumElements,12,0);

	//we are expanding rows that may be too short
	row=column=0;
	for (i=0;i<m_NumElements;i++)
	{
		ts=m_pElementList+i;
		if (ts->Type==11)
			column++;
		if (ts->Type==12)
		{
			if (column<m_MaxNumColumns-1)
			{
				ts->Type=11;
				InsertEmptyElement(i+1,12,0);
			}
			else
			{
				row++;
				column=0;
			}
		}
	}

	//inserty dummy objects if neccessary
	int found_any=0;
	//ts=m_pElementList;
	for (i=0;i<m_NumElements;i++)
	{
		ts=m_pElementList+i;
		if (ts->Type==11) 
		{
			if (!found_any) 
			{InsertEmptyElement(i,0,0);i++;;if (m_Selection>i) m_Selection++;}
			found_any=0;
			continue;
		}
		if (ts->Type==12) 
		{
			if (!found_any) {InsertEmptyElement(i,0,0);i++;if (m_Selection>i) m_Selection++;}
			found_any=0;
			continue;
		}
		found_any=1;
	}

	//adjust borderlines of matrix cells
	for (int i=0;i<m_MaxNumRows;i++)
		for (int j=0;j<m_MaxNumColumns;j++)
		{
			tCellAttributes attrib,attrib2;
			if (GetCellAttributes(i,j,&attrib))
			{
				if ((j<m_MaxNumColumns-1) && (GetCellAttributes(i,j+1,&attrib2)))
					*(attrib.right_border)=*(attrib2.left_border);
				if ((i<m_MaxNumRows-1) && (GetCellAttributes(i+1,j,&attrib2)))
					*(attrib.bottom_border)=*(attrib2.top_border);

			}
		}
	return 1;
} 

//used for text search feature
int CExpression::ConvertToPlainText(int buffer_size,char *compile,char force_parentheses)
{
	char parentheses=(this->m_ParenthesesFlags&0x81)?this->m_DrawParentheses:0;
	if ((force_parentheses) && (parentheses==0)) parentheses='(';
	if (parentheses)
	{
		int l=(int)strlen(compile);
		compile[l]=parentheses;
		compile[++l]=0;
		if (parentheses=='(') parentheses=')';
		if (parentheses=='[') parentheses=']';
		if (parentheses=='{') parentheses='}';
	}

	char wordend=0;
	for (int i=0;i<this->m_NumElements;i++)
	{
		int len=(int)strlen(compile);
		if (len>=buffer_size) break;

		tElementStruct *ts=this->m_pElementList+i;
		if (!ts->pElementObject) {strcat(compile,"\01");continue;}

		if ((ts->Type==1) || (ts->Type==6))
		{
			int ismath=1;
			if ((ts->Type==1) && (ts->pElementObject->m_Text)) ismath=0;

			if (len+(int)strlen(ts->pElementObject->Data1)>=buffer_size) return 0;
			if ((ts->Type!=1) || ((ts->pElementObject->m_Text!=2) && (ts->pElementObject->m_Text!=3)))
			{
				if (wordend) {strcat(compile+len," ");len++;}

				if ((i==0) || ((ts-1)->Type!=1) || ((ts-1)->pElementObject->m_Text==0))
				if ((!ismath) && (len) && (compile[len-1]!=' ') && (compile[len-1]!='(') && (compile[len-1]!='[')) {strcat(compile+len," ");len++;}
			}
			strcat(compile+len,ts->pElementObject->Data1);
			if (!ismath) wordend=1;
			if ((ts->Type==1) && (ts->pElementObject->m_Text==4)) wordend=0;

			if ((ts->Type==6) && (ts->pElementObject->Expression1))
			{
				int ret=((CExpression*)ts->pElementObject->Expression1)->ConvertToPlainText(buffer_size,compile,1);
				if (ret==0) return 0;
			}
		}
		else if (ts->Type==2)
		{
			char ch=ts->pElementObject->Data1[0];
			if (ch!=ts->pElementObject->Data2[1]) ch=1;
			if (ch==(char)0xD7) ch='*';
			compile[len]=ch;
			compile[++len]=0;
			wordend=0;
		}
		else if (ts->Type==3)
		{
			int ret=((CExpression*)ts->pElementObject->Expression1)->ConvertToPlainText(buffer_size,compile,(((CExpression*)ts->pElementObject->Expression1)->m_NumElements>1)?1:0);
			if (ret==0) return 0;
			strcat(compile+len,"^");
			ret=((CExpression*)ts->pElementObject->Expression2)->ConvertToPlainText(buffer_size,compile,(((CExpression*)ts->pElementObject->Expression2)->m_NumElements>1)?1:0);
			if (ret==0) return 0;
			wordend=0;
		}
		else if (ts->Type==5)
		{
			int ret=((CExpression*)ts->pElementObject->Expression1)->ConvertToPlainText(buffer_size,compile);
			if (ret==0) return 0;
			wordend=0;
		}
		else if (ts->Type==7)
		{
			char ch=ts->pElementObject->Data1[0];
			if (ch=='S') strcat(compile,"sum");
			else if (ch=='P') strcat(compile,"prod");
			else if (ch=='I') strcat(compile,"int");
			else strcat(compile+len,"\01");
			int ret=((CExpression*)ts->pElementObject->Expression1)->ConvertToPlainText(buffer_size,compile,1);
			if (ret==0) return 0;
			wordend=0;
		}
		else if (ts->Type==9)
		{
			int ret=((CExpression*)ts->pElementObject->Expression1)->ConvertToPlainText(buffer_size,compile);
			if (ret==0) return 0;
			strcat(compile+len,"\01");
			wordend=0;
		}
		else
		{
			strcat(compile+len,"\01");
			wordend=0;
		}
	}


	if (parentheses)
	{
		int l=(int)strlen(compile);
		compile[l]=parentheses;
		compile[++l]=0;
	}
	return 1;
}

int CExpression::SearchForString(char *str)
{
	{
		char *compile=(char*)malloc(8192);
		compile[0]=0;
		ConvertToPlainText(8000,compile);

		//check if the compiled string containst the asked string
		int x1=(int)strlen(compile);
		int x2=(int)strlen(str);
		if (x1>=x2)
		{
			for (int j=0;j<=x1-x2;j++)
			{
				if (strnicmp(compile+j,str,x2)==0) return 1;
			}
		}
		free(compile);
	}

	for (int i=0;i<this->m_NumElements;i++)
	{
		tElementStruct *ts=this->m_pElementList+i;
		if (ts->pElementObject)
		{
			CExpression *e1=((CExpression*)ts->pElementObject->Expression1);
			CExpression *e2=((CExpression*)ts->pElementObject->Expression2);
			CExpression *e3=((CExpression*)ts->pElementObject->Expression3);

			if (ts->Type==4)
			{
				if ((e1) && (e1->SearchForString(str))) return 1;
				if ((e2) && (e2->SearchForString(str))) return 1;
			}
			else if (ts->Type==8)
			{
				if ((e1) && (e1->SearchForString(str))) return 1;
			}
			else if (ts->Type==10)
			{
				if ((e1) && (e1->SearchForString(str))) return 1;
				if ((e2) && (e2->SearchForString(str))) return 1;
				if ((e3) && (e3->SearchForString(str))) return 1;
			}
		}
	}

	return 0;

}

int CExpression::DeleteSelection(char selection_type)
{
	if ((m_Selection==0x7FFF) && (selection_type!=2))  //the whole expression is to be deleted
	{
		if ((m_DrawParentheses) && (m_pPaternalElement) && (m_pPaternalElement->m_Type==5))
		{
			//special handling for parenthese type element
			if ((m_ParenthesesSelected) && (m_pPaternalExpression))
			{
				for (int i=0;i<m_pPaternalExpression->m_NumElements;i++)
				{
					tElementStruct *theElement=m_pPaternalExpression->m_pElementList+i;
					if ((theElement->pElementObject==this->m_pPaternalElement))
					{
						m_pPaternalExpression->DeleteElement(i);
						return 2; //returns 2 to show that the whole expression is deleted
					}
				}
			}
			else
			{
				goto delete_one_by_one;
			}
		}

		Delete();
		return 1;
	}


	if (m_Selection) return 1; //nothing to be deleted - the cursor is pointing at insertion point
delete_one_by_one:
	if (m_IsRowInsertion) return 1;
	if (m_IsColumnInsertion) return 1;

	int i;

	if (m_IsMatrixElementSelected)
	{
		//matrix mode deletion
		//first go for every row and check if everything is selected
		for (i=0;i<m_MaxNumRows;i++)
		{
			int j;
			int all_selected=1;
			int any_found=0;
			for (j=0;j<m_MaxNumColumns;j++)
			{
				int first=FindMatrixElement(i,j,0);
				if (first<0) first=0;
				while ((first<m_NumElements) && ((m_pElementList+first)->Type!=11) && ((m_pElementList+first)->Type!=12))
				{
					if ((m_pElementList+first)->IsSelected!=selection_type) {all_selected=0;break;}
					else any_found=1;
					first++;
				}
				if (all_selected==0) break;
			}

			if ((any_found) && (all_selected))
			{
				//deleting the whole row
				j=FindMatrixElement(i,0,0);
				if (j<0) j=0;
				while (j<m_NumElements)
				{
					if ((m_pElementList+j)->Type==12)
					{
						DeleteElement(j);
						break;
					} 
					DeleteElement(j);
					if ((m_NumElements==1) && (m_pElementList->Type==0)) break;
					if ((m_NumElements<=j) && (j>0))
					{
						DeleteElement(j-1);
						break;
					}
				}
				i--;
				m_MaxNumRows--;
			}
		}

		if (m_NumElements<=1) {m_MaxNumColumns=1;m_MaxNumRows=1;}


		//now go and check if there is any column to be deleted
		for (i=0;i<m_MaxNumColumns;i++)
		{
			int j;
			int all_selected=1;
			int any_found=0;
			for (j=0;j<m_MaxNumRows;j++)
			{
				int first=FindMatrixElement(j,i,0);
				if (first<0) first=0;
				while ((first<m_NumElements) && ((m_pElementList+first)->Type!=11) && ((m_pElementList+first)->Type!=12))
				{
					if ((m_pElementList+first)->IsSelected!=selection_type) {all_selected=0;break;}
					else any_found=1;
					first++;
				}
				if (all_selected==0) break;
			}

			if ((any_found) && (all_selected))
			{
				//deleting the whole column
				int ii;
				for (ii=0;ii<m_MaxNumRows;ii++)
				{
					j=FindMatrixElement(ii,i,0);
					if (j<0) j=0;
					while (j<m_NumElements)
					{
						if (((m_pElementList+j)->Type==12) || (m_pElementList+j)->Type==11)
						{
							if (((m_pElementList+j)->Type==12) && (i>0))
								(m_pElementList+j-1)->Type=12;
							DeleteElement(j);
							break;
						}
						DeleteElement(j);
						if ((m_NumElements==1) && (m_pElementList->Type==0)) break;
					}
				}
				if ((m_pElementList+m_NumElements-1)->Type==11)
					(m_pElementList+m_NumElements-1)->Type=12;
				i--;
				m_MaxNumColumns--;
			}
		}
	}

	{
		//delete all selected elements
		for (i=0;i<m_NumElements;i++)
		{
			tElementStruct *ts=m_pElementList+i;
			//if (selection_type!=2)
			{
				if (selection_type!=2) if (ts->Type==11) {ts->IsSelected=0;continue;}
				if (ts->Type==12) {ts->IsSelected=0;continue;}
			}
			if (ts->IsSelected==selection_type) {DeleteElement(i);i--;}
		}
	}

	AdjustMatrix();

	return 1;
}

int CExpression::PaintDecoration(CDC *DC,short zoom,int X, int Y,int LastDecorationElement,int i, int LastDecoration,int color)
{
	int ActualSize=GetActualFontSize(zoom);
	int X1=(m_pElementList+LastDecorationElement)->X_pos;
	int X2;
	int AllSelected=1;
	int maxabove=ActualSize/4;
	int maxbelow=ActualSize/4;
	Y+=(m_pElementList+LastDecorationElement)->Y_pos;
	X2=(m_pElementList+i-1)->X_pos+(m_pElementList+i-1)->Length;
	int ii;
	for (ii=LastDecorationElement;ii<i;ii++)
	{
		if ((m_pElementList+ii)->IsSelected==0) AllSelected=0;
		if (((m_pElementList+ii)->Type!=11) && ((m_pElementList+ii)->Type!=12))
		{
			int b=(m_pElementList+ii)->Below;
			int a=(m_pElementList+ii)->Above;
			if (b>ActualSize/4) b-=m_MarginY; //trick test - checking if the margin is already included or not
			if (a>ActualSize/4) a-=m_MarginY; //trick test - -||-
			if (a>maxabove) maxabove=a;
			if (b>maxbelow) maxbelow=b;
		}
	}

	if (LastDecoration==1) //strikeout
	{
		DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),AllSelected,color));
		DC->MoveTo(X+X1,Y+min(ActualSize/2,maxbelow));
		DC->LineTo(X+X2,Y-min(ActualSize/2,maxabove));
	}
	if (LastDecoration==2) //encircle
	{
		DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),AllSelected,color));
		DC->Arc(X+X1-m_MarginX,Y-maxabove-m_MarginY,X+X2+m_MarginX,Y+maxbelow+m_MarginY, X+X1,Y+1,X+X1,Y);
	}
	if (LastDecoration==3) //underline
	{
		DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),AllSelected,color));
		DC->MoveTo(X+X1,Y+maxbelow+m_MarginY+((IsHighQualityRendering)?ActualSize/20:-ActualSize/12));
		DC->LineTo(X+X2,Y+maxbelow+m_MarginY+((IsHighQualityRendering)?ActualSize/20:-ActualSize/12));

	}
	if (LastDecoration==4) //overline
	{
		DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),AllSelected,color));
		DC->MoveTo(X+X1,Y-maxabove-3*m_MarginY/4+((IsHighQualityRendering)?-ActualSize/10:-ActualSize/20));
		DC->LineTo(X+X2,Y-maxabove-3*m_MarginY/4+((IsHighQualityRendering)?-ActualSize/10:-ActualSize/20));
	}
	if (LastDecoration==5) //under curly parentheses
	{
		int start=X+X1;
		int stop=X+X2;
		int len=X2-X1;
		int py=Y+maxbelow+m_MarginY+((IsHighQualityRendering)?ActualSize/20:-ActualSize/12)+ActualSize/12;
		int dy=2*ActualSize/13;
		if (len<dy*5) dy=len/5;
		DC->SelectObject(GetPenFromPool(max(ActualSize/20,1),AllSelected,color));

		DC->MoveTo(start,py-dy);
		DC->LineTo(start+dy,py);
		DC->LineTo(start+len/2-dy,py);
		DC->LineTo(start+len/2,py+dy);
		DC->LineTo(start+len/2+dy,py);
		DC->LineTo(stop-dy,py);
		DC->LineTo(stop,py-dy);
	}
	return 0;
}



//will start keyboard entry nearest to given x,y position
// direction<0 find positions above x,y;  direction>0 find positions below x,y
int KSAdistance=0x7FFFFFFF;
CExpression *KSAfexpr;
int KSAfselection;
char KSAInternal=0;
char KSAcurpos=0;
CExpression *KSAoriginal=NULL;
int CExpression::KeyboardStartAt(int X, int Y, char direction,char between)
{
	int i;

	if (!KSAInternal)
	{
		if (m_pPaternalExpression==NULL) 
		{
			if (KSAoriginal==NULL) KSAoriginal=this;
			KSAdistance=0x7FFFFFFF;
			KSAfexpr=NULL;
			KSAfselection=0;
			KSAInternal=1;
			KSAcurpos=-1;
		}
		else
		{
			if (KSAoriginal==NULL) KSAoriginal=this;
			int ii;
			for (ii=0;ii<m_pPaternalExpression->m_NumElements;ii++)
			{
				tElementStruct *ts=m_pPaternalExpression->m_pElementList+ii;
				if ((ts->pElementObject) && (ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12))
				{
					if (ts->pElementObject->Expression1==(CObject*)this)
					{	
						X+=ts->X_pos+ts->pElementObject->E1_posX;	
						Y+=ts->Y_pos+ts->pElementObject->E1_posY; 
						break;
					}
					if (ts->pElementObject->Expression2==(CObject*)this)
					{	
						X+=ts->X_pos+ts->pElementObject->E2_posX;	
						Y+=ts->Y_pos+ts->pElementObject->E2_posY; 
						break;
					}
					if (ts->pElementObject->Expression3==(CObject*)this)
					{	
						X+=ts->X_pos+ts->pElementObject->E3_posX;	
						Y+=ts->Y_pos+ts->pElementObject->E3_posY; 
						break;
					}
				}
			}
			return m_pPaternalExpression->KeyboardStartAt(X,Y,direction,between);
		}
	}

	for (i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((direction==-1) && (ts->Y_pos<=Y)) //finding UP
		{
			int delta=Y-ts->Y_pos;
			if ((ts->Type!=1) || (ts->pElementObject->m_Text==0) || (between==0))
			{
			if ((abs(ts->X_pos-X)<=delta*2) || (between==2)) //candidate found
			{
				int d=abs(ts->X_pos-X)*2+delta; //not really distance (for simple calculation)
				if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i;KSAcurpos=-1;}
			}
			if ((abs(ts->X_pos+ts->Length-X)<=delta*2) || (between==2))
			{
				int d=abs(ts->X_pos+ts->Length-X)*2+delta;
				if ((i<m_NumElements-1) && ((m_pElementList+i+1)->Y_pos>Y)) d=0x7FFFFFFF;
				if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i+1;KSAcurpos=-1;if (ts->Type==0) KSAfselection=i;}
			}
			}
			if ((between) && (ts->Type==1) && (ts->pElementObject->m_Text)) //try positions inside variables
			{
				for (int j=0;j<=(int)strlen(ts->pElementObject->Data1);j++)
				{
					if (abs(ts->X_pos+ts->pElementObject->Data3[j]-X)<=delta*2)
					{
						int d=abs(ts->X_pos+ts->pElementObject->Data3[j]-X)*2+delta;
						if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i;KSAcurpos=j;}
					}
				}
			}
		}

		if ((direction==1) && (ts->Y_pos>=Y)) //finding DOWN
		{
			int delta=ts->Y_pos-Y;
			if ((ts->Type!=1) || (ts->pElementObject->m_Text==0) || (between==0))
			{
			if ((abs(ts->X_pos-X)<=delta*2) || (between==2)) //candidate found
			{
				int d=abs(ts->X_pos-X)*2+delta; //not really distance (for simple calculation)
				if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i;KSAcurpos=-1;}
			}
			if ((abs(ts->X_pos+ts->Length-X)<=delta*2) || (between==2))
			{
				int d=abs(ts->X_pos+ts->Length-X)*2+delta;
				if ((i<m_NumElements-1) && ((m_pElementList+i+1)->Y_pos<Y)) d=0x7FFFFFFF;
				if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i+1;KSAcurpos=-1;if (ts->Type==0) KSAfselection=i;}
			}
			}
			if ((between) && (ts->Type==1) && (ts->pElementObject->m_Text)) //try positions inside variables
			{
				for (int j=0;j<=(int)strlen(ts->pElementObject->Data1);j++)
				{
					if (abs(ts->X_pos+ts->pElementObject->Data3[j]-X)<=delta*2)
					{
						int d=abs(ts->X_pos+ts->pElementObject->Data3[j]-X)*2+delta;
						if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i;KSAcurpos=j;}
					}
				}
			}
		}

		if ((direction==-2) && (ts->X_pos<=X)) //finding LEFT
		{
			int tmpy=abs(ts->Y_pos-Y+1+m_MarginY/6);

			{
				int delta=X-ts->X_pos-ts->Length;
				if (tmpy<=delta*5) //candidate found
				//if (delta>0)
				{
					int d=tmpy*3+delta*2; //not really distance (for simple calculation)
					if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i+1;if ((ts->Type==0) || (ts->Type==11) || (ts->Type==12)) KSAfselection=i;}
				}
			}
			if ((ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12))
			{
				int delta=X-ts->X_pos;

				if (tmpy<=delta*5) //candidate found
				//if (delta>0)
				{
					int d=tmpy*3+delta*2; //not really distance (for simple calculation)
					if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i;}
				}
			}
		}

		if ((direction==2) && (ts->X_pos+ts->Length>=X)) //finding RIGHT
		{
			int tmpy=abs(ts->Y_pos-Y+1+m_MarginY/6);

			{
				int delta=ts->X_pos-X;
				if (tmpy<=delta*5) //candidate found
				//if (delta>0)
				{
					int d=tmpy*3+delta*2; //not really distance (for simple calculation)
					if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i;}
				}
			}
			if ((ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12))
			{
				int delta=ts->X_pos+ts->Length-X;

				if (tmpy<=delta*5) //candidate found
				//if (delta>0)
				{
					int d=tmpy*3+delta*2; //not really distance (for simple calculation)
					if (d<KSAdistance) {KSAdistance=d;KSAfexpr=this;KSAfselection=i+1;if ((ts->Type==0) || (ts->Type==11) || (ts->Type==12)) KSAfselection=i;}
				}
			}
		}
		if ((ts->pElementObject) && (ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12))
		{
			int is_ok=7;
			CExpression *e1=(CExpression*)ts->pElementObject->Expression1;
			CExpression *e2=(CExpression*)ts->pElementObject->Expression2;
			CExpression *e3=(CExpression*)ts->pElementObject->Expression3;
			
			//special handlings
			if (KSAoriginal==this)
			{
				if ((ts->Type!=4) && ((direction==2) || (direction==-2))) is_ok&=0x01;
			}

			if ((e1) && (is_ok&0x01)) e1->KeyboardStartAt(X-ts->X_pos-ts->pElementObject->E1_posX,Y-ts->Y_pos-ts->pElementObject->E1_posY,direction,between);
			if ((e2) && (is_ok&0x02)) e2->KeyboardStartAt(X-ts->X_pos-ts->pElementObject->E2_posX,Y-ts->Y_pos-ts->pElementObject->E2_posY,direction,between);
			if ((e3) && (is_ok&0x04)) e3->KeyboardStartAt(X-ts->X_pos-ts->pElementObject->E3_posX,Y-ts->Y_pos-ts->pElementObject->E3_posY,direction,between);
			
		}
	}

	if (m_pPaternalExpression==NULL) 
	{
		KSAInternal=0;
		if (KSAfexpr)
		{
			if (KSAoriginal) 
			{
				KSAoriginal->m_Selection=0;KSAoriginal->m_IsKeyboardEntry=0;
			}

			KeyboardEntryObject=(CObject*)(KSAfexpr);
			if ((between) && (KSAcurpos>=0)) 
			{
				KSAfexpr->m_IsKeyboardEntry=KSAfselection+1;
				KSAfexpr->m_KeyboardCursorPos=KSAcurpos;
				KSAoriginal=NULL;
				CDC *mdc=Toolbox->GetDC();
				Toolbox->PaintTextcontrolbox(mdc);
				Toolbox->ReleaseDC(mdc);
				return 1;
			}
			KSAfexpr->m_IsKeyboardEntry=KSAfselection+1;
			KSAfexpr->m_KeyboardCursorPos=0;
			if (KSAfexpr->m_IsKeyboardEntry>1)
			{
				if ((KSAfexpr->m_pElementList+KSAfexpr->m_IsKeyboardEntry-2)->Type==0) KSAfexpr->m_IsKeyboardEntry--;
			}
			KSAfexpr->InsertEmptyElement(KSAfexpr->m_IsKeyboardEntry-1,1,0);
			KSAoriginal=NULL;
				CDC *mdc=Toolbox->GetDC();
				Toolbox->PaintTextcontrolbox(mdc);
				Toolbox->ReleaseDC(mdc);
			return 1;
		}
		else
		{
			KSAoriginal=NULL;
			return 0;
		}
	}

	return 0;
}

//returns cursor position (used to automaticli scroll the window)
int CExpression::GetKeyboardCursorPos(int * X, int * Y)
{
	if (m_IsKeyboardEntry)
	{
		tElementStruct *ts=m_pElementList+min(m_IsKeyboardEntry-1,m_NumElements-1);
		*Y=ts->Y_pos;
		*X=ts->X_pos;
		if (ts->Type==1) 
			*X+=ts->pElementObject->Data3[m_KeyboardCursorPos];
		else if (m_IsKeyboardEntry==m_NumElements)
			*X+=ts->Length;
		goto getkeyboardcursorpos_finish;
	}

	int i;
	for (i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12) && (ts->pElementObject))
		{
			if (ts->pElementObject->Expression1)
				if (((CExpression*)(ts->pElementObject->Expression1))->GetKeyboardCursorPos(X,Y))
				{
					*X+=ts->X_pos+ts->pElementObject->E1_posX;
					*Y+=ts->Y_pos+ts->pElementObject->E1_posY;
					goto getkeyboardcursorpos_finish;
				}
			if (ts->pElementObject->Expression2)
				if (((CExpression*)(ts->pElementObject->Expression2))->GetKeyboardCursorPos(X,Y))
				{
					*X+=ts->X_pos+ts->pElementObject->E2_posX;
					*Y+=ts->Y_pos+ts->pElementObject->E2_posY;
					goto getkeyboardcursorpos_finish;
				}
			if (ts->pElementObject->Expression3)
				if (((CExpression*)(ts->pElementObject->Expression3))->GetKeyboardCursorPos(X,Y))
				{
					*X+=ts->X_pos+ts->pElementObject->E3_posX;
					*Y+=ts->Y_pos+ts->pElementObject->E3_posY;
					goto getkeyboardcursorpos_finish;
				}

		}
	}
	return 0;
getkeyboardcursorpos_finish:

	if ((m_IsVertical) && (m_pPaternalExpression==NULL))
	{
		int tmp=*X;
		*X=*Y+m_OverallAbove;
		*Y=m_OverallLength/2-tmp;
	}
	return 1;

}

int CExpression::ChangeFontSize(float factor)
{
	float fs=(float)m_FontSize;
	fs*=factor;
	m_FontSize=(short)fs;
	if ((fs-((float)m_FontSize))>0.5) m_FontSize++;
	if (m_FontSize<10) 
		m_FontSize=10;

	//now go to all submembers;
	int i;
	for (i=0;i<m_NumElements;i++)
	{
		tElementStruct *theElement=m_pElementList+i;

		if ((theElement->Type>0) && (theElement->pElementObject))
		{
			if (theElement->pElementObject->Expression1) 
				((CExpression*)(theElement->pElementObject->Expression1))->ChangeFontSize(factor);
			if (theElement->pElementObject->Expression2) 
				((CExpression*)(theElement->pElementObject->Expression2))->ChangeFontSize(factor);
			if (theElement->pElementObject->Expression3) 
				((CExpression*)(theElement->pElementObject->Expression3))->ChangeFontSize(factor);
		}
	}
	return 0;
}

int CExpression::SetColor(int color)
{
	m_Color=color;
	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((ts->Type!=0) && (ts->Type!=11)  && (ts->Type!=12) && (ts->pElementObject))
			ts->pElementObject->SetColor(-1);
	}
	return 1;
}

// ************************************************************************************
// ************************************************************************************
// ***********************  symbolic calculator routines  *****************************
// ************************************************************************************
// ************************************************************************************

//this structure definses levels of operators
const struct OPERATOR_LEVELS
{
	char comutative;
	char default_operator;
	char operator_list[14];
} Operators[]=
{
	{(char)0,(char)0xFF,{(char)0xFF,(char)0xFE,0}},		//column and row separators (matrix)
	{(char)0,';',       {';',0}},
	{(char)1,',',		{',',0}},
	{(char)1,'=',		{'=','>','<',(char)0xB9,(char)0xBB,(char)0xA3,(char)0xB3,(char)0x01,(char)0x02,0}},
	{(char)1,'+',		{'+','-',(char)0xB1,(char)0xB2,0}},
	{(char)1,(char)0xD7,{(char)0xD7,'/',0}},
	//{(char)1,'*',       {'*',0}},				//convolution
	{(char)0,0,{0}}
};

//three levels are pre-defined (in 'InitCalculator' function) because are often used
int PlusLevel=-1;
int MulLevel=-1;
int EqLevel=-1;


//returns defualt operator for given level 
//(this is somewhat inverse function to 'GetOperatorLevel')
char CExpression::GetDefaultElementType(const int Level)
{
	return Operators[Level].default_operator;
}


//this function returns the size and type of first mathematical element (at the same operator level)
// for example in expression  -a*b*c+g-h*d and for Level that equals '+' operator
// the function will return 6 (length of '-a*b*c') and will set element_type to '-' and has_preoperator to 1)
int CExpression::GetElementLen(const unsigned int StartPos, const unsigned int EndPos, const unsigned int Level, char * element_type, char *has_preoperator)
{
	unsigned char prev_elm_type;
	unsigned char oper_type;
	unsigned char ll;

	tElementStruct *theElement=m_pElementList+StartPos;
	prev_elm_type=theElement->Type;

	if (prev_elm_type==2) {oper_type=(unsigned char)theElement->pElementObject->Data1[0];ll=OperatorLevelTable[oper_type];}
	else if (prev_elm_type==11) {prev_elm_type=2;oper_type=0xFF;ll=0;}
	else if (prev_elm_type==12) {prev_elm_type=2;oper_type=0xFE;ll=0;}
	else goto getelementlen_continue;

	if (ll==Level)
	{
		*element_type=oper_type;
		*has_preoperator=1;
	}
	else
	{
getelementlen_continue:
		*element_type=Operators[Level].default_operator;
		*has_preoperator=0;
	}

	unsigned int i=StartPos+1;
	theElement++;

	for (;i<=EndPos;i++,theElement++)
	{
		int elm_type=theElement->Type;
		if (elm_type==2) {prev_elm_type=2;oper_type=(unsigned char)theElement->pElementObject->Data1[0];ll=OperatorLevelTable[oper_type];}
		else if ((elm_type==11) || (elm_type==12)) {prev_elm_type=2;ll=0;}
		else if (prev_elm_type!=2) {prev_elm_type=elm_type;ll=MulLevel;}
		else {prev_elm_type=elm_type;continue;}

		if (ll<=Level) return i-StartPos;
	}
	
	return i-StartPos;
}


//this function returns level of the lowest-level operator found at specified positions
//if it cannot determine operator level (for example if only one variable is contained)
//then the level of the 'default_oper' is returned (or -1 if 'default_oper'==0)
int CExpression::FindLowestOperatorLevel(const unsigned int StartPos,const unsigned int EndPos,const char default_oper)
{
	//if (EndPos==-1) EndPos=m_NumElements-1;
	if (StartPos==EndPos)
	{
		//the fast case
		if (default_oper) return OperatorLevelTable[(unsigned char)default_oper];
		return -1;
	}

	unsigned char prev_elm_type=2;
	unsigned char Level=0xFF;
	tElementStruct *theElement=m_pElementList+StartPos;
	for (unsigned int i=StartPos;i<=EndPos;i++,theElement++)
	{
		int elm_type=theElement->Type;
		if (elm_type==2) 
		{
			char oper_type=theElement->pElementObject->Data1[0];

			//special handling for leading '+' and '-'.(No operator level can be determined from leading + and -.)
			if ((i==StartPos) && 
				((oper_type=='-') || (oper_type=='+') || (oper_type==(char)0xB1) || (oper_type==(char)0xB2))) 
				continue;

			prev_elm_type=2;
			unsigned char j=OperatorLevelTable[(unsigned char)oper_type];
			if (j<Level) Level=j;
			continue;
		}
		else if ((elm_type==11) || (elm_type==12)) {return (0);} //cannot go any lower level
		else if ((prev_elm_type!=2) && (MulLevel<Level)) Level=MulLevel; 
		prev_elm_type=elm_type;
	}

	if (Level!=0xFF) return (Level);  //level found

	if (default_oper) return (OperatorLevelTable[(unsigned char)default_oper]);
	return (-1);
}

//faster version of the above function - used when the whole expression is to be analyzed
int CExpression::FindLowestOperatorLevel(const char default_oper)
{
	if (m_NumElements==1)
	{
		//the fast case
		if (default_oper) return OperatorLevelTable[(unsigned char)default_oper];
		return -1;
	}

	const unsigned int EndPos=m_NumElements;
	unsigned char prev_elm_type=2;
	unsigned char Level=0xFF;
	tElementStruct *theElement=m_pElementList;
	for (unsigned int i=0;i<EndPos;i++,theElement++)
	{
		int elm_type=theElement->Type;
		if (elm_type==2) 
		{
			char oper_type=theElement->pElementObject->Data1[0];

			//special handling for leading '+' and '-'. 
			//No operator level can be determined from leading + and -.
			if ((i==0) && 
				((oper_type=='+') || (oper_type=='-') || (oper_type==(char)0xB1) || (oper_type==(char)0xB2))) 
				continue;
			prev_elm_type=2;
			unsigned char j=OperatorLevelTable[(unsigned char)oper_type];
			if (j<Level) Level=j;
			continue;
		}
		else if ((elm_type==11) || (elm_type==12)) {return 0;} //cannot go any lower level
		else if ((prev_elm_type!=2) && (MulLevel<Level)) Level=MulLevel; 
		prev_elm_type=elm_type;
	}

	if (Level!=0xFF) {return Level;}  //level found

	if (default_oper) return OperatorLevelTable[(unsigned char)default_oper];
	return -1;
}

//compare this expression (fragment) to the other one (fragment)
//returns non-zero if equal, or zero if non-equal
//'a+b+c' is equal to 'b+c+a'.
int CExpression::CompareExpressions(int StartPos, int EndPos, CExpression* Other, int StartPos2, int EndPos2)
{
	if (Other==NULL) return 0;
	PROFILE_TIME(&PROFILER.CompareExpressions);

	if (EndPos<0) EndPos=m_NumElements-1;
	if (EndPos2<0) EndPos2=Other->m_NumElements-1;

	int Level,Level2;
	if (EndPos==StartPos)
	{
		if (EndPos2==StartPos2)
		{
			//this one single element is to be compared (not used for fraction 'type=4')
			tElementStruct *theElement=m_pElementList+StartPos;
			tElementStruct *theElement2=Other->m_pElementList+StartPos2;

			if (theElement->Type==theElement2->Type)
				if (theElement->Type!=4)
					xreturn (CompareElement(theElement,theElement2)); 

			Level2=MulLevel;
		}
		else if (StartPos2>EndPos2)
		{xreturn (0);}
		else
			Level2=Other->FindLowestOperatorLevel(StartPos2,EndPos2,(char)0xD7);

		Level=MulLevel;
	}
	else if (StartPos>EndPos)
		{xreturn (0);}
	else
	{
		if (EndPos2==StartPos2)
			Level2=MulLevel;
		else if (StartPos2>EndPos2)
		{xreturn (0);}
		else
			Level2=Other->FindLowestOperatorLevel(StartPos2,EndPos2,(char)0xD7);
		Level=FindLowestOperatorLevel(StartPos,EndPos,(char)0xD7);
	}


	if (Level!=Level2) xreturn (0);

	//special compare for multiplication level fragments
	if ((Level==MulLevel) || (Level2==MulLevel))
	{
		int rval=this->StrikeoutCommonFactors(StartPos,EndPos,1,Other,StartPos2,EndPos2,1);
		StrikeoutRemove(StartPos,EndPos);
		Other->StrikeoutRemove(StartPos2,EndPos2);
		xreturn (rval);
	}

	if (Operators[Level].comutative==0)
	{
		//non-comutative comparation - for levels where order is important 'a;b'!='b;a'
		int i=StartPos;
		int j=StartPos2;
		while (1)
		{
			char element_type,element_type2;
			char preoperator,preoperator2;
			int l1=GetElementLen(i,EndPos,Level,&element_type,&preoperator);
			int l2=Other->GetElementLen(j,EndPos2,Level,&element_type2,&preoperator2);
			//if ((l1==0) || (l2==0)) xreturn (0);  //removed for speed
			if (element_type!=element_type2) xreturn (0);
			if (CompareExpressions(i+preoperator,i+l1-1,Other,j+preoperator2,j+l2-1)==0) 
				xreturn (0);
			i+=l1;
			j+=l2;
			if (i>EndPos) break;
			if (j>EndPos2) break;
		}
		if (i!=EndPos+1) xreturn (0);
		if (j!=EndPos2+1) xreturn (0);
		xreturn (1);
	}
	else
	{
		//comutative comparation - for levels where order is un-important 'a+b'='b+a'
		int i=StartPos;
		int OtherLenSum=0;
		while (1)
		{
			char element_type,preoperator;
			int l1=GetElementLen(i,EndPos,Level,&element_type,&preoperator);
			//if (l1==0) goto compare_expressions_earlyexit; //removed for speed
			int j=StartPos2;
			while (1)
			{
				char element_type2,preoperator2;
				tElementStruct *theElement=Other->m_pElementList+j;
				int l2=Other->GetElementLen(j,EndPos2,Level,&element_type2,&preoperator2);
				//if (l2==0) goto compare_expressions_earlyexit; //removed for speed
				if ((element_type==element_type2) && ((theElement->IsSelected&0x08)==0))
				{
					if (CompareExpressions(i+preoperator,i+l1-1,Other,j+preoperator2,j+l2-1)==1) 
					{OtherLenSum+=l2;theElement->IsSelected|=0x08;break;}
				}

				j+=l2;
				if (j>EndPos2) goto compare_expressions_earlyexit; //did not find matching
			}
			i+=l1;
			if (i>EndPos) break;
		}
		if (OtherLenSum+StartPos2!=EndPos2+1) goto compare_expressions_earlyexit;

		//the two expresions are the same
		tElementStruct *te=Other->m_pElementList+StartPos2;
		for (int ii=StartPos2;ii<=EndPos2;ii++,te++) te->IsSelected&=0xF7;
		xreturn (1);
	}

compare_expressions_earlyexit:
	//the  two expression differ
	tElementStruct *te=Other->m_pElementList+StartPos2;
	for (int ii=StartPos2;ii<=EndPos2;ii++,te++) te->IsSelected&=0xF7;
	xreturn (0);
}


typedef struct
{
	unsigned short position;
	char et;
	char p;
} tElementStorage;


//Calculates the given equation. Returns non-zero if any computation is done.
//Returns zero if no computation can be done.
int CExpression::Compute(int StartPos, int EndPos,int ComputationType, int OutsideLevel)
{
	//initial testing of input parameters
	if (EndPos<0) EndPos=m_NumElements-1;
	if (StartPos>EndPos) return 0;
	if (EndPos>m_NumElements-1) return 0;


	//special handling for absolute value or determinants
	if ((this->m_pPaternalExpression==NULL) && 
		((this->m_ParentheseShape=='|') || (this->m_ParentheseShape=='\\') ||(this->m_ParentheseShape=='c') || (this->m_ParentheseShape=='f')) && 
		((this->m_ParenthesesFlags&0x1C)==0) && (StartPos==0) && (EndPos==m_NumElements-1))
	{
		// we do a trick - nest the whole expression into empty one (expand otside)
		// so that computation can be made in "ComputeParenthese"

		CExpression *tmp=new CExpression(NULL,NULL,this->m_FontSize);

		char shape=this->m_ParentheseShape;
		tmp->InsertEmptyElement(0,5,shape);
		CExpression *tmptmp=(CExpression*)tmp->m_pElementList->pElementObject->Expression1;
		tmptmp->CopyExpression(this,0);
		if (tmp->ComputeParentheses(0,5,ComputationType,OutsideLevel))
		{
			if ((tmp->m_NumElements>1) || (tmp->m_pElementList->Type!=5) ||
				(((CExpression*)((tmp->m_pElementList->pElementObject)->Expression1))->m_ParentheseShape!=shape))
				this->CopyExpression(tmp,0);
			else
				this->CopyExpression(tmptmp,0);
			delete tmp;
			int cntr=0;
			while ((Compute(0,m_NumElements-1,ComputationType,OutsideLevel)) && (cntr<50)) cntr++;
			return 1;
		}
		delete tmp;
	}
	

	if ((StartPos==EndPos) && 
		(((m_pElementList+StartPos)->Type<=2) || 
		 ((m_pElementList+StartPos)->Type==11) || 
		 ((m_pElementList+StartPos)->Type==12))) 
		return 0;


	PROFILE_TIME(&PROFILER.Compute);

	int IsComputedTestMask=0;
	int IsComputedTestMask2=0;
	if ((StartPos==0) && (EndPos==m_NumElements-1))
	{
		//if the whole expression is taken in computation, then calculate the mask
		IsComputedTestMask=1<<(ComputationType*2);
		IsComputedTestMask2=IsComputedTestMask<<1;

		//if we already made all computations of this equation (for the same ComputationType)
		//then there is no need to try any further
		if (m_IsComputed&IsComputedTestMask) xreturn (0);
	}

	int Level=FindLowestOperatorLevel(StartPos,EndPos);

	int tmpLevel=Level;
	if (tmpLevel==-1) tmpLevel=PlusLevel;

	//store position of all elements into an structure array
	int pos=StartPos;
	int num_factors=0;
	tElementStorage *es;
	tElementStorage small_storage[5];
	tElementStorage *large_storage=NULL;
	tElementStorage *first_element=small_storage;
	int ElementsReserved=5;
	es=first_element;

	while (1)
	{
		int l=GetElementLen(pos,EndPos,tmpLevel,&(es->et),&(es->p));
		if (l==0) goto compute_exit;
		es->position=(unsigned short)pos;
		es++;
		num_factors++;
		pos+=l;
		if (pos>EndPos) break;
		if (num_factors>=ElementsReserved-1)
		{
			large_storage=(tElementStorage*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,(ElementsReserved+32)*sizeof(tElementStorage));
			memcpy(large_storage,first_element,sizeof(tElementStorage)*ElementsReserved);
			if (ElementsReserved>32) HeapFree(ProcessHeap,0,first_element);
			first_element=large_storage;
			es=first_element+num_factors;
			ElementsReserved+=32;
		}
	}
	es->position=(unsigned short)EndPos+1;



	//computation algorithm for comutative operators - try pairs, every member with every memeber
	int fnd=0;
	if ((Level!=-1) && (Operators[Level].comutative==1) && ((m_IsComputed&IsComputedTestMask2)==0))
	{
		es=first_element;
		for (int i=0;i<num_factors-1;i++)
		{
			tElementStorage *es2=es+1;
			for (int j=i+1;j<num_factors;j++)
			{				
				if (ExecuteComputation(	es->position+es->p,  (es+1)->position-1, es->et,
										es2->position+es2->p,(es2+1)->position-1,es2->et,ComputationType))
				{
					if (IsComputedTestMask)
					{
						//for speed, start from beginning instead of returning
						tElementStorage *es3=es;
						int pos=es3->position;
						num_factors=i;
						EndPos=m_NumElements-1;
						while (1)
						{
							int l=GetElementLen(pos,EndPos,Level,&(es3->et),&(es3->p));
							if (l==0) goto compute_finish;
							es3->position=(unsigned short)pos;
							num_factors++;
							pos+=l;
							es3++;
							if (num_factors>=ElementsReserved-1)
							{
								large_storage=(tElementStorage*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,(ElementsReserved+32)*sizeof(tElementStorage));
								memcpy(large_storage,first_element,sizeof(tElementStorage)*ElementsReserved);
								if (ElementsReserved>32) HeapFree(ProcessHeap,0,first_element);
								first_element=large_storage;
								es3=first_element+num_factors;
								ElementsReserved+=32;
							}

							if (pos>EndPos) break;
						}
						es3->position=(unsigned short)(EndPos+1);
						fnd=1;
						i--;
						es=first_element+i;
						break;
					}
					else
						goto compute_finish;
				}
				es2++;
			}
			es++;
		}
	}
	if (fnd) goto compute_finish;
	m_IsComputed|=IsComputedTestMask2;

	//if still here, then all pairs are tried, but no computation is made
	//Now try computation over individual elements


    

	//no pair can be calculated - try calculate element by element
	if (Level==-1) Level=(OutsideLevel>PlusLevel)?OutsideLevel:PlusLevel;
	int retval=0;
	es=first_element;
	for (int ii=0;ii<num_factors;ii++)
	{
		char element_type=es->et;
		char preoperator=es->p;
		int i=es->position;
		int l1=(es+1)->position-i;

		es++;

		tElementStruct *ts=m_pElementList+i+preoperator;

		if ((l1-preoperator==1) && (Level>=PlusLevel) && (ts->pElementObject))
		{
			//one single element is found
			int rval=0;
			int cnt=0;

			if ((ts->Type==5) && (ts->pElementObject->Expression1))
			{
				//parentheses
				{
					while (((rval=ComputeParentheses(i+preoperator,element_type,ComputationType,Level))<0) && (cnt<50)) cnt++;
					if (cnt) rval=-1;
				}
			}			
			else if ((ts->Type==6) && (ts->pElementObject->Expression1))
			{
				int is_ln=0;
				int is_log=0;
				int is_sincos=0;
				int is_asincos=0;
				int is_hsincos=0;
				if ((strnicmp(ts->pElementObject->Data1,"ln",2)==0) && 
					(strlen(ts->pElementObject->Data1)==2)) is_ln=1;
				if ((strnicmp(ts->pElementObject->Data1,"log",3)==0) && 
					(strlen(ts->pElementObject->Data1)==3)) is_log=1;
				if (strcmp(ts->pElementObject->Data1,"sin")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"cos")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"tg")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"tan")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"cot")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"ctg")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"sec")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"cosec")==0) is_sincos=1;
				if (strcmp(ts->pElementObject->Data1,"csc")==0) is_sincos=1;

				if (strcmp(ts->pElementObject->Data1,"arcsin")==0) is_asincos=1;
				if (strcmp(ts->pElementObject->Data1,"arccos")==0) is_asincos=1;
				if (strcmp(ts->pElementObject->Data1,"arctg")==0) is_asincos=1;
				if (strcmp(ts->pElementObject->Data1,"arctan")==0) is_asincos=1;
				if (strcmp(ts->pElementObject->Data1,"arccot")==0) is_asincos=1;
				if (strcmp(ts->pElementObject->Data1,"arcsec")==0) is_asincos=1;
				if (strcmp(ts->pElementObject->Data1,"arccsc")==0) is_asincos=1;
				
				if (strcmp(ts->pElementObject->Data1,"sh")==0) is_hsincos=1;
				if (strcmp(ts->pElementObject->Data1,"ch")==0) is_hsincos=1;
				if (strcmp(ts->pElementObject->Data1,"th")==0) is_hsincos=1;
				if (strcmp(ts->pElementObject->Data1,"cth")==0) is_hsincos=1;

				if ((is_ln) || (is_log))
				{
					//Logarithm
					while (((rval=ComputeLog(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
					if (cnt) rval=-1;
				}
				else if (is_sincos)
				{
					//trigonometric functions
					while (((rval=ComputeSinCos(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
					if (cnt) rval=-1;
				}
				else if (is_asincos)
				{
					//trigonometric functions
					while (((rval=ComputeASinCos(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
					if (cnt) rval=-1;
				}
				else if (is_hsincos)
				{
					//trigonometric functions
					while (((rval=ComputeHSinCos(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
					if (cnt) rval=-1;
				}
				else goto compute_continue_unknown;
			}
			else if ((ts->Type==4) && (ts->pElementObject->Expression1) &&
				(ts->pElementObject->Expression2))
			{
				//fraction
				while (((rval=ComputeFraction(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
				if (cnt) rval=-1;
			}
			else if ((ts->Type==8) && (ts->pElementObject->Expression1))
			{
				//root
				while (((rval=ComputeRoot(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
				if (cnt) rval=-1;
			}
			else if ((ts->Type==3) && (ts->pElementObject->Expression1) && (ts->pElementObject->Expression2))
			{
				//exponent
				while (((rval=ComputeExponent(i+preoperator,element_type,ComputationType))<0) && (cnt<50)) cnt++;
				if (cnt) rval=-1;
			}
			else
			{
compute_continue_unknown:
				if (ts->pElementObject->Expression1)
					while ((((CExpression*)(ts->pElementObject->Expression1))->Compute(0,-1,ComputationType)) && (cnt<50)) cnt++;
				if (cnt) rval=-1;
				cnt=0;
				if (ts->pElementObject->Expression2)
					while ((((CExpression*)(ts->pElementObject->Expression2))->Compute(0,-1,ComputationType)) && (cnt<50)) cnt++;
				if (cnt) rval=-1;
				cnt=0;
				if (ts->pElementObject->Expression3)
					while ((((CExpression*)(ts->pElementObject->Expression3))->Compute(0,-1,ComputationType)) && (cnt<50)) cnt++;
				if (cnt) rval=-1;
			}

			if (rval>0) goto compute_finish;
			if (rval) retval=1;
		}

		if ((l1-preoperator>1) || (Level<PlusLevel))
		{
			int k=0;
			if ((Level==MulLevel) && ((m_pElementList+i)->Type==2) &&
				(GetOperatorLevel((m_pElementList+i)->pElementObject->Data1[0])==PlusLevel))
			{
				//for example '-a*b'
				k=1;
			}
			
			if (Compute(i+preoperator+k,i+l1-1,ComputationType,Level)) goto compute_finish;
		}
	}

	if (retval) goto compute_finish;

	//flag that we succesfully finished computation of this expression
	m_IsComputed|=IsComputedTestMask;
compute_exit:
	if (large_storage) HeapFree(ProcessHeap,0,large_storage);
	xreturn (0);

compute_finish:
	if (large_storage) HeapFree(ProcessHeap,0,large_storage);
	xreturn (1);
}



//multiplies two elements, if possible
int CExpression::MultiplyElements(int StartPos, int EndPos, int inv, CExpression *Other, int StartPos2, int EndPos2, int inv2,int ComputationType)
{
	PROFILE_TIME(&PROFILER.MultiplyElements);
	if (EndPos==-1) EndPos=m_NumElements-1;
	if (EndPos2==-1) EndPos2=Other->m_NumElements-1;

	double N1,N2;
	int prec1,prec2;
	int is_pure1=IsPureNumber(StartPos,EndPos-StartPos+1,&N1,&prec1);
	int is_pure2=Other->IsPureNumber(StartPos2,EndPos2-StartPos2+1,&N2,&prec2);
	int Level1=FindLowestOperatorLevel(StartPos,EndPos,(char)0xD7);
	int Level2=Other->FindLowestOperatorLevel(StartPos2,EndPos2,(char)0xD7);


	// ***************************************************************************
	// multiplications with 'zero' and 'one'
	// ***************************************************************************
	if (is_pure1)
	{
		int minus_adjusted=0;
		if ((fabs(N1+1.0)<1e-100) && ((m_pElementList+StartPos)->Type==2) && //minus one
			(GetOperatorLevel((m_pElementList+StartPos)->pElementObject->Data1[0])==PlusLevel))
		{StartPos++;N1=-N1;minus_adjusted=1;}

		if (fabs(N1-1.0)<1e-100)
		{
			int ok=1;

			//we will not do it if the form is '1/something' (at the beginning of the group)
			if ((StartPos<m_NumElements-1) &&
				((m_pElementList+StartPos+1)->Type==2) && 
				((m_pElementList+StartPos+1)->pElementObject->Data1[0]=='/'))
			{
				if (StartPos==0) ok=0;
				else if ((m_pElementList+StartPos-1)->Type==11) ok=0;
				else if ((m_pElementList+StartPos-1)->Type==12) ok=0;
				else if (((m_pElementList+StartPos-1)->Type==2) && ((m_pElementList+StartPos-1)->pElementObject) &&
					(GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])<MulLevel))
					ok=0;
			}

			//also not if the 'one' or 'minus one' is the only variable in expression
			if (FindLowestOperatorLevel()==-1)
				xreturn (0);
		
			if (ok)
			{
				RemoveSequence(MulLevel,StartPos,EndPos);
				xreturn (1);
			}

		}

		if ((fabs(N1)<1e-100) && (inv==1))
		{
			Other->RemoveSequence(MulLevel,StartPos2,EndPos2);
			for (int k=StartPos;k<=EndPos;k++)
				DeleteElement(StartPos);
			if ((StartPos>0) && ((m_pElementList+StartPos-1)->Type==2) &&
				(GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])==MulLevel))
				{StartPos--;DeleteElement(StartPos);}
			InsertEmptyElement(StartPos,1,'0');
			xreturn (1);
		}
		if (minus_adjusted) //minus one
			{StartPos--;N1=-N1;}
	}

	if (is_pure2)
	{
		int minus_adjusted=0;
		if ((fabs(N2+1.0)<1e-100) && ((Other->m_pElementList+StartPos2)->Type==2) && //minus one
			(GetOperatorLevel((Other->m_pElementList+StartPos2)->pElementObject->Data1[0])==PlusLevel))
		{StartPos2++;N2=-N2;minus_adjusted=1;}

		if (fabs(N2-1.0)<1e-100)
		{
			int ok=1;

			//we will not do it if the form is '1/something' (at the beginning of the group)
			if ((StartPos2<Other->m_NumElements-1) &&
				((Other->m_pElementList+StartPos2+1)->Type==2) && 
				((Other->m_pElementList+StartPos2+1)->pElementObject->Data1[0]=='/'))
			{
				if (StartPos2==0) ok=0;
				else if ((Other->m_pElementList+StartPos2-1)->Type==11) ok=0;
				else if ((Other->m_pElementList+StartPos2-1)->Type==12) ok=0;
				else if (((Other->m_pElementList+StartPos2-1)->Type==2) && ((Other->m_pElementList+StartPos2-1)->pElementObject) &&
					(GetOperatorLevel((Other->m_pElementList+StartPos2-1)->pElementObject->Data1[0])<MulLevel))
					ok=0;
			}

			//also not if the 'one' or 'minus one' is the only variable in expression
			if (Other->FindLowestOperatorLevel()==-1)
				xreturn (0);
		
			if (ok)
			{
				Other->RemoveSequence(MulLevel,StartPos2,EndPos2);
				xreturn (1);
			}
		}

		if ((fabs(N2)<1e-100) && (inv2==1))
		{
			for (int k=StartPos2;k<=EndPos2;k++)
				Other->DeleteElement(StartPos2);
			if ((StartPos2>0) && ((Other->m_pElementList+StartPos2-1)->Type==2) &&
				(GetOperatorLevel((Other->m_pElementList+StartPos2-1)->pElementObject->Data1[0])==MulLevel))
				{StartPos2--;Other->DeleteElement(StartPos2);}
			Other->InsertEmptyElement(StartPos2,1,'0');

			RemoveSequence(MulLevel,StartPos,EndPos);
			xreturn (1);
		}
		if (minus_adjusted) //minus one
			{StartPos2--;N2=-N2;}
	}


	if ((StartPos==EndPos) && (!is_pure1))
	{
		tElementStruct *ts=m_pElementList+StartPos;
		if (ts->Type==5)
		{
			int retval=((CExpression*)(ts->pElementObject->Expression1))->MultiplyElements(0,-1,inv,Other,StartPos2,EndPos2,inv2,ComputationType);
			if (retval) xreturn (retval);
		}
	}

	if ((StartPos2==EndPos2) && (!is_pure2))
	{
		tElementStruct *ts=Other->m_pElementList+StartPos2;
		if (ts->Type==5)
		{
			if (ComputationType==10) //special handling for 1/(2/3)
				((CExpression*)(ts->pElementObject->Expression1))->Compute(0,-1,ComputationType,GetOperatorLevel('/'));
			int retval=MultiplyElements(StartPos,EndPos,inv,(CExpression*)(ts->pElementObject->Expression1),0,-1,inv2,ComputationType);
			if (retval) xreturn (retval);
		}
	}

	if ((StartPos==EndPos) && (!is_pure1) && ((Level2>=MulLevel) || (inv!=inv2)))
	{
		tElementStruct *ts=m_pElementList+StartPos;
		if (ts->Type==4)
		{
			int retval=((CExpression*)(ts->pElementObject->Expression1))->MultiplyElements(0,-1,inv,Other,StartPos2,EndPos2,inv2,ComputationType);
			if (retval) xreturn (retval);
			retval=((CExpression*)(ts->pElementObject->Expression2))->MultiplyElements(0,-1,-inv,Other,StartPos2,EndPos2,inv2,ComputationType);
			if (retval) xreturn (retval);
		}
	}

	if ((StartPos2==EndPos2) && (!is_pure2) && ((Level1>=MulLevel) || (inv!=inv2)))
	{
		tElementStruct *ts=Other->m_pElementList+StartPos2;
		if (ts->Type==4)
		{
			int retval=MultiplyElements(StartPos,EndPos,inv,(CExpression*)(ts->pElementObject->Expression1),0,-1,inv2,ComputationType);
			if (retval) xreturn (retval);
			retval=MultiplyElements(StartPos,EndPos,inv,(CExpression*)(ts->pElementObject->Expression2),0,-1,-inv2,ComputationType);
			if (retval) xreturn (retval);
		}
	}

	//we never multiply numbers in form A*10^X (where 1<A<10, and X is integer)
	//this is to avoid infinite loop for pure numbers in scientific form
	//if (Other==this)  //this line is comented out to prevent infinte loop: (A*10^x)*10^y
	if ((StartPos2==EndPos2) && ((Other->m_pElementList+StartPos2)->Type==3) && ((Other->m_pElementList+StartPos2)->pElementObject))
	if ((StartPos==EndPos) && ((m_pElementList+StartPos)->Type==1))
	if (inv2==inv)
	{
		CExpression *argument=(CExpression*)((Other->m_pElementList+StartPos2)->pElementObject->Expression1);
		CExpression *exponent=(CExpression*)((Other->m_pElementList+StartPos2)->pElementObject->Expression2);
		int prec1,prec2,prec3;
		double N1,N2,N3;
		if (IsPureNumber(StartPos,1,&N1,&prec1))
			if ((N1>=(1.0-1e-100)) && (N1<10.0))
				if (argument->IsPureNumber(0,argument->m_NumElements,&N2,&prec2))
					if (fabs(N2-10.0)<1e-100)
						if (exponent->IsPureNumber(0,exponent->m_NumElements,&N3,&prec3))
							if (fabs(N3-(int)N3)<1e-100) 
								xreturn (0);

	}

	//*********************************************************************
	//multiplication of two pure numbers 
	//*********************************************************************
	if ((is_pure1) && (is_pure2) && 
		(((fabs(N1-1.0)>1e-100) && (fabs(N2-1.0)>1e-100)) || (ComputationType==10)))
	{
		int prec=max(prec1,prec2);
		if (((prec1>0) && (prec2>0)) || (inv==inv2) || (ComputationType==10))
		{
			double result;
			int ResultPos=-1;
			CExpression *Res;
			if ((inv==-1) && (inv2==1) && (fabs(N1)>1e-100))
			{
				result=N2/N1;
				ResultPos=StartPos2;
				Res=Other;
			}
			if ((inv2==-1) && (inv==1) && (fabs(N2)>1e-100))
			{
				result=N1/N2;
				ResultPos=StartPos;
				Res=this;
			}
			if (inv==inv2)
			{
				result=N1*N2;
				ResultPos=StartPos;
				Res=this;
			}
			if (ResultPos>=0)
			{
				for (int k=StartPos2;k<=EndPos2;k++)
					Other->DeleteElement(StartPos2);
				if ((fabs(result-1.0)>1e-100) && (Res==Other) && (ResultPos==StartPos2))
					Other->GenerateASCIINumber(result,(long long)(result+((result>=0)?0.01:-0.01)),(result-(long long)(result+((result>=0)?0.01:-0.01)))<1e-100?1:0,prec,ResultPos);
				else if (Other->m_pElementList->Type==0)
					Other->InsertEmptyElement(0,1,'1');
				else if ((Other->m_NumElements==1) && (Other->m_pElementList->Type==2) &&
					(GetOperatorLevel(Other->m_pElementList->pElementObject->Data1[0])==PlusLevel))
					Other->InsertEmptyElement(1,1,'1');
				else
				{
					if ((StartPos2>0) && ((Other->m_pElementList+StartPos2-1)->Type==2) &&
						(GetOperatorLevel((Other->m_pElementList+StartPos2-1)->pElementObject->Data1[0])==MulLevel))
						{StartPos2--;Other->DeleteElement(StartPos2);}
					if (((Other->m_pElementList+StartPos2)->Type==2) && 
						((StartPos2==0) || 
						((Other->m_pElementList+StartPos2-1)->Type==11) || 
						((Other->m_pElementList+StartPos2-1)->Type==12) || 
						(((Other->m_pElementList+StartPos2-1)->Type==2) && (GetOperatorLevel((Other->m_pElementList+StartPos2-1)->pElementObject->Data1[0])<MulLevel))))
					{
						if ((Other->m_pElementList+StartPos2)->pElementObject->Data1[0]==(char)0xD7)
							Other->DeleteElement(StartPos2);
						if ((Other->m_pElementList+StartPos2)->pElementObject->Data1[0]=='/')
							Other->InsertEmptyElement(StartPos2,1,'1');
					}
				}


				for (int k=StartPos;k<=EndPos;k++)
					DeleteElement(StartPos);
				if ((fabs(result-1.0)>1e-100) && (Res==this) && (ResultPos==StartPos))
					GenerateASCIINumber(result,(long long)(result+((result>=0)?0.01:-0.01)),(result-(long long)(result+((result>=0)?0.01:-0.01)))<1e-100?1:0,prec,ResultPos);
				else if (m_pElementList->Type==0)
					InsertEmptyElement(0,1,'1');
				else if ((m_NumElements==1) && (m_pElementList->Type==2) &&
					(GetOperatorLevel(m_pElementList->pElementObject->Data1[0])==PlusLevel))
					InsertEmptyElement(1,1,'1');
				else 
				{
					if ((StartPos>0) && ((m_pElementList+StartPos-1)->Type==2) &&
						(GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])==MulLevel))
						{StartPos--;DeleteElement(StartPos);}
					if (((m_pElementList+StartPos)->Type==2) && 
						((StartPos==0) || 
						((m_pElementList+StartPos-1)->Type==11) || 
						((m_pElementList+StartPos-1)->Type==12) || 
						(((m_pElementList+StartPos-1)->Type==2) && (GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])<MulLevel))))
					{
						if ((m_pElementList+StartPos)->pElementObject->Data1[0]==(char)0xD7)
							DeleteElement(StartPos);
						if ((m_pElementList+StartPos)->pElementObject->Data1[0]=='/')
							InsertEmptyElement(StartPos,1,'1');
					}
				}

				xreturn (1);
			}
		}
		else if ((fabs(N1)<1e+9) && (fabs(N2)<1e+9))
		{
			//reduces two pure numbers
			if (ReduceTwoNumbers(&N1,&N2))
			{
				long long t1=(long long)(N1+((N1>=0)?0.01:-0.01));
				long long t2=(long long)(N2+((N2>=0)?0.01:-0.01));
				for (int k=StartPos2;k<=EndPos2;k++)
					Other->DeleteElement(StartPos2);
				Other->GenerateASCIINumber((double)t2,t2,1,0,StartPos2);
				for (int k=StartPos;k<=EndPos;k++)
					DeleteElement(StartPos);
				GenerateASCIINumber((double)t1,t1,1,0,StartPos);
				xreturn (1);
			}
		}
	}



	//we have two expressions that are divided - check if it is posible to divide them
	if ((inv!=inv2) && (Level1==PlusLevel) && (Level2==PlusLevel))
	{
		int chg=0;
		StrikeoutCommonFactors(StartPos,EndPos,1,Other,StartPos2,EndPos2,1,NULL,1);
		if (Other->StrikeoutRemove(StartPos2,EndPos2,1)) chg=1;
		if (StrikeoutRemove(StartPos,EndPos,1)) chg=1;
		if (chg)
		{
			if ((StartPos2) && ((Other->m_pElementList+StartPos2-1)->Type==2) &&
				(GetOperatorLevel((Other->m_pElementList+StartPos2-1)->pElementObject->Data1[0])==MulLevel))
				Other->DeleteElement(--StartPos2);
			if (((Other->m_pElementList)->Type==2) &&
				(Other->m_pElementList)->pElementObject->Data1[0]=='/')
				Other->InsertEmptyElement(0,1,'1');
			if (((Other->m_pElementList)->Type==2) &&
				(Other->m_pElementList)->pElementObject->Data1[0]==(char)0xD7)
				Other->DeleteElement(0);
			if (Other->m_pElementList->Type==0) 
				Other->InsertEmptyElement(0,1,'1');
			if ((StartPos) && ((m_pElementList+StartPos-1)->Type==2) &&
				(GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])==MulLevel))
				DeleteElement(--StartPos);
			if (((m_pElementList)->Type==2) &&
				(m_pElementList)->pElementObject->Data1[0]=='/')
				InsertEmptyElement(0,1,'1');
			if (((m_pElementList)->Type==2) &&
				(m_pElementList)->pElementObject->Data1[0]==(char)0xD7)
				DeleteElement(0);
			if (m_pElementList->Type==0)
				InsertEmptyElement(0,1,'1');
			xreturn (1);
		}
	}



	//*********************************************************************
	//multiplication with parenthese
	//*********************************************************************
	/*if (ComputationType==3)  //this code is obsolete
	{
		tElementStruct *ts=NULL;
		CExpression *pe=NULL;
		int VeryStart,VeryEnd;
		if (Level1==PlusLevel) {ts=m_pElementList+StartPos;pe=Other;VeryStart=StartPos2;VeryEnd=EndPos2;}
		else if (Level2==PlusLevel) {ts=Other->m_pElementList+StartPos2;pe=this;VeryStart=StartPos;VeryEnd=EndPos;}

		if (ts)
		{
			int pp=0;
			while (1)
			{
				VeryStart--;
				if (VeryStart<0) break;
				if ((pe->m_pElementList+VeryStart)->Type==11) break;
				if ((pe->m_pElementList+VeryStart)->Type==12) break;
				if (((pe->m_pElementList+VeryStart)->Type==2) && ((pe->m_pElementList+VeryStart)->pElementObject) &&
					(GetOperatorLevel((pe->m_pElementList+VeryStart)->pElementObject->Data1[1])<MulLevel)) break;
			}
			VeryStart++;
			if ((VeryStart>0) && ((pe->m_pElementList+VeryStart-1)->Type==2) && ((pe->m_pElementList+VeryStart-1)->pElementObject) &&
				(GetOperatorLevel((pe->m_pElementList+VeryStart-1)->pElementObject->Data1[0])==PlusLevel))
			{
				pp=1;
				VeryStart--;
			}
			while (1)
			{
				VeryEnd++;
				if (VeryEnd>pe->m_NumElements-1) break;
				if ((pe->m_pElementList+VeryEnd)->Type==11) break;
				if ((pe->m_pElementList+VeryEnd)->Type==12) break;
				if (((pe->m_pElementList+VeryEnd)->Type==2) && ((pe->m_pElementList+VeryEnd)->pElementObject) &&
					(GetOperatorLevel((pe->m_pElementList+VeryEnd)->pElementObject->Data1[1])<MulLevel)) break;
			}
			VeryEnd--;

			if (VeryEnd-VeryStart<pe->m_NumElements-1)
			{
				CExpression *test1=new CExpression(NULL,NULL,100);
				CExpression *test2=new CExpression(NULL,NULL,100);
				for (int kk=0;kk<VeryStart;kk++)
					test1->InsertElement(pe->m_pElementList+kk,test1->m_NumElements);
				for (int kk=VeryStart;kk<=VeryEnd;kk++)
					test2->InsertElement(pe->m_pElementList+kk,test2->m_NumElements);
				for (int kk=VeryEnd+1;kk<pe->m_NumElements;kk++)
					test1->InsertElement(pe->m_pElementList+kk,test1->m_NumElements);

				while (test1->Compute(0,test1->m_NumElements-1,0));
				while (test2->Compute(0,test2->m_NumElements-1,0));
				int level=test1->FindLowestOperatorLevel();
				int level2=test2->FindLowestOperatorLevel();


				if ((level==-1) || (level>PlusLevel)) level=PlusLevel;
				if ((level2==PlusLevel) && (level==level2))
				{
					int found_identical_summand=0;
					int pos=0;
					while (!found_identical_summand)
					{
						char et,p;
						int l=test1->GetElementLen(pos,test1->m_NumElements-1,level2,&et,&p);
						if (l==0) break;

						int pos2=0;
						while (!found_identical_summand)
						{
							char et2,p2;
							int l2=test2->GetElementLen(pos2,test2->m_NumElements-1,level2,&et2,&p2);
							if (l2==0) break;

							tPureFactors PF;
							PF.N1=PF.N2=PF.N3=PF.N4=1.0;
							PF.is_frac1=PF.is_frac2=0;
							PF.prec1=PF.prec2=0;
							int rv=test1->StrikeoutCommonFactors(pos+p,pos+l-1,1,test2,pos2+p2,pos2+l2-1,1,&PF);
							if (rv) found_identical_summand=1;
							test1->StrikeoutRemove(pos+p,pos+l-1);
							test2->StrikeoutRemove(pos2+p2,pos2+l2-1);

							pos2+=l2;
							if (pos2>test2->m_NumElements-1) break;
						}

						pos+=l;
						if (pos>test1->m_NumElements-1) break;
					}

					if (found_identical_summand)
					{
						if (test2->m_pElementList->Type==2) pp=0;
						for (int kk=VeryStart+pp;kk<=VeryEnd;kk++)
							pe->DeleteElement(VeryStart+pp);
						for (int kk=0;kk<test2->m_NumElements;kk++)
							pe->InsertElement(test2->m_pElementList+kk,VeryStart+pp+kk);

						xreturn (1);
					}
				}
			}
		}
	}*/
	if (ComputationType==0)
	{
		tElementStruct *ts=NULL;
		CExpression *pe=NULL;
		int at_front=0;
		int start,end;
		int invert;
		if ((Level1==GetOperatorLevel((char)0xFF)) && (Level2==Level1))
		{
			//special case - matrix multiplication
			int rows1=1,rows2=1,columns1=1,columns2=1;
			if (!GetMatrixSize(&rows1,&columns1)) xreturn (0);
			if (!Other->GetMatrixSize(&rows2,&columns2)) xreturn (0);
			if ((columns1!=rows2) || (this->m_ParentheseShape!=Other->m_ParentheseShape) ||
				((this->m_ParentheseShape=='|') || (this->m_ParentheseShape=='\\'))) // not multipy determinants
				xreturn (0);
			int rows=rows1;
			int columns=columns2;
			CExpression *tmp=new CExpression(NULL,NULL,100);
			for (int ii=0;ii<rows;ii++)
			{
				for (int jj=0;jj<columns;jj++)
				{
					for (int kk=0;kk<columns1;kk++)
					{
						if (kk)
							tmp->InsertEmptyElement(tmp->m_NumElements,2,'+');
						tmp->InsertEmptyElement(tmp->m_NumElements,5,'(');
						CExpression *f1=(CExpression*)((tmp->m_pElementList+tmp->m_NumElements-1)->pElementObject->Expression1);
						tmp->InsertEmptyElement(tmp->m_NumElements,5,'(');
						CExpression *f2=(CExpression*)((tmp->m_pElementList+tmp->m_NumElements-1)->pElementObject->Expression1);
						int pos=this->FindMatrixElement(ii,kk,0);
						if (pos<0) pos=0;
						while ((pos<this->m_NumElements) && ((this->m_pElementList+pos)->Type<11))
						{
							f1->InsertElement(this->m_pElementList+pos,f1->m_NumElements);
							pos++;
						}
						int pos2=Other->FindMatrixElement(kk,jj,0);
						if (pos2<0) pos2=0;
						while ((pos2<Other->m_NumElements) && ((Other->m_pElementList+pos2)->Type<11))
						{
							f2->InsertElement(Other->m_pElementList+pos2,f2->m_NumElements);
							pos2++;
						}
					}
					if (jj<columns-1) tmp->InsertEmptyElement(tmp->m_NumElements,11,0);
				}
				if (ii<rows-1) tmp->InsertEmptyElement(tmp->m_NumElements,12,0);
			}
			int pst=this->m_ParenthesesFlags;
			int shp=this->m_ParentheseShape;
			this->CopyExpression(tmp,0);
			this->m_ParenthesesFlags=pst;
			this->m_ParentheseShape=shp;
			Other->Delete();
			Other->InsertEmptyElement(0,1,'1');
			delete tmp;
			xreturn (1);

		}
		if ((Level1<=PlusLevel) && ((inv==1) || (inv==inv2))) 
			{pe=Other;ts=m_pElementList+StartPos;start=StartPos2;end=EndPos2;invert=inv2;}
		else if ((Level2<=PlusLevel) && ((inv2==1) || (inv==inv2)))
			{pe=this;ts=Other->m_pElementList+StartPos2;start=StartPos;end=EndPos;invert=inv;at_front=1;}


		if ((ts) && (start==end) && ((pe->m_pElementList+start)->Type==5))
		{
			CExpression *tt=(CExpression*)((pe->m_pElementList+start)->pElementObject->Expression1);
			if (tt->FindLowestOperatorLevel((char)0xD7)==GetOperatorLevel((char)0xFF))
				ts=NULL;
		}


		if (ts) 
		{
			//first we will expand multiplication
			if (invert==1)
			{
				do
				{
					if (end+1>=pe->m_NumElements) break;
					if ((pe->m_pElementList+end+1)->Type==11) break;
					if ((pe->m_pElementList+end+1)->Type==12) break;
					if (((pe->m_pElementList+end+1)->Type==2) && ((pe->m_pElementList+end+1)->pElementObject) &&
						(GetOperatorLevel((pe->m_pElementList+end+1)->pElementObject->Data1[0])<MulLevel)) break;
					if ((pe->m_pElementList+end+1)->Type==5) break;
					if ((pe->m_pElementList+end+1)->Type==4) break;
					end++;
				} while (1);
			}

			if ((pe->m_pElementList+end)->Type==2) end--;


			CExpression *arg=(pe==this)?Other:this;
			int InsideLevel=arg->FindLowestOperatorLevel();
			if ((InsideLevel==GetOperatorLevel((char)0xFF)) && ((arg->m_ParentheseShape=='|') || (arg->m_ParentheseShape=='\\')))
				InsideLevel=-1; //don't do it for determinants
			if ((InsideLevel!=-1) && (InsideLevel<MulLevel))
			{
				int is_frac;
				if ((start>0) && ((pe->m_pElementList+start-1)->Type==2) &&
					((pe->m_pElementList+start-1)->pElementObject->Data1[0]=='/'))
					is_frac=0;
				else
					is_frac=1;

				int pos=0;
				while (1)
				{
					char p,et;
					int l=arg->GetElementLen(pos,arg->m_NumElements-1,InsideLevel,&et,&p);
					if (l==0) break;
					
					int lv2=arg->FindLowestOperatorLevel(pos+p,pos+l-1,(char)0xD7);
					if (lv2<MulLevel)
					{
						arg->InsertEmptyElement(pos+p,5,'(');
						CExpression *tmp=(CExpression*)((arg->m_pElementList+pos+p)->pElementObject->Expression1);
						for (int iii=pos+p+1;iii<pos+l+1;iii++)
						{
							tmp->InsertElement(arg->m_pElementList+pos+p+1,tmp->m_NumElements);
							arg->DeleteElement(pos+p+1);
						}
						l=1+p;
					}

					if ((at_front) && (inv==inv2))
					{
						int ll=pe->FindLowestOperatorLevel(start,end,(char)0xD7);
						if ((ll<MulLevel) || ((pe->m_pElementList+start)->Type==2))
						{
							arg->InsertEmptyElement(pos+p,5,'(');
							CExpression *tmp=(CExpression*)((arg->m_pElementList+pos+p)->pElementObject->Expression1);
							l++;
							for (int kk=start;kk<=end;kk++)
								tmp->InsertElement(pe->m_pElementList+kk,kk-start);
						}
						else
							for (int kk=start;kk<=end;kk++,l++)
								arg->InsertElement(pe->m_pElementList+kk,pos+p+kk-start);
					}
					else
					{
						int use_inversion=0;
						if ((invert==-1) && (inv!=inv2)) use_inversion=1;

						if ((!use_inversion) || (!is_frac))
						{
							if (use_inversion)
							{
								arg->InsertEmptyElement(pos+l,2,'/');
								l++;
							}

							int ll=pe->FindLowestOperatorLevel(start,end,(char)0xD7);
							if ((ll<MulLevel) || ((pe->m_pElementList+start)->Type==2) ||
								((pe->m_NumElements>1) && (use_inversion)))
							{
								arg->InsertEmptyElement(pos+l,5,'(');
								CExpression *tmp=(CExpression*)((arg->m_pElementList+pos+l)->pElementObject->Expression1);
								l++;
								for (int kk=start;kk<=end;kk++)
									tmp->InsertElement(pe->m_pElementList+kk,kk-start);
							}
							else
								for (int kk=start;kk<=end;kk++,l++)
									arg->InsertElement(pe->m_pElementList+kk,pos+l);							
						}
						else
						{

							arg->InsertEmptyElement(pos+l,4,0);
							CExpression *n=(CExpression*)((arg->m_pElementList+pos+l)->pElementObject->Expression1);
							CExpression *d=(CExpression*)((arg->m_pElementList+pos+l)->pElementObject->Expression2);
							n->InsertEmptyElement(0,1,'1');
							l++;
							for (int kk=start;kk<=end;kk++)
								d->InsertElement(pe->m_pElementList+kk,d->m_NumElements);	
						}
					}

					pos+=l;
					if (pos>arg->m_NumElements-1) break;
				}
				
				for (int kk=start;kk<=end;kk++)
					pe->DeleteElement(start);
				if ((at_front) && (start<pe->m_NumElements) && ((pe->m_pElementList+start)->Type==2) && ((pe->m_pElementList+start)->pElementObject->Data1[0]==(char)0xD7))
				{
					pe->DeleteElement(start); //deleting the multiplication dot
				}
				if ((!at_front) && (start-1<pe->m_NumElements) && (start-1>0) && ((pe->m_pElementList+start-1)->Type==2) && 
					(((pe->m_pElementList+start-1)->pElementObject->Data1[0]==(char)0xD7) || ((pe->m_pElementList+start-1)->pElementObject->Data1[0]=='/')))
				{
					pe->DeleteElement(start-1); //deleting the multiplication dot
				}
				if ((start==1) && ((pe->m_pElementList+start-1)->Type==2) && ((pe->m_pElementList+start-1)->pElementObject) &&
					(GetOperatorLevel((pe->m_pElementList+start-1)->pElementObject->Data1[0])==PlusLevel))
				{
					if ((pe->m_pElementList+start-1)->pElementObject->Data1[0]=='+') 
						pe->DeleteElement(start-1);
					else
						pe->InsertEmptyElement(start,1,'1');
				}
				if (pe->m_pElementList->Type==0)
					pe->InsertEmptyElement(0,1,'1');


				//NOTICE: These two lines uncomented can make computation faster
				//        but the result looks not nice: try: [a/(b+c) + d/(e+f)]^4
				//        one can fix the ComputationType to 1, but then there is no benefit????
				//int cnt=0;
				//while ((arg->Compute(0,arg->m_NumElements-1,ComputationType)) && (cnt<50)) cnt++;
				xreturn (1);
			}
		}
	}



	//jump over + or - preoperator (examples are expressions +a*b*c)
	if (((m_pElementList+StartPos)->Type==2) && (Level1==MulLevel) &&
		((m_pElementList+StartPos)->pElementObject) &&
		(Operators[GetOperatorLevel((m_pElementList+StartPos)->pElementObject->Data1[0])].default_operator=='+'))
	{
		StartPos++;
	}
	if (((Other->m_pElementList+StartPos2)->Type==2) && (Level2==MulLevel) &&
		((Other->m_pElementList+StartPos2)->pElementObject) &&
		(Operators[GetOperatorLevel((Other->m_pElementList+StartPos2)->pElementObject->Data1[0])].default_operator=='+'))
	{
		StartPos2++;
	}

	if ((StartPos<EndPos) && (!is_pure1))
	{
		if (Level1==MulLevel)
		{
			int pos=StartPos;
			while (1)
			{
				char et,p;
				int l=GetElementLen(pos,EndPos,Level1,&et,&p);
				if (l==0) break;

				int rval=MultiplyElements(pos+p,pos+l-1,(et=='/')?-inv:inv,Other,StartPos2,EndPos2,inv2,ComputationType);
				if (rval) xreturn (rval);

				pos+=l;
				if (pos>EndPos) break;
			}
			xreturn (0);
		}
	}


	if ((StartPos2<EndPos2) && (!is_pure2))
	{
		if (Level2==MulLevel)
		{
			int pos=StartPos2;
			while (1)
			{
				char et,p;
				int l=Other->GetElementLen(pos,EndPos2,Level2,&et,&p);
				if (l==0) break;

				int rval=MultiplyElements(StartPos,EndPos,inv,Other,pos+p,pos+l-1,(et=='/')?-inv2:inv2,ComputationType);
				if (rval) xreturn (rval);

				pos+=l;
				if (pos>EndPos2) break;
			}
			xreturn (0);
		}
	}

	if ((StartPos2==EndPos2) && (!is_pure2) && (StartPos==EndPos) && (!is_pure1) && (ComputationType==2))
	{
		tElementStruct *ts=m_pElementList+StartPos;
		tElementStruct *ts2=Other->m_pElementList+StartPos2;

		// multiplication of two roots
		
		if ((ts->Type==8) && (ts2->Type==8))
		{
			CExpression *base1=(CExpression*)(ts->pElementObject->Expression2);
			CExpression *base2=(CExpression*)(ts2->pElementObject->Expression2);
			if (((base1==NULL) && (base2==NULL)) || 
				((base1) && (base2) && (base1->CompareExpressions(0,base1->m_NumElements-1,base2,0,base2->m_NumElements-1))))
			{
				
				CExpression *arg1=(CExpression*)(ts->pElementObject->Expression1);
				CExpression *arg2=(CExpression*)(ts2->pElementObject->Expression1);

				int lvl1=arg1->FindLowestOperatorLevel((char)0xD7);
				int lvl2=arg2->FindLowestOperatorLevel((char)0xD7);

				int neg1=0;
				int neg2=0;

				if (lvl1>=MulLevel)
				{
					tPureFactors PF;
					PF.N1=PF.N2=1.0;
					PF.is_frac1=0;
					PF.prec1=0;
					arg1->StrikeoutCommonFactors(0,arg1->m_NumElements-1,1,NULL,0,0,1,&PF);
					arg1->StrikeoutRemove(0,arg1->m_NumElements-1);
					if (PF.N1*PF.N2<0) neg1=1;
				}
				if (lvl2>=MulLevel)
				{
					tPureFactors PF;
					PF.N1=PF.N2=1.0;
					PF.is_frac1=0;
					PF.prec1=0;
					arg2->StrikeoutCommonFactors(0,arg2->m_NumElements-1,1,NULL,0,0,1,&PF);
					arg2->StrikeoutRemove(0,arg2->m_NumElements-1);
					if (PF.N1*PF.N2<0) neg2=1;
				}
				if ((neg1==0) || (neg2==0)) //don't do it for two negative numbers, for example 'sqrt(-1) * sqrt(-1)' is not equal to 'sqrt(1)'
				{
					if (inv!=inv2)
					{
						arg1->InsertEmptyElement(0,4,0);
						CExpression *n=(CExpression*)(arg1->m_pElementList->pElementObject->Expression1);
						CExpression *d=(CExpression*)(arg1->m_pElementList->pElementObject->Expression2);
						n->InsertSequence('+',0,arg1,1,arg1->m_NumElements-1);
						while (arg1->m_NumElements>1)
							arg1->DeleteElement(1);
						d->InsertSequence('+',0,arg2,0,arg2->m_NumElements-1);
					}
					else
					{
						if (lvl1<MulLevel)
						{
							arg1->InsertEmptyElement(0,5,'(');
							CExpression *a=(CExpression*)((arg1->m_pElementList)->pElementObject->Expression1);
							int ttt=arg1->m_NumElements;
							for (int kk=1;kk<ttt;kk++)
							{
								a->InsertElement(arg1->m_pElementList+1,a->m_NumElements);
								arg1->DeleteElement(1);
							}
						}

						arg1->InsertSequence((char)0xD7,arg1->m_NumElements,arg2,0,arg2->m_NumElements-1);
					}
					Other->DeleteElement(StartPos2);
					if ((StartPos2==1) && (Other->m_pElementList->Type==2) &&
						(GetOperatorLevel(Other->m_pElementList->pElementObject->Data1[0])==PlusLevel))
						Other->InsertEmptyElement(StartPos2,1,'1');
					if ((StartPos2==0) && (Other->m_pElementList->Type==0)) Other->InsertEmptyElement(0,1,'1');
					xreturn (1);
				}
			}
		}

		//multiplication of two exponents 


		if ((ts->Type==3) && (ts2->Type==3))
		{
			CExpression *base1=(CExpression*)(ts->pElementObject->Expression2);
			CExpression *base2=(CExpression*)(ts2->pElementObject->Expression2);
			if ((base1) && (base2) && (base1->CompareExpressions(0,base1->m_NumElements-1,base2,0,base2->m_NumElements-1)))
			{
				CExpression *arg1=(CExpression*)(ts->pElementObject->Expression1);
				CExpression *arg2=(CExpression*)(ts2->pElementObject->Expression1);

				if (inv!=inv2)
				{
					arg1->InsertEmptyElement(0,4,0);
					CExpression *n=(CExpression*)(arg1->m_pElementList->pElementObject->Expression1);
					CExpression *d=(CExpression*)(arg1->m_pElementList->pElementObject->Expression2);
					n->InsertSequence('+',0,arg1,1,arg1->m_NumElements-1);
					while (arg1->m_NumElements>1)
						arg1->DeleteElement(1);
					d->InsertSequence('+',0,arg2,0,arg2->m_NumElements-1);
				}
				else
				{
					if (arg1->FindLowestOperatorLevel((char)0xD7)<MulLevel)
					{
						arg1->InsertEmptyElement(0,5,'(');
						CExpression *a=(CExpression*)((arg1->m_pElementList)->pElementObject->Expression1);
						int ttt=arg1->m_NumElements;
						for (int kk=1;kk<ttt;kk++)
						{
							a->InsertElement(arg1->m_pElementList+1,a->m_NumElements);
							arg1->DeleteElement(1);
						}
					}
					arg1->InsertSequence((char)0xD7,arg1->m_NumElements,arg2,0,arg2->m_NumElements-1);
				}
				Other->RemoveSequence(MulLevel,StartPos2,StartPos2);
				xreturn (1);
			}
		}
	}






	//*********************************************************************
	//try multiplication of identical elements
	//*********************************************************************

	if ((StartPos<EndPos) && (StartPos2<EndPos2))
	{
		if (CompareExpressions(StartPos,EndPos,Other,StartPos2,EndPos2)==0) xreturn (0);
	}
	else
	{
		if (StartPos<EndPos) xreturn (0);
		if (StartPos2<EndPos2) xreturn (0);
	}

	CExpression *arg_first=NULL;
	CExpression *arg_second=NULL;
	CExpression *exp_first=NULL;
	CExpression *exp_second=NULL;

	if (StartPos==EndPos)
	if (((m_pElementList+StartPos)->Type==3) && ((m_pElementList+StartPos)->pElementObject))
	{
		arg_first=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
		exp_first=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);
		if (((arg_first->m_NumElements==1) && (arg_first->m_pElementList->Type==5))
			|| (arg_first->m_NumElements>1))
			if (ComputationType==0) xreturn (0);
	}

	if (StartPos2==EndPos2)
	if (((Other->m_pElementList+StartPos2)->Type==3) && ((Other->m_pElementList+StartPos2)->pElementObject))
	{
		arg_second=(CExpression*)((Other->m_pElementList+StartPos2)->pElementObject->Expression1);
		exp_second=(CExpression*)((Other->m_pElementList+StartPos2)->pElementObject->Expression2);
		if (((arg_second->m_NumElements==1) && (arg_second->m_pElementList->Type==5))
			|| (arg_second->m_NumElements>1))
			if (ComputationType==0) xreturn (0);
	}

	int delete_no_first=0;
	if ((arg_first==NULL) && (arg_second==NULL))
	{
		if ((StartPos==EndPos) && (StartPos2==EndPos2))
		{
			if (CompareElement(m_pElementList+StartPos,Other->m_pElementList+StartPos2)==0) xreturn (0);
			if (((m_pElementList+StartPos)->Type==5) && (inv==inv2) && (ComputationType==0))
			{
				//for ComputationType==0 we never calculate (a+b)(a+b) into (a+b)^2
				CExpression *a=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
				if (a->FindLowestOperatorLevel()==PlusLevel) xreturn (0);
			}
		}
		else
		{
			if (CompareExpressions(StartPos,EndPos,Other,StartPos2,EndPos2)==0) xreturn (0);
		}

		if (((inv==1) && (inv2==-1)) ||
			((inv==-1) && (inv2==1)))
		{
			InsertEmptyElement(StartPos,1,'1');
		}
		else
		{
			InsertEmptyElement(StartPos,3,'2');
			CExpression *arg=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
			CExpression *exp=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);

			for (int k=StartPos;k<=EndPos;k++)
				arg->InsertElement(m_pElementList+k+1,k-StartPos);
		}
	}
	if ((arg_first) && (arg_second==NULL))
	{
		if (ComputationType==1)
		{
			double N;
			int prec;
			if (!exp_first->IsPureNumber(0,exp_first->m_NumElements,&N,&prec)) xreturn (0);
		}

		if ((StartPos2==EndPos2) && (arg_first->m_NumElements==1))
		{
			if (Other->CompareElement(Other->m_pElementList+StartPos2,arg_first->m_pElementList)==0) xreturn (0);
		}
		else
		{
			if (Other->CompareExpressions(StartPos2,EndPos2,arg_first,0,arg_first->m_NumElements-1)==0) xreturn (0);
		}
		CExpression *arg=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
		CExpression *exp=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);

		double N;
		int prec;
		if (exp->IsPureNumber(0,exp->m_NumElements,&N,&prec))
		{
			if (inv!=inv2) N-=1.0; else N+=1.0;
			exp->Delete();
			exp->GenerateASCIINumber(N,(long long)(N+((N>=0)?0.01:-0.01)),(fabs(N-(long long)(N+((N>=0)?0.01:-0.01)))<1e-100)?1:0,prec,0);
		}
		else
		{
			int exp_level=exp->FindLowestOperatorLevel();
			if ((exp_level<PlusLevel) && (exp_level!=-1))
			{
				//we will have to enclose in parentheses
				exp->InsertEmptyElement(0,5,'(');
				int k=0;
				while (exp->m_NumElements>1)
				{
					((CExpression*)(exp->m_pElementList->pElementObject->Expression1))->InsertElement(exp->m_pElementList+1,k++);
					exp->DeleteElement(1);
				}
			}
			
			exp->InsertEmptyElement(exp->m_NumElements,2,(inv2!=inv)?'-':'+');
			exp->InsertEmptyElement(exp->m_NumElements,1,'1');
		}
		StartPos--;EndPos--;
		if ((this==Other) && (StartPos2>StartPos)) {StartPos2--;EndPos2--;}
		delete_no_first=1;
	}
	if ((arg_first==NULL) && (arg_second))
	{
		if (ComputationType==1)
		{
			double N;
			int prec;
			if (!exp_second->IsPureNumber(0,exp_second->m_NumElements,&N,&prec)) xreturn (0);
		}
		if ((StartPos==EndPos) && (arg_second->m_NumElements==1))
		{
			if (CompareElement(m_pElementList+StartPos,arg_second->m_pElementList)==0) xreturn (0);
		}
		else
		{
			if (CompareExpressions(StartPos,EndPos,arg_second,0,arg_second->m_NumElements-1)==0) xreturn (0);
		}
		InsertEmptyElement(StartPos,3,0);
		CExpression *arg=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
		CExpression *exp=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);
		double N;
		int prec;
		if (exp_second->IsPureNumber(0,exp_second->m_NumElements,&N,&prec))
		{
			if (inv!=inv2) N=1.0-N; else N+=1.0;
			exp->Delete();
			exp->GenerateASCIINumber(N,(long long)(N+((N>=0)?0.01:-0.01)),(fabs(N-(long long)(N+((N>=0)?0.01:-0.01)))<1e-100)?1:0,prec,0);
		}
		else
		{
			int exp_level=exp_second->FindLowestOperatorLevel();
			if ((exp_level<PlusLevel) && (exp_level!=-1))
			{
				//we will have to enclose in parentheses
				exp_second->InsertEmptyElement(0,5,'(');
				int k=0;
				while (exp_second->m_NumElements>1)
				{
					((CExpression*)(exp_second->m_pElementList->pElementObject->Expression1))->InsertElement(exp_second->m_pElementList+1,k++);
					exp_second->DeleteElement(1);
				}
			}
			exp->CopyExpression(exp_second,0);
			exp->InsertEmptyElement(0,2,(inv!=inv2)?'-':'+');
			exp->InsertEmptyElement(0,1,'1');
		}
		for (int k=StartPos;k<=EndPos;k++)
			arg->InsertElement(m_pElementList+k+1,k-StartPos);
	}
	if ((arg_first) && (arg_second))
	{
		if (ComputationType==1)
		{
			double N;
			int prec;
			if (!exp_first->IsPureNumber(0,exp_first->m_NumElements,&N,&prec)) xreturn (0);
			if (!exp_second->IsPureNumber(0,exp_second->m_NumElements,&N,&prec)) xreturn (0);
		}

		if ((arg_first->m_NumElements==1) && (arg_second->m_NumElements==1))
		{
			if (arg_first->CompareElement(arg_first->m_pElementList,arg_second->m_pElementList)==0) xreturn (0);
		}
		else
		{
			if (arg_first->CompareExpressions(0,arg_first->m_NumElements-1,arg_second,0,arg_second->m_NumElements-1)==0) xreturn (0);
		}
		int exp_level=exp_first->FindLowestOperatorLevel();
		if ((exp_level<PlusLevel) && (exp_level!=-1))
		{
			//we will have to enclose in parentheses
			exp_first->InsertEmptyElement(0,5,'(');
			int k=0;
			while (exp_first->m_NumElements>1)
			{
				((CExpression*)(exp_first->m_pElementList->pElementObject->Expression1))->InsertElement(exp_first->m_pElementList+1,k++);
				exp_first->DeleteElement(1);
			}
		}
		exp_level=exp_second->FindLowestOperatorLevel();
		if (((exp_level<PlusLevel) && (exp_level!=-1)) || (exp_second->m_pElementList->Type==2))
		{
			//we will have to enclose in parentheses
			exp_second->InsertEmptyElement(0,5,'(');
			int k=0;
			while (exp_second->m_NumElements>1)
			{
				((CExpression*)(exp_second->m_pElementList->pElementObject->Expression1))->InsertElement(exp_second->m_pElementList+1,k++);
				exp_second->DeleteElement(1);
			}
		}
		CExpression *arg=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
		CExpression *exp=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);
		exp->InsertEmptyElement(exp->m_NumElements,2,(inv!=inv2)?'-':'+');
		for (int k=0;k<exp_second->m_NumElements;k++)
			exp->InsertElement(exp_second->m_pElementList+k,exp->m_NumElements);
		StartPos--;EndPos--;
		if ((this==Other) && (StartPos2>StartPos)) {StartPos2--;EndPos2--;}
		delete_no_first=1;
	}

	if ((Other==this) && (StartPos2>StartPos)) {StartPos2++;EndPos2++;}

	Other->RemoveSequence(MulLevel,StartPos2,EndPos2);
	if (!delete_no_first)
		RemoveSequence(MulLevel,StartPos+1,EndPos+1);
	xreturn (1);

	if ((StartPos2>0) &&
		((Other->m_pElementList+StartPos2-1)->Type==2) && 
		(GetOperatorLevel((Other->m_pElementList+StartPos2-1)->pElementObject->Data1[0])==MulLevel))
		StartPos2--;

	for (int k=StartPos2;k<=EndPos2;k++) Other->DeleteElement(StartPos2);
	if (Other->m_pElementList->Type==0) Other->InsertEmptyElement(0,1,'1');
	if (!delete_no_first)
		for (int k=StartPos+1;k<=EndPos+1;k++) DeleteElement(StartPos+1);
	xreturn (1);
}


int CExpression::ExecuteComputation(int StartPos, int EndPos, char element_type, int StartPos2, int EndPos2, char element_type2,int ComputationType)
{
	/*if (StartPos2<StartPos) return 0;
	if (EndPos>StartPos2) return 0;
	if (StartPos>EndPos) return 0;
	if (StartPos2>EndPos2) return 0;*/

	PROFILE_TIME(&PROFILER.ExecuteComputation);


	//**************************************************************************
	//***                                                                    ***
	//***                       M U L T I P L I C A T I O N                  ***
	//***                                                                    ***
	//**************************************************************************
	if (((element_type==(char)0xD7) || (element_type=='/')) && ((element_type2==(char)0xD7) || (element_type2=='/')))
	{
		if (((m_pElementList+StartPos)->Type==2) && 
			(GetOperatorLevel((m_pElementList+StartPos)->pElementObject->Data1[0])==PlusLevel))
			xreturn (MultiplyElements(StartPos+1,EndPos,(element_type=='/')?-1:1,this,StartPos2,EndPos2,(element_type2=='/')?-1:1,ComputationType));
		xreturn (MultiplyElements(StartPos,EndPos,(element_type=='/')?-1:1,this,StartPos2,EndPos2,(element_type2=='/')?-1:1,ComputationType));

	}



	//**************************************************************************
	//***                                                                    ***
	//***                            S U M M A T I O N                       ***
	//***                                                                    ***
	//**************************************************************************
	if (((element_type=='+') || (element_type=='-')) && ((element_type2=='+') || (element_type2=='-')))
	{
		//**********************************************************************
		//try summation of identical elements
		//**********************************************************************

		tPureFactors PureFactors;
		PureFactors.N1=PureFactors.N2=PureFactors.N3=PureFactors.N4=1.0;
		PureFactors.prec1=PureFactors.prec2=0;
		PureFactors.is_frac1=PureFactors.is_frac2=0;

		int rvl=StrikeoutCommonFactors(StartPos,EndPos,1,this,StartPos2,EndPos2,1,&PureFactors);

		double N1=PureFactors.N1;
		double N2=PureFactors.N2;
		double N3=PureFactors.N3;
		double N4=PureFactors.N4;
		char is_fraction=((PureFactors.is_frac1)||(PureFactors.is_frac2));
		char precision=max(PureFactors.prec1,PureFactors.prec2);

		//summing with zero
		if ((N2!=0) && (fabs(N1/N2)<1e-100)) 
		{
			//the first one is pure zero
			StrikeoutRemove(StartPos2,EndPos2);
			if ((StartPos) && ((m_pElementList+StartPos-1)->Type==2) &&
				(GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])==PlusLevel))
				StartPos--;
			for (int k=StartPos;k<=EndPos;k++) {DeleteElement(StartPos);StartPos2--;}
			xreturn (1);
		}
		if ((N4!=0) && (fabs(N3/N4)<1e-100))
		{
			//the second one is pure zero
			StrikeoutRemove(StartPos,EndPos);
			if ((StartPos2) && ((m_pElementList+StartPos2-1)->Type==2) &&
				(GetOperatorLevel((m_pElementList+StartPos2-1)->pElementObject->Data1[0])==PlusLevel))
				StartPos2--;
			for (int k=StartPos2;k<=EndPos2;k++) DeleteElement(StartPos2);
			xreturn (1);
		}

		if (rvl)
		{
			if (element_type=='-') N1=-N1;
			if (element_type2=='-') N3=-N3;


			//do the math
			//delete second summand and reorganize the first summand
			if (StartPos2<1) xreturn (0);
			if ((m_pElementList+StartPos2-1)->Type!=2) xreturn (0);
			if ((m_pElementList+StartPos2-1)->pElementObject->Data1[0]!=element_type2) xreturn (0);
			for (int k=StartPos2-1;k<=EndPos2;k++) DeleteElement(StartPos2-1);
			StrikeoutRemove(StartPos,EndPos,2);

			//if only '1' was left after the StrikeoutRemove, we will delete it
			if ((m_NumElements==1) && (m_pElementList->Type==1) && (strcmp(m_pElementList->pElementObject->Data1,"1")==0))
				DeleteElement(0);

			//insert first
			double brojnik,nazivnik;
			double pN2=N2;

			if (fabs(N2-N4)<1e-100)
			{
				brojnik=N1+N3;
				nazivnik=N2;
			}
			else if (ReduceTwoNumbers(&N2,&N4))
			{
				nazivnik=N4*pN2;
				brojnik=N1*N4+N2*N3;
			}
			else
			{
				brojnik=N1*N4+N2*N3;
				nazivnik=N2*N4; 
			}
			
			if (nazivnik<0) {nazivnik=-nazivnik;brojnik=-brojnik;}

			if (fabs(brojnik)<1e-100)
			{
				//the calculation equals to zero
				InsertEmptyElement(StartPos,1,'0');
				xreturn (1);
			}

			ReduceTwoNumbers(&brojnik,&nazivnik);


			//delete operator just before
			if ((StartPos) && ((m_pElementList+StartPos-1)->Type==2) &&
				((m_pElementList+StartPos-1)->pElementObject->Data1[0]==element_type))
			{
				DeleteElement(--StartPos);
			}
			if (brojnik<0)
			{
				//adding the minus preoperator sign
				brojnik=-brojnik;
				InsertEmptyElement(StartPos++,2,'-');
			}
			else if (StartPos) 
			{
				if (((m_pElementList+StartPos-1)->Type!=2) && ((m_pElementList+StartPos-1)->Type!=11) && ((m_pElementList+StartPos-1)->Type!=12))
					InsertEmptyElement(StartPos++,2,'+');
			}

			this->GenerateASCIIFraction(StartPos++,brojnik,nazivnik,precision,is_fraction);
			xreturn (1);

		}
		else
		{
			StrikeoutRemove(StartPos,EndPos);
			StrikeoutRemove(StartPos2,EndPos2);
		}


		//***************************************************************************
		//matrix summantion
		//***************************************************************************
		if ((StartPos==EndPos) && (StartPos2==EndPos2) && 
			((m_pElementList+StartPos)->Type==5) &&
			((m_pElementList+StartPos2)->Type==5))
		{
			CExpression *inside1=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
			CExpression *inside2=(CExpression*)((m_pElementList+StartPos2)->pElementObject->Expression1);

			int lvl1=inside1->FindLowestOperatorLevel((char)0xD7);
			int lvl2=inside2->FindLowestOperatorLevel((char)0xD7);

			if ((lvl1==lvl2) && (GetOperatorLevel((char)0xFF)==lvl1))
			{
				//we have two matrices - find rows and columns number
				int rows1=1,rows2=1,columns1=1,columns2=1;
				if (!inside1->GetMatrixSize(&rows1,&columns1)) xreturn (0);
				if (!inside2->GetMatrixSize(&rows2,&columns2)) xreturn (0);

				if ((columns1==columns2) && (rows1==rows2) && 
					(inside1->m_ParentheseShape==inside2->m_ParentheseShape) &&
					((inside1->m_ParentheseShape!='|') && (inside1->m_ParentheseShape!='\\')))  //will not adding determinants
				{
					CExpression *tmp=new CExpression(NULL,NULL,100);
					for (int ii=0;ii<rows1;ii++)
					{
						for (int jj=0;jj<columns1;jj++)
						{
							tmp->InsertEmptyElement(tmp->m_NumElements,5,'(');
							CExpression *a=(CExpression*)((tmp->m_pElementList+tmp->m_NumElements-1)->pElementObject->Expression1);
							int pos=inside1->FindMatrixElement(ii,jj,0);
							if (pos<0) pos=0;
							while (((inside1->m_pElementList+pos)->Type<11) && (pos<inside1->m_NumElements))
							{
								a->InsertElement(inside1->m_pElementList+pos,a->m_NumElements);
								pos++;
							}

							if (element_type=='-')
							{
								char ch='+';
								if (element_type2=='+') ch='-';
								if (element_type2==(char)0xB1) ch=(char)0xB2;
								if (element_type2==(char)0xB2) ch=(char)0xB1;
								tmp->InsertEmptyElement(tmp->m_NumElements,2,ch);
							}
							else
								tmp->InsertEmptyElement(tmp->m_NumElements,2,element_type2);

							tmp->InsertEmptyElement(tmp->m_NumElements,5,'(');
							a=(CExpression*)((tmp->m_pElementList+tmp->m_NumElements-1)->pElementObject->Expression1);
							pos=inside2->FindMatrixElement(ii,jj,0);
							if (pos<0) pos=0;
							while (((inside2->m_pElementList+pos)->Type<11) && (pos<inside2->m_NumElements))
							{
								a->InsertElement(inside2->m_pElementList+pos,a->m_NumElements);
								pos++;
							}

							if (jj<columns1-1) tmp->InsertEmptyElement(tmp->m_NumElements,11,0);
						}
						if (ii<rows1-1) tmp->InsertEmptyElement(tmp->m_NumElements,12,0);
					}
					int pst=inside1->m_ParenthesesFlags;
					int shp=inside1->m_ParentheseShape;
					inside1->CopyExpression(tmp,0);
					inside1->m_ParenthesesFlags=pst;
					inside1->m_ParentheseShape=shp;
					DeleteElement(StartPos2);
					DeleteElement(StartPos2-1);
					delete tmp;
					xreturn (1);
				}
			}



		}



		//***************************************************************************
		//fractions with the same denominator
		//***************************************************************************

		if (ComputationType!=0)
		{
			int fraction_found=0;
			CExpression *fraction_denom;
			{
				int pos=StartPos;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) 
					{
						if (!fraction_found)
						{
							fraction_found=1;
							if (et=='/')
								fraction_denom=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression1);
							else
								fraction_denom=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression2);
						}
						else
						{
							fraction_found=0;
							break;
						}
					}
					pos+=l;
					if (pos>EndPos) break;
				}
			}	

			int fraction_found2=0;
			if (fraction_found)
			{
				int pos=StartPos2;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos2,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) 
					{			
						if (!fraction_found2)
						{
							fraction_found2=1;
							CExpression *d1;
							if (et=='/')
								d1=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression1);
							else
								d1=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression2);
						
							if (d1->CompareExpressions(0,d1->m_NumElements-1,fraction_denom,0,fraction_denom->m_NumElements-1)==0)
							{
								fraction_found=0;
								break;
							}	
						}
						else
						{
							fraction_found=0;
							break;
						}
					}
					pos+=l;
					if (pos>EndPos2) break;
				}
			}


			if ((fraction_found) && (fraction_found2))
			{
				InsertEmptyElement(StartPos,4,0);
				CExpression *num=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
				CExpression *denom=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);
				InsertEmptyElement(StartPos+1,2,element_type);
				StartPos+=2;EndPos+=2;StartPos2+=2;EndPos2+=2;
				denom->CopyExpression(fraction_denom,0);

				int pos=StartPos;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) 
					{
						CExpression *n;
						if (et=='/')
							n=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression2);
						else
							n=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression1);
						num->InsertEmptyElement(num->m_NumElements,5,'(');
						CExpression *a=(CExpression*)((num->m_pElementList+num->m_NumElements-1)->pElementObject->Expression1);
						a->CopyExpression(n,0); a->m_ParenthesesFlags=0x02;
					}
					else
					{
						for (int i=pos;i<pos+l;i++)
							num->InsertElement(m_pElementList+i,num->m_NumElements);
					}
					pos+=l;
					if (pos>EndPos) break;
				}

				if (element_type=='-')
				{
					if (element_type2=='+') num->InsertEmptyElement(num->m_NumElements,2,'-');
					if (element_type2=='-') num->InsertEmptyElement(num->m_NumElements,2,'+');
					if (element_type2==(char)0xB1) num->InsertEmptyElement(num->m_NumElements,2,(char)0xB2);
					if (element_type2==(char)0xB2) num->InsertEmptyElement(num->m_NumElements,2,(char)0xB1);
				}
				else
					num->InsertEmptyElement(num->m_NumElements,2,element_type2);

				pos=StartPos2;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos2,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) 
					{
						CExpression *n;
						if (et=='/')
							n=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression2);
						else
							n=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression1);
						num->InsertEmptyElement(num->m_NumElements,5,'(');
						CExpression *a=(CExpression*)((num->m_pElementList+num->m_NumElements-1)->pElementObject->Expression1);
						a->CopyExpression(n,0); a->m_ParenthesesFlags=0x02;
					}
					else
					{
						for (int i=pos;i<pos+l;i++)
							num->InsertElement(m_pElementList+i,num->m_NumElements);
					}
					pos+=l;
					if (pos>EndPos2) break;
				}
				//delete leftovers and finish
				RemoveSequence(PlusLevel,StartPos2,EndPos2);
				RemoveSequence(PlusLevel,StartPos,EndPos);
				xreturn (1);
			}

		}
		//***************************************************************************
		//fractions with the different denominator
		//***************************************************************************
		
		if (ComputationType==2)
		{
			//first check if there is at least one fraction to do with
			int fraction_found=0;
			{
				int pos=StartPos;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) {fraction_found=1;break;}

					pos+=l;
					if (pos>EndPos) break;
				}
			}	
			if (!fraction_found)
			{
				int pos=StartPos2;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos2,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) {fraction_found=2;break;}

					pos+=l;
					if (pos>EndPos2) break;
				}
			}

			
			if (fraction_found)
			{
				//yes, there is at least one fraction in these two summands so we will continue
				//create the 'summed' fration
				InsertEmptyElement(StartPos,4,0);
				CExpression *num=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
				CExpression *denom=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);
				InsertEmptyElement(StartPos+1,2,element_type);
				StartPos+=2;EndPos+=2;StartPos2+=2;EndPos2+=2;

				//take care about plus/mins sign
				char et2=element_type2;
				if (element_type=='-')
				{
					if (et2=='+') et2='-';
					else if (et2=='-') et2='+';
					else if (et2==(char)0xB1) et2=(char)0xB2;
					else if (et2==(char)0xB2) et2=(char)0xB1;
				}
				num->InsertEmptyElement(0,2,et2);

				int P1=0;int P2=1;  //P1 is insertion position in first numerator summand, and P2 in second

				//now run through first summand and analyze its factors - distribute them to either numerator or denominator
				int pos=StartPos;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) 
					{
						CExpression *n=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression1);
						CExpression *d=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression2);
						if (et=='/') {CExpression *t=n;n=d;d=t;}
						if ((n->FindLowestOperatorLevel((char)0xD7)<MulLevel) ||
							((n->m_pElementList->Type==2) && (GetOperatorLevel(n->m_pElementList->pElementObject->Data1[0])==PlusLevel)))
						{
							num->InsertEmptyElement(P1++,5,'(');P2++;
							CExpression *a=(CExpression*)((num->m_pElementList+P1-1)->pElementObject->Expression1);
							for (int k=0;k<n->m_NumElements;k++)
								a->InsertElement(n->m_pElementList+k,a->m_NumElements);
						}
						else
						{
							for (int k=0;k<n->m_NumElements;k++)
							{num->InsertElement(n->m_pElementList+k,P1++);P2++;}
						}

						if ((d->FindLowestOperatorLevel((char)0xD7)<MulLevel) ||
							((d->m_pElementList->Type==2) && (GetOperatorLevel(d->m_pElementList->pElementObject->Data1[0])==PlusLevel)))
						{
							num->InsertEmptyElement(P2++,5,'(');
							CExpression *a=(CExpression*)((num->m_pElementList+P2-1)->pElementObject->Expression1);
							for (int k=0;k<d->m_NumElements;k++)
								a->InsertElement(d->m_pElementList+k,a->m_NumElements);
							denom->InsertElement(num->m_pElementList+P2-1,denom->m_NumElements);
						}
						else
						{
							for (int k=0;k<d->m_NumElements;k++)
							{
								denom->InsertElement(d->m_pElementList+k,denom->m_NumElements);	
								num->InsertElement(d->m_pElementList+k,P2++);
							}
						}
					}
					else
					{
						for (int k=pos;k<pos+l;k++)
						{num->InsertElement(m_pElementList+k,P1++);P2++;}
					}

					pos+=l;
					if (pos>EndPos) break;
				}

				//also, do the same with the second summand
				pos=StartPos2;
				while (1)
				{
					char et,p;
					int l=GetElementLen(pos,EndPos2,MulLevel,&et,&p);
					if (l==0) break;

					if ((l-p==1) && ((m_pElementList+pos+p)->Type==4)) 
					{
						CExpression *n=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression1);
						CExpression *d=(CExpression*)((m_pElementList+pos+p)->pElementObject->Expression2);
						if (et=='/') {CExpression *t=n;n=d;d=t;}
						if ((n->FindLowestOperatorLevel((char)0xD7)<MulLevel) ||
							((n->m_pElementList->Type==2) && (GetOperatorLevel(n->m_pElementList->pElementObject->Data1[0])==PlusLevel)))
						{
							num->InsertEmptyElement(P2++,5,'(');
							CExpression *a=(CExpression*)((num->m_pElementList+P2-1)->pElementObject->Expression1);
							for (int k=0;k<n->m_NumElements;k++)
								a->InsertElement(n->m_pElementList+k,a->m_NumElements);
						}
						else
						{
							for (int k=0;k<n->m_NumElements;k++)
								num->InsertElement(n->m_pElementList+k,P2++);								
						}

						if ((d->FindLowestOperatorLevel((char)0xD7)<MulLevel) ||
							((d->m_pElementList->Type==2) && (GetOperatorLevel(d->m_pElementList->pElementObject->Data1[0])==PlusLevel)))
						{
							num->InsertEmptyElement(P1++,5,'(');P2++;
							CExpression *a=(CExpression*)((num->m_pElementList+P1-1)->pElementObject->Expression1);
							for (int k=0;k<d->m_NumElements;k++)
								a->InsertElement(d->m_pElementList+k,a->m_NumElements);
							denom->InsertElement(num->m_pElementList+P1-1,denom->m_NumElements);
						}
						else
						{
							for (int k=0;k<d->m_NumElements;k++)
							{
								denom->InsertElement(d->m_pElementList+k,denom->m_NumElements);	
								num->InsertElement(d->m_pElementList+k,P1++);P2++;
							}
						}
					}
					else
					{
						for (int k=pos;k<pos+l;k++)
							num->InsertElement(m_pElementList+k,P2++);
					}

					pos+=l;
					if (pos>EndPos2) break;
				}

				//delete leftovers and finish
				RemoveSequence(PlusLevel,StartPos2,EndPos2);
				RemoveSequence(PlusLevel,StartPos,EndPos);

				int cntr=0;
				while ((this->Compute(StartPos-2,StartPos-2,ComputationType)) && (cntr<50)) cntr++;
				xreturn (1);
			}
		}
	}



	xreturn (0);
}


//checks if the element at StartPos-Len is a pure number - returns the number as double (with precision)
int CExpression::IsPureNumber(const int StartPos,const int Len, double * number,int *precision)
{
	*number=1.0;
	*precision=0;
	tElementStruct *theElement;
	int minus_sign;
	
	if (Len==1)
	{
		minus_sign=0;
		theElement=m_pElementList+StartPos;
	}
	else if (Len==2)
	{
		theElement=m_pElementList+StartPos;
		if (theElement->Type==2)
		{
			char oper=theElement->pElementObject->Data1[0];
			if (oper=='-') minus_sign=1;
			else if (oper=='+') minus_sign=0;
			else return 0;
			theElement++;
		}
		else 
			return 0;
	}
	else
		return 0;

	//if (theElement->pElementObject==NULL) return 0; //this check not needed - removed for speed
	
	if (theElement->Type==1) //the element is an variable
	{
		if (theElement->pElementObject->Expression1) return 0; //no index (TODO what about hex and other bases)
		char *start=theElement->pElementObject->Data1;

		if (*start=='-') {start++;minus_sign=(minus_sign)?0:1;}  //for easier function plotting we support numbers with integrated minus
		if ((*start<'0') || (*start>'9')) return 0;

		char *ch=start+1;

		if ((theElement->pElementObject->Data1[14]==0) && ((theElement->pElementObject->Data1[15]&0x7F)==126)) //if the number already exist in the high precision
		{
			//find out the precision (number of decimals)
			int has_decpoint=0;
			for (;*ch;ch++)
			{
				if (has_decpoint) {(*precision)++;continue;}
				if ((*ch=='.') || (*ch==',')) has_decpoint=1;
			}
			*number=*(double*)(theElement->pElementObject->Data1+16);
			if (minus_sign) *number=-(*number);
			return 1;
		}
		
		if (*ch==0) //one-digit number found (for speed)
		{
			if (minus_sign)
				*number=(double)('0'-*start);
			else
				*number=(double)(*start-'0');
			return 1;
		}

		//find out the precision (number of decimals)
		int has_decpoint=0;
		char *decpoint;
		for (;*ch;ch++)
		{
			if (has_decpoint) {(*precision)++;continue;}
			if (*ch=='.') has_decpoint=1;
			else if (*ch==',') {has_decpoint=2;decpoint=ch;*ch='.';}
		}

		//number found
		*number=atof(start);
		if (has_decpoint==2) *decpoint=',';
		if (minus_sign) *number=-(*number);
		return 1;
	}
	if (theElement->Type==3) //exponent
	{
		double N1;
		double N2;
		int prec1,prec2;

		CExpression *a=(CExpression*)(theElement->pElementObject->Expression1);
		CExpression *e=(CExpression*)(theElement->pElementObject->Expression2);
		if ((a->m_NumElements<=2) && (e->m_NumElements<=2))
		if (a->IsPureNumber(0,a->m_NumElements,&N1,&prec1))
			if (e->IsPureNumber(0,e->m_NumElements,&N2,&prec2))
			{
				long long N2int=(long long)(N2+((N2>=0)?0.01:-0.01));
				if ((N1>=0) || (fabs(N2-(double)N2int)<1e-100))
				{
					if (fabs(N2-(double)N2int)<1e-100) N2=(double)N2int;
					*precision=max(prec1,prec2);
					*number=pow(N1,N2);
					if (minus_sign) *number=-(*number);
					return 1;
				}
			}
	}
	if ((theElement->Type==5)) //parentheses
	{
		double N1;
		CExpression *a=(CExpression*)(theElement->pElementObject->Expression1);
		if ((a->m_NumElements<=2) && (a->IsPureNumber(0,a->m_NumElements,&N1,precision)))
		{
			*number=N1;
			if ((a->m_ParentheseShape=='|') || (a->m_ParentheseShape=='\\')) *number=fabs(*number);
			if (a->m_ParentheseShape=='c') *number=ceil(*number);//(int)((*number>0)?(*number+1):(*number)); //ceiling
			if (a->m_ParentheseShape=='f') *number=floor(*number);//(int)((*number>0)?(*number):(*number-1.0)); //floor
			if (minus_sign) *number=-(*number);
			return 1;
		}
	}
	if (theElement->Type==4)  //fraction
	{
		double N1,N2;
		int prec1,prec2;
		CExpression *nom=(CExpression*)(theElement->pElementObject->Expression1);
		CExpression *denom=(CExpression*)(theElement->pElementObject->Expression2);
		if ((nom->m_NumElements<=2) && (denom->m_NumElements<=2))
		if (nom->IsPureNumber(0,nom->m_NumElements,&N1,&prec1))
		{
			if (denom->IsPureNumber(0,denom->m_NumElements,&N2,&prec2))
			{
				if (N2<0) {N2=-N2;N1=-N1;}
				ReduceTwoNumbers(&N1,&N2);

				if (fabs(N2-1.0)<1e-100)
				{
					*precision=max(prec1,prec2);
					*number=N1;
					if (minus_sign) *number=-(*number);
					return 1;
				}
			}
		}
	}
	
	return 0;
}


//returns number of elements that is added to expression
int CExpression::GenerateASCIINumber(double number_dbl,long long number_int, char is_integer, int precision, int position)
{
	int shortening=0;
	if (precision>=100) {shortening=1;precision-=100;} //shortening means that the value will be trimed for ending zeros

	int NumElems=m_NumElements;
	if (m_pElementList->Type==0) NumElems=0;

	char buffer[32];
	memset(buffer,0,24);

	if (precision<0) precision=0;
	if (precision>15) precision=15;

	if (fabs(number_dbl)>=1000000.0) is_integer=0;
	if (fabs(number_dbl-number_int)>1e-100) is_integer=0;

	if (is_integer)
	{
		if (number_int<0)
		{
			sprintf(buffer,"%I64d",-number_int);
			buffer[23]=0;
			if ((position==0) && (NumElems==0))
			{
				InsertEmptyElement(0,2,'-');
				InsertEmptyElement(1,1,'0');
				memcpy((m_pElementList+1)->pElementObject->Data1,buffer,24);
				memset((m_pElementList+1)->pElementObject->Data2,0,24);
				return 2;
			}
			InsertEmptyElement(position,5,'(');
			CExpression *tmp=(CExpression*)((m_pElementList+position)->pElementObject->Expression1);
			tmp->InsertEmptyElement(0,2,'-');
			tmp->InsertEmptyElement(1,1,'0');
			memcpy((tmp->m_pElementList+1)->pElementObject->Data1,buffer,24);
			memset((tmp->m_pElementList+1)->pElementObject->Data2,0,24);
			return 1;
		}
		sprintf(buffer,"%I64d",number_int);
		buffer[23]=0;
		InsertEmptyElement(position,1,0);
		memcpy((m_pElementList+position)->pElementObject->Data1,buffer,24);
		memset((m_pElementList+position)->pElementObject->Data2,0,24);
		return 1;
	}


	if ((fabs(number_dbl)<10000000.0) && (fabs(number_dbl)>=0.001))
	{		
		char format[10];
		int ln;
		do
		{
			sprintf(format,"%%.%df",precision);
			ln=sprintf(buffer,format,fabs(number_dbl));
			precision++;
		} while ((fabs(number_dbl)>1e-100) && (fabs((atof(buffer)-fabs(number_dbl))/fabs(number_dbl))>0.02));
		precision--;

		if (fabs(atof(buffer)-fabs(number_dbl))>1e-12)
		{
			//if the number is not precise (rounded) then check if by adding single more decimal place it becomes exact
			precision++;
			sprintf(format,"%%.%df",precision);
			char buffer2[32];
			int ln2=sprintf(buffer2,format,fabs(number_dbl));
			if (fabs(atof(buffer2)-fabs(number_dbl))<=1e-12)
			{
				ln=ln2;
				strcpy(buffer,buffer2);
			}
		}

		if ((shortening) && (precision>0))
		{
			while ((ln>1) && (buffer[ln-1]=='0'))
				{buffer[ln-1]=0;ln--;}
			if ((buffer[ln-1]=='.') || (buffer[ln-1]==',')) {buffer[ln-1]=0;ln--;}
		}
		if (ln<15)
		{
			buffer[14]=0;
			buffer[15]=126;  //special flag
			if (fabs(atof(buffer)-fabs(number_dbl))>1e-12)
				buffer[15]|=0x80; //flag that the number is rounded
			(*(double*)(buffer+16))=fabs(number_dbl);
		}
		if (UseCommaAsDecimal)
			for (int gg=0;gg<ln;gg++) if (buffer[gg]=='.') buffer[gg]=',';

		if (number_dbl<0)
		{
			if ((position==0) && (NumElems==0))
			{
				InsertEmptyElement(0,2,'-');
				InsertEmptyElement(1,1,'0');
				memcpy((m_pElementList+1)->pElementObject->Data1,buffer,24);
				memset((m_pElementList+1)->pElementObject->Data2,0,24);	
				
				return 2;
			}
			InsertEmptyElement(position,5,'(');
			CExpression *tmp=(CExpression*)((m_pElementList+position)->pElementObject->Expression1);
			tmp->InsertEmptyElement(0,2,'-');
			tmp->InsertEmptyElement(1,1,'0');
			memcpy((tmp->m_pElementList+1)->pElementObject->Data1,buffer,24);
			memset((tmp->m_pElementList+1)->pElementObject->Data2,0,24);
			return 1;
		}

		InsertEmptyElement(position,1,'0');
		memcpy((m_pElementList+position)->pElementObject->Data1,buffer,24);
		memset((m_pElementList+position)->pElementObject->Data2,0,24);
		return 1;
	}
	else
	{
		//the followng code will write the number in scientific format (a*10^b)

		//first calculate the real part (rr) and the exponent (exxp)
		int exxp=0;
		double rr;
		if (fabs(number_dbl)<1.0)
		{
			while (1)
			{
				rr=number_dbl*pow(10.0,-exxp);
				if (abs(rr)>=1.0) break; 
				exxp--;
				if (exxp<-100) 	{rr=0.0;exxp=0;break;}
			}
		}
		else
		{
			while (1)
			{
				rr=number_dbl/pow(10.0,exxp);
				if (abs(rr)<10.0) break;
				exxp++;
				if (exxp>100) {rr=(number_dbl<0)?-1.0:1.0;exxp=100;break;}
			}

		}

		//just check range (in no case the fabs(rr) should be greater than 10.0)
		if (fabs(rr)<1.0) {exxp-=1;rr*=10.0;}
		if (fabs(rr)>=10.0) {exxp+=1;rr/=10.0;}


		long long rr_int;
		int is_int;
		rr_int=(long long)(rr+((rr<0)?-0.5:0.5));
		if (fabs(rr-rr_int)<1e-100) is_int=1; else is_int=0;
		if (is_int)
		{
			//it is integer
			if (abs((long)rr_int)==10) {rr_int/=10;exxp+=1;}
			sprintf(buffer,"%I64d",((rr_int<0)?(-rr_int):rr_int));
			buffer[23]=0;
		}
		else
		{
			//it is not integer - check how high precision we need
			double my_number=rr*pow(10.0,precision);
			if (fabs(my_number)<0x7FFFFFFFFFFFFFF)
			{
				if (fabs(my_number-(long long)my_number)>=0.01) precision+=2;
			}

			char format[10];
			sprintf(format,"%%.%df",precision);
			int ln=sprintf(buffer,format,((rr<0)?(-rr):rr));
			if ((shortening) && (precision>0))
			{
				while ((ln>1) && (buffer[ln-1]=='0'))
					{buffer[ln-1]=0;ln--;}
				if ((buffer[ln-1]=='.') ||(buffer[ln-1]==',')) {buffer[ln-1]=0;ln--;}
			}

			//just check, because it must never be greater than 10
			if (strcmp(buffer,"10")==0) {strcpy(buffer,"1");rr/=10;exxp+=1;}

			if (ln<15)
			{
				buffer[14]=0;
				buffer[15]=126;  //special flag
				if (fabs(atof(buffer)-fabs(number_dbl))>1e-12)
					buffer[15]|=0x80; //flag that the number is rounded
				(*(double*)(buffer+16))=fabs(rr);
			}
			if (UseCommaAsDecimal)
				for (int gg=0;gg<ln;gg++) if (buffer[gg]=='.') buffer[gg]=',';

		}

		{
			CExpression *tmp;
			if (((position==0) && (NumElems==0)) || 
				((exxp==0) && (rr>=0)))
			{
				//if we are placing at the first position
				//or we will place only single number (no exponent, no sign)
				tmp=this;
			}
			else
			{
				//otherwise, generate the number inside parentheses
				InsertEmptyElement(position,5,'(');
				tmp=(CExpression*)((m_pElementList+position)->pElementObject->Expression1);
				position=0;
			}
			if (rr<0) tmp->InsertEmptyElement(position++,2,'-');
			tmp->InsertEmptyElement(position++,1,'0');
			memcpy((tmp->m_pElementList+position-1)->pElementObject->Data1,buffer,24);
			memset((tmp->m_pElementList+position-1)->pElementObject->Data2,0,24);
			if (exxp)
			{
				memset(buffer,0,24);
				tmp->InsertEmptyElement(position++,2,(char)0xD7);
				tmp->InsertEmptyElement(position++,3,5);
				CExpression *tmp2=(CExpression*)((tmp->m_pElementList+position-1)->pElementObject->Expression2);
				tmp2->InsertEmptyElement(0,1,'0');
				itoa(abs(exxp),buffer,10);
				strcpy((tmp2->m_pElementList+0)->pElementObject->Data1,buffer);
				memset((tmp2->m_pElementList+0)->pElementObject->Data2,0,24);
				if (exxp<0) tmp2->InsertEmptyElement(0,2,'-');				
			}
		}
	}
	return m_NumElements-NumElems;
}


//this function is frequently executed, must be very fast
int CExpression::CompareElement(const tElementStruct * ts1,const tElementStruct * ts2)
{
	if (ts1->Type!=ts2->Type) return 0;
	if (ts1->Decoration!=ts2->Decoration) return 0;
	if ((ts1->pElementObject==NULL) && (ts2->pElementObject==NULL)) return 1;
	if ((ts1->pElementObject==NULL) || (ts2->pElementObject==NULL)) return 0;

	if (ts1->Type==1) //variable
	{
		//fast handling - for single character variables without any index
		unsigned short t1=*(unsigned short*)(ts1->pElementObject->Data1);
		if ((t1<256) && (ts1->pElementObject->Expression1==NULL) && (ts2->pElementObject->Expression1==NULL))
		{
			if ((t1==*(unsigned short*)(ts2->pElementObject->Data1)) &&
				(*(ts1->pElementObject->Data2)==*(ts2->pElementObject->Data2))) //font
				return 1;
			else
				return 0;
		}

		//standard handling
		if (strcmp(ts1->pElementObject->Data1,ts2->pElementObject->Data1)) return 0;
		int len1=(int)strlen(ts1->pElementObject->Data1);
		if (memcmp(ts1->pElementObject->Data2,ts2->pElementObject->Data2,len1)) return 0;
	}
	else if (ts1->Type==5) //parenthese
	{
		//if (((CExpression*)(ts1->pElementObject->Expression1))->m_IsText!=((CExpression*)(ts2->pElementObject->Expression1))->m_IsText) return 0;
	}
	else if (ts1->Type==6) //function
	{
		if (strcmp(ts1->pElementObject->Data1,ts2->pElementObject->Data1)) return 0;
	}
	else if (ts1->Type==4) //fraction
	{
		int t1=0;if (ts1->pElementObject->Data1[0]==' ') t1=1;
		int t2=0;if (ts2->pElementObject->Data1[0]==' ') t2=1;
		if (t1!=t2) return 0;
	}
	else if (ts1->Type==7) //sum, pi, integral
	{
		if (ts1->pElementObject->Data1[0]!=ts2->pElementObject->Data1[0]) return 0;

		//for integrals also check the dimension (double, triple integral)
		if ((ts1->pElementObject->Data1[0]=='i') || (ts1->pElementObject->Data1[0]=='I') || 
			(ts1->pElementObject->Data1[0]=='o') || (ts1->pElementObject->Data1[0]=='O'))
			if (ts1->pElementObject->Data2[2]!=ts2->pElementObject->Data2[2]) return 0;
	}
	else if (ts1->Type==2)
	{
		if (ts1->pElementObject->Data1[0]!=ts2->pElementObject->Data1[0]) return 0;
	}


	if ((ts1->pElementObject->Expression1) && (ts2->pElementObject->Expression1))
	{
		if (((CExpression*)(ts1->pElementObject->Expression1))->CompareExpressions(0,-1,((CExpression*)(ts2->pElementObject->Expression1)),0,-1)==0) return 0;
	}
	else if (ts1->pElementObject->Expression1!=ts2->pElementObject->Expression1) return 0;

	if ((ts1->pElementObject->Expression2) && (ts2->pElementObject->Expression2))
	{
		if (((CExpression*)(ts1->pElementObject->Expression2))->CompareExpressions(0,-1,((CExpression*)(ts2->pElementObject->Expression2)),0,-1)==0) return 0;
	}
	else if (ts1->pElementObject->Expression2!=ts2->pElementObject->Expression2) return 0;

	if ((ts1->pElementObject->Expression3) && (ts2->pElementObject->Expression3))
	{
		if (((CExpression*)(ts1->pElementObject->Expression3))->CompareExpressions(0,-1,((CExpression*)(ts2->pElementObject->Expression3)),0,-1)==0) return 0;
	}
	else if (ts1->pElementObject->Expression3!=ts2->pElementObject->Expression3) return 0;


	return 1;
}




int CExpression::ComputeExponent(int Position, char element_type, int ComputationType)
{	
	int retval=0;
	tElementStruct *ts=m_pElementList+Position;
	if (ts->Type!=3) return 0;
	if (ts->pElementObject==NULL) return 0;
	if (ts->pElementObject->Expression1==NULL) return 0;
	if (ts->pElementObject->Expression2==NULL) return 0;

	PROFILE_CNT(&PROFILER.ComputeExponent);

	CExpression *exp=(CExpression*)(ts->pElementObject->Expression2);
	CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);

	//take care about exponent

	int cnt=0;
	//if (ComputationType!=2)
	{
		while ((exp->Compute(0,exp->m_NumElements-1,(ComputationType==10)?10:0)) && (cnt<50)) cnt++;
		if (cnt) retval=1;
	}

	//take care about argument
	if (ComputationType==1)
	{
		cnt=0;
		while ((arg->Compute(0,arg->m_NumElements-1,1)) && (cnt<50)) cnt++;
		if (cnt) retval=1;
		if (arg->FactorizeExpression()) retval=1;
	}
	else if ((ComputationType!=2) && (ComputationType!=3))
	{
		cnt=0;
		while ((arg->Compute(0,arg->m_NumElements-1,(ComputationType==10)?10:0)) && (cnt<50)) cnt++;
		if (cnt) retval=1;
	}

	double ArgVal;
	double ExpVal;
	int is_argument_pure=0;
	int is_exponent_pure=0;
	int arg_prec;
	int exp_prec;
	if (ComputationType==10)
	{
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;
		PF.prec1=0;
		int ret=arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
		arg->StrikeoutRemove(0,arg->m_NumElements-1);
		if (ret) {is_argument_pure=1;ArgVal=PF.N1/PF.N2;arg_prec=PF.prec1;}

		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;
		PF.prec1=0;
		ret=exp->StrikeoutCommonFactors(0,exp->m_NumElements-1,1,NULL,0,0,1,&PF);
		exp->StrikeoutRemove(0,exp->m_NumElements-1);
		if (ret) {is_exponent_pure=1;ExpVal=PF.N1/PF.N2;exp_prec=PF.prec1;}
	}
	else
	{
		if (arg->IsPureNumber(0,arg->m_NumElements,&ArgVal,&arg_prec)) is_argument_pure=1;
		if (exp->IsPureNumber(0,exp->m_NumElements,&ExpVal,&exp_prec)) is_exponent_pure=1;
	}

	if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==1) &&
		(arg->m_pElementList->pElementObject->Data1[0]==ImaginaryUnit) &&
		(arg->m_pElementList->pElementObject->Data1[1]==0) &&
		((arg->m_pElementList->pElementObject->m_VMods)==0) &&
		((arg->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60))
	{
		//the argument is the imaginary unit
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;
		PF.prec1=0;
		int ret=exp->StrikeoutCommonFactors(0,exp->m_NumElements-1,1,NULL,0,0,1,&PF);
		if (PF.N2<0) {PF.N1=-PF.N1;PF.N2=-PF.N2;}
		ReduceTwoNumbers(&PF.N1,&PF.N2);
		long long N=(long long)PF.N1;
		if ((N>=2) || (N<0))
		{
			PF.N1=PF.N1/(double)N;
			int n;
			if (N>=0) n=(int)(N%4); else n=(int)((4-((-N)%4))%4);
			{
				if (n==0) InsertEmptyElement(Position,1,'1');
				if (n==1) InsertEmptyElement(Position,1,ImaginaryUnit);
				if (n==2) 
				{
					InsertEmptyElement(Position,5,'(');
					((CExpression*)((m_pElementList+Position)->pElementObject->Expression1))->InsertEmptyElement(0,2,'-');
					((CExpression*)((m_pElementList+Position)->pElementObject->Expression1))->InsertEmptyElement(1,1,'1');
				}
				if (n==3) 
				{
					InsertEmptyElement(Position,5,'(');
					((CExpression*)((m_pElementList+Position)->pElementObject->Expression1))->InsertEmptyElement(0,2,'-');
					((CExpression*)((m_pElementList+Position)->pElementObject->Expression1))->InsertEmptyElement(1,1,ImaginaryUnit);
				}
			}
			if ((ret==0) || (fabs(PF.N1-1.0)>1e-100) || (fabs(PF.N2-1.0)>1e-100))
			{
				exp->StrikeoutRemove(0,exp->m_NumElements,2);
				int p=0;
				if ((exp->m_pElementList->Type==2) && (GetOperatorLevel(exp->m_pElementList->pElementObject->Data1[0])==PlusLevel))
					p=1;
				exp->GenerateASCIIFraction(p,PF.N1,PF.N2,PF.prec1,PF.is_frac1);
				return 1;
			}
			else
			{
				DeleteElement(Position+1);
				return 1;
			}
		}
	}

	if (is_argument_pure)
	{
		if (fabs(ArgVal)<1e-100) // 0^X
		{
			DeleteElement(Position);
			InsertEmptyElement(Position,1,'0');
			return 1;
		}
		if (fabs(ArgVal-1.0)<1e-100) //1^X
		{
			DeleteElement(Position);
			InsertEmptyElement(Position,1,'1');
			return 1;
		}
	}

	if (is_exponent_pure)
	{
		if (fabs(ExpVal)<1e-100) //X^0
		{
			DeleteElement(Position);
			InsertEmptyElement(Position,1,'1');
			return 1;
		}
		if (fabs(ExpVal-1.0)<1e-100)  //X^1
		{
			if (GetOperatorLevel(element_type)!=MulLevel)
				Position=InsertSequence((char)0xD7,Position,arg,0,arg->m_NumElements-1);
			else
				Position=InsertSequence(element_type,Position,arg,0,arg->m_NumElements-1);
			RemoveSequence(MulLevel,Position);
			return 1;
		}

		if (is_argument_pure)
		{
			if ((fabs(ArgVal-10.0)>1e-100) || (fabs(ExpVal-(int)ExpVal)>1e-100))
			{
				double N2=ArgVal;
				int is_int=0;
				int add_imaginary=0;
				int precision2=max(arg_prec,exp_prec);

				//round the exponent value if it is near integer
				if (fabs(ExpVal-(long long)ExpVal)<1e-100) 
				{
					is_int=1;
					ExpVal=(double)((long long)(ExpVal+((ExpVal>=0)?0.01:-0.01)));
				}

				double Re,Im=0;
				if ((N2<0) && (is_int==0))
				{
					//result might be imaginary number
					precision2++;
					N2=-N2;
					Re=cos(3.14159265359*ExpVal);
					Im=sin(3.14159265359*ExpVal);	
					if (fabs(Im)>1e-10)
						add_imaginary=1;
					else
						Im=0.0;
					if (fabs(Re)<1e-10) Re=0.0;
					Re*=pow(N2,ExpVal);
					Im*=pow(N2,ExpVal);	
				}
				else
					Re=pow(N2,ExpVal);


				if (add_imaginary)
				{
					if ((fabs(Im-1.0)<1e-100) && (ComputationType==0))
					{
						DeleteElement(Position);

						if (ImaginaryUnit)
							InsertEmptyElement(Position,1,ImaginaryUnit);
						else
						{
							InsertEmptyElement(Position,8,1);
							CExpression *tmp=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
							tmp->InsertEmptyElement(0,2,'-');
							tmp->InsertEmptyElement(1,1,'1');
						}

						long long N2_int=(long long)(Re+((Re<0)?-0.5:0.5));
						if (fabs(Re-N2_int)<1e-100) is_int=1; else is_int=0;
						GenerateASCIINumber(Re,N2_int,is_int,precision2,Position);

						return 1;
					}
					else if (ComputationType==0)
					{
						DeleteElement(Position);

						InsertEmptyElement(Position,5,'(');
						CExpression *tmp=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
						if (ImaginaryUnit)
							tmp->InsertEmptyElement(0,1,ImaginaryUnit);
						else
						{
							tmp->InsertEmptyElement(0,8,1);
							CExpression *tmp2=(CExpression*)((tmp->m_pElementList)->pElementObject->Expression1);
							tmp2->InsertEmptyElement(0,2,'-');
							tmp2->InsertEmptyElement(1,1,'1');
						}
						long long N2_int=(long long)(Im+((Im<0)?-0.5:0.5));
						if (fabs(Im-N2_int)<1e-100) is_int=1; else is_int=0;
						tmp->GenerateASCIINumber(Im,N2_int,is_int,precision2,0);
						tmp->InsertEmptyElement(0,2,'+');
						N2_int=(long long)(Re+((Re<0)?-0.5:0.5));
						if (fabs(Re-N2_int)<1e-100) is_int=1; else is_int=0;
						tmp->GenerateASCIINumber(Re,N2_int,is_int,precision2,0);
						return 1;
					}
				}
				else
				{
					DeleteElement(Position);

					long long N2_int=(long long)(Re+((Re<0)?-0.5:0.5));
					if (fabs(Re-N2_int)<1e-100) is_int=1; else is_int=0;
					GenerateASCIINumber(Re,N2_int,is_int,precision2,Position);

					return 1;
				}
			}
		}
	}

	if (ComputationType==2) return 0;  //ComputationType==2 -> for fraction summing, root multiplication 
	if (ComputationType==3) return 0;  //ComputationType==3 -> simple calculations only

	int ArgLevel=arg->FindLowestOperatorLevel();

	//inversion of a matrix
	if ((is_exponent_pure) && (fabs(ExpVal-(-1.0))<1e-100))
	{
		CExpression *a=arg;
		if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==5))
			a=(CExpression*)(arg->m_pElementList->pElementObject->Expression1);
		if (a->FindLowestOperatorLevel()==GetOperatorLevel((char)0xFF))
		{
			if (a->MatrixInvert(ComputationType)) 
			{
				exp->Delete();
				exp->InsertEmptyElement(0,1,'1');
				return 1;
			}
		}
	}

	//matrix transpose
	if ((exp->m_NumElements==1) && (exp->m_pElementList->Type==1) &&
		(*(unsigned short*)(exp->m_pElementList->pElementObject->Data1)==0x0054) && //this is "T" string
		((exp->m_pElementList->pElementObject->m_VMods)==0) &&
		((exp->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60))
	{
		CExpression *a=arg;
		if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==5))
			a=(CExpression*)(arg->m_pElementList->pElementObject->Expression1);
		if (a->FindLowestOperatorLevel()==GetOperatorLevel((char)0xFF))
		{
			int rows,columns;
			a->GetMatrixSize(&rows,&columns);
			if ((rows>1) || (columns>1))
			{
				CExpression *M=new CExpression(NULL,NULL,100);
				for (int ii=0;ii<columns;ii++)
				{
					for (int jj=0;jj<rows;jj++)
					{
						int pos=a->FindMatrixElement(jj,ii,0);
						if (pos<0) pos=0;
						char et,p;
						int l=a->GetElementLen(pos,a->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
						for (int k=pos;k<pos+l;k++)
							M->InsertElement(a->m_pElementList+k,M->m_NumElements);

						if (jj<rows-1) M->InsertEmptyElement(M->m_NumElements,11,0);
					}
					if (ii<columns-1) M->InsertEmptyElement(M->m_NumElements,12,0);
				}

				int ps=a->m_ParentheseShape;
				//int pd=a->m_ParentheseData;
				//int ph=a->m_ParentheseHeightFactor;
				int fp=a->m_ParenthesesFlags;
				a->CopyExpression(M,0);
				a->m_ParentheseShape=ps;
				//a->m_ParentheseData=pd;
				//a->m_ParentheseHeightFactor=ph;
				a->m_ParenthesesFlags=fp;
				delete M;
				exp->Delete();
				exp->InsertEmptyElement(0,1,'1');
				return 1;
			}
		}
	}

	// if minus sign is argument of exponential function 
	// if the sign is plus, just delete the + sign,
	// if the sign is minus and the exponent is pure integer then depends if exp. is odd or even
	if (((ArgLevel==-1) || (ArgLevel>=MulLevel)) &&
		(arg->m_NumElements>1) &&
		(arg->m_pElementList->Type==2))
	{
		char *ch=&(arg->m_pElementList->pElementObject->Data1[0]);
		if (*ch=='+') 
		{
			arg->DeleteElement(0);
		}
		else if ((is_exponent_pure) && (fabs(ExpVal-(long long)ExpVal)<1e-100))
		{
			long long n=(long long)ExpVal;
			if ((n&0x01)==0) 
			{
				arg->DeleteElement(0);
			}
			else
			{
				if ((Position) && ((m_pElementList+Position-1)->Type==2) &&
					((m_pElementList+Position-1)->pElementObject->Data1[0]==element_type)) Position--;
				InsertEmptyElement(Position,5,'(');
				CExpression *tmp=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
				tmp->InsertEmptyElement(0,2,*ch);
				tmp->InsertEmptyElement(1,1,'1');
				arg->DeleteElement(0);
				return 1;
			}
		}
	}



	//exponent of summation (x+y)^n
	if (ComputationType==0)
	if ((ArgLevel==PlusLevel) && (is_exponent_pure))
	{
		if ((fabs(ExpVal-(long long)ExpVal)<1e-100) && (ExpVal>1.0) && (ExpVal<6.0))
		{
			int n=(int)(ExpVal+((ExpVal>=0)?0.01:-0.01));
			for (int kk=0;kk<n;kk++)
			{
				InsertEmptyElement(Position+1,5,'(');
				CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
				tmp->CopyExpression(arg,0);tmp->m_ParenthesesFlags=0x02;
				if (kk<n-1)
				if (element_type=='/') InsertEmptyElement(Position+1,2,'/');
			}
			RemoveSequence(MulLevel,Position,Position);
			return 1;
		}
	}



	//exponent of fraction
	if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==4))
	{
		CExpression *nom=(CExpression*)(arg->m_pElementList->pElementObject->Expression1);
		CExpression *denom=(CExpression*)(arg->m_pElementList->pElementObject->Expression2);
		if ((nom) && (denom))
		{
			InsertEmptyElement(Position+1,4,0);
			CExpression *n=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
			CExpression *d=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression2);

			n->InsertEmptyElement(0,3,0);
			CExpression *tmp=(CExpression*)(n->m_pElementList->pElementObject->Expression1);
			CExpression *tmp2=(CExpression*)(n->m_pElementList->pElementObject->Expression2);
			tmp2->CopyExpression(exp,0);
			tmp->CopyExpression(nom,0);tmp->m_ParenthesesFlags=0x02;;

			d->InsertEmptyElement(0,3,0);
			tmp=(CExpression*)(d->m_pElementList->pElementObject->Expression1);
			tmp2=(CExpression*)(d->m_pElementList->pElementObject->Expression2);
			tmp2->CopyExpression(exp,0);
			tmp->CopyExpression(denom,0);tmp->m_ParenthesesFlags=0x02;

			DeleteElement(Position);
			return 1;
		}
	}

	//exponent of exponent
	if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==3))
	{
		CExpression *a=(CExpression*)(arg->m_pElementList->pElementObject->Expression1);
		CExpression *e=(CExpression*)(arg->m_pElementList->pElementObject->Expression2);
		if ((a) && (e))
		{
			int explevel=exp->FindLowestOperatorLevel((char)0xD7);
			if (explevel<MulLevel)
			{
				exp->InsertEmptyElement(0,5,'(');
				CExpression *tmp=(CExpression*)(exp->m_pElementList->pElementObject->Expression1);
				int cc=exp->m_NumElements;
				for (int kk=1;kk<cc;kk++)
				{
					tmp->InsertElement(exp->m_pElementList+1,kk-1);
					exp->DeleteElement(1);
				}
			}
			int exp2level=e->FindLowestOperatorLevel((char)0xD7);
			if ((exp2level<MulLevel) || (e->m_pElementList->Type==2))
			{
				exp->InsertEmptyElement(exp->m_NumElements,5,'(');
				CExpression *tmp=(CExpression*)((exp->m_pElementList+exp->m_NumElements-1)->pElementObject->Expression1);
				tmp->CopyExpression(e,0);tmp->m_ParenthesesFlags=0x02;
			}
			else
			{
				for (int kk=0;kk<e->m_NumElements;kk++)
					exp->InsertElement(e->m_pElementList+kk,exp->m_NumElements);
			}

			for (int kk=0;kk<a->m_NumElements;kk++)
				arg->InsertElement(a->m_pElementList+kk,arg->m_NumElements);

			arg->DeleteElement(0);
			return 1;
		}
	}

	//exponent of logarithm (X^logx(y))
	if ((exp->m_NumElements==1) && (exp->m_pElementList->Type==6))
	{
		int is_log=0;
		int is_ln=0;
		if ((strnicmp(exp->m_pElementList->pElementObject->Data1,"log",3)==0) &&
			(strlen(exp->m_pElementList->pElementObject->Data1)==3)) is_log=1;
		if ((strnicmp(exp->m_pElementList->pElementObject->Data1,"ln",2)==0) &&
			(strlen(exp->m_pElementList->pElementObject->Data1)==2)) is_ln=1;
		if ((is_log) || (is_ln))
		{
			CExpression *a=(CExpression*)(exp->m_pElementList->pElementObject->Expression1);
			CExpression *base=(CExpression*)(exp->m_pElementList->pElementObject->Expression2);
			if ((a) && (base) && (arg->CompareExpressions(0,-1,base,0,-1)))
			{
				InsertEmptyElement(Position+1,5,'(');
				CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
				tmp->CopyExpression(a,0);
				DeleteElement(Position);
				return 1;
			}
			else if ((a) && (is_log))
			{
				double N;
				int prec;
				if (arg->IsPureNumber(0,arg->m_NumElements,&N,&prec))
					if (fabs(N-10.0)<1e-100)										
					{
						InsertEmptyElement(Position+1,5,'(');
						CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
						tmp->CopyExpression(a,0);
						DeleteElement(Position);
						return 1;
					}
			} 
			else if ((a) && (is_ln))
			{
				if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==1) &&
					(*(unsigned short*)(arg->m_pElementList->pElementObject->Data1)==0x0065) && //this is "e" string
					(((arg->m_pElementList->pElementObject->Data2[0])&0xE4)==0))
				{
					InsertEmptyElement(Position+1,5,'(');
					CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
					tmp->CopyExpression(a,0);
					DeleteElement(Position);
					return 1;
				}
			}
		}
	}

	//exponent of root  sqrt(x)^y
	if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==8))
	{
		CExpression *a=(CExpression*)(arg->m_pElementList->pElementObject->Expression1);
		CExpression *base=(CExpression*)(arg->m_pElementList->pElementObject->Expression2);
		if (a)
		{
			if ((base) && (base->CompareExpressions(0,-1,exp,0,-1)))
			{
				InsertEmptyElement(Position+1,5,'(');
				CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
				tmp->CopyExpression(a,0);tmp->m_ParenthesesFlags=0x02;
				DeleteElement(Position);
				return 1;
			}
			int prec;
			double N;
			if ((base==NULL) && (exp->IsPureNumber(0,exp->m_NumElements,&N,&prec)))
				if (fabs(N-2.0)<1e-100)
				{
					InsertEmptyElement(Position+1,5,'(');
					CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
					tmp->CopyExpression(a,0);tmp->m_ParenthesesFlags=0x02;
					DeleteElement(Position);
					return 1;
				}


			if (base)
				base->IsPureNumber(0,-1,&N,&prec);
			else
				N=2.0;
			if ((N>1.0) && (fabs(N-(long long)N)<1e-100))
			{
				tPureFactors PF;
				PF.N1=PF.N2=1.0;
				PF.prec1=0;
				PF.is_frac1=0;

				int rv=exp->StrikeoutCommonFactors(0,exp->m_NumElements-1,1,NULL,0,0,1,&PF);
				exp->StrikeoutRemove(0,exp->m_NumElements-1);
				if (fabs(N-1.0)>1e-100)
				{
					double n=PF.N1/N;
					if (fabs(n-(long long)n)<1e-100)
					{
						InsertEmptyElement(Position+1,5,'(');
						CExpression *tmp=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
						tmp->InsertEmptyElement(0,3,0);
						CExpression *tmp2=(CExpression*)(tmp->m_pElementList->pElementObject->Expression1);
						CExpression *tmp3=(CExpression*)(tmp->m_pElementList->pElementObject->Expression2);
						tmp2->CopyExpression(a,0);tmp2->m_ParenthesesFlags=0x02;
						tmp3->InsertEmptyElement(0,4,0);
						CExpression *tmp4=(CExpression*)(tmp3->m_pElementList->pElementObject->Expression1);
						CExpression *tmp5=(CExpression*)(tmp3->m_pElementList->pElementObject->Expression2);
						tmp4->CopyExpression(exp,0);
						if (base) 
							tmp5->CopyExpression(base,0);
						else
							tmp5->InsertEmptyElement(0,1,'2');
						DeleteElement(Position);
						retval=1;
					}
				}
			}
			if (retval) return retval;
		}
	}

	//combined argument: (abc)^x  ->  a^x b^x c^x
	if ((ComputationType==1) ||
		((exp->m_pElementList->Type==1) && (exp->m_NumElements==1)) ||
		((exp->m_pElementList->Type==2) && ((exp->m_pElementList+1)->Type==1) && (exp->m_NumElements==2)))
	{
		CExpression *marg=arg;
		int Level=ArgLevel;
		if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==5))
		{
			marg=(CExpression*)(arg->m_pElementList->pElementObject->Expression1);
			Level=marg->FindLowestOperatorLevel();
		}
		if (Level==MulLevel)
		{
			int pos=0;
			int gg=0;
			while (1)
			{
				char et;
				char p;
				int l;
				l=marg->GetElementLen(pos,marg->m_NumElements-1,Level,&et,&p);
				if (l-p==0) break;

				if (pos>0)
					if (((element_type=='/') && (et!='/')) || 
						((element_type!='/') && (et=='/')))
					{
						InsertEmptyElement(Position+1+gg,2,'/');
						gg++;
					}
				InsertEmptyElement(Position+1+gg,3,0);
				CExpression *a=(CExpression*)((m_pElementList+Position+1+gg)->pElementObject->Expression1);
				CExpression *e=(CExpression*)((m_pElementList+Position+1+gg)->pElementObject->Expression2);
				gg++;
				for (int kk=pos+p;kk<pos+l;kk++)
					a->InsertElement(marg->m_pElementList+kk,a->m_NumElements);
				e->CopyExpression(exp,0);

				pos+=l;
				if (pos>marg->m_NumElements-1) break;
			}

			DeleteElement(Position);

			return 1;
		}
	}

	//complex exponent a^(x+y)  ->   a^x a^y
	if (ComputationType==1)
	{
		int lvl=exp->FindLowestOperatorLevel();
		if (lvl==PlusLevel)
		{
			CExpression *tmpb=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
			int pos=0;
			int zz=0;
			while (1)
			{
				char et;
				char p;
				int l=exp->GetElementLen(pos,exp->m_NumElements-1,lvl,&et,&p);
				if (l==0) break;

				zz++;
				InsertEmptyElement(Position+zz,3,0);
				CExpression *tmpa=(CExpression*)((m_pElementList+Position+zz)->pElementObject->Expression1);
				tmpa->CopyExpression(tmpb,0);tmpa->m_ParenthesesFlags=tmpb->m_ParenthesesFlags;
				CExpression *tmp=(CExpression*)((m_pElementList+Position+zz)->pElementObject->Expression2);
				for (int kk=pos+p;kk<pos+l;kk++)
					tmp->InsertElement(exp->m_pElementList+kk,kk-pos-p);
				if (et!='+') tmp->InsertEmptyElement(0,2,et);

				pos+=l;
				if (pos>exp->m_NumElements-1) break;
			}
			DeleteElement(Position);
			return 1;
		}
	}

	return retval;
}



int ExtractVariablesMode=0;

int CExpression::FactorizeExpression(int force_factorization)
{
	//if already factorized we don't try it again because factorization is very slow
	if (m_IsComputed&0x00800000) return 0;
	m_IsComputed|=0x00800000;

	//check if the expression is at all a candidate for factorization
	{
		if ((m_NumElements==1) && (m_pElementList->Type<=2)) return 0;

		int iii;
		tElementStruct *ts=m_pElementList;
		for (iii=0;iii<m_NumElements;iii++,ts++)
			if (ts->Type!=1) break;
		if (iii==m_NumElements) return 0;
	}

	PROFILE_TIME(&PROFILER.FactorizeExpression);
	

	int is_first_pass=1;
	CExpression *original=new CExpression(NULL,NULL,100);
	original->CopyExpression(this,0);

factorizeexpression_try_again:

	{
		//prepare for factorization
		int cnt=0;
		while ((Compute(0,m_NumElements-1,1)) && (cnt<50)) cnt++;
	}

	int Level=FindLowestOperatorLevel((char)0xD7);
	if (Level>MulLevel) goto factorizeexpression_exit;

	
	if (Level==MulLevel)
	{
		int any_change=0;
		int pos=0;
		while (1)
		{
			char et,p;
			int l=GetElementLen(pos,m_NumElements-1,Level,&et,&p);
			if (l==0) break;

			tElementStruct *ts=m_pElementList+pos+p;

			// jump over +/- sign
			if ((l>1) && (ts->Type==2) &&
				(GetOperatorLevel(ts->pElementObject->Data1[0])==PlusLevel))
			{
				ts++;pos++;l--;
			}

			if (l-p==1)
			{
				if (ts->Type==5)
				{
					CExpression *tmp=(CExpression*)(ts->pElementObject->Expression1);
					if (tmp) if (tmp->FactorizeExpression()) any_change=1;
				}
			}

			pos+=l;
			if (pos>m_NumElements-1) break;
		}

		if (any_change)
		{
			delete original;
			xreturn (1);
		}
		else
			goto factorizeexpression_exit;
	}

	if (m_NumElements<=2) 
	{
		goto factorizeexpression_exit;
	}


	
	// *********************************************************************************
	// FACTORIZATION BY GROUPING
	//
	// By calling the 'ExtractVariables' for every summand in expression, the expression
	// is divided (analyzed) to its variables and constants. 
	// All combinations of summands are then checked.
	// *********************************************************************************

	//TODO - ExtractVariables treba preraditi da vraca X^(3.3) -> X^3*X^0.3

	//make some additional checkings (to proceed the expression must be summation of summands)
	if (Level>PlusLevel) Level=PlusLevel;
	if (Level<PlusLevel) goto factorizeexpression_exit;

	//prepare sturcture that will hold list of all variables in this expression
	tVariableList *VarList;
	int summand_no;
	VarList=(tVariableList*)this->GenerateVariableList(0,m_NumElements-1,&summand_no);
	if (!VarList) goto factorizeexpression_exit;


	// try various combinations of groups (there must be integer number of groups and
	// integer number of summands in a group)
	int zz=summand_no;
	while (((double)zz>=sqrt((double)summand_no)-1e-100) && (zz>1))
	{
		if (summand_no%zz) {zz--;continue;}
		int n=zz;  				   //number of summands in group
		int groups=summand_no/n;   //number of groups (how many groups we can make)
		zz--;

		//the following structure is used when calculating various combinations of
		//summands in groups.
		struct
		{
			int summand_pos[MAX_SUMMANDS];  //relative position of summand ('3' means third non-used summand)
			int summand_rpos[MAX_SUMMANDS]; //real possition of summand in expression
			int N;							//number of free summands
		} sstate[10];

		//initially buld the above structure
		for (int s=0;s<groups;s++)
		{
			for (int i=0;i<n;i++)
				sstate[s].summand_pos[i]=i;
			sstate[s].N=summand_no-s*n;
		}

		
		tVariableList *commVarList; //make temporary varibale list - for commons
		tVariableList *tmpVarList; //make temporary variable list - for leftovers
		commVarList=(tVariableList*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,2*sizeof(tVariableList));
		tmpVarList=commVarList+1;

		int try_count=0;
		int finished=0;
		while ((!finished) && (try_count<15000))
		{
			try_count++;
			
			//find real positions of summands that will be grouped (jump over used ones)
			char summand_used[MAX_SUMMANDS];
			memset(&summand_used,0,MAX_SUMMANDS);
			for (int s=0;s<groups;s++)
			{
				int pos=0;
				int frees=0;
				for (int i=0;i<n;i++)
				{
					while ((summand_used[pos]) || (frees<sstate[s].summand_pos[i]))
					{
						if (summand_used[pos]==0) frees++;
						pos++;
					}
					sstate[s].summand_rpos[i]=pos;
					summand_used[pos]=1;
					frees++;
					pos++;
				}
			}

			memcpy(tmpVarList,VarList,sizeof(tVariableList));
			memset(commVarList,0,sizeof(tVariableList));

			//extract all common factors from groups
			for (int s=0;s<groups;s++)
			{				
				for (int i=0;i<VarList->NumVariables;i++)
				{
					int rpos;
					rpos=sstate[s].summand_rpos[0];
					double max_order=tmpVarList->Variables[i].summand[rpos].dorder;
					if (fabs(max_order)>1e-100)
					{
						int k;
						if (max_order>0)
							for (k=1;k<n;k++)
							{
								rpos=sstate[s].summand_rpos[k];
								double d=tmpVarList->Variables[i].summand[rpos].dorder;
								if (d<1e-100) break;
								if (d<max_order) max_order=d;
							}
						else
							for (k=1;k<n;k++)
							{
								rpos=sstate[s].summand_rpos[k];
								double d=tmpVarList->Variables[i].summand[rpos].dorder;
								if (d>-1e-100) break;
								if (d>max_order) max_order=d;
							}

						if (k==n)
						{
							rpos=sstate[s].summand_rpos[0];
							commVarList->Variables[i].summand[rpos].dorder=max_order;
							tmpVarList->Variables[i].summand[rpos].dorder-=max_order;
							for (int k=1;k<n;k++)
							{
								rpos=sstate[s].summand_rpos[k];
								tmpVarList->Variables[i].summand[rpos].dorder-=max_order;
							}
						}
					}

					if (i==0)
					{
						//check constants
						int prec;
						int rpos=sstate[s].summand_rpos[0];
						double N1=tmpVarList->Constants[rpos].N1;
						double N2=tmpVarList->Constants[rpos].N2;
						int order1=0;
						int order2=0;
						if (N1>1e+18) {order1=(int)log10(N1)-10;N1/=pow(10.0,order1);}
						if (N2>1e+18) {order2=(int)log10(N2)-10;N2/=pow(10.0,order2);}
						if (fabs(N1)>=1.0)
							while (fabs(N1/10-(long long)(N1/10))<1e-100) {N1/=10;order1++;}
						else
							while (fabs(N1-(long long)(N1))>1e-100) {N1=N1*10;order1--;}
						if (fabs(N2)>=1.0)
							while (fabs(N2/10-(long long)(N2/10))<1e-100) {N2/=10;order2++;}
						else
							while (fabs(N2-(long long)(N2))>1e-100) {N2=N2*10;order2--;}

						tmpVarList->Constants[rpos].N1=1.0;
						tmpVarList->Constants[rpos].N2=1.0;

						prec=VarList->Constants[rpos].prec;
						for (int k=1;k<n;k++)
						{
							int rpos=sstate[s].summand_rpos[k];
							double N3=tmpVarList->Constants[rpos].N1;
							double N4=tmpVarList->Constants[rpos].N2;
							int order3=0;
							int order4=0;
							if (N3>1e+18) {order3=(int)log10(N3)-10;N3/=pow(10.0,order3);}
							if (N4>1e+18) {order4=(int)log10(N4)-10;N4/=pow(10.0,order4);}
							if (fabs(N3)>=1.0)
								while (fabs(N3/10-(long long)(N3/10))<1e-100) {N3/=10;order3++;}
							else
								while (fabs(N3-(long long)(N3))>1e-100) {N3=N3*10;order3--;}
							if (fabs(N4)>=1.0)
								while (fabs(N4/10-(long long)(N4/10))<1e-100) {N4/=10;order4++;}
							else
								while (fabs(N4-(long long)(N4))>1e-100) {N4=N4*10;order4--;}

							if (VarList->Constants[rpos].prec>prec) prec=VarList->Constants[rpos].prec;

							int steps=0;
							while ((order1>order3) && (order3>=0)) {N1*=10;order1--;steps++;if (steps>6) break;}
							while ((order3>order1) && (order1>=0)) {N3*=10;order3--;steps++;if (steps>6) break;}
							while ((order1>order3) && (order1<0)) {N1*=10;order1--;steps++;if (steps>6) break;}
							while ((order3>order1) && (order3<0)) {N3*=10;order3--;steps++;if (steps>6) break;}
/*
							if (((order1>0) && (order3>0)) || ((order1<0) && (order3<0)))
							{
								double e=1.0;
								if (order1+order3>=0)
								{
									int ord=min(order1,order3);
									e=pow(10.0,ord);
									order1-=ord;
									order3-=ord;
								}
								else
								{
									int ord=max(order1,order3);
									e=pow(10.0,ord);
									order1-=ord;
									order3-=ord;
								}

								if (fabs(e)>1.0e-100)
								{
									double delta=N1/e;
									N1=e;
									for (int kk=0;kk<k;kk++)
										tmpVarList->Constants[sstate[s].summand_rpos[kk]].N1*=delta;
									tmpVarList->Constants[rpos].N1/=N1*pow(10.0,order1+order2);


									int delta=order1;
									order1=((order1+order3)>=0)?min(order1,order3):max(order1,order3);
									order3-=order1;
									double tmp1;
									if (delta>=order1)
										tmp1=pow(10.0,delta-order1);
									else
										tmp1=1/pow(10.0,order1-delta);
									for (int kk=0;kk<k;kk++)
										tmpVarList->Constants[sstate[s].summand_rpos[kk]].N1*=tmp1;
									if (order1>=0)
										tmpVarList->Constants[rpos].N1/=pow(10.0,order1);
									else
										tmpVarList->Constants[rpos].N1*=pow(10.0,-order1);
									
								}
							}
*/

							double e=1.0;
							double oN1=N1;
							double oN3=N3;
							ReduceTwoNumbers(&oN1,&oN3);
							e=N1/oN1;
							if ((N1<0) && (N3<0)) e*=-1.0;
							/*for (int kk=2;kk<40;kk++)
							{
								double t1=oN1/(double)kk;
								double t2=oN3/(double)kk;
								if ((fabs(t1-(long long)t1)<1e-100) && (fabs(t2-(long long)t2)<1e-100))
								{
									oN1=t1;
									oN3=t2;
									e*=kk;
									kk--;
								}
							}*/ 
							//if (e>1.0)
							{

								double delta=N1/e;
								N1=e;
								for (int kk=0;kk<k;kk++)
									tmpVarList->Constants[sstate[s].summand_rpos[kk]].N1*=delta;
								tmpVarList->Constants[rpos].N1/=N1*pow(10.0,order1);
							}

							e=1.0;
							double oN2=N2;
							double oN4=N4;
							ReduceTwoNumbers(&oN2,&oN4);
							e=N2/oN2;
							if ((N2<0) && (N4<0)) e*=-1.0;
							/*
							for (int kk=2;kk<40;kk++)
							{
								double t1=oN2/(double)kk;
								double t2=oN4/(double)kk;
								if ((fabs(t1-(long long)t1)<1e-100) && (fabs(t2-(long long)t2)<1e-100))
								{
									oN2=t1;
									oN4=t2;
									e*=kk;
									kk--;
								}
							}
							*/
							//if (e>1.0)
							{
								double delta=N2/e;
								N2=e;
								for (int kk=0;kk<k;kk++)
									tmpVarList->Constants[sstate[s].summand_rpos[kk]].N2*=delta;
								tmpVarList->Constants[rpos].N2/=N2*pow(10.0,order2);
							}
						}
						double ttmp;
						if (order1>=0) ttmp=pow(10.0,order1); else ttmp=1.0/pow(10.0,-order1);
						commVarList->Constants[rpos].N1=N1*ttmp;
						if (order2>=0) ttmp=pow(10.0,order2); else ttmp=1.0/pow(10.0,-order2);
						commVarList->Constants[rpos].N2=N2*ttmp;
						commVarList->Constants[rpos].prec=prec;
					}
				}
			}

			//now compare if leftovers of all groups are equal
			char checked[MAX_SUMMANDS];

			int found_sollution=1;
			for (int s=1;s<groups;s++)
			{
				memset(checked,0,n);

				int founds=0;
				double ratio=0;
				for (int kk=0;kk<n;kk++)
				{
					int fnd=0;
					int rpos0=sstate[0].summand_rpos[kk];
					for (int k=0;k<n;k++)
					{
						if (checked[k]==0)
						{
							int rpos=sstate[s].summand_rpos[k];
							int i=0;
							for (i=0;i<VarList->NumVariables;i++)
							{
								double ord1=tmpVarList->Variables[i].summand[rpos].dorder;
								double ord2=tmpVarList->Variables[i].summand[rpos0].dorder;
								if (fabs(ord1-ord2)>1e-100) break;
							}
							if (i==VarList->NumVariables)
							{
								double N1=tmpVarList->Constants[rpos].N1;
								double N2=tmpVarList->Constants[rpos].N2;
								double N3=tmpVarList->Constants[rpos0].N1;
								double N4=tmpVarList->Constants[rpos0].N2;
								if ((fabs(N2)>1e-100) && (fabs(N4)>1e-100)) 
								if ((fabs(N1)>1e-100) && (fabs(N3)>1e-100))
								{
									N1/=N2;
									N3/=N4;
									if (ratio==0)
										ratio=(double)N1/(double)N3;
									else 
										if (fabs(ratio*N3-N1)>1e-100) continue;

									//we found the identical element
									fnd=1;
									checked[k]=1;
									founds++;
									break;
								}
							}
						}
					}
					if (!fnd) break;
				}
				if (founds<n) {found_sollution=0;break;}
				if ((founds==n) && (fabs(ratio)>1e-100) && (fabs(ratio-1.0)>1e-100))
				{
					int rpos=sstate[s].summand_rpos[0];
					commVarList->Constants[rpos].N1*=ratio;
					for (int kk=0;kk<n;kk++)
					{
						rpos=sstate[s].summand_rpos[kk];
						tmpVarList->Constants[rpos].N1/=ratio;
					}
				}
			}

			if ((found_sollution) && (commVarList->NumVariables==0) && (force_factorization==0) &&
				(fabs(commVarList->Constants[0].N1+1.0)<1e-100) &&
				(fabs(commVarList->Constants[0].N2-1.0)<1e-100))
			{
				found_sollution=0; //we do not allow extraction of minus 1 (this would not a nice factorization, anyway)
			}

			if (found_sollution) 
			{
				if (groups==1)
				{
					for (int i=0;i<VarList->NumVariables;i++)
					{
						for (int k=0;k<summand_no;k++)
							if (fabs(commVarList->Variables[i].summand[k].dorder)>1e-100) {finished=2;break;}
					}
					if (fabs(commVarList->Constants[0].N1-1.0)>1e-100) finished=2;
					if (fabs(commVarList->Constants[0].N2-1.0)>1e-100) finished=2;
				}
				else
					finished=2;
			}
			
			if (!finished)
			{
				//calculate new positions for next generation
				sstate[groups-1].summand_pos[n-1]++;
				for (int s=groups-1;s>=0;s--)
				{
					for (int i=n-1;i>=1;i--)
					{
						if (sstate[s].summand_pos[i]>sstate[s].N-n+i) 
						{
							if (i==1) 
							{
								if (s==0) {finished=1;break;}
								for (int ii=0;ii<n;ii++) sstate[s].summand_pos[ii]=ii;
								sstate[s-1].summand_pos[n-1]++;
							}
							else
							{
								sstate[s].summand_pos[i-1]++;
								for (int ii=i;ii<n;ii++)
									sstate[s].summand_pos[ii]=sstate[s].summand_pos[ii-1]+1;
								//sstate[s].summand_pos[i]=sstate[s].summand_pos[i-1]+1;
							}
						}
						if (finished) break;
					}
					if (finished) break;
				}
			}
		}

		if (finished==2)
		{
			//print out the sollution
			CExpression *factorized=new CExpression(NULL,NULL,100);
			CExpression *arg;
			if (groups>1)
			{
				factorized->InsertEmptyElement(0,5,'(');
				arg=(CExpression*)((factorized->m_pElementList+0)->pElementObject->Expression1);
			}
			else
				arg=factorized;

			commVarList->NumVariables=VarList->NumVariables;
			for (int kk=0;kk<VarList->NumVariables;kk++)
			{
				commVarList->Variables[kk].exponent=VarList->Variables[kk].exponent;
				commVarList->Variables[kk].len=VarList->Variables[kk].len;
				commVarList->Variables[kk].pos=VarList->Variables[kk].pos;
				commVarList->Variables[kk].variable=VarList->Variables[kk].variable;
			}

			for (int s=0;s<groups;s++)
			{
				int rpos=sstate[s].summand_rpos[0];

				
				int ppos=arg->m_NumElements;if (arg->m_pElementList->Type==0) ppos=0;
				arg->SynthetizeExpression(ppos,(void*)commVarList,rpos);
			}


			factorized->InsertEmptyElement(factorized->m_NumElements,5,'(');
			arg=(CExpression*)((factorized->m_pElementList+factorized->m_NumElements-1)->pElementObject->Expression1);
			for (int k=0;k<n;k++)
			{
				int rpos=sstate[0].summand_rpos[k];

				int ppos=arg->m_NumElements;if (arg->m_pElementList->Type==0) ppos=0;
				arg->SynthetizeExpression(ppos,(void*)tmpVarList,rpos);
			}


			int tmpp=this->m_ParenthesesFlags;
			CopyExpression(factorized,0);this->m_ParenthesesFlags=tmpp;
			delete factorized;
		}

		HeapFree(ProcessHeap,0,commVarList);

		if (finished==2) 
		{
			//delete all temporary created objects
			this->FreeVariableList(VarList);
			goto factorizeexpression_finish;
		}
	}
	//delete all temporary created objects
	this->FreeVariableList(VarList);


	if (is_first_pass)
	{
		is_first_pass=0;
		int cnt=0;
		while ((Compute(0,m_NumElements-1,0)) && (cnt<50)) {cnt++;}
		if (cnt) goto factorizeexpression_try_again;
	}



	//try for quadratic formula


	int retval=0;
	CExpression *tmp=new CExpression(NULL,NULL,100);
	for (int kk=0;kk<=m_NumElements-1;kk++)
		tmp->InsertElement(m_pElementList+kk,tmp->m_NumElements);


	
	int order=tmp->Polynomize(NULL);
	if (order==2)
	{
		CExpression *tmp2=new CExpression(NULL,NULL,100);
		int lvl=tmp->FindLowestOperatorLevel();
		if (lvl==PlusLevel)
		{
			char et,p;
			int l=tmp->GetElementLen(0,tmp->m_NumElements-1,lvl,&et,&p);
			if ((tmp->m_pElementList+l-1)->Type==3)
			{
				tPureFactors aPF;
				aPF.N1=aPF.N2=1.0;
				aPF.is_frac1=0;
				aPF.prec1=aPF.prec2=0;
				if (l>=2)
				{
					//checking if 'a' has some pure-number factor
					//if it has, we will temporarely remove, but will have to add it later (check for aPF variable)
		
					tmp->StrikeoutCommonFactors(0,l-2,1,NULL,0,0,1,&aPF);
					tmp->StrikeoutRemove(0,l-2);
					if (fabs(aPF.N1/aPF.N2-1.00)>1e-100)
					{
						CExpression *arg=(CExpression*)((tmp->m_pElementList+l-1)->pElementObject->Expression1);
						CExpression *variable=new CExpression(NULL,NULL,100);
						variable->CopyExpression(arg,0);

						tmp->InsertEmptyElement(0,5,'(');
						for (int jj=1;jj<tmp->m_NumElements;jj++)
							((CExpression*)tmp->m_pElementList->pElementObject->Expression1)->InsertElement(tmp->m_pElementList+jj,jj-1);
						while (tmp->m_NumElements>1)
							tmp->DeleteElement(1);
						tmp->GenerateASCIIFraction(0,aPF.N2,aPF.N1,max(aPF.prec1,aPF.prec2),1);
						
						int cntr=0;
						while ((tmp->Compute(0,tmp->m_NumElements-1,0)) && (cntr<50)) cntr++;
						tmp->Polynomize(variable);
						l=tmp->GetElementLen(0,tmp->m_NumElements-1,lvl,&et,&p);
						delete variable;
						//this->CopyExpression(tmp,0);
						//goto factorizeexpression_finish;
					}
				}
				CExpression *exp=(CExpression*)((tmp->m_pElementList+l-1)->pElementObject->Expression2);
				CExpression *arg=(CExpression*)((tmp->m_pElementList+l-1)->pElementObject->Expression1);
				double N;
				int prec;
				if ((arg) && (exp) && (exp->IsPureNumber(0,exp->m_NumElements,&N,&prec)) &&
					(fabs(N-2.0)<1e-100))
				{
					char et2,p2;
					int l2=tmp->GetElementLen(l,tmp->m_NumElements-1,lvl,&et2,&p2);
					if (l2-p2>=arg->m_NumElements)
					{
						if (arg->CompareExpressions(0,arg->m_NumElements-1,tmp,l2+l-arg->m_NumElements,l2+l-1))
						{
							//this is factor for x^1
							char et3,p3;
							int l3=tmp->GetElementLen(l+l2,tmp->m_NumElements-1,lvl,&et3,&p3);
							if (l3)
							{
								//build tmp2
								tmp2->InsertEmptyElement(0,5,'(');
								CExpression *factor1=(CExpression*)(tmp2->m_pElementList->pElementObject->Expression1);
								for (int kk=0;kk<arg->m_NumElements;kk++)
									factor1->InsertElement(arg->m_pElementList+kk,factor1->m_NumElements);
								factor1->InsertEmptyElement(factor1->m_NumElements,2,'-');
								factor1->InsertEmptyElement(factor1->m_NumElements,4,0);
								CExpression *nom=(CExpression*)((factor1->m_pElementList+factor1->m_NumElements-1)->pElementObject->Expression1);
								CExpression *denom=(CExpression*)((factor1->m_pElementList+factor1->m_NumElements-1)->pElementObject->Expression2);
								nom->InsertEmptyElement(0,2,'-');
								nom->InsertEmptyElement(1,5,'(');
								CExpression *arg2=(CExpression*)((nom->m_pElementList+1)->pElementObject->Expression1);
								for (int kk=l;kk<l+l2-arg->m_NumElements;kk++)
									arg2->InsertElement(tmp->m_pElementList+kk,arg2->m_NumElements);
								if ((arg2->m_pElementList->Type==0) || 
									((arg2->m_NumElements==1) && (arg2->m_pElementList->Type==2)))
									arg2->InsertEmptyElement(arg2->m_NumElements,1,'1');
								nom->InsertEmptyElement(2,2,'+');
								nom->InsertEmptyElement(3,8,1);
								CExpression *arg3=(CExpression*)((nom->m_pElementList+3)->pElementObject->Expression1);
								arg3->InsertElement(nom->m_pElementList+1,0);
								arg3->InsertElement(nom->m_pElementList+1,1);
								arg3->InsertEmptyElement(2,2,'-');
								arg3->InsertEmptyElement(3,1,'4');
								arg3->InsertEmptyElement(4,5,'(');
								CExpression *arg4=(CExpression*)((arg3->m_pElementList+4)->pElementObject->Expression1);
								for (int kk=0;kk<l-1;kk++)
									arg4->InsertElement(tmp->m_pElementList+kk,arg4->m_NumElements);
								if ((arg4->m_pElementList->Type==0) ||
									((arg4->m_NumElements==1) && (arg4->m_pElementList->Type==2)))
									arg4->InsertEmptyElement(arg4->m_NumElements,1,'1');

								arg3->InsertEmptyElement(5,5,'(');
								CExpression *arg5=(CExpression*)((arg3->m_pElementList+5)->pElementObject->Expression1);
								for (int kk=l+l2;kk<l+l2+l3;kk++)
									arg5->InsertElement(tmp->m_pElementList+kk,arg5->m_NumElements);
								if ((arg5->m_pElementList->Type==0) ||
									((arg5->m_NumElements==1) && (arg5->m_pElementList->Type==2)))
									arg5->InsertEmptyElement(arg5->m_NumElements,1,'1');
								denom->InsertEmptyElement(0,5,'(');
								CExpression *arg6=(CExpression*)((denom->m_pElementList->pElementObject->Expression1));
								arg6->CopyExpression(arg4,0);
								denom->InsertEmptyElement(0,1,'2');
								
								int cntr=0;
								while ((arg3->Compute(0,arg3->m_NumElements-1,0)) && (cntr<50)) cntr++;

								int llv=arg3->FindLowestOperatorLevel((char)0xD7);
								if ((llv>=MulLevel))
								{
									tPureFactors PF;
									PF.N1=PF.N2=1.0;
									PF.is_frac1=0;
									PF.prec1=0;
									int rslt=arg3->StrikeoutCommonFactors(0,arg3->m_NumElements-1,1,NULL,0,0,1,&PF);
									arg3->StrikeoutRemove(0,arg3->m_NumElements-1);
									if (/*(!rslt) || */(PF.N1*PF.N2>-(1e-100)))
									{
										tmp2->InsertEmptyElement(1,5,'(');
										CExpression *factor2=(CExpression*)((tmp2->m_pElementList+1)->pElementObject->Expression1);
										factor2->CopyExpression(factor1,0);
										(nom->m_pElementList+2)->pElementObject->Data1[0]='-';

										int cntr=0;
										while ((factor1->Compute(0,factor1->m_NumElements-1,0)) && (cntr<50)) cntr++;
										cntr=0;
										while ((factor2->Compute(0,factor2->m_NumElements-1,0)) && (cntr<50)) cntr++;
										if (!force_factorization)
										{
											//we don't do it if square root or imaginary unit is found
											char tmpstr[3];
											tmpstr[0]=ImaginaryUnit;
											tmpstr[1]=0;
											for (int iii=0;iii<factor1->m_NumElements;iii++)
												if (((factor1->m_pElementList+iii)->Type==8) ||
													(((factor1->m_pElementList+iii)->Type==1) && 
													 (strcmp((factor1->m_pElementList+iii)->pElementObject->Data1,tmpstr)==0) &&
													 (((factor1->m_pElementList+iii)->pElementObject->m_VMods)==0) &&
													 (((factor1->m_pElementList+iii)->pElementObject->Data2[0]&0xE0)!=0x60)))
												{
													tmp2->Delete();
													break;
												}
										}
									}
									else
										tmp2->Delete();
								}
								else
									tmp2->Delete();
							}
						}
						else
						{
							//this is factor for x^0
							tmp2->InsertEmptyElement(0,5,'(');
							CExpression *factor1=(CExpression*)(tmp2->m_pElementList->pElementObject->Expression1);
							for (int kk=0;kk<arg->m_NumElements;kk++)
								factor1->InsertElement(arg->m_pElementList+kk,factor1->m_NumElements);
							factor1->InsertEmptyElement(factor1->m_NumElements,2,'-');
							factor1->InsertEmptyElement(factor1->m_NumElements,8,1);
							CExpression *arg1=(CExpression*)((factor1->m_pElementList+factor1->m_NumElements-1)->pElementObject->Expression1);
							arg1->InsertEmptyElement(0,4,0);
							CExpression *nom=(CExpression*)(arg1->m_pElementList->pElementObject->Expression1);
							CExpression *denom=(CExpression*)(arg1->m_pElementList->pElementObject->Expression2);
							for (int kk=0;kk<l-1;kk++)
								denom->InsertElement(tmp->m_pElementList+kk,denom->m_NumElements);
							if ((denom->m_pElementList->Type==0) ||
								((denom->m_NumElements==1) && (denom->m_pElementList->Type==2)))
								denom->InsertEmptyElement(denom->m_NumElements,1,'1');
							for (int kk=l;kk<l+l2;kk++)
								nom->InsertElement(tmp->m_pElementList+kk,nom->m_NumElements);
							if ((nom->m_pElementList->Type==0) ||
								((nom->m_NumElements==1) && (nom->m_pElementList->Type==2)))
								nom->InsertEmptyElement(nom->m_NumElements,1,'1');							
							arg1->InsertEmptyElement(0,2,'-');

							int cntr=0;
							while ((arg1->Compute(0,arg1->m_NumElements-1,0)) && (cntr<50)) cntr++;

							int llv=arg1->FindLowestOperatorLevel((char)0xD7);
							if (llv>=MulLevel)
							{
								tPureFactors PF;
								PF.N1=PF.N2=1.0;
								PF.is_frac1=0;
								PF.prec1=0;
								int rslt=arg1->StrikeoutCommonFactors(0,arg1->m_NumElements-1,1,NULL,0,0,1,&PF);
								arg1->StrikeoutRemove(0,arg1->m_NumElements-1);
								if (/*(!rslt) ||*/ (PF.N1*PF.N2>-(1e-100)))
								{
									tmp2->InsertEmptyElement(1,5,'(');
									CExpression *factor2=(CExpression*)((tmp2->m_pElementList+1)->pElementObject->Expression1);
									factor2->CopyExpression(factor1,0);
									(factor2->m_pElementList+factor2->m_NumElements-2)->pElementObject->Data1[0]='+';

									int cntr=0;
									while ((factor1->Compute(0,factor1->m_NumElements-1,0)) && (cntr<50)) cntr++;
									cntr=0;
									while ((factor2->Compute(0,factor2->m_NumElements-1,0)) && (cntr<50)) cntr++;
									if (!force_factorization)
									{
										//we don't do it if square root or imaginary unit is found
										char tmpstr[3];
										tmpstr[0]=ImaginaryUnit;
										tmpstr[1]=0;
										for (int iii=0;iii<factor1->m_NumElements;iii++)
											if (((factor1->m_pElementList+iii)->Type==8) ||
												(((factor1->m_pElementList+iii)->Type==1) && 
													(strcmp((factor1->m_pElementList+iii)->pElementObject->Data1,tmpstr)==0) &&
													(((factor1->m_pElementList+iii)->pElementObject->m_VMods)==0) &&
													(((factor1->m_pElementList+iii)->pElementObject->Data2[0]&0xE0)!=0x60)))
											{
												tmp2->Delete();
												break;
											}
									}
								}
								else
									tmp2->Delete();
							}
							else
								tmp2->Delete();
						}
					}
				}
				if ((fabs(aPF.N1/aPF.N2-1.00)>1e-100) && (tmp2->m_pElementList->Type))
				{
					tmp2->InsertEmptyElement(0,5,'(');
					for (int jj=1;jj<tmp2->m_NumElements;jj++)
						((CExpression*)tmp2->m_pElementList->pElementObject->Expression1)->InsertElement(tmp2->m_pElementList+jj,jj-1);
					while (tmp2->m_NumElements>1)
						tmp2->DeleteElement(1);
					tmp2->GenerateASCIIFraction(0,aPF.N1,aPF.N2,max(aPF.prec1,aPF.prec2),1);
					int cntr=0;
					while ((tmp2->Compute(0,tmp2->m_NumElements-1,1)) && (cntr<50)) cntr++;
				}
				/*if ((a_factor_negative) && (tmp2->m_pElementList->Type))
				{
					tmp2->InsertEmptyElement(0,2,'-');
				}*/
			}
		}
		if (tmp2->m_pElementList->Type!=0)
		{
			int t=m_ParenthesesFlags;
			CopyExpression(tmp2,0);
			m_ParenthesesFlags=t;
			retval=1;
		}
		delete tmp2;
	}
	delete tmp;

	if (retval) 
		goto factorizeexpression_finish;
	else
		goto factorizeexpression_exit;


factorizeexpression_finish:
	delete original;
	if (FindLowestOperatorLevel((char)0xD7)==MulLevel)
		FactorizeExpression();
	{
		int cnt=0;
		while ((Compute(0,m_NumElements-1,1)) && (cnt<50)) cnt++;
	}
	this->m_IsComputed|=0x00800000;
	xreturn (1);

factorizeexpression_exit:
	this->CopyExpression(original,0);
	this->m_IsComputed|=0x00800000;
	delete original;
	xreturn (0);
}

int CExpression::ComputeRoot(int Position, char element_type, int ComputationType)
{
	int retval=0;

	tElementStruct *ts=m_pElementList+Position;
	if (ts->Type!=8) return 0;

	CExpression *base=(CExpression*)(ts->pElementObject->Expression2);
	CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
	if (arg==NULL) return 0;
	if (ComputationType==2) return 0; //fraction summation, root multiplication
	if (ComputationType==3) return 0; //simple calculations only

	//factorize the root argument and prepare it for calculation
	int cntr=0;
	if (ComputationType!=10)
	{
		cntr=0;
		while ((arg->Compute(0,arg->m_NumElements-1,1)) && (cntr<50)) cntr++;
		if (cntr) retval=1;
		if (arg->FactorizeExpression()) retval=1;
	}
	else
	{
		cntr=0;
		while ((arg->Compute(0,arg->m_NumElements-1,ComputationType)) && (cntr<50)) cntr++;
		if (cntr) retval=1;
	}


	//also prepare the base of root (if exists)
	if ((base) && ((base->m_NumElements>1) || (base->m_pElementList->Type>1)))
	{
		cntr=0;
		while ((base->Compute(0,base->m_NumElements-1,0)) && (cntr<50)) cntr++;
	}
	

	//if the argument is in factorized form, then we can take roots (factor by factor)
	int Level1=arg->FindLowestOperatorLevel((char)0xD7);
	if (Level1==MulLevel)
	{
		int pos=0;
		while (1)
		{
			char et1;
			char p1;
			int l1;
			l1=arg->GetElementLen(pos,arg->m_NumElements-1,Level1,&et1,&p1);
			if (l1==0) break;

			//jump over + or - sign if it exists
			if  ((pos==0) && ((arg->m_pElementList+pos+p1)->Type==2) && 
				 (GetOperatorLevel((arg->m_pElementList+pos+p1)->pElementObject->Data1[0])==PlusLevel))
				{pos++;l1--;} 

			if (l1-p1==1)
			{
				if ((arg->m_pElementList+pos+p1)->Type==4)
				{
					//calculates root of fraction (by calculating root of nominator and denominator)
					CExpression *nom=(CExpression*)((arg->m_pElementList+pos+p1)->pElementObject->Expression1);
					CExpression *denom=(CExpression*)((arg->m_pElementList+pos+p1)->pElementObject->Expression2);

					CExpression *tmp=new CExpression(NULL,NULL,100);
					CExpression *tmp2=new CExpression(NULL,NULL,100);

					tmp->InsertEmptyElement(0,4,0);
					CExpression *n=(CExpression*)(tmp->m_pElementList->pElementObject->Expression1);
					CExpression *d=(CExpression*)(tmp->m_pElementList->pElementObject->Expression2);
					if (et1=='/')
					{
						CExpression *ttt;
						ttt=n;n=d;d=ttt;
					}
					if (base==NULL)
					{
						n->InsertEmptyElement(0,8,1);
						d->InsertEmptyElement(0,8,1);
					}
					else
					{
						n->InsertEmptyElement(0,8,0);
						((CExpression*)(n->m_pElementList->pElementObject->Expression2))->CopyExpression(base,0);
						d->InsertEmptyElement(0,8,0);
						((CExpression*)(n->m_pElementList->pElementObject->Expression2))->CopyExpression(base,0);
					}
					((CExpression*)(n->m_pElementList->pElementObject->Expression1))->CopyExpression(nom,0);
					((CExpression*)(d->m_pElementList->pElementObject->Expression1))->CopyExpression(denom,0);
					
					//make the calculation
					int cnt=0;
					while ((n->Compute(0,n->m_NumElements-1,(ComputationType==10)?10:1)) && (cnt<50)) cnt++;
					cnt=0;
					while ((d->Compute(0,d->m_NumElements-1,(ComputationType==10)?10:1)) && (cnt<50)) cnt++;

					//evaluate the result - separate factors that are now outside of root
					tmp2->InsertEmptyElement(0,4,0);
					nom=(CExpression*)((tmp2->m_pElementList)->pElementObject->Expression1);
					denom=(CExpression*)((tmp2->m_pElementList)->pElementObject->Expression2);

					int found_anything=0;
					for (int iii=0;iii<2;iii++)
					{
						int poss=0;
						while (1)
						{
							char et,p;
							int l=n->GetElementLen(poss,n->m_NumElements-1,MulLevel,&et,&p);
							if (l==0) break;

							if ((l-p==1) && ((n->m_pElementList+poss+p)->Type==8))
							{
								CExpression *a=(CExpression*)((n->m_pElementList+poss+p)->pElementObject->Expression1);
								nom->InsertSequence(et,nom->m_NumElements,a,0,a->m_NumElements-1);
								n->RemoveSequence(MulLevel,poss+p,poss+l-1);
								l=0;
							}
							else if (n->m_NumElements)
							{
								if (n->m_pElementList->Type!=0)
								{
									if ((n->m_pElementList->Type!=1) ||
										(*(unsigned short*)(n->m_pElementList->pElementObject->Data1)!=0x0031)) //this is "1" string
										//(strcmp(n->m_pElementList->pElementObject->Data1,"1")!=0))
										found_anything=1;
								}
							}
							poss+=l;
							if (poss>n->m_NumElements-1) break;
						}
						CExpression *tt;
						tt=n;n=d;d=tt;
						tt=nom;nom=denom;denom=tt;
					}
					if (found_anything)
					{
						if (n->m_pElementList->Type==0) n->InsertEmptyElement(0,1,'1');
						if (d->m_pElementList->Type==0) d->InsertEmptyElement(0,1,'1');
						if (nom->m_pElementList->Type==0) nom->InsertEmptyElement(0,1,'1');
						if (denom->m_pElementList->Type==0) denom->InsertEmptyElement(0,1,'1');
						arg->DeleteElement(pos+p1);
						arg->InsertElement(tmp2->m_pElementList,pos+p1);
						if (GetOperatorLevel(element_type)!=MulLevel)
							Position=InsertSequence((char)0xD7,Position,tmp,0,tmp->m_NumElements-1);
						else
							Position=InsertSequence(element_type,Position,tmp,0,tmp->m_NumElements-1);
						retval=1;
					}
					delete tmp;
					delete tmp2;

				}
				else if ((arg->m_pElementList+pos+p1)->Type==3)
				{
					int make_reduction=0;
					CExpression *e=(CExpression*)((arg->m_pElementList+pos+p1)->pElementObject->Expression2);
					CExpression *a=(CExpression*)((arg->m_pElementList+pos+p1)->pElementObject->Expression1);
					int Level2=e->FindLowestOperatorLevel((char)0xD7);
					if (Level2==MulLevel)
					{			
						CExpression *tmp=new CExpression(NULL,NULL,100);
						if (base)
						{
							tmp->CopyExpression(base,0);
						}
						else
						{
							tmp->InsertEmptyElement(0,1,'2');
						}

						//test if the root can be taken (examine if the exponent of this
						//root-argument factor is divisible by root base )

						tPureFactors PF;
						PF.N1=PF.N2=PF.N3=PF.N4=1.0;
						PF.is_frac1=PF.is_frac2=0;
						PF.prec1=PF.prec2=0;
						int rval=tmp->StrikeoutCommonFactors(0,tmp->m_NumElements-1,1,e,0,e->m_NumElements-1,1,&PF);
						double xx=PF.N3*PF.N2/PF.N1/PF.N4;
						if ((rval) && (fabs(xx-(long long)xx)<1e-100))
						{
							//yes! We managed to take the root, print out the result
							PF.N3=PF.N3*PF.N2;
							PF.N3=PF.N3/PF.N1;
							ReduceTwoNumbers(&PF.N3,&PF.N4);
							e->StrikeoutRemove(0,e->m_NumElements-1,2);
							int pp=0;
							if ((e->m_pElementList->Type==2) &&
								(GetOperatorLevel(e->m_pElementList->pElementObject->Data1[0])==PlusLevel))
								pp=1;
							e->GenerateASCIIFraction(pp,PF.N3,PF.N4,PF.prec2,PF.is_frac2);
							if (GetOperatorLevel(element_type)!=MulLevel)
								Position=InsertSequence(et1,Position,arg,pos+p1,pos+l1-1);
							else
								Position=InsertSequence((et1==element_type)?(char)0xD7:'/',Position,arg,pos+p1,pos+l1-1);
							if (arg->RemoveSequence(Level1,pos+p1,pos+l1-1)) l1=0;							
							retval=1;
						}
						else 
						{
							//we didn't manage to calculate the root - however check two more special cases
							//this is only possible if the root base is pure number (integer that is greater than 1)
							int ok=1;
							for (int kk=0;kk<tmp->m_NumElements;kk++)
								if (((tmp->m_pElementList->IsSelected)&0x40)==0) {ok=0;break;}

							ReduceTwoNumbers(&PF.N1,&PF.N2);
							
							if ((ok) && (PF.N1>=1.0) && (fabs(PF.N1-(long long)PF.N1)<1e-100) && (fabs(PF.N2-1.0)<1e-100))
							{
								//first if exponent has pure number factor that is non-integer
								//then we can take root by dividing this number with root base
								//example: sqrt(x^2.3)=x^1.15    example2: sqrt(x^(3/7))=x^(3/14)
								{
									int pp=0;
									if ((fabs(PF.N3-(long long)PF.N3)>1e-100) || 
										(fabs(PF.N4-1.0)>1e-100))
									{
										e->StrikeoutRemove(0,e->m_NumElements,2);

										PF.N4*=PF.N1;
										if ((e->m_pElementList->Type==2) &&
											(GetOperatorLevel(e->m_pElementList->pElementObject->Data1[0])==PlusLevel))
											pp=1;
										e->GenerateASCIIFraction(pp,PF.N3,PF.N4,PF.prec2,PF.is_frac2);

										if (GetOperatorLevel(element_type)!=MulLevel)
											Position=InsertSequence(et1,Position,arg,pos+p1,pos+l1-1);
										else
											Position=InsertSequence((et1==element_type)?(char)0xD7:'/',Position,arg,pos+p1,pos+l1-1);
										if (arg->RemoveSequence(Level1,pos+p1,pos+l1-1)) l1=0;
										retval=1;
									}
									else if (fabs(PF.N3-(long long)PF.N3)<1e-100)
									{
										//second if the exponent has pure number factor that is integer
										//maybe we can take partial root (  example sqrt(x^3)=x*sqrt(x)  )
										int minus=1;
										long long n=(long long)(PF.N3+((PF.N3>=0)?0.01:-0.01));
										long long p=(long long)(PF.N1+0.01);
										if (n<0) {n=-n;minus=-1;}
										if (n>p)
										{
											e->StrikeoutRemove(0,e->m_NumElements-1,2);
											if ((e->m_pElementList->Type==2) &&
												(GetOperatorLevel(e->m_pElementList->pElementObject->Data1[0])==PlusLevel))
												pp=1;
											e->GenerateASCIINumber((double)(n/p)*minus,(n/p)*minus,1,0,pp);
											if (GetOperatorLevel(element_type)!=MulLevel)
												Position=InsertSequence(et1,Position,arg,pos+p1,pos+l1-1);
											else
												Position=InsertSequence((et1==element_type)?(char)0xD7:'/',Position,arg,pos+p1,pos+l1-1);
											e->Delete();
											e->GenerateASCIINumber((double)(n%p)*minus,(n%p)*minus,1,0,pp);
											l1--;
											retval=1;		
										}
										else
											e->StrikeoutRemove(0,e->m_NumElements-1);
									}
									else
									{
										e->StrikeoutRemove(0,e->m_NumElements-1);
									}
								}
							}
							else
							{
								e->StrikeoutRemove(0,e->m_NumElements-1);
							}
						}
						delete tmp;
					}
				}
			}
			pos+=l1;
			if (pos>arg->m_NumElements-1) break;
			if (arg->m_pElementList->Type==0) break;
		}

		if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==0))
			RemoveSequence(MulLevel,Position,Position);
		if (retval) return retval;
	}



	double N1;
	int prec1;
	if (base==NULL)
	{
		N1=2.0;
		prec1=0;
	}
	if ((base==NULL) || (base->IsPureNumber(0,base->m_NumElements,&N1,&prec1)))
		if ((fabs(N1-(long long)N1)<1e-100) && (N1>0.999))
		{
			N1=(double)((long long)(N1+((N1>=0)?0.01:-0.01)));



			{
				//if the argument of root equlas to 'one' then delete it
				//if it equals to -1, then result is the imaginary unit (when used)
				double N;
				int prec;
				if (arg->IsPureNumber(0,arg->m_NumElements,&N,&prec))
				{
					if (fabs(N-1.0)<1e-100)
					{
						InsertEmptyElement(Position,1,'1');
						DeleteElement(Position+1);
						return 1;
					}
					if ((fabs(N+1.0)<1e-100) && (ImaginaryUnit) && (fabs(N1-2.0)<1e-100))
					{
						InsertEmptyElement(Position,1,ImaginaryUnit);
						DeleteElement(Position+1);
						return 1;
					}
				}

				//if square root of imaginary unit
				if (ImaginaryUnit)
				if (fabs(N1-2.0)<1e-100)
				{
					int p=0;
					int ch='+';
					if ((arg->m_pElementList->Type==2) && (arg->m_pElementList->pElementObject->Data1[0]=='-'))
						{p=1;ch='-';}
					if ((arg->m_pElementList->Type==2) && (arg->m_pElementList->pElementObject->Data1[0]=='+'))
						p=1;
					if ((arg->m_NumElements==1+p) && ((arg->m_pElementList+p)->Type==1) &&
						((arg->m_pElementList+p)->pElementObject->Data1[0]==ImaginaryUnit) &&
						((arg->m_pElementList+p)->pElementObject->Data1[1]==0) &&
						(((arg->m_pElementList+p)->pElementObject->m_VMods)==0) &&
						(((arg->m_pElementList+p)->pElementObject->Data2[0]&0xE0)!=0x60))
					{
						InsertEmptyElement(Position,4,0);
						CExpression *n=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
						CExpression *d=(CExpression*)((m_pElementList+Position)->pElementObject->Expression2);
						d->InsertEmptyElement(0,8,1);
						CExpression *a=(CExpression*)(d->m_pElementList->pElementObject->Expression1);
						a->InsertEmptyElement(0,1,'2');
						n->InsertEmptyElement(0,1,ImaginaryUnit);
						n->InsertEmptyElement(1,2,ch);
						n->InsertEmptyElement(2,1,'1');
						DeleteElement(Position+1);
						return 1;
					}
				}
				
			}

			tPureFactors PF;
			PF.N1=PF.N2=1.0;
			PF.prec1=0;
			PF.is_frac1=0;
			int rv=arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
			
			double N2=PF.N1,N3=PF.N2;
			int prec2=PF.prec1;
			char is_frac=PF.is_frac1;

			int retval=0;
			{
				if ((fabs(fabs(N2)-1.0)>1e-100) || (fabs(N3-1.0)>1e-100) || (rv==1))
				{
					if (N3<0) {N2=-N2;N3=-N3;}
					if ((ComputationType==10) && (fabs(N3)>1e-100)) {N2=N2/N3;N3=1.0;}
					double nom=pow(fabs(N2),1.0/N1);
					double denom=pow(N3,1.0/N1);

					if ((prec2>0) && (denom>0))
					{
						//real numers: check if root can be taken; if not, calculate nom/denom;
						double x2=nom*pow(10.0,prec2);
						double x3=denom*pow(10.0,prec2);
						if ((fabs(x3-(long long)x3)>1e-100) || (fabs(x2-(long long)x2)>1e-100))
						{
							nom=nom/denom;
							denom=1.0;
						}
					}

                    long long nom_leftover=1;
					long long denom_leftover=1;

					if (ComputationType!=10)
					{
						long long maxlong=0x7FFFFFFFFFFFFFFF;
						if (fabs(denom-(long long)denom)>1e-100)
						if ((prec2==0) && (fabs(N3)<maxlong))
						{
							//can we partialy take root from 'denom'
							long long i_denom=1;

							long long n=(long long)(fabs(N3)+0.01);
							if (n>0)
							for (int iii=2;iii<11;iii++)
							{
								long long c=(long long)pow((double)iii,(int)(N1+((N1>=0)?0.01:-0.01)));
								if (n%c==0)
								{
									i_denom*=iii;
									n/=c;
									iii--;
								}
							}
							if (i_denom!=1) {denom_leftover=n;denom=(double)i_denom;}
						}

						if (fabs(nom-(long long)nom)>1e-100)
						if ((prec2==0) && (fabs(N2)<maxlong))
						{
							//can we partialy take root from 'nom'
							long long i_nom=1;

							long long n=(long long)(fabs(N2)+0.01);
							if (n>0)
							for (int iii=2;iii<40;iii++)
							{
								long long c=(long long)pow((double)iii,(int)(N1+((N1>=0)?0.01:-0.01)));
								if (c==0) break;
								if (n%c==0)
								{
									i_nom*=iii;
									n/=c;
									iii--;
								}
							}
							if (i_nom!=1.0) {nom_leftover=n;nom=(double)i_nom;}
						}
					}

					
					if ((nom_leftover!=1) || (denom_leftover!=1))
					{
						if ((nom_leftover==1) && (fabs(N2-1.0)>1e-100))
							{nom_leftover=(long long)(fabs(N2)+0.01);nom=1.0;}
						if ((denom_leftover==1) && (fabs(N3-1.0)>1e-100)) 
							{denom_leftover=(long long)(N3+0.01);denom=1.0;}

						int pp=0;
						if ((arg->m_pElementList->Type==2) && 
							(GetOperatorLevel(arg->m_pElementList->pElementObject->Data1[0])==PlusLevel))
							pp=1;

						if (denom_leftover!=1)
						{
							arg->InsertEmptyElement(pp,4,0);
							CExpression *t=(CExpression*)((arg->m_pElementList+pp)->pElementObject->Expression1);
							CExpression *v=(CExpression*)((arg->m_pElementList+pp)->pElementObject->Expression2);
							t->GenerateASCIINumber((double)nom_leftover,nom_leftover,1,0,0);
							v->GenerateASCIINumber((double)denom_leftover,denom_leftover,1,0,0);
						}
						else
							arg->GenerateASCIINumber((double)nom_leftover,nom_leftover,1,0,pp);
					}



					
					if ((((fabs(nom*1000-(long long)(nom*1000))<1e-100) && 
						(fabs(denom*1000-(long long)(denom*1000))<1e-100)) || (ComputationType==10)) &&
						((fabs(denom)>1e-100) && (fabs(nom/denom-1.0)>1e-100)))
					{
						arg->StrikeoutRemove(0,arg->m_NumElements-1,2);
						retval=1;

						if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==0))
						{
							Position-=RemoveSequence(MulLevel,Position,Position);
						}
						else
						{
							if (GetOperatorLevel(element_type)==MulLevel)
							if ((Position) &&
								((m_pElementList+Position-1)->Type==2) &&
								((m_pElementList+Position-1)->pElementObject->Data1[0]==element_type))
								Position--;
						}

						if ((fabs(denom-1.0)<1e-100) || (is_frac==0))
						{
							if (is_frac==0) nom=nom/denom;
							if (N2<0) 
							{
								if ((((int)N1)%2)==1)
								{
									InsertEmptyElement(Position,5,0);
									CExpression *t=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
									t->InsertEmptyElement(0,2,'-');
									t->InsertEmptyElement(1,1,'1');
								}
								else
								{
									if (ImaginaryUnit)
										InsertEmptyElement(Position,1,ImaginaryUnit);
									else
									{
										InsertEmptyElement(Position,8,1);
										CExpression *t=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
										t->InsertEmptyElement(0,2,'-');
										t->InsertEmptyElement(1,1,'1');
									}
								}
								if (element_type=='/') InsertEmptyElement(Position,2,'/');
							}
							GenerateASCIINumber(nom,(long long)nom,(fabs(nom-(long long)nom)<1e-100)?1:0,prec2+2,Position);
							if (element_type=='/') InsertEmptyElement(Position,2,'/');
						}
						else
						{
							if (N2<0) 
							{
								if ((((int)N1)%2)==1)
								{
									InsertEmptyElement(Position,5,0);
									CExpression *t=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
									t->InsertEmptyElement(0,2,'-');
									t->InsertEmptyElement(1,1,'1');
								}
								else
								{
									if (ImaginaryUnit) 
										InsertEmptyElement(Position,1,ImaginaryUnit);
									else
									{
										InsertEmptyElement(Position,8,1);
										CExpression *t=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
										t->InsertEmptyElement(0,2,'-');	
										t->InsertEmptyElement(1,1,'1');
									}
								}
								if (element_type=='/') InsertEmptyElement(Position,2,'/');
							}
							InsertEmptyElement(Position,4,0);
							CExpression *n=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
							CExpression *d=(CExpression*)((m_pElementList+Position)->pElementObject->Expression2);
							n->GenerateASCIINumber(nom,(long long)nom,(fabs(nom-(long long)nom)<1e-100)?1:0,prec2+2,0);
							d->GenerateASCIINumber(denom,(long long)denom,(fabs(denom-(long long)denom)<1e-100)?1:0,prec2+2,0);
							if (element_type=='/') InsertEmptyElement(Position,2,'/');
						}
					}
					else
						arg->StrikeoutRemove(0,arg->m_NumElements-1);
				}
			}

			//delete res;
			if (retval) return retval;

		}

	if (ComputationType==1)
	{
		//root of fraction
		if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==4))
		{
			InsertEmptyElement(Position+1,4,0);
			CExpression *n=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
			CExpression *d=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression2);
			CExpression *cn=(CExpression*)((arg->m_pElementList)->pElementObject->Expression1);
			CExpression *cd=(CExpression*)((arg->m_pElementList)->pElementObject->Expression2);
			if (base==NULL)
			{
				n->InsertEmptyElement(0,8,1);
				n=(CExpression*)(n->m_pElementList->pElementObject->Expression1);
				d->InsertEmptyElement(0,8,1);
				d=(CExpression*)(d->m_pElementList->pElementObject->Expression1);
			}
			else
			{
				n->InsertEmptyElement(0,8,0);
				CExpression *t=(CExpression*)(n->m_pElementList->pElementObject->Expression2);
				t->CopyExpression(base,0);
				n=(CExpression*)(n->m_pElementList->pElementObject->Expression1);
				d->InsertEmptyElement(0,8,0);
				t=(CExpression*)(d->m_pElementList->pElementObject->Expression2);
				t->CopyExpression(base,0);
				d=(CExpression*)(d->m_pElementList->pElementObject->Expression1);
			}
			n->CopyExpression(cn,0);
			d->CopyExpression(cd,0);
			DeleteElement(Position);
			return 1;
		}


		int Level=arg->FindLowestOperatorLevel();
		if (Level==MulLevel)
		{
			int tt=0;
			int pos=0;
			while (1)
			{
				char et,p;
				int l=arg->GetElementLen(pos,arg->m_NumElements-1,Level,&et,&p);
				if (l==0) break;

				if (((element_type=='/') && (et!='/')) ||
					((element_type!='/') && (et=='/')))
					{InsertEmptyElement(Position+1+tt,2,'/');tt++;}
				InsertEmptyElement(Position+1+tt,8,(base)?0:1);
				CExpression *a=(CExpression*)((m_pElementList+Position+1+tt)->pElementObject->Expression1);
				if (base)
				{
					CExpression *b=(CExpression*)((m_pElementList+Position+1+tt)->pElementObject->Expression2);
					b->CopyExpression(base,0);
				}

				a->InsertSequence((char)0xD7,0,arg,pos+p,pos+l-1);

				tt++;

				pos+=l;
				if (pos>arg->m_NumElements-1) break;
			}
			RemoveSequence(MulLevel,Position,Position);
			return 1;
		}
	}
				
	return 0;
}

int CExpression::ComputeFraction(int Position, char element_type, int ComputationType)
{
	int retval=0;
	PROFILE_TIME(&PROFILER.ComputeFraction);
	tElementStruct *ts=m_pElementList+Position;

	//reduce fraction
	CExpression *nom=(CExpression*)(ts->pElementObject->Expression1);
	CExpression *denom=(CExpression*)(ts->pElementObject->Expression2);

	int Level1=nom->FindLowestOperatorLevel((char)0xD7);
	int Level2=denom->FindLowestOperatorLevel((char)0xD7);
	
	if (ComputationType!=10) //if not pure number only calculation (for function plotting)
	{
		//check if this is derivation: d(f(x))/d(x) 
		if ((nom->m_NumElements==1) && (denom->m_NumElements==1) &&
			(nom->m_pElementList->Type==6) &&
			(denom->m_pElementList->Type==6) &&
			(*(unsigned short*)(nom->m_pElementList->pElementObject->Data1)==0x0064) && //this is "d" string
			((nom->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60) &&
			(*(unsigned short*)(denom->m_pElementList->pElementObject->Data1)==0x0064) && //this is "d" string
			((denom->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60))
		{
			CExpression *arg=(CExpression*)(nom->m_pElementList->pElementObject->Expression1);
			CExpression *var=(CExpression*)(denom->m_pElementList->pElementObject->Expression1);
			if ((arg) && (var))
			if (arg->ContainsVariable(0,arg->m_NumElements-1,var,0,var->m_NumElements))
			{
				if (arg->Derivate(var))
				{
					InsertEmptyElement(Position,5,'(');
					CExpression *a=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
					a->CopyExpression(arg,0); a->m_ParenthesesFlags=0x02;
					DeleteElement(Position+1);
					xreturn (1);
				}
			}
		}


		//quick check if there are common factors betwen nom and denom
		//the simple method: compare every factor with every factor
		if ((Level1==MulLevel) && (Level2==MulLevel))
		{
			int pos=0;
			while (1)
			{
				char et,p;
				int l=nom->GetElementLen(pos,nom->m_NumElements-1,Level1,&et,&p);
				if (l==0) break;

				tElementStruct *ts1=nom->m_pElementList+pos+p;
				int pos2=0;
				while (1)
				{
					char et2,p2;
					int l2=denom->GetElementLen(pos2,denom->m_NumElements-1,Level2,&et2,&p2);
					if (l2==0) break;

					if (et==et2) 
					if (nom->CompareExpressions(pos+p,pos+l-1,denom,pos2+p2,pos2+l2-1))
					{
						tElementStruct *ts2=denom->m_pElementList+pos2+p2;
						if ((l-p==1) && (l2-p2==1) && 
							(ts1->Type==1) && (*(unsigned short*)(ts1->pElementObject->Data1)==0x0031) && //this is "1" string (strcmp((nom->m_pElementList+pos+p)->pElementObject->Data1,"1")==0) &&
							(ts2->Type==1) && (*(unsigned short*)(ts2->pElementObject->Data1)==0x0031)) //this is "1" string (strcmp((denom->m_pElementList+pos2+p2)->pElementObject->Data1,"1")==0))
						{
							//do nothing - we dont reduce two pure ones:   1/1
						}
						else
						{
							nom->RemoveSequence(MulLevel,pos,pos+l-1);
							denom->RemoveSequence(MulLevel,pos2,pos2+l2-1);
							pos=0;l=0;retval=-1;
							break;
						}
					}
					pos2+=l2;
					if (pos2>denom->m_NumElements-1) break;
				}
				pos+=l;
				if (pos>nom->m_NumElements-1) break;
			}
		}
	}



	if (ComputationType!=2)  //not fraction summation (for speed)
	{
		if ((ComputationType!=10) && (denom->FactorizeExpression())) //ComputationType==10 -> Pure Numbers calculation
			retval=-1;
		else if ((Level2==PlusLevel) || (ComputationType==1) || (ComputationType==10)) //without this filter it works extremely slow
		{
			int cntr=0;
			while ((denom->Compute(0,denom->m_NumElements-1,ComputationType)) && (cntr<50)) cntr++;
			if (cntr) 
			{	
				retval=-1;
				if (ComputationType!=10) denom->FactorizeExpression();
			}
		} 
		else
		{
			int cntr=0;
			while ((denom->Compute(0,denom->m_NumElements-1,3)) && (cntr<50)) cntr++;
			if (cntr) retval=-1;
		}

		if ((ComputationType!=10) && (nom->FactorizeExpression()))
			retval=-1;
		else if ((Level1==PlusLevel) || (ComputationType==1) || (ComputationType==10))
		{
			int cntr=0;
			while ((nom->Compute(0,nom->m_NumElements-1,ComputationType)) && (cntr<50)) cntr++;
			if (cntr) 
			{
				retval=-1;
				if (ComputationType!=10) nom->FactorizeExpression();
			}
		}
		else
		{
			int cntr=0;
			while ((nom->Compute(0,nom->m_NumElements-1,3)) && (cntr<50)) cntr++;
			if (cntr) retval=-1;
		}
	}
	else
	{
		if ((ImaginaryUnit) && (Level2==PlusLevel) && (this->m_pPaternalExpression==NULL))
		{
			//if there is a Imaginary Unit in the denominator - try to move it to numerator
			char tmpstr[3];
			tmpstr[0]=ImaginaryUnit;
			tmpstr[1]=0;
			if (denom->ContainsVariable(0,denom->m_NumElements-1,NULL,0,0,tmpstr,0))
			{
				int order=denom->Polynomize(NULL,tmpstr,0);
				if (order==1)
				{
					char et,p;
					int l=denom->GetElementLen(0,denom->m_NumElements-1,PlusLevel,&et,&p);
					if ((l<denom->m_NumElements) && ((et=='+') || (et=='-')))
					{
						char et2,p2;
						int l2=denom->GetElementLen(l,denom->m_NumElements-1,PlusLevel,&et2,&p2);
						if ((l2+l==denom->m_NumElements) && ((et2=='+') || (et2=='-')))
						{
							denom->InsertEmptyElement(0,5,'(');
							CExpression *tmp=(CExpression*)(denom->m_pElementList->pElementObject->Expression1);

							if (et2!='+')
								tmp->InsertEmptyElement(0,2,et2);
							for (int k=l+1+p2;k<=l+l2;k++)
								tmp->InsertElement(denom->m_pElementList+k,tmp->m_NumElements);
							int sign_pos=tmp->m_NumElements;
							tmp->InsertEmptyElement(tmp->m_NumElements,2,et);
							for (int k=1+p;k<l+1;k++)
								tmp->InsertElement(denom->m_pElementList+k,tmp->m_NumElements);
							for (int k=1;k<=l+l2;k++)
								denom->DeleteElement(1);

							nom->InsertEmptyElement(0,5,'(');
							tmp=(CExpression*)(nom->m_pElementList->pElementObject->Expression1);
							int ccc=nom->m_NumElements;
							for (int k=1;k<ccc;k++)
							{
								tmp->InsertElement(nom->m_pElementList+1,tmp->m_NumElements);
								nom->DeleteElement(1);
							}

							nom->InsertElement(denom->m_pElementList,1);
							tmp=(CExpression*)((nom->m_pElementList+1)->pElementObject->Expression1);
							if (((tmp->m_pElementList+sign_pos)->Type==2) && ((tmp->m_pElementList+sign_pos)->pElementObject->Data1[0]==et))
							{
								if (et=='+') (tmp->m_pElementList+sign_pos)->pElementObject->Data1[0]='-';
								if (et=='-') (tmp->m_pElementList+sign_pos)->pElementObject->Data1[0]='+';
							}

							denom->InsertElement(nom->m_pElementList+1,1);

							int cntr=0;
							while ((nom->Compute(0,nom->m_NumElements-1,0)) && (cntr<50)) cntr++;
							cntr=0;
							while ((denom->Compute(0,denom->m_NumElements-1,0)) && (cntr<50)) cntr++;
							
							retval=-1;
						}					
					}
				}
			}
		}
		
		int cnt=0;
		while ((denom->Compute(0,denom->m_NumElements-1,2)) && (cnt<50)) cnt++;
		if (cnt) retval=-1;
		cnt=0;
		while ((nom->Compute(0,nom->m_NumElements-1,2)) && (cnt<50)) cnt++;
		if (cnt) retval=-1;
	}



	{
		tPureFactors PureFactors;
		PureFactors.N1=PureFactors.N2=PureFactors.N3=PureFactors.N4=1.0;
		PureFactors.prec1=PureFactors.prec2=0;
		PureFactors.is_frac1=PureFactors.is_frac2=0;
		int force_calc=0;
		if ((nom->StrikeoutCommonFactors(0,nom->m_NumElements-1,1,denom,0,denom->m_NumElements-1,1,&PureFactors,1)) &&
			(ComputationType==10))
		{
			PureFactors.N1*=PureFactors.N4/PureFactors.N3/PureFactors.N2;;
			PureFactors.N2=PureFactors.N3=PureFactors.N4=1.0;
			force_calc=1;
		}
		int sign_change=0;
		if (PureFactors.N2<0) {PureFactors.N2=-PureFactors.N2;PureFactors.N1=-PureFactors.N1;sign_change=1;}
		if (PureFactors.N3<0) {PureFactors.N3=-PureFactors.N3;PureFactors.N1=-PureFactors.N1;sign_change=1;}
		if (PureFactors.N4<0) {PureFactors.N4=-PureFactors.N4;PureFactors.N1=-PureFactors.N1;sign_change=1;}
		if ((sign_change) || 
			((PureFactors.N2==PureFactors.N4) && (ReduceTwoNumbers(&PureFactors.N1,&PureFactors.N3))) ||
			(force_calc))
		{
			//remove all common factors and pure numbers
			nom->StrikeoutRemove(0,nom->m_NumElements-1,1);
			denom->StrikeoutRemove(0,denom->m_NumElements-1,1);

			//if (nom->m_pElementList->Type==0) nom->InsertEmptyElement(0,1,'1');
			//if (denom->m_pElementList->Type==0) denom->InsertEmptyElement(0,1,'1');

			if ((nom->m_NumElements==1) && (nom->m_pElementList->Type==1) && (strcmp(nom->m_pElementList->pElementObject->Data1,"1")==0))
				nom->DeleteElement(0);
			if ((denom->m_NumElements==1) && (denom->m_pElementList->Type==1) && (strcmp(denom->m_pElementList->pElementObject->Data1,"1")==0))
				denom->DeleteElement(0);

			int l1=nom->FindLowestOperatorLevel((char)0xD7);
			int l2=denom->FindLowestOperatorLevel((char)0xD7);

			//re-insert pure numbers (after these were reduced)
			if (l1<MulLevel)
			{
				nom->InsertEmptyElement(0,5,'(');
				CExpression *tmp=(CExpression*)(nom->m_pElementList->pElementObject->Expression1);
				for (int kk=1;kk<nom->m_NumElements;kk++)
					tmp->InsertElement(nom->m_pElementList+kk,tmp->m_NumElements);
				while (nom->m_NumElements>1) nom->DeleteElement(1);
			}
			int pp=0;
			if ((nom->m_pElementList->Type==2) &&
				(GetOperatorLevel(nom->m_pElementList->pElementObject->Data1[0])==PlusLevel))
				pp=1;
			nom->GenerateASCIIFraction(pp,PureFactors.N1,PureFactors.N2,PureFactors.prec1,PureFactors.is_frac1);
			if (l2<MulLevel)
			{
				denom->InsertEmptyElement(0,5,'(');
				CExpression *tmp=(CExpression*)(denom->m_pElementList->pElementObject->Expression1);
				for (int kk=1;kk<denom->m_NumElements;kk++)
					tmp->InsertElement(denom->m_pElementList+kk,tmp->m_NumElements);
				while (denom->m_NumElements>1) denom->DeleteElement(1);					
			}
			if ((denom->m_pElementList->Type==2) &&
				(GetOperatorLevel(denom->m_pElementList->pElementObject->Data1[0])==PlusLevel))
				pp=1;
			denom->GenerateASCIIFraction(pp,PureFactors.N3,PureFactors.N4,PureFactors.prec2,PureFactors.is_frac2);
			retval=-1;
		}
		else
		{
	 		//remove only common factors that are not pure numbers
	 		if (nom->StrikeoutRemove(0,nom->m_NumElements-1,3)) retval=-1;
			if (denom->StrikeoutRemove(0,denom->m_NumElements-1,3)) retval=-1;

			if (nom->m_pElementList->Type==0) nom->InsertEmptyElement(0,1,'1');
			if (denom->m_pElementList->Type==0) denom->InsertEmptyElement(0,1,'1');
		}
	}


	Level1=nom->FindLowestOperatorLevel((char)0xD7);
	Level2=denom->FindLowestOperatorLevel((char)0xD7);

	
	//some special handling for speed - if pure numbers only calculation (function plotting) change negative numbers into handier form with integrated minus sign
	if (is_call_from_ploter)
	if ((ComputationType==10) && (nom->m_NumElements==2) &&
		(nom->m_pElementList->Type==2) && (nom->m_pElementList->pElementObject->Data1[0]=='-') &&
		((nom->m_pElementList+1)->Type==1))
	{
		nom->DeleteElement(0);
		memmove(nom->m_pElementList->pElementObject->Data1+1,nom->m_pElementList->pElementObject->Data1,11);
		nom->m_pElementList->pElementObject->Data1[0]='-';
	}


	//if denominator is equal to one then convert into non-fraction
	if (((denom->m_NumElements==1) && (denom->m_pElementList->Type==1)) ||
		((denom->m_NumElements==2) && (denom->m_pElementList->Type==2) && ((denom->m_pElementList+1)->Type==1)))
	{
		double N;
		int prec;
		if (denom->IsPureNumber(0,denom->m_NumElements,&N,&prec))
		{
			if (fabs(N-1.0)<1e-100)  //+1
			{
				if (nom->m_NumElements==1)
				{
					InsertElement(nom->m_pElementList,Position+1);
					DeleteElement(Position);
				}
				else
				{
					InsertEmptyElement(Position+1,5,'(');
					CExpression *arg=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
					arg->CopyExpression(nom,0);arg->m_ParenthesesFlags=0x02;
					DeleteElement(Position);
				}
				xreturn (1);
			}
			if (fabs(N+1.0)<1e-100) //-1
			{
				InsertEmptyElement(Position+1,5,'(');
				CExpression *arg=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
				arg->CopyExpression(nom,0);arg->m_ParenthesesFlags=0x02;
				InsertEmptyElement(Position+1,5,'(');
				arg=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);
				arg->InsertEmptyElement(0,2,'-');
				arg->InsertEmptyElement(0,1,'1');
				DeleteElement(Position);
				xreturn (1);
			}
		}
	}

	//if numerator is equal to zero, then the fraction is zero
	if (((nom->m_NumElements==1) && (nom->m_pElementList->Type==1)) ||
		((nom->m_NumElements==2) && (nom->m_pElementList->Type==2) && ((nom->m_pElementList+1)->Type==1)))
	{
		double N;
		int prec;
		if (nom->IsPureNumber(0,nom->m_NumElements,&N,&prec))
			if (fabs(N)<1e-100)
			{
				InsertEmptyElement(Position+1,1,'0');
				DeleteElement(Position);
				xreturn (1);
			}
	}

	//make fraction beautiful (exchange factors between nominator and denominator to
	//look more beautiful - this only can be done if nom and denom are in form of product)
	for (int iii=0;iii<2;iii++)
	{
		int n_pos=0;
		if (Level1==MulLevel)
		while (1)
		{
			char et1;
			char p1;
			int l_nom;
			l_nom=nom->GetElementLen(n_pos,nom->m_NumElements-1,Level1,&et1,&p1);
			if (l_nom-p1==0) break;

			if ((n_pos==0) && (l_nom>1) && (nom->m_pElementList->Type==2) &&
				(GetOperatorLevel(nom->m_pElementList->pElementObject->Data1[0])==PlusLevel))
			{
				l_nom--;
				n_pos++;
			}

			//put the Imaginary Unit to the numerator side
			if ((iii==1) && (ImaginaryUnit))
			if ((l_nom==1) && ((nom->m_pElementList+n_pos)->Type==1) &&
				((nom->m_pElementList+n_pos)->pElementObject->Data1[0]==ImaginaryUnit) &&
				((nom->m_pElementList+n_pos)->pElementObject->Data1[1]==0) &&
				(((nom->m_pElementList+n_pos)->pElementObject->m_VMods)==0) &&
				(((nom->m_pElementList+n_pos)->pElementObject->Data2[0]&0xE0)!=0x60))
			{
				if (Level2<Level1) //put denominator into parentheses
				{
					denom->InsertEmptyElement(0,5,'(');
					CExpression *tmp=(CExpression*)((denom->m_pElementList)->pElementObject->Expression1);
					int kk=0;
					while (denom->m_NumElements>1)
					{
						tmp->InsertElement(denom->m_pElementList+1,kk++);
						denom->DeleteElement(1);
					}
				}
				denom->InsertEmptyElement(denom->m_NumElements,5,'(');
				CExpression *tmp=(CExpression*)((denom->m_pElementList+denom->m_NumElements-1)->pElementObject->Expression1);
				tmp->InsertEmptyElement(0,2,'-');
				tmp->InsertEmptyElement(1,1,ImaginaryUnit);
				for (int kk=n_pos;kk<n_pos+l_nom;kk++)
					nom->DeleteElement(n_pos);
				if ((nom->m_pElementList->Type==0) || ((nom->m_NumElements==1) && (nom->m_pElementList->Type==2)))
					nom->InsertEmptyElement(nom->m_NumElements,1,'1');
				xreturn (-1);
			}

			//put elements with '/' to the other side of fraction
			if (et1=='/')
			{
				if (Level2<Level1) //put denominator into parentheses
				{
					denom->InsertEmptyElement(0,5,'(');
					CExpression *tmp=(CExpression*)((denom->m_pElementList)->pElementObject->Expression1);
					int kk=0;
					while (denom->m_NumElements>1)
					{
						tmp->InsertElement(denom->m_pElementList+1,kk++);
						denom->DeleteElement(1);
					}
				}

				for (int kk=n_pos+p1;kk<n_pos+l_nom;kk++)
					denom->InsertElement(nom->m_pElementList+kk,denom->m_NumElements);

				for (int kk=n_pos;kk<n_pos+l_nom;kk++)
					nom->DeleteElement(n_pos);		
				if ((nom->m_pElementList->Type==0) || ((nom->m_NumElements==1) && (nom->m_pElementList->Type==2)))
					nom->InsertEmptyElement(nom->m_NumElements,1,'1');
				xreturn (-1);
			}

			//put elements with negtive exponents to the other side of fraction
			if ((l_nom-p1==1) && (et1==(char)0xD7) && ((nom->m_pElementList+n_pos+p1)->Type==3))
			{
				CExpression *exp=(CExpression*)((nom->m_pElementList+n_pos+p1)->pElementObject->Expression2);
				tPureFactors PF;
				PF.N1=PF.N2=PF.N3=PF.N4=1.0;
				PF.is_frac1=PF.is_frac2=0;
				PF.prec1=PF.prec2=0;
				exp->StrikeoutCommonFactors(0,exp->m_NumElements-1,1,NULL,0,0,1,&PF);
				if (PF.N2<0) {PF.N1=-PF.N1;PF.N2=-PF.N2;}
				if (PF.N1<0)
				{
					double N1=-PF.N1;
					double N2=PF.N2;
					int is_fraction=PF.is_frac1;
					int precision=PF.prec1;
					exp->StrikeoutRemove(0,exp->m_NumElements-1,2);
					int pp=0;
					if ((exp->m_pElementList->Type==2) && (GetOperatorLevel((exp->m_pElementList->pElementObject->Data1[0])==PlusLevel)))
						pp=1;
					exp->GenerateASCIIFraction(pp,N1,N2,precision,is_fraction);

					if (Level2<Level1) //put denominator into parentheses
					{
						denom->InsertEmptyElement(0,5,'(');
						CExpression *tmp=(CExpression*)((denom->m_pElementList)->pElementObject->Expression1);
						int kk=0;
						while (denom->m_NumElements>1)
						{
							tmp->InsertElement(denom->m_pElementList+1,kk++);
							denom->DeleteElement(1);
						}
					}

					for (int kk=n_pos+p1;kk<n_pos+l_nom;kk++)
						denom->InsertElement(nom->m_pElementList+kk,denom->m_NumElements);

					for (int kk=n_pos;kk<n_pos+l_nom;kk++)
						nom->DeleteElement(n_pos);	
					if ((nom->m_pElementList->Type==0) || ((nom->m_NumElements==1) && (nom->m_pElementList->Type==2)))
						nom->InsertEmptyElement(nom->m_NumElements,1,'1');
					
					xreturn (-1);
				}
				else
					exp->StrikeoutRemove(0,exp->m_NumElements-1);

			}

			//if there are subfractions
			if ((l_nom-p1==1) && (et1==(char)0xD7) && ((nom->m_pElementList+n_pos+p1)->Type==4))
			{
				CExpression *nn=(CExpression*)((nom->m_pElementList+n_pos+p1)->pElementObject->Expression1);
				CExpression *dd=(CExpression*)((nom->m_pElementList+n_pos+p1)->pElementObject->Expression2);
				int nnl=nn->FindLowestOperatorLevel();
				if (nnl==-1) nnl=Level1;
				int ddl=dd->FindLowestOperatorLevel();
				if (ddl==-1) ddl=Level1;

				{
					nom->InsertEmptyElement(nom->m_NumElements,5,'(');
					CExpression *tmp=(CExpression*)((nom->m_pElementList+nom->m_NumElements-1)->pElementObject->Expression1);
					for (int kk=0;kk<nn->m_NumElements;kk++)
						tmp->InsertElement(nn->m_pElementList+kk,tmp->m_NumElements);
				}

				if (Level2<Level1) //put denominator into parentheses
				{
					denom->InsertEmptyElement(0,5,'(');
					CExpression *tmp=(CExpression*)((denom->m_pElementList)->pElementObject->Expression1);
					int kk=0;
					while (denom->m_NumElements>1)
					{
						tmp->InsertElement(denom->m_pElementList+1,kk++);
						denom->DeleteElement(1);
					}
				}

				{
					denom->InsertEmptyElement(denom->m_NumElements,5,'(');
					CExpression *tmp=(CExpression*)((denom->m_pElementList+denom->m_NumElements-1)->pElementObject->Expression1);
					for (int kk=0;kk<dd->m_NumElements;kk++)
						tmp->InsertElement(dd->m_pElementList+kk,tmp->m_NumElements);
				}

				nom->RemoveSequence(Level1,n_pos,n_pos+l_nom-1); 
				//for (int kk=n_pos;kk<n_pos+l_nom;kk++)
				//	nom->DeleteElement(n_pos);
				xreturn (-1);
			}
			n_pos+=l_nom;
			if (n_pos>nom->m_NumElements-1) break;
		}

		CExpression *ttmp;
		int ttmpl;
		ttmp=nom;nom=denom;denom=ttmp;
		ttmpl=Level2;Level2=Level1;Level1=ttmpl;
	}


	//if nominator has 'plus level' then we transform the fraction into sum of fractions
	//example: (a+b)/c  ->   a/c + b/c
	if ((ComputationType==0) && (nom->m_NumElements>2) &&
		(GetOperatorLevel(element_type)==PlusLevel))
	{
		while (nom->Compute(0,nom->m_NumElements-1,0));

		int lvl=nom->FindLowestOperatorLevel();
		if (lvl==PlusLevel)
		{
			int pos=0;
			int jj=1;
			while (1)
			{
				char et,p;
				int l=nom->GetElementLen(pos,nom->m_NumElements-1,lvl,&et,&p);
				if (l==0) break;

				
				char net=et;
				if (element_type=='-')
				{
					if (et=='+') net='-';
					if (et=='-') net='+';
				}

				InsertEmptyElement(Position+jj,2,net);
				jj++;
				
				InsertEmptyElement(Position+jj,4,0);
				CExpression *n=(CExpression*)((m_pElementList+Position+jj)->pElementObject->Expression1);
				CExpression *d=(CExpression*)((m_pElementList+Position+jj)->pElementObject->Expression2);
				d->CopyExpression(denom,0);
				for (int kk=pos+p;kk<pos+l;kk++)
					n->InsertElement(nom->m_pElementList+kk,n->m_NumElements);


				jj++;
				pos+=l;
				if (pos>nom->m_NumElements-1) break;
			}

			DeleteElement(Position);
			if ((Position>0) && ((m_pElementList+Position-1)->Type==2) &&
				(GetOperatorLevel((m_pElementList+Position-1)->pElementObject->Data1[0])==PlusLevel))
				DeleteElement(--Position);
			if (((m_pElementList+Position)->Type==2) && ((m_pElementList+Position)->pElementObject->Data1[0]=='+'))
			{
				if (Position==0) DeleteElement(Position);
				else if ((m_pElementList+Position-1)->Type==11) DeleteElement(Position);
				else if ((m_pElementList+Position-1)->Type==12) DeleteElement(Position);
				else if ((m_pElementList+Position-1)->Type==2)
				{
					if (GetOperatorLevel((m_pElementList+Position-1)->pElementObject->Data1[0])<PlusLevel) DeleteElement(Position);
				}
			}

			xreturn (1);
		}
	}
	
	xreturn (retval);
}


//returns 1 if the exponent has to be kept, or 0 if it can be deleted
int CExpression::ExtractVariables(int StartPos, int EndPos, double order,int summand_no,tVariableList *VarList,CExpression *exponent,char style)
{	
	if (summand_no>=MAX_SUMMANDS) return 0;

	PROFILE_TIME(&PROFILER.ExtractVariables);

	int retval=0;
	int Level=FindLowestOperatorLevel(StartPos,EndPos,(char)0xD7);

	int pos=StartPos;
	while (1)
	{
		char et,p;
		int l;
		double ord=order;
		int stl=style;
		if (Level>=MulLevel)
		{
			l=GetElementLen(pos,EndPos,Level,&et,&p);
			if (l==0) break;

			//jump over +/- sign
			if ((pos==0) && (m_pElementList->Type==2) &&
				(GetOperatorLevel(m_pElementList->pElementObject->Data1[0])==PlusLevel))
			{
				if (m_pElementList->pElementObject->Data1[0]=='-') VarList->Constants[summand_no].N1*=-1;
				pos++;
				l--;
				if (l==0) break;
			}

			if (et=='/')  {ord=-ord;stl|=0x02;}
		}
		else
		{
			l=EndPos-StartPos+1;
			et=(char)0xD7;
			p=0;
		}

		tElementStruct *ts=m_pElementList+pos+p;

		double N;
		int prec;
		if ((exponent==NULL) && (IsPureNumber(pos+p,l-p,&N,&prec)))
		{
			if (ord>0) VarList->Constants[summand_no].N1*=pow(N,(double)ord);
			if (ord<0) VarList->Constants[summand_no].N2*=pow(N,-(double)ord);
			if (VarList->Constants[summand_no].prec<prec) VarList->Constants[summand_no].prec=prec;
		}
		else if ((ts->Type==5) && (l-p==1))
		{
			CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
			if (arg->ExtractVariables(0,arg->m_NumElements-1,ord,summand_no,VarList,exponent,stl))
				retval=1;
		}
		else if ((ts->Type==4) && (l-p==1))
		{
			stl|=0x01;
			CExpression *nom=(CExpression*)(ts->pElementObject->Expression1);
			CExpression *denom=(CExpression*)(ts->pElementObject->Expression2);
			if (nom->ExtractVariables(0,nom->m_NumElements-1,ord,summand_no,VarList,exponent,stl))
				retval=1;
			if (denom->ExtractVariables(0,denom->m_NumElements-1,-ord,summand_no,VarList,exponent,stl))
				retval=1;
		}
		else if ((ts->Type==3) && (l-p==1))
		{
			int pure_order=0;
			CExpression *a=(CExpression*)(ts->pElementObject->Expression1);	
			CExpression *e=(CExpression*)(ts->pElementObject->Expression2);
			tPureFactors PF;
			PF.N1=PF.N2=1.0;
			PF.prec1=0;
			PF.is_frac1=0;
			int rval;
			if (e->IsPureNumber(0,e->m_NumElements,&PF.N1,&PF.prec1))
				rval=1;
			else
			{
				rval=e->StrikeoutCommonFactors(0,e->m_NumElements-1,1,NULL,0,0,1,&PF);
				e->StrikeoutRemove(0,e->m_NumElements-1);
				ReduceTwoNumbers(&PF.N1,&PF.N2);
			}

			if (ExtractVariablesMode==0)
			{
				int pure_order=0;
				pure_order=(int)(PF.N1+((PF.N1>0)?0.01:-0.01));
				if (pure_order==0) pure_order=(PF.N1>=0)?1:-1;
				PF.N1/=(double)pure_order;
				ord*=pure_order;
			}
			else
			{
				ord*=PF.N1/PF.N2;
				PF.N1=1.0;PF.N2=1.0;
			}


			CExpression *var=NULL;
			if ((fabs(PF.N1-1.0)>1e-100) || (fabs(PF.N2-1.0)>1e-100) || (rval==0))
			{
				//we have to create a temporary exponent variable

				tPureFactors PF2;
				PF2.N1=PF2.N2=1.0;

				var=new CExpression(NULL,NULL,100);
				var->CopyExpression(e,0);
				var->StrikeoutCommonFactors(0,var->m_NumElements-1,1,NULL,0,0,1,&PF2);
				var->StrikeoutRemove(0,var->m_NumElements-1,2);

				if (exponent)
				{
					var->InsertEmptyElement(var->m_NumElements,5,'(');
					CExpression *tmp=(CExpression*)((var->m_pElementList+var->m_NumElements-1)->pElementObject->Expression1);
					tmp->CopyExpression(exponent,0);tmp->m_ParenthesesFlags=0x02;
				}

				int pp=0;
				if ((var->m_pElementList->Type==2) && 
					(GetOperatorLevel(var->m_pElementList->pElementObject->Data1[0])==PlusLevel))
					pp=1;
				var->GenerateASCIIFraction(pp,PF.N1,PF.N2,PF.prec1,PF.is_frac1);
			}
			if (!a->ExtractVariables(0,a->m_NumElements-1,ord,summand_no,VarList,var,stl))
			{
				if (var) delete var;
			}
			// will not set retval to 1 because the exponent is not used

		}
		else
		{
			//new variable found - updating the variable list
			tVariable *tvar=VarList->Variables;
			int i=0;
			for (i=0;i<VarList->NumVariables;i++,tvar++)
			{
				int var_sign=0;
				for (int kk=0;kk<=summand_no;kk++) 
				{
					double O=tvar->summand[kk].dorder;
					if (O>1e-100) {var_sign=1;break;}
					else if (O<-1e-100) {var_sign=-1;break;}
				}

				if (((ord>0) && (var_sign>=0)) ||
					((ord<0) && (var_sign<=0)))
					if (CompareExpressions(pos+p,pos+l-1,tvar->variable,tvar->pos,tvar->pos+tvar->len-1))
						if (((exponent==NULL) && (tvar->exponent==NULL)) ||
							((exponent) && (tvar->exponent) &&
							(exponent->CompareExpressions(0,exponent->m_NumElements-1,tvar->exponent,0,tvar->exponent->m_NumElements-1))))
							break;
			}
			if (i<VarList->NumVariables)
			{
				tSummand *tsum=&(tvar->summand[summand_no]);
				tsum->dorder+=ord;
				tsum->order+=(int)(ord+((ord>0)?0.01:-0.01));
				tsum->style|=stl;
			}
			else if (VarList->NumVariables<MAX_VARIABLES)
			{
				tSummand *tsum=&(tvar->summand[summand_no]);
				tvar->exponent=exponent;
				tvar->len=l-p;
				tsum->dorder=ord;
				tsum->order=(int)(ord+((ord>0)?0.01:-0.01));
				tvar->pos=pos+p;
				tvar->variable=this;
				tsum->style=stl;
				VarList->NumVariables++;
				retval=1;
			}		
		}

		pos+=l;
		if (pos>EndPos) break;
	}

	xreturn (retval);
}

//changes this expression into standard polinomia form
//it uses the given variable to make the polinomization. If the variable is NULL or
//is an empty expression it autodetects the variable. If variable is non-NULL, but is
//empty, it copies the autodetected variable into 'variable'
int CExpression::Polynomize(CExpression * variable,char *alternative_variable,char is_greek)
{
	int polynome_order=-1;

	if (m_IsComputed>>24) 
		return (m_IsComputed>>24);

	PROFILE_TIME(&PROFILER.Polynomize);

	//prepare the expression for polynomization
	{
		int cnt=0;
		while ((Compute(0,m_NumElements-1,0)) && (cnt<50)) cnt++;
	}
	
	//make some additional checkings (to proceed the expression must be summation of summands)
	int Level=FindLowestOperatorLevel('+');
	if (Level>PlusLevel) Level=PlusLevel;
	if (Level<PlusLevel) xreturn (-1);

	//prepare sturcture that will hold list of all variables in this expression
	tVariableList *VarList;
	VarList=(tVariableList*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,sizeof(tVariableList));
	for (int i=0;i<MAX_SUMMANDS;i++) VarList->Constants[i].N1=VarList->Constants[i].N2=1.0;

	//create list of all variables (run through all summands and call 'ExtractVariables')
	int pos=0;
	int summand_no=0;
	while (1)
	{
		char et,p;
		int l=GetElementLen(pos,m_NumElements-1,Level,&et,&p);
		if (l==0) break;

		if (et=='-') VarList->Constants[summand_no].N1=-VarList->Constants[summand_no].N1;
		ExtractVariables(pos+p,pos+l-1,1.0,summand_no,VarList);

		summand_no++;
		pos+=l;
		if (pos>m_NumElements-1) break;
	}


	//if not even a single variable found - return error
	if (VarList->NumVariables==0) 
	{
		HeapFree(ProcessHeap,0,VarList);
		xreturn (-1);
	}


	int reduction=1;
	int var_order=-1;
	int max_found=0;
	int the_highest_level=0;
	if (((variable==NULL) || (variable->m_pElementList->Type==0)) && (alternative_variable==NULL))
	{
		//the polynome variable was not defined, so we will find most suitable one.
		//Decide what variable to use for polynome development - 
		//the one that is used in large number of diffrent orders (powers)
		for (int i=0;i<VarList->NumVariables;i++)
		{
			int the_lowest_style=0x7F;
			int num_found=0;
			int highest_level=0;
			char lowest_style=0;
			for (int j=0;j<20;j++)
				for (int kk=0;kk<summand_no;kk++)
					if (abs(VarList->Variables[i].summand[kk].order)==j) 
					{
						lowest_style|=VarList->Variables[i].summand[kk].style;
						highest_level=j;
						num_found++;
						break;
					}

			if ((num_found>max_found) || 
				((num_found==max_found) && (highest_level>the_highest_level)) ||
				((num_found==max_found) && (highest_level==the_highest_level) && (lowest_style<the_lowest_style)))
			{
				the_lowest_style=lowest_style;
				the_highest_level=highest_level;
				max_found=num_found;
				var_order=i;
			}			 
		}
	}
	else 
	{
		//the polynome variable is defined, so search throught the list of variables
		//and find it
		for (int kk=0;kk<VarList->NumVariables;kk++)
		{
			int fnd=0;
			if ((variable) && (variable->m_pElementList->Type))
			{
				if (variable->CompareExpressions(0,variable->m_NumElements-1,
					VarList->Variables[kk].variable,
					VarList->Variables[kk].pos,
					VarList->Variables[kk].pos+VarList->Variables[kk].len-1)) 
					fnd=1;
			}
			else
			{
				tElementStruct *ts=VarList->Variables[kk].variable->m_pElementList+VarList->Variables[kk].pos;

				if ((VarList->Variables[kk].len==1) &&
					(strcmp(ts->pElementObject->Data1,alternative_variable)==0) &&
					((ts->pElementObject->m_VMods)==0) &&
					((((ts->pElementObject->Data2[0]&0xE0)!=0x60) && (!is_greek)) ||
					(((ts->pElementObject->Data2[0]&0xE0)==0x60) && (is_greek))))
					fnd=1;
			}


			if (fnd)
			{
				var_order=kk;
				for (int j=0;j<summand_no;j++)
				{
					if (abs(VarList->Variables[kk].summand[j].order)>the_highest_level)
						the_highest_level=abs(VarList->Variables[kk].summand[j].order);
					if (VarList->Variables[kk].summand[j].order) max_found++;
				}
				break;
			}
		}
	}

	//error - the polynome variable was not found
	if (var_order==-1)
	{
		goto polynomize_end;
	}

	//check if we can reduce orders (example: x^6+x^4+x^2 -> (x^2)^3+(x^2)^2+(x^2)
	//if ((the_highest_level>0) && (max_found>2))
	for (int i=2;i<20;i++)
	{
		int founds=0;
		int reducabile=1;
		for (int j=0;j<summand_no;j++)
		{
			if (VarList->Variables[var_order].summand[j].order) founds++;
			double N=(double)VarList->Variables[var_order].summand[j].order/(double)i/(double)reduction;
			if (fabs(N-(long long)(N+((N>=0)?0.01:-0.01)))>1e-100)
			{
				reducabile=0;
			}
		}
		if (reducabile)
		{
			reduction*=i;
			i--;
		}
		if (founds<=1) {reduction=1;break;}
	} 

	//now find if any other variable has constant ratio with the choosen variable.
	//mark all such variables as polynome variables
	for (int i=0;i<VarList->NumVariables;i++)
	{
		double ratio=0;
		int j=0;
		for (j=0;j<summand_no;j++)
		{
			if ((VarList->Variables[var_order].summand[j].order==0) &&
				(VarList->Variables[i].summand[j].order==0)) continue;

			if ((VarList->Variables[var_order].summand[j].order!=0) &&
				(VarList->Variables[i].summand[j].order==0)) break;

			if ((VarList->Variables[var_order].summand[j].order==0) &&
				(VarList->Variables[i].summand[j].order!=0)) break;

			if (fabs(ratio)<1e-100)
			{
				ratio=(double)VarList->Variables[i].summand[j].order*reduction/(double)VarList->Variables[var_order].summand[j].order;
				if (fabs(ratio-(long long)ratio)>1e-100) break;
				if (fabs(ratio)<0.9999) break;
			}
			else 
			{
				if (fabs((double)VarList->Variables[var_order].summand[j].order*ratio-(double)VarList->Variables[i].summand[j].order*reduction)>1e-100) break;
			}					
		}
		VarList->Variables[i].summand[summand_no].order=0;
		if ((j==summand_no) && (fabs(ratio)>0.999))
		{
			//mark this as polynome variable
			VarList->Variables[i].summand[summand_no].order=(int)(ratio+((ratio>=0)?0.01:-0.01));
		}
	}

	//at last, build the polynome
	CExpression *polynome=new CExpression(NULL,NULL,100);
	for (int order=20;order>=0;order--)
	{
		CExpression *arg=NULL;  //inside of parenthese
		CExpression *a;    //variable argument
		CExpression *e;    //variable exponent
		CExpression *numerator2=NULL;
		CExpression *denominator2=NULL;	

		int summands_found=0;
		for (int j=0;j<summand_no;j++)
			if (abs(VarList->Variables[var_order].summand[j].order)==order*reduction)
				summands_found++;

		int found_any=0;
		int residuum_added=0;
		int residuum_pos=0;
		for (int j=0;j<summand_no;j++)
		{
			CExpression *numerator=NULL;
			CExpression *denominator=NULL;					
			if (abs(VarList->Variables[var_order].summand[j].order)==order*reduction)
			{
				if (polynome_order==-1) polynome_order=order;

				if (!found_any)
				{


					if (summands_found>1)
					{
						if (polynome->m_pElementList->Type!=0)
						{
						if (((polynome->m_pElementList+polynome->m_NumElements-1)->Type!=2) && 
							((polynome->m_pElementList+polynome->m_NumElements-1)->Type!=11) && 
							((polynome->m_pElementList+polynome->m_NumElements-1)->Type!=12))
							polynome->InsertEmptyElement(polynome->m_NumElements,2,'+');
						}

						polynome->InsertEmptyElement(polynome->m_NumElements,5,'(');
						arg=(CExpression*)((polynome->m_pElementList+polynome->m_NumElements-1)->pElementObject->Expression1);
						residuum_pos=0;
					}
					else
					{
						arg=polynome;
						if (polynome->m_pElementList->Type==0)
							residuum_pos=0;
						else
							residuum_pos=polynome->m_NumElements;
					}

					if (order==1)
					{
						a=polynome;
					}
					else if (order)
					{
						polynome->InsertEmptyElement(polynome->m_NumElements,3,0);
						a=(CExpression*)((polynome->m_pElementList+polynome->m_NumElements-1)->pElementObject->Expression1);
						e=(CExpression*)((polynome->m_pElementList+polynome->m_NumElements-1)->pElementObject->Expression2);

						e->GenerateASCIINumber((double)order,order,1,0,0);
					}
				}


				{

					int minus=1;
					if (VarList->Constants[j].N1<0) {VarList->Constants[j].N1=-VarList->Constants[j].N1;minus=-minus;}
					if (VarList->Constants[j].N2<0) {VarList->Constants[j].N1=-VarList->Constants[j].N2;minus=-minus;}
					double N1=VarList->Constants[j].N1;
					double N2=VarList->Constants[j].N2;

					if (minus==-1)
						arg->InsertEmptyElement(residuum_pos++,2,'-');
					else if ((residuum_pos) && ((arg->m_pElementList+residuum_pos-1)->Type!=2))
						arg->InsertEmptyElement(residuum_pos++,2,'+');

					residuum_added=0;

					int prec=VarList->Constants[j].prec;
					if (fabs(N2-1.0)>1e-100)
					{
						arg->InsertEmptyElement(residuum_pos++,4,0);
						CExpression *n=(CExpression*)((arg->m_pElementList+residuum_pos-1)->pElementObject->Expression1);
						CExpression *d=(CExpression*)((arg->m_pElementList+residuum_pos-1)->pElementObject->Expression2);
						n->GenerateASCIINumber(N1,(long long)(N1+((N1>0)?0.01:-0.01)),(fabs(N1-(long long)N1)<1e-100)?1:0,prec,0);
						d->GenerateASCIINumber(N2,(long long)(N2+((N2>0)?0.01:-0.01)),(fabs(N2-(long long)N2)<1e-100)?1:0,prec,0);
						residuum_added=1;
					} 
					else if (fabs(N1-1.0)>1e-100)
					{
						int pos;
						pos=arg->m_NumElements;
						if (arg->m_pElementList->Type==0) pos=0;
						residuum_pos+=arg->GenerateASCIINumber(N1,(long long)(N1+((N1>0)?0.01:-0.01)),(fabs(N1-(long long)N1)<1e-100)?1:0,prec,residuum_pos);
						residuum_added=1;
					}

				}
				
				for (int i=0;i<VarList->NumVariables;i++)
				if (VarList->Variables[i].summand[j].order)
				{
					if (VarList->Variables[i].summand[summand_no].order)
					{
						if (VarList->Variables[i].summand[summand_no].style!=order)
						{
							VarList->Variables[i].summand[summand_no].style=order; //mark that we already used this one for this order

							CExpression *a2=a;
							int order2=VarList->Variables[i].summand[j].order/order;
							if (VarList->Variables[i].summand[j].style&0x01)
							{
								if (numerator2==NULL)
								{
									a->InsertEmptyElement(a->m_NumElements,4,0);
									numerator2=(CExpression *)((a->m_pElementList+a->m_NumElements-1)->pElementObject->Expression1);
									denominator2=(CExpression *)((a->m_pElementList+a->m_NumElements-1)->pElementObject->Expression2);
								}
								if (order2>0) a2=numerator2;
								if (order2<0) {a2=denominator2;order2=-order2;}
							}
							if ((VarList->Variables[i].summand[j].style&0x02) && (order2<0))
							{
								if (a2->m_pElementList->Type==0) a2->InsertEmptyElement(0,1,'1');
								a2->InsertEmptyElement(a2->m_NumElements,2,'/');
								order2=-order2;
							}

							if ((VarList->Variables[i].exponent) || 
								(order2!=1))
							{
								a2->InsertEmptyElement(a2->m_NumElements,3,0);
								CExpression *aa=(CExpression*)((a2->m_pElementList+a2->m_NumElements-1)->pElementObject->Expression1);
								CExpression *ee=(CExpression*)((a2->m_pElementList+a2->m_NumElements-1)->pElementObject->Expression2);
								if (VarList->Variables[i].exponent)
									ee->CopyExpression(VarList->Variables[i].exponent,0);
								int pp=0;
								if ((ee->m_pElementList->Type==2) && (GetOperatorLevel(ee->m_pElementList->pElementObject->Data1[0])==PlusLevel))
									pp=1;
								if (order2!=1)
									ee->GenerateASCIINumber((double)order2,order2,1,0,pp);
								for (int kk=0;kk<VarList->Variables[i].len;kk++)
									aa->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,aa->m_NumElements);
							}
							else if (VarList->Variables[i].variable->FindLowestOperatorLevel(VarList->Variables[i].pos,VarList->Variables[i].pos+VarList->Variables[i].len-1,(char)0xD7)<MulLevel)
							{
								a2->InsertEmptyElement(a2->m_NumElements,5,'(');
								CExpression *tmp=(CExpression*)((a2->m_pElementList+a2->m_NumElements-1)->pElementObject->Expression1);
								for (int kk=0;kk<VarList->Variables[i].len;kk++)
									tmp->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,tmp->m_NumElements);
							}
							else
							{
								for (int kk=0;kk<VarList->Variables[i].len;kk++)
									a2->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,a2->m_NumElements);
							}
						}
					}
					else
					{
						int copyback=1;
						residuum_added=1;
						int xresiduum_pos=residuum_pos;
						CExpression *arg2=arg;
						int order2=VarList->Variables[i].summand[j].order;
						if (VarList->Variables[i].summand[j].style&0x01)
						{
							if (numerator==NULL)
							{
								arg->InsertEmptyElement(residuum_pos,4,0);
								numerator=(CExpression *)((arg->m_pElementList+residuum_pos)->pElementObject->Expression1);
								denominator=(CExpression *)((arg->m_pElementList+residuum_pos)->pElementObject->Expression2);
								residuum_pos++;
							}
							if (order2>0) {arg2=numerator;}
							if (order2<0) {arg2=denominator;order2=-order2;}

							xresiduum_pos=arg2->m_NumElements;
							if (arg2->m_pElementList->Type==0) xresiduum_pos=0;
							copyback=0;
						}
						if ((VarList->Variables[i].summand[j].style&0x02) && (order2<0))
						{
							if ((xresiduum_pos==0) || (((arg2->m_pElementList+xresiduum_pos-1)->Type==2) &&
								(GetOperatorLevel((arg2->m_pElementList+xresiduum_pos-1)->pElementObject->Data1[0])<MulLevel)))
								arg2->InsertEmptyElement(xresiduum_pos++,1,'1');
							arg2->InsertEmptyElement(xresiduum_pos++,2,'/');
							order2=-order2;
							
						}

						if ((VarList->Variables[i].exponent) || 
							(order2!=1))
						{
							arg2->InsertEmptyElement(xresiduum_pos,3,0);
							CExpression *aa=(CExpression*)((arg2->m_pElementList+xresiduum_pos)->pElementObject->Expression1);
							CExpression *ee=(CExpression*)((arg2->m_pElementList+xresiduum_pos)->pElementObject->Expression2);
							xresiduum_pos++;
							if (VarList->Variables[i].exponent)
								ee->CopyExpression(VarList->Variables[i].exponent,0);
							if (order2!=1)
								ee->GenerateASCIINumber((double)order2,order2,1,0,0);
							for (int kk=0;kk<VarList->Variables[i].len;kk++)
								aa->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,aa->m_NumElements);

						}
						else if (VarList->Variables[i].variable->FindLowestOperatorLevel(VarList->Variables[i].pos,VarList->Variables[i].pos+VarList->Variables[i].len-1,(char)0xD7)<MulLevel)
						{
							arg2->InsertEmptyElement(xresiduum_pos,5,'(');
							CExpression *tmp=(CExpression*)((arg2->m_pElementList+xresiduum_pos)->pElementObject->Expression1);
							xresiduum_pos++;
							for (int kk=0;kk<VarList->Variables[i].len;kk++)
								tmp->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,tmp->m_NumElements);
						}
						else
						{
							for (int kk=0;kk<VarList->Variables[i].len;kk++)
								arg2->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,xresiduum_pos++);
						}
						if (copyback) residuum_pos=xresiduum_pos;
					}
				} //for all variables
				found_any=1;
			} //if this summand contains the polynome variable at the exact level
			if ((numerator) && (numerator->m_pElementList->Type==0)) numerator->InsertEmptyElement(0,1,'1');
			if ((denominator) && (denominator->m_pElementList->Type==0)) denominator->InsertEmptyElement(0,1,'1');
			if ((residuum_added==0) && (arg) && ((arg!=polynome) || (order==0)))
			{
				arg->InsertEmptyElement(residuum_pos++,1,'1');
				residuum_added=1;
			}
		} //for every summand in original expression

		if ((numerator2) && (numerator2->m_pElementList->Type==0)) numerator2->InsertEmptyElement(0,1,'1');
		if ((denominator2) && (denominator2->m_pElementList->Type==0)) denominator2->InsertEmptyElement(0,1,'1');
	} //for every polynome order




	//if the variable is empty (but existing) then copy the polynome variable int it
	if ((variable) && (variable->m_pElementList->Type==0))
	{
		for (int kk=0;kk<VarList->Variables[var_order].len;kk++)
			variable->InsertElement(VarList->Variables[var_order].variable->m_pElementList+VarList->Variables[var_order].pos+kk,variable->m_NumElements);
	}

	//copy the bult polynome int this variable
	CopyExpression(polynome,0);
	delete polynome;

polynomize_end:
	//delete all temporary created objects
	for (int i=0;i<MAX_VARIABLES;i++)
		if (VarList->Variables[i].exponent)
			delete VarList->Variables[i].exponent;
	HeapFree(ProcessHeap,0,VarList);

	m_IsComputed|=polynome_order<<24;
	xreturn (polynome_order);
}

//divides this polynome with polynome Q and places the result into 'result' ('result's previous content is deleted)
//polynome variable is in 'variable'. if 'variable' is NULL, the variable will be autodetected
//'this', 'Q', and 'result' are changed
int CExpression::DividePolynome(CExpression *Q, int orderQ, CExpression * result, CExpression * variable)
{
	int retval=0;
	if (variable==NULL) return 0;
	if (result==NULL) return 0;
	if (Q==NULL) return 0;
	if (orderQ<=0) return 0;

	PROFILE_TIME(&PROFILER.DividePolynome);

	CExpression *tmp=NULL;
	CExpression *P=this;

	//firs extract factors from 'P' into factor array

	tmp=new CExpression(NULL,NULL,100);
	result->Delete();


	CExpression *factors[16];
	char factors_et[16];
	for (int k=0;k<16;k++)
		factors[k]=NULL;

	int pnum_factors=0;
	int pos=0;
	while (1)
	{
		char et,p;
		int l=P->GetElementLen(pos,P->m_NumElements-1,PlusLevel,&et,&p);
		if (l==0) goto divide_polynome_exit2;

		int fnd=0;
		tElementStruct *ts=P->m_pElementList+pos+l-1;
		if (ts->Type==3) 
		{
			CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
			CExpression *exp=(CExpression*)(ts->pElementObject->Expression2);
			double N;
			int prec;
			if ((exp->IsPureNumber(0,exp->m_NumElements,&N,&prec)) && (N>0) && (fabs(N-(long long)N)<1e-100))
			{
				int ord=(int)(N+0.01);
				if (variable->CompareExpressions(0,variable->m_NumElements-1,arg,0,arg->m_NumElements-1))
				{
					if (factors[ord]==NULL) 
						factors[ord]=new CExpression(NULL,NULL,100);
					for (int k=pos+p;k<pos+l-1;k++)
						factors[ord]->InsertElement(P->m_pElementList+k,factors[ord]->m_NumElements);
					if (factors[ord]->m_pElementList->Type==0) factors[ord]->InsertEmptyElement(0,1,'1');
					factors_et[ord]=et;
					if (ord>pnum_factors) pnum_factors=ord;
					fnd=1;
				}
			}
		}
		if (!fnd)
		{
			if ((l>=variable->m_NumElements) &&
				(variable->CompareExpressions(0,variable->m_NumElements-1,P,pos+l-variable->m_NumElements,pos+l-1)))
			{
				if (factors[1]==NULL) 
					factors[1]=new CExpression(NULL,NULL,100);
				for (int k=pos+p;k<pos+l-variable->m_NumElements;k++)
					factors[1]->InsertElement(P->m_pElementList+k,factors[1]->m_NumElements);
				if (factors[1]->m_pElementList->Type==0) factors[1]->InsertEmptyElement(0,1,'1');
				factors_et[1]=et;
				if (pnum_factors<1) pnum_factors=1;
				fnd=1;
			}
		}
		if (!fnd)
		{
			if (factors[0]==NULL)
				factors[0]=new CExpression(NULL,NULL,100);
			for (int k=pos+p;k<=pos+l-1;k++)
				factors[0]->InsertElement(P->m_pElementList+k,factors[0]->m_NumElements);
			factors_et[0]=et;
			fnd=1;
		}

		pos+=l;
		if (pos>=P->m_NumElements) break;
	}
	if (pnum_factors==0) goto divide_polynome_exit2;
	if (pnum_factors<orderQ) goto divide_polynome_exit2;

	for (int k=0;k<pnum_factors;k++)
		if (factors[k]==NULL)
		{
			factors[k]=new CExpression(NULL,NULL,100);
			factors[k]->InsertEmptyElement(0,1,'0');
		}
	int num_factors=pnum_factors;


	//now we have factors extracted into array, begin dividing
	char p2,et2;
	int LevelQ=Q->FindLowestOperatorLevel();
	if ((LevelQ==-1) || (LevelQ>PlusLevel)) LevelQ=PlusLevel;
	if (LevelQ<PlusLevel) goto divide_polynome_exit2; //strange error - should not happen
	int l2=Q->GetElementLen(0,Q->m_NumElements-1,LevelQ,&et2,&p2);
	int l2f=l2-1;
	if (orderQ==1)
	{
		if ((l2>=variable->m_NumElements) &&
			(variable->CompareExpressions(0,variable->m_NumElements-1,Q,l2-variable->m_NumElements,l2-1)))
		{
			l2f=l2-variable->m_NumElements;
		}
	}

	int security_cntr=0;
	while (security_cntr<20)
	{
		security_cntr++;

		tmp->Delete();
		tmp->InsertEmptyElement(0,4,0);
		CExpression *num=(CExpression*)(tmp->m_pElementList->pElementObject->Expression1);
		CExpression *denom=(CExpression*)(tmp->m_pElementList->pElementObject->Expression2);
		num->CopyExpression(factors[num_factors],0);
		for (int kk=p2;kk<l2f;kk++)
			denom->InsertElement(Q->m_pElementList+kk,denom->m_NumElements);
		if (l2f-p2==0)
			denom->InsertEmptyElement(0,1,'1');


		int cnt=0;
		while ((tmp->Compute(0,tmp->m_NumElements-1,1)) && (cnt<50)) cnt++;

		//now the 'tmp' holds the multyplying factor
		char et=factors_et[num_factors];
		if (et2=='-')
		{
			if (et=='+') result->InsertEmptyElement(result->m_NumElements,2,et='-');
			else if (et=='-') result->InsertEmptyElement(result->m_NumElements,2,et='+');
		}
		if (et2=='+')
		{
			if (et=='+') result->InsertEmptyElement(result->m_NumElements,2,et='+');
			else if (et=='-') result->InsertEmptyElement(result->m_NumElements,2,et='-');
		}
		for (int kk=0;kk<tmp->m_NumElements;kk++)
			result->InsertElement(tmp->m_pElementList+kk,result->m_NumElements);
		if (num_factors>orderQ)
		{
			result->InsertEmptyElement(result->m_NumElements,3,0);
			CExpression *arg=(CExpression*)((result->m_pElementList+result->m_NumElements-1)->pElementObject->Expression1);
			CExpression *exp=(CExpression*)((result->m_pElementList+result->m_NumElements-1)->pElementObject->Expression2);
			arg->CopyExpression(variable,0);
			exp->GenerateASCIINumber((double)(num_factors-orderQ),num_factors-orderQ,1,0,0);
		}
		num_factors--;

		int tmpf=num_factors;
		int pos=l2;
		if (pos<=Q->m_NumElements-1)
		while (1)
		{
			if (tmpf<0) goto divide_polynome_exit2; //should never happen

			char et2,p2;
			int l2=Q->GetElementLen(pos,Q->m_NumElements-1,LevelQ,&et2,&p2);
			if (l2==0) break;

			char ee='+';
			if (et2=='-')
			{
				if (et=='+') ee='+';
				if (et=='-') ee='-';
			}
			if (et2=='+')
			{
				if (et=='+') ee='-';
				if (et=='-') ee='+';
			}		
			factors[tmpf]->InsertEmptyElement(factors[tmpf]->m_NumElements,2,ee);
			for (int kk=pos+p2;kk<pos+l2;kk++)
				factors[tmpf]->InsertElement(Q->m_pElementList+kk,factors[tmpf]->m_NumElements);
			for (int kk=0;kk<tmp->m_NumElements;kk++)
				factors[tmpf]->InsertElement(tmp->m_pElementList+kk,factors[tmpf]->m_NumElements);

			tmpf--;
			pos+=l2;
			if (pos>Q->m_NumElements-1) break;
		}


		int all_zero=1;
		for (int k=0;k<=num_factors;k++)
		{
			cnt=0;
			while ((factors[k]->Compute(0,factors[k]->m_NumElements-1,0)) && (cnt<50)) cnt++;

			double N;
			int prec;
			if (!factors[k]->IsPureNumber(0,factors[k]->m_NumElements,&N,&prec)) 
			{
				all_zero=0;
			}
			else
			{
				if (fabs(N)>1e-100) all_zero=0;
			}
		}
		if (all_zero) {retval=1;goto divide_polynome_exit2;}


		//check if finished - 'P/Q' does not exist
		if (orderQ>num_factors) goto divide_polynome_exit2; 
	}

divide_polynome_exit2:
	delete tmp;
	for (int i=0;i<=pnum_factors;i++)
		if (factors[i]) delete factors[i];
	xreturn (retval);

}

const int PrimeNumbers[25]=
{
	11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,113
};

//divides N1 and N2 by common factor
int CExpression::ReduceTwoNumbers(double * N1, double * N2)
{
	//first check if N1 and N2 are round numbers
	
	long long t1=(long long)(*N1+((*N1>=0)?0.01:-0.01));
	if (fabs(*N1-(double)t1)>1e-100) return 0;
	long long t2=(long long)(*N2+((*N2>=0)?0.01:-0.01));
	if (fabs(*N2-(double)t2)>1e-100) return 0;

	if ((t1==0) || (t2==0)) return 0;
	if ((t1==1) || (t1==-1)) return 0;
	if ((t2==1) || (t2==-1)) return 0;

	if ((*(((unsigned long*)&t1)+1)==0xFFFFFFFF) || (*(((unsigned long*)&t1)+1)==0))
	if ((*(((unsigned long*)&t2)+1)==0xFFFFFFFF) || (*(((unsigned long*)&t2)+1)==0))
	{
		//faster verzion of algorithm - works with longs
		long st1=(long)t1;
		long st2=(long)t2;

		if (st1==st2) {*N1=1.0;*N2=1.0;return 1;}

		long tt=st1;
		while (((st1&1)==0) && ((st2%2)==0)) {st1/=2;st2/=2;} //cannot make bit shift (negative numbers)
		while (((st1%3)==0) && ((st2%3)==0)) {st1/=3;st2/=3;}
		while (((st1%5)==0) && ((st2%5)==0)) {st1/=5;st2/=5;}
		while (((st1%7)==0) && ((st2%7)==0)) {st1/=7;st2/=7;}
		if (((st1<=10) && (st1>=0)) || ((st1>=-10) && (st1<0))) goto end_reduce_two_numbers;
		if (((st2<=10) && (st2>=0)) || ((st2>=-10) && (st2<0))) goto end_reduce_two_numbers;
		for (int i=0;i<25;i++)
		{
			long p=(long)PrimeNumbers[i];
			while (((st1%p)==0) && ((st2%p)==0)) {st1/=p;st2/=p;}
		}
	end_reduce_two_numbers:
		if (tt!=st1) {*N1=(double)st1;*N2=(double)st2;return 1;}
		return 0;
	}


	//slower version of algorithm - works with long longs
	if (t1==t2) {*N1=1.0;*N2=1.0;return 1;}

	long long tt=t1;

	while (((t1&1)==0) && ((t2%2)==0)) {t1/=2;t2/=2;} //cannot make bit shift (negative numbers)
	while (((t1%3)==0) && ((t2%3)==0)) {t1/=3;t2/=3;}
	while (((t1%5)==0) && ((t2%5)==0)) {t1/=5;t2/=5;}
	while (((t1%7)==0) && ((t2%7)==0)) {t1/=7;t2/=7;}
	if (((t1<=10) && (t1>=0)) || ((t1>=-10) && (t1<0))) goto end_reduce_two_numbers_s;
	if (((t2<=10) && (t2>=0)) || ((t2>=-10) && (t2<0))) goto end_reduce_two_numbers_s;
	for (int i=0;i<25;i++)
	{
		long long p=(long long)PrimeNumbers[i];
		while (((t1%p)==0) && ((t2%p)==0)) {t1/=p;t2/=p;}
	}

end_reduce_two_numbers_s:
	if (tt!=t1) {*N1=(double)t1;*N2=(double)t2;return 1;}
	return 0;
}

//removes given mathematical sequence from mathematical expression
//taking care that math formula remains well-formed
int CExpression::RemoveSequence(int Level, int StartPos, int EndPos)
{ 
	int sStartPos=StartPos;
	if (StartPos<0) return 0;
	if (EndPos>=m_NumElements) return 0;
	if (EndPos==-1)
	{
		if (Level==0) 
			EndPos=m_NumElements-1;
		else
		{
			char p,et;
			int l=GetElementLen(StartPos,m_NumElements-1,Level,&et,&p);
			EndPos=StartPos+l-1;
		}
	}

	for (int kk=StartPos;kk<=EndPos;kk++)
		DeleteElement(StartPos);

	//remove the operator in front of this element
	if (StartPos)
	{
		if (((m_pElementList+StartPos-1)->Type==11) && (Level==GetOperatorLevel((char)0xFF)))
			DeleteElement(--StartPos);
		else if (((m_pElementList+StartPos-1)->Type==12) && (Level==GetOperatorLevel((char)0xFE)))
			DeleteElement(--StartPos);
		else if (((m_pElementList+StartPos-1)->Type==2) && (Level==GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0])))
			DeleteElement(--StartPos);
	}

	//remove operators that are now at very first position
	int FrontLevel=MulLevel;
	if (StartPos==0) FrontLevel=-1;
	else if ((m_pElementList+StartPos-1)->Type==11) FrontLevel=GetOperatorLevel((char)0xFF);
	else if ((m_pElementList+StartPos-1)->Type==12) FrontLevel=GetOperatorLevel((char)0xFE);
	else if ((m_pElementList+StartPos-1)->Type==2) FrontLevel=GetOperatorLevel((m_pElementList+StartPos-1)->pElementObject->Data1[0]);
	if (FrontLevel<Level)
	{
		if (((m_pElementList+StartPos)->Type==11) && (Level==GetOperatorLevel((char)0xFF)))
			DeleteElement(StartPos);
		else if (((m_pElementList+StartPos)->Type==12) && (Level==GetOperatorLevel((char)0xFE)))
			DeleteElement(StartPos);
		else if (((m_pElementList+StartPos)->Type==2) && (Level==GetOperatorLevel((m_pElementList+StartPos)->pElementObject->Data1[0])))
		{
			char ch=(m_pElementList+StartPos)->pElementObject->Data1[0];
			if (ch=='/')
				InsertEmptyElement(StartPos,1,'1'); //special handling
			else if ((ch=='-') || (ch==(char)0xB2) || (ch==(char)0xB1)) //minus-plus and plus-minus
			{
				//do nothing
			}
			else 
				DeleteElement(StartPos);		
		}
	} 

	//if the expression is left empty (or only with one + or - opertor
	if ((m_pElementList->Type==0) ||
		((m_pElementList->Type==2) && (m_NumElements==1) && 
		(GetOperatorLevel(m_pElementList->pElementObject->Data1[0])==PlusLevel)))
	{
		if (Level==PlusLevel) InsertEmptyElement(m_NumElements,1,'0');
		if (Level==MulLevel) InsertEmptyElement(m_NumElements,1,'1');
	}

	return sStartPos-StartPos;
}

int CExpression::InsertSequence(char element_type, int Position, CExpression * Source, int StartPos, int EndPos)
{
	int Level=-1;
	if ((element_type=='+') || (element_type=='-')) Level=PlusLevel;
	else if ((element_type=='/') || (element_type==(char)0xD7)) Level=MulLevel;
	else Level=GetOperatorLevel(element_type);
	if (Level==-1) return 0;

	if (StartPos<0) return 0;
	if (Source==NULL) return 0;
	if (Position>m_NumElements) Position=m_NumElements;
	if (Position<0) Position=0;
	if ((Position) && ((m_pElementList+Position-1)->Type==0)) Position--;
	if (EndPos==-1)
	{
		char et,p;
		EndPos=StartPos+Source->GetElementLen(StartPos,Source->m_NumElements-1,Level,&et,&p)-1;
	}
	if (EndPos<StartPos) return 0;


	int is_first=0;
	//find the beginning of sequence of this level
	//if the position is the first in the sequence of this level, then set the 'is_first' variable
	{
		if ((element_type==(char)0xFF) || (element_type==(char)0xFE)) //row or column separator
		{
			while (Position>=0)
			{
				if (Position==m_NumElements) break;
				if ((m_pElementList+Position)->Type==12) break;
				if ((m_pElementList+Position)->Type==11) break;
				Position--;
			}
			if (Position<0) {Position=0;is_first=1;}
		}
		else
		{
			int armed=0;
			while (Position>=0)
			{
				if (Position>0)
				{
					if ((m_pElementList+Position-1)->Type==12) {is_first=1;break;}
					else if ((m_pElementList+Position-1)->Type==11) {is_first=1;break;}
					else if ((m_pElementList+Position-1)->Type==2)
					{
						int lvl=GetOperatorLevel((m_pElementList+Position-1)->pElementObject->Data1[0]);
						if (lvl<Level) {is_first=1;break;}
					}
				}


				if (Position<m_NumElements)
				{
					if ((m_pElementList+Position)->Type==12) break;
					else if ((m_pElementList+Position)->Type==11) break;
					else if ((m_pElementList+Position)->Type==2)
					{
						int lvl=GetOperatorLevel((m_pElementList+Position)->pElementObject->Data1[0]);
						if (lvl<=Level) break;
					}
					else if ((m_pElementList+Position)->Type!=2)
					{
						if ((MulLevel<=Level) && (Position==0)) break;
						if ((MulLevel<=Level) && (Position) && ((m_pElementList+Position-1)->Type!=2)) break;
						if ((MulLevel<=Level) && (Position) && ((m_pElementList+Position-1)->Type==2) &&
							(GetOperatorLevel((m_pElementList+Position-1)->pElementObject->Data1[0])<=Level)) break;
					}
				}

				if (Position==m_NumElements) break;

				Position--;
				armed=1;
			}
			if (Position<=0) {Position=0;is_first=1;}
		}

	}

	//now we have starting position of our copy-into point

	int use_dot=0;
	if ((!is_first) && (Position>0))
	{
		//clear garbage - there should be no operator (with lower or equal level)
		//to the left of the inserting position.
		if ((((m_pElementList+Position-1)->Type==11) || ((m_pElementList+Position-1)->Type==12)) &&
			(GetOperatorLevel((char)0xFF)<=Level))
		{
			Position--;
			DeleteElement(Position);
		}
		else if (((m_pElementList+Position-1)->Type==2) && 
			(GetOperatorLevel((m_pElementList+Position-1)->pElementObject->Data1[0])<=Level))
		{
			Position--;
			DeleteElement(Position);
		}
		if (Position==0) is_first=1;
	}
	

	int lvl=Source->FindLowestOperatorLevel(StartPos,EndPos);

	if (lvl==-1) 
	{
		if (((Source->m_pElementList+StartPos)->Type==2) && 
			(GetOperatorLevel((Source->m_pElementList+StartPos)->pElementObject->Data1[0])==PlusLevel))
			lvl=PlusLevel;
		else
			lvl=max(Level,MulLevel);
	}

	if ((lvl<Level) ||
		((lvl==MulLevel) && ((Source->m_pElementList+StartPos)->Type==2) && (GetOperatorLevel((Source->m_pElementList+StartPos)->pElementObject->Data1[0])==PlusLevel)))
	{
		char pt='(';
		if ((StartPos==0) && (EndPos==Source->m_NumElements-1) && (Source->m_ParenthesesFlags) &&
			(Source->m_ParentheseShape)) pt=Source->m_ParentheseShape;
		if (element_type!=(char)0xD7) 			
		{
			if (element_type==(char)0xFF) InsertEmptyElement(Position++,11,0);
			else if (element_type==(char)0xFE) InsertEmptyElement(Position++,12,0);
			else InsertEmptyElement(Position++,2,element_type);
		}
		InsertEmptyElement(Position++,5,pt);
		CExpression *tmp=(CExpression*)((m_pElementList+Position-1)->pElementObject->Expression1);
		for (int kk=StartPos;kk<=EndPos;kk++)
			tmp->InsertElement(Source->m_pElementList+kk,tmp->m_NumElements);
	}
	else
	{
		int pos=StartPos;
		while (1)
		{
			char et,p;
			int l=Source->GetElementLen(pos,EndPos,Level,&et,&p);
			if (l==0) break;
			
			char ch=et;
			if (Level<lvl) ch=element_type;
			if ((Level==PlusLevel) && (element_type!='+'))
			{
				if ((et=='+') &&			 (element_type=='-'))		 ch='-';
				else if ((et=='+') &&		 (element_type==(char)0xB1)) ch=(char)0xB1;
				else if ((et=='+') &&		 (element_type==(char)0xB2)) ch=(char)0xB2;
				else if ((et=='-') &&		 (element_type=='-'))		 ch='+';
				else if ((et=='-') &&		 (element_type==(char)0xB1)) ch=(char)0xB2;
				else if ((et=='-') &&		 (element_type==(char)0xB2)) ch=(char)0xB1;
				else if ((et==(char)0xB1) && (element_type=='-'))		 ch=(char)0xB2;
				else if ((et==(char)0xB1) && (element_type==(char)0xB1)) ch='+';
				else if ((et==(char)0xB1) && (element_type==(char)0xB2)) ch='-';
				else if ((et==(char)0xB2) && (element_type=='-'))		 ch=(char)0xB1;
				else if ((et==(char)0xB2) && (element_type==(char)0xB1)) ch='-'; 
				else if ((et==(char)0xB2) && (element_type==(char)0xB2)) ch='+';
			}
			if ((Level==MulLevel) && (element_type!=(char)0xD7))
			{
				if ((et==(char)0xD7) &&		 (element_type=='/'))		 ch='/';
				else if ((et=='/') &&		 (element_type=='/'))		 ch=(char)0xD7;
			}

			if ((is_first) && (ch=='/'))
			{
				InsertEmptyElement(Position++,1,'1');
				is_first=0;
			}

			if ((p) || (ch!=(char)0xD7))
			{
				if ((!is_first) || ((Level==PlusLevel) && (ch!='+')))
				{
					if (ch==(char)0xFF) InsertEmptyElement(Position++,11,0);
					else if (ch==(char)0xFE) InsertEmptyElement(Position++,12,0);
					else InsertEmptyElement(Position++,2,ch);
				}
			}
			is_first=0;

			for (int kk=pos+p;kk<pos+l;kk++)
				InsertElement(Source->m_pElementList+kk,Position++);

			pos+=l;
			if (pos>EndPos) break;
		}
	}

	return Position;
}

#ifdef PROFILE_ON
DWORD profiler_overall_start;
#endif
int CExpression::PROFILERClear(void)
{
#ifdef PROFILE_ON
	memset(&PROFILER,0,sizeof(PROFILER));
	profiler_overall_start=GetTickCount();
	PROFILE_TIME(&PROFILER.OVERALL);
#endif
	return 0;
}

int CExpression::PROFILEREnd(void)
{
#ifdef PROFILE_ON
	tProfiler *profilerid=&PROFILER.OVERALL;
	DWORD profiler_start=profiler_overall_start;
	tProfiler *profiler_caller=NULL;
	xreturn (0);
#endif
	return 0;
}

//Strikes out (flags) all common factors in two expressions - returns: 0-not all factors striked out; 1-all factors striked out
//For example, expression1: 'abc'; expression2: 'abd' - strikes out factors 'a' and 'b' in both expressions
//This is used when comparing expressions - if all factors are striked out, then these two are equal
//Function doesn't remove stikeout flag after it finishes (to make possible for recursive call)
//Primary caller MUST call 'StrikeoutRemove' after it uses this function to delete strikeout flags
//If the 'PureFactors' structure is given, it is filled (in this case all pure-number factors are striked out)
int CExpression::StrikeoutCommonFactors(int StartPos, int EndPos, int inv, CExpression * Other, int StartPos2, int EndPos2, int inv2, tPureFactors *PureFactors,int UseDeepCalculation)
{
	PROFILE_TIME(&PROFILER.StrikeoutCommonFactors);

	int Level2;
	int Level=FindLowestOperatorLevel(StartPos,EndPos,(char)0xD7);
	tElementStruct *firstElement1=m_pElementList+StartPos;
	tElementStruct *firstElement2;
	if (Other) 
	{
		Level2=Other->FindLowestOperatorLevel(StartPos2,EndPos2,(char)0xD7); 
		firstElement2=Other->m_pElementList+StartPos2;
	}
	else 
		Level2=-1;

	//check for flags 0x04 - if yes, redirect
	if ((Level==PlusLevel) && (firstElement1->IsSelected&0x04))
	{
		CExpression *tmp1=*(CExpression**)&(firstElement1->pElementObject->Data3[0]);
		if (tmp1)
		{
			int rval=tmp1->StrikeoutCommonFactors(0,tmp1->m_NumElements-1,inv,Other,StartPos2,EndPos2,inv2,PureFactors,UseDeepCalculation);
			int all_pures=1;
			tElementStruct *ts=tmp1->m_pElementList;
			int kk;
			for (kk=0;kk<tmp1->m_NumElements;kk++,ts++)
			{
				if ((ts->IsSelected&0x40)==0) break;
				if ((ts->IsSelected&0x20)==0) all_pures=0;
			}
			if (kk==tmp1->m_NumElements)
			{
				for (int kk=StartPos;kk<=EndPos;kk++,firstElement1++) 
					(firstElement1->IsSelected)|=0x40|((all_pures)?0x20:0x00);
			}
			xreturn (rval);
		}
		else 
			xreturn (0);
	}
	if ((Level2==PlusLevel) && (firstElement2->IsSelected&0x04))
	{
		CExpression *tmp1=*(CExpression**)&(firstElement2->pElementObject->Data3[0]);
		if (tmp1)
		{
			int rval=StrikeoutCommonFactors(StartPos,EndPos,inv,tmp1,0,tmp1->m_NumElements-1,inv2,PureFactors,UseDeepCalculation);
			int all_pures=1;
			tElementStruct *ts=tmp1->m_pElementList;
			int kk;
			for (kk=0;kk<tmp1->m_NumElements;kk++,ts++)
			{
				if ((ts->IsSelected&0x40)==0) break;
				if ((ts->IsSelected&0x20)==0) all_pures=0;
			}
			if (kk==tmp1->m_NumElements)
			{
				for (int kk=StartPos2;kk<=EndPos2;kk++,firstElement2++) 
					(firstElement2->IsSelected)|=0x40|((all_pures)?0x20:0x00);
			}
			xreturn (rval);
		}
		else 
			xreturn (0);
	}

	if (UseDeepCalculation)
	{
		//we have to do with expression that are not in factorized form - we will try to factorize
		//them or divide them as polynomes. If we succed, the result will be stored in temporary
		//CExpression variable, and an element that will hold pointer to this variable will be
		//flagged with 0x04 bit.
		if ((Level==PlusLevel) && ((firstElement1->IsSelected&0x44)==0))
		{
			//try to factorize frist expression
			if ((StartPos>0) || (EndPos<m_NumElements-1) || ((m_IsComputed&0x00800000)==0))
			{
				CExpression *tmp1=new CExpression(NULL,NULL,100);
				tElementStruct *ts=firstElement1;
				for (int kk=StartPos;kk<=EndPos;kk++,ts++)
					tmp1->InsertElement(ts,tmp1->m_NumElements);

				tmp1->FactorizeExpression();
				if ((StartPos==0) && (EndPos==m_NumElements-1)) m_IsComputed|=0x00800000;

				Level=tmp1->FindLowestOperatorLevel((char)0xD7);
				if (Level>PlusLevel)
				{
					firstElement1->IsSelected|=0x04;
					*(CExpression**)&(firstElement1->pElementObject->Data3[0])=tmp1;
					xreturn (StrikeoutCommonFactors(StartPos,EndPos,inv,Other,StartPos2,EndPos2,inv2,PureFactors,UseDeepCalculation));
				}
				delete tmp1;
			}
		}
		if ((Level2==PlusLevel) && ((firstElement2->IsSelected&0x44)==0))
		{
			//try to factorize second expression
			if ((StartPos2>0) || (EndPos2<Other->m_NumElements-1) || ((Other->m_IsComputed&0x00800000)==0))
			{
				CExpression *tmp1=new CExpression(NULL,NULL,100);
				tElementStruct *ts=firstElement2;
				for (int kk=StartPos2;kk<=EndPos2;kk++,ts++)
					tmp1->InsertElement(ts,tmp1->m_NumElements);

				tmp1->FactorizeExpression();
				if ((StartPos2==0) && (EndPos2==Other->m_NumElements-1)) Other->m_IsComputed|=0x00800000;

				Level2=tmp1->FindLowestOperatorLevel((char)0xD7);
				if (Level2>PlusLevel)
				{
					firstElement2->IsSelected|=0x04;
					*(CExpression**)&(firstElement2->pElementObject->Data3[0])=tmp1;
					xreturn (StrikeoutCommonFactors(StartPos,EndPos,inv,Other,StartPos2,EndPos2,inv2,PureFactors,UseDeepCalculation));
				}
				delete tmp1;
			}
		}

		if ((Level2==PlusLevel) && ((firstElement2->IsSelected&0x44)==0) &&
			 (Level==PlusLevel) && ((firstElement1->IsSelected&0x44)==0))
		{
			//neither expression can be factorized 
			//we are going to try convert to polynomes and divide
			if (inv==inv2)
			if ((StartPos>0) || (EndPos<m_NumElements-1) || ((m_IsComputed&0x00800000)!=0) ||
				(StartPos2>0) || (EndPos2<Other->m_NumElements-1) || ((Other->m_IsComputed&0x00800000)!=0))
			{
				//first we check if both expressions are the same
				tElementStruct *ts;
				if (CompareExpressions(StartPos,EndPos,Other,StartPos2,EndPos2))
				{
					ts=firstElement1;
					for (int ii=StartPos;ii<=EndPos;ii++,ts++)
						ts->IsSelected|=0x40;
					ts=firstElement2;
					for (int ii=StartPos2;ii<=EndPos2;ii++,ts++)
						ts->IsSelected|=0x40;
					xreturn (1);
				}


				//then check if these two expressions are similar ( A=-B )

				CExpression *res=new CExpression(NULL,NULL,100);
				int rval=0;

				ts=firstElement1;
				int no_first_sign=1;
				for (int ii=StartPos;ii<=EndPos;ii++,ts++)
					if (ts->Type==2)
					{
						char *ch=ts->pElementObject->Data1;
						if ((ii==0) && (GetOperatorLevel(*ch)==PlusLevel)) no_first_sign=0;
						if (*ch=='+') *ch='-';
						else if (*ch=='-') *ch='+';
						else if (*ch==(char)0xB1) *ch=(char)0xB2;
						else if (*ch==(char)0xB2) *ch=(char)0xB1;
					}
				int IsComputedFlags;
				if (no_first_sign) {IsComputedFlags=this->m_IsComputed;InsertEmptyElement(0,2,'-');firstElement1=m_pElementList+StartPos;EndPos++;}
				rval=CompareExpressions(StartPos,EndPos,Other,StartPos2,EndPos2);
				ts=firstElement1;
				for (int ii=StartPos;ii<=EndPos;ii++,ts++)
					if (ts->Type==2)
					{
						char *ch=ts->pElementObject->Data1;
						if (*ch=='+') *ch='-';
						else if (*ch=='-') *ch='+';
						else if (*ch==(char)0xB1) *ch=(char)0xB2;
						else if (*ch==(char)0xB2) *ch=(char)0xB1;
					}
				if (no_first_sign) {DeleteElement(0);EndPos--;this->m_IsComputed=IsComputedFlags;}

				if (rval)
				{
					//yes, A=-B, so we replace the first one with -1, and strikeout the other one
					res->InsertEmptyElement(0,2,'-');
					res->InsertEmptyElement(1,1,'1');
					firstElement1->IsSelected|=0x04;
					*(CExpression**)&(firstElement1->pElementObject->Data3[0])=res;
					ts=firstElement2;
					for (int kk=StartPos2;kk<=EndPos2;kk++,ts++)
						ts->IsSelected|=0x40;
					xreturn (StrikeoutCommonFactors(StartPos,EndPos,inv,Other,StartPos2,EndPos2,inv2,PureFactors,UseDeepCalculation));
				}

				//finally, convert them to polynomes and try to divide

				int is_first_complex=0,is_second_complex=0;
				CExpression *first=new CExpression(NULL,NULL,100);
				ts=firstElement1;

				for (int kk=StartPos;kk<=EndPos;kk++,ts++)
					first->InsertElement(ts,first->m_NumElements);					
				CExpression *second=new CExpression(NULL,NULL,100);
				ts=firstElement2;
				for (int kk=StartPos2;kk<=EndPos2;kk++,ts++)
					second->InsertElement(ts,second->m_NumElements);

				char tmpstr[3];
				tmpstr[0]=ImaginaryUnit;
				tmpstr[1]=0;
				if (first->ContainsVariable(0,first->m_NumElements-1,NULL,0,0,tmpstr,0))
					is_first_complex=1;
				if (second->ContainsVariable(0,second->m_NumElements-1,NULL,0,0,tmpstr,0))
					is_second_complex=1;

				CExpression *variable=new CExpression(NULL,NULL,100);

				rval=0;
				if (!is_second_complex)
				{
					int orderQ=second->Polynomize(variable);
					if ((orderQ>=1) && (orderQ<=first->Polynomize(variable)))
						rval=first->DividePolynome(second,orderQ,res,variable);
				}

				if (rval)
				{
					firstElement1->IsSelected|=0x04;
					*(CExpression**)&(firstElement1->pElementObject->Data3[0])=res;
					ts=firstElement2;
					for (int kk=StartPos2;kk<=EndPos2;kk++,ts++)
						ts->IsSelected|=0x40;
					delete first;
					delete second;
					delete variable;
					xreturn (StrikeoutCommonFactors(StartPos,EndPos,inv,Other,StartPos2,EndPos2,inv2,PureFactors,UseDeepCalculation));
				}
				else
				{
					rval=0;
					if (!is_first_complex)
					{
						variable->Delete();
						int orderQ=first->Polynomize(variable);
						if ((orderQ>=1) && (orderQ<=second->Polynomize(variable)))
							rval=second->DividePolynome(first,orderQ,res,variable);
					}

					if (rval)
					{
						firstElement2->IsSelected|=0x04;
						*(CExpression**)&(firstElement2->pElementObject->Data3[0])=res;
						ts=firstElement1;
						for (int kk=StartPos;kk<=EndPos;kk++,ts++)
							ts->IsSelected|=0x40;
						delete first;
						delete second;
						delete variable;
						xreturn (StrikeoutCommonFactors(StartPos,EndPos,inv,Other,StartPos2,EndPos2,inv2,PureFactors,UseDeepCalculation));

					}
				}

				//polynome division not successfull
				delete first;
				delete second;
				delete res;
				delete variable;
				xreturn (0);
			}
		}
	}




	//jump over +/- sign
	int minus_sign=0,jumpover=0;
	int minus_sign2=0,jumpover2=0;
	if ((Level>=MulLevel) && (firstElement1->Type==2))
	{
		if (firstElement1->pElementObject->Data1[0]=='+') 
		{
			jumpover=1;
			(firstElement1->IsSelected)|=0x60;
		}
		if (firstElement1->pElementObject->Data1[0]=='-') 
		{
			jumpover=1;minus_sign=1;
			if ((PureFactors) && ((firstElement1->IsSelected&0x40)==0))
			{
				PureFactors->N1=-PureFactors->N1;
				(firstElement1->IsSelected)|=0x60;
			}
		}
	}
	if ((Other) && (Level2>=MulLevel) && (firstElement2->Type==2))
	{
		if (firstElement2->pElementObject->Data1[0]=='+') 
		{
			jumpover2=1;
			(firstElement2->IsSelected)|=0x60;
		}
		if (firstElement2->pElementObject->Data1[0]=='-') 
		{
			jumpover2=1;minus_sign2=1;
			if ((PureFactors) && ((firstElement2->IsSelected&0x40)==0))
			{
				PureFactors->N3=-PureFactors->N3;
				(firstElement2->IsSelected)|=0x60;
			}
		}
	}
	if ((minus_sign) && (minus_sign2) && (PureFactors==NULL))
	{
		(firstElement1->IsSelected)|=0x40;
		(firstElement2->IsSelected)|=0x40;
	}



	int pos=StartPos+jumpover;
	while (1)
	{
		char et,p;
		int l;
		int invert=inv;
		if (Level>=MulLevel)
		{
			l=GetElementLen(pos,EndPos,Level,&et,&p);
			if (l==0) break;
			if (et=='/') invert=-invert;
		}
		else
		{
			l=EndPos-StartPos+1;
			et=(char)0xD7;
			p=0;
		}

		firstElement1=m_pElementList+pos+p;
		if ((l-p==1) && ((firstElement1->IsSelected&0x40)==0))
		{
			if (PureFactors)
			{
				double N;
				int prec;
				if (IsPureNumber(pos+p,l-p,&N,&prec))
				{
					if (invert==1) PureFactors->N1*=N; else PureFactors->N2*=N;
					if (prec>PureFactors->prec1) PureFactors->prec1=prec;
					if (firstElement1->Type==4) PureFactors->is_frac1=1;
					for (int kk=pos;kk<pos+l;kk++) ((m_pElementList+kk)->IsSelected)|=0x60;
				}
			}
			else if ( (firstElement1->Type==1) &&
				(*(unsigned short*)(firstElement1->pElementObject->Data1)==0x0031)) //this is "1" string
				for (int kk=pos;kk<pos+l;kk++) ((m_pElementList+kk)->IsSelected)|=0x40;
		}

		int pos2=StartPos2+jumpover2;

		while (1)
		{
			int invert2;
			char et2,p2;
			int l2;
			if (Other)
			{
				invert2=inv2;
				if (Level2>=MulLevel)
				{
					l2=Other->GetElementLen(pos2,EndPos2,Level2,&et2,&p2);
					if (l2==0) break;
					if (et2=='/') invert2=-invert2;
				}
				else
				{
					l2=EndPos2-StartPos2+1;
					et2=(char)0xD7;
					p2=0;
				}

				firstElement2=Other->m_pElementList+pos2+p2;

				if ((l2-p2==1) && ((firstElement2->IsSelected&0x40)==0))
				{
					if (PureFactors)
					{
						double N;
						int prec;
						if (Other->IsPureNumber(pos2+p2,l2-p2,&N,&prec))
						{
							if (invert2==1) PureFactors->N3*=N; else PureFactors->N4*=N;
							if (prec>PureFactors->prec2) PureFactors->prec2=prec;
							if (firstElement2->Type==4) PureFactors->is_frac2=1;
							for (int kk=pos2;kk<pos2+l2;kk++) ((Other->m_pElementList+kk)->IsSelected)|=0x60;
						}
					}
					else if ((firstElement2->Type==1) &&
						(*(unsigned short*)(firstElement2->pElementObject->Data1)==0x0031)) //this is "1" string
						for (int kk=pos2;kk<pos2+l2;kk++) ((Other->m_pElementList+kk)->IsSelected)|=0x40;
				}

				if ((firstElement1->IsSelected&0x40)==0)
				if ((firstElement2->IsSelected&0x40)==0)
				{
					//int fnd=0;
					if ((l-p>1) && (l2-p2>1) && (invert==invert2) && 
						(CompareExpressions(pos+p,pos+l-1,Other,pos2+p2,pos2+l2-1)))
					{
						for (int kk=pos;kk<pos+l;kk++) ((m_pElementList+kk)->IsSelected)|=0x40;
						for (int kk=pos2;kk<pos2+l2;kk++) ((Other->m_pElementList+kk)->IsSelected)|=0x40;
						//fnd=1;
					} 
					else
					{
						int type1=firstElement1->Type;
						int type2=firstElement2->Type;
						if ((invert==invert2) && (l-p==1) && (l2-p2==1) && (type1!=4) && (type1!=5) && (type2!=4) && (type2!=5) &&
							(CompareElement(firstElement1,firstElement2)))
						{
							for (int kk=pos;kk<pos+l;kk++) ((m_pElementList+kk)->IsSelected)|=0x40;
							for (int kk=pos2;kk<pos2+l2;kk++) ((Other->m_pElementList+kk)->IsSelected)|=0x40;
							//fnd=1;
						}
						else
						{
							CExpression *first=NULL;
							CExpression *second=NULL;
							if ((type1==3) && (l-p==1))
								first=(CExpression*)(firstElement1->pElementObject->Expression1);
							if ((type2==3) && (l2-p2==1))
								second=(CExpression*)(firstElement2->pElementObject->Expression1);
							if (((type1==4) || (type1==5)) && (l-p==1)) second=NULL; //we skip if fraction or parenthese
							if (((type2==4) || (type2==5)) && (l2-p2==1)) first=NULL;
							if ((first) || (second))
							{
								int start1,end1,start2,end2;
								if (first) {start1=0;end1=first->m_NumElements-1;} else {first=this;start1=pos+p;end1=pos+l-1;}
								if (second){start2=0;end2=second->m_NumElements-1;}else	{second=Other;start2=pos2+p2;end2=pos2+l2-1;}

								if (first->CompareExpressions(start1,end1,second,start2,end2))
								{
									CExpression *exp1=NULL;
									CExpression *exp2=NULL;
									tPureFactors PF;
									PF.N1=PF.N2=PF.N3=PF.N4=1.0;
									PF.prec1=PF.prec2=0;
									PF.is_frac1=PF.is_frac2=0;
									if (first!=this)  exp1=(CExpression*)(firstElement1->pElementObject->Expression2);
									if (second!=Other) exp2=(CExpression*)(firstElement2->pElementObject->Expression2);

									int rval=0;
									if ((exp1) && (exp2))
									{
										rval=exp1->StrikeoutCommonFactors(0,exp1->m_NumElements-1,1,exp2,0,exp2->m_NumElements-1,1,&PF);
										exp1->StrikeoutRemove(0,exp1->m_NumElements-1);
										exp2->StrikeoutRemove(0,exp2->m_NumElements-1);
									}
									else if (exp1)
									{
										rval=exp1->StrikeoutCommonFactors(0,exp1->m_NumElements-1,1,NULL,0,0,1,&PF);
										exp1->StrikeoutRemove(0,exp1->m_NumElements-1);
									}
									else if (exp2)
									{
										rval=exp2->StrikeoutCommonFactors(0,exp2->m_NumElements-1,1,NULL,0,0,1,&PF);
										exp2->StrikeoutRemove(0,exp2->m_NumElements-1);
										PF.is_frac2=PF.is_frac1;
										PF.prec2=PF.prec1;
										PF.N3=PF.N1;PF.N4=PF.N2;
										PF.is_frac1=0;PF.prec1=0;PF.N1=1.0;PF.N2=1.0;
									}
									
									if (rval)
									{
										double N1t=1.0;
										double N2t=1.0;
										double N3t=1.0;
										double N4t=1.0;
										double *N1=&N1t;
										double *N2=&N2t;
										double *N3=&N3t;
										double *N4=&N4t;
										if (first!=this) 
										{
											N1=(double*)&(firstElement1->pElementObject->Data3[4]);
											N2=(double*)&(firstElement1->pElementObject->Data3[8]);
											if ((firstElement1->IsSelected&0x10)==0) {*N1=PF.N1;*N2=PF.N2;}
										}
										if (second!=Other) 
										{
											N3=(double*)&(firstElement2->pElementObject->Data3[4]);
											N4=(double*)&(firstElement2->pElementObject->Data3[8]);
											if ((firstElement2->IsSelected&0x10)==0) {*N3=PF.N3;*N4=PF.N4;}
										}
										if (*N2<0) {*N2=-*N2;*N1=-*N1;}
										if (*N4<0) {*N4=-*N4;*N3=-*N3;}
										if (invert==-1) *N1=-*N1;
										if (invert2==-1) *N3=-*N3;

										//TODO - vjerojatno treba napraviti da se razlièito
										//ponasa pri skracivanju razlomaka i extrahiranju varijabli
										if ((((*N1)>=0) && ((*N3)>=0)) || (((*N1)<=0) && ((*N3)<=0)))
										{
											if ((second!=Other) && (fabs((*N1)/(*N2))<=fabs((*N3)/(*N4))))
											{
												*N3=(*N3)*(*N2)-(*N1)*(*N4);
												*N4=(*N2)*(*N4);
												*N1=0;
												*N2=1.0;
												if (invert2==-1) *N3=-*N3;
												(firstElement2->IsSelected)|=0x10;
												if (fabs((*N3)/(*N4))<1e-100) 
													for (int kk=pos2;kk<pos2+l2;kk++) 
														((Other->m_pElementList+kk)->IsSelected)|=0x40;
												for (int kk=pos;kk<pos+l;kk++) ((m_pElementList+kk)->IsSelected)|=0x40;
												//fnd=1;
											}
											else if ((first!=this) && (fabs((*N1)/(*N2))>=fabs((*N3)/(*N4))))
											{
												*N1=(*N1)*(*N4)-(*N3)*(*N2);
												*N2=(*N2)*(*N4);
												*N3=0;
												*N4=1.0;
												if (invert==-1) *N1=-*N1;
												(firstElement1->IsSelected)|=0x10;
												if (fabs((*N1)/(*N2))<1e-100) 
													for (int kk=pos;kk<pos+l;kk++) 
														((m_pElementList+kk)->IsSelected)|=0x40;
												for (int kk=pos2;kk<pos2+l2;kk++) ((Other->m_pElementList+kk)->IsSelected)|=0x40;
												//fnd=1;
											}
										}
									}
								}
							}
						}
					}
				}
				if ((firstElement2->IsSelected&0x40)==0)
				{
					if ((Level2>=MulLevel) && (l2-p2==1))
					{
						if ((firstElement2->Type==5) && (invert==invert2))
						{
							CExpression *tmp=(CExpression*)(firstElement2->pElementObject->Expression1);
							StrikeoutCommonFactors(pos+p,pos+l-1,invert,tmp,0,tmp->m_NumElements-1,invert2,PureFactors,UseDeepCalculation);
							int all_pures=1;
							tElementStruct *ts=tmp->m_pElementList;
							int kk;
							for (kk=0;kk<tmp->m_NumElements;kk++,ts++)
							{
								if ((ts->IsSelected&0x40)==0) break;
								if ((ts->IsSelected&0x20)==0) all_pures=0;
							}
							if (kk==tmp->m_NumElements)
							{
								ts=Other->m_pElementList+pos2;
								for (int kk=0;kk<l2;kk++,ts++) (ts->IsSelected)|=0x40|((all_pures)?0x20:0x00);
							}
						}
						if (firstElement2->Type==4)
						{
							if (PureFactors) PureFactors->is_frac2=1;
							CExpression *tmp=(CExpression*)(firstElement2->pElementObject->Expression1);
							CExpression *tmp2=(CExpression*)(firstElement2->pElementObject->Expression2);
							StrikeoutCommonFactors(pos+p,pos+l-1,invert,tmp,0,tmp->m_NumElements-1,invert2,PureFactors,UseDeepCalculation);
							StrikeoutCommonFactors(pos+p,pos+l-1,invert,tmp2,0,tmp2->m_NumElements-1,-invert2,PureFactors,UseDeepCalculation);

							int all_pures=1;
							tElementStruct *ts=tmp->m_pElementList;
							int kk;
							for (kk=0;kk<tmp->m_NumElements;kk++,ts++)
							{
								if ((ts->IsSelected&0x40)==0) break;
								if ((ts->IsSelected&0x20)==0) all_pures=0;
							}
							if (kk==tmp->m_NumElements)
							{
								ts=tmp2->m_pElementList;
								int kk2;
								for (kk2=0;kk2<tmp2->m_NumElements;kk2++,ts++)
								{
									if ((ts->IsSelected&0x40)==0) break;
									if ((ts->IsSelected&0x20)==0) all_pures=0;
								}
								if (kk2==tmp2->m_NumElements)
								{
									ts=Other->m_pElementList+pos2;
									for (int kk=0;kk<l2;kk++,ts++) (ts->IsSelected)|=0x40|((all_pures)?0x20:0x00);
								}
							}
						}
					}
				}
			}
			else
			{
				invert2=1;
				l2=0;
				et2=(char)0xD7;
				p2=0;
			}
			if ((firstElement1->IsSelected&0x40)==0)
			{
				if ((Level>=MulLevel) && (l-p==1))
				{
					if ((firstElement1->Type==5) && (invert==invert2))
					{
						CExpression *tmp=(CExpression*)(firstElement1->pElementObject->Expression1);
						tmp->StrikeoutCommonFactors(0,tmp->m_NumElements-1,invert,Other,pos2+p2,pos2+l2-1,invert2,PureFactors,UseDeepCalculation);
						int all_pures=1;

						tElementStruct *ts=tmp->m_pElementList;
						int kk;
						for (kk=0;kk<tmp->m_NumElements;kk++,ts++)
						{
							if ((ts->IsSelected&0x40)==0) break;
							if ((ts->IsSelected&0x20)==0) all_pures=0;
						}
						if (kk==tmp->m_NumElements)
						{
							ts=m_pElementList+pos;
							for (int kk=0;kk<l;kk++,ts++) (ts->IsSelected)|=0x40|((all_pures)?0x20:0x00);
						}
					}

					if (firstElement1->Type==4)
					{
						if (PureFactors) PureFactors->is_frac1=1;
						CExpression *tmp=(CExpression*)(firstElement1->pElementObject->Expression1);
						CExpression *tmp2=(CExpression*)(firstElement1->pElementObject->Expression2);

						tmp->StrikeoutCommonFactors(0,tmp->m_NumElements-1,invert,Other,pos2+p2,pos2+l2-1,invert2,PureFactors,UseDeepCalculation);
						tmp2->StrikeoutCommonFactors(0,tmp2->m_NumElements-1,-invert,Other,pos2+p2,pos2+l2-1,invert2,PureFactors,UseDeepCalculation);
						
						int all_pures=1;
						tElementStruct *ts=tmp->m_pElementList;
						int kk;
						for (kk=0;kk<tmp->m_NumElements;kk++,ts++)
						{
							if ((ts->IsSelected&0x40)==0) break;
							if ((ts->IsSelected&0x20)==0) all_pures=0;
						}
						if (kk==tmp->m_NumElements)
						{
							ts=tmp2->m_pElementList;
							int kk2;
							for (kk2=0;kk2<tmp2->m_NumElements;kk2++,ts++)
							{
								if ((ts->IsSelected&0x40)==0) break;
								if ((ts->IsSelected&0x20)==0) all_pures=0;
							}
							if (kk2==tmp2->m_NumElements)
							{
								ts=m_pElementList+pos;
								for (int kk=0;kk<l;kk++,ts++) (ts->IsSelected)|=0x40|((all_pures)?0x20:0x00);
							}
						}
					}
				}
			}
			if (Other==NULL) break;
			pos2+=l2;
			if (pos2>EndPos2) break;
		}

		pos+=l;
		if (pos>EndPos) break;
	}


	tElementStruct *ts=m_pElementList+StartPos;
	if (Level>=MulLevel)
	{
		for (int kk=StartPos;kk<=EndPos;kk++,ts++)
			if (((ts->IsSelected&0x40)==0) &&
				((ts->Type!=2) || (GetOperatorLevel(ts->pElementObject->Data1[0])!=MulLevel)))
				xreturn (0);
	}
	else if ((ts->IsSelected&0x40)==0) xreturn (0);

	if (Other)
	{
		ts=Other->m_pElementList+StartPos2;
		if (Level2>=MulLevel)
		{
			for (int kk=StartPos2;kk<=EndPos2;kk++,ts++)
				if (((ts->IsSelected&0x40)==0) &&
					((ts->Type!=2) || (GetOperatorLevel(ts->pElementObject->Data1[0])!=MulLevel)))
					xreturn (0);
		}
		else if ((ts->IsSelected&0x40)==0) xreturn (0);
	}

	xreturn (1);
}

//clears strikeout flags - however check the strikeout flags (according to the test_value)
//to decide wheter the element will actually be removed
int CExpression::StrikeoutRemove(int StartPos, int EndPos, char test_value)
{
	int retval=0;

	PROFILE_TIME(&PROFILER.StrikeoutRemove);
	//test_value -> 0 - only remove strikeout flags
	//              1 - remove every factor that is striked out
	//              2 - only remove factors that are pure number (also delete all strikeout flags)
	//              3 - only remove factors that are not pure numbers (also delete all strikeout flags)


	if ((EndPos<0)|| (EndPos>=m_NumElements)) EndPos=m_NumElements-1;
	if (StartPos<0) StartPos=0;
	if (StartPos>=m_NumElements) xreturn(0);
	tElementStruct *theElement=m_pElementList+StartPos;

	if (theElement->IsSelected&0x04)
	{
		//special flag - we have to redirect
		CExpression *tmp1=*(CExpression**)&(theElement->pElementObject->Data3[0]);
		if (tmp1)
		{
			if (tmp1->StrikeoutRemove(0,tmp1->m_NumElements-1,test_value))
				retval=1;
			if ((test_value==1) || (test_value==3))
			{
				for (int kk=StartPos;kk<=EndPos;kk++)
					DeleteElement(StartPos);
				for (int kk=0;kk<tmp1->m_NumElements;kk++)
					InsertElement(tmp1->m_pElementList+kk,StartPos+kk);
				EndPos=StartPos+tmp1->m_NumElements-1;
			}
			delete tmp1;
		}
	}


	for (int i=StartPos;i<=EndPos;i++)
	{
		if (((test_value==1) && (theElement->IsSelected&0x40)) ||
			 ((test_value==2) && (theElement->IsSelected&0x20)) ||
			 ((test_value==3) && ((theElement->IsSelected&0x20)==0) && (theElement->IsSelected&0x40)))
		{
			DeleteElement(i);
			retval=1;
			i--;
			theElement--;
			EndPos--;
		}
		else
		{
			if ((theElement->IsSelected&0x10) && (theElement->Type==3) && ((test_value==1) || (test_value==3)))
			{
				//this exponent is marked as 'partially striked-out'
				double N1=*(double*)&(theElement->pElementObject->Data3[4]);
				double N2=*(double*)&(theElement->pElementObject->Data3[8]);
				ReduceTwoNumbers(&N1,&N2);
				tPureFactors PF;
				PF.prec1=PF.prec2=0;
				PF.is_frac1=PF.is_frac2=0;
				CExpression *exp=(CExpression*)(theElement->pElementObject->Expression2);
				exp->StrikeoutCommonFactors(0,exp->m_NumElements-1,1,NULL,0,0,1,&PF);
				exp->StrikeoutRemove(0,exp->m_NumElements-1,2);
				int pp=0;
				if ((exp->m_pElementList->Type==2) &&
					(GetOperatorLevel(exp->m_pElementList->pElementObject->Data1[0])==PlusLevel)) 
					pp=1;
				exp->GenerateASCIIFraction(pp,N1,N2,PF.prec1,PF.is_frac1);
				retval=1;
			}
			(theElement->IsSelected)&=0x01;
			if ((theElement->Type==4) || (theElement->Type==5))
			{
				CExpression *E1=(CExpression*)(theElement->pElementObject->Expression1);
				CExpression *E2=(CExpression*)(theElement->pElementObject->Expression2);

				if (E1) if (E1->StrikeoutRemove(0,-1,test_value)) retval=1;
				if (E2) if (E2->StrikeoutRemove(0,-1,test_value)) retval=1;

				if ((theElement->Type==5) && (E1->m_pElementList->Type==0))
				{
					DeleteElement(i);
					i--;
					theElement--;
					EndPos--;
					retval=1;
				}
				else if (theElement->Type==4) 
				{
					if ((E1->m_pElementList->Type==0) && (E2->m_pElementList->Type==0))
					{
						DeleteElement(i);
						i--;
						theElement--;
						EndPos--;
						retval=1;
					}
					else if (E1->m_pElementList->Type==0)
					{
						E1->InsertEmptyElement(0,1,'1');
						retval=1;
					}
					else if (E2->m_pElementList->Type==0)
					{
						E2->InsertEmptyElement(0,1,'1');
						retval=1;
					}
				}
			}
		}
		theElement++;
	}

	if (test_value)
	{
		if ((StartPos==EndPos) && ((m_pElementList+StartPos)->Type==2) &&
			(GetOperatorLevel((m_pElementList+StartPos)->pElementObject->Data1[0])==PlusLevel))
			InsertEmptyElement(StartPos+1,1,'1');

		if ((StartPos==0) && (m_NumElements==1) && (m_pElementList->Type==0))
			InsertEmptyElement(0,1,'1');
	}

	xreturn (retval);
}



// can be called only once, before any symbolic-calculator function is used
int CExpression::InitCalculator(void)
{
	memset(OperatorLevelTable,127,256);
	int i=0;
	while (Operators[i].default_operator)
	{
		int j=0;
		while (Operators[i].operator_list[j])
		{
			OperatorLevelTable[(unsigned char)(Operators[i].operator_list[j])]=i;
			j++;
		}
		if (Operators[i].default_operator=='+') PlusLevel=i;
		if (Operators[i].default_operator==(char)0xD7) MulLevel=i;
		if (Operators[i].default_operator=='=') EqLevel=i;

		i++;
	}

	return 0;
}

// changes the expression so it becomes more beautiful (adds '*' between constants...)
int CExpression::MakeExpressionBeautiful(void)
{
	int retval=0;

	//remove parenthese if the whole expression is inside it: (a+b+c)
	if ((this->m_pPaternalElement==NULL) && (this->m_NumElements==1) && (this->m_pElementList->Type==5) &&
		(m_pElementList->pElementObject->Expression2==NULL) &&
		(((CExpression*)m_pElementList->pElementObject->Expression1)->m_ParentheseShape!='c') && //not for ceiling
		(((CExpression*)m_pElementList->pElementObject->Expression1)->m_ParentheseShape!='f') && //not for floor
		((((CExpression*)m_pElementList->pElementObject->Expression1)->m_ParenthesesFlags&0x1C)==0))
	{
		CExpression *arg=(CExpression*)m_pElementList->pElementObject->Expression1;
		int istable=0;
		for (int j=0;j<arg->m_NumElements;j++)
			if ((arg->m_pElementList+j)->Type>8) {istable=1;break;}
		if (!istable)
		{
			for (int i=0;i<arg->m_NumElements;i++)
				InsertElement(arg->m_pElementList+i,i+1);
			DeleteElement(0);
		}
	}

	//if any single element within parentheses, remove the parentheses
	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((ts->Type==5) && (ts->pElementObject) && (ts->pElementObject->Expression2==NULL) &&
			(((CExpression*)ts->pElementObject->Expression1)->m_NumElements==1) &&
			(((CExpression*)ts->pElementObject->Expression1)->m_ParentheseShape!='c') && //not for ceiling
			(((CExpression*)ts->pElementObject->Expression1)->m_ParentheseShape!='f') && //not for floor
			((((CExpression*)ts->pElementObject->Expression1)->m_ParenthesesFlags&0x1C)==0))
		{
			CExpression *arg=(CExpression*)ts->pElementObject->Expression1;
			int istable=0;
			for (int j=0;j<arg->m_NumElements;j++)
				if ((arg->m_pElementList+j)->Type>8) {istable=1;break;}
			if (!istable)
			{
				InsertElement(((CExpression*)ts->pElementObject->Expression1)->m_pElementList,i);
				DeleteElement(i+1);
			}
		}
	}

	//move constants at the front
	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;

		if ((ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12) && (ts->Type!=2) && (ts->Type!=1))
		{
			if (ts->pElementObject->Expression1) ((CExpression*)(ts->pElementObject->Expression1))->MakeExpressionBeautiful();
			if (ts->pElementObject->Expression2) ((CExpression*)(ts->pElementObject->Expression2))->MakeExpressionBeautiful();
			if (ts->pElementObject->Expression3) ((CExpression*)(ts->pElementObject->Expression3))->MakeExpressionBeautiful();
		}
		if ((i==0) || 
			((i>0) && ((ts-1)->Type==11)) ||
			((i>0) && ((ts-1)->Type==12)) ||
			((i>0) && ((ts-1)->Type==2) && (GetOperatorLevel((ts-1)->pElementObject->Data1[0])<PlusLevel))) //start of an expression (higher than plus level)
		{
			if ((ts->Type==2) && (ts->pElementObject->Data1[0]=='+'))
			{
				//removes '+' operator when at the beginning (like in 'a=+b' or '+b+a')
				DeleteElement(i);
				i--;
				continue;
			}
			if ((ts->Type==2) && (ts->pElementObject->Data1[0]=='-'))
				if ((m_NumElements>i+1) && ((ts+1)->Type==1)) 
				{
					char et,p;
					if (GetElementLen(i,m_NumElements-1,PlusLevel,&et,&p)==2)
					{
						double N;
						int prec;
						if ((IsPureNumber(i+1,1,&N,&prec)) && (fabs(N)<1e-100))
						{
							//removes '-' operator when expression is '-0'
							DeleteElement(i);
							i--;
							continue;
						}
					}
				}
		}
		if (i==0) continue;

		double N;
		int prec;
		if (IsPureNumber(i,1,&N,&prec))
		if ((i>=m_NumElements-1) || ((ts+1)->Type!=2) || (GetOperatorLevel((ts+1)->pElementObject->Data1[0])<=MulLevel))
		{
			int has_preop=0;
			if (((ts-1)->Type==2) && (GetOperatorLevel((ts-1)->pElementObject->Data1[0])==MulLevel))
			{
				if ((ts-1)->pElementObject->Data1[0]=='/') has_preop=-1; else has_preop=1;
			}

			if (has_preop>=0)
			{
				int j;
				int add_dot=0;
				for (j=i-1;j>=0;j--)
				{
					tElementStruct *ts2=m_pElementList+j;
					if (ts2->Type==11) {break;}
					if (ts2->Type==12) {break;}
					if ((ts2->Type==2) && (GetOperatorLevel(ts2->pElementObject->Data1[0])<MulLevel)) {break;}
					if (/*(ts2->Type==1) &&*/ (IsPureNumber(j,1,&N,&prec))) {add_dot=1;break;}
				}

				j++;

				if (j<i)
				{
					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->InsertElement(ts,0);
					InsertElement(tmp->m_pElementList,j);
					delete tmp;
					DeleteElement(i+1);
					if (has_preop) DeleteElement(i);
					retval=1;
				}
				if ((j<=i) && (add_dot)) {InsertEmptyElement(j,2,(char)0xD7);retval=1;}

			}
			else
			{
				int j;
				for (j=i+1;j<m_NumElements;j++)
				{
					tElementStruct *ts2=m_pElementList+j;
					if (ts2->Type==11) {break;}
					if (ts2->Type==12) {break;}
					if ((ts2->Type==2) && (GetOperatorLevel(ts2->pElementObject->Data1[0])<MulLevel)) {break;}
					//if (/*(ts2->Type==1) &&*/ (IsPureNumber(j,1,&N,&prec))) {break;}
				}

				if (j>i-1)
				{
					
					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->InsertElement(ts,0);
					InsertElement(tmp->m_pElementList,j);
					delete tmp;
					InsertEmptyElement(j,2,'/');
					DeleteElement(i);
					DeleteElement(i-1);
					//i--;
					retval=1;
				}
			}
		}
	}


	//now check if any summand is zero - if yes,delete it
	int lvl=FindLowestOperatorLevel((char)0xD7);
	if (lvl==PlusLevel)
	{
		int pos=0;
		while (1)
		{
			char et,p;
			int l=GetElementLen(pos,m_NumElements-1,lvl,&et,&p);
			if (l==0) break;

			double N;
			int prec;
			if ((IsPureNumber(pos+p,l-p,&N,&prec)) && (fabs(N)<1e-100))
			{
				RemoveSequence(PlusLevel,pos,pos+l-1);
				l=0;
				retval=1;
			}
			pos+=l;
			if (pos>=m_NumElements) break;
		}
	}

	//check if there is multiplication of a constants with fraction that has constant numerator (like: 2*3/5 )but without multiplication sign in between
	//this is to prevent confustion where the result looks like a mixed fraction
	for (int i=0;i<m_NumElements-1;i++)
	{
		tElementStruct *ts1=m_pElementList+i;
		tElementStruct *ts2=m_pElementList+i+1;

		if ((ts1->Type==1) && (ts2->Type==4) && 
			((i+2==m_NumElements) || ((ts2+1)->Type!=2) || ((ts2+1)->pElementObject->Data1[0]!='!'))) //not a factoriel behind fraction
		{
			double N1,N2;
			int p1,p2;
			if (IsPureNumber(i,1,&N1,&p1)) //only if the first number is an integer
			{
				CExpression *num=((CExpression*)(ts2->pElementObject->Expression1));
				if ((num->m_NumElements>=1) && (num->m_NumElements<=2) && (num->m_pElementList->Type>=1) && (num->m_pElementList->Type<=2) && (num->IsPureNumber(0,num->m_NumElements,&N2,&p2)))
				{
					if (fabs(N1-(int)N1)<1e-100)
					{
						//the first number is an integer so we will multiply and put everything into the numerator
						N1=N1*N2;
						int iii=num->m_NumElements;
						for (int ii=0;ii<iii;ii++)
						num->DeleteElement(0);
						
						num->GenerateASCIINumber(N1,0,0,max(p1,p2),0);
						DeleteElement(i);
						continue;
					}
					else
					{
						//the first number is not an integer, we will put a multiplying dot in between
						InsertEmptyElement(i+1,2,(char)0xD7);
						i++;
						continue;
					}
				}
			}
		}
	}
	

	return retval;
}


//returns 0 if not suitable for computation; 1 - if suitable; 2 - if suitable and only contains pure numbers
//this function MUST NOT change the expression unless the autocorrect is set!
int CExpression::IsSuitableForComputation(int autocorrect)
{
#ifdef TEACHER_VERSION
	if (TheFileType=='r')
	{
		if (TheMathFlags&0x01) return 0;
	}
#endif
	int PrevOperLevel=-1;
	int non_operator_found=0;
	int non_pure_number_found=0;

	int text_var_counter=0;
	int math_var_counter=0;
	//if (m_IsText) return 0;

	for (int i=0;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;

		ts->IsSelected&=0x01;

		if (ts->Type==0) return 0;
		if ((ts->Type!=11) && (ts->Type!=12) && (ts->pElementObject==NULL)) return 0;
		if ((ts->pElementObject) && (ts->pElementObject->m_Text) && (ts->Type!=1)) //we allow some variables as text
			return 0;

		if (ts->Type==1)
		{
			if (ts->pElementObject->m_Text) text_var_counter++; else math_var_counter++;
			if (text_var_counter>8) return 0;

			if (ts->pElementObject->Data1[0]<=32) 
			{
				if (autocorrect) {DeleteElement(i);i--;continue;}
				return 0;
			}
			double N;
			int prec;
			if (!IsPureNumber(i,1,&N,&prec)) non_pure_number_found=1; 

			//check if there are suspicious characters in variable names
			char ch;
			int iii=0;
			while (ch=ts->pElementObject->Data1[iii])
			{
				if ((ch=='+') || (ch=='-') || (ch==(char)0xD7) || (ch=='/') ||
					(ch=='=') || (ch=='<') || (ch=='>')) return 0;
				iii++;
			}
		}

		if (ts->Type==2)
		{
			char ch=ts->pElementObject->Data1[0];
			int lvl=-1;
			if ((ch=='+') || (ch=='-')) lvl=PlusLevel;
			if ((ch=='/') || (ch==(char)0xD7)) lvl=MulLevel;
			if (lvl==-1)
			{
				int k=0;
				while ((lvl==-1) && (Operators[k].default_operator))
				{
					int j=0;
					while (Operators[k].operator_list[j])
					{
						if (Operators[k].operator_list[j]==ch) {lvl=k;break;}
						j++;
					}
					k++;
				}
				if (lvl==-1) return 0; //operator was not found in our list
			}

			if ((lvl==PlusLevel) || (lvl==MulLevel))
			{
				if (PrevOperLevel==lvl) return 0; //not two operators of the plus level nor mul level
				if (i==m_NumElements-1) return 0; //should not end with plus level or mul level operator
			}

			PrevOperLevel=lvl;
		}
		else
			PrevOperLevel=-1;

		if (ts->Type==7) return 0;  //sigma, pi, integral
		if (ts->Type==9) return 0;  //condition list
		if (ts->Type==10) return 0; // condition list

		if ((ts->Type!=0) && (ts->Type!=11) && (ts->Type!=12) && (ts->Type!=2))
		{
			non_operator_found=1;
			if (ts->pElementObject->Expression1) 
			{
				int rv=((CExpression*)(ts->pElementObject->Expression1))->IsSuitableForComputation();
				if (rv==0) return 0;
				if (rv==1) non_pure_number_found=1;
			}
			if (ts->pElementObject->Expression2) 
			{
				int rv=((CExpression*)(ts->pElementObject->Expression2))->IsSuitableForComputation();
				if (rv==0) return 0;
				if (rv==1) non_pure_number_found=1;
			}
			if (ts->pElementObject->Expression3) 
			{
				int rv=((CExpression*)(ts->pElementObject->Expression3))->IsSuitableForComputation();
				if (rv==0) return 0;
				if (rv==1) non_pure_number_found=1;
			}
		}
	}
	if (text_var_counter>math_var_counter) return 0;
	
#ifdef TEACHER_VERSION
	if (TheFileType=='r')
	{
		if (TheMathFlags&0x01) return 0;
		if ((TheMathFlags&0x02) && (non_pure_number_found)) return 0;
	}
#endif
	if (!non_operator_found) return 0;
	return (non_pure_number_found)?1:2;
}

int CExpression::ComputeLog(int Position, char element_type, int ComputationType)
{
	tElementStruct *ts=m_pElementList+Position;
	if (ts->Type!=6) return 0;

	PROFILE_CNT(&PROFILER.ComputeLog);

	int is_log=0;
	int is_ln=0;
	if (strncmp(ts->pElementObject->Data1,"log",3)==0) is_log=1;
	if (strncmp(ts->pElementObject->Data1,"ln",2)==0) is_ln=1;
	if ((is_log==0) && (is_ln==0)) return 0;

	//TODO - logaritam razlomka, logaritam potencije, logaritam korjiena

	CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
	CExpression *base=(CExpression*)(ts->pElementObject->Expression2);

	if (arg)
	{
		int cnt=0;
		while ((arg->Compute(0,arg->m_NumElements-1,ComputationType)) && (cnt<50)) cnt++;
		if (cnt) return -1;
	}
	if (base)
	{
		int cnt=0;
		while ((base->Compute(0,base->m_NumElements-1,(ComputationType==10)?10:0)) && (cnt<50)) cnt++;
		if (cnt) return -1;
	}
	//logarithm of pure number
	{
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;
		PF.prec1=0;
		double NN;
		int pr;
		int ret=arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
		arg->StrikeoutRemove(0,arg->m_NumElements-1);
		//if (arg->IsPureNumber(0,arg->m_NumElements,&NN,&pr))
		if (ret)
		{
			//logarithm of 1
			NN=PF.N1/PF.N2;
			pr=PF.prec1;
			if (fabs(NN-1.0)<1e-100)
			{
				DeleteElement(Position);
				InsertEmptyElement(Position,1,'0');
				return 1;
			}
		
			if (NN>0)
			{
				double baseN;
				int base_prec;
				if ((base==NULL) || (base->IsPureNumber(0,base->m_NumElements,&baseN,&base_prec)))
				{
					if ((base==NULL) && (is_log)) NN=log10(NN);
					if ((base==NULL) && (is_ln)) NN=log(NN);
					if (base) NN=log(NN)/log(baseN);

					//calculating log that gives not very round result
					if ((fabs(NN*1000-(long long)(NN*1000))<1e-100) || (ComputationType==2) || (ComputationType==10))
					{
						DeleteElement(Position);
						if (fabs(NN-(long long)NN)>1e-100) pr++;
						if (fabs(NN*10-(long long)(NN*10))>1e-100) pr++;
						GenerateASCIINumber(NN,(long long)(NN+((NN>0)?0.01:-0.01)),(fabs(NN-(long long)NN)<1e-100)?1:0,pr,Position);
						return 1;
					}
				}
			}
		}
	}



	if (ComputationType==2) return 0;
	if (ComputationType==3) return 0;

	//make blind compare of logharitm base and argument
	if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==3))
	{
		CExpression *a=(CExpression*)((arg->m_pElementList)->pElementObject->Expression1);
		CExpression *e=(CExpression*)((arg->m_pElementList)->pElementObject->Expression2);
		if (((base) && (base->CompareExpressions(0,base->m_NumElements-1,a,0,a->m_NumElements-1))) ||
			((is_log) && (a->m_NumElements==1) && (a->m_pElementList->Type==1) && (strcmp(a->m_pElementList->pElementObject->Data1,"10")==0)) ||
			((is_ln) && (a->m_NumElements==1) && (a->m_pElementList->Type==1) && (strcmp(a->m_pElementList->pElementObject->Data1,"e")==0) && ((a->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60)))
		{
			InsertEmptyElement(Position,5,'(');
			a=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
			a->CopyExpression(e,0);a->m_ParenthesesFlags=0x02;
			DeleteElement(Position+1);
			return 1;
		}
	}
	if (((base) && (base->CompareExpressions(0,base->m_NumElements-1,arg,0,arg->m_NumElements-1))) ||
		((is_log) && (arg->m_NumElements==1) && (arg->m_pElementList->Type==1) && (strcmp(arg->m_pElementList->pElementObject->Data1,"10")==0)) ||
		((is_ln) && (arg->m_NumElements==1) && (arg->m_pElementList->Type==1) && (strcmp(arg->m_pElementList->pElementObject->Data1,"e")==0) && ((arg->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60)))
	{
		DeleteElement(Position);
		InsertEmptyElement(Position,1,'1');
		return 1;
	}

	int Level=arg->FindLowestOperatorLevel((char)0xD7);
	if ((Level==MulLevel) && (ComputationType==0))
	{
		int retval=0;

		//prepare sturcture that will hold list of all variables in this expression
		tVariableList *VarList;
		int summand_no;
		VarList=(tVariableList*)arg->GenerateVariableList(0,arg->m_NumElements-1,&summand_no);

		/*VarList=(tVariableList*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,sizeof(tVariableList));
		for (int i=0;i<MAX_SUMMANDS;i++) VarList->Constants[i].N1=VarList->Constants[i].N2=1.0;

		char has_sign=0;
		if ((arg->m_pElementList->Type==2) && (GetOperatorLevel(arg->m_pElementList->pElementObject->Data1[0])==PlusLevel))
			has_sign=arg->m_pElementList->pElementObject->Data1[0];

		//creating the varible list
		ExtractVariablesMode=1;
		arg->ExtractVariables((has_sign)?1:0,arg->m_NumElements-1,1.0,0,VarList);
		ExtractVariablesMode=0;*/

		CExpression *solution=NULL;
		if (VarList)
		if ((VarList->NumVariables>1) ||
			((VarList->NumVariables==1) && ((fabs(VarList->Constants[0].N1-1.0)>1e-100) || (fabs(VarList->Constants[0].N2-1.0)>1e-100))))
		{
			InsertEmptyElement(Position+1,5,'(');
			solution=(CExpression*)((m_pElementList+Position+1)->pElementObject->Expression1);

			for (int i=-1;i<VarList->NumVariables;i++)
			{
				if (i==-1)
				{
					//adding the constant
				
					if ((fabs(VarList->Constants[0].N1-1.0)<1e-100) && (fabs(VarList->Constants[0].N2-1.0)<1e-100)) continue;
					double N=VarList->Constants[0].N1/VarList->Constants[0].N2;

					if (base)
						solution->InsertEmptyElement(solution->m_NumElements,6,18); //log10
					else
						solution->InsertEmptyElement(solution->m_NumElements,6,19); //log
					tElementStruct *t=solution->m_pElementList+solution->m_NumElements-1;
					tElementStruct *t2=m_pElementList+Position;

					memcpy(t->pElementObject->Data1,t2->pElementObject->Data1,4);
					memcpy(t->pElementObject->Data2,t2->pElementObject->Data2,4);
					if (base)
					{
						CExpression *b=(CExpression*)(t->pElementObject->Expression2);
						b->CopyExpression(base,0);
					}
					CExpression *a=(CExpression*)(t->pElementObject->Expression1);

					a->GenerateASCIINumber(N,(long long)(N+((N>0)?0.01:-0.01)),(fabs(N-(long long)N)<1e-100)?1:0,VarList->Constants[0].prec,0);
					
					continue;
				}

				double x=VarList->Variables[i].summand[0].dorder;

				if ((solution->m_pElementList->Type) || (x<0))
				{
					solution->InsertEmptyElement(solution->m_NumElements,2,(x<0)?'-':'+');
				}

				int anything_added=0;
				x=fabs(x);
				if (fabs(x-1.0)>1e-100)
				{
					int prec=0;
					double xx=x;
					while (fabs(xx-(long long)xx)>1e-100) {prec++;xx*=10;if (prec>8) break;}
					int pp=solution->m_NumElements;
					if (solution->m_pElementList->Type==0) pp=0;
					solution->GenerateASCIINumber(x,(long long)(x+((x>0)?0.01:-0.01)),(fabs(x-(long long)x)<1e-100)?1:0,prec,pp);
					anything_added=1;
				}

				if (VarList->Variables[i].exponent)
				{
					solution->InsertSequence((char)0xD7,solution->m_NumElements,VarList->Variables[i].exponent,0,VarList->Variables[i].exponent->m_NumElements-1);
					anything_added=1;
				}

				CExpression *var=VarList->Variables[i].variable;
				int varlen=VarList->Variables[i].len;
				int varpos=VarList->Variables[i].pos;

				int add_log=1;

				if ((base==NULL) && (varlen==1) && (is_ln) &&
					((var->m_pElementList+varpos)->Type==1) && 
					(*(unsigned short*)((var->m_pElementList+varpos)->pElementObject->Data1)==0x0065) && //this is "e" string
					//(strcmp((var->m_pElementList+varpos)->pElementObject->Data1,"e")==0) &&
					(((var->m_pElementList+varpos)->pElementObject->Data2[0]&0xE0)!=0x60))
					add_log=0;

				
				if ((base==NULL) && (is_log))
				{
					double N;
					int prec;
					if ((var->IsPureNumber(varpos,varlen,&N,&prec)) && (fabs(N-10.0)<1e-100))
						add_log=0;
				}

				if ((base) && (base->CompareExpressions(0,base->m_NumElements-1,var,varpos,varpos+varlen-1)))
					add_log=0;

				if (add_log)
				{
					if (base)
						solution->InsertEmptyElement(solution->m_NumElements,6,18); //log10
					else
						solution->InsertEmptyElement(solution->m_NumElements,6,19); //log
					tElementStruct *t=solution->m_pElementList+solution->m_NumElements-1;
					tElementStruct *t2=m_pElementList+Position;

					memcpy(t->pElementObject->Data1,t2->pElementObject->Data1,4);
					memcpy(t->pElementObject->Data2,t2->pElementObject->Data2,4);
					if (base)
					{
						CExpression *b=(CExpression*)(t->pElementObject->Expression2);
						b->CopyExpression(base,0);
					}
					CExpression *a=(CExpression*)(t->pElementObject->Expression1);


					for (int k=VarList->Variables[i].pos;k<VarList->Variables[i].pos+VarList->Variables[i].len;k++)
						a->InsertElement(VarList->Variables[i].variable->m_pElementList+k,a->m_NumElements);

				}
				else if (!anything_added)
				{
					solution->InsertEmptyElement(solution->m_NumElements,1,'1');
				}

			}	
		}

		//delete all temporary created objects
		arg->FreeVariableList(VarList);
		/*for (int i=0;i<MAX_VARIABLES;i++)
			if (VarList->Variables[i].exponent)
				delete VarList->Variables[i].exponent;
		HeapFree(ProcessHeap,0,VarList);*/

		if (solution)
		{
			retval=1;
			DeleteElement(Position);
		}
		if (retval) return retval;

	}


	return 0;
}

int CExpression::ContainsVariable(int StartPos, int EndPos, CExpression * variable, int VarPos,int VarLen, char *alternative_variable,char is_greek)
{
	if ((variable==NULL) && (alternative_variable==NULL)) return 0;
	if (StartPos<0) return 0;
	if (EndPos<0) return 0;
	if (EndPos<StartPos) return 0;
	if (EndPos>=m_NumElements) return 0;
	if ((variable) && (VarPos<0)) return 0;
	if ((variable) && (VarPos+VarLen>variable->m_NumElements)) return 0;
	if ((variable) && (VarLen==0) && ((m_pElementList->pElementObject+VarPos)->Data1[0]<='9')) return 0;

	for (int i=StartPos;i<=EndPos;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((ts->Type==2) || (ts->Type==11) || (ts->Type==12) || (ts->Type==0)) continue;

		if (variable)
		{
			if ((VarLen==1) && ((variable->m_pElementList+VarPos)->Type==1))
			{
				if (CompareExpressions(i,i,variable,VarPos,VarPos)) return 1;
			}
			else
			{
				if (ts->Type==1)
				{
					double N;
					int prec;
					if (!IsPureNumber(i,1,&N,&prec))
						if (variable->ContainsVariable(VarPos,VarPos+VarLen-1,this,i,1))
							return 1;
				}
			}
		}
		else if (ts->Type==1)
		{
			if ((strcmp(ts->pElementObject->Data1,alternative_variable)==0) &&
				((ts->pElementObject->m_VMods)==0) &&
				((((ts->pElementObject->Data2[0]&0xE0)!=0x60) && (!is_greek)) ||
				 (((ts->pElementObject->Data2[0]&0xE0)==0x60) && (is_greek))))
			{
				return 1;
			}
		}

		if (ts->pElementObject->Expression1)
		{
			if (((CExpression*)(ts->pElementObject->Expression1))->ContainsVariable(0,((CExpression*)(ts->pElementObject->Expression1))->m_NumElements-1,variable,VarPos,VarLen,alternative_variable,is_greek)) 
				return 1;
		}
		if (ts->pElementObject->Expression2)
		{
			if (((CExpression*)(ts->pElementObject->Expression2))->ContainsVariable(0,((CExpression*)(ts->pElementObject->Expression2))->m_NumElements-1,variable,VarPos,VarLen,alternative_variable,is_greek)) 
				return 1;
		}
		if (ts->pElementObject->Expression3)
		{
			if (((CExpression*)(ts->pElementObject->Expression3))->ContainsVariable(0,((CExpression*)(ts->pElementObject->Expression3))->m_NumElements-1,variable,VarPos,VarLen,alternative_variable,is_greek)) 
				return 1;
		}
	}

	return 0;
}

int CExpression::ExtractVariable(CExpression* variable, int VarPos, int VarLen, int ComputationLevel)
{

	//first check if the level is 'equality'
	int lvl=this->FindLowestOperatorLevel();
	if (lvl!=EqLevel) return 0;

	int Start1;
	int End1;
	int Start2;
	int End2;

	//now find the first part of the equation (Start1 ... End1)
	char et,p;
	int l=GetElementLen(0,m_NumElements-1,EqLevel,&et,&p);
	Start1=p;
	End1=l-1;
	if (End1>=m_NumElements) return 0;

	//find the last part of the equation (Start2 ... End2)
	int pos=l;
	while (1)
	{
		l=GetElementLen(pos,m_NumElements-1,EqLevel,&et,&p);
		if (l==0) return 0;

		Start2=pos+p;
		End2=pos+l-1;

		pos+=l;
		if (pos>m_NumElements-1) break;
	}

	if (ComputationLevel<1) ComputationLevel=1;

	if (ComputationLevel==1)  //first level handling - knowns to the right, unknowns to the left
	{		
		if (Start2-1>End1+1)
		{
			//there are several equations 'exp1=exp2=exp3'
			
			char c=(m_pElementList+End1+1)->pElementObject->Data1[0];
			for (int i=End1+1;i<Start2;i++)
			{
				if (((m_pElementList+i)->Type==2) && (GetOperatorLevel((m_pElementList+i)->pElementObject->Data1[0])==EqLevel))
				{
					const char equality[10][10]={
						//               non-eqaul   <=            <<         >=              >>        approx.
						{0,         '=',(char)0xB9,(char)0xA3,'<',(char)0x01,(char)0xB3,'>',(char)0x02,(char)0xBB,},
						{'=',       0,  1,         1,         1,  1,         1,         1,  1,         1},
						{(char)0xB9,0,  0,         0,         0,  0,         0,         0,  0,         0},
						{(char)0xA3,0,  1,         0,         1,  1,         2,         2,  2,         0},
						{'<',       0,  1,         0,         0,  1,         2,         2,  2,         0},
						{(char)0x01,0,  1,         0,         0,  0,         2,         2,  2,         0},
						{(char)0xB3,0,  1,         2,         2,  2,         0,         1,  1,         0},
						{'>',       0,  1,         2,         2,  2,         0,         0,  1,         0},
						{(char)0x02,0,  1,         2,         2,  2,         0,         0,  0,         0},
						{(char)0xBB,0,  1,         1,         1,  1,         1,         1,  1,         0}
					};
					char c2=(m_pElementList+i)->pElementObject->Data1[0];
					int l=1;
					while (l<10) 
						if (equality[l++][0]==c) break;
					int k=1;
					while (k<10)
						if (equality[0][k++]==c2) break;
					if ((k<10) && (l<10))
					{
						if (equality[k][l]==1) c=c2;
						if (equality[k][l]==2) return 0;
					}
				}
			}
			for (int i=End1+1;i<Start2;i++)
			{
				DeleteElement(End1+1);
			}

			InsertEmptyElement(End1+1,2,c);
			return 1;
		}


		//now we have only the simple equations A=B (or A<B or A<<B or A>C...)

		//go through all summands and calculate ones that contain the given variable

		int lvl1=FindLowestOperatorLevel(Start1,End1,'+');
		int lvl2=FindLowestOperatorLevel(Start2,End2,'+');

		if (lvl1>=PlusLevel)
		{
			//move knowns to right, and unknowns to left
			int pos=Start1;
			if ((End1>Start1) || (m_pElementList->Type!=1) || (strcmp(m_pElementList->pElementObject->Data1,"0")))
			while (1)
			{
				char p,et;
				int l=GetElementLen(pos,End1,PlusLevel,&et,&p);
				if (l==0) return 0;

				if (!ContainsVariable(pos+p,pos+l-1,variable,VarPos,VarLen))
				{
					if (et=='+') et='-';
					else if (et=='-') et='+';
					else if (et==(char)0xB2) et=(char)0xB1;
					else if (et==(char)0xB1) et=(char)0xB2;

					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->InsertEmptyElement(0,2,et);

					for (int ii=pos+p;ii<pos+l;ii++)
					{
						tmp->InsertElement(m_pElementList+ii,tmp->m_NumElements);
					}
					for (int ii=pos;ii<pos+l;ii++)
						{DeleteElement(pos);End1--;}
					if (End1<0) InsertEmptyElement(0,1,'0');

					double N;
					int prec;
					if ((!tmp->IsPureNumber(0,tmp->m_NumElements,&N,&prec)) || (fabs(N)>1e-100))
						for (int ii=0;ii<tmp->m_NumElements;ii++)
							InsertElement(tmp->m_pElementList+ii,m_NumElements);

					delete tmp;
					return 1;
				}
				pos+=l;
				if (pos>End1) break;
			}
		}

		if (lvl2>=PlusLevel)
		{

			int pos=Start2;
			if ((End2>Start2) || ((m_pElementList+Start2)->Type!=1) || (strcmp((m_pElementList+Start2)->pElementObject->Data1,"0")))
			while (1)
			{
				char p,et;
				int l=GetElementLen(pos,End2,PlusLevel,&et,&p);
				if (l==0) return 0;

				if (ContainsVariable(pos+p,pos+l-1,variable,VarPos,VarLen))
				{
					if (et=='+') et='-';
					else if (et=='-') et='+';
					else if (et==(char)0xB2) et=(char)0xB1;
					else if (et==(char)0xB1) et=(char)0xB2;
					
					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->InsertEmptyElement(0,2,et);

					for (int ii=pos+p;ii<pos+l;ii++)
					{
						tmp->InsertElement(m_pElementList+ii,tmp->m_NumElements);
					}
					for (int ii=pos;ii<pos+l;ii++)
						{DeleteElement(pos);End2--;}
					if (End2<Start2) InsertEmptyElement(Start2,1,'0');

					double N;
					int prec;
					if ((!tmp->IsPureNumber(0,tmp->m_NumElements,&N,&prec)) || (fabs(N)>1e-100))
						for (int ii=0;ii<tmp->m_NumElements;ii++)
							InsertElement(tmp->m_pElementList+ii,End1+1+ii);

					delete tmp;
					return 1;
				}
				pos+=l;
				if (pos>End2) break;
			}
		}

		//still here? - three is no more level 1 operations, increase levele to level 2
		return 2;
	}



	if (ComputationLevel==2) //the second level - divide
	{
		int lvl1=FindLowestOperatorLevel(Start1,End1,(char)0xD7);

		if (lvl1>=MulLevel)
		{
			int found_any=0;

			//prepare sturcture that will hold list of all variables in this expression
			tVariableList *VarList;
			int summand_no;
			VarList=(tVariableList*)this->GenerateVariableList(Start1,End1,&summand_no);

			if ((VarList) && (summand_no==1))
			{
				for (int i=0;i<VarList->NumVariables;i++)
				{
					VarList->Variables[i].summand[1]=VarList->Variables[i].summand[0];
					VarList->Variables[i].summand[2]=VarList->Variables[i].summand[0];
					int contains_variable=0;
					if ((VarList->Variables[i].variable->ContainsVariable(VarList->Variables[i].pos,VarList->Variables[i].pos+VarList->Variables[i].len-1,variable,VarPos,VarLen)) ||
						((VarList->Variables[i].exponent) && (VarList->Variables[i].exponent->ContainsVariable(0,VarList->Variables[i].exponent->m_NumElements-1,variable,VarPos,VarLen))))
						contains_variable=1;
					if ((contains_variable) && (VarList->Variables[i].summand[2].dorder>0))
					{
						VarList->Variables[i].summand[2].dorder=0;
					}
					else
					{
						VarList->Variables[i].summand[2].dorder=-VarList->Variables[i].summand[2].dorder;
						VarList->Variables[i].summand[2].style|=0x01;
						VarList->Variables[i].summand[1].dorder=0;
						found_any|=1;
						if (contains_variable) found_any|=2;
					}
				}

				VarList->Constants[2].N1=VarList->Constants[0].N2;
				VarList->Constants[2].N2=VarList->Constants[0].N1;

				if (fabs(VarList->Constants[0].N1/VarList->Constants[0].N2-1.0)>1e-100)
					found_any|=1;

				if (found_any)
				{
					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->InsertElement(m_pElementList+End1+1,0);
					tmp->InsertEmptyElement(1,5,'(');
					CExpression *a=(CExpression*)((tmp->m_pElementList+1)->pElementObject->Expression1);

					for (int i=Start2;i<=End2;i++)
						a->InsertElement(m_pElementList+i,a->m_NumElements);

					tmp->SynthetizeExpression(1,VarList,2);

					tmp->SynthetizeExpression(0,VarList,1);

					CopyExpression(tmp,0);
					delete tmp;
				}

			}

			//delete all temporary created objects
			this->FreeVariableList(VarList);

			if (found_any&0x02) return 1;
			if (found_any) return 2;
		}


		//still here - go to third level
		return 3;
	}



	if ((ComputationLevel==3) || (ComputationLevel==4))//the third level - inverse functions
	{
		//take care about leading sign
		if (((m_pElementList+Start1)->Type==2) && ((m_pElementList+Start1)->pElementObject->Data1[0]=='+'))
		{
			DeleteElement(Start1);
			return 3;
		}
		if (((m_pElementList+Start1)->Type==2) && ((m_pElementList+Start1)->pElementObject->Data1[0]=='-'))
		{
			InsertEmptyElement(Start2,5,'(');
			CExpression *tmp=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
			for (int ii=Start2+1;ii<=End2+1;ii++)
			{
				tmp->InsertElement(m_pElementList+Start2+1,tmp->m_NumElements);
				DeleteElement(Start2+1);
			}
			InsertEmptyElement(Start2,2,'-');

			for (int ii=Start1;ii<=End1;ii++)
			{
				if ((m_pElementList+ii)->Type==2)
				{
					char *ch=(m_pElementList+ii)->pElementObject->Data1;
					if (*ch=='+') *ch='-';
					else if (*ch=='-') *ch='+';
					else if (*ch==(char)0xB1) *ch=(char)0xB2;
					else if (*ch==(char)0xB2) *ch=(char)0xB1;
				}
			}
			return 3;
		}




		//check if there is only single function at the left side
		if (Start1==End1)
		{
			tElementStruct *ts=m_pElementList+Start1;

			if (ts->Type==6) //logarithm
			{
				int is_log=0;
				if (strcmp(ts->pElementObject->Data1,"log")==0) is_log=1;
				if (strcmp(ts->pElementObject->Data1,"ln")==0) is_log=2;
				if (is_log)
				{
					CExpression *b=(CExpression*)(ts->pElementObject->Expression2);
					CExpression *a=(CExpression*)(ts->pElementObject->Expression1);
					if ((a->ContainsVariable(0,a->m_NumElements-1,variable,VarPos,VarLen)) &&
						((b==NULL) || (!b->ContainsVariable(0,b->m_NumElements-1,variable,VarPos,VarLen))))
					{
						if (ComputationLevel==3) goto extract_variable_midstep_exit;

						if (b)
							InsertEmptyElement(Start2,3,0);
						else if (is_log==2)
							InsertEmptyElement(Start2,3,4);
						else
							InsertEmptyElement(Start2,3,5);
						CExpression *aa=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
						CExpression *ee=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression2);
						if (b) aa->CopyExpression(b,0);

						for (int ii=Start2+1;ii<=End2+1;ii++)
						{
							ee->InsertElement(m_pElementList+Start2+1,ee->m_NumElements);
							DeleteElement(Start2+1);
						}
						for (int ii=0;ii<a->m_NumElements;ii++)
							InsertElement(a->m_pElementList+ii,Start1+ii+1);
						DeleteElement(Start1);
						return 1;

					}
				}
			}

			if (ts->Type==8) //root
			{
				CExpression *b=(CExpression*)(ts->pElementObject->Expression2);
				CExpression *a=(CExpression*)(ts->pElementObject->Expression1);
				if ((a->ContainsVariable(0,a->m_NumElements-1,variable,VarPos,VarLen)) &&
					((b==NULL) || (!b->ContainsVariable(0,b->m_NumElements-1,variable,VarPos,VarLen))))
				{
					if (ComputationLevel==3) goto extract_variable_midstep_exit;

					InsertEmptyElement(Start2,3,0);
					CExpression *aa=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
					CExpression *ee=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression2);
					if (b==NULL)
						ee->InsertEmptyElement(0,1,'2');
					else
						ee->CopyExpression(b,0);

					for (int ii=Start2+1;ii<=End2+1;ii++)
					{
						aa->InsertElement(m_pElementList+Start2+1,aa->m_NumElements);
						DeleteElement(Start2+1);
					}
					for (int ii=0;ii<a->m_NumElements;ii++)
						InsertElement(a->m_pElementList+ii,Start1+ii+1);
					DeleteElement(Start1);
					return 1;

				}


			}
			if (ts->Type==4) //fraction
			{
				CExpression *d=(CExpression*)(ts->pElementObject->Expression2);
				CExpression *n=(CExpression*)(ts->pElementObject->Expression1);

				//variable is in denominator
				if ((d->ContainsVariable(0,d->m_NumElements-1,variable,VarPos,VarLen)) &&
					(!n->ContainsVariable(0,n->m_NumElements-1,variable,VarPos,VarLen)))
				{
					if (ComputationLevel==3) goto extract_variable_midstep_exit;

					InsertEmptyElement(Start2,4,0);
					CExpression *nn=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
					CExpression *dd=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression2);

					for (int ii=Start2+1;ii<=End2+1;ii++)
					{
						dd->InsertElement(m_pElementList+Start2+1,dd->m_NumElements);
						DeleteElement(Start2+1);
					}
					nn->CopyExpression(n,0);
					for (int ii=0;ii<d->m_NumElements;ii++)
						InsertElement(d->m_pElementList+ii,Start1+ii+1);
					DeleteElement(Start1);
					return 1;

				}
			}

			if (ts->Type==3) //exponent
			{
				CExpression *e=(CExpression*)(ts->pElementObject->Expression2);
				CExpression *a=(CExpression*)(ts->pElementObject->Expression1);

				if (e->ContainsVariable(0,a->m_NumElements-1,variable,VarPos,VarLen))
				{
					if (ComputationLevel==3) goto extract_variable_midstep_exit;

					int p;
					double N;
					int type=18;
					if ((a->IsPureNumber(0,a->m_NumElements,&N,&p)) && (fabs(N-10.0)<1e-100))
						type=19;
					if ((a->m_NumElements==1) && (a->m_pElementList->Type==1) && 
						(strcmp(a->m_pElementList->pElementObject->Data1,"e")==0) &&
						((a->m_pElementList->pElementObject->Data2[0]&0xE0)!=0x60))
						type=17;

					InsertEmptyElement(Start2,6,type);
					CExpression *tmp=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
					for (int ii=Start2+1;ii<=End2+1;ii++)
					{
						tmp->InsertElement(m_pElementList+Start2+1,tmp->m_NumElements);
						DeleteElement(Start2+1);
					}
					if (type==18)
					{
						CExpression *b=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression2);
						b->CopyExpression(a,0);
					}
					InsertEmptyElement(Start1,6,type);
					if (type==18)
					{
						CExpression *b=(CExpression*)((m_pElementList+Start1)->pElementObject->Expression2);
						b->CopyExpression(a,0);
					}
					tmp=(CExpression*)((m_pElementList+Start1)->pElementObject->Expression1);
					for (int ii=Start1+1;ii<=End1+1;ii++)
					{
						tmp->InsertElement(m_pElementList+Start1+1,tmp->m_NumElements);
						DeleteElement(Start1+1);
					}

					return 1;
				}
				if (a->ContainsVariable(0,a->m_NumElements-1,variable,VarPos,VarLen))
				{
					//if exponent is negative ( a^(-b) )
					if (e->FindLowestOperatorLevel((char)0xD7)==MulLevel)
					{
						if ((e->m_pElementList->Type==2) && (e->m_pElementList->pElementObject->Data1[0]=='-'))
						{
							if (ComputationLevel==3) goto extract_variable_midstep_exit;

							e->DeleteElement(0);
							InsertEmptyElement(Start2,4,0);
							CExpression *n=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
							CExpression *d=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression2);
							n->InsertEmptyElement(0,1,'1');
							for (int ii=Start2+1;ii<=End2+1;ii++)
							{
								d->InsertElement(m_pElementList+Start2+1,d->m_NumElements);
								DeleteElement(Start2+1);
							}
							return 1;
						}
					}

					int p;
					double N;
					if ((e->IsPureNumber(0,e->m_NumElements,&N,&p)) && (N>0) &&
						(fabs(N-(long long)N)<1e-100))
					{
						if (ComputationLevel==3) goto extract_variable_midstep_exit;

						if (fabs(N-2.0)<1e-100)
							InsertEmptyElement(Start2,8,1);
						else
						{
							InsertEmptyElement(Start2,8,0);
							((CExpression*)((m_pElementList+Start2)->pElementObject->Expression2))->CopyExpression(e,0);
						}
						CExpression *t=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
						for (int ii=Start2+1;ii<=End2+1;ii++)
						{
							t->InsertElement(m_pElementList+Start2+1,t->m_NumElements);
							DeleteElement(Start2+1);
						}
						for (int ii=0;ii<a->m_NumElements;ii++)
							InsertElement(a->m_pElementList+ii,Start1+ii+1);
						DeleteElement(Start1);
						return 1;

					}
					else
					{
						if (ComputationLevel==3) goto extract_variable_midstep_exit;

						InsertEmptyElement(Start2,3,0);
						CExpression *aa=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression1);
						CExpression *ee=(CExpression*)((m_pElementList+Start2)->pElementObject->Expression2);
						ee->InsertEmptyElement(0,4,0);
						CExpression *n=(CExpression*)((ee->m_pElementList)->pElementObject->Expression1);
						CExpression *d=(CExpression*)((ee->m_pElementList)->pElementObject->Expression2);
						n->InsertEmptyElement(0,1,'1');
						d->CopyExpression(e,0);

						for (int ii=Start2+1;ii<=End2+1;ii++)
						{
							aa->InsertElement(m_pElementList+Start2+1,aa->m_NumElements);
							DeleteElement(Start2+1);
						}
						for (int ii=0;ii<a->m_NumElements;ii++)
							InsertElement(a->m_pElementList+ii,Start1+ii+1);
						DeleteElement(Start1);
						return 1;
					}
				}
			}
		}

		//if (ComputationLevel==4)
		{
			int lvl1=FindLowestOperatorLevel(Start1,End1,(char)0xD7);
			if (lvl1<MulLevel)
			{
				CExpression *tmp=new CExpression(NULL,NULL,100);
				for (int ii=Start1;ii<=End1;ii++)
					tmp->InsertElement(m_pElementList+ii,tmp->m_NumElements);


				int was_done=0;
				if (tmp->FactorizeExpression())
				{
					was_done=1;
				}

				if (!was_done)
				{
					int cnt=0;
					while ((tmp->Compute(0,tmp->m_NumElements-1,1)) && (cnt<50)) cnt++;					
					if (tmp->FindLowestOperatorLevel((char)0xD7)>=MulLevel)
					{
						was_done=2;
					}
				}
				if (was_done)
				{
					//we managed to convert the left side into mutiplication level expression
					//now we will check if any of the multiplication factors doesn't contain the variable
					int summand_no;
					tVariableList *VarList=(tVariableList*)tmp->GenerateVariableList(0,tmp->m_NumElements-1,&summand_no);
					if ((VarList) /*&& (was_done==2)*/)  //removed 08.09.2010.
					{
						int kkk=0;
						if (was_done==1)
						if ((fabs(VarList->Constants->N1-1.0)<1e-100) &&
							(fabs(VarList->Constants->N2-1.0)<1e-100))
						for (kkk=0;kkk<VarList->NumVariables;kkk++)
						{
							if (VarList->Variables[kkk].variable->ContainsVariable(	VarList->Variables[kkk].pos,
								VarList->Variables[kkk].pos+VarList->Variables[kkk].len-1,
								variable,VarPos,VarLen)==0) break;

						}
						if (kkk<VarList->NumVariables)
						{
							tmp->FreeVariableList(VarList);
							for (int ii=Start1;ii<=End1;ii++) DeleteElement(0);

							for (int ii=0;ii<tmp->m_NumElements;ii++)
								InsertElement(tmp->m_pElementList+ii,Start1+ii);
							delete tmp;
							return 2;
						}
					}
					tmp->FreeVariableList(VarList);
				}
				
				delete tmp;
			}
			

			{
				pos=Start1;
				while (1)
				{
					char p,et;
					int l=GetElementLen(pos,End1,PlusLevel,&et,&p);
					if (l==0) return 0;

					if (ContainsVariable(pos+p,pos+l-1,variable,VarPos,VarLen))
					{
						int prev=m_NumElements;
						if (Compute(pos+p,pos+l-1,0)) 
						{
							//change sign if needed
							l+=m_NumElements-prev;
							if (et=='-')
							for (int ii=pos+p;ii<pos+l;ii++)
								if ((m_pElementList+ii)->Type==2)
								{
									if ((m_pElementList+ii)->pElementObject->Data1[0]=='-') ((m_pElementList+ii)->pElementObject->Data1[0])='+';
									else if ((m_pElementList+ii)->pElementObject->Data1[0]=='+') ((m_pElementList+ii)->pElementObject->Data1[0])='-';
									else if ((m_pElementList+ii)->pElementObject->Data1[0]==(char)0xB1) ((m_pElementList+ii)->pElementObject->Data1[0])=(char)0xB2;
									else if ((m_pElementList+ii)->pElementObject->Data1[0]==(char)0xB2) ((m_pElementList+ii)->pElementObject->Data1[0])=(char)0xB1;
								}
							return 1;
						}
					}
					pos+=l;
					if (pos>End1) break;
				}
			}
		}




		int cnt=0;
		while ((Compute(Start2,m_NumElements-1,3)) && (cnt<50)) cnt++;
		if (((m_pElementList+Start2)->Type==2) && ((m_pElementList+Start2)->pElementObject->Data1[0]=='+'))
		{
			DeleteElement(Start2);
		}
		//still here? - we finished
		return 0;
	}
	

extract_variable_midstep_exit:

	int cnt=0;
	while ((Compute(Start2,m_NumElements-1,3)) && (cnt<50)) cnt++;
	if (((m_pElementList+Start2)->Type==2) && ((m_pElementList+Start2)->pElementObject->Data1[0]=='+'))
	{
		DeleteElement(Start2);
	}
	return 4;
}

int CExpression::SynthetizeExpression(int Position, void * VList, int summand_no)
{
	int orgPosition=Position;
	tVariableList *VarList=(tVariableList*)VList;

	double N1=VarList->Constants[summand_no].N1;
	double N2=VarList->Constants[summand_no].N2;

	int anyfound=0;

	if (N1*N2<0)
	{
		this->InsertEmptyElement(Position++,2,'-');
		if (N1<0) N1=-N1;
		if (N2<0) N2=-N2;
	}
	else if (Position)
	{
		if (((this->m_pElementList+Position-1)->Type!=2) && 
			((this->m_pElementList+Position-1)->Type!=11) && 
			((this->m_pElementList+Position-1)->Type!=12))
			this->InsertEmptyElement(Position++,2,'+');
	}

	CExpression *numerator=NULL,*denominator=NULL;

	if (fabs(N2-1.0)>1e-100)
	{
		this->InsertEmptyElement(Position++,4,0);
		numerator=(CExpression*)((this->m_pElementList+Position-1)->pElementObject->Expression1);
		denominator=(CExpression*)((this->m_pElementList+Position-1)->pElementObject->Expression2);
		numerator->GenerateASCIINumber(N1,(long long)(N1+((N1>0)?0.01:-0.01)),(fabs(N1-(long long)N1)<1e-100)?1:0,VarList->Constants[summand_no].prec,0);
		denominator->GenerateASCIINumber(N2,(long long)(N2+((N2>0)?0.01:-0.01)),(fabs(N2-(long long)N2)<1e-100)?1:0,VarList->Constants[summand_no].prec,0);
		anyfound=1;
	}
	else if (fabs(N1-1.0)>1e-100)
	{
		this->GenerateASCIINumber(N1,(long long)(N1+((N1>0)?0.01:-0.01)),(fabs(N1-(long long)N1)<1e-100)?1:0,VarList->Constants[summand_no].prec,Position++);
		anyfound=1;
	}

	for (int i=0;i<VarList->NumVariables;i++)
		if (fabs(VarList->Variables[i].summand[summand_no].dorder)>1e-100)
		{
			CExpression *dest=this;
			int tmpPos=0;
			int *destpos;
			destpos=&Position;

			double ord=VarList->Variables[i].summand[summand_no].dorder;
			if (VarList->Variables[i].summand[summand_no].style&0x01)
			{
				//quick check if there is anything for the denominator
				int fndit=0;
				for (int j=0;j<VarList->NumVariables;j++)
					if (VarList->Variables[j].summand[summand_no].dorder<0) {fndit=1;break;}

				if (fndit)
				{
					if (numerator==NULL)
					{
						this->InsertEmptyElement(Position++,4,0);
						numerator=(CExpression*)((this->m_pElementList+Position-1)->pElementObject->Expression1);
						denominator=(CExpression*)((this->m_pElementList+Position-1)->pElementObject->Expression2);
					}
					destpos=&tmpPos;
					if (ord>=0)
						dest=numerator;
					else
						{dest=denominator;ord=-ord;}
				}
			}
			if ((VarList->Variables[i].summand[summand_no].style&0x02) && (ord<0))
			{
				//if (dest->m_pElementList->Type==0) dest->InsertEmptyElement(0,1,'1');
				dest->InsertEmptyElement(*destpos,2,'/');(*destpos)++;
				ord=-ord;
			}
			if ((fabs(ord-1.0)>1e-100) || (VarList->Variables[i].exponent))
			{
				dest->InsertEmptyElement(*destpos,3,0);(*destpos)++;
				CExpression *aa=(CExpression*)((dest->m_pElementList+(*destpos)-1)->pElementObject->Expression1);
				CExpression *ee=(CExpression*)((dest->m_pElementList+(*destpos)-1)->pElementObject->Expression2);
				for (int kk=0;kk<VarList->Variables[i].len;kk++)
					aa->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,aa->m_NumElements);
				if (VarList->Variables[i].exponent)
					ee->CopyExpression(VarList->Variables[i].exponent,0);
				if (fabs(ord-1.0)>1e-100)
				{
					int pp=0;
					if ((ee->m_pElementList->Type==2) && (GetOperatorLevel(ee->m_pElementList->pElementObject->Data1[0])==PlusLevel))
						pp=1;
					ee->GenerateASCIINumber((double)ord,(int)(ord+((ord>0)?0.01:-0.01)),(fabs(ord-(int)ord)<1e-100)?1:0,0,pp);
				}
				anyfound=1;
			}
			else
			{
				CExpression *var=VarList->Variables[i].variable;
				if (var->FindLowestOperatorLevel(VarList->Variables[i].pos,VarList->Variables[i].pos+VarList->Variables[i].len-1,(char)0xD7)<MulLevel)
				{
					dest->InsertEmptyElement((*destpos),5,'(');(*destpos)++;
					CExpression *aa=(CExpression*)((dest->m_pElementList+(*destpos)-1)->pElementObject->Expression1);
					for (int kk=0;kk<VarList->Variables[i].len;kk++)
						aa->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,aa->m_NumElements);

				}
				else
					for (int kk=0;kk<VarList->Variables[i].len;kk++)
					{
						dest->InsertElement(VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos+kk,*destpos);
						(*destpos)++;
					}
				anyfound=1;
			}
		}

	if ((numerator) && (numerator->m_pElementList->Type==0)) numerator->InsertEmptyElement(0,1,'1');
	if ((denominator) && (denominator->m_pElementList->Type==0)) denominator->InsertEmptyElement(0,1,'1');

	if (anyfound==0)
		this->InsertEmptyElement(Position++,1,'1');

	return Position-orgPosition;
}

int CExpression::ComputeMatrixDeterminant(int ComputationType)
{
	int rows,cols;


	if (ComputationType!=2) return 0;

	if (!this->GetMatrixSize(&rows,&cols)) return 0;
	if (rows!=cols) return 0;
	if (rows<2) return 0;
	

	if (rows==2)
	{
		//2x2 determinant;
		CExpression *a=new CExpression(NULL,NULL,100);

		a->InsertEmptyElement(0,5,'(');
		CExpression *aa=(CExpression*)(a->m_pElementList->pElementObject->Expression1);

		int pos=FindMatrixElement(0,0,0);
		while ((pos<m_NumElements) && ((m_pElementList+pos)->Type<11))
		{
			aa->InsertElement(m_pElementList+pos,aa->m_NumElements);
			pos++;
		}
		a->InsertEmptyElement(1,5,'(');
		aa=(CExpression*)((a->m_pElementList+1)->pElementObject->Expression1);
		pos=FindMatrixElement(1,1,0);
		while ((pos<m_NumElements) && ((m_pElementList+pos)->Type<11))
		{
			aa->InsertElement(m_pElementList+pos,aa->m_NumElements);
			pos++;
		}
		a->InsertEmptyElement(2,2,'-');
		a->InsertEmptyElement(3,5,'(');
		aa=(CExpression*)((a->m_pElementList+3)->pElementObject->Expression1);
		pos=FindMatrixElement(0,1,0);
		while ((pos<m_NumElements) && ((m_pElementList+pos)->Type<11))
		{
			aa->InsertElement(m_pElementList+pos,aa->m_NumElements);
			pos++;
		}
		a->InsertEmptyElement(4,5,'(');
		aa=(CExpression*)((a->m_pElementList+4)->pElementObject->Expression1);
		pos=FindMatrixElement(1,0,0);
		while ((pos<m_NumElements) && ((m_pElementList+pos)->Type<11))
		{
			aa->InsertElement(m_pElementList+pos,aa->m_NumElements);
			pos++;
		}

		CopyExpression(a,0);this->m_ParenthesesFlags=0x02;this->m_ParentheseShape='(';
		delete a;
		return 1;
	}


	CExpression *a=new CExpression(NULL,NULL,100);
	for (int i=0;i<rows;i++)
	{

		if (i%2) 
			a->InsertEmptyElement(a->m_NumElements,2,'-');
		else if (i)
			a->InsertEmptyElement(a->m_NumElements,2,'+');

		a->InsertEmptyElement(a->m_NumElements,5,'(');
		CExpression *aa=(CExpression*)((a->m_pElementList+a->m_NumElements-1)->pElementObject->Expression1);
		int pos=FindMatrixElement(0,i,0);
		while ((pos<m_NumElements) && ((m_pElementList+pos)->Type<11))
		{
			aa->InsertElement((m_pElementList+pos),aa->m_NumElements);
			pos++;
		}

		a->InsertEmptyElement(a->m_NumElements,5,'|');
		aa=(CExpression*)((a->m_pElementList+a->m_NumElements-1)->pElementObject->Expression1);
		for (int ii=1;ii<rows;ii++)
		{
			for (int jj=0;jj<cols;jj++)
			{
				if (jj!=i)
				{
					if ((aa->m_pElementList->Type) && ((aa->m_pElementList+aa->m_NumElements-1)->Type!=12))
						aa->InsertEmptyElement(aa->m_NumElements,11,0);
					pos=FindMatrixElement(ii,jj,0);
					while ((pos<m_NumElements) && ((m_pElementList+pos)->Type<11))
					{
						aa->InsertElement((m_pElementList+pos),aa->m_NumElements);
						pos++;
					}
				}
			}
			if (ii<rows-1) aa->InsertEmptyElement(aa->m_NumElements,12,0);
		}

	}

	CopyExpression(a,0);this->m_ParenthesesFlags=0x02;this->m_ParentheseShape='(';
	delete a;
	return 1;	


	return 0;
}

//calculates the invert matrix
int CExpression::MatrixInvert(int ComputationType)
{
	int rows;
	int columns;
	if (!GetMatrixSize(&rows,&columns)) return 0;

	if ((rows<=0) || (columns<=0)) return 0;
	if ((rows+columns)<=2) return 0;

	if (ComputationType!=2)
		if (rows!=columns) return 0;

	//only for round or square parentheses (vertical, both brackets)
	if ((this->m_ParentheseShape!='[') && (this->m_ParentheseShape!='(')) return 0;
	if ((this->m_ParenthesesFlags&0x1C)!=0) return 0;


	if (rows==columns)
	{
		CExpression *det=new CExpression(NULL,NULL,100);
		det->CopyExpression(this,0);
		det->m_ParentheseShape='|';
		//det->m_ParentheseData=0;
		det->m_ParenthesesFlags=0x03;

		det->ComputeMatrixDeterminant(2);

		int cntr=0;
		while ((det->Compute(0,det->m_NumElements-1,2)) &&  (cntr<50)) cntr++;

		CExpression *C=new CExpression(NULL,NULL,100);

		for (int i=0;i<rows;i++)
		{
			if (i!=0) C->InsertEmptyElement(C->m_NumElements,12,0);
			for (int j=0;j<columns;j++)			
			{
				if (j!=0) C->InsertEmptyElement(C->m_NumElements,11,0);
				if ((i+j)%2) C->InsertEmptyElement(C->m_NumElements,2,'-');
				C->InsertEmptyElement(C->m_NumElements,4,0);
				CExpression *n=(CExpression*)((C->m_pElementList+C->m_NumElements-1)->pElementObject->Expression1);
				CExpression *d=(CExpression*)((C->m_pElementList+C->m_NumElements-1)->pElementObject->Expression2);
				n->InsertEmptyElement(0,5,(columns>2)?'|':'(');
				CExpression *subdet=(CExpression*)(n->m_pElementList->pElementObject->Expression1);
				d->CopyExpression(det,0);
				int first_row=1;
				for (int ii=0;ii<rows;ii++)
				if (ii!=j)
				{
					if (!first_row) {subdet->InsertEmptyElement(subdet->m_NumElements,12,0);first_row=0;}
					int first_col=1;
					for (int jj=0;jj<columns;jj++)
						if ((ii!=j) && (jj!=i))
						{
							if (!first_col) {subdet->InsertEmptyElement(subdet->m_NumElements,11,0);first_col=0;}
							first_col=0;
							first_row=0;
							int pos=this->FindMatrixElement(ii,jj,0);
							char et,p;
							int l=this->GetElementLen(pos,this->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
							for (int k=pos;k<pos+l;k++)
								subdet->InsertElement(this->m_pElementList+k,subdet->m_NumElements);
						}
				}

				cntr=0;

				while ((n->Compute(0,n->m_NumElements-1,2)) && (cntr<50)) cntr++;

			}
		}

		cntr=0;
		while ((C->Compute(0,C->m_NumElements-1,2)) && (cntr<50)) cntr++;




		int ps=this->m_ParentheseShape;
		//int ph=this->m_ParentheseHeightFactor;
		//int pd=this->m_ParentheseData;
		int fp=this->m_ParenthesesFlags;
		CopyExpression(C,0);
		this->m_ParentheseShape=ps;
		//this->m_ParentheseHeightFactor=ph;
		//this->m_ParentheseData=pd;
		this->m_ParenthesesFlags=fp;
		delete C;
		delete det;

		return 1;
	}

	/*
	//create copy of this matrix
	CExpression *M=new CExpression(NULL,NULL,100);
	M->CopyExpression(this,0);

	//expand matrix M with diagonal matrix I
	for (int i=0;i<rows;i++)
	{
		int pos=M->m_NumElements;
		if (i<rows-1) pos=M->FindMatrixElement(i+1,0)-1;

		for (int j=0;j<columns;j++)
		{
			M->InsertEmptyElement(pos++,11,0);
			if (i==j)
				M->InsertEmptyElement(pos++,1,'1');
			else
				M->InsertEmptyElement(pos++,1,'0');
		}
	}
	columns*=2;


	for (int i=0;i<rows;i++)
	{
		//check if the diagonal element is non-zero, if it is zero then exchange rows
		int pos=M->FindMatrixElement(i,i);
		if (pos<0) goto matrix_invert_end;

		char et;
		char p;
		int len=M->GetElementLen(pos,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
		if (len<=0) goto matrix_invert_end;

		int prec;
		double N;
		if ((M->IsPureNumber(pos,len,&N,&prec)) && (fabs(N)<1e-100))
		{
			//yes, the diagonal element is zero - we will try to find a row that has non-zero
			//element at this column, and exchange these two rows

			int ii;
			for (ii=i+1;ii<rows;ii++)
			{
				int pos2=M->FindMatrixElement(ii,i);
				if (pos2<0) goto matrix_invert_end;
				int len2=M->GetElementLen(pos2,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
				if (len2<=0) goto matrix_invert_end;
				if ((!M->IsPureNumber(pos2,len2,&N,&prec)) || (fabs(N)>1e-100))
					break;
			}
			if (ii<rows)
			{
				//exchange rows
				CExpression *tmpa=new CExpression(NULL,NULL,100);
				CExpression *tmpb=new CExpression(NULL,NULL,100);

				int posa=M->FindMatrixElement(i,0);
				for (int jj=posa;jj<M->m_NumElements;jj++)
				{
					tmpa->InsertElement((M->m_pElementList+posa),tmpa->m_NumElements);
					M->DeleteElement(posa);
					if ((M->m_pElementList+posa)->Type==12) break;
				}

				int posb=M->FindMatrixElement(ii-1,0);
				for (int jj=posb;jj<M->m_NumElements;jj++)
				{
					tmpb->InsertElement((M->m_pElementList+posb),tmpb->m_NumElements);
					M->DeleteElement(posb);
					if ((M->m_pElementList+posb)->Type==12) break;
				}

				for (int jj=0;jj<tmpa->m_NumElements;jj++)
					M->InsertElement(tmpa->m_pElementList+jj,posb++);

				for (int jj=0;jj<tmpb->m_NumElements;jj++)
					M->InsertElement(tmpb->m_pElementList+jj,posa++);

				delete tmpa;
				delete tmpb;
			}
			else
			{
				if (ComputationType!=2) goto matrix_invert_end;
				goto matrix_invert_finish;
			}
		}

		//check if the diagonal element is one, if it is not one then divide the row by the diagonal element
		pos=M->FindMatrixElement(i,i);
		if (pos<0) goto matrix_invert_end;

		len=M->GetElementLen(pos,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
		if (len<=0) goto matrix_invert_end;

		if ((!M->IsPureNumber(pos,len,&N,&prec)) || (fabs(N-1.0)>1e-100))
		{
			//the diagonal element is non-one, so we will divide the row whith it

			//store the diagonal element into the 'tmp' variable
			CExpression *tmp=new CExpression(NULL,NULL,100);

			for (int jj=pos;jj<pos+len;jj++)
				tmp->InsertElement(M->m_pElementList+jj,tmp->m_NumElements);

			for (int jj=0;jj<columns;jj++)
			{
				int pos2=M->FindMatrixElement(i,jj);
				if (pos2<0) goto matrix_invert_end;
				int len2=M->GetElementLen(pos2,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
				if (len2<=0) goto matrix_invert_end;

				if (jj==i)
				{
					for (int kk=pos2;kk<pos2+len2;kk++)
						M->DeleteElement(pos2);
					M->InsertEmptyElement(pos2,1,'1');
				}
				else if ((len2!=1) || ((M->m_pElementList+pos2)->Type!=1) || 
					(strcmp((M->m_pElementList+pos2)->pElementObject->Data1,"0")))
				{

					M->InsertEmptyElement(pos2,4,0);
					CExpression *n=(CExpression*)((M->m_pElementList+pos2)->pElementObject->Expression1);
					CExpression *d=(CExpression*)((M->m_pElementList+pos2)->pElementObject->Expression2);

					for (int kk=pos2+1;kk<pos2+1+len2;kk++)
					{
						n->InsertElement((M->m_pElementList+pos2+1),n->m_NumElements);
						M->DeleteElement(pos2+1);
					}
					d->CopyExpression(tmp,0);
					int cntr=0;
					int len3,pos3;
					do 
					{
						cntr++;
						pos3=M->FindMatrixElement(i,jj);
						len3=M->GetElementLen(pos3,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
					} while ((M->Compute(pos3,pos3+len3-1,1)) && (cntr<50));
				}
			}

			delete tmp;
		}


		//finally go down-column and try to set every member to zero (by
		//substracting the current row multiplied with member)


		for (int j=0;j<rows;j++)
		{
			if (j!=i)
			{
				int pos2=M->FindMatrixElement(j,i);
				if (pos2<0) goto matrix_invert_end;
				int len2=M->GetElementLen(pos2,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
				if (len2<=0) goto matrix_invert_end;

				if ((!M->IsPureNumber(pos2,len2,&N,&prec)) || (fabs(N)>1e-100))
				{
					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->InsertEmptyElement(0,5,'(');
					CExpression *a=(CExpression*)(tmp->m_pElementList->pElementObject->Expression1);
					for (int kk=pos2;kk<pos2+len2;kk++)
					{
						a->InsertElement(M->m_pElementList+pos2,a->m_NumElements);
						M->DeleteElement(pos2);
					}
					M->InsertEmptyElement(pos2,1,'0');

					for (int k=i+1;k<columns;k++)
					{
						tmp->InsertEmptyElement(0,5,'(');
						a=(CExpression*)(tmp->m_pElementList->pElementObject->Expression1);
						int pos3=M->FindMatrixElement(i,k);
						int len3=M->GetElementLen(pos3,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
						for (int kk=pos3;kk<pos3+len3;kk++)
							a->InsertElement(M->m_pElementList+kk,a->m_NumElements);

						pos3=M->FindMatrixElement(j,k);
						len3=M->GetElementLen(pos3,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
						M->InsertEmptyElement(pos3+len3,2,'-');
						M->InsertElement(tmp->m_pElementList,pos3+len3+1);
						M->InsertElement(tmp->m_pElementList+1,pos3+len3+2);
						tmp->DeleteElement(0);

						int cntr=0;
						do 
						{
							cntr++;
							pos3=M->FindMatrixElement(j,k);
							len3=M->GetElementLen(pos3,M->m_NumElements-1,GetOperatorLevel((char)0xFF),&et,&p);
						} while ((M->Compute(pos3,pos3+len3-1,2)) && (cntr<50));
					
					}
					delete tmp;
				}

			}
		}
	}

matrix_invert_finish:

	//delete the diagonal I matrix (now on left side)
	columns/=2;
	for (i=0;i<rows;i++)
	{
		int pos=M->FindMatrixElement(i,0);
		int j=columns;
		while (j)
		{
			if ((M->m_pElementList+pos)->Type==11) j--;
			M->DeleteElement(pos);
		}
	}

	int ps=this->m_ParentheseShape;
	int hf=this->m_ParentheseHeightFactor;
	this->CopyExpression(M,0);
	this->m_ParentheseShape=ps;
	this->m_ParentheseHeightFactor=hf;
	delete M;
	return 1;

matrix_invert_end:
	delete M;

	*/


	return 0;
}

// //returns zero if matrix is non-regular (different number of columns per row)
int CExpression::GetMatrixSize(int* rows, int* columns)
{
	*rows=1;
	*columns=1;
	int retval=1;
	int first_columns=0;
	tElementStruct *ts=m_pElementList;
	for (int ii=0;ii<m_NumElements;ii++,ts++)
	{
		//tElementStruct *ts=m_pElementList+ii;
		if (ts->Type==11) (*columns)++;
		if ((ts->Type==12) && (ii<m_NumElements-1))
		{
			(*rows)++;
			if ((first_columns) && (first_columns!=(*columns))) retval=0;
			first_columns=*columns;
			*columns=1;
		}
	}
	return retval;
}


int CExpression::ComputeParentheses(int Position, char element_type, int ComputationType,int Level)
{
	int retval=0;

	CExpression *argument=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);

	if (((argument->m_ParentheseShape=='|') || (argument->m_ParentheseShape=='\\')) && ((argument->m_ParenthesesFlags&0x1C)==0))
	if (argument->FindLowestOperatorLevel()==GetOperatorLevel((char)0xFF))
	{
		//determinant		
		return argument->ComputeMatrixDeterminant(ComputationType);
		
	}

	//we are not considering certain types of parentheses
	if ((argument->m_ParentheseShape=='/') || (argument->m_ParentheseShape=='<') ||
		(argument->m_ParentheseShape=='b') || (argument->m_ParentheseShape=='x')) return 0;
	
	//also, not horizontal parentheses
	if (argument->m_ParenthesesFlags&0x04) return 0;

	//calculate parenthese content
	int cntr=0;
	while ((argument->Compute(0,argument->m_NumElements-1,ComputationType)) && (cntr<50)) cntr++;
	if (cntr>0) retval=-1;

	//for parentheses where single bracket is missing, we are finished
	if ((argument->m_ParenthesesFlags&0x1C)!=0) return retval;



	//finally, we are going to remove brackets if it is possible

	int InsideLevel=argument->FindLowestOperatorLevel();

	if ((InsideLevel==-1) || (InsideLevel>PlusLevel))
	{
		//if minus sign is inside, like: (-a)
		if ((argument->m_pElementList->Type==2) && (argument->m_pElementList->pElementObject->Data1[0]=='-') &&
			(argument->m_ParentheseShape!='c') && (argument->m_ParentheseShape!='f') && (argument->m_ParentheseShape!='|'))
		{
			argument->DeleteElement(0);
			if ((argument->m_ParentheseShape=='|') || (argument->m_ParentheseShape=='\\')) return 1; //the absolute value
			int p=Position;
			do
			{
				if ((m_pElementList+p)->Type==11) {InsertEmptyElement(p+1,2,'-');return 1;}
				if ((m_pElementList+p)->Type==12) {InsertEmptyElement(p+1,2,'-');return 1;}
				if ((m_pElementList+p)->Type==2)
				{
					char *ch=(m_pElementList+p)->pElementObject->Data1;
					int ll=GetOperatorLevel(*ch);
					if (ll<PlusLevel) {InsertEmptyElement(p+1,2,'-');return 1;}
					if (ll==PlusLevel) 
					{
						if (*ch=='+') *ch='-';
						else if (*ch=='-') *ch='+';
						else if (*ch==(char)0xB1) *ch=(char)0xB2;
						else if (*ch==(char)0xB2) *ch=(char)0xB1;
						return 1;
					}
				}
				if (p==0) {InsertEmptyElement(p,2,'-');return 1;}
				p--;
			} while (1);
		}

		//if the plus sign is inside, like (+a)
		if ((argument->m_pElementList->Type==2) && (argument->m_pElementList->pElementObject->Data1[0]=='+'))
		{
			argument->DeleteElement(0);
			retval=1;
		}
	}


	//for absolute-value we go no further
	if ((argument->m_ParentheseShape=='|') || (argument->m_ParentheseShape=='\\'))
	{
		if ((InsideLevel<MulLevel) && (InsideLevel!=-1)) return 1;

		//check if there is pure number factor inside. if yes, take it out
		tPureFactors PF;
		PF.N1=1.0;PF.N2=1.0;
		PF.is_frac1=0;
		PF.prec1=0;
		int ret=argument->StrikeoutCommonFactors(0,argument->m_NumElements-1,1,NULL,0,0,1,&PF);
	
		if ((fabs(PF.N1-1.0)>1e-100) || (fabs(PF.N2-1.0)>1e-100))
		{
			argument->StrikeoutRemove(0,argument->m_NumElements-1,2);
			PF.N1=fabs(PF.N1);
			PF.N2=fabs(PF.N2);
			ReduceTwoNumbers(&PF.N1,&PF.N2);
			this->GenerateASCIIFraction(Position,PF.N1,PF.N2,PF.prec1,PF.is_frac1);
			return 1;

		}
		argument->StrikeoutRemove(0,argument->m_NumElements-1);
		return retval;
	}

	//ceiling and floor functions
	if ((argument->m_ParentheseShape=='c') || (argument->m_ParentheseShape=='f'))
	{
		if ((InsideLevel<MulLevel) && (InsideLevel!=-1)) return 1;

		double N=1.0;
		int p=0;
		if (argument->IsPureNumber(0,argument->m_NumElements,&N,&p))
		{
			if (argument->m_ParentheseShape=='c')
				N=ceil(N);//(int)((N>0)?(N+1):(N));
			else
				N=floor(N);//(int)((N>0)?(N):(N-1));
			DeleteElement(Position);
			this->GenerateASCIINumber(N,(int)N,1,p,Position);
			return 1;
		}
	}

	//we can drop the parentheses if the level inside is higher than the outside level
	{
		if (InsideLevel==-1) InsideLevel=Level;

		if ((InsideLevel>Level) || 
			((InsideLevel==Level) && (element_type!='/') && (element_type!='-')))
		if (argument->m_pElementList->Type!=2)
		{
			CElement *element=(m_pElementList+Position)->pElementObject;
			int num_units=argument->m_NumElements;
			m_NumElements+=num_units-1;
			if (m_ElementListReservations<m_NumElements)
			{
				m_ElementListReservations=m_NumElements;
				m_pElementList=(tElementStruct*)HeapReAlloc(ProcessHeap,0,(void*)m_pElementList,m_ElementListReservations*sizeof(tElementStruct));
				if (m_pElementList==NULL) {AfxMessageBox("Cannot alloc memory (3)!!!",MB_OK,0);return 0;}
			}
			memmove(m_pElementList+Position+num_units-1,
				m_pElementList+Position,
				(m_NumElements-Position-num_units+1)*sizeof(tElementStruct));
			for (int ii=0;ii<argument->m_NumElements;ii++)
			{
				if ((argument->m_pElementList+ii)->pElementObject)
				{
					(argument->m_pElementList+ii)->pElementObject->m_pPaternalExpression=(CObject*)this;
					if ((argument->m_pElementList+ii)->pElementObject->Expression1)
						((CExpression*)((argument->m_pElementList+ii)->pElementObject->Expression1))->m_pPaternalExpression=this;
					if ((argument->m_pElementList+ii)->pElementObject->Expression2)
						((CExpression*)((argument->m_pElementList+ii)->pElementObject->Expression2))->m_pPaternalExpression=this;
					if ((argument->m_pElementList+ii)->pElementObject->Expression3)
						((CExpression*)((argument->m_pElementList+ii)->pElementObject->Expression3))->m_pPaternalExpression=this;
				}
				*(m_pElementList+ii+Position)=*(argument->m_pElementList+ii);
			}
			((CExpression*)(element->Expression1))->m_NumElements=0; //to suppres deletion of sub-elements
			delete element;
			this->m_IsComputed=0;
			return 1;
		}

		if ((InsideLevel>Level) ||
			((InsideLevel==Level) && (Level>=PlusLevel) && ((ComputationType!=1) || (argument->m_NumElements==1))))
		{
			int pos=0;
			int jj=1;
			while (1)
			{
				char et,p;
				int l=argument->GetElementLen(pos,argument->m_NumElements-1,InsideLevel,&et,&p);
				if (l==0) break;

				if (element_type==(char)0xD7)
				{
					if (p) InsertEmptyElement(Position+(jj++),2,et);
				}
				else if (element_type=='/')
				{
					if (et==(char)0xD7) InsertEmptyElement(Position+(jj++),2,'/');
				}
				else if (element_type=='+')
				{
					if (p) InsertEmptyElement(Position+(jj++),2,et);
				}
				else if (element_type=='-')
				{
					if (et=='+') InsertEmptyElement(Position+(jj++),2,'-');
					else if (et=='-') InsertEmptyElement(Position+(jj++),2,'+');
					else if (et==(char)0xB1) InsertEmptyElement(Position+(jj++),2,(char)0xB2); //plus-minus -> minus-plus
					else if (et==(char)0xB2) InsertEmptyElement(Position+(jj++),2,(char)0xB1);
					else if (p) InsertEmptyElement(Position+(jj++),2,et);
				}
				else
					InsertElement(argument->m_pElementList+pos,Position+(jj++));

				for (int kk=pos+p;kk<pos+l;kk++)
				{
					InsertElement(argument->m_pElementList+kk,Position+jj);
					jj++;
				}


				pos+=l;
				if (pos>argument->m_NumElements-1) break;
			}
			
			DeleteElement(Position); //delete the parenthese element

			if (Position>0)
				if ((m_pElementList+Position-1)->Type==2)
				if ((m_pElementList+Position-1)->pElementObject->Data1[0]==element_type)
					if ((m_pElementList+Position)->Type==2)
						DeleteElement(Position-1);

			return 1;
		}
	}

	return retval;
}



int CExpression::GenerateASCIIFraction(int Position,double num, double denom, int precision , int is_fraction)
{
	if ((is_fraction) && (fabs(denom)>1e-100))
	{
		InsertEmptyElement(Position,4,0);
		CExpression *n=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
		CExpression *d=(CExpression*)((m_pElementList+Position)->pElementObject->Expression2);

		n->GenerateASCIINumber(num,(long long)(num+(num>0)?0.01:-0.01),(fabs(num-(long long)num)<1e-100)?1:0,precision,0);
		d->GenerateASCIINumber(denom,(long long)(denom+(denom>0)?0.01:-0.01),(fabs(denom-(long long)denom)<1e-100)?1:0,precision,0);
	}
	else 
	{
		int level=PlusLevel;
		if (Position)
		{
			tElementStruct *ts=m_pElementList+Position-1;
			if ((ts->Type!=11) && (ts->Type!=12) && (ts->Type!=0) &&
				((ts->Type!=2) || (GetOperatorLevel(ts->pElementObject->Data1[0])>=MulLevel)))
				level=MulLevel;
		}
		if (Position<m_NumElements)
		{
			tElementStruct *ts=m_pElementList+Position;
			if ((ts->Type!=11) && (ts->Type!=12) && (ts->Type!=0) &&
				((ts->Type!=2) || (GetOperatorLevel(ts->pElementObject->Data1[0])>=MulLevel)))
				level=MulLevel;
		}

		if ((fabs(num-1.0)>1e-100) || (level<MulLevel))
			GenerateASCIINumber(num,(long long)(num+(num>0)?0.01:-0.01),(fabs(num-(long long)num)<1e-100)?1:0,precision,Position);
		if (fabs(denom-1.0)>1e-100)
		{
			//serch for the last position
			while (1)
			{
				if (Position>=m_NumElements) {Position=m_NumElements;break;}
				if ((this->m_pElementList+Position)->Type==11) break;
				if ((this->m_pElementList+Position)->Type==12) break;
				if (((this->m_pElementList+Position)->Type==2) &&
					(GetOperatorLevel((this->m_pElementList+Position)->pElementObject->Data1[0])<MulLevel))
					break;
				Position++;
			}
			if ((Position==0) || 
				((m_pElementList+Position-1)->Type==11) ||
				((m_pElementList+Position-1)->Type==12) ||
				(((m_pElementList+Position-1)->Type==2) && (GetOperatorLevel((m_pElementList+Position-1)->pElementObject->Data1[0])<MulLevel)))
				InsertEmptyElement(Position++,1,'1');
			
			InsertEmptyElement(Position++,2,'/');
			GenerateASCIINumber(denom,(long long)(denom+(denom>0)?0.01:-0.01),(fabs(denom-(long long)denom)<1e-100)?1:0,precision,Position);
		}
	}

	return 0;
}

int CExpression::ComputeSinCos(int Position, char element_type, int ComputationType)
{
	int retval=0;
	if (Position<0) return 0;
	if (Position>m_NumElements-1) return 0;
	if ((m_pElementList+Position)->Type!=6) return 0;

	int function_type=0;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"sin")==0) function_type=1;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"cos")==0) function_type=2;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"tg")==0) function_type=3;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"tan")==0) function_type=3;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"cot")==0) function_type=4;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"ctg")==0) function_type=4;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"sec")==0) function_type=5;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"cosec")==0) function_type=6;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"csc")==0) function_type=6;

	CExpression *arg=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
	if (!arg) return 0;

	{
		int cnt=0;
		while ((arg->Compute(0,arg->m_NumElements-1,(ComputationType==10)?10:0)) && (cnt<50)) cnt++;
		if (cnt) retval=-1;
	}

	int lvl=arg->FindLowestOperatorLevel((char)0xD7);
	if (lvl>=MulLevel)
	{
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;PF.prec1=0;
		arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
		if (PF.N1*PF.N2<0)
		{
			arg->StrikeoutRemove(0,arg->m_NumElements-1,2);
			if ((arg->m_NumElements==1) && (arg->m_pElementList->Type==1) && (strcmp(arg->m_pElementList->pElementObject->Data1,"1")==0))
				arg->DeleteElement(0); //if the StrikeoutRemove left only '1' then we are deleting it
			arg->GenerateASCIIFraction(0,-PF.N1,PF.N2,PF.prec1,PF.is_frac1);
			if ((function_type==1) || (function_type==3) || (function_type==4))
			{
				InsertEmptyElement(Position,5,'(');
				CExpression *a=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
				a->InsertEmptyElement(0,2,'-');
				a->InsertElement(m_pElementList+Position+1,1);
				DeleteElement(Position+1);
			}
			return 1;
		}
		else
			arg->StrikeoutRemove(0,arg->m_NumElements-1);
	}

	if (lvl>=PlusLevel)
	{

		//prepare sturcture that will hold list of all variables in this expression
		tVariableList *VarList;
		VarList=(tVariableList*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,sizeof(tVariableList));
		for (int i=0;i<MAX_SUMMANDS;i++) VarList->Constants[i].N1=VarList->Constants[i].N2=1.0;

		//create list of all variables (run through all summands and call 'ExtractVariables')
		int pos=0;
		int summand_no=0;
		while (1)
		{
			char et,p;
			int l=arg->GetElementLen(pos,arg->m_NumElements-1,PlusLevel,&et,&p);
			if (l==0) break;

			if (arg->ContainsVariable(pos+p,pos+l-1,NULL,0,0,"p",1))
			{
				if (et=='-') VarList->Constants[summand_no].N1=-VarList->Constants[summand_no].N1;
				ExtractVariablesMode=1;
				arg->ExtractVariables(pos+p,pos+l-1,1.0,summand_no,VarList);
				ExtractVariablesMode=0;
			}

			VarList->Constants[summand_no].summand_pos=pos;

			summand_no++;
			if (summand_no>=MAX_SUMMANDS) break;

			pos+=l;
			if (pos>arg->m_NumElements-1) break;
		}

		if ((VarList) && (VarList->NumVariables))
		{
			for (int i=0;i<VarList->NumVariables;i++)
			{
				int len=VarList->Variables[i].len;
				int pos=VarList->Variables[i].pos;
				CExpression *var=VarList->Variables[i].variable;
				if ((VarList->Variables[i].exponent==NULL) && (len==1) && 
					((var->m_pElementList+pos)->Type==1) &&
					(strcmp((var->m_pElementList+pos)->pElementObject->Data1,"p")==0) &&
					(((var->m_pElementList+pos)->pElementObject->Data2[0]&0xE0)==0x60))
				{
					int done=0;

					//this variable is 'pi' - go throught summands and find where order is 1
					for (int j=0;j<summand_no;j++)
					{
						if (fabs(VarList->Variables[i].summand[j].dorder-1.00)<1e-100)
						{
							//now check that this summand has no other variable than this 'pi'
							int k;
							for (k=0;k<VarList->NumVariables;k++)
							if (k!=i)
							{
								if (fabs(VarList->Variables[k].summand[j].dorder)>1e-100) 
									break;
							}

							if (k==VarList->NumVariables)
							{
								//yes, we found a summand that only has 'pi' and constants

								double N1=VarList->Constants[j].N1;					
								double N2=VarList->Constants[j].N2;

								if (summand_no==1)
								{
									int up=-1,down;
									int is_sqrt=0;
									int is_minus=0;
									double N=N1;
									if ((function_type==1) && (fabs(N1/N2)<1+1e-100))
									{
										if (N<0) {is_minus=1;N=-N;}
										if (N/N2>0.5+1e-100) N=N2-N;
										if (fabs(N/N2-1.0/6.0)<1e-100) {up=1;down=2;}
										if (fabs(N/N2-1.0/4.0)<1e-100) {up=2;down=2;is_sqrt=1;}
										if (fabs(N/N2-1.0/3.0)<1e-100) {up=3;down=2;is_sqrt=1;}
										if (fabs(N/N2-1.0/2.0)<1e-100) {up=1;down=1;}
									}
									if ((function_type==2) && (fabs(N1/N2)<1+1e-100))
									{
										if (N<0) {N=-N;}
										if (N/N2>0.5+1e-100) {N=N2-N;is_minus=1;}
										if (fabs(N/N2-1.0/6.0)<1e-100) {up=3;down=2;is_sqrt=1;}
										if (fabs(N/N2-1.0/4.0)<1e-100) {up=2;down=2;is_sqrt=1;}
										if (fabs(N/N2-1.0/3.0)<1e-100) {up=1;down=2;}
										if (fabs(N/N2-1.0/2.0)<1e-100) {up=0;down=1;}
									}
									if ((function_type==3) && (fabs(N1/N2)<1+1e-100))
									{
										if (N<0) {N=-N;is_minus=1;}
										if (N/N2>0.5+1e-100) {N=N2-N;is_minus=!is_minus;}
										if (fabs(N/N2-1.0/6.0)<1e-100) {up=1;down=3;is_sqrt=1;}
										if (fabs(N/N2-1.0/4.0)<1e-100) {up=1;down=1;}
										if (fabs(N/N2-1.0/3.0)<1e-100) {up=3;down=1;is_sqrt=1;}
										//if (fabs(N/N2-1.0/2.0)<1e-100) {up=0;down=1;}
									}
									if ((function_type==4) && (fabs(N1/N2)<1+1e-100))
									{
										if (N<0) {N=-N;is_minus=1;}
										if (N/N2>0.5+1e-100) {N=N2-N;is_minus=!is_minus;}
										if (fabs(N/N2-1.0/6.0)<1e-100) {up=3;down=1;is_sqrt=1;}
										if (fabs(N/N2-1.0/4.0)<1e-100) {up=1;down=1;}
										if (fabs(N/N2-1.0/3.0)<1e-100) {up=1;down=3;is_sqrt=1;}
										if (fabs(N/N2-1.0/2.0)<1e-100) {up=0;down=1;}
									}
									if (up>=0)
									{
										InsertEmptyElement(Position,4,0);
										CExpression *n=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
										CExpression *d=(CExpression*)((m_pElementList+Position)->pElementObject->Expression2);
										if (is_minus)
											n->InsertEmptyElement(0,2,'-');
										if ((is_sqrt) && (up>1))
										{
											n->InsertEmptyElement(n->m_NumElements,8,1);
											CExpression *a=(CExpression*)((n->m_pElementList+n->m_NumElements-1)->pElementObject->Expression1);
											a->InsertEmptyElement(0,1,'0'+up);
										}
										else
											n->InsertEmptyElement(n->m_NumElements,1,'0'+up);
										if ((is_sqrt) && (up==1))
										{
											d->InsertEmptyElement(d->m_NumElements,8,1);
											CExpression *a=(CExpression*)((d->m_pElementList+n->m_NumElements-1)->pElementObject->Expression1);
											a->InsertEmptyElement(0,1,'0'+down);
										}
										else
											d->InsertEmptyElement(0,1,'0'+down);
										DeleteElement(Position+1);
										retval=1;
										goto computesincos_end;
									}
								}

								if (function_type)
								{
									int changed=0;
									int sign=0;
									double step=2.0*fabs(N2);
									if (fabs(N1)>2*step)
									{
										double tmp=((long long)fabs(N1/step)-1)*step;
										if (N1>0) N1-=tmp; else N1+=tmp;
										changed=1;
									}

									while (N1>=step-1e-100) {N1-=step;changed=1;}
									while (N1<=-step+1e-100) {N1+=step;changed=1;}
									step/=2.0;
									while (N1>=step-1e-100) {N1-=step;changed=1;sign=1;}
									while (N1<=-step+1e-100) {N1+=step;changed=1;sign=1;}
									if ((function_type==3) || (function_type==4)) sign=0;

									if (changed)
									{
										if (fabs(N1)<1e-100)
										{
											int pos2=VarList->Constants[j].summand_pos;
											arg->RemoveSequence(PlusLevel,pos2);
											if (arg->m_pElementList->Type==0)
												arg->InsertEmptyElement(0,1,'0');
											retval=-1;
											done=1;
										}
										else 
										{
											if (N1+N2<0) 
												arg->InsertEmptyElement(arg->m_NumElements,2,'-');
											else
												arg->InsertEmptyElement(arg->m_NumElements,2,'+');
											arg->GenerateASCIIFraction(arg->m_NumElements,fabs(N1),fabs(N2),VarList->Constants[j].prec,(fabs(N2-1.0)>0)?1:0);
											arg->InsertEmptyElement(arg->m_NumElements,1,5);

											int pos2=VarList->Constants[j].summand_pos;
											arg->RemoveSequence(PlusLevel,pos2);
											retval=-1;
											done=1;
										}
										if (sign)
										{
											InsertEmptyElement(Position,5,'(');
											CExpression *a=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
											a->InsertEmptyElement(0,2,'-');
											a->InsertElement(m_pElementList+Position+1,1);
											DeleteElement(Position+1);
											retval=1;
											goto computesincos_end;
										}
									}
								}
							}
						}
						if (done) break;
					}
					if (done) break;
				}
			}
computesincos_end:
			for (int i=0;i<MAX_VARIABLES;i++)
				if (VarList->Variables[i].exponent)
					delete VarList->Variables[i].exponent;
			HeapFree(ProcessHeap,0,VarList);

			if (retval>0) return 1;
		}
		else
			HeapFree(ProcessHeap,0,VarList);

	}


	if (lvl>=MulLevel)
	{
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;PF.prec1=0;
		int rval=arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
		if (rval)
		{
			double N=PF.N1/PF.N2;
			int prec=PF.prec1;
			{
				if (fabs(N)<1e-100)
				{
					//the argument is pure zero
					if ((function_type==1) || (function_type==3)) //sinus or tangens
					{
						InsertEmptyElement(Position,1,'0');
						DeleteElement(Position+1);
						return 1;
					}
					if ((function_type==2) || (function_type==5)) //cosinus, secant
					{
						InsertEmptyElement(Position,1,'1');
						DeleteElement(Position+1);
						return 1;
					}
				}

				double res;
				if (function_type)
				{
					int is_ok=0;
					if (function_type==1) {res=sin(N);is_ok=1;}
					if (function_type==2) {res=cos(N);is_ok=1;}
					if (function_type==3) {res=tan(N);is_ok=1;}
					if ((function_type==4) && (fabs(N)>1e-100)) {res=1.0/tan(N);is_ok=1;}
					if (function_type==5) {res=1.0/cos(N);is_ok=1;} //secant
					if (function_type==6) {res=1.0/sin(N);is_ok=1;} //cosecant

					if (is_ok)
					{
						if (prec<2) prec=2;
						DeleteElement(Position);
						GenerateASCIINumber(res,(long long)(res+((res>0)?0.01:-0.01)),(fabs(res-(long long)res)<1e-100)?1:0,prec,Position);
						return 1;
					}
				}
			}
		}
	}

	return retval;
}


int CExpression::ComputeASinCos(int Position, char element_type, int ComputationType)
{
	int retval=0;
	if (Position<0) return 0;
	if (Position>m_NumElements-1) return 0;
	if ((m_pElementList+Position)->Type!=6) return 0;

	int function_type=0;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arcsin")==0) function_type=1;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arccos")==0) function_type=2;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arctg")==0) function_type=3;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arctan")==0) function_type=3;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arccot")==0) function_type=4;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arcctg")==0) function_type=4;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arcsec")==0) function_type=5;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"arccsc")==0) function_type=6;

	CExpression *arg=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
	if (!arg) return 0;

	{
		int cnt=0;
		while ((arg->Compute(0,arg->m_NumElements-1,(ComputationType==10)?10:0)) && (cnt<50)) cnt++;
		if (cnt) retval=-1;
	}

	int lvl=arg->FindLowestOperatorLevel((char)0xD7);

	if (lvl>=MulLevel)
	{
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;PF.prec1=0;
		int rval=arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
		if (rval)
		{
			double N=PF.N1/PF.N2;
			int prec=PF.prec1;
			{
				if ((fabs(N)>1.00) && (function_type<=2))
					return 0; //arccos, arcsin not defined
				if ((fabs(N)<1.00) && ((function_type==5) || (function_type==6)))
					return 0; //arcsec, arccsc not defined

				double res;
				if (function_type)
				{

					int is_ok=0;
					if (function_type==1) {res=asin(N);is_ok=1;} //arcsin
					if (function_type==2) {res=acos(N);is_ok=1;} //arccos
					if (function_type==3) {res=atan(N);is_ok=1;} //arctan
					if (function_type==4) {res=atan(1/N);is_ok=1;} //arccot
					if (function_type==5) {res=acos(1/N);is_ok=1;} //arcsecant
					if (function_type==6) {res=asin(1/N);is_ok=1;} //arccsc
					if (is_ok)
					{
						if (prec<2) prec=2;
						DeleteElement(Position);
						GenerateASCIINumber(res,(long long)(res+((res>0)?0.01:-0.01)),(fabs(res-(long long)res)<1e-100)?1:0,prec,Position);
						return 1;
					}
				}
			}
		}
	}

	return retval;
}


int CExpression::ComputeHSinCos(int Position, char element_type, int ComputationType)
{
	int retval=0;
	if (Position<0) return 0;
	if (Position>m_NumElements-1) return 0;
	if ((m_pElementList+Position)->Type!=6) return 0;

	int function_type=0;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"sh")==0) function_type=1;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"ch")==0) function_type=2;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"th")==0) function_type=3;
	if (strcmp((m_pElementList+Position)->pElementObject->Data1,"cth")==0) function_type=4;

	CExpression *arg=(CExpression*)((m_pElementList+Position)->pElementObject->Expression1);
	if (!arg) return 0;

	{
		int cnt=0;
		while ((arg->Compute(0,arg->m_NumElements-1,(ComputationType==10)?10:0)) && (cnt<50)) cnt++;
		if (cnt) retval=-1;
	}

	int lvl=arg->FindLowestOperatorLevel((char)0xD7);

	if (lvl>=MulLevel)
	{
		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=0;PF.prec1=0;
		int rval=arg->StrikeoutCommonFactors(0,arg->m_NumElements-1,1,NULL,0,0,1,&PF);
		if (rval)
		{
			double N=PF.N1/PF.N2;
			int prec=PF.prec1;
			{
				/*if (fabs(N)<1e-100)
				{
					//the argument is pure zero
					if ((function_type==1) || (function_type==3)) //arcsinus or tangens
					{
						InsertEmptyElement(Position,1,'0');
						DeleteElement(Position+1);
						return 1;
					}
					if ((function_type==2) || (function_type==5)) //cosinus, secant
					{
						InsertEmptyElement(Position,1,'1');
						DeleteElement(Position+1);
						return 1;
					}
				}*/

				double res;
				if (function_type)
				{

					int is_ok=0;
					if (function_type==1) {res=sinh(N);is_ok=1;} //arcsin
					if (function_type==2) {res=cosh(N);is_ok=1;} //arccos
					if (function_type==3) {res=tanh(N);is_ok=1;} //arctan
					if (function_type==4) {res=1/tanh(N);is_ok=1;} //arccot
					if (is_ok)
					{
						if (prec<2) prec=2;
						DeleteElement(Position);
						GenerateASCIINumber(res,(long long)(res+((res>0)?0.01:-0.01)),(fabs(res-(long long)res)<1e-100)?1:0,prec,Position);
						return 1;
					}
				}
			}
		}
	}

	return retval;
}


//derivates the expression with the given variable
int CExpression::Derivate(CExpression * variable, int internal_call)
{
	//if not function of the 'variable' then the derivation is zero
	if (ContainsVariable(0,m_NumElements-1,variable,0,variable->m_NumElements)==0)
	{
		Delete();
		InsertEmptyElement(0,1,'0');
		return 1;
	}

	int level=FindLowestOperatorLevel();
	if (level==-1)
	{
		int pos=0;
		int minus_sign=0;
		if ((m_pElementList->Type==2) && (GetOperatorLevel(m_pElementList->pElementObject->Data1[0])==PlusLevel))
		{
			if (m_pElementList->pElementObject->Data1[0]=='-') minus_sign=1;
			pos=1;
		}

		if (m_NumElements-pos==1)
		{
			//single element

			tElementStruct *ts=m_pElementList+pos;

			if ((ts->Type==1) && (CompareExpressions(pos,pos,variable,0,variable->m_NumElements-1)))
			{
				//derivation of the 'variable' (dx/dx)
				Delete();
				InsertEmptyElement(0,1,'1');
				return 1;
			}

			if (ts->Type==3)
			{
				CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
				CExpression *exp=(CExpression*)(ts->pElementObject->Expression2);

				if (exp->ContainsVariable(0,exp->m_NumElements-1,variable,0,variable->m_NumElements)==0)
				{
					//derivation of exponent f(x)^y

					InsertEmptyElement(pos,5,'(');
					CExpression *a2=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
					a2->CopyExpression(exp,0); a2->m_ParenthesesFlags=0x02;
					exp->InsertEmptyElement(exp->m_NumElements,2,'-');
					exp->InsertEmptyElement(exp->m_NumElements,1,'1');
					CExpression *tmp=new CExpression(NULL,NULL,100);
					tmp->CopyExpression(arg,0);
					tmp->Derivate(variable,1);
					InsertEmptyElement(m_NumElements,5,'(');
					CExpression *a=(CExpression*)((m_pElementList+m_NumElements-1)->pElementObject->Expression1);
					a->CopyExpression(tmp,0); a->m_ParenthesesFlags=0x02;
					delete tmp;
					return 1;
				}

			}
			if (ts->Type==4)
			{
				//derivation of fraction
				CExpression *n=(CExpression*)(ts->pElementObject->Expression1);
				CExpression *d=(CExpression*)(ts->pElementObject->Expression2);

				InsertEmptyElement(pos,4,0);
				CExpression *n2=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
				CExpression *d2=(CExpression*)((m_pElementList+pos)->pElementObject->Expression2);
				n2->InsertEmptyElement(0,5,'(');
				CExpression *a1=(CExpression*)((n2->m_pElementList+0)->pElementObject->Expression1);
				n2->InsertEmptyElement(1,5,'(');
				CExpression *a2=(CExpression*)((n2->m_pElementList+1)->pElementObject->Expression1);
				n2->InsertEmptyElement(2,2,'-');
				n2->InsertEmptyElement(3,5,'(');
				CExpression *a3=(CExpression*)((n2->m_pElementList+3)->pElementObject->Expression1);
				n2->InsertEmptyElement(4,5,'(');
				CExpression *a4=(CExpression*)((n2->m_pElementList+4)->pElementObject->Expression1);
				d2->InsertEmptyElement(0,3,'2'); //square
				CExpression *a5=(CExpression*)((d2->m_pElementList+0)->pElementObject->Expression1);
				a5->CopyExpression(d,0); a5->m_ParenthesesFlags=0x02;
				a2->CopyExpression(d,0); a2->m_ParenthesesFlags=0x02;
				a4->CopyExpression(n,0); a4->m_ParenthesesFlags=0x02;

				CExpression *tmp=new CExpression(NULL,NULL,100);

				tmp->CopyExpression(n,0);
				tmp->Derivate(variable,1);
				a1->CopyExpression(tmp,0); a1->m_ParenthesesFlags=0x02;
				tmp->CopyExpression(d,0);
				tmp->Derivate(variable,1);
				a3->CopyExpression(tmp,0); a3->m_ParenthesesFlags=0x02;

				DeleteElement(pos+1);
				delete tmp;
				return 1;
			}

			if (ts->Type==5)
			{
				//derivation of parentheses
				CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
				arg->Derivate(variable,1);
				return 1;
			}

			if (ts->Type==6)
			{
				//derivation of function
				int done=0;
				CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);
				CExpression *tmp=new CExpression(NULL,NULL,100);
				tmp->CopyExpression(arg,0);
				tmp->Derivate(variable,1);

				if ((ts->pElementObject->Data2[0]&0xE0)!=0x60) //not greek font
				{
					if ((strnicmp(ts->pElementObject->Data1,"log",3)==0) ||
						(strnicmp(ts->pElementObject->Data1,"ln",2)==0))
					{
						CExpression *base=(CExpression*)(ts->pElementObject->Expression2);
						int is_base_null=0;
						if (base==NULL)
						{
							is_base_null=1;
							base=new CExpression(NULL,NULL,100);
							if (strnicmp(ts->pElementObject->Data1,"ln",2)==0)
								base->InsertEmptyElement(0,1,4);  //the 'e' (natural number)
							else
							{
								base->InsertEmptyElement(0,1,0);
								strcpy(base->m_pElementList->pElementObject->Data1,"10");
								base->m_pElementList->pElementObject->Data2[1]=base->m_pElementList->pElementObject->Data2[0];
							}
						}
						InsertEmptyElement(pos,4,0);
						CExpression *n=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
						CExpression *d=(CExpression*)((m_pElementList+pos)->pElementObject->Expression2);
						n->InsertEmptyElement(0,6,17); //ln  (the natural logarithm)
						CExpression *a1=(CExpression*)(n->m_pElementList->pElementObject->Expression1);
						a1->CopyExpression(base,0); a1->m_ParenthesesFlags=0x02;
						d->CopyExpression(arg,0); d->m_ParenthesesFlags=0x02;
						DeleteElement(pos+1);

						if (is_base_null) delete base;
						done=1;
					}
					else if (strnicmp(ts->pElementObject->Data1,"sin",3)==0)
					{
						InsertEmptyElement(pos,6,2); //cos
						CExpression *a1=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
						a1->CopyExpression(arg,0); a1->m_ParenthesesFlags=0x02;
						DeleteElement(pos+1);
						done=1;
					}
					else if (strnicmp(ts->pElementObject->Data1,"cos",3)==0)
					{
						InsertEmptyElement(pos,5,'(');
						CExpression *a1=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
						a1->InsertEmptyElement(1,2,'-');
						a1->InsertEmptyElement(1,6,1); //sin
						CExpression *a3=(CExpression*)((a1->m_pElementList+1)->pElementObject->Expression1);
						a3->CopyExpression(arg,0); a3->m_ParenthesesFlags=0x02;
						DeleteElement(pos+1);
						done=1;
					}
					else if ((strnicmp(ts->pElementObject->Data1,"tg",2)==0) ||
						(strnicmp(ts->pElementObject->Data1,"tan",3)==0))
					{
						InsertEmptyElement(pos,5,'(');
						CExpression *a1=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
						a1->InsertEmptyElement(1,2,'-');
						a1->InsertEmptyElement(1,6,4); //ctg
						CExpression *a3=(CExpression*)((a1->m_pElementList+1)->pElementObject->Expression1);
						a3->CopyExpression(arg,0); a3->m_ParenthesesFlags=0x02;
						DeleteElement(pos+1);
						done=1;
					}
					else if (strnicmp(ts->pElementObject->Data1,"cot",3)==0)
					{
						InsertEmptyElement(pos,5,'(');
						CExpression *a1=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
						a1->InsertEmptyElement(1,2,'-');
						a1->InsertEmptyElement(1,6,3); //tg
						CExpression *a3=(CExpression*)((a1->m_pElementList+1)->pElementObject->Expression1);
						a3->CopyExpression(arg,0); a3->m_ParenthesesFlags=0x02;
						DeleteElement(pos+1);
						done=1;
					}
				}
				if (done)
				{
					InsertEmptyElement(pos,5,'(');
					CExpression *a2=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
					a2->CopyExpression(tmp,0); a2->m_ParenthesesFlags=0x02;
				}
				delete tmp;
				if (done) return 1;
			}

			if (ts->Type==8)
			{
				//derivation of root
				CExpression *base=(CExpression*)(ts->pElementObject->Expression2);
				CExpression *arg=(CExpression*)(ts->pElementObject->Expression1);

				if ((base==NULL) || (base->ContainsVariable(0,base->m_NumElements-1,variable,0,variable->m_NumElements)==0))
				{
					int is_base_null=0;
					if (base==NULL)
					{
						is_base_null=1;
						base=new CExpression(NULL,NULL,100);
						base->InsertEmptyElement(0,1,'2');
					}
					CExpression *tmp=new CExpression(NULL,NULL,100);
					
					InsertEmptyElement(pos,4,0);
					CExpression *n=(CExpression*)((m_pElementList+pos)->pElementObject->Expression1);
					CExpression *d=(CExpression*)((m_pElementList+pos)->pElementObject->Expression2);
					tmp->CopyExpression(arg,0);
					tmp->Derivate(variable,1);
					n->CopyExpression(tmp,0);

					d->InsertEmptyElement(0,5,'(');
					CExpression *a1=(CExpression*)((d->m_pElementList+0)->pElementObject->Expression1);
					a1->CopyExpression(base,0); a1->m_ParenthesesFlags=0x02;
					if (is_base_null)
						d->InsertEmptyElement(1,8,1);
					else
						d->InsertEmptyElement(1,8,0);
					CExpression *a2=(CExpression*)((d->m_pElementList+1)->pElementObject->Expression1);
					CExpression *a3=(CExpression*)((d->m_pElementList+1)->pElementObject->Expression2);
					if (a3)
					{
						a3->CopyExpression(base,0);
					}
					a2->InsertEmptyElement(0,3,0);
					CExpression *a4=(CExpression*)(a2->m_pElementList->pElementObject->Expression1);
					CExpression *a5=(CExpression*)(a2->m_pElementList->pElementObject->Expression2);
					a4->CopyExpression(arg,0); a4->m_ParenthesesFlags=0x02;
					a5->CopyExpression(base,0);
					a5->InsertEmptyElement(a5->m_NumElements,2,'-');
					a5->InsertEmptyElement(a5->m_NumElements,1,'1');

					if (is_base_null) delete base;
					delete tmp;
					DeleteElement(pos+1);
					return 1;
				}
			}
		}
	}
	else if (level==MulLevel)
	{
		char et,p;
		int l=GetElementLen(0,m_NumElements-1,MulLevel,&et,&p);
		CExpression *tmp1=new CExpression(NULL,NULL,100);
		CExpression *tmp2=new CExpression(NULL,NULL,100);
		for (int k=0;k<l;k++)
			tmp1->InsertElement(m_pElementList+k,tmp1->m_NumElements);
		for (int k=l;k<m_NumElements;k++)
			tmp2->InsertElement(m_pElementList+k,tmp2->m_NumElements);
		
		Delete();
		InsertEmptyElement(0,5,'(');
		CExpression *a1=(CExpression*)((m_pElementList+0)->pElementObject->Expression1);
		InsertEmptyElement(1,5,'(');
		CExpression *a2=(CExpression*)((m_pElementList+1)->pElementObject->Expression1);
		InsertEmptyElement(2,2,'+');
		InsertEmptyElement(3,5,'(');
		CExpression *a3=(CExpression*)((m_pElementList+3)->pElementObject->Expression1);
		InsertEmptyElement(4,5,'(');
		CExpression *a4=(CExpression*)((m_pElementList+4)->pElementObject->Expression1);

		a2->CopyExpression(tmp2,0); a2->m_ParenthesesFlags=0x02;
		a3->CopyExpression(tmp1,0); a3->m_ParenthesesFlags=0x02;

		tmp1->Derivate(variable,1);
		tmp2->Derivate(variable,1);

		a1->CopyExpression(tmp1,0); a1->m_ParenthesesFlags=0x02;
		a4->CopyExpression(tmp2,0); a4->m_ParenthesesFlags=0x02;

		int ccc=m_NumElements;
		for (int k=5;k<ccc;k++)
			DeleteElement(5);
		
		delete tmp1;
		delete tmp2;
		return 1;
	}
	else if (level==PlusLevel)
	{
		CExpression *res=new CExpression(NULL,NULL,100);

		int pos=0;
		while (1)
		{
			char et,p;
			int l=GetElementLen(pos,m_NumElements-1,level,&et,&p);
			if (l==0) break;

			CExpression *tmp=new CExpression(NULL,NULL,100);
			for (int k=pos+p;k<pos+l;k++)
				tmp->InsertElement(m_pElementList+k,tmp->m_NumElements);

			tmp->Derivate(variable,1);
			if (p)
				res->InsertEmptyElement(res->m_NumElements,2,et);
			res->InsertEmptyElement(res->m_NumElements,5,'(');
			CExpression *a=(CExpression*)((res->m_pElementList+res->m_NumElements-1)->pElementObject->Expression1);
			a->CopyExpression(tmp,0); a->m_ParenthesesFlags=0x02;

			delete tmp;

			pos+=l;
			if (pos>m_NumElements-1) break;
		}

		CopyExpression(res,0);
		delete res;
		return 1;
	}

	if (!internal_call) return 0;

	InsertEmptyElement(0,4,1); //derivation
	CExpression *n=(CExpression *)(m_pElementList->pElementObject->Expression1);
	CExpression *d=(CExpression *)(m_pElementList->pElementObject->Expression2);
	CExpression *narg=(CExpression *)(n->m_pElementList->pElementObject->Expression1);
	CExpression *darg=(CExpression *)(d->m_pElementList->pElementObject->Expression1);
	int ccc=m_NumElements;
	for (int kk=1;kk<ccc;kk++)
	{
		narg->InsertElement(m_pElementList+1,narg->m_NumElements);
		DeleteElement(1);
	}
	darg->CopyExpression(variable,0); darg->m_ParenthesesFlags=0x02;

	return 1;
}


//Functions and structures to support function plotting
typedef struct
{
	int NumPositions;
	struct
	{
		CExpression *expr;
		short position;
		char variable_type;
	} positions[50];
	int starting_point;  //where the right side of expression actually starts
	char equation_type; //type of equation ('=', '<', '>'...)
} tVariablePositions;

CRITICAL_SECTION section3;
int wwwwxxxx=0;
int CExpression::PlotterGetEquationInfo(CExpression **variable,int *position,int *starting_point,char *equation_type)
{
	//this function gets the variable out


	int sp=0;
	if ((equation_type) && (starting_point))
	{
		*equation_type='=';
		*starting_point=0;
		int level=this->FindLowestOperatorLevel('+');
		if (level==EqLevel)
		{
			char et,p;
			int l=GetElementLen(0,m_NumElements-1,level,&et,&p);
			if (l==0) {return 0;}
			if (p==1) l=0; //in the case the equation starts with '='  (example:  =x^2)
			if (l>=m_NumElements-1) {return 0;}
			*starting_point=l+1;
			*equation_type=et;
		}
		else if (level<PlusLevel)
			{return 0;}
		sp=*starting_point;
	}

	if ((variable==NULL) || (position==NULL)) {return 1;}

	for (int i=sp;i<m_NumElements;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if (ts->Type==1)
		{
			double N;
			int prec;
			if (ts->pElementObject->Data1[0])
			if (!IsPureNumber(i,1,&N,&prec))
			{
				if ((ts->pElementObject->Data1[1]!=0) ||
					((ts->pElementObject->Data1[0]!='e') && ((ts->pElementObject->Data2[0]&0xE0)!=0x60)) ||
					((ts->pElementObject->Data1[0]!='p') && ((ts->pElementObject->Data2[0]&0xE0)==0x60)))
				{
					*position=i;
					*variable=this;
					return 1;
				}
			}
		}
		if ((ts->Type!=0) && (ts->Type!=2) && (ts->Type!=11) && (ts->Type!=12))
		{
			if (ts->pElementObject->Expression1)
				if (((CExpression*)(ts->pElementObject->Expression1))->PlotterGetEquationInfo(variable,position,NULL,NULL)) {return 1;}
			if (ts->pElementObject->Expression2)
				if (((CExpression*)(ts->pElementObject->Expression2))->PlotterGetEquationInfo(variable,position,NULL,NULL)) {return 1;}
			if (ts->pElementObject->Expression3)
				if (((CExpression*)(ts->pElementObject->Expression3))->PlotterGetEquationInfo(variable,position,NULL,NULL)) {return 1;}
		}
	}

	return 0;
}

void *CExpression::PlotterPrepareVariablePositions(void *VP)
{
	if (wwwwxxxx==0) {InitializeCriticalSection(&section3);wwwwxxxx=1;}
	int cntrr=0;
	while (!TryEnterCriticalSection(&section3))
	{
		Sleep(10);cntrr++;if (cntrr>300) {return NULL;}		
	}
	tVariablePositions *VarPos=(tVariablePositions*)VP;

	try
	{
		int starting_point=0;
		if (VarPos==NULL)
		{
			//check the structure of expression
			char type;
			if (!PlotterGetEquationInfo(NULL,NULL,&starting_point,&type)) {LeaveCriticalSection(&section3);return NULL;}

			//reserve memory
			//VarPos=(tVariablePositions*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,sizeof(tVariablePositions));
			VarPos=new tVariablePositions; memset(VarPos,0,sizeof(tVariablePositions));
			VarPos->NumPositions=0;

			//store the original (will be needed during calculation)
			//VarPos->original=new CExpression(NULL,NULL,100);
			//VarPos->original->CopyExpression(this,0);
			VarPos->starting_point=starting_point;
			VarPos->equation_type=type;
		}

		for (int i=starting_point;i<m_NumElements;i++)
		{
			tElementStruct *ts=m_pElementList+i;
			if (ts->Type==1)
			{
				if ((ts->pElementObject->Data1[1]==0) && (ts->pElementObject->Data1[0]=='e') && ((ts->pElementObject->Data2[0]&0xE0)!=0x60))
				{
					VarPos->positions[VarPos->NumPositions].position=i;
					VarPos->positions[VarPos->NumPositions].expr=this;
					VarPos->positions[VarPos->NumPositions].variable_type=1;
					if (VarPos->NumPositions<50) VarPos->NumPositions++;
				}
				else if ((ts->pElementObject->Data1[1]==0) && (ts->pElementObject->Data1[0]=='p') && ((ts->pElementObject->Data2[0]&0xE0)==0x60))
				{
					VarPos->positions[VarPos->NumPositions].position=i;
					VarPos->positions[VarPos->NumPositions].expr=this;
					VarPos->positions[VarPos->NumPositions].variable_type=2;
					if (VarPos->NumPositions<50) VarPos->NumPositions++;
				}
				else
				{
					double N;
					int prec;
					if (!IsPureNumber(i,1,&N,&prec))
					{
						VarPos->positions[VarPos->NumPositions].position=i;
						VarPos->positions[VarPos->NumPositions].expr=this;
						VarPos->positions[VarPos->NumPositions].variable_type=0;
						if (VarPos->NumPositions<50) VarPos->NumPositions++;
					}
				}
			}
			if ((ts->Type!=0) && (ts->Type!=2) && (ts->Type!=11) && (ts->Type!=12))
			{
				if (ts->pElementObject->Expression1)
					((CExpression*)(ts->pElementObject->Expression1))->PlotterPrepareVariablePositions(VarPos);
				if (ts->pElementObject->Expression2)
					((CExpression*)(ts->pElementObject->Expression2))->PlotterPrepareVariablePositions(VarPos);
				if (ts->pElementObject->Expression3)
					((CExpression*)(ts->pElementObject->Expression3))->PlotterPrepareVariablePositions(VarPos);
			}
		}
	}
	catch(...)
	{
		LeaveCriticalSection(&section3);return NULL;
	}
	LeaveCriticalSection(&section3);
	return (void*)VarPos;
}

int CExpression::PlotterReleaseVariablePositions(void *VarPos)
{
	if (wwwwxxxx==0) {InitializeCriticalSection(&section3);wwwwxxxx=1;}
	int cntrr=0;
	while (!TryEnterCriticalSection(&section3))
	{
		Sleep(10);cntrr++;if (cntrr>300) {return 0;}		
	}

	try
	{
	//this->CopyExpression(((tVariablePositions*)VarPos)->original,0);
	//delete ((tVariablePositions*)VarPos)->original;
	//HeapFree(ProcessHeap,0,VarPos);

	delete VarPos;
	}
	catch(...)
	{
		LeaveCriticalSection(&section3);return 0;
	}
	LeaveCriticalSection(&section3);
	return 1;
}
//int sshit=0;
double CExpression::PlotterCalculateFunctionValue(double X,void *VP)
{
	if (wwwwxxxx==0) {InitializeCriticalSection(&section3);wwwwxxxx=1;}
	int cntrr=0;
	while (!TryEnterCriticalSection(&section3))
	{
		Sleep(10);cntrr++;if (cntrr>300) {return sqrt(-1.0);}		
	}

	tVariablePositions *VarPos=(tVariablePositions*)VP;

	//first replace all variables with the given value

	is_call_from_ploter=1;

	try
	{
		char ASCIIx[50];
		//sprintf(ASCIIx,"%.12lf",X);
		sprintf(ASCIIx,"%.1lf",X); //we don't need very high precision because numbers are actually writen as floats
		ASCIIx[14]=0;


		for (int i=0;i<VarPos->NumPositions;i++)
		{
			tElementStruct *ts=(VarPos->positions[i].expr)->m_pElementList+VarPos->positions[i].position;
			memset(ts->pElementObject->Data2,0,24);
			ts->pElementObject->Data1[14]=0; 
			ts->pElementObject->Data1[15]=126;
			if (VarPos->positions[i].variable_type==0)
			{
				strcpy(ts->pElementObject->Data1,ASCIIx);
				(*((double*)&(ts->pElementObject->Data1[16])))=fabs(X);
			}
			else if (VarPos->positions[i].variable_type==1)
			{
				strcpy(ts->pElementObject->Data1,"2.71");
				(*((double*)&(ts->pElementObject->Data1[16])))=2.71828182845904523;
			}
			else if (VarPos->positions[i].variable_type==2)
			{
				strcpy(ts->pElementObject->Data1,"3.14");
				(*((double*)&(ts->pElementObject->Data1[16])))=3.141592653589793238;
			}
		}
	}
	catch(...)
	{
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);return sqrt(-1.0);
	}

	CExpression *tmp;
	try
	{
		tmp=new CExpression(NULL,NULL,100);
		if (VarPos->starting_point==0) {tmp->m_ParenthesesFlags=this->m_ParenthesesFlags;tmp->m_ParentheseShape=this->m_ParentheseShape;}
		for (int i=VarPos->starting_point;i<m_NumElements;i++)
			tmp->InsertElement(this->m_pElementList+i,tmp->m_NumElements);
	}
	catch(...) 
	{
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);return sqrt(-1.0);
	}

	try
	{
		int cntr=0;
		while ((tmp->Compute(0,tmp->m_NumElements-1,10)) && (cntr<50)) cntr++;
	}
	catch(...)
	{
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);return sqrt(-1.0);
	}

	tPureFactors PF;
	PF.N1=PF.N2=1.0;
	PF.is_frac1=0;
	PF.prec1=0;

	int rval=0;
	try
	{
		rval=tmp->StrikeoutCommonFactors(0,tmp->m_NumElements-1,1,NULL,0,0,1,&PF);
		delete tmp;
	}
	catch(...)
	{
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);return sqrt(-1.0);

	}

	double retval=0;

	try
	{
	if (!rval)
	{
		//found no unique solution
		double f=sqrt(-1.0);
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);
		return f; //intentionally returning NaN
	}

	retval=PF.N1/PF.N2;
	if (_finite(retval))
	{
		if (retval<-1e+48) retval=-1e+48;
		if (retval>1e+48) retval=1e+48;
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);
		return retval;
	}
	if (_isnan(retval)) {is_call_from_ploter=0;return retval;}
	//infinite numbers
	if (PF.N1*PF.N2<0)
	{is_call_from_ploter=0;LeaveCriticalSection(&section3);return -1e+48;}
	else if (PF.N1*PF.N2>0)
	{is_call_from_ploter=0;LeaveCriticalSection(&section3);return 1e+48;}
	else
	{
		is_call_from_ploter=0;
		if (PF.N1<0) 
		{LeaveCriticalSection(&section3);return -1e+48;}
		else
		{LeaveCriticalSection(&section3);return 1e+48;}
	}
	}
	catch(...)
	{
		is_call_from_ploter=0;
		LeaveCriticalSection(&section3);return sqrt(-1.0);
	}

	is_call_from_ploter=0;
	LeaveCriticalSection(&section3);
	return retval;
}


//this functions substitutes into Base from the Substitute 
//Base - the base expression into which it will be substitutetd
//Substitute - expression or equation from where substitution will be made (extracted)
//Variable - the variable (if defined) that will be used for substittion
int CExpression::MakeSubstitution(CExpression *Substitute, CExpression *Variable)
{
	//validate input parameters
	if (Substitute==NULL) return 0;
	if (this->m_pElementList->Type==0) return 0;
	if (Substitute->m_pElementList->Type==0) return 0;

	if (!this->IsSuitableForComputation()) return 0;
	if (!Substitute->IsSuitableForComputation()) return 0;
	if ((Variable) && (Variable->m_pElementList->Type))
		if (!Variable->IsSuitableForComputation()) return 0;
	if (Substitute->FindLowestOperatorLevel((char)0xD7)<EqLevel) return 0;

	if (Substitute->FindLowestOperatorLevel()!=EqLevel) return 0;
	
	//transform the Substitute into simple equation - delete all medium elements (leave first and last only)
	int first_end=0;
	while (first_end<Substitute->m_NumElements)
	{
		if ((((Substitute->m_pElementList)+first_end)->Type==2) &&
			(GetOperatorLevel((Substitute->m_pElementList+first_end)->pElementObject->Data1[0])==EqLevel))
			break;
		first_end++;
	}
	if (first_end>=Substitute->m_NumElements) {return 0;} //strange error
	int last_start=Substitute->m_NumElements-1;
	while (last_start>=0)
	{
		if ((((Substitute->m_pElementList)+last_start)->Type==2) &&
			(GetOperatorLevel((Substitute->m_pElementList+last_start)->pElementObject->Data1[0])==EqLevel))
			break;
		last_start--;
	}
	if (last_start<first_end) {return 0;} //strange error

	for (int i=0;i<last_start-first_end+1;i++)
		Substitute->DeleteElement(first_end);

	//force it into equation (even if it was non-equation). This is questionable
	Substitute->InsertEmptyElement(first_end,2,'=');
	

	CExpression *var=new CExpression(NULL,NULL,100);

	//if the Variable is not defined, try to conclude what variable to be used for substitution
	if ((Variable) && (Variable->m_pElementList->Type))
	{
		//variable must be of appropriate level
		if (Variable->FindLowestOperatorLevel((char)0xD7)<PlusLevel) {delete var;return 0;}

		tPureFactors PF;
		PF.N1=PF.N2=1.0;
		PF.is_frac1=PF.prec1=0;
		if (!Variable->StrikeoutCommonFactors(0,Variable->m_NumElements-1,1,NULL,0,0,1,&PF))
		{
			//we are going to use the variable as defined
			Variable->StrikeoutRemove(0,Variable->m_NumElements-1);
			var->CopyExpression(Variable,0);
		}
		else
		{
			//the variable is useless (only pure numbers), we will try to determine our own (down latter)
			delete var;
			return 0;
		}
	}
	else
	{
		//the variable is not defined (or is empty) so we are going
		//to try to determine if there is a suitable varaibles

		if (Substitute->FindLowestOperatorLevel(0,first_end-1,(char)0xD7)<PlusLevel) {delete var;return 0;} //strange error
		if (Substitute->FindLowestOperatorLevel(first_end+1,Substitute->m_NumElements-1,(char)0xD7)<PlusLevel) {delete var; return 0;} //strange error

		//create list of all variables of the Substitute, then try for every 
		//variable if it is also present in the Base
		int summand_no=0;
		tVariableList *VarList=(tVariableList*)Substitute->GenerateVariableList(0,Substitute->m_NumElements-1,&summand_no);
		if (VarList==NULL) {delete var;return 0;} //unexpected error

		//now go throught list of all variables and check if it exists in the Base
		int founds=0;
		int fnd=0;
		for (int i=0;i<VarList->NumVariables;i++)
		{
			if (this->ContainsVariable(0,this->m_NumElements-1,VarList->Variables[i].variable,VarList->Variables[i].pos,VarList->Variables[i].len))
			{
				founds++;
				fnd=i;
			}
		}
		if (founds==1)
		{
			//we found one and only one variable shared between Base and Substitute
			//we will use it
			for (int i=0;i<VarList->Variables[fnd].len;i++)
				var->InsertElement(VarList->Variables[fnd].variable->m_pElementList+VarList->Variables[fnd].pos+i,var->m_NumElements);
		}

		//we can now delete the variable list
		FreeVariableList(VarList);

		//if still the variable is not defined, define it as the left side
		//of the Substitute equation
		if (var->m_pElementList->Type==0)
		{
			for (int i=0;i<first_end;i++)
				var->InsertElement(Substitute->m_pElementList+i,var->m_NumElements);
		}
		
		if (var->m_pElementList->Type==0) {delete var; return 0;} //we were unable to determine the variable
	}


	//now we have:
	//Base
	//Substitute (in form of equation)
	//var (the variable that is used in substitution)


	//We proceed by extracting the variable 'var' from 'Substitute' onto the left side
	int cnt=0;
	try
	{
		int retval=0;
		int ret=1;
		int prev_ret=1;
		while (cnt<50)
		{
			if (cnt)
			{
				ret=Substitute->ExtractVariable(var,0,var->m_NumElements,ret);
				if (Substitute->ContainsVariable(0,Substitute->m_NumElements-1,var,0,var->m_NumElements)==0)
				{
					//an error - variable cannot be extracted
					delete var;
					return 0;
				}
			}
			else
			{
				//if there are no variables on the right side, try substitution
				//(otherwise, extract)
				char et,p;
				int l=Substitute->GetElementLen(0,Substitute->m_NumElements-1,EqLevel,&et,&p);
				if (Substitute->ContainsVariable(l+1,Substitute->m_NumElements-1,var,0,var->m_NumElements))
				{
					cnt++;
					continue;
				}
			}
			
			if ((ret==0) || (cnt==0) || ((ret==4) && (prev_ret!=ret)))
			{
				char et,p;
				int l=Substitute->GetElementLen(0,Substitute->m_NumElements-1,EqLevel,&et,&p);
				if ((l==0) || (l==Substitute->m_NumElements)) {delete var; return 0;} //strange error
				int l2=Substitute->GetElementLen(l+1,Substitute->m_NumElements-1,EqLevel,&et,&p);
				if ((l2==0) || (l2+l+1!=Substitute->m_NumElements)) {delete var;return 0;} //strange error
			
				CExpression *Find=new CExpression(NULL,NULL,100);
				for (int i=0;i<l;i++)
					Find->InsertElement(Substitute->m_pElementList+i,Find->m_NumElements);

				CExpression *Replace=new CExpression(NULL,NULL,100);
				for (int i=l+1;i<Substitute->m_NumElements;i++)
					Replace->InsertElement(Substitute->m_pElementList+i,Replace->m_NumElements);

				//we will try to find the left part of the Substitute
				if (this->FindReplace(0,this->m_NumElements-1,Find,Replace))
					retval=1;

				delete Find;
				delete Replace;

				if (ret==0) break; //we finished
			}
			prev_ret=ret;
			cnt++;
		}

		delete var;
		return retval;
	}
	catch(...)
	{
		delete var;
		return 0;
	}


	return 0;
}



void *CExpression::GenerateVariableList(int StartPos,int EndPos,int *summand_no, tVariableList *VarList)
{
	int prim_level=0;
	if (VarList==NULL)
	{
		prim_level=1;
		VarList=(tVariableList*)HeapAlloc(ProcessHeap,HEAP_ZERO_MEMORY,sizeof(tVariableList));
		if (VarList==NULL) return NULL;
		for (int i=0;i<MAX_SUMMANDS;i++) VarList->Constants[i].N1=VarList->Constants[i].N2=1.0;
		*summand_no=0;
	}

	int Level=FindLowestOperatorLevel(StartPos,EndPos,'+');
	if (Level<PlusLevel)
	{
		int pos=StartPos;
		while (1)
		{
			char et,p;
			int l=GetElementLen(pos,EndPos,Level,&et,&p);
			if (l==0) break;

			//recursively call
			VarList=(tVariableList*)GenerateVariableList(pos+p,pos+l-1,summand_no,VarList);
			if (VarList==NULL) return NULL;
			
			pos+=l;
			if (pos>EndPos) break;
		}
		return VarList;
	}


	//create list of all variables (run through all summands and call 'ExtractVariables')
	int pos=StartPos;
	while (1)
	{
		char et,p;
		int l=GetElementLen(pos,EndPos,PlusLevel,&et,&p);
		if (l==0) break;

		if (et=='-') VarList->Constants[*summand_no].N1=-VarList->Constants[*summand_no].N1;
		ExtractVariablesMode=1;
		ExtractVariables(pos+p,pos+l-1,1.0,*summand_no,VarList);
		ExtractVariablesMode=0;

		(*summand_no)++;
		if ((*summand_no)>=MAX_SUMMANDS)
		{
			FreeVariableList(VarList);
			return NULL;
		}

		pos+=l;
		if (pos>EndPos) break;
	}

	//if not even a single variable found - return error
	if ((VarList->NumVariables==0) && (prim_level))
	{
		HeapFree(ProcessHeap,0,VarList);
		return NULL;
	}

	return (void*)VarList;
}


void CExpression::FreeVariableList(tVariableList *VarList)
{
	if (VarList==NULL) return;
	for (int i=0;i<MAX_VARIABLES;i++)
		if (VarList->Variables[i].exponent)
			delete VarList->Variables[i].exponent;
	HeapFree(ProcessHeap,0,VarList);
}

//find and replace (if 'Replace==NULL' then find only)
int CExpression::FindReplace(int StartPos,int EndPos,CExpression *Find, CExpression *Replace)
{
	int occr=0;
	int BaseLvl=FindLowestOperatorLevel(StartPos,EndPos,(char)0xD7);
	int FindLvl=Find->FindLowestOperatorLevel((char)0xD7);
	if (Replace)
	{
		if (this->m_pPaternalElement)
		{
			if (((this->m_pPaternalElement->m_Type==1) && (this==(CExpression*)this->m_pPaternalElement->Expression1)) ||
				((this->m_pPaternalElement->m_Type==6) && (this==(CExpression*)this->m_pPaternalElement->Expression2)) ||
				((this->m_pPaternalElement->m_Type==5) && (this==(CExpression*)this->m_pPaternalElement->Expression2)))
				return 0; //we do not replace in indexes
		}
		//the find shuld not have leading minus or plus (for MulLevel)
		if ((FindLvl>=MulLevel) && (Find->m_pElementList->Type==2))
		{
			char ch=Find->m_pElementList->pElementObject->Data1[0];
			if ((ch=='+') || (ch=='-') || (ch==(char)0xD7))
			{
				Find->DeleteElement(0);
				if (ch=='-')
				{
					if ((Replace->FindLowestOperatorLevel((char)0xD7)<MulLevel) || (Replace->m_pElementList->Type==2))
					{
						Replace->InsertEmptyElement(0,5,'(');
						CExpression *a=(CExpression*)(Replace->m_pElementList->pElementObject->Expression1);
						while (Replace->m_NumElements>1)
						{
							a->InsertElement(Replace->m_pElementList+1,a->m_NumElements);
							Replace->DeleteElement(1);
						}
					}
					Replace->InsertEmptyElement(0,2,'-');
				}
			}
		}
	}
	

	if (BaseLvl<FindLvl)
	{
		int pos=StartPos;
		while(1)
		{
			char et,p;
			int l=GetElementLen(pos,EndPos,BaseLvl,&et,&p);
			if (l==0) break;

			CExpression *tmp=new CExpression(NULL,NULL,100);
			for (int i=pos+p;i<pos+l;i++)
				tmp->InsertElement(m_pElementList+i,tmp->m_NumElements);

			int tt=tmp->FindReplace(0,tmp->m_NumElements-1,Find,Replace);
			occr+=tt;
			if ((tt) && (Replace))
			{
				int delta=m_NumElements;
				for (int i=pos;i<pos+l;i++)
					DeleteElement(pos);
				pos=InsertSequence(et,pos,tmp,0,tmp->m_NumElements-1);
				EndPos+=m_NumElements-delta;
				l=0;
			}

			delete tmp;

			pos+=l;
			if (pos>EndPos) break;
		}

		return occr;
	}


	//check every element and test its subexpressions
	for (int i=StartPos;i<=EndPos;i++)
	{
		tElementStruct *ts=m_pElementList+i;
		if ((ts->pElementObject) && (ts->Type))
		{
			if (ts->pElementObject->Expression1)
				occr+=((CExpression*)ts->pElementObject->Expression1)->FindReplace(0,((CExpression*)ts->pElementObject->Expression1)->m_NumElements-1,Find,Replace);
			if (ts->pElementObject->Expression2)
				occr+=((CExpression*)ts->pElementObject->Expression2)->FindReplace(0,((CExpression*)ts->pElementObject->Expression2)->m_NumElements-1,Find,Replace);
			if (ts->pElementObject->Expression3)
				occr+=((CExpression*)ts->pElementObject->Expression3)->FindReplace(0,((CExpression*)ts->pElementObject->Expression3)->m_NumElements-1,Find,Replace);
		}
	}

	if (BaseLvl==FindLvl)
	{

		//first try if it is possible to make substitution the simple way
		//this way, we can place replacement at proper position
		if (Replace)
		{
			//first make markings on Replace variables, beacuse we don't want to
			//make infinite loop by replacing already replaced variables
			for (int i=0;i<Replace->m_NumElements;i++)
				(Replace->m_pElementList+i)->Decoration|=0x40;


			int pos=StartPos;
			while (1)
			{
				char et,p;
				int l=GetElementLen(pos,EndPos,BaseLvl,&et,&p);
				if (l==0) break;

				if (BaseLvl>PlusLevel)
				{
					if ((p==0) && ((this->m_pElementList+pos)->Type==2) &&
						(GetOperatorLevel((this->m_pElementList+pos)->pElementObject->Data1[0])==PlusLevel))
						{pos++;l--;}
				}

				int pos2=0;
				int pos3=pos;
				int inverted=-1;
				while (1)
				{
					char et2,p2,et3,p3;
					int l2=Find->GetElementLen(pos2,Find->m_NumElements-1,BaseLvl,&et2,&p2);
					if (l2==0) break;
					int l3=GetElementLen(pos3,EndPos,BaseLvl,&et3,&p3);
					if (l3==0) break;

					if (inverted==-1)
					{
						if (BaseLvl==MulLevel)
						{
							if ((et2==(char)0xD7) && (et3=='/')) inverted=1;
							else if ((et2=='/') && (et3==(char)0xD7)) inverted=1;
							else inverted=0;
						}
						else if (BaseLvl==PlusLevel)
						{
							if (((et2=='+') && (et3=='-')) ||
								((et2=='-') && (et3=='+')) ||
								((et2==(char)0xB1) && (et3==(char)0xB2)) ||
								((et2==(char)0xB2) && (et3==(char)0xB3))) inverted=1;
							else inverted=0;
						}
						else
							inverted=0;
					}
					
					if (inverted)
					{
						if (et3=='+') et3='-';
						else if (et3=='-') et3='+';
						else if (et3==(char)0xB2) et3=(char)0xB1;
						else if (et3==(char)0xB1) et3=(char)0xB2;
						else if (et3=='/') et3=(char)0xD7;
						else if (et3==(char)0xD7) et3='/';
					}

					if (et2!=et3) break;
					if (CompareExpressions(pos3+p3,pos3+l3-1,Find,pos2+p2,pos2+l2-1)==0) break;

					pos2+=l2;
					if (pos2>Find->m_NumElements-1)
					{
						if (Replace)
						{
							int delta=m_NumElements;
							for (int i=pos;i<pos3+l3;i++)
								DeleteElement(pos);
							char ett=et;
							if (BaseLvl==MulLevel) ett=(inverted)?'/':(char)0xD7;
							if (BaseLvl==PlusLevel) ett=(inverted)?'-':'+';
							pos=InsertSequence(ett,pos,Replace,0,Replace->m_NumElements-1);
							delta=m_NumElements-delta;
							EndPos+=delta;
							l=0;
						}
						occr++;
						break;
					}
					pos3+=l3;
					if (pos3>EndPos) break;
				}
				pos+=l;
				if (pos>EndPos) break;
			}
		}


		if (BaseLvl==PlusLevel)
		{
			CExpression *fnd=new CExpression(NULL,NULL,100);
			fnd->CopyExpression(Find,0);
			CExpression *base=new CExpression(NULL,NULL,100);
			for (int i=StartPos;i<=EndPos;i++)
				base->InsertElement(m_pElementList+i,base->m_NumElements);

			int cnt=0;
			while ((base->Compute(0,base->m_NumElements-1,0)) && (cnt<50)) cnt++;

			cnt=0;
			while ((fnd->Compute(0,fnd->m_NumElements-1,0)) && (cnt<50)) cnt++;

			int is_factor_defined=0;
			double factorN,factorD;
			int factorPrec=0,factorFrac=0;
			int pos=0;
			while (1)
			{
				char et,p;
				int l=fnd->GetElementLen(pos,fnd->m_NumElements-1,PlusLevel,&et,&p);
				if (l==0) break;


				int found=0;
				int pos2=0;
				while (1)
				{
					char et2,p2;
					int l2=base->GetElementLen(pos2,base->m_NumElements-1,PlusLevel,&et2,&p2);
					if (l2==0) break;

					tPureFactors PF;
					PF.N1=PF.N2=PF.N3=PF.N4=1.0;
					PF.is_frac1=PF.is_frac2=0;
					PF.prec1=PF.prec2=0;
					int res=base->StrikeoutCommonFactors(pos2+p2,pos2+l2-1,1,fnd,pos+p,pos+l-1,1,&PF);
					base->StrikeoutRemove(pos2+p2,pos2+l2-1);
					fnd->StrikeoutRemove(pos+p,pos+l-1);
					if ((et2=='+') && (et=='-')) {PF.N3=-PF.N3;et2=et;}
					if ((et2=='-') && (et=='+')) {PF.N3=-PF.N3;et2=et;}
					if ((et2==(char)0xB1) && (et==(char)0xB2)) {PF.N3=-PF.N3;et2=et;}
					if ((et2==(char)0xB2) && (et==(char)0xB1)) {PF.N3=-PF.N3;et2=et;}

					if ((res) && (et2==et))
					{
						if (PF.prec1>factorPrec) factorPrec=PF.prec1;
						if (PF.prec2>factorPrec) factorPrec=PF.prec2;
						if ((PF.is_frac1) || (PF.is_frac2)) factorFrac=1;
						double fN=PF.N1*PF.N4;
						double fD=PF.N2*PF.N3;
						if (!is_factor_defined)
							{factorN=fN;factorD=fD;found=1;is_factor_defined=1;}
						else
							if (fabs(fN/fD-factorN/factorD)<1e-100) {found=1;}
						if (found)
						{
							for (int ii=pos2;ii<pos2+l2;ii++)
								(base->m_pElementList+ii)->IsSelected|=0x01;
							break;
						}
					}

					pos2+=l2;
					if (pos2>base->m_NumElements-1) break;
				}

				if (found==0) break;

				pos+=l;
				if (pos>fnd->m_NumElements-1)
				{
					if (Replace)
					{
						int minus=0;
						if (factorD<0) {factorD=-factorD;factorN=-factorN;}
						if (factorN<0) {minus=1;factorN=-factorN;}

						base->InsertEmptyElement(base->m_NumElements,2,(minus)?'-':'+');
						int t=base->GenerateASCIIFraction(base->m_NumElements,factorN,factorD,factorPrec,factorFrac);
						base->InsertSequence((char)0xD7,base->m_NumElements,Replace,0,Replace->m_NumElements-1);

						for (int i=0;i<base->m_NumElements;i++)
							if ((base->m_pElementList+i)->IsSelected&0x01)
							{
								base->DeleteElement(i);
								i--;
							}

						for (int i=StartPos;i<=EndPos;i++)
							DeleteElement(StartPos);
						InsertSequence('+',StartPos,base,0,base->m_NumElements-1);
					}

					occr++;
					break;
				}
			}

			delete fnd;
			delete base;			
		}
		else if (BaseLvl==MulLevel)
		{
			//check if there is any variable in 'Find' (the VarList will be generated only when there is)
			int summand_no;
			tVariableList *VarList=(tVariableList*)Find->GenerateVariableList(0,Find->m_NumElements-1,&summand_no);
			if (VarList)
			{
				Find->FreeVariableList(VarList);
				CExpression *fnd=new CExpression(NULL,NULL,100);
				CExpression *base=new CExpression(NULL,NULL,100);
				for (int i=StartPos;i<=EndPos;i++)
					base->InsertElement(m_pElementList+i,base->m_NumElements);

				int powert=0;
				double N1=1.0;
				double N2=1.0;
				int maxprec=0;
				int isfrac=0;
				int done=0;
				while (1)
				{
					fnd->CopyExpression(Find,0);
					tPureFactors PF;
					PF.N1=PF.N2=PF.N3=PF.N4=1.0;
					PF.is_frac1=PF.is_frac2=0;
					PF.prec1=PF.prec2=0;
					int res=base->StrikeoutCommonFactors(0,base->m_NumElements-1,1,fnd,0,fnd->m_NumElements-1,1,&PF);
					fnd->StrikeoutRemove(0,fnd->m_NumElements-1,1);
					double N;
					int prec;
					if ((fnd->m_pElementList->Type==0) || (fnd->IsPureNumber(0,fnd->m_NumElements,&N,&prec)))
					{

						base->StrikeoutRemove(0,base->m_NumElements-1,1);
						N1*=PF.N1*PF.N4;
						N2*=PF.N2*PF.N3;
						if ((PF.is_frac1) || (PF.is_frac2)) isfrac=1;
						if (PF.prec1>maxprec) maxprec=PF.prec1;
						if (PF.prec2>maxprec) maxprec=PF.prec2;
						occr++;
						powert++;
						done=1;
					}
					else
					{
						base->StrikeoutRemove(0,base->m_NumElements-1);
						break;
					}
				}

				while (1)
				{
					fnd->CopyExpression(Find,0);
					tPureFactors PF;
					PF.N1=PF.N2=PF.N3=PF.N4=1.0;
					PF.is_frac1=PF.is_frac2=0;
					PF.prec1=PF.prec2=0;
					int res=base->StrikeoutCommonFactors(0,base->m_NumElements-1,1,fnd,0,fnd->m_NumElements-1,-1,&PF);
					fnd->StrikeoutRemove(0,fnd->m_NumElements-1,1);
					double N;
					int prec;
					if ((fnd->m_pElementList->Type==0) || (fnd->IsPureNumber(0,fnd->m_NumElements,&N,&prec)))
					{
						base->StrikeoutRemove(0,base->m_NumElements-1,1);
						N1*=PF.N1*PF.N4;
						N2*=PF.N2*PF.N3;
						if ((PF.is_frac1) || (PF.is_frac2)) isfrac=1;
						if (PF.prec1>maxprec) maxprec=PF.prec1;
						if (PF.prec2>maxprec) maxprec=PF.prec2;
						occr++;
						powert--;
						done=1;
					}
					else
					{
						base->StrikeoutRemove(0,base->m_NumElements-1);
						break;
					}
				}
				if ((done) && (Replace))
				{
					for (int i=StartPos;i<=EndPos;i++)
						DeleteElement(StartPos);
					if (powert!=1)
					{
						InsertEmptyElement(StartPos,3,0);
						CExpression *b=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression1);
						CExpression *e=(CExpression*)((m_pElementList+StartPos)->pElementObject->Expression2);
						b->CopyExpression(Replace,0);
						b->m_ParenthesesFlags=0x02;
						b->m_ParentheseShape='(';
						e->GenerateASCIINumber((double)powert,(long long)powert,1,0,0);
						if (base->m_pElementList->Type)
							for (int i=0;i<base->m_NumElements;i++)
								InsertElement(base->m_pElementList+i,StartPos+1+i);
						GenerateASCIIFraction(StartPos,N1,N2,maxprec,isfrac);
					}
					else
					{
						base->InsertSequence(GetDefaultElementType(MulLevel),0,Replace,0,Replace->m_NumElements-1);
						base->GenerateASCIIFraction(0,N1,N2,maxprec,isfrac);
						InsertSequence(GetDefaultElementType(MulLevel),StartPos,base,0,base->m_NumElements-1);
					}
				}
				delete fnd;
				delete base;
			}
		}
		else if (CompareExpressions(StartPos,EndPos,Find,0,Find->m_NumElements-1))
		{
			if (Replace)
			{
				for (int i=StartPos;i<=EndPos;i++)
					DeleteElement(StartPos);
				for (int i=0;i<Replace->m_NumElements;i++)
					InsertElement((Replace->m_pElementList+i),StartPos+i);
			}
		}

		//delete markings from replace variables
		if (Replace)
		{
			for (int i=0;i<Replace->m_NumElements;i++)
				(Replace->m_pElementList+i)->Decoration&=0x3F;
			for (int i=0;i<m_NumElements;i++)
			{
				(m_pElementList+i)->Decoration&=0x3F;
				if ((m_pElementList+i)->Type==5)
				{
					CExpression *a=(CExpression*)((m_pElementList+i)->pElementObject->Expression1);
					for (int j=0;j<a->m_NumElements;j++)
						(a->m_pElementList+j)->Decoration&=0x3F;
				}
			}
		}
	}

	return occr;
}



typedef struct 
{
	char name[24];
	char font[24];
} tRawVariableStorage;

int CExpression::SolveSystemOfEquations(CExpression *System[],int *NumEquations, CObject *ThePopupMenu)
{
	//we have a system of equations, we are going to suppose that
	//every variable is an unknown - we try for each variable in every equation
	//and make substitutions to others
	for (int i=0;i<*NumEquations;i++)
		System[i]->CodeDecodeUnitsOfMeasurement(0,-1);

	tRawVariableStorage *VS=(tRawVariableStorage*)calloc(sizeof(tRawVariableStorage),24);
	int num_variables=CountVariablesInSystem(System,*NumEquations,(void*)VS);
	free(VS);
	if (num_variables<0) return 0;

	int prefer_singles=1;
	int ReducedNEq=*NumEquations;
	for (int i=0;i<ReducedNEq;i++)
	{
		if (System[i]->FindLowestOperatorLevel((char)0xD7)==EqLevel)
		{
			tRawVariableStorage *VS=(tRawVariableStorage*)calloc(sizeof(tRawVariableStorage),24);
			int num_variables=CountVariablesInSystem(&System[i],1,(void*)VS);
			
			if (num_variables==0)
			{
				//strange, we delete this variable from list
				free(VS);
				delete System[i];
				for (int kk=i+1;kk<*NumEquations;kk++) System[kk-1]=System[kk];
				(*NumEquations)--;
				ReducedNEq--;
				i--;
				continue;
			}

			// if there is only one signle variable, then we have one of sollutions
			if (num_variables==1)
			{
				CExpression *variable=new CExpression(NULL,NULL,100);
				variable->InsertEmptyElement(0,1,'a');
				strcpy(variable->m_pElementList->pElementObject->Data1,VS->name);
				memcpy(variable->m_pElementList->pElementObject->Data2,VS->font,strlen(VS->name));
				free(VS);

				for (int j=0;j<*NumEquations;j++)
					if (j!=i) System[j]->MakeSubstitution(System[i],variable);
				

				if (ThePopupMenu)
				{
					int cnt=0;
					int ret=1;
					while (cnt<50)
					{
						ret=System[i]->ExtractVariable(variable,0,variable->m_NumElements,ret);
						if (ret==0) break;
						cnt++;
					}
					System[i]->MakeExpressionBeautiful();
					if (!((PopupMenu*)ThePopupMenu)->AddMathMenuOption(System[i]))
						delete System[i];

				}
				else
					delete System[i];

				delete variable;

				//delete this equation, as it is used now
				for (int kk=i+1;kk<*NumEquations;kk++) System[kk-1]=System[kk];
				(*NumEquations)--;
				ReducedNEq=*NumEquations;
				i=-1;
				continue;
			}

			//for every variable in variable list, we will try substitution
			//if we manage to make successfull substitution, then we will accept it
			//the successfull substitution means that we completely eliminated at least one variable
			if ((ReducedNEq>1) && (prefer_singles==0))
			for (int j=0;j<num_variables;j++)
			{
				CExpression *variable=new CExpression(NULL,NULL,100);
				variable->InsertEmptyElement(0,1,'a');
				strcpy(variable->m_pElementList->pElementObject->Data1,VS[j].name);
				memcpy(variable->m_pElementList->pElementObject->Data2,VS[j].font,strlen(VS[j].name));

				//for (int k=0;k<VarList->Variables[j].len;k++)
				//	variable->InsertElement(VarList->Variables[j].variable->m_pElementList+VarList->Variables[j].pos+k,variable->m_NumElements);


				//make copy of current equations - the temporary system
				CExpression *S[24];
				int NEq=0;
				for (int k=0;k<ReducedNEq;k++)
				if (k!=i)
				{
					S[NEq]=new CExpression(NULL,NULL,100);
					S[NEq]->CopyExpression(System[k],0);
					NEq++;
				}

				//try substitution
				for (int k=0;k<NEq;k++)
				{
					CExpression *Substitute=new CExpression(NULL,NULL,100);
					Substitute->CopyExpression(System[i],0);

					S[k]->MakeSubstitution(Substitute,variable);

					delete Substitute;
				}

				//check if any variable is reduced
				tRawVariableStorage *VS2=(tRawVariableStorage*)calloc(sizeof(tRawVariableStorage),24);
				int tmp_cnt=CountVariablesInSystem(S,NEq,VS2);
				free(VS2);
				if (tmp_cnt<num_variables)
				{
					//Yes, there is reduced number of variables
					//accept the temporary system for the next step

					S[NEq]=System[i];				

					//delete the old system
					for (int k=0;k<ReducedNEq;k++)
						if (i!=k) delete System[k];

					//copy new system
					for (int k=0;k<NEq;k++)
					{
						int cnt=0;
						while ((S[k]->Compute(0,S[k]->m_NumElements-1,0)) && (cnt<50)) cnt++;
						System[k]=S[k];
					}
					System[NEq]=S[NEq];
					ReducedNEq=NEq;


					num_variables=tmp_cnt;
					prefer_singles=1;

					delete variable;
					
					i=-1; //this will restart outer for/next
					break;
				}
				else
				{
					//delete the temporary system
					for (int k=0;k<NEq;k++)
						delete S[k];
					delete variable;
				}
			}

			free(VS);

		}
		if ((prefer_singles) && (i==ReducedNEq-1))
		{
			//try once more, but now don't prefer singles any more
			prefer_singles=0;
			i=-1;
		}
	}

	if (*NumEquations<1) return 0; //that's all

	if (ThePopupMenu)
	{
		//add the rest in the popup menu

		for (int i=0;i<*NumEquations;i++)
		{
			int cnt=0;
			
			tRawVariableStorage *VS2=(tRawVariableStorage*)calloc(sizeof(tRawVariableStorage),24);
			int tmp_cnt=CountVariablesInSystem(&System[i],1,VS2);
			if ((tmp_cnt==1) && (System[i]->ContainsVariable(0,System[i]->m_NumElements-1,NULL,0,0,VS2->name,((VS2->font[0]&0xE0)==0x60)?1:0)))
			{
				CExpression *var=new CExpression(NULL,NULL,100);
				var->InsertEmptyElement(0,1,'a');
				strcpy(var->m_pElementList->pElementObject->Data1,VS2->name);
				memcpy(var->m_pElementList->pElementObject->Data2,VS2->font,strlen(VS2->name));

				int ret=1;
				while (cnt<50)
				{
					ret=System[i]->ExtractVariable(var,0,1,ret);
					if (ret==0) break;
					cnt++;
				}
				delete var;

			}
			free(VS2);

			cnt=0;
			while ((System[i]->Compute(0,System[i]->m_NumElements-1,0)) && (cnt<50)) cnt++;
			if (!((PopupMenu*)ThePopupMenu)->AddMathMenuOption(System[i]))
				delete System[i];
		}
		*NumEquations=0;
	}
	else
		CopyExpression(System[0],0);
	return 1;

}

int CExpression::CountVariablesInSystem(CExpression *System[],int NumEquations, void *VariableStorage)
{
	int ret=0;


	if (NumEquations>1)
	{
		for (int i=0;i<NumEquations;i++)
		{
			int t=CountVariablesInSystem(&System[i],1,VariableStorage);
			if (t<0) return t;
			ret+=t;
		}
		return ret;
	}
	else if (NumEquations==1)
	{
		CExpression *tmp=System[0];
		for (int i=0;i<tmp->m_NumElements;i++)
		{
			tElementStruct *ts=tmp->m_pElementList+i;

			if ((ts->Type==0) || (ts->Type==11) || (ts->Type==12) || (ts->pElementObject==NULL)) continue;

			if (ts->Type==1)
			{
				double N;
				int prec;
				if (tmp->IsPureNumber(i,1,&N,&prec)) continue;

				int fnd=1;
				tRawVariableStorage *VS=(tRawVariableStorage*)VariableStorage;
				int cnt=0;
				while (VS->name[0])
				{
					if ((strcmp(VS->name,ts->pElementObject->Data1)==0) && 
						(memcmp(VS->font,ts->pElementObject->Data2,strlen(VS->name))==0)) 
						{fnd=0;break;}
					VS++;
					cnt++;if (cnt>=24) return -1;
				}
				if (fnd) 
				{
					strcpy(VS->name,ts->pElementObject->Data1);
					memcpy(VS->font,ts->pElementObject->Data2,strlen(VS->name));
					ret++;
				}
				continue;

			}

			int t;
			if (ts->pElementObject->Expression1)
			{
				t=((CExpression*)(ts->pElementObject->Expression1))->CountVariablesInSystem((CExpression**)&(ts->pElementObject->Expression1),1,VariableStorage);
				if (t<0) return t;
				ret+=t;
			}
			if (ts->pElementObject->Expression2)
			{
				t=((CExpression*)(ts->pElementObject->Expression2))->CountVariablesInSystem((CExpression**)&(ts->pElementObject->Expression2),1,VariableStorage);
				if (t<0) return t;
				ret+=t;
			}
			if (ts->pElementObject->Expression3)
			{
				t=((CExpression*)(ts->pElementObject->Expression3))->CountVariablesInSystem((CExpression**)&(ts->pElementObject->Expression3),1,VariableStorage);
				if (t<0) return t;
				ret+=t;
			}

		}
	}

	return ret;
}


//this function is called beffore any computation (with code_decode=0) and
//after the last computation (with code_decode=1). It must always be used in pair.
//
//This translates physical units so they can be used in computtion engine
//For example, it translates (5kg/3s) into (5kg/3/s)
//
//returns the change in length of the expression
int CExpression::CodeDecodeUnitsOfMeasurement(int StartPos,int EndPos)
{
	int retval=0;
	if (EndPos==-1) EndPos=m_NumElements-1;
	int lvl=FindLowestOperatorLevel(StartPos,EndPos);
	if ((lvl==-1) || (lvl>MulLevel)) return retval;


	if (lvl<MulLevel)
	{
		int pos=StartPos;
		while (1)
		{
			char et,p;
			int l=GetElementLen(pos,EndPos,lvl,&et,&p);
			int tt=CodeDecodeUnitsOfMeasurement(pos+p,pos+l-1);
			retval+=tt;
			l+=tt;
			pos+=l;
			if (pos>EndPos) break;
		}

		return retval;
	}

	//jump over the unary operator (leading plus or minus)
	if (((m_pElementList+StartPos)->Type==2) && (GetOperatorLevel((m_pElementList+StartPos)->pElementObject->Data1[0])==PlusLevel))
		StartPos++;

	char prev_op=(char)0xD7;
	int pos=StartPos;
	while (1)
	{
		char et,p;
		int l=GetElementLen(pos,EndPos,MulLevel,&et,&p);
		if (l-p==1)
		{
			tElementStruct *ts=m_pElementList+pos+p;
			if (ts->pElementObject->IsMeasurementUnit())
			{
				if (prev_op=='/')
				{
					if (p) 
						(m_pElementList+pos)->pElementObject->Data1[0]=(et=='/')?(char)0xD7:'/';
					else 
					{
						InsertEmptyElement(pos,2,'/');
						retval++;
						p=1;
						l++;
					}
				}
			}
			else 
			{
				prev_op=et;
				if (ts->pElementObject)
				{
				if (ts->pElementObject->Expression1)
					((CExpression*)(ts->pElementObject->Expression1))->CodeDecodeUnitsOfMeasurement(0,-1);
				if (ts->pElementObject->Expression2)
					((CExpression*)(ts->pElementObject->Expression2))->CodeDecodeUnitsOfMeasurement(0,-1);
				if (ts->pElementObject->Expression3)
					((CExpression*)(ts->pElementObject->Expression3))->CodeDecodeUnitsOfMeasurement(0,-1);
				}
			}
		}
		else prev_op=et;

		pos+=l;
		if (pos>EndPos) break;
	}

	return retval;
}

/*struct UNIT_CONVERTER
{
	int group;
	char compound_unit[6];
	struct
	{
		char name[4];
		int order;
	} units[6];
	float conversion_factor;
} unit_converter[]=
{
	{0,"N",{{"kg",1},{"m",1},{"s",-2}},1.0},

	{0,"",{{"",1}},0.0}

};

int CExpression::ComputeUnits()
{
NOT FINISHED!!!
	if (this->FindLowestOperatorLevel((char)0xD7)!=MulLevel) return 0;

	int summand_no=0;
	tVariableList *VarList=(tVariableList*)this->GenerateVariableList(0,this->m_NumElements-1,&summand_no);
	if (VarList==NULL) {return 0;} //unexpected error

	//check if there is any physical unit
	for (int i=0;i<VarList->NumVariables;i++)
	{
		if (VarList->Variables[i].len==1)
		{
			tElementStruct *ts=VarList->Variables[i].variable->m_pElementList+VarList->Variables[i].pos;
			if (ts->pElementObject->IsMeasurementUnit())
			{
			
				//found and measurement unit - now run throught list of compound units and check if the same
				int j=0;
				while (unit_converter[j].compound_unit[0])
				{
					char *str1=unit_converter[j].compound_unit;
					char *str2=ts->pElementObject->Data1[0];
					if (strcmp(str1,str2)==0)
					{

					}
				}
				
				

			}
		}
	}


	this->FreeVariableList(VarList);
	
}
*/