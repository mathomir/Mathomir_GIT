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
#include "Toolbox.h"
#include ".\toolbox.h"
#include "Mainfrm.h"
#include "popupmenu.h"
#include "drawing.h"
#include "MathomirDoc.h"
#include "MathomirView.h"

int ToolboxAutoopen;
int ToolboxAutoopenX;
int ToolboxAutoopenY;

extern char IsWindowOutOfFocus;
char TemporaryShowColoredSelection;
extern int NumSelectedObjects;
extern int NumSelectedDrawings;
extern CExpression *ClipboardExpression;
extern CToolbox *Toolbox;
extern PopupMenu *Popup;
extern CDrawing* ClipboardDrawing;
extern CMathomirView *pMainView;
extern int SelectedLineWidth;
extern int SelectedLineColor;
extern int PrintRendering;
char *TheKeycodeString;
extern int EasycastListStart;
extern int MovingStartX;
extern int MovingStartY;
// CToolbox
extern char HelpTutorLoaded;

CExpression *ToolboxCharacter;

int AlternateVirtualKeyboard=0;
const char *GetCommandFromCreationCode(short CreationCode);
//global variables - defining toolbox structures
int ToolboxNumMembers;
int origToolboxNumMembers;
struct TOOLBOX_MEMBERS
{
	short NumSubmembers;
	CExpression *Submembers[32];
	short Length[32];
	short Above[32]; //if -1, the object is a drawing (CDrawing), not expression
	short Below[32];
	short zoom[32];
	short prevToolboxSize[32];
	short AcceleratorKey[32];
	short SelectedSubmember;
	char Keycodes[32][9];
	short origNumSubmembers;
	short CreationCode[32]; //the code that is used in creation
	unsigned int userdef_mask; //for every submember that is user defined the mask bit is set (max 32 submembers)
} ToolboxMembers[24];

struct TOOLBOX_FONT_FORMATING
{
	short NumFormats;
	char UniformFormats2[12]; //color, singleshoot... the high bit is always on (for compatibility reasons)
	CExpression *UniformFormats[12];  //for 'uniform' keyboard mode!!!
	CExpression *MixedFormat;
	short UniformAccKey[12];
	short MixedAccKey;
	short SelectedUniform;
} ToolboxFontFormating;

char FontAdditionalData; //the first high bit must be high

struct TOOLBOX_KEYBOARD_ELEMENTS
{
	CExpression *Key[32];
	short X[32];
	short Y[32];
	short code[32];
	char FormatingBigCaps[32];
	char FormatingSmallCaps[32];
	char FormatingBigGreek[32];
	char FormatingSmallGreek[32];
	unsigned char FormatingBigCaps2[32];
	unsigned char FormatingSmallCaps2[32];
	unsigned char FormatingBigGreek2[32];
	unsigned char FormatingSmallGreek2[32];
	int NumKeys;
} ToolboxKeyboardElements;

const struct TOOLBOX_KEYCODES
{
	unsigned short code;
	char name[6];
} ToolboxKeycodes[]={
	{2,"FONT"},
	{3,"SCAN"},
	{'1',"1"},
	{'2',"2"},
	{'3',"3"},
	{'4',"4"},
	{'5',"5"},
	{'6',"6"},
	{'7',"7"},
	{'8',"8"},
	{'9',"9"},
	{'0',"0"},

{'A',"A"},
{'B',"B"},
{'D',"D"},
{'E',"E"},
{'F',"F"},
{'G',"G"},
{'H',"H"},
{'I',"I"},
{'J',"J"},
{'K',"K"},
{'L',"L"},
{'M',"M"},
{'N',"N"},
{'O',"O"},
{'P',"P"},
{'Q',"Q"},
{'R',"R"},
//{'S',"S"},
{'T',"T"},
{'U',"U"},
{'W',"W"},
{'Y',"Y"},

	{VK_F1,"F1"},
	{VK_F2,"F2"},
	{VK_F3,"F3"},
	{VK_F4,"F4"},
	{VK_F5,"F5"},
	{VK_F7,"F7"},
	{VK_F8,"F8"},
	{VK_F9,"F9"},
	{VK_F11,"F11"},
	{VK_F12,"F12"},
	{VK_OEM_PLUS,"+"},
	{VK_OEM_MINUS,"-"},
	{VK_OEM_COMMA,","},
	{VK_OEM_PERIOD,"."},
	{VK_UP+0x100,"Up"},
	{VK_DOWN+0x100,"Down"},
	{VK_LEFT+0x100,"Left"},
	{VK_RIGHT+0x100,"Right"},
	{VK_SPACE,"Space"},
	{VK_DELETE+0x100,"Del"},
	{VK_HOME+0x100,"Home"},
	{VK_END+0x100,"End"},
	{VK_TAB,"Tab"},
	{'1'+0x200,"1"},
	{'2'+0x200,"2"},
	{'3'+0x200,"3"},
	{'4'+0x200,"4"},
	{'5'+0x200,"5"},
	{'6'+0x200,"6"},
	{'7'+0x200,"7"},
	{'8'+0x200,"8"},
	{'9'+0x200,"9"},
	{'0'+0x200,"0"},
	{'A'+0x200,"A"},
{'B'+0x200,"B"},
{'C'+0x200,"C"},
{'D'+0x200,"D"},
{'E'+0x200,"E"},
{'F'+0x200,"F"},
{'G'+0x200,"G"},
{'H'+0x200,"H"},
{'I'+0x200,"I"},
{'J'+0x200,"J"},
{'K'+0x200,"K"},
{'L'+0x200,"L"},
{'M'+0x200,"M"},
{'N'+0x200,"N"},
{'O'+0x200,"O"},
{'P'+0x200,"P"},
{'Q'+0x200,"Q"},
{'R'+0x200,"R"},
{'S'+0x200,"S"},
{'T'+0x200,"T"},
{'U'+0x200,"U"},
{'W'+0x200,"W"},
{'Y'+0x200,"Y"},

	{VK_F1+0x200,"F1"},
	{VK_F2+0x200,"F2"},
	{VK_F3+0x200,"F3"},
	{VK_F4+0x200,"F4"},
	{VK_F5+0x200,"F5"},
	{VK_F7+0x200,"F7"},
	{VK_F8+0x200,"F8"},
	{VK_F9+0x200,"F9"},
	{VK_F11+0x200,"F11"},
	{VK_F12+0x200,"F12"},
	{4,""}, //separator
{'a'+0x400,"aa"},
{'b'+0x400,"bb"},
{'c'+0x400,"cc"},
{'d'+0x400,"dd"},
{'e'+0x400,"ee"},
{'f'+0x400,"ff"},
{'g'+0x400,"gg"},
{'h'+0x400,"hh"},
{'i'+0x400,"ii"},
{'j'+0x400,"jj"},
{'k'+0x400,"kk"},
{'l'+0x400,"ll"},
{'m'+0x400,"mm"},
{'n'+0x400,"nn"},
{'o'+0x400,"oo"},
{'p'+0x400,"pp"},
{'q'+0x400,"qq"},
{'r'+0x400,"rr"},
{'s'+0x400,"ss"},
{'t'+0x400,"tt"},
{'u'+0x400,"uu"},
{'v'+0x400,"vv"},
{'w'+0x400,"ww"},
{'x'+0x400,"xx"},
{'y'+0x400,"yy"},
{'z'+0x400,"zz"},
{'A'+0x400,"AA"},
{'B'+0x400,"BB"},
{'C'+0x400,"CC"},
{'D'+0x400,"DD"},
{'E'+0x400,"EE"},
{'F'+0x400,"FF"},
{'G'+0x400,"GG"},
{'H'+0x400,"HH"},
{'I'+0x400,"II"},
{'J'+0x400,"JJ"},
{4,""}, //separator
{1,"EXIT"},
{'K'+0x400,"KK"},
{'L'+0x400,"LL"},
{'M'+0x400,"MM"},
{'N'+0x400,"NN"},
{'O'+0x400,"OO"},
{'P'+0x400,"PP"},
{'Q'+0x400,"QQ"},
{'R'+0x400,"RR"},
{'S'+0x400,"SS"},
{'T'+0x400,"TT"},
{'U'+0x400,"UU"},
{'V'+0x400,"VV"},
{'W'+0x400,"WW"},
{'X'+0x400,"XX"},
{'Y'+0x400,"YY"},
{'Z'+0x400,"ZZ"},
{'&'+0x400,"&&"},
{';'+0x400,";;"},
{':'+0x400,"::"},
{'%'+0x400,"%%"},
{'$'+0x400,"$$"},
{0,""}
};

const struct 
{
	unsigned char member,submember;
} SpecialDrawingToolbox[9]=
{
{12,1},
{12,0},
{13,2},

{12,3},
{12,2},
{12,4},

{12,14},
{12,15},
{12,5}
};


const struct 
{
	unsigned short lang_code;
	char name[10];
	char long_name[32];
	unsigned char pixel_len;
} ToolbarIcons[]=
{
	{31101,"Undo","Undo (Ctrl+Z)",25}, //0
	{31002,"Save","Save (Ctrl+S)",25}, //1
	{31102,"Cut","Cut (Ctrl+X)",25},   //2
	{31103,"Copy","Copy (Ctrl+C)",25}, //3
	{31104,"Paste","Paste (Ctrl+V)",25},   //4
	{30301,"Zoom","Zoom to default (1:1)",25}, //5
	{0,"Image","Take bitmap image (F9)",25},    //6
	{32838,"Grid","Activate grid",25},         //7
	{6017,"Draw","Hand-drawing tools(F4)",25}, //8
	{48,"Increase","Increase font size",25},//9
	{47,"Decrease","Decrease font size",25},//10
	{560,"Left","Align to left",25},         //11
	{562,"Center","Align to horizontal center",25},//12
	{564,"Right","Align to right",25},    //13
	{561,"Top","Align to top",25},        //14
	{563,"Center","Align to vertical center",25}, //15
	{565,"Bottom","Align to bottom",25},  //16
	{550,"Nodes","Edit drawing nodes (Ctrl)",25},//17
	{93,"Vertical","Vertical text orientation",25}, //18
	{6018,"Headline","Convert to headline",25},//19
	{535,"H.mirror","Horizontal mirror",25},//20
	{536,"V.mirror","Vertical mirror",25},//21
	{505,"Group","Group objects",25},//22
	{506,"Ungroup","Ungroup objects",25},//23
	{578,"Distrib.","Distribute vertically",25},//24
	{72,"Left","Left align",25},//25
	{73,"Center","Center align",25},//26
	{74,"Right","Right align",25},//27
	{15,"Bold","Bold (b)",25},//28
	{14,"Italic","Italic (i)",25},//29
	{6,"U.line","Underline (u)",25},//30
	{130,"U.brace","Under brace",25},//31
	{6010,"Line","Line drawing (Alt+1)",25},//32
	{6008,"Free","Freehand drawing (Alt+2)",25},//33
	{6019,"Arrow","Arrow drawing (Alt+3)",25},//34
	{6009,"Rect","Rectangle drawing (Alt+5)",25},//35
	{6011,"Ellip.","Ellipse drawing (Alt+4)",25},//36
	{6012,"Eraser","Large eraser (Alt+9)",25},//37
	{6012,"Eraser","Pencil eraser (Alt+6)",25},//38
	{32783,"Present.","Presentation mode (F5)",25},//39
};

struct
{
	unsigned char IconIndex; //index from the ToolbarIcons array
	unsigned char IconState; //0=disabled, 1=enabled, 2=active
	unsigned char pixel_len; //
	unsigned char pixel_separator; //length of the separator in pixels
} ToolbarConfig[24];

int ToolbarNumItems;
int ToolbarSelectedItem=-1;
int ToolbarIsClicked=0;
int ToolbarHelpTimer=0;
int ToolbarItemPos;
//int ToolbarPreviousTool;
extern CEdit *StaticMessageWindow;


#pragma optimize("s",on)
IMPLEMENT_DYNAMIC(CToolbox, CWnd)
CToolbox::CToolbox(int IsSubtoolbox)
{
	Subtoolbox=NULL;
	ContextMenu=NULL;
	Keyboard=NULL;
	Toolbar=0;
	m_IsSubtoolbox=0;
	m_IsKeyboard=0;
	m_IsContextMenu=0;
	m_IsToolbar=0;
	m_IsMain=0;
	m_SelectedElement=-1;
	m_FontModeElement=-1;
	m_KeyboardElement=-1;
	m_SelectedColor=-1;
	m_SelectedTextControl=-1;

	if (IsSubtoolbox==0)
	{
		ToolboxNumMembers=0;
		memset(&ToolboxMembers,0,sizeof(ToolboxMembers));

		AddSubmember(2,'+');
		AddSubmember(2,'-');
		AddSubmember(2,(char)0xD7);  //multiplying dot
		AddSubmember(2,'/');
		AddSubmember(2,(char)0xB1);  //plus/minus 
		AddSubmember(2,(char)0xB2);   //the minus-plus operator
		AddSubmember(2,(char)0xB8);  //div ?? (like dash with two dots)
		AddSubmember(2,'&');
		AddSubmember(2,'m');   //mod
		AddSubmember(2,'a');   //and
		AddSubmember(2,'o');   //or
		AddSubmember(2,'A');   //nand
		AddSubmember(2,'O');   //nor
		AddSubmember(2,'x');   //xor
		AddSubmember(2,'n');   //not
		AddSubmember(2,'!');   //factoriel
		AddSubmember(2,(char)0xD9);   //and
		AddSubmember(2,(char)0xDA);   //or
		AddSubmember(2,'|');
		
		
		
		AddSubmember(2,(char)0xD8);   //not
		ToolboxNumMembers++;

		AddSubmember(2,'=');  
		AddSubmember(2,(char)0xB9);  //non equal
		AddSubmember(2,'<');
		AddSubmember(2,'>');
		AddSubmember(2,(char)0x01); //  <<
		AddSubmember(2,(char)0x02); //  >>
		AddSubmember(2,(char)0xBB);AddAccelerator('A');  //aprox. equal (waved equal)
		AddSubmember(2,(char)0x9F); //equal with hat
		AddSubmember(2,(char)0xA3);  //less or equal
		AddSubmember(2,(char)0xB3);  //greater or equal
		AddSubmember(2,(char)0xB5);  //proportianal?? (like infinite, but not closed)
		AddSubmember(2,(char)0x40);  //approx equal?? (equal with a tilda)
		AddSubmember(2,(char)0x7E);  //approx. equal (tilda)
		AddSubmember(2,(char)0xBA);  //triple equal
		AddSubmember(2,(char)0xA0);   //approx equal (equal with dot above)
		AddSubmember(2,'1'); // :=
		AddSubmember(2,'2'); // =:
		AddSubmember(2,'3'); // :<=>
		ToolboxNumMembers++;

		AddSubmember(2,(char)0xC5);  //circled +
		AddSubmember(2,(char)0xC4);  //cyrcled x
		AddSubmember(2,(char)0x24);  //Exists
		AddSubmember(2,(char)0x22);  //All
		AddSubmember(2,(char)0x27);  //??? like 'e' backwards
		AddSubmember(2,(char)0xCE);  //element
		AddSubmember(2,(char)0xCF);  //not element
		AddSubmember(2,(char)0xC7);  //intersection
		AddSubmember(2,(char)0xC8);  //union
		AddSubmember(2,(char)0xC9);  //sub-enum right
		AddSubmember(2,(char)0xCA);  //sub-or-equal-enum right
		AddSubmember(2,(char)0xCC);  //sub-enum left
		AddSubmember(2,(char)0xCD);  //sub-or-equal-enum left
		AddSubmember(2,(char)0xCB);  //not a sub-enum
		AddSubmember(2,(char)0x5C);  //three dots (in triangle formation)
		AddSubmember(2,(char)']');  //complement? 
		AddSubmember(2,(char)0x5B);  //three dots (in upside-down triangle formation)
		AddSubmember(2,'#');   

		ToolboxNumMembers++;

		AddSubmember(2,(char)0xB4);  //vector product 'x'
		AddSubmember(2,(char)0x44);  //delta operator
		AddSubmember(2,(char)0xD1);  //nabla operator
		AddSubmember(2,'r');   //rotor
		AddSubmember(2,'*');   //convolution
		AddSubmember(2,(char)0xF0);  //D'Alambert operator (empty square)
		AddSubmember(2,(char)0xB7);  //big dot
		AddSubmember(2,0x03);   //little circle
		AddSubmember(2,'P');   //the paralel operator
		AddSubmember(2,'p');   //the perpendicular operator
		AddSubmember(2,(char)0xDE); //double arrow right 
		AddSubmember(2,(char)0xDC); //double arrow left
		AddSubmember(2,(char)0xDD); //double arrow up
		AddSubmember(2,(char)0xDF); //double arrow down
		AddSubmember(2,(char)0xDB); //double arrow both way
		AddSubmember(2,(char)0xAE); //arrow right
		AddSubmember(2,(char)0xAC); //arrow left
		AddSubmember(2,(char)0xAD); //arrow up
		AddSubmember(2,(char)0xAF); //arrow down
		AddSubmember(2,(char)0xAB); //arrow both way
		AddSubmember(2,(char)0xE2);  // |-->  arrow
		AddSubmember(2,(char)0xE3);  // -->  arrow with expression over it
		ToolboxNumMembers++;

		AddSubmember(8,1);AddAccelerator('R');   //without expression2
		AddSubmember(8,0);   //root
		AddSubmember(8,'3'); //cube root
		AddSubmember(8,9);   //sqare root of two
		AddSubmember(3,0); //exponent (power) function
		AddSubmember(103,0); //exponent (power) function
		AddSubmember(3,'2'); //square
		AddSubmember(103,'2'); //square
		//AddSubmember(3,'3'); //qube
		//AddSubmember(103,'3'); //qube
		AddSubmember(3,1); //invers
		AddSubmember(103,1); //invers
		AddSubmember(3,2); //invers square
		AddSubmember(103,2); //invers square
		//AddSubmember(3,3); //invers square
		//AddSubmember(103,3); //invers square
		AddSubmember(3,4);  //e^x
		AddSubmember(3,5);  //10^x
		ToolboxNumMembers++;


		AddSubmember(4,0);  //standard rational number
		AddSubmember(4,10); // X / 2
		AddSubmember(4,'/'); //rational number with diagonal line
		AddSubmember(4,' '); //a over b
		AddSubmember(4,1); //derivation
		AddSubmember(4,2); //derivation with parentheses
		AddSubmember(4,3); //second derivation
		AddSubmember(4,4); //second derivation with parentheses
		AddSubmember(4,5); //partial derivation
		AddSubmember(4,6); //partial derivation with parentheses
		AddSubmember(4,7); //second partial derivation
		AddSubmember(4,8); //second partial derivation with parentheses
		AddSubmember(4,'d'); //derivation d/d()
		AddSubmember(4,'p'); //partial derivation par/par()
		AddSubmember(10,3); //vertical line with condition list (as an element) - three subexpressions, two vertical lines
		AddSubmember(10,1); //vertical line with condition list (as an element) - three subexpressions, signle vertical line
		AddSubmember(10,7); //vertical line with condition list (as an element) - two subexpressions, two vertical lines
		AddSubmember(10,5); //vertical line with condition list (as an element) - two subexpressions, signle vertical line
		AddSubmember(9,0); //vertical line with condition list - single line
		AddSubmember(10,4); //vertical line with condition list (as an element) - two subexpressions, no vertical line

		ToolboxNumMembers++;

		AddSubmember(5,'('); //parentheses
		AddSubmember(5,'[');
		AddSubmember(5,'{');
		AddSubmember(5,'|');
		AddSubmember(5,'<');
		AddSubmember(5,'\\');
		AddSubmember(5,'l'); //left curly parentheses
		AddSubmember(5,'r'); //right curly parentheses
		AddSubmember(5,(char)(0x80|0x01)); //vector vertical, 1x2
		AddSubmember(5,(char)(0x80|0x02)); //vector vertical, 1x3
		//AddSubmember(5,(char)(0x80|0x03)); //vector vertical, 1x4
		//AddSubmember(5,(char)(0x80|0x04)); //vector vertical, 1x5
		AddSubmember(5,(char)(0x80|0x10)); //vector horizontal, 2x1
		AddSubmember(5,(char)(0x80|0x20)); //vector horizontal, 3x1
		//AddSubmember(5,(char)(0x80|0x30)); //vector horizontal, 4x1
		//AddSubmember(5,(char)(0x80|0x40)); //vector horizontal, 5x1
		AddSubmember(5,(char)(0x80|0x11)); //matrix, 2x2
		AddSubmember(5,(char)(0x80|0x22)); //matrix, 3x3
		//AddSubmember(5,(char)(0x80|0x33)); //matrix, 4x4
		//AddSubmember(5,(char)(0x80|0x44)); //matrix, 5x5
		AddSubmember(1,6);
		AddSubmember(1,7);
		AddSubmember(1,8);
		AddSubmember(1,9);
		ToolboxNumMembers++;

		AddSubmember(7,'S'); //sigma
		//AddSubmember(107,'S'); //sigma
		AddSubmember(7,'s'); //sigma
		//AddSubmember(107,'s'); //sigma
		AddSubmember(7,'P'); //pi
		//AddSubmember(107,'P'); //pi
		AddSubmember(7,'p'); //pi
		//AddSubmember(107,'p'); //pi
		AddSubmember(7,'I'); //integral
		AddSubmember(7,'O'); //circular integral
		AddSubmember(7,'i'); //integral - non-determined
		AddSubmember(7,'o'); //circular integral - non-determined
		AddSubmember(7,'I'-3); //double integral 
		AddSubmember(7,'O'-3); //double circular integral 
		AddSubmember(7,'I'-4); //triple integral
		AddSubmember(7,'O'-4); //triple circular integral
		AddSubmember(7,'I'-1); AddAccelerator('I'); //integral - with dx
		//AddSubmember(7,'I'-2); //integral - with ()dx
		AddSubmember(7,'i'-1); //integral - non-determined - with dx
		//AddSubmember(7,'i'-2); //integral - non-determined - with dx
		AddSubmember(7,'|'); //bar
		AddSubmember(7,'/'); //right bar

		ToolboxNumMembers++;

		AddSubmember(6,17); //ln
		//AddSubmember(106,17); //ln
		AddSubmember(6,25); //limes
		//AddSubmember(106,25); //limes
		AddSubmember(6,18); //log10
		//AddSubmember(106,18); //log10
		AddSubmember(6,19); AddAccelerator('L'); //log
		//AddSubmember(106,19); //log
		AddSubmember(6,20); //rotor
		AddSubmember(106,37); //dirac delta
		//AddSubmember(106,20); //rotor
		AddSubmember(6,21); //nabla
		AddSubmember(106,21); //nabla
		AddSubmember(6,22); //delta
		AddSubmember(106,22); //delta
		AddSubmember(106,38); //Real part
		AddSubmember(106,39); //Imaginary part
		AddSubmember(6,23);AddAccelerator('D'); //d (derivation)
		AddSubmember(106,23); //d (derivation)
		AddSubmember(6,24); //partial derivation
		AddSubmember(106,24); //partial derivation
		AddSubmember(106,'f'); AddAccelerator('F'); //general function f
		AddSubmember(106,'g'); AddAccelerator('G');//general function g
		ToolboxNumMembers++;

		AddSubmember(6,1);   //sin
		AddSubmember(6,5);   //asin
		AddSubmember(6,2);   //cos
		AddSubmember(6,6);   //acos
		AddSubmember(6,3);   //tg
		AddSubmember(6,7);   //atg
		AddSubmember(6,26);   //tan
		AddSubmember(6,40);   //atg
		AddSubmember(6,4);   //ctg
		AddSubmember(6,8);   //actg
		AddSubmember(6,27);   //sec
		AddSubmember(6,30);   //arcsec
		AddSubmember(6,29);   //csc
		AddSubmember(6,31);   //arccsc
		ToolboxNumMembers++;


		AddSubmember(6,9);   //sh
		AddSubmember(6,13);   //ash
		AddSubmember(6,10);   //ch
		AddSubmember(6,14);   //ach
		AddSubmember(6,11);   //th
		AddSubmember(6,15);   //ath
		AddSubmember(6,12);   //cth
		AddSubmember(6,16);   //acth
		AddSubmember(6,32);   //sech
		AddSubmember(6,34);   //arsech
		AddSubmember(6,33);   //csch
		AddSubmember(6,35);   //arcsch
		ToolboxNumMembers++;

		//AddSubmember(5,'T'); AddAccelerator('T'); //Text box (use 't' for textbox without parentheses )
		AddSubmember(201,(char)0xA5); //infinite
		AddSubmember(1,13); //permille
		AddSubmember(1,4);  //e
		AddSubmember(1,5);  //pi
		AddSubmember(201,(char)0xA6);
		//AddSubmember(201,(char)0xA2);
		AddSubmember(1,11); //imaginary unit
		AddSubmember(201,(char)0xC0); //aleph
		AddSubmember(201,(char)0xC1);
		AddSubmember(201,(char)0xC2);
		AddSubmember(201,(char)0xC3);
		AddSubmember(201,(char)0xC6);
		AddSubmember(2,(char)0xD0); //angle
		AddSubmember(2,(char)0xBC);  //triple dot - note that this is an operator (not variable)
		//AddSubmember(201,(char)0xA1); //pisani Y
		AddSubmember(2,(char)0x9E); //triangle
		AddSubmember(1,(char)0x24); //$
		AddSubmember(1,(char)0xA3); //pound
		AddSubmember(1,'€');
		AddSubmember(1,'@');
		AddSubmember(201,(char)0xB0); //degree
		AddSubmember(1,10); //degree celsius
		
		AddSubmember(9,'H'); //hyperlink
		AddSubmember(9,'L'); //label

		ToolboxNumMembers++;

		AddDrawingSubmember(3,30,30); //freehand Line
		AddDrawingSubmember(2,30,30); //Line
		AddDrawingSubmember(1,30,30); //rectangle
		AddDrawingSubmember(4,30,30); //circle
		AddDrawingSubmember(7,30,30); //eraser, small
		AddDrawingSubmember(6,30,30); //eraser, large		
		AddDrawingSubmember(8,30,30); //coordinate system
		AddDrawingSubmember(9,30,30); //coordinate system, central
		AddDrawingSubmember(10,30,30); //raster, linear x, linear y
		AddDrawingSubmember(11,30,30); //raster, log x, linear y
		AddDrawingSubmember(12,30,30); //raster, linear x, log y
		AddDrawingSubmember(13,30,30); //raster, log x, log y
		AddDrawingSubmember(15,30,30); //5-sided polygon
		AddDrawingSubmember(16,30,30); //6-sided polygon
		AddDrawingSubmember(17,15,15); //center-drawn circle
		AddDrawingSubmember(27,15,15); //center-drawn rectangle
		AddDrawingSubmember(28,30,30); //sinus function graph
		AddDrawingSubmember(29,30,30); //x^2 function graph
		AddDrawingSubmember(30,30,30); //triangle 90 degree
		AddDrawingSubmember(31,30,30); //triangle - two sides equal
		AddDrawingSubmember(32,30,-28); //sector of circle
		AddDrawingSubmember(33,30,20); //segment of circle

		ToolboxNumMembers++;

		AddDrawingSubmember(18,30,30); //section breaker - horizontal line
		AddDrawingSubmember(19,10,30); //parenthese
		AddDrawingSubmember(5,30,30); //arrow
		AddDrawingSubmember(14,30,30); //freehand arrow
		AddDrawingSubmember(20,20,20); //Okay sign
		AddDrawingSubmember(21,20,20); //exclamation mark
		AddDrawingSubmember(22,20,20); //question mark
		AddDrawingSubmember(23,20,20); //star
		AddDrawingSubmember(25,30,30); //diagonals \ \ \ ...
		AddDrawingSubmember(26,30,30); //diagonals / / / ...
		AddDrawingSubmember(50,20,20); //drawing box
		AddDrawingSubmember(51,20,20); //function ploter

		AddDrawingSubmember(52,20,20); //bitmap image


		ToolboxNumMembers++;

		memset((void*)&ToolboxFontFormating,0,(size_t)sizeof(ToolboxFontFormating));
		AddFontFormating(0,0,0,0); //sans-serif
		AddFontFormating(0,0,1,0); //sans-serif, italic

		AddFontFormating(1,0,0,0); //serif
		AddFontFormating(1,0,1,0); //serif, italic

		AddFontFormating(2,0,0,0); //monospaced, arrow

		AddFontFormating(0,1,0,0); //sans-serif, bold, italic,
		AddFontFormating(1,0,1,2); //serif, italic, arrow
		AddFontFormating(1,0,1,3); //serif, italic, hat

		ToolboxFontFormating.MixedFormat=new CExpression(NULL,NULL,100);
		ToolboxFontFormating.MixedFormat->InsertEmptyElement(0,1,'M');
		ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0]=0x22; //Italic, serif (by default)
		ToolboxFontFormating.SelectedUniform=0;
		m_FontModeElement=-1;
		m_FontModeSelection=1;
		ToolboxFontFormating.UniformAccKey[0]='U';
		ToolboxFontFormating.MixedAccKey='M';

		memset((void*)&ToolboxKeyboardElements,0,sizeof(ToolboxKeyboardElements));
		AddKeyboardKey(2,10,'A');
		AddKeyboardKey(47,20,'B');
		AddKeyboardKey(27,20,'C');
		AddKeyboardKey(22,10,'D');
		AddKeyboardKey(20,0,'E');
		AddKeyboardKey(32,10,'F');
		AddKeyboardKey(42,10,'G');
		AddKeyboardKey(52,10,'H');
		AddKeyboardKey(70,0,'I');
		AddKeyboardKey(62,10,'J');
		AddKeyboardKey(72,10,'K');
		AddKeyboardKey(82,10,'L');
		AddKeyboardKey(67,20,'M');
		AddKeyboardKey(57,20,'N');
		AddKeyboardKey(80,0,'O');
		AddKeyboardKey(90,0,'P');
		AddKeyboardKey(0,0,'Q');
		AddKeyboardKey(30,0,'R');
		AddKeyboardKey(12,10,'S');
		AddKeyboardKey(40,0,'T');
		AddKeyboardKey(60,0,'U');
		AddKeyboardKey(37,20,'V');
		AddKeyboardKey(10,0,'W');
		AddKeyboardKey(17,20,'X');
		AddKeyboardKey(50,0,'Y');
		AddKeyboardKey(7,20,'Z');
	}

	if (IsSubtoolbox==0)
	{
		LoadSettings(NULL);
		//init symbolic calculator (to be called only once)
		ToolboxMembers[0].Submembers[0]->InitCalculator();

		Subtoolbox=new CToolbox(1);
		Subtoolbox->m_IsSubtoolbox=-1;
		Subtoolbox->CreateEx(/*WS_EX_TOPMOST*/0,AfxRegisterWndClass(CS_OWNDC),"Subtoolbox",WS_POPUP,5,5,10,10,theApp.m_pMainWnd->m_hWnd,NULL,0);
		Subtoolbox->ShowWindow(SW_HIDE);

		ContextMenu=new CToolbox(2);
		ContextMenu->m_IsContextMenu=1;
		ContextMenu->CreateEx(WS_EX_TOPMOST,AfxRegisterWndClass(CS_OWNDC),"Context Menu",WS_POPUP,5,5,10,10,theApp.m_pMainWnd->m_hWnd,NULL,0);
		ContextMenu->ShowWindow(SW_HIDE);

		Toolbar=new CToolbox(4);
		Toolbar->m_IsToolbar=1;
		Toolbar->CreateEx(/*WS_EX_TOPMOST*/0,AfxRegisterWndClass(CS_OWNDC),"Toolbar",WS_CHILD,5,5,10,10,theApp.m_pMainWnd->m_hWnd,NULL,0);
		Toolbar->ShowWindow(SW_HIDE);

		Keyboard=new CToolbox(3);
		Keyboard->m_IsKeyboard=1;
		Keyboard->CreateEx(/*WS_EX_TOPMOST*/WS_EX_TOOLWINDOW,AfxRegisterWndClass(CS_OWNDC),"Virtual Keyboard",WS_SYSMENU|WS_CAPTION| WS_POPUP,5,5,10,10,theApp.m_pMainWnd->m_hWnd,NULL,0);
		Keyboard->ShowWindow(SW_HIDE);

		Keyboard->m_KeyboardX=-1;
		Keyboard->m_KeyboardY=-1;

		m_IsMain=1;
		
	}
}

#pragma optimize("s",on)
int CToolbox::AddAccelerator(short code)
{
	ToolboxMembers[ToolboxNumMembers].AcceleratorKey[ToolboxMembers[ToolboxNumMembers].NumSubmembers-1]=code;
	return 0;
}

#pragma optimize("s",on)
int CToolbox::AddKeyboardKey(int x, int y, char key)
{
	int i=ToolboxKeyboardElements.NumKeys;
	ToolboxKeyboardElements.Key[i]=new CExpression(NULL,NULL,100);
	ToolboxKeyboardElements.Key[i]->InsertEmptyElement(0,1,key);
	//ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data2[0]=0x02;
	ToolboxKeyboardElements.code[i]=(short)key;
	ToolboxKeyboardElements.X[i]=x;
	ToolboxKeyboardElements.Y[i]=y;
	ToolboxKeyboardElements.FormatingBigCaps[i]=0x22; //by default we set everything to serif italic
	ToolboxKeyboardElements.FormatingSmallCaps[i]=0x22;
	ToolboxKeyboardElements.FormatingBigGreek[i]=0x02;
	ToolboxKeyboardElements.FormatingSmallGreek[i]=0x02;
	ToolboxKeyboardElements.FormatingBigCaps2[i]=0x00; //by default we have no decorations
	ToolboxKeyboardElements.FormatingSmallCaps2[i]=0x00;
	ToolboxKeyboardElements.FormatingBigGreek2[i]=0x00;
	ToolboxKeyboardElements.FormatingSmallGreek2[i]=0x00;
	ToolboxKeyboardElements.NumKeys++;
	return 0;
}

#pragma optimize("s",on)
int CToolbox::AddFontFormating(int FaceType, int IsBold, int IsItalic, int Modificator)
{
	int i=ToolboxFontFormating.NumFormats;
	ToolboxFontFormating.UniformFormats2[i]=(char)0x80;
	ToolboxFontFormating.UniformFormats[i]=new CExpression(NULL,NULL,100);
	ToolboxFontFormating.UniformFormats[i]->InsertEmptyElement(0,1,'U');
	ToolboxFontFormating.UniformFormats[i]->m_pElementList->pElementObject->Data2[0]=
		(FaceType<<5)+(IsItalic<<1)+IsBold;
	ToolboxFontFormating.UniformFormats[i]->m_pElementList->pElementObject->m_VMods=Modificator<<2;

	if ((Modificator) || (FaceType==3)) 
		ToolboxFontFormating.UniformFormats2[i]=(char)0x81;

	ToolboxFontFormating.NumFormats++;
	return 0;
}

#pragma optimize("s",on)
int CToolbox::AddSubmember(short Type, char data)
{
	int member=ToolboxNumMembers;
	int forceparentheses=0;
	int greek=0;
	if (Type>200) {Type-=200;greek=1;}
	if (Type>100) {Type-=100;forceparentheses=1;}
	ToolboxMembers[member].Submembers[ToolboxMembers[member].NumSubmembers]=new CExpression(NULL,NULL,100);
	ToolboxMembers[member].Submembers[ToolboxMembers[member].NumSubmembers]->InsertEmptyElement(0,Type,data);
	ToolboxMembers[member].CreationCode[ToolboxMembers[member].NumSubmembers]=Type*256+data;
	if (forceparentheses)
	{
		//forces parentheses in expression1 of the element;
		CExpression *e1=(CExpression*)(ToolboxMembers[member].Submembers[ToolboxMembers[member].NumSubmembers]->m_pElementList->pElementObject->Expression1);
		if (e1) 
			e1->m_ParenthesesFlags|=1;
	}
	if (greek)
	{
		ToolboxMembers[member].Submembers[ToolboxMembers[member].NumSubmembers]->m_pElementList->pElementObject->Data2[0]|=0x60;
	}
	ToolboxMembers[member].NumSubmembers++;
	return 0;
}

#pragma optimize("s",on)
int CToolbox::AddDrawingSubmember(int form, int Cx, int Cy)
{
	int member=ToolboxNumMembers;
	ToolboxMembers[member].Submembers[ToolboxMembers[member].NumSubmembers]=(CExpression*)new CDrawing();
	((CDrawing*)ToolboxMembers[member].Submembers[ToolboxMembers[member].NumSubmembers])->InsertEmptyElement(form,Cx,Cy);
	ToolboxMembers[member].Above[ToolboxMembers[member].NumSubmembers]=-1; //indicates that this is drawing
	ToolboxMembers[member].NumSubmembers++;
	return 0;
}

#pragma optimize("s",on)
CToolbox::~CToolbox()
{
	if (m_IsMain)
	{
		delete Subtoolbox;
		delete ContextMenu;
		delete Keyboard;
		int i,j;
		for (i=0;i<ToolboxNumMembers;i++)
			for (j=0;j<ToolboxMembers[i].NumSubmembers;j++)
			{
				if (ToolboxMembers[i].Above[j]==-1)
					delete ((CDrawing*)(ToolboxMembers[i].Submembers[j]));
				else
					delete ToolboxMembers[i].Submembers[j];
			}
		for (i=0;i<ToolboxFontFormating.NumFormats;i++)
		{
			delete ToolboxFontFormating.UniformFormats[i];
		}
		delete ToolboxFontFormating.MixedFormat;
		for (i=0;i<ToolboxKeyboardElements.NumKeys;i++)
			delete ToolboxKeyboardElements.Key[i];
	}
	if (LanguageStrings)
	{
		free(LanguageStrings);LanguageStrings=0;
		if (LanguagePointers) free(LanguagePointers);
		LanguagePointers=0;
	}
	
}

#pragma optimize("",on)

BEGIN_MESSAGE_MAP(CToolbox, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEWHEEL()
//	ON_WM_ACTIVATE()
//	ON_WM_MOUSEACTIVATE()
	ON_WM_CHAR()
	ON_WM_CLOSE()
	ON_COMMAND(ID_SAVEOPTIONS_SAVEASDEFAULT, OnSaveoptionsSaveasdefault)
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
END_MESSAGE_MAP()

char greeksymbolsactivated;
char textmodeactivated;
int ToolboxDisableDeselect;
void CToolbox::PaintToolboxHeader(CDC *dc)
{
	//painting main toolbox header (font data)

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(dc,ToolboxSize,ToolboxSize/2);
	CDC pdc;
	pdc.CreateCompatibleDC(dc);
	pdc.SelectObject(bmp);

	short l,a,b;
	short Cx,Cy;

	//deselect everything
	for (int i=0;i<ToolboxFontFormating.NumFormats;i++)
	{
		if (!Popup->IsWindowVisible())
		{
			ToolboxFontFormating.UniformFormats[i]->DeselectExpression();
		}
	}
	if (!Popup->IsWindowVisible()) ToolboxFontFormating.MixedFormat->DeselectExpression();


	//paint background
	if (dc->GetDeviceCaps(BITSPIXEL)>16)
	{
		for (int i=0;i<ToolboxSize/2;i+=3)
		{
			int k=i/2+1;
			k=255-k;
			pdc.FillSolidRect(1,i,ToolboxSize-2,3,RGB(k,k,255));
		}
	}
	else
		pdc.FillSolidRect(1,0,ToolboxSize-2,ToolboxSize/2,RGB(255,255,255));

	//Paint the unifrom formatting icon
	ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->CalculateSize(dc,14*ToolboxSize/8,&l,&a,&b,0,1);
	Cy=ToolboxSize/4-ToolboxSize/25;
	Cx=ToolboxSize/4-l/2-ToolboxSize/24; if (ToolboxSize>80) Cx+=1;
	if ((m_FontModeElement==0) && (!m_IsArrowSelected))
		pdc.FillSolidRect(1,0,ToolboxSize/2-1,ToolboxSize/2,RGB(200,200,255));
	ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->PaintExpression(&pdc,14*ToolboxSize/8,Cx,Cy);

	if (ToolboxFontFormating.UniformFormats2[ToolboxFontFormating.SelectedUniform]&0x01)
	{
		//paint the down-arrow for singleshot formats
		pdc.SelectObject(GetStockObject(BLACK_PEN));
		int rr=ToolboxSize/16;
		int xx=ToolboxSize/2-rr-2;
		int yy=rr;
		pdc.MoveTo(xx,yy);
		pdc.LineTo(xx,yy+2*rr);
		pdc.LineTo(xx-rr,yy+rr);
		pdc.MoveTo(xx,yy+2*rr);
		pdc.LineTo(xx+rr,yy+rr);
	}
	if ((m_FontModeSelection==0) || (textmodeactivated))
		PaintCheckedSign(&pdc,ToolboxSize/5+ToolboxSize/32,ToolboxSize/2-ToolboxSize/5,ToolboxSize/6,1);

	//Paint the mixed formatting
	ToolboxFontFormating.MixedFormat->CalculateSize(&pdc,14*ToolboxSize/8,&l,&a,&b,0,1);
	Cx=3*ToolboxSize/4-l/2-ToolboxSize/24; if (ToolboxSize>80) Cx+=1;
	if ((m_FontModeElement==1) && (!m_IsArrowSelected))
		pdc.FillSolidRect(ToolboxSize/2,0,ToolboxSize/2-1,ToolboxSize/2,RGB(200,200,255));
	ToolboxFontFormating.MixedFormat->PaintExpression(&pdc,14*ToolboxSize/8,Cx,Cy);
	if (m_FontModeSelection==1) 
		PaintCheckedSign(&pdc,ToolboxSize/2+ToolboxSize/5+ToolboxSize/32,ToolboxSize/2-ToolboxSize/5,ToolboxSize/6,(textmodeactivated==1)?2:1);

	//paint borderlines
	pdc.SelectObject(GetPenFromPool(1,0,RGB(96,96,96)));
	int ttt=0;
	pdc.MoveTo(ToolboxSize/2+ttt-1,0);pdc.LineTo(ToolboxSize/2+ttt-1,ToolboxSize/2);
	pdc.MoveTo(0,ToolboxSize/2);pdc.LineTo(ToolboxSize,ToolboxSize/2);

	//paint shadowed triangles
	int i;
	for (i=0;i<2;i++)
	{
		int IsBlue=0;
		if ((i==m_FontModeElement) && (m_IsArrowSelected)) IsBlue=1;
		int Lx=ToolboxSize/2;
		int Ly=ToolboxSize/2-1;

		int kk=1;
		//if ((ToolboxSize<56) && (i==0)) kk=2;

		for (int j=ToolboxSize/5-2;j>=0;j-=4)
		{
			if (m_FontModeElement!=i)
			{
				pdc.SelectObject(GetPenFromPool(1,0,RGB(255,255,255)));
				pdc.MoveTo(Lx*(i+1)-j-2-kk,Ly-0);
				pdc.LineTo(Lx*(i+1)-0-kk,Ly-j-2);
			}

			pdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(176,176,255)));
			pdc.MoveTo(Lx*(i+1)-j-1-kk,Ly-0);
			pdc.LineTo(Lx*(i+1)-0-kk,Ly-j-1);
			pdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(176,176,255)));
			pdc.MoveTo(Lx*(i+1)-j-0-kk,Ly-0);
			pdc.LineTo(Lx*(i+1)-0-kk,Ly-j-0);
			if (j)
			{
				pdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(220,220,255)));
				pdc.MoveTo(Lx*(i+1)-j+1-kk,Ly-0);
				pdc.LineTo(Lx*(i+1)-0-kk,Ly-j+1);
			}
		}
	}


	dc->BitBlt(0,0,ToolboxSize,ToolboxSize/2,&pdc,0,0,SRCCOPY);
}

// CToolbox message handlers
int prevContextMenuSelection;
void CToolbox::OnPaint()
{
	CPaintDC pdc(this); // device context for painting
	RECT ClientRect;
	this->GetClientRect(&ClientRect);

	if (m_IsMain)
	{
		int i;
		while (PrintRendering) return;
		if (ToolboxNumMembers&0x01) //if odd number of items, we need to paint one empty place
			pdc.FillSolidRect(ToolboxSize/2,(ToolboxNumMembers-1)/2*m_ItemHeight+ToolboxSize/2,ToolboxSize/2,m_ItemHeight,RGB(255,255,255));

		PaintToolboxHeader(&pdc);

		for (i=0;i<ToolboxNumMembers;i++)
		{
			PaintToolboxElement(&pdc,i,0);
		}
		if ((IsDrawingMode) || (NumSelectedObjects)) PaintColorbox(&pdc);
		else if ((KeyboardEntryBaseObject) && (KeyboardEntryObject) && (IsDrawingMode==0)) PaintTextcontrolbox(&pdc);
		else PaintColorbox(&pdc);
		if (UseToolbar) ConfigureToolbar();

		return;
	}
	else if (m_IsSubtoolbox)
	{
		pdc.FillSolidRect(0,0,ClientRect.right,ClientRect.bottom,RGB(255,255,255));
		int i;
		if (m_IsSubtoolbox==-4)
		{
			for (i=0;i<9;i++)
				PaintToolboxElement(&pdc,i,0);
		}
		else if (m_IsSubtoolbox>0)
		{
			for (i=0;i<ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers;i++)
			{
				PaintToolboxElement(&pdc,i,0);
			}
		}
		else if (m_IsSubtoolbox==-2)
		{
			//painting subtoolbox for mixed fonts
			for (i=0;i<4;i++)
				for (int j=0;j<ToolboxKeyboardElements.NumKeys;j++)
					PaintToolboxElement(&pdc,i*100+j,0);
			PaintToolboxElement(&pdc,400,0); //title line and reset
		}
		else if (m_IsSubtoolbox==-1)
		{
			//painting subtoolbox for uniform fonts
			for (i=0;i<ToolboxFontFormating.NumFormats;i++)
			{
				PaintToolboxElement(&pdc,i,0);
			}
		}
		return;
	}
	else if (m_IsKeyboard)
	{
		pdc.FillSolidRect(0,0,ClientRect.right,ClientRect.bottom,SHADOW_BLUE_COLOR2);
		int i;
		for (i=0;i<ToolboxKeyboardElements.NumKeys;i++)
		{
			PaintKeyboardElement(&pdc,i);
		}
		if (AlternateVirtualKeyboard) PaintKeyboardElement(&pdc,255);
		
		return;
	}
	else if (m_IsToolbar)
	{
		PaintToolbar(&pdc);
		return ;
	}


	CDC dc;
	dc.CreateCompatibleDC(&pdc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&pdc,ClientRect.right,ClientRect.bottom);
	dc.SelectObject(bmp);

	if (m_IsContextMenu)
	{
		int TS=30+ToolboxSize/2;
		dc.FillSolidRect(0,0,ClientRect.right,ClientRect.bottom,SHADOW_BLUE_COLOR);

		int i=0;
		int SelectedKeycode=-1;
		dc.SelectObject(GetPenFromPool(1,0,SHADOW_BLUE_COLOR2));
		dc.MoveTo(0,0);
		dc.LineTo(ClientRect.right-1,0); dc.LineTo(ClientRect.right-1,ClientRect.bottom-1);
		dc.LineTo(0,ClientRect.bottom-1); dc.LineTo(0,0);

		dc.SetTextAlign(TA_TOP);

		if ((m_ContextMenuMember>=0) && (m_ContextMenuMember<ToolboxNumMembers) &&
			(m_ContextMenuSubmember>=0) && (m_ContextMenuSubmember<ToolboxMembers[m_ContextMenuMember].NumSubmembers))
		{
			SelectedKeycode=ToolboxMembers[m_ContextMenuMember].AcceleratorKey[m_ContextMenuSubmember];
		}
		if (m_ContextMenuMember==-1)
			SelectedKeycode=ToolboxFontFormating.UniformAccKey[m_ContextMenuSubmember];
		if (m_ContextMenuMember==-2)
			SelectedKeycode=ToolboxFontFormating.MixedAccKey;

		int is_header=0;
		int is_drawing=0;
		if (m_ContextMenuMember<0)
		{
			is_header=1;
		}
		else if (ToolboxMembers[m_ContextMenuMember].Above[m_ContextMenuSubmember]==-1)
		{
			is_drawing=1;
		}
	

		while (ToolboxKeycodes[i].code!=0)
		{
			unsigned short Keycode=ToolboxKeycodes[i].code;

			char string[40];
			sprintf(string,"Ctrl+%s",ToolboxKeycodes[i].name);
			if (Keycode&0x200) sprintf(string,"Ctrl+Shft+%s",ToolboxKeycodes[i].name);
			if (Keycode&0x400) sprintf(string,"\'%s\'",ToolboxKeycodes[i].name);


			if (Keycode==1)
			{
				CopyTranslatedString(string,"EXIT",0,39);
			}
			if (Keycode==2)
			{
				if (is_header)
				{
					CopyTranslatedString(string,"FONT...",2500,39);
				}
				else if (!is_drawing)
				{
					CopyTranslatedString(string,"Easycast..",2501,39);
				}
				else string[0]=0;
			}
			if (Keycode==3)
			{

				if ((m_ContextMenuMember>=0) && (m_ContextMenuSubmember>=0) && (ToolboxMembers[m_ContextMenuMember].userdef_mask&(1<<m_ContextMenuSubmember)))
					CopyTranslatedString(string,"Delete",501,39);
				else
					string[0]=0;  //separator
			}
			if (Keycode==4)
			{
				string[0]=0;  //separator
			}
			//if (Keycode>3)
			//	dc.SelectObject(GetFontFromPool(4,0,(i==m_ContextMenuSelection)?1:0,TS/4-1));
			//else
			dc.SelectObject(GetFontFromPool(4,0,0,TS/4-1));

			int used_keycode=IsAcceleratorUsed(Keycode);
			

			if (i==m_ContextMenuSelection)
				dc.SetTextColor(BLUE_COLOR);
			else
				dc.SetTextColor((used_keycode)?RGB(140,140,140):0);
			if (((is_header) || (is_drawing)) && (Keycode&0x400)) dc.SetTextColor(RGB(200,200,200));

			{
				int xpos=0;
				if (i>=23) xpos+=TS;
				if (i>=46) xpos+=7*TS/8;
				if (i>=69) xpos+=10*TS/8;
				if (i>=92) xpos+=10*TS/8;
				if (i>=115) xpos+=11*TS/8;
				if (i>=138) xpos+=5*TS/8;

				int ypos=(i%23)*(TS/4-1)+5;

				dc.TextOut(xpos+TS/5,ypos,string);
				if ((Keycode<=3) || (i==m_ContextMenuSelection))
				{
					dc.SetBkMode(TRANSPARENT);
					dc.TextOut(xpos+TS/5+1,ypos,string);
					dc.SetBkMode(OPAQUE);
				}
				if (SelectedKeycode==Keycode) PaintCheckedSign(&dc,xpos+2,ypos,TS/5,1);
			}
			i++;
		}
	}


	pdc.BitBlt(0,0,ClientRect.right,ClientRect.bottom,&dc,0,0,SRCCOPY);
	// Do not call CWnd::OnPaint() for painting messages
}
void CToolbox::ToolboxChangeIndividualKeyFont(void)
{
	int row=m_SelectedElement/100;
	int key=m_SelectedElement%100;

	if ((row==4) && (m_IsArrowSelected))
	{
		//clicked at 'reset all'
		m_ContextMenuMember=-2;

		DefineAcceleratorCode(2);

		return;
	}
	if (row>3) return;
	if (key>ToolboxKeyboardElements.NumKeys-1) return;

	unsigned char fdata=0;
	unsigned char vmods=0;
	char symbol=0;
	char prevsymbol=ToolboxKeyboardElements.Key[key]->m_pElementList->pElementObject->Data1[0];
	fdata=ToolboxKeyboardElements.Key[key]->m_pElementList->pElementObject->Data2[0];
	vmods=ToolboxKeyboardElements.Key[key]->m_pElementList->pElementObject->m_VMods;
	if (row==0) {fdata=ToolboxKeyboardElements.FormatingBigCaps[key];vmods=ToolboxKeyboardElements.FormatingBigCaps2[key];symbol=toupper(prevsymbol);}
	if (row==1) {fdata=ToolboxKeyboardElements.FormatingSmallCaps[key];vmods=ToolboxKeyboardElements.FormatingSmallCaps2[key];symbol=tolower(prevsymbol);}
	if (row==2) {fdata=ToolboxKeyboardElements.FormatingBigGreek[key];fdata=fdata&0x1F;fdata=fdata|0x60;vmods=ToolboxKeyboardElements.FormatingBigGreek2[key];symbol=toupper(prevsymbol);}
	if (row==3) {fdata=ToolboxKeyboardElements.FormatingSmallGreek[key];fdata=fdata&0x1F;fdata=fdata|0x60;vmods=ToolboxKeyboardElements.FormatingSmallGreek2[key];symbol=tolower(prevsymbol);}
	if (ToolboxCharacter) delete ToolboxCharacter;
	ToolboxCharacter=new CExpression(NULL,NULL,100);
	ToolboxCharacter->InsertEmptyElement(0,1,symbol);
	ToolboxCharacter->m_pElementList->pElementObject->Data2[0]=fdata;
	ToolboxCharacter->m_pElementList->pElementObject->m_VMods=vmods;


	Toolbox->m_FontModeSelection=1; 
	Toolbox->AdjustKeyboardFont();
	Toolbox->PaintToolboxHeader(Toolbox->GetDC());
	
	ToolboxCharacter->SelectExpression(1);
	ToolboxCharacter->m_Selection=0;
	FontAdditionalData=0;
	Popup->ShowPopupMenu(ToolboxCharacter,(CWnd*)this,0,23);
	//the PopupCloses function will be called after popupmenu closs
}

int CToolbox::IsAcceleratorUsed(unsigned short Keycode)
{
	for (int kkk=0;kkk<ToolboxNumMembers;kkk++)
		for (int lll=0;lll<ToolboxMembers[kkk].NumSubmembers;lll++)
			if (ToolboxMembers[kkk].AcceleratorKey[lll]==Keycode) 
			return 1;
	for (int kkk=0;kkk<ToolboxFontFormating.NumFormats;kkk++)
	{
		if (ToolboxFontFormating.UniformAccKey[kkk]==Keycode) return 1;
	}
	if (ToolboxFontFormating.MixedAccKey==Keycode) return 1;
	return 0;
}
void CToolbox::AdjustPosition(void)
{
	if (!theApp.m_pMainWnd->IsWindowVisible()) return;

	if ((ViewOnlyMode) && (Toolbox))
	{
		if ((theApp.m_pMainWnd) && (theApp.m_pMainWnd->IsWindowVisible()))
		{
			Toolbox->ShowWindow(SW_HIDE);
			Toolbox->Toolbar->ShowWindow(SW_HIDE);
			Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
		}
		ToolboxSize=0;
		return;
	}

	RECT wr,cr;
	int border,header;

	if (m_IsMain)
	{
		theApp.m_pMainWnd->GetClientRect(&cr);
		m_ItemHeight=(cr.bottom-ToolboxSize/2-ToolboxSize-7)/((ToolboxNumMembers+1)/2);
		if (m_ItemHeight>2*ToolboxSize/3) m_ItemHeight=2*ToolboxSize/3;
		else if (m_ItemHeight<ToolboxSize/3) m_ItemHeight=ToolboxSize/3;

		int additionals=ToolboxSize+5; //for the colorbox or text-controlbox
		if (m_ItemHeight<ToolboxSize/2) 
		{
			m_LowHeightMode=1; 
			additionals=ToolboxSize*2/3+1+5; //shortened colorbox/text-controlbox
			m_ItemHeight+=ToolboxSize/3/((ToolboxNumMembers+1)/2);

		}
		else 
			m_LowHeightMode=0;
		m_ItemHeight&=0xFFFE; //item height must be an even number for nicer painting

		//if ((theApp.m_pMainWnd) && (theApp.m_pMainWnd->IsWindowVisible()))
		//	if (!Toolbox->IsWindowVisible()) ShowWindow(SW_SHOW);
		//theApp.m_pMainWnd->GetWindowRect(&wr);
		//theApp.m_pMainWnd->GetClientRect(&cr);
		//border=((wr.right-wr.left)-(cr.right-cr.left))/2;
		//header=wr.bottom-wr.top-border-(cr.bottom-cr.top);
		//int additionals=0;
		//if ((KeyboardEntryBaseObject) && (KeyboardEntryObject) && (IsDrawingMode==0))
		//	additionals=ToolboxSize+5;
		//else 
		//	additionals=2*ToolboxSize/3+5;
		//int additionals=((m_LowHeightMode)?ToolboxSize/2:ToolboxSize)+5;

		SetWindowPos(NULL, //insert after
			0,//wr.left+border,  //x
			1,//wr.top+header+1,  //y
			ToolboxSize, //cx
			((ToolboxNumMembers+1)/2)*m_ItemHeight+ToolboxSize/2+additionals,
			SWP_NOZORDER | SWP_NOACTIVATE);
		ShowWindow(SW_SHOWNA);
		if (Subtoolbox) Subtoolbox->AdjustPosition();
		if (ContextMenu) ContextMenu->AdjustPosition();
		if (Toolbar) Toolbar->AdjustPosition();
	}
	else if (m_IsSubtoolbox)
	{
		RECT wr;
		int cx,cy;
		Toolbox->GetWindowRect(&wr);
		m_ItemHeight=2*ToolboxSize/3;
		if (m_IsSubtoolbox==-4) //special hand-drawing toolbox
		{
			POINT p;
			GetCursorPos(&p);
			if ((UseToolbar) && (p.y<wr.top+ToolboxSize/2))
			{
				wr.top=p.y-ToolboxSize/8;
				wr.left=p.x-2*ToolboxSize/2+ToolboxSize/8;
			}
			else
			{
				wr.top=p.y-3*ToolboxSize/2+ToolboxSize/8;
				wr.left=p.x-3*ToolboxSize/2+ToolboxSize/8;
			}
			
			cx=3*ToolboxSize/2+1;
			cy=3*ToolboxSize/2+1;
			cx+=2;
			cy+=2;
			
		}
		else if (m_IsSubtoolbox==-2)
		{
			//mixed formatting subtoolbox
			wr.left+=ToolboxSize/2+ToolboxSize/3-ToolboxSize/10;
			wr.top+=ToolboxSize/3+ToolboxSize/5;
			cx=(ToolboxSize/4)*26;
			cy=ToolboxSize+ToolboxSize/3+ToolboxSize/4;
			cx+=2;
			cy+=2;
		}
		else if (m_IsSubtoolbox==-1)
		{
			//uniform formatting subtoolbox 
			wr.left+=ToolboxSize/3-ToolboxSize/10;
			wr.top+=ToolboxSize/3+ToolboxSize/5;
			cx=ToolboxSize*3;
			cy=8*(2*ToolboxSize/5);
			cx+=2;
			cy+=2;
		}
		else
		{
			//subtoolbox for toolbox members
			POINT c;
			GetCursorPos(&c);
			Toolbox->ScreenToClient(&c);
			c.y+=wr.top;

			wr.left+=((m_IsSubtoolbox+1)%2)*ToolboxSize/2+ToolboxSize/2-ToolboxSize/10;
			wr.top+=((m_IsSubtoolbox+1)/2)*2*ToolboxSize/3+ToolboxSize/3+ToolboxSize/5;
			if (wr.top-c.y>ToolboxSize/4) wr.top=c.y+ToolboxSize/4;
			cx=(ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers+1)/2*ToolboxSize/2;
			//cy=4*ToolboxSize/3;
			cy=2*m_ItemHeight;
			cx+=2;
			cy+=2;
		}
		SetWindowPos(NULL ,wr.left,wr.top,cx,cy,SWP_NOZORDER | SWP_NOACTIVATE);
		
	}
	else if (m_IsContextMenu)
	{
		POINT cursor;
		GetCursorPos(&cursor);
		RECT wr;
		::GetWindowRect(GetDesktopWindow()->m_hWnd,&wr);
		if (cursor.x>wr.right) wr.right=GetSystemMetrics(SM_CXVIRTUALSCREEN);
		if (cursor.y>wr.bottom)	wr.bottom=GetSystemMetrics(SM_CYVIRTUALSCREEN);

		cursor.x+=10;
		cursor.y+=10;
		int TS=30+ToolboxSize/2;
		int Xsize=7*TS+TS/4;
		int Ysize=23*(TS/4-1)+TS/8;
		if (cursor.x+Xsize>wr.right) cursor.x=wr.right-Xsize;
		if (cursor.y+Ysize>wr.bottom) cursor.y=wr.bottom-Ysize;
		SetWindowPos(0,cursor.x,cursor.y,Xsize,Ysize,SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else if (m_IsKeyboard)
	{
		RECT wr;
		Toolbox->GetWindowRect(&wr);
		int CorrX,CorrY;
		SetWindowPos(0,wr.right+ToolboxSize/4,wr.top+ToolboxSize/4,80,80,SWP_NOZORDER | SWP_NOACTIVATE);
		RECT cr;
		GetClientRect(&cr);
		CorrX=10*ToolboxSize/3+6-cr.right;
		CorrY=ToolboxSize+6-cr.bottom;
		SetWindowPos(0,wr.right+ToolboxSize/4,wr.top+ToolboxSize/4+((UseToolbar)?ToolboxSize/2:0),80+CorrX,80+CorrY,SWP_NOZORDER);
	}
	else if (m_IsToolbar)
	{
		theApp.m_pMainWnd->GetWindowRect(&wr);
		theApp.m_pMainWnd->GetClientRect(&cr);
		border=((wr.right-wr.left)-(cr.right-cr.left))/2;
		header=wr.bottom-wr.top-border-(cr.bottom-cr.top);

		
		if ((UseToolbar) && (ViewOnlyMode==0) && (pMainView->IsWindowVisible()))
		{
			SetWindowPos(NULL, //insert after
				ToolboxSize,//wr.left+border+ToolboxSize,  //x
				1,//wr.top+header+1,  //y
				cr.right-ToolboxSize, //cx
				ToolboxSize/2-1,
				SWP_NOZORDER | SWP_NOACTIVATE);
			ShowWindow(SW_SHOWNA);
			if (m_IsToolbar)
			{
				ConfigureToolbar();
				InvalidateRect(NULL,0);
			}
		}
		else
			ShowWindow(SW_HIDE);
		if ((Toolbox->Keyboard) && (Toolbox->Keyboard->IsWindowVisible()))
			Toolbox->Keyboard->SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		if ((Toolbox->Subtoolbox) && (Toolbox->Subtoolbox->IsWindowVisible()))
			Toolbox->Subtoolbox->SetWindowPos(&wndTop,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
	}
}
extern int IsALTDown;
extern int IsSHIFTALTDown;
extern int IsSHIFTDown;
extern int prevDrawingMode2;

#pragma optimize("s",on)
void CToolbox::OnLButtonDown(UINT nFlags, CPoint point)
{
	try
	{
		IsWindowOutOfFocus=0;

	if (m_IsMain)
	{
		ToolboxAutoopen=0;

		//clicked on main toolbox
		if (point.y<ToolboxSize/2)
		{
			//clicked on main-toolbox header
			if ((m_FontModeElement>=0) && (m_FontModeElement<2))
			{
				if (m_IsArrowSelected)
				{
					/*if (m_FontModeElement==1)
					{
						
						//opening keyboard
						if (Keyboard->IsWindowVisible())
							Keyboard->ShowWindow(SW_HIDE);
						else
						{
							NoImageAutogeneration=2;
							Keyboard->AdjustPosition();
							Keyboard->InvalidateRect(NULL,0);
							AdjustKeyboardFont();
							NoImageAutogeneration=2;
							Keyboard->ShowWindow(SW_SHOW);
						}
						

					}
					else*/
					{
						Subtoolbox->m_IsSubtoolbox=-m_FontModeElement-1;
						Subtoolbox->AdjustPosition();
						Subtoolbox->InvalidateRect(NULL,0);
						Subtoolbox->UpdateWindow();
						NoImageAutogeneration=2;
						m_IsArrowSelected=0;
						Subtoolbox->m_SelectedElement=-1;
						Subtoolbox->m_IsArrowSelected=0;
						Subtoolbox->ShowWindow(SW_SHOWNA);
					}
				}
				else
				{
					if ((ToolboxFontFormating.UniformFormats2[ToolboxFontFormating.SelectedUniform]&0x01)==0) //if it is not a singleshot mode
					{
						m_prevFontModeSelection=ToolboxFontFormating.SelectedUniform;
					}
					if (m_FontModeElement==1)
						m_prevFontModeSelection|=0x80; //high bit set if mixed mode is seelcted

					m_FontModeSelection=m_FontModeElement;
					if ((m_FontModeSelection==1) && (textmodeactivated) && (KeyboardEntryObject))
					{
						//we are going to select the math typing mode becase user clicked to the mixed formatting (the mixed formatting is only possible in math typing mode)
						CDC *ddc=pMainView->GetDC();
						{int tmp=IsALTDown;IsALTDown=1;((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(ddc,ViewZoom,' ',0,0,0,1);IsALTDown=tmp;}
						pMainView->ReleaseDC(ddc);
					}
					PaintToolboxHeader(this->GetDC());
					AdjustKeyboardFont();
					this->ReformatKeyboardSelection();
				}
			}
		}
		else if ((m_SelectedElement<ToolboxNumMembers) && (m_SelectedElement>=0))
		{
			if (m_IsArrowSelected)
			{
				if (Subtoolbox->m_IsSubtoolbox!=m_SelectedElement+1)
				{
					Subtoolbox->m_IsSubtoolbox=m_SelectedElement+1;
					Subtoolbox->m_SelectedElement=-1;
					Subtoolbox->AdjustPosition();
					Subtoolbox->InvalidateRect(NULL,1);
					Subtoolbox->UpdateWindow();
					NoImageAutogeneration=2;
					Subtoolbox->ShowWindow(SW_SHOWNA);
				}
			}
			else
			{
				PickUpElementFromToolbox(m_SelectedElement,ToolboxMembers[m_SelectedElement].SelectedSubmember);
			}
		}
		else if (m_SelectedTextControl>0)
		{
			CExpression *exp=(CExpression*)KeyboardEntryObject;
			if (exp)
			{
				CDC *ddc=pMainView->GetDC();
				if (m_SelectedTextControl==1) {int tmp=IsALTDown;IsALTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,' ',0,0,0,1);IsALTDown=tmp;}
				if (m_SelectedTextControl==2) {int tmp=IsALTDown;IsALTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,0x0D,0,0,0,1);IsALTDown=tmp;}
				if ((m_SelectedTextControl==3) && (exp->m_MaxNumColumns<50))
					{int tmp=IsSHIFTDown;IsSHIFTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,9,0,0,0,1);IsSHIFTDown=tmp;}
				if ((m_SelectedTextControl==4) && (exp->m_MaxNumRows<50))
					{int tmp=IsSHIFTDown;IsSHIFTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,0x0D,0,0,0,1);IsSHIFTDown=tmp;}
				if ((m_SelectedTextControl>=5) && (m_SelectedTextControl<=9))
				{
					if ((exp->m_IsKeyboardEntry>0) && (exp->m_IsKeyboardEntry<=exp->m_NumElements)) 
					{
						tElementStruct *e=exp->m_pElementList+exp->m_IsKeyboardEntry-1;
						if ((exp->m_IsKeyboardEntry>1) && (exp->m_KeyboardCursorPos==0)) e--;
						if (e->Type==1)
						{
							//clear any existing decoration
							e->pElementObject->m_VMods=0;
						}
						else m_SelectedTextControl=0;
					}
				}
				if (m_SelectedTextControl==5) {int tmp=IsALTDown;IsALTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,',',0,0,0,1);IsALTDown=tmp;}
				if (m_SelectedTextControl==6) {int tmp=IsALTDown;IsALTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,',',0,0,0,1);exp->KeyboardKeyHit(ddc,ViewZoom,',',0,0,0,1);IsALTDown=tmp;}
				if (m_SelectedTextControl==7) {exp->KeyboardKeyHit(ddc,ViewZoom,'^',0,0,0,1);
				((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(ddc,ViewZoom,'^',0,0,0,1);}
				if (m_SelectedTextControl==8) {int tmp=IsALTDown;IsALTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,'.',0,0,0,1);IsALTDown=tmp;}
				if (m_SelectedTextControl==9) {int tmp=IsALTDown;IsALTDown=1;exp->KeyboardKeyHit(ddc,ViewZoom,'.',0,0,0,1);exp->KeyboardKeyHit(ddc,ViewZoom,'.',0,0,0,1);IsALTDown=tmp;}
			
				short l,a,b;
				while (exp->m_pPaternalExpression) exp=exp->m_pPaternalExpression;
				exp->CalculateSize(ddc,ViewZoom,&l,&a,&b);

				pMainView->ReleaseDC(ddc);
				pMainView->InvalidateRect(NULL,0);
				pMainView->UpdateWindow();
			}
		}
		else if (m_SelectedColor>=0)
		{
			int LW=-100;
			int CL=-100;

			if (m_SelectedColor==20) 
			{
				//the "edit nodes" option
				if ((ToolbarEditNodes) && (ToolbarConfig[ToolbarSelectedItem].IconState!=3) && (!IsDrawingMode))
				{
					ToolbarEditNodes=0;
					pMainView->SetMousePointer();
				}
				else
				{
					ToolbarEditNodes=1;
					//ToolbarPreviousTool=IsDrawingMode;
					IsDrawingMode=0;
					pMainView->SetMousePointer();
				}
				CDC *DC=this->GetDC();
				this->PaintColorbox(DC);
				this->ReleaseDC(DC);
			}
			else if (m_SelectedColor==21) 
			{
				//use cross
				if (ToolbarUseCross) ToolbarUseCross=0; else ToolbarUseCross=1;
				pMainView->InvalidateRect(NULL,0);
				pMainView->UpdateWindow();
			}
			else if (m_SelectedColor==22) 
			{
				//select last object
				//pMainView->OnSysKeyDown(' ',0,0);
				pMainView->OnKeyDown(0x0D,0,0);
				CDC *DC=this->GetDC();
				this->PaintColorbox(DC);
				this->ReleaseDC(DC);
			}
			else
			{
				//line color and width adjustment
				int LW=-100;
				int CL=-100;

				if (m_SelectedColor>=10)
				{
					int width=m_SelectedColor-10;
					if (width==0) LW=65*DRWZOOM/100;
					if (width==1) LW=DRWZOOM;
					if (width==2) LW=3*DRWZOOM/2;
					if (width==3) LW=2*DRWZOOM;
					if (width==4) LW=3*DRWZOOM;
					if (width==5) LW=4*DRWZOOM;
				}
				if (m_SelectedColor<10) CL=m_SelectedColor;
				if ((NumSelectedObjects==0) || (IsDrawingMode))
				{
					if (LW!=-100) SelectedLineWidth=LW;
					if (CL!=-100) SelectedLineColor=CL;
				}
				PaintColorbox(this->GetDC());
				//check if there is any selected object (drawing) and change color and line size of it
				int repaint=0;
				int any=0;
				for (int i=0;i<NumDocumentElements;i++)
				{
					tDocumentStruct *ds=TheDocument+i;

					if ((ds->Object) && (ds->MovingDotState==3))
					{
						any=1;
						if (ds->Type==2)
						{

							if (m_SelectedColor<10) ((CDrawing*)(ds->Object))->SetColor(CL);
							if (m_SelectedColor>=10)((CDrawing*)(ds->Object))->SetLineWidth(LW);
							int x,y,w,h;
							((CDrawing*)(ds->Object))->AdjustCoordinates(&x,&y,&w,&h);
							ds->absolute_X+=x;
							ds->absolute_Y+=y;
							repaint=1;
							
						}
						if (ds->Type==1)
						{
							((CExpression*)(ds->Object))->SetColor(CL);
						}
					}
				}

				TemporaryShowColoredSelection=2;
				if (repaint) pMainView->RepaintTheView(); else pMainView->InvalidateRect(NULL,0);

				if ((NumSelectedObjects==0) || (IsDrawingMode))
				if ((any==0) && (IsDrawingMode==0)) 
				{
					IsDrawingMode=prevDrawingMode2;
					if (IsDrawingMode<=0) IsDrawingMode=3; 
					pMainView->SetMousePointer();
				}
			}
			
		}
	}
	else if (m_IsSubtoolbox>0)
	{
		int xx=point.x*2/ToolboxSize;
		int yy=point.y*3/ToolboxSize/2;

		if ((m_SelectedElement<ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers) && (m_SelectedElement>=0))
		{
			//clicked on something in the sub-toolbox
			if (m_IsArrowSelected)
			{
				ToolboxMembers[m_IsSubtoolbox-1].SelectedSubmember=m_SelectedElement;
				m_IsSubtoolbox=-1;
				ShowWindow(SW_HIDE);
				Toolbox->InvalidateRect(NULL,0);
				Toolbox->UpdateWindow();
			}
			else
			{
				PickUpElementFromToolbox(m_IsSubtoolbox-1,m_SelectedElement);
			}
		}
	}
	else if (m_IsSubtoolbox==-4) //special drawing toolbox
	{
		int xx=point.x*2/ToolboxSize;
		int yy=point.y*2/ToolboxSize;

		int member=3*yy+xx;
		int m=-1,s;
		if ((member>=0) && (member<=8)) {m=SpecialDrawingToolbox[member].member;s=SpecialDrawingToolbox[member].submember;}
		ShowWindow(SW_HIDE);
		if (m>-1)
		{
			this->PickUpElementFromToolbox(m,s);
			Sleep(100);
		}
		if (pMainView) pMainView->SetMousePointer();
	}
	else if (m_IsSubtoolbox==-2) //the mixed font subtoolbox
	{
		ToolboxChangeIndividualKeyFont();
	}
	else if (m_IsSubtoolbox==-1) //the uniform font subtoolbox
	{
		if ((m_SelectedElement<ToolboxFontFormating.NumFormats) && (m_SelectedElement>=0))
		{
			//clicked on something in the uniform font sub-toolbox
			if (m_IsSubtoolbox==-1) //permanent
			{
				if (m_IsArrowSelected)
				{
					//we will open the font popup-menu for this font
					m_ContextMenuMember=-1;
					m_ContextMenuSubmember=m_SelectedElement; //we are pretending like if it is the call from the context menu (accelerator menu)
					DefineAcceleratorCode(2);
				}
				else
				{
					Toolbox->m_FontModeSelection=0;
					ToolboxFontFormating.SelectedUniform=m_SelectedElement;
					if ((ToolboxFontFormating.UniformFormats2[ToolboxFontFormating.SelectedUniform]&0x01)==0) //if it is not a singleshot mode
					{
						Toolbox->m_prevFontModeSelection=ToolboxFontFormating.SelectedUniform;
					}

					AdjustKeyboardFont();
					this->ReformatKeyboardSelection();
					
					//hide subtoolbox menu
					m_IsSubtoolbox=-1;
					ShowWindow(SW_HIDE);
				}
			}
			
			Toolbox->InvalidateRect(NULL,0);
			Toolbox->UpdateWindow();
		}
	}
	else if (m_IsContextMenu)
	{
		//clicked at context menu
		int i=0;
		while (ToolboxKeycodes[i].code!=0)
		{
			if (i==m_ContextMenuSelection)
			{
				if (ToolboxKeycodes[i].code==4) return; //just a separator
				DefineAcceleratorCode(ToolboxKeycodes[i].code);	
			}
			i++;
		}
		m_ContextMenuSelection=-1;
		m_ContextMenuMember=-1;
		ShowWindow(SW_HIDE);
	}
	else if (m_IsKeyboard)
	{
		RECT cr;
		GetClientRect(&cr);
		if (AlternateVirtualKeyboard==2)
		{
			AlternateVirtualKeyboard=3;
			PaintKeyboardElement(this->GetDC(),255);
			KeyboardHit(0,0);
		} 
		else if ((AlternateVirtualKeyboard==3) && (point.x>cr.right-20) && (point.y>cr.bottom-15))
		{
			AlternateVirtualKeyboard=2;
			PaintKeyboardElement(this->GetDC(),255);
			KeyboardHit(0,0);
		}
		else if ((m_KeyboardElement>=0) && (m_KeyboardElement<ToolboxKeyboardElements.NumKeys))
		{
			if (ClipboardExpression)
			{
				delete ClipboardExpression;
			}
			ClipboardExpression=new CExpression(NULL,NULL,100);
			ClipboardExpression->CopyExpression(ToolboxKeyboardElements.Key[m_KeyboardElement],0);
			short l,a,b;
			ClipboardExpression->CalculateSize(this->GetDC(),100,&l,&a,&b);
			//pMainView->SetActiveWindow();
		}

	}
	else if (m_IsToolbar)
	{
		ToolbarIsClicked=1;
		InvalidateRect(NULL,0);
		UpdateWindow();
		Sleep(50);
		ToolbarIsClicked=0;

		if ((ToolbarSelectedItem>=0) && (ToolbarSelectedItem<ToolbarNumItems) && (ToolbarConfig[ToolbarSelectedItem].IconState))
		{
			int icon=ToolbarConfig[ToolbarSelectedItem].IconIndex;

			
			if (icon==0) pMainView->OnEditUndo();
			if (icon==1) {DisplayShortText("Saving...",10,10,7000); pMainView->GetDocument()->OnFileSave();}
			if (icon==2) pMainView->OnEditCut();
			if (icon==3) pMainView->OnEditCopy();
			if (icon==4) pMainView->OnEditPaste();
			if (icon==5) pMainView->OnViewZoomto1();
			if (icon==6) pMainView->OnEditCopyImage();
			if (icon==39) pMainView->OnHqRend();
			/*if ((icon==17)) //edit nodes
			{
				if ((ToolbarEditNodes) && (ToolbarConfig[ToolbarSelectedItem].IconState!=3))
				{
					ToolbarEditNodes=0;
					//IsDrawingMode=ToolbarPreviousTool;
					pMainView->SetMousePointer();
				}
				else
				{
					ToolbarEditNodes=1;
					ToolbarPreviousTool=IsDrawingMode;
					IsDrawingMode=0;
					pMainView->SetMousePointer();
				}
			}*/
			if ((icon>=32) && (icon<=38))
			{
				int tmp=0;
				if (icon==32) tmp=2;
				if (icon==33) tmp=3;
				if (icon==34) tmp=5;
				if (icon==35) tmp=1;
				if (icon==36) tmp=4;
				if (icon==37) tmp=6;
				if (icon==38) tmp=7;
				if (IsDrawingMode==tmp) tmp=0;
				IsDrawingMode=tmp;
				//ToolbarEditNodes=0;
				pMainView->SetMousePointer();
			}
			if (icon==7)
			{
				if (IsShowGrid) IsShowGrid=0; else IsShowGrid=1;
				((CMainFrame*)theApp.m_pMainWnd)->AdjustMenu();
				pMainView->InvalidateRect(NULL,0);
				pMainView->UpdateWindow();
			}
			if (icon==8) pMainView->OnKeyDown(VK_F4,0,0);
			if (((KeyboardEntryBaseObject) && (KeyboardEntryObject)) || ((NumSelectedObjects==1) && (NumSelectedDrawings==0)))
			{
				int yes=0;
				tDocumentStruct *parentstr=KeyboardEntryBaseObject;
				CExpression *exp=((CExpression*)KeyboardEntryObject);
				CExpression *parent=(KeyboardEntryBaseObject)?(((CExpression*)(KeyboardEntryBaseObject->Object))):NULL;
				if ((parent==0) || (exp==0))
				{
					tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
					for (int i=NumDocumentElements-1;i>=0;i--,ds--)
					{
						if (ds->MovingDotState==3)
						{
							parent=exp=((CExpression*)ds->Object);
							parentstr=ds;
							break;
						}
					}
				}
				if ((parent) && (exp) && (parentstr) && (parentstr->Type==1))
				{
				
					if ((icon==9) && (exp->m_FontSize<1400)) {exp->ChangeFontSize((float)1.2);yes=1;}
					if ((icon==10) && (exp->m_FontSize>25)) {exp->ChangeFontSize((float)(1.0/1.2));yes=1;}
					if (icon==18)
					{
						yes=1;
						if (parent->m_IsVertical) parent->m_IsVertical=0; else parent->m_IsVertical=1;
					}
					if (icon==19)
					{
						yes=1;
						
						{
							float fontsize;
							float rel=(float)((float)parent->m_FontSize*100.0/(float)DefaultFontSize); 
							if ((parent->m_IsHeadline) && (rel>248.0)) 
							{
								fontsize=(float)(100.0/(float)parent->m_FontSize);
								parent->m_IsHeadline=0;
							}
							else
							{
								if (rel>190.0) fontsize=(float)(250.0/(float)parent->m_FontSize);
								else if (rel>135.0) fontsize=(float)(207.4/(float)parent->m_FontSize);
								else fontsize=(float)(144.0/(float)parent->m_FontSize);
								parent->m_IsHeadline=1;
							}
							fontsize=fontsize*(float)DefaultFontSize/(float)100.0;
							parent->ChangeFontSize(fontsize);
							
						}
					}
					if ((icon>=28) && (icon<=31) && (exp))
					{
						for (int i=0;i<exp->m_NumElements;i++)
						{
							tElementStruct *ts=exp->m_pElementList+i;
							if (ts->IsSelected==2)
							{
								if ((ts->Type==1) && (ts->pElementObject))
								if ((icon==28) || (icon==29))
								{
									yes=1;
									for (int j=0;j<(int)strlen(ts->pElementObject->Data1);j++)
										ts->pElementObject->Data2[j]|=(icon==28)?0x01:0x02;
								}
								if (icon==30) {ts->Decoration=3;yes=1;}
								if (icon==31) {ts->Decoration=5;yes=2;}
							}
						}
					}
					if ((icon>=25) && (icon<=27) && (exp))
					{
						yes=1;
						//left/right/center alignment for text boxes (including matrix cell boxes)
						int align=exp->m_Alignment;

						if ((exp->m_IsKeyboardEntry) && ((exp->m_MaxNumRows>1) || (exp->m_MaxNumColumns>1)))
						{
							tCellAttributes a;
							a.alignment=NULL;
							for (int i=0;i<exp->m_MaxNumRows;i++)
								for (int j=0;j<exp->m_MaxNumColumns;j++)
								{
									int k=exp->FindMatrixElement(i,j,0);
									if (k>exp->m_IsKeyboardEntry-1)
									{
										
										i=1000; //force exit;
										break;
									}
									exp->GetCellAttributes(i,j,&a);
								}
							if (a.alignment)
							{
								if (icon==25) *a.alignment='l';
								if (icon==26) *a.alignment='c';
								if (icon==27) *a.alignment='r';
							}
						}
						else
						{
							if (icon==25) exp->m_Alignment=1;
							if (icon==26) exp->m_Alignment=0;
							if (icon==27) exp->m_Alignment=2;
						}
					
						
					}
					if (yes) 
					{
						short l,a,b;CDC *dc=pMainView->GetDC();
						parent->CalculateSize(dc,ViewZoom,&l,&a,&b);
						pMainView->ReleaseDC(dc);
						parentstr->Above=a*100/ViewZoom;
						parentstr->Below=b*100/ViewZoom;
						parentstr->Length=l*100/ViewZoom;

						pMainView->InvalidateRect(NULL,0);pMainView->UpdateWindow();
					}
				}
			}
			if (NumSelectedObjects)
			{
				pMainView->m_PopupMenuObject=0;
				if (icon==11) Popup->OnLButtonDown(560,CPoint(-10000,-10001));
				if (icon==12) Popup->OnLButtonDown(562,CPoint(-10000,-10001));
				if (icon==13) Popup->OnLButtonDown(564,CPoint(-10000,-10001));
				if (icon==14) Popup->OnLButtonDown(561,CPoint(-10000,-10001));
				if (icon==15) Popup->OnLButtonDown(563,CPoint(-10000,-10001));
				if (icon==16) Popup->OnLButtonDown(565,CPoint(-10000,-10001));
				if (icon==20) Popup->OnLButtonDown(535,CPoint(-10000,-10001)); //h.mirror
				if (icon==21) Popup->OnLButtonDown(536,CPoint(-10000,-10001)); //v.mirror
				if (icon==22) Popup->OnLButtonDown(505,CPoint(-10000,-10001)); //group
				if (icon==23) Popup->OnLButtonDown(506,CPoint(-10000,-10001)); //ungroup
				if (icon==24) Popup->OnLButtonDown(578,CPoint(-10000,-10001)); //vertical distribute
			}
			Toolbox->ConfigureToolbar();
			Toolbox->Toolbar->InvalidateRect(NULL,0);
			Toolbox->Toolbar->UpdateWindow();
		}
	}


	if (!m_IsContextMenu)
	{
		//we will close context menu if clicked anywhere else except on context menu
		if (Toolbox->ContextMenu)
		{
			Toolbox->ContextMenu->ShowWindow(SW_HIDE);
			Toolbox->ContextMenu->m_ContextMenuSelection=-1;
			Toolbox->ContextMenu->m_ContextMenuMember=-1;
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
	}
	catch(...)
	{
	}
}

#pragma optimize("",on)

/*int CToolbox::AdjustToolboxParentheses(void)
{
	int i,j;
	for (i=0;i<ToolboxNumMembers;i++)
	{
		for (j=0;j<ToolboxMembers[i].NumSubmembers;j++)
		if (ToolboxMembers[i].Above[j]!=-1) //not for drawing elements
		{
			ToolboxMembers[i].Submembers[j]->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			if (ToolboxMembers[i].Submembers[j]->m_pElementList==NULL) break;
			if (ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject==NULL) break;
			if (ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject->Expression1)
				((CExpression*)(ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject->Expression1))->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			if (ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject->Expression2)
				((CExpression*)(ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject->Expression2))->m_ParentheseHeightFactor=1;//DefaultParentheseType;
			if (ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject->Expression3)
				((CExpression*)(ToolboxMembers[i].Submembers[j]->m_pElementList->pElementObject->Expression3))->m_ParentheseHeightFactor=1;//DefaultParentheseType;
		}
	}

	if (Subtoolbox) Subtoolbox->AdjustToolboxParentheses();
	return 0;
}*/

void CToolbox::HideUnhideColorbox()
{
	static int prevDrawingMode;
	int tmpDM=IsDrawingMode+((NumSelectedObjects)?1000:0);
	if (prevDrawingMode!=tmpDM)
	{
		//InvalidateRect(NULL,0);
		RECT r;
		r.left=0;r.right=ToolboxSize;
		r.top=(ToolboxNumMembers+1)/2*2*ToolboxSize/3+ToolboxSize/2;r.bottom=r.top+ToolboxSize+5;
		InvalidateRect(&r,0);
		AdjustPosition();
		prevDrawingMode=tmpDM;
	}
	return;
}
char TextControlboxMode=0;
void CToolbox::PaintColorbox(CDC * dc)
{
	
	if (dc==NULL) dc=this->GetDC();
	TextControlboxMode=-1;
	int ToolboxBottom=(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2; //the bottom of toolbox main part
	dc->FillSolidRect(1,ToolboxBottom,ToolboxSize-2,4,RGB(128,128,128));
	dc->FillSolidRect(0,ToolboxBottom+4,ToolboxSize,1,RGB(96,96,96));


	CDC pdc;
	pdc.CreateCompatibleDC(dc);
	CBitmap tmpbmp;
	tmpbmp.CreateCompatibleBitmap(dc,ToolboxSize,ToolboxSize);
	pdc.SelectObject(tmpbmp);

	if (dc->GetDeviceCaps(BITSPIXEL)>16)
	{
		for (int i=0;i<ToolboxSize;i+=3)
		{
			int k=i/2;
			k=255-k;
			pdc.FillSolidRect(0,i,ToolboxSize,3,RGB(k,k,255));
		}
	}
	else
		pdc.FillSolidRect(0,0,ToolboxSize,ToolboxSize,RGB(255,255,255));


	pdc.FillSolidRect(ToolboxSize/2,2*ToolboxSize/3-6,1,5,RGB(220,220,220));

	int t3=ToolboxSize/3;
	int tmpy=max(0,ToolboxSize-60)/16;
	//painting the colors
	int len=(ToolboxSize+2)/5;
	int len2=(ToolboxSize+3)/7;
	if (m_SelectedColor<5) pdc.FillSolidRect(m_SelectedColor*len+0,0,len,t3,SHADOW_BLUE_COLOR2);
	for (int j=0;j<5;j++)
	{
		unsigned int color=ColorTable[j];
		if ((NumSelectedObjects==0) || (IsDrawingMode))
		{
			CPen pen;
			CBrush brush;
			pen.CreatePen(PS_SOLID,1,color);
			brush.CreateSolidBrush(color);
			POINT p[16];
			if (ToolboxSize>90)
			{
				p[0].x=8;p[0].y=1;
				p[1].x=13;p[1].y=2;
				p[2].x=8;p[2].y=20;
				p[3].x=3;p[3].y=25;
				p[4].x=3;p[4].y=19;
				p[5].x=3;p[5].y=19;
				p[6].x=7;p[6].y=20;
			}
			else if(ToolboxSize>=80)
			{
				p[0].x=8;p[0].y=1;
				p[1].x=12;p[1].y=2;
				p[2].x=7;p[2].y=17;
				p[3].x=3;p[3].y=21;
				p[4].x=3;p[4].y=16;
				p[5].x=3;p[5].y=16;
				p[6].x=7;p[6].y=17;
			}
			else
			{
				p[0].x=8;p[0].y=1;
				p[1].x=11;p[1].y=2;
				p[2].x=6;p[2].y=12;
				p[3].x=3;p[3].y=15;
				p[4].x=3;p[4].y=11;
				p[5].x=3;p[5].y=11;
				p[6].x=6;p[6].y=12;
			}

			for (int k=0;k<7;k++)
			{
				p[k].x+=j*len-((ToolboxSize<80)?1:-2);
				p[k].y+=tmpy;
			}

			pdc.SelectObject(pen);
			pdc.SelectObject(brush);
			pdc.Polygon(p,5);
			pen.DeleteObject();

			pen.CreatePen(PS_SOLID,1,RGB(255,255,255));
			pdc.SelectObject(pen);
			pdc.Polyline(&p[5],2);
		}
		else
			pdc.FillSolidRect(j*len+(len-len2)/2,ToolboxSize/16,len2,len2+ToolboxSize/24,color);
	}
	if ((NumSelectedObjects==0) || (IsDrawingMode))
	{
		int clr=max(0,SelectedLineColor);
		PaintCheckedSign(&pdc,clr*len+len/8,ToolboxSize/5+2,ToolboxSize/8,1);
	}

	

	//painting the line sizes
	len=ToolboxSize/6;
	len2=ToolboxSize/30;
	if ((m_SelectedColor>=10) && (m_SelectedColor<20))pdc.FillSolidRect((m_SelectedColor-10)*len,t3,len,t3,SHADOW_BLUE_COLOR2);
	
	for (int j=0;j<6;j++)
	{
		pdc.FillSolidRect(j*len+1,3*t3/2+1-j*len2+tmpy,len-1,j*len2+1,RGB(64,64,64));

		if ((NumSelectedObjects==0) || (IsDrawingMode))
		if (((SelectedLineWidth<9*DRWZOOM/10) && (j==0)) ||
			((SelectedLineWidth>=9*DRWZOOM/10) && (SelectedLineWidth<12*DRWZOOM/10) && (j==1)) ||
			((SelectedLineWidth>=12*DRWZOOM/10) && (SelectedLineWidth<18*DRWZOOM/10) && (j==2)) ||
			((SelectedLineWidth>=18*DRWZOOM/10) && (SelectedLineWidth<22*DRWZOOM/10) && (j==3)) ||
			((SelectedLineWidth>=22*DRWZOOM/10) && (SelectedLineWidth<32*DRWZOOM/10) && (j==4)) ||
			((SelectedLineWidth>=32*DRWZOOM/10) && (j==5)))
			PaintCheckedSign(&pdc,j*len+2,t3+ToolboxSize/5,ToolboxSize/8,1);
	}

	pdc.SelectObject(GetStockObject(BLACK_PEN));
	int yy=2*ToolboxSize/3+tmpy;
	int tt=(ToolboxSize<80)?3:((ToolboxSize<90)?4:5);
	if (ToolboxSize>96) tt=5;

	if (m_SelectedColor>=20)
		pdc.FillSolidRect((m_SelectedColor-20)*ToolboxSize/3+1,yy,ToolboxSize/3,ToolboxSize/3,SHADOW_BLUE_COLOR2);

	pdc.MoveTo(tt,yy+4*tt+1);
	pdc.LineTo(3*tt-1,yy+tt);
	pdc.LineTo(4*tt,yy+3*tt);
	pdc.LineTo(5*tt,yy+tt);
	pdc.FillSolidRect(tt-tt/2,yy+4*tt-tt/2+1,tt,tt,RGB(128,0,0)); 
	pdc.FillSolidRect(3*tt-tt/2-1,yy+tt-tt/2,tt,tt,RGB(128,0,0)); 
	pdc.FillSolidRect(4*tt-tt/2,yy+3*tt-tt/2,tt,tt,RGB(128,0,0)); 
	pdc.FillSolidRect(5*tt-tt/2,yy+tt-tt/2,tt,tt,RGB(128,0,0)); 
	if (ToolbarEditNodes) PaintCheckedSign(&pdc,3*tt,yy+4*tt-2,2*tt+1,(IsDrawingMode)?2:1);
	pdc.SelectObject(GetStockObject(BLACK_PEN));

	pdc.MoveTo(ToolboxSize/2,yy+tt-1);
	pdc.LineTo(ToolboxSize/2,yy+2*tt+1);
	pdc.MoveTo(ToolboxSize/2,yy+4*tt-1);
	pdc.LineTo(ToolboxSize/2,yy+5*tt+1);
	pdc.MoveTo(ToolboxSize/2-2*tt-1,yy+3*tt);
	pdc.LineTo(ToolboxSize/2-tt+1,yy+3*tt);
	pdc.MoveTo(ToolboxSize/2+2*tt+1,yy+3*tt);
	pdc.LineTo(ToolboxSize/2+tt-1,yy+3*tt);
	if (ToolbarUseCross) PaintCheckedSign(&pdc,ToolboxSize/2+1,yy+4*tt-2,2*tt+1,1);

	pdc.FillSolidRect(ToolboxSize-5*tt+2,yy+2*tt+1,3*tt-2,3*tt-2,RGB(164,164,255));
	pdc.FillSolidRect(ToolboxSize-5*tt-tt+2,yy+2*tt-tt+1,tt+1,tt+1,RGB(128,0,0));
	pdc.FillSolidRect(ToolboxSize-1*tt-tt-1,yy+2*tt-tt+1,tt+1,tt+1,RGB(128,0,0));
	pdc.FillSolidRect(ToolboxSize-5*tt-tt+2,yy+6*tt-tt-2,tt+1,tt+1,RGB(128,0,0));
	pdc.FillSolidRect(ToolboxSize-1*tt-tt-1,yy+6*tt-tt-2,tt+1,tt+1,RGB(128,0,0));


	dc->BitBlt(1,ToolboxBottom+5,ToolboxSize-2,ToolboxSize-1,&pdc,1,0,SRCCOPY);

	if (m_SelectedColor==0) this->ShowHelptext("black color","","Alt+B","",6070);
	if (m_SelectedColor==1) this->ShowHelptext("red color","","Alt+R","",6071);
	if (m_SelectedColor==2) this->ShowHelptext("green color","","Alt+G","",6072);
	if (m_SelectedColor==3) this->ShowHelptext("blue color","","","",6073);
	if (m_SelectedColor==4) this->ShowHelptext("gray color","","","",6074);

	if ((m_SelectedColor>=10) && (m_SelectedColor<=19)) this->ShowHelptext("line width","","","",6080);
	if (m_SelectedColor==20) this->ShowHelptext("node edit","","Ctrl","",6090);
	if (m_SelectedColor==21) this->ShowHelptext("crosshair\r\ncursor","","Ctrl","",6091);
	if (m_SelectedColor==22) this->ShowHelptext("select last","","Enter","",6092);

	if (m_SelectedColor<0) this->ShowHelptext("","","","",0);

}

void CToolbox::HideUnhideTextcontrolbox()
{
	static int prevWasTextmode;
	int tmpDM=((KeyboardEntryBaseObject) && (KeyboardEntryObject) && (IsDrawingMode==0))?1:0;
	if (prevWasTextmode!=tmpDM)
	{
		InvalidateRect(NULL,0);
		AdjustPosition();
		prevWasTextmode=tmpDM;
	}
	return;
}


void CToolbox::PaintTextcontrolbox(CDC * dc)
{
	if ((IsDrawingMode)||(NumSelectedObjects)) return;
	if (dc==NULL) dc=this->GetDC();
	dc->FillSolidRect(1,(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2,ToolboxSize-2,5,RGB(128,128,128));
	dc->FillSolidRect(0,(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2+4,ToolboxSize,1,RGB(96,96,96));

	CDC pdc;
	pdc.CreateCompatibleDC(dc);
	CBitmap tmpbmp;
	tmpbmp.CreateCompatibleBitmap(dc,ToolboxSize,ToolboxSize);
	pdc.SelectObject(tmpbmp);
	//pdc.FillSolidRect(0,0,ToolboxSize,ToolboxSize,RGB(255,255,255));

	CExpression *exp=(CExpression*)KeyboardEntryObject;
	if (exp)
	{
		int isText=0;
		if ((exp->m_pElementList+exp->m_IsKeyboardEntry-1)->Type==1)
		{
			if ((exp->m_pElementList+exp->m_IsKeyboardEntry-1)->pElementObject->m_Text) isText=1;
		}


		TextControlboxMode=(char)isText;
		pdc.SetTextAlign(TA_TOP);
		pdc.SetBkMode(TRANSPARENT);
		
		RECT r={0,0,ToolboxSize,ToolboxSize};
		int zz=ToolboxSize/32;
		if (isText)
		{
			pdc.FillSolidRect(&r,RGB(192,240,192));
			pdc.SelectObject(GetFontFromPool(4,0,1,ToolboxSize/3+ToolboxSize/8));
			r.bottom-=ToolboxSize/3+ToolboxSize/4-1;
			if (m_SelectedTextControl==1) pdc.FillSolidRect(&r,RGB(128,128,255));
			pdc.SetTextColor(RGB(0,192,0));
			pdc.TextOut(5,-zz,"T",1);
			pdc.SelectObject(GetFontFromPool(4,0,1,ToolboxSize/5+((ToolboxSize<60)?1:0)));
			char bff[16];
			CopyTranslatedString(bff,"Text",6052,15);
			pdc.TextOut(ToolboxSize/2-ToolboxSize/10,ToolboxSize/8-4-zz,bff);
			CopyTranslatedString(bff,"mode",6053,15);
			pdc.TextOut(ToolboxSize/2-ToolboxSize/10,ToolboxSize/4-3-zz,bff);
		}
		else
		{
			pdc.FillSolidRect(&r,RGB(208,208,248));
			pdc.SelectObject(GetFontFromPool(3,0,1,ToolboxSize/3+ToolboxSize/6));
			r.bottom-=ToolboxSize/3+ToolboxSize/4-1;
			if (m_SelectedTextControl==1) pdc.FillSolidRect(&r,RGB(128,128,255));
			pdc.SetTextColor(RGB(0,0,255));
			pdc.TextOut(5,-2-zz,"S",1);
			pdc.SelectObject(GetFontFromPool(4,0,1,ToolboxSize/5+((ToolboxSize<60)?1:0)));
			char bff[16];
			CopyTranslatedString(bff,"Math",6054,15);
			pdc.TextOut(ToolboxSize/2-ToolboxSize/10,ToolboxSize/8-4-zz,bff);
			CopyTranslatedString(bff,"mode",6055,15);
			pdc.TextOut(ToolboxSize/2-ToolboxSize/10,ToolboxSize/4-3-zz,bff);
		}

		int yy=ToolboxSize-ToolboxSize/3;
		if (m_SelectedTextControl==2) pdc.FillSolidRect(0,yy+1,ToolboxSize/3,ToolboxSize/3,RGB(128,128,255));
		if (m_SelectedTextControl==3) pdc.FillSolidRect(ToolboxSize/3,yy+1,ToolboxSize/3,ToolboxSize/3,RGB(128,128,255));
		if (m_SelectedTextControl==4) pdc.FillSolidRect(2*ToolboxSize/3,yy+1,ToolboxSize/3,ToolboxSize/3,RGB(128,128,255));
		int yyy=yy-ToolboxSize/4+1;
		int mmm=(ToolboxSize+1)/5;
		if (m_SelectedTextControl==5) pdc.FillSolidRect(0,yyy,mmm,ToolboxSize/4,RGB(128,128,255));
		if (m_SelectedTextControl==6) pdc.FillSolidRect(mmm,yyy,mmm,ToolboxSize/4,RGB(128,128,255));
		if (m_SelectedTextControl==7) pdc.FillSolidRect(2*mmm,yyy,mmm,ToolboxSize/4,RGB(128,128,255));
		if (m_SelectedTextControl==8) pdc.FillSolidRect(3*mmm,yyy,mmm,ToolboxSize/4,RGB(128,128,255));
		if (m_SelectedTextControl==9) pdc.FillSolidRect(4*mmm,yyy,mmm,ToolboxSize/4,RGB(128,128,255));
	
		if (m_SelectedTextControl==1) this->ShowHelptext("Typing mode","","Alt+Space","",6001);
		if (m_SelectedTextControl==2) this->ShowHelptext("Line wrap","","Alt+Enter","",6002);
		if (m_SelectedTextControl==3) this->ShowHelptext("Insert column","","Shft+Tab","",6003);
		if (m_SelectedTextControl==4) this->ShowHelptext("Insert row","","Shft+Enter","",6004);
		if (m_SelectedTextControl==4) this->ShowHelptext("Insert row","","Shft+Enter","",6004);
		if (m_SelectedTextControl==5) this->ShowHelptext("Vector","","Alt+,","",6020);
		if (m_SelectedTextControl==6) this->ShowHelptext("Dash","","Alt+,,","",6021);
		if (m_SelectedTextControl==7) this->ShowHelptext("Hat","","^ ^","",6022);
		if (m_SelectedTextControl==8) this->ShowHelptext("Dot","","Alt+.","",6023);
		if (m_SelectedTextControl==9) this->ShowHelptext("Double dot","","Alt+..","",6024);
		
		if (m_SelectedTextControl<=0) this->ShowHelptext("","","","",0);
	

		int Y=yy;
		pdc.FillSolidRect(0,Y+1,ToolboxSize,1,RGB(96,96,96));
		pdc.FillSolidRect(0,yyy,ToolboxSize,1,RGB(96,96,96));

		//a with various decorations
		pdc.FillSolidRect(mmm,yyy,1,ToolboxSize/4,RGB(96,96,96));
		pdc.FillSolidRect(2*mmm,yyy,1,ToolboxSize/4,RGB(96,96,96));
		pdc.FillSolidRect(3*mmm,yyy,1,ToolboxSize/4,RGB(96,96,96));
		pdc.FillSolidRect(4*mmm,yyy,1,ToolboxSize/4,RGB(96,96,96));
		CExpression *exp=new CExpression(NULL,NULL,100);
		exp->InsertEmptyElement(0,1,'a');
		short l,a,b;
		unsigned short zoom=5*ToolboxSize/4;
		exp->CalculateSize(&pdc,zoom,&l,&a,&b,0,1);
		int ccc=0;
		
		exp->m_pElementList->pElementObject->m_VMods=0x08;
		exp->PaintExpression(&pdc,zoom,ccc,yyy+zoom/8,0);
		exp->m_pElementList->pElementObject->m_VMods=0x04;
		exp->PaintExpression(&pdc,zoom,ccc+mmm,yyy+zoom/8,0);
		exp->m_pElementList->pElementObject->m_VMods=0x0C;
		exp->PaintExpression(&pdc,zoom,ccc+2*mmm,yyy+zoom/8,0);
		exp->m_pElementList->pElementObject->m_VMods=0x14;
		exp->PaintExpression(&pdc,zoom,ccc+3*mmm,yyy+zoom/8,0);
		exp->m_pElementList->pElementObject->m_VMods=0x18;
		exp->PaintExpression(&pdc,zoom,ccc+4*mmm,yyy+zoom/8,0);
		
		delete exp;
		
		pdc.FillSolidRect(ToolboxSize/3,Y+1,1,ToolboxSize/3,RGB(96,96,96));

		//the 'line wrap' sign
		int tt=ToolboxSize/30;
		int ttt=tt;
		if (tt%2) tt--;

		pdc.SelectObject(GetStockObject(BLACK_BRUSH));
		pdc.SelectObject(GetStockObject(BLACK_PEN));
		POINT p[16];
		p[0].x=ToolboxSize/3-3*ttt;p[0].y=Y+3*ttt;
		p[1].x=p[0].x+tt;p[1].y=p[0].y;
		p[2].x=p[1].x;p[2].y=Y+ToolboxSize/3-4*ttt;
		p[3].x=3*ttt;p[3].y=p[2].y;
		p[4].x=p[3].x;p[4].y=p[2].y+3*tt/2;
		p[5].x=p[3].x-2*tt;p[5].y=p[2].y-tt/2;
		p[6].x=p[3].x;p[6].y=p[2].y-5*tt/2;
		p[7].x=p[3].x;p[7].y=p[2].y-tt;
		p[8].x=p[0].x;p[8].y=p[7].y;
		p[9].x=p[0].x;p[9].y=p[0].y;

		pdc.Polygon(p,10);

		//the 'column insertion' sign
		pdc.FillSolidRect(2*ToolboxSize/3,Y+1,1,ToolboxSize/3,RGB(96,96,96));
		p[0].x=ToolboxSize/3+3*ttt;p[0].y=Y+ToolboxSize/6-tt/2;
		p[1].x=2*ToolboxSize/3-4*ttt;p[1].y=p[0].y;
		p[2].x=p[1].x;p[2].y=p[0].y-3*tt/2;
		p[3].x=p[1].x+2*tt;p[3].y=p[0].y+tt/2;
		p[4].x=p[1].x;p[4].y=p[0].y+5*tt/2;
		p[5].x=p[1].x;p[5].y=p[0].y+tt;
		p[6].x=p[0].x;p[6].y=p[5].y;
		p[7].x=p[0].x;p[7].y=p[0].y;

		pdc.Polygon(p,8);

		//the 'row insertion' sign
		p[0].x=2*ToolboxSize/3+ToolboxSize/6-tt/2;p[0].y=Y+3*ttt;
		p[1].x=p[0].x;p[1].y=Y+ToolboxSize/3-4*ttt;
		p[2].x=p[0].x-3*tt/2;p[2].y=p[1].y;
		p[3].x=p[0].x+tt/2;p[3].y=p[1].y+2*tt;
		p[4].x=p[0].x+5*tt/2;p[4].y=p[1].y;
		p[5].x=p[0].x+tt;p[5].y=p[1].y;
		p[6].x=p[0].x+tt;p[6].y=p[0].y;
		p[7].x=p[0].x;p[7].y=p[0].y;

		pdc.Polygon(p,8);
	}
	pdc.FillSolidRect(0,ToolboxSize-1,ToolboxSize,1,RGB(96,96,96));
	
	dc->BitBlt(1,(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2+5,ToolboxSize-2,ToolboxSize-1,&pdc,1,0,SRCCOPY);
}

//this paints elements of the toolbox (both, main toolbox and subtoolbox)
int CToolbox::PaintToolboxElement(CDC * dc, int member , char IsBlue)
{
	int i,j;
	RECT cr;
	GetClientRect(&cr);
	cr.bottom--;
	cr.right--;

	
	if (m_IsSubtoolbox==-1) //paint members of uniform font formating subtoolbox
	{
		int Ly=(ToolboxSize+2)/5;
		int Lx=3*ToolboxSize/2;
		int Cx=Lx;
		int Cy=member*(2*ToolboxSize/5)+Ly;

		CDC xdc;
		xdc.CreateCompatibleDC(dc);
		CBitmap xbmp;
		xbmp.CreateCompatibleBitmap(dc,3*ToolboxSize+2,2*ToolboxSize/5);
		xdc.SelectObject(xbmp);

		short l,a,b;

		int zz=0;if (ToolboxSize<60) zz=1;

		if (m_SelectedElement==member)
		{ 
			if (m_IsArrowSelected)
				xdc.FillSolidRect(Cx-Lx,Ly-Ly-zz,2*Lx,2*Ly+zz,RGB(240,240,255));
			else
				xdc.FillSolidRect(Cx-Lx,Ly-Ly-zz,2*Lx,2*Ly+zz,RGB(200,200,255));
		}
		else if (dc->GetDeviceCaps(BITSPIXEL)>16)
		{
			for (int i=0;i<ToolboxSize/2;i+=3)
			{
				int k=i/2;
				k=255-k;
				xdc.FillSolidRect(Cx-Lx,Ly-Ly+i-zz,2*Lx,3,RGB(k,k,255));
			}
		}
		else
			xdc.FillSolidRect(Cx-Lx,Ly-Ly-zz,2*Lx,2*Ly+zz,RGB(255,255,255));

		if (m_IsSubtoolbox==-1)
		{
			ToolboxFontFormating.UniformFormats[member]->CalculateSize(&xdc,100*ToolboxSize/60,&l,&a,&b,0,0);
			ToolboxFontFormating.UniformFormats[member]->PaintExpression(&xdc,100*ToolboxSize/60,ToolboxSize/4-l/2-ToolboxSize/24,Ly);

			if (ToolboxFontFormating.UniformFormats2[member]&0x01)
			{
				//painting down-arrow for singleshot formats
				xdc.SelectObject(GetStockObject(BLACK_PEN));
				int rr=ToolboxSize/16;
				int xx=ToolboxSize/2-rr-2;
				int yy=rr;
				xdc.MoveTo(xx,yy);
				xdc.LineTo(xx,yy+2*rr);
				xdc.LineTo(xx-rr,yy+rr);
				xdc.MoveTo(xx,yy+2*rr);
				xdc.LineTo(xx+rr,yy+rr); 
			}
			CExpression *exp=new CExpression(NULL,NULL,105);
			unsigned char fnt=ToolboxFontFormating.UniformFormats[member]->m_pElementList->pElementObject->Data2[0];
			unsigned char vmods=ToolboxFontFormating.UniformFormats[member]->m_pElementList->pElementObject->m_VMods;
			int clr=ToolboxFontFormating.UniformFormats[member]->m_pElementList->pElementObject->m_Color;
			for (int i=0;i<12;i+=2)
			{
				exp->InsertEmptyElement(i,1,'A'+i/2);
				exp->InsertEmptyElement(i+1,1,'a'+i/2);
				(exp->m_pElementList+i)->pElementObject->Data2[0]=fnt;
				(exp->m_pElementList+i+1)->pElementObject->Data2[0]=fnt;
				(exp->m_pElementList+i)->pElementObject->m_Color=clr;
				(exp->m_pElementList+i+1)->pElementObject->m_Color=clr;
				(exp->m_pElementList+i)->pElementObject->m_VMods=vmods;
				(exp->m_pElementList+i+1)->pElementObject->m_VMods=vmods;
			}

			exp->CalculateSize(&xdc,ToolboxSize+30,&l,&a,&b);
			exp->PaintExpression(&xdc,ToolboxSize+30,3*ToolboxSize/4,Ly);

			delete exp;

			if ((ToolboxFontFormating.SelectedUniform==member) && (Toolbox->m_FontModeSelection==0))
				PaintCheckedSign(&xdc,ToolboxSize/2-Ly/2,Ly,3*Ly/4,1);


			if (member==m_SelectedElement)
			{
				//painting the 'font...' arrow
				CBrush br;
				if (m_IsArrowSelected)
				{
					br.CreateSolidBrush(RGB(200,200,255));
					xdc.SelectObject(br);
				}
				else
				{
					br.CreateSolidBrush(RGB(240,240,255));
					xdc.SelectObject(br);
				}
				xdc.SelectObject(GetStockObject(BLACK_PEN));
				POINT p[5];
				int cc=ToolboxSize*2;
				p[0].x=cc;			p[0].y=Ly-1;
				p[1].x=cc+Ly;		p[1].y=Ly+Ly-1;
				p[2].x=Cx+Lx;		p[2].y=Ly+Ly-1;
				p[3].x=Cx+Lx;		p[3].y=Ly-Ly-1;
				p[4].x=cc+Ly;		p[4].y=Ly-Ly-1;
				xdc.Polygon(p,5);
				char bff[48];
				CopyTranslatedString(bff,"font...",5081,47);
				int zzz=(int)strlen(bff);
				xdc.SelectObject(GetFontFromPool(4,0,0,ToolboxSize/4-1-((zzz>9)?1:0)));
				xdc.SetTextAlign(TA_LEFT);
				xdc.SetBkMode(TRANSPARENT);
				xdc.SetTextColor(0);
				xdc.TextOutA(cc+3*Ly/4-((zzz>9)?(ToolboxSize/40):0),Ly-ToolboxSize/10,bff,zzz);
			}
		}
		if ((ToolboxSize%5)==0)
			dc->BitBlt(0,Cy-Ly,3*ToolboxSize,2*Ly,&xdc,0,0,SRCCOPY);
		else
			dc->BitBlt(0,Cy-Ly+1,3*ToolboxSize,2*Ly,&xdc,0,0,SRCCOPY);

		dc->SelectObject(GetPenFromPool(1,0,RGB(96,96,96)));
		dc->MoveTo(Cx-Lx,Cy+Ly-1);dc->LineTo(Cx+Lx,Cy+Ly-1);
		dc->MoveTo(0,0);
		dc->LineTo(cr.right-2,0);dc->LineTo(cr.right-2,cr.bottom-2);dc->LineTo(0,cr.bottom-2);dc->LineTo(0,0);

		dc->FillSolidRect(2,cr.bottom-1,cr.right,2,RGB(64,64,64));
		dc->FillSolidRect(cr.right-1,2,2,cr.bottom,RGB(64,64,64));
		dc->FillSolidRect(0,cr.bottom-1,2,2,RGB(255,255,255));
		dc->FillSolidRect(cr.right-1,0,2,2,RGB(255,255,255));
		return 0;
	}

	if (m_IsSubtoolbox==-2)  //paint members of mixed font formatting subtoolbox (a table of characters, latin and greek, upercase and lowercase)
	{
		int row=member/100;
		int key=member%100;
		if ((row>4) || (key>ToolboxKeyboardElements.NumKeys)) return 0;

		CDC xdc;
		xdc.CreateCompatibleDC(dc);
		CBitmap xbmp;

		if (row==4)
		{
			//the last line - showing title and "alter all" option
			xbmp.CreateCompatibleBitmap(dc,cr.right,ToolboxSize/4);
			xdc.SelectObject(xbmp);
			xdc.FillSolidRect(0,0,cr.right,ToolboxSize/4,RGB(224,224,224));

			xdc.SelectObject(GetFontFromPool(4,0,0,2*ToolboxSize/9));
			xdc.SetTextAlign(TA_LEFT);
			xdc.SetBkMode(TRANSPARENT);
			xdc.SetTextColor(RGB(128,128,128));

			char bff[128];
			CopyTranslatedString(bff,"Per-character font settings",5082,127);
			xdc.TextOutA(2,ToolboxSize/32,bff,(int)strlen(bff));
		
			if (m_IsArrowSelected)
				xdc.FillSolidRect(5*ToolboxSize,0,cr.right-2-5*ToolboxSize+1,ToolboxSize/4,RGB(200,200,255));

			xdc.SetTextColor(0);
			CopyTranslatedString(bff,"Alter all...",5083,127);
			xdc.TextOutA(5*ToolboxSize+2,ToolboxSize/32,bff,(int)strlen(bff));
			xdc.FillSolidRect(5*ToolboxSize,0,1,ToolboxSize/4,RGB(128,128,128));
			xdc.FillSolidRect(cr.right-2,0,1,ToolboxSize/4,RGB(128,128,128));

			
			dc->BitBlt(0,4*ToolboxSize/3,cr.right,ToolboxSize/4,&xdc,0,0,SRCCOPY);
			dc->FillSolidRect(0,0,cr.right-2,1,RGB(128,128,128));
			dc->FillSolidRect(0,0,1,cr.bottom-2,RGB(128,128,128));
			dc->FillSolidRect(2,cr.bottom-1,cr.right,2,RGB(64,64,64));
			dc->FillSolidRect(cr.right-1,2,2,cr.bottom,RGB(64,64,64));
			dc->FillSolidRect(0,cr.bottom-1,2,2,RGB(255,255,255));
			dc->FillSolidRect(cr.right-1,0,2,2,RGB(255,255,255));
			return 0;
		}

		if (key>=ToolboxKeyboardElements.NumKeys) return 0;
		
		xbmp.CreateCompatibleBitmap(dc,ToolboxSize/4,ToolboxSize/3);
		xdc.SelectObject(xbmp);
		if (member==m_SelectedElement)
			xdc.FillSolidRect(0,0,ToolboxSize/4,ToolboxSize/3,RGB(200,200,255));
		else if (dc->GetDeviceCaps(BITSPIXEL)>16)
		{
			for (int i=0;i<ToolboxSize/3;i+=3)
			{
				int k=255-i;
				xdc.FillSolidRect(0,i,ToolboxSize/4,3,RGB(k,k,255));
			}
		}
		else
			xdc.FillSolidRect(0,0,ToolboxSize/4,ToolboxSize/3,RGB(255,255,255));

		short l,a,b;
		unsigned char fdata=0;
		unsigned char vmods=0;
		char symbol=0;

		char prevsymbol=ToolboxKeyboardElements.Key[key]->m_pElementList->pElementObject->Data1[0];
		if (row==0) {fdata=ToolboxKeyboardElements.FormatingBigCaps[key];vmods=ToolboxKeyboardElements.FormatingBigCaps2[key];symbol=toupper(prevsymbol);}
		if (row==1) {fdata=ToolboxKeyboardElements.FormatingSmallCaps[key];vmods=ToolboxKeyboardElements.FormatingSmallCaps2[key];symbol=tolower(prevsymbol);}
		if (row==2) {fdata=ToolboxKeyboardElements.FormatingBigGreek[key];fdata=fdata&0x1F;fdata=fdata|0x60;vmods=ToolboxKeyboardElements.FormatingBigGreek2[key];symbol=toupper(prevsymbol);}
		if (row==3) {fdata=ToolboxKeyboardElements.FormatingSmallGreek[key];fdata=fdata&0x1F;fdata=fdata|0x60;vmods=ToolboxKeyboardElements.FormatingSmallGreek2[key];symbol=tolower(prevsymbol);}

		CExpression *exp=new CExpression(NULL,NULL,100);
		exp->InsertEmptyElement(0,1,symbol);
		exp->m_pElementList->pElementObject->Data2[0]=fdata;
		exp->m_pElementList->pElementObject->m_VMods=vmods;
		exp->CalculateSize(&xdc,100*ToolboxSize/80,&l,&a,&b);
		exp->PaintExpression(&xdc,100*ToolboxSize/73,ToolboxSize/8-l/2-1,ToolboxSize/6);;
		delete exp;

		xdc.FillSolidRect(0,ToolboxSize/3-1,ToolboxSize/4,1,RGB(128,128,128));
		xdc.FillSolidRect(ToolboxSize/4-1,0,1,ToolboxSize/3,RGB(128,128,128));

		dc->BitBlt(key*(ToolboxSize/4),row*ToolboxSize/3,ToolboxSize/4,ToolboxSize/3,&xdc,0,0,SRCCOPY);
		dc->FillSolidRect(0,0,cr.right-2,1,RGB(128,128,128));
		dc->FillSolidRect(0,0,1,cr.bottom-2,RGB(128,128,128));
		return 0;
	}
	


	//for paintg toolbox elements, we will temporary enable high quiality rendering
	int tmpHQR=IsHighQualityRendering;
	int tmpHR=IsHalftoneRendering;
	IsHalftoneRendering=(ToolboxSize<70)?1:0;
	IsHighQualityRendering=1;

	if (m_IsSubtoolbox==-4) //Special drawing toolbox
	{
		CDC xdc;
		xdc.CreateCompatibleDC(dc);
		CBitmap xbmp;
		xbmp.CreateCompatibleBitmap(dc,ToolboxSize/2+2,ToolboxSize/2+2);
		xdc.SelectObject(xbmp);
		
		if (xdc.GetDeviceCaps(BITSPIXEL)>16)
		{
			for (int i=0;i<ToolboxSize/2;i+=2)
			{
				int k=i/2;
				k=240-k;
				xdc.FillSolidRect(0,i,ToolboxSize/2,2,RGB(k,k,255));
			}
		}
		else
			xdc.FillSolidRect(0,0,ToolboxSize/2+1,ToolboxSize/2+1,(SHADOW_BLUE_COLOR));

		int m=-1,s;
		if ((member>=0) && (member<=8)) {m=SpecialDrawingToolbox[member].member;s=SpecialDrawingToolbox[member].submember;}

		int x=(member%3)*ToolboxSize/2;
		int y=(member/3)*ToolboxSize/2;
		if ((m>0) && (ToolboxMembers[m].Above[s]==-1))
		{
			if (m_SelectedElement==member)
				xdc.FillSolidRect(0,0,ToolboxSize/2,ToolboxSize/2,SHADOW_BLUE_COLOR2);

			int zm;
			for (zm=70;zm>30;zm-=4)
			{
				ToolboxMembers[m].prevToolboxSize[s]=ToolboxSize;
				ToolboxMembers[m].zoom[s]=zm*ToolboxSize/50;
				if (ToolboxMembers[m].Above[s]==-1)
				{
					((CDrawing*)ToolboxMembers[m].Submembers[s])->CalculateSize(&xdc,ToolboxMembers[m].zoom[s],&ToolboxMembers[m].Length[s],&ToolboxMembers[m].Below[s]);
					if (ToolboxMembers[m].Length[s]<ToolboxSize/3) break;
				}
			}
			if (((CDrawing*)ToolboxMembers[m].Submembers[s])->OriginalForm==IsDrawingMode)
			{
				//PaintCheckedSign(&xdc,ToolboxSize/3,ToolboxSize/3,ToolboxSize/8,1);
				xdc.FillSolidRect(0,1,ToolboxSize/2,2,RGB(190,0,85));
				xdc.FillSolidRect(1,0,2,ToolboxSize/2,RGB(190,0,85));
				xdc.FillSolidRect(ToolboxSize/2-2,0,2,ToolboxSize/2,RGB(190,0,85));
				xdc.FillSolidRect(0,ToolboxSize/2-2,ToolboxSize/2,2,RGB(190,0,85));

			}
			((CDrawing*)ToolboxMembers[m].Submembers[s])->PaintDrawing(&xdc,ToolboxMembers[m].zoom[s],(ToolboxSize/2-ToolboxMembers[m].Length[s])/2+1,ToolboxSize/16,-1,-1);

			xdc.SelectObject(GetFontFromPool(4,0,0,12+ToolboxSize/32));
			xdc.SetBkMode(TRANSPARENT);
			char buff[1];
			buff[0]=member+'1';
			xdc.TextOutA(2,ToolboxSize/2-12-ToolboxSize/64,buff,1);
			xdc.SetBkMode(OPAQUE);
		}
		dc->BitBlt(x,y,ToolboxSize/2+1,ToolboxSize/2+1,&xdc,0,0,SRCCOPY);
		dc->SelectObject(GetPenFromPool(1,0,RGB(96,96,96)));
		for (int i=0;i<4;i++)
		{
			dc->MoveTo(i*ToolboxSize/2,0);dc->LineTo(i*ToolboxSize/2,ToolboxSize*3/2);
			dc->MoveTo(0,i*ToolboxSize/2);dc->LineTo(3*ToolboxSize/2+1,i*ToolboxSize/2);

		}
		dc->FillSolidRect(2,cr.bottom-1,cr.right,2,RGB(64,64,64));
		dc->FillSolidRect(cr.right-1,2,2,cr.bottom,RGB(64,64,64));
		dc->FillSolidRect(0,cr.bottom-1,2,2,RGB(255,255,255));
		dc->FillSolidRect(cr.right-1,0,2,2,RGB(255,255,255));
		
		IsHighQualityRendering=tmpHQR;
		IsHalftoneRendering=tmpHR;	
		return 0;
	}


	int Lx=(ToolboxSize+((m_IsMain)?2:0))/4;
	int Ly=m_ItemHeight/2;
	int Cx,Cy;
	if (m_IsMain)
	{
		j=member;
		i=ToolboxMembers[j].SelectedSubmember;
		if ((i<0) || (i>64)) 
		{
			IsHighQualityRendering=tmpHQR;
			IsHalftoneRendering=tmpHR;
			return 0;
		}
		Cx=(j%2)*ToolboxSize/2+(ToolboxSize+2)/4-1;
		//Cy=(j/2)*2*ToolboxSize/3+ToolboxSize/2+ToolboxSize/3-1;
		Cy=(j/2)*m_ItemHeight+Ly-1+ToolboxSize/2;
	}
	else if (m_IsSubtoolbox>0)
	{
		j=m_IsSubtoolbox-1;
		i=member;
		Cx=(i/2)*ToolboxSize/2+ToolboxSize/4;
		//Cy=(i%2)*2*ToolboxSize/3+ToolboxSize/3;
		Cy=(i%2)*m_ItemHeight+Ly;
	}

	CDC xdc;
	xdc.CreateCompatibleDC(dc);
	CBitmap xbmp;
	xbmp.CreateCompatibleBitmap(dc,ToolboxSize/2,ToolboxSize);
	
	xdc.SelectObject(xbmp);

	int issel=0;
	if ((member==m_SelectedElement) && (!IsBlue)) issel=1;
	if (issel)
		xdc.FillSolidRect(0,0,ToolboxSize/2,m_ItemHeight,RGB(200,200,255));
	else if (xdc.GetDeviceCaps(BITSPIXEL)>16) 
	{
		int aa=m_ItemHeight;
		int bb=ToolboxSize/2+1;
		unsigned char cc=255;
		for (int i=0;i<aa;i+=4)
		{
			cc--;
			xdc.FillSolidRect(0,i,bb,4,RGB(cc,cc,255));
		}
	}
	else
		xdc.FillSolidRect(0,0,ToolboxSize/2,m_ItemHeight,RGB(255,255,255));


	
	if (ToolboxMembers[j].prevToolboxSize[i]!=m_ItemHeight*2)
	{
		int zm=70;
		ToolboxMembers[j].prevToolboxSize[i]=m_ItemHeight*2;
		ToolboxMembers[j].zoom[i]=zm*ToolboxSize/50;
		int corr=0;
		int llx,lly;
		if (ToolboxMembers[j].Above[i]==-1)
		{
			((CDrawing*)ToolboxMembers[j].Submembers[i])->CalculateSize(&xdc,ToolboxMembers[j].zoom[i],&ToolboxMembers[j].Length[i],&ToolboxMembers[j].Below[i]);
			lly=m_ItemHeight;
			llx=ToolboxSize/3;
		}
		else
		{
			ToolboxMembers[j].Submembers[i]->CalculateSize(&xdc,ToolboxMembers[j].zoom[i],&ToolboxMembers[j].Length[i],&ToolboxMembers[j].Above[i],&ToolboxMembers[j].Below[i],1,1);
			if (ToolboxMembers[j].Submembers[i]->m_pElementList->Type==5) corr=ToolboxSize/8;
			lly=27*m_ItemHeight/32-corr;
			llx=ToolboxSize/2+8-zm/10-corr;
		}
		int delta1=32*(ToolboxMembers[j].Length[i])/llx;
		int delta2=32*(ToolboxMembers[j].Above[i]+ToolboxMembers[j].Below[i])/lly;
		if (delta2>delta1) delta1=delta2;
		if (delta1>32)
		{
			zm=zm*32/delta1;
			int minzoom=(ToolboxMembers[j].userdef_mask&(1<<i))?15:25;
			if (zm<minzoom)zm=minzoom;
		}
		
		if (zm!=70)
		{
			ToolboxMembers[j].zoom[i]=zm*ToolboxSize/50;
			if (ToolboxMembers[j].Above[i]==-1)
				((CDrawing*)ToolboxMembers[j].Submembers[i])->CalculateSize(&xdc,ToolboxMembers[j].zoom[i],&ToolboxMembers[j].Length[i],&ToolboxMembers[j].Below[i]);
			else
				ToolboxMembers[j].Submembers[i]->CalculateSize(&xdc,ToolboxMembers[j].zoom[i],&ToolboxMembers[j].Length[i],&ToolboxMembers[j].Above[i],&ToolboxMembers[j].Below[i],1,1);
		}
	}
	int DeltaY=0;
	
	if ((ToolboxMembers[j].Above[i]>ToolboxSize/3) && (ToolboxMembers[j].Below[i]>ToolboxSize/3))
		DeltaY=(ToolboxMembers[j].Above[i]-ToolboxMembers[j].Below[i])/2;
	else if (ToolboxMembers[j].Above[i]>ToolboxSize/3)
		DeltaY=ToolboxMembers[j].Above[i]-ToolboxSize/3;
	else if (ToolboxMembers[j].Below[i]>ToolboxSize/3)
		DeltaY=-(ToolboxMembers[j].Below[i]-ToolboxSize/3);
	if (ToolboxMembers[j].Above[i]==-1)
	{
		DeltaY=ToolboxMembers[j].Below[i]/2;
	}


	int move=0;if (IsBlue) move=(ToolboxSize>70)?2:1;

	if (m_IsMain)
	{
		for (int i=ToolboxSize/5+move-1;i>=0;i-=4)
		{
			if (!issel)
			{
				xdc.SelectObject(GetPenFromPool(1,0,RGB(255,255,255)));
				xdc.MoveTo(2*Lx-i-2,2*Ly-0);
				xdc.LineTo(2*Lx-0,2*Ly-i-2);
			}
			xdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(176,176,255)));
			xdc.MoveTo(2*Lx-i-1,2*Ly-0);
			xdc.LineTo(2*Lx-0,2*Ly-i-1);
			xdc.MoveTo(2*Lx-i-0,2*Ly-0);
			xdc.LineTo(2*Lx-0,2*Ly-i-0);
			if (i)
			{
				xdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(220,220,255)));
				xdc.MoveTo(2*Lx-i+1,2*Ly-0);
				xdc.LineTo(2*Lx-0,2*Ly-i+1);
			}
		}
	}
	else if (m_IsSubtoolbox)
	{
		for (int i=0;i<=ToolboxSize/5+move-2;i+=4)
		{
			xdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(176,176,255)));
			xdc.MoveTo(i+1,0);
			xdc.LineTo(0,i+1);
			xdc.MoveTo(i+2,0);
			xdc.LineTo(0,i+2);
			xdc.SelectObject(GetPenFromPool(1,0,IsBlue?RGB(0,0,255):RGB(220,220,255)));
			xdc.MoveTo(i+3,0);
			xdc.LineTo(0,i+3);
		}
	}

	if (ToolboxMembers[j].Above[i]==-1) //the drawing element
		((CDrawing*)ToolboxMembers[j].Submembers[i])->PaintDrawing(&xdc,ToolboxMembers[j].zoom[i],Lx-ToolboxMembers[j].Length[i]/2,Ly-DeltaY,-1,-1);
	else
		ToolboxMembers[j].Submembers[i]->PaintExpression(&xdc,ToolboxMembers[j].zoom[i],Lx-ToolboxMembers[j].Length[i]/2,Ly+DeltaY);


	dc->BitBlt(Cx-Lx+1,Cy-Ly+1,2*Lx-1,2*Ly-1,&xdc,1,1,SRCCOPY);

	dc->SelectObject(GetPenFromPool(1,0,RGB(96,96,96)));

	if (m_IsMain)
	{
		dc->MoveTo(0,ToolboxSize/2);dc->LineTo(cr.right,ToolboxSize/2);
		dc->MoveTo(0,cr.bottom);dc->LineTo(cr.right,cr.bottom);

		dc->MoveTo(0,Cy+Ly);dc->LineTo(cr.right,Cy+Ly);
		int ttt=0;if (ToolboxSize==102) ttt=-1;
		dc->MoveTo(Cx+Lx+ttt,ToolboxSize/2);dc->LineTo(Cx+Lx+ttt,(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2/*cr.bottom*/);
		dc->MoveTo(cr.right,0);dc->LineTo(cr.right,cr.bottom+1);
		dc->MoveTo(0,0);dc->LineTo(0,cr.bottom);

	}
	else if (m_IsSubtoolbox>0)
	{
		dc->MoveTo(0,0);
		dc->LineTo(cr.right-2,0);dc->LineTo(cr.right-2,cr.bottom-2);dc->LineTo(0,cr.bottom-2);dc->LineTo(0,0);
		dc->MoveTo(Cx-Lx,0);dc->LineTo(Cx-Lx,cr.bottom-2);
		dc->MoveTo(0,Cy+Ly);dc->LineTo(cr.right,Cy+Ly);
		dc->FillSolidRect(2,cr.bottom-1,cr.right,2,RGB(64,64,64));
		dc->FillSolidRect(cr.right-1,2,2,cr.bottom,RGB(64,64,64));
		dc->FillSolidRect(0,cr.bottom-1,2,2,RGB(255,255,255));
		dc->FillSolidRect(cr.right-1,0,2,2,RGB(255,255,255));
	}


	IsHighQualityRendering=tmpHQR;
	IsHalftoneRendering=tmpHR;
	return 0;
}

int toolbar_prevZoom=0;
int toolbar_prevPage=0;
int toolbar_maxX;
void CToolbox::UpdateToolbar(char force_redraw)
{
	if (!this) return;
	if (!UseToolbar) return;
	if (this->m_IsToolbar==0) return;
	if (toolbar_maxX<100) return;

	RECT r;
	GetClientRect(&r);

	int page=(ViewY+30)/PaperHeight+1;
	
	if (toolbar_maxX<r.right-ToolboxSize/2)
	{
		if ((toolbar_prevPage==page) && (toolbar_prevZoom==ViewZoom) && (!force_redraw)) return;

		CDC *pdc;
		pdc=GetDC();
		CBitmap mb;
		mb.CreateCompatibleBitmap(pdc,ToolboxSize,r.bottom);
		CDC mdc;
		mdc.CreateCompatibleDC(pdc);
		mdc.SelectObject(mb);

		if (pdc->GetDeviceCaps(BITSPIXEL)>16)
		{
			int j=0;
			for (int i=0;i<r.bottom;i+=3,j+=2)
			{
				unsigned int color=RGB(240-j,240-j,255);
				mdc.FillSolidRect(0,i,ToolboxSize,3,color);
			}
		}
		else
			mdc.FillSolidRect(0,0,ToolboxSize,r.bottom,RGB(224,224,255));

		int h=ToolboxSize/4;
		mdc.SelectObject(GetFontFromPool(4,0,0,max(14,(ToolboxSize+30)/7)));
		mdc.SetTextAlign(TA_RIGHT);
		mdc.SetBkMode(TRANSPARENT);
		char buff[64];

		sprintf(buff,"%d",ViewZoom);
		mdc.SetTextColor(RGB(160,160,160));
		mdc.TextOut(ToolboxSize-2,(ToolboxSize-25)/32,buff,(int)strlen(buff));
		CSize ex=mdc.GetTextExtent(buff,(int)strlen(buff));
		
		sprintf(buff,"%d",page);
		mdc.TextOut(ToolboxSize-2,h,buff,(int)strlen(buff));
		CSize ex2=mdc.GetTextExtent(buff,(int)strlen(buff));
		mdc.SetTextColor(RGB(192,192,192));
		mdc.TextOut(ToolboxSize-2-ex.cx,(ToolboxSize-25)/32,"Z:",2);
		mdc.TextOut(ToolboxSize-2-ex2.cx,h,"P:",2);
		

		toolbar_prevZoom=ViewZoom;
		toolbar_prevPage=page;

		pdc->BitBlt(r.right-ToolboxSize/2,0,ToolboxSize/2,r.bottom,&mdc,ToolboxSize-ToolboxSize/2,0,SRCCOPY);
		ReleaseDC(pdc);
	}
	else
	{
		toolbar_prevZoom=0;
		toolbar_prevPage=0;
	}
}

CBitmap *ToolbarBitmap;
int ToolbarBitmapSizeX;
int ToolbarBitmapSizeY;
int CToolbox::PaintToolbar(CDC *dc)
{
	if (Toolbox->Toolbar==0) return 0;

	RECT r;
	Toolbox->Toolbar->GetClientRect(&r);
	CDC pdc;
	pdc.CreateCompatibleDC(dc);
	if ((ToolbarBitmapSizeX<r.right+150) || (ToolbarBitmapSizeY<r.bottom))
	{
		if (ToolbarBitmap) delete ToolbarBitmap;
		ToolbarBitmap=new CBitmap();
		ToolbarBitmapSizeX=r.right+150;
		ToolbarBitmapSizeY=r.bottom;
		ToolbarBitmap->CreateCompatibleBitmap(dc,ToolbarBitmapSizeX,ToolbarBitmapSizeY);
	}
	if (!ToolbarBitmap) return 0;
	pdc.SelectObject(ToolbarBitmap);

	if (pdc.GetDeviceCaps(BITSPIXEL)>16)
	{
		int j=0;
		for (int i=0;i<r.bottom;i+=3,j+=2)
		{
			unsigned int color=RGB(240-j,240-j,255);
			pdc.FillSolidRect(0,i,r.right,3,color);
		}
	}
	else
		pdc.FillSolidRect(0,0,r.right,r.bottom,RGB(224,224,255));

	CBitmap bmp;
	if (ToolboxSize>110) bmp.LoadBitmap(IDB_TOOLBAR2); else bmp.LoadBitmap(IDB_TOOLBAR);
	CDC mdc;
	mdc.CreateCompatibleDC(&pdc);
	mdc.SelectObject(bmp);
	
	pdc.SetTextColor(RGB(0,0,0));
	pdc.SetBkMode(TRANSPARENT);
	pdc.SetTextAlign(TA_CENTER);

	int x=ToolboxSize/8;
	int y=ToolboxSize/30+max(0,ToolboxSize-84)/8;if (ToolboxSize>110) y-=3;
	int wasshort=1;
	int iconwidth=24; if (ToolboxSize>110) iconwidth=31;

	for (int i=0;i<ToolbarNumItems;i++)
	{
		int icon=ToolbarConfig[i].IconIndex;
		int len=ToolbarConfig[i].pixel_len;
		int t=(len-ToolbarIcons[icon].pixel_len+3-2)/2;
	
		unsigned int color=RGB(0,128,128);
		if ((i==ToolbarSelectedItem) && (ToolbarConfig[i].IconState)) 
		{
			if (ToolbarIsClicked)
				pdc.FillSolidRect(x+1,0,len-2,ToolboxSize/2,RGB(128,128,255));
			else
				pdc.FillSolidRect(x+1,0,len-2,ToolboxSize/2,RGB(192,192,240));
		}
		if (ToolbarConfig[i].IconState==0) color=RGB(192,192,192);

		pdc.FillSolidRect(r.right+ToolboxSize/2,0,iconwidth,iconwidth,color);
		int iconposx=icon*(iconwidth+1)+1;
		pdc.BitBlt(r.right+ToolboxSize/2,0,iconwidth,iconwidth+1,&mdc,iconposx,0,SRCPAINT);
		pdc.TransparentBlt(x+t,y,iconwidth,iconwidth,&pdc,r.right+ToolboxSize/2,0,iconwidth,iconwidth,RGB(255,255,255));
		if (ToolbarConfig[i].IconState==2)
			PaintCheckedSign(&pdc, x+t+iconwidth/2-((ToolboxSize>=120)?0:2),y+iconwidth/2+((ToolboxSize>=120)?3:0),14, 1);
		if (ToolbarConfig[i].IconState==3)
			PaintCheckedSign(&pdc, x+t+iconwidth/2-((ToolboxSize>=120)?0:2),y+iconwidth/2+((ToolboxSize>=120)?3:0),14, 2);

		if (ToolboxSize>=80) //we don't paint description texts under icons if toolbox is of very small size
		{
			char buff[64];
			CopyTranslatedString(buff,ToolbarIcons[icon].name,ToolbarIcons[icon].lang_code,64);
			int l=(int)strlen(buff);
			for (int k=1;k<l;k++)
			{
				if ((buff[k]==9) || (buff[k]==10) || (buff[k]==13)) {buff[k]=0;l=k;break;}
			}
			if (l>7)
			{
				for (int k=10;k<l;k++)
					if ((buff[k]=='.') || (buff[k]==' ') || (buff[k]==',') || (buff[k]==';') || (buff[k]=='-')) {buff[k]=0;l=k;break;}
			}
			if (l>13)
			{
				buff[11]='.';
				buff[12]=0;
				l=12;
			}
			int fntsz=13;
			if (ToolboxSize>96) fntsz=15;
			if (ToolboxSize>115) fntsz=17;
			if (l>9) fntsz--;
			pdc.SelectObject(GetFontFromPool(4,0,0,fntsz));
			if (ToolbarConfig[i].IconState==0) pdc.SetTextColor(RGB(160,160,160)); else pdc.SetTextColor(RGB(0,0,0));
			pdc.TextOutA(x+len/2+1,y+fntsz+ToolboxSize/8+1,buff,(int)strlen(buff));
		
		}

		int isshort=(ToolbarConfig[i].pixel_separator)?0:1;
		if (!wasshort)
		{
			int t=ToolbarConfig[i-1].pixel_separator/2;
			pdc.FillSolidRect(x-t,2,1,r.bottom-4,RGB(200,200,255));
			pdc.FillSolidRect(x-t+1,2,1,r.bottom-4,RGB(255,255,255));
		}

		wasshort=isshort;
		x+=ToolbarConfig[i].pixel_len+ToolbarConfig[i].pixel_separator;
	}

	toolbar_maxX=x;
	if (toolbar_maxX<r.right-ToolboxSize/2)
		dc->BitBlt(0,0,r.right-ToolboxSize/2,r.bottom,&pdc,0,0,SRCCOPY);
	else
		dc->BitBlt(0,0,r.right,r.bottom,&pdc,0,0,SRCCOPY);
	UpdateToolbar(1);

	return 1;
}

void CToolbox::ToolbarShowHelp()
{
	if (ToolbarSelectedItem<0) return;
	POINT p;
	GetCursorPos(&p);
	pMainView->ScreenToClient(&p);

	int icon=ToolbarConfig[ToolbarSelectedItem].IconIndex;

	if ((ToolboxSize<80) || (strcmp(ToolbarIcons[icon].name,ToolbarIcons[icon].long_name)))
	{
		char buff[128];
		CopyTranslatedString(buff,ToolbarIcons[icon].long_name,ToolbarIcons[icon].lang_code,127);
		for (int i=0;i<(int)strlen(buff);i++)
		{
			if ((buff[i]<=13) && (buff[i]>0)) {buff[i]=0;break;} //remove special cahracters
		}
		DisplayShortText(buff,ToolbarItemPos+ToolbarConfig[ToolbarSelectedItem].pixel_len/2-2,min(25,max(1,p.y+18)),65535,4);
	}
}

void AddToolbarOption(unsigned char icon_index, unsigned char icon_state, unsigned char pixel_len, unsigned char pixel_separator)
{
	int items=ToolbarNumItems;
	ToolbarConfig[items].IconIndex=icon_index;
	ToolbarConfig[items].IconState=icon_state;
	ToolbarConfig[items].pixel_len=pixel_len;
	ToolbarConfig[items].pixel_separator=pixel_separator;
	ToolbarNumItems++;
}

int CToolbox::ConfigureToolbar()
{
	//this functions decides what options are going to be visible in the toolbar
	//must work fast!!!
	if ((UseToolbar==0) || (Toolbox->Toolbar==NULL) || (Toolbox->Toolbar->m_hWnd==NULL)) return 0;

	unsigned int checksum=0;
	for (int i=0;i<ToolbarNumItems;i++)
	{
		checksum+=ToolbarConfig[i].IconIndex*64*64+ToolbarConfig[i].IconState*64*64*64+ToolbarConfig[i].pixel_len+ToolbarConfig[i].pixel_separator*64;
		checksum*=(i+1);
	}

	int items=0;
	int LongSeparator=2;//ToolboxSize/6;
	int ShortSeparator=0;//ToolboxSize/24;
	int VeryLongSeparator=ToolboxSize/4;
	int pixel_len=2*ToolboxSize/3;

	RECT r;
	Toolbox->Toolbar->GetClientRect(&r);
	int factor=r.right/(pixel_len+1);

	//items that are always present

	ToolbarNumItems=0;

	int has_selection=0;
	int last_drawing_delete=0;
	if (NumSelectedObjects) has_selection=1;
	else if ((KeyboardEntryBaseObject) && (KeyboardEntryObject) && (((CExpression*)KeyboardEntryObject)->m_IsKeyboardEntry))
	{
		//check if there is a keyboard selection
		CExpression *e=(CExpression*)KeyboardEntryObject;
		for (int i=0;i<e->m_NumElements;i++)
			if ((e->m_pElementList+i)->IsSelected==2) {has_selection=2;break;}
	}
	if ((IsDrawingMode) && (NumDocumentElements>0) && (TheDocument[NumDocumentElements-1].Type==2) && (!KeyboardEntryObject)) 
		last_drawing_delete=1;

	//adding 'save' option
	if (factor>7)
	AddToolbarOption(1,NumDocumentElements?1:0,pixel_len,LongSeparator);

	//adding 'undo' option
	AddToolbarOption(0,1,pixel_len,LongSeparator);

	//adding 'cut' option
	AddToolbarOption(2,((has_selection)||(last_drawing_delete))?1:0,pixel_len,ShortSeparator);

	//adding 'copy' option
	AddToolbarOption(3,(has_selection)?1:0,pixel_len,ShortSeparator);

	//adding 'paste' option
	AddToolbarOption(4,1,pixel_len,LongSeparator);

	//adding 'image' option
	//AddToolbarOption(6,(NumSelectedObjects)?1:0,pixel_len,LongSeparator);

	//adding '1:1 zoom' option
	AddToolbarOption(5,(ViewZoom==DefaultZoom)?0:1,pixel_len,ShortSeparator);

	//adding 'grid' option
	if (factor>7)
	AddToolbarOption(7,(IsShowGrid)?2:1,pixel_len,ShortSeparator);
	
	//adding 'presentation mode' option
	if (factor>7)
	AddToolbarOption(39,(IsHighQualityRendering)?2:1,pixel_len,LongSeparator);

	/*if (factor>7)
	{
		//adding 'Edit nodes' option
		AddToolbarOption(17,(ToolbarEditNodes)?((IsDrawingMode)?3:2):1,pixel_len,LongSeparator);
	}*/


	int additional_options=0;

	if ((factor>20) || 
		((KeyboardEntryObject) && (KeyboardEntryBaseObject) && (NumSelectedObjects<=1)) ||
		((NumSelectedObjects==1) && (NumSelectedDrawings==0)))
	{
		int tmp=0;
		CExpression *parent=NULL;
		CExpression *exp=NULL;


		if ((KeyboardEntryObject) && (KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1)) 
		{
			parent=((CExpression*)KeyboardEntryBaseObject->Object);
			exp=((CExpression*)KeyboardEntryObject);
		}
		if ((NumSelectedObjects==1) && (KeyboardEntryObject==NULL))
		{
			tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
			for (int i=NumDocumentElements-1;i>=0;i--,ds--)
			{
				if (ds->MovingDotState==3) 
				{
					if (ds->Type==1) exp=parent=((CExpression*)ds->Object);
					break;
				}
			}
		}
		
		//adding 'Increase font size' option
		AddToolbarOption(9,(parent)?1:0,pixel_len,ShortSeparator);
		//adding 'Decrease font size' option
		AddToolbarOption(10,(parent)?1:0,pixel_len,ShortSeparator);
		//adding 'Headline' option
		AddToolbarOption(19,(parent)?((parent->m_IsHeadline)?2:1):0,pixel_len,ShortSeparator);
		//adding 'Vertical orientation' option
		AddToolbarOption(18,(parent)?((parent->m_IsVertical)?2:1):0,pixel_len,LongSeparator);

		if (has_selection==2)
		{
			additional_options=1;
			//adding 'Bold' option
			AddToolbarOption(28,1,pixel_len,ShortSeparator);
			//adding 'Italic' option
			AddToolbarOption(29,1,pixel_len,ShortSeparator);
			//adding 'Underline' option
			AddToolbarOption(30,1,pixel_len,ShortSeparator);
			//adding 'Underbrace' option
			AddToolbarOption(31,1,pixel_len,ShortSeparator);
		}
		if ((exp) && (has_selection!=2))
		{
			int align=exp->m_Alignment;
			int add_align_options=0;

			if ((exp->m_MaxNumRows>1) || (exp->m_MaxNumColumns>1))
			{
				if (exp->m_IsKeyboardEntry)
				{
					add_align_options=1;
					for (int i=0;i<exp->m_MaxNumRows;i++)
						for (int j=0;j<exp->m_MaxNumColumns;j++)
						{
							int k=exp->FindMatrixElement(i,j,0);
							if (k>exp->m_IsKeyboardEntry-1)
							{
								i=1000; //force exit;
								break;
							}
							tCellAttributes a;
							if (exp->GetCellAttributes(i,j,&a))
							{
							if (*a.alignment=='l') align=1;
							if (*a.alignment=='c') align=0;
							if (*a.alignment=='r') align=2;
							}
						}
				}
			}
			else
			{
				for (int i=0;i<exp->m_NumElements;i++)
					if (((exp->m_pElementList+i)->Type==2) && ((exp->m_pElementList+i)->pElementObject->Data1[0]==(char)0xFF))
					{
						add_align_options=1;
						break;
					}
			}


			if (add_align_options)
			{
				//adding 'left align' option
				AddToolbarOption(25,(align==1)?2:1,pixel_len,ShortSeparator);
				//adding 'center align' option
				AddToolbarOption(26,(align==0)?2:1,pixel_len,ShortSeparator);
				//adding 'right align' option
				AddToolbarOption(27,(align==2)?2:1,pixel_len,LongSeparator);
			}
		}
	}

	if ((NumSelectedObjects>1) || ((NumSelectedDrawings==1) && (NumSelectedObjects==1)))
	{
		int tmp=ToolboxSize/24;
		if (NumSelectedObjects>1)
		{
			//adding 'Align left' option
			AddToolbarOption(11,1,pixel_len-tmp,ShortSeparator);
			//adding 'Align h. center' option
			AddToolbarOption(12,1,pixel_len-tmp,ShortSeparator);
			//adding 'Align right' option
			AddToolbarOption(13,1,pixel_len-tmp,ShortSeparator);
			//adding 'Align top' option
			AddToolbarOption(14,1,pixel_len-tmp,ShortSeparator);
			//adding 'Align v. center' option
			AddToolbarOption(15,1,pixel_len-tmp,ShortSeparator);
			//adding 'Align bottom' option
			AddToolbarOption(16,1,pixel_len-tmp,ShortSeparator);
		}
		if (NumSelectedDrawings)
		{
			//adding 'H.mirror' option
			AddToolbarOption(20,1,pixel_len-tmp,ShortSeparator);
			//adding 'V.mirror' option
			AddToolbarOption(21,1,pixel_len-tmp,ShortSeparator);
		}
		if (NumSelectedObjects==1)
		{
			//adding the "Ungroup" option
			tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
			for (int i=NumDocumentElements-1;i>=0;i--,ds--)
			{
				if (ds->MovingDotState==3) 
				{
					if (ds->Type==2)
					{
						CDrawing *d=(CDrawing*)ds->Object;
						if ((d->NumItems) && (d->Items->pSubdrawing) && ((d->Items->Type==2) ||(d->Items->Type==0)))
							AddToolbarOption(23,1,pixel_len-tmp,ShortSeparator);
					}
					break;
				}
			}
			
		}
		if (NumSelectedObjects>1)
		{
			//adding 'Group' option
			AddToolbarOption(22,1,pixel_len-tmp,ShortSeparator);
			//adding 'Distribute' option
			AddToolbarOption(24,1,pixel_len-tmp,ShortSeparator);
		}
	} 
	else if (!additional_options)
	{

		if (factor-ToolbarNumItems<5)
		{
			//adding 'Drawing mode' option
			AddToolbarOption(8,1,pixel_len,ShortSeparator);
		}
		else
		{
			//adding 'Line' option
			AddToolbarOption(32,(IsDrawingMode==2)?2:1,pixel_len,ShortSeparator);
			//adding 'Freehand' option
			AddToolbarOption(33,(IsDrawingMode==3)?2:1,pixel_len,ShortSeparator);
			//adding 'Arrow' option
			AddToolbarOption(34,(IsDrawingMode==5)?2:1,pixel_len,ShortSeparator);
			//adding 'Ellipse' option
			AddToolbarOption(36,(IsDrawingMode==4)?2:1,pixel_len,ShortSeparator);
			//adding 'Rectangle' option
			AddToolbarOption(35,(IsDrawingMode==1)?2:1,pixel_len,ShortSeparator);			
			//adding 'Eraser' option
			//AddToolbarOption(37,(IsDrawingMode==6)?2:1,pixel_len,ShortSeparator);
			//adding 'Pencil Eraser' option
			AddToolbarOption(38,(IsDrawingMode==7)?2:1,pixel_len,ShortSeparator);
		}

		/*if ((IsDrawingMode) || (ToolbarEditNodes))
		{
			//adding 'Edit nodes' option
			AddToolbarOption(17,(ToolbarEditNodes)?2:1,pixel_len,ShortSeparator);
		}*/

	}

	unsigned int checksum2=0;
	for (int i=0;i<ToolbarNumItems;i++)
	{
		checksum2+=ToolbarConfig[i].IconIndex*64*64+ToolbarConfig[i].IconState*64*64*64+ToolbarConfig[i].pixel_len+ToolbarConfig[i].pixel_separator*64;
		checksum2*=(i+1);
	}
	if (checksum!=checksum2)
		if (Toolbox->Toolbar) {Toolbox->Toolbar->InvalidateRect(NULL,0);Toolbox->Toolbar->UpdateWindow();}
	return 1;
}

int CToolbox::HideSubtoolbox(void)
{
	if (m_IsMain)
	{
		Subtoolbox->ShowWindow(SW_HIDE);
		Subtoolbox->m_IsSubtoolbox=-1;
	}
	else if (m_IsSubtoolbox)
	{
		m_IsSubtoolbox=-1;
		ShowWindow(SW_HIDE);
	}
	return 0;
}




int prevToolboxElement=-1;
int prevSubtoolboxElement=-1;
int prevFontElement=-1;
int prevKeyboardElement=-1;
int prevToolbarItem=-1;
void CToolbox::OnMouseMove(UINT nFlags, CPoint point)
{
	try
	{
	
	if (m_IsToolbar)
		SetCursor(::LoadCursor(NULL,IDC_ARROW));
	else
		pMainView->SetMousePointer();

	m_SelectedColor=-1;
	m_SelectedTextControl=-1;
	int element=-1;
	int font_element=-1;
	RECT cr;
	GetClientRect(&cr);

	if (!(this->m_IsMain)) ToolboxAutoopen=0;
	//release capture if mouse outside toolbox window
	if ((point.x<0) || (point.y<0) || (point.x>cr.right) || (point.y>cr.bottom))
	{

		ReleaseCapture();
		if (m_IsSubtoolbox==-4)
		{
			m_IsSubtoolbox=0;
			ShowWindow(SW_HIDE);
			pMainView->RepaintTheView();
			
		}
		if (this->m_IsMain) ToolboxAutoopen=0;
		//if (this->m_IsToolbar) 
		{
			if ((StaticMessageWindow) && (StaticMessageWindow->IsWindowVisible()) && (ToolbarHelpTimer>1))
			{
				StaticMessageWindow->ShowWindow(SW_HIDE);
				InvalidateRect(NULL,1);
			}
			ToolbarHelpTimer=0;
			if ((ToolbarSelectedItem>-1) && (Toolbox->Toolbar))
			{
				ToolbarSelectedItem=-1;
				prevToolbarItem=-1;
				Toolbox->Toolbar->InvalidateRect(NULL,0);
				Toolbox->Toolbar->UpdateWindow();
			}
			
		}
		goto finish_toolbox_mousemove;
	}

	//also release capture if window is not visible
	if (!IsWindowVisible())
	{

		ReleaseCapture();
		if (this->m_IsMain) ToolboxAutoopen=0;
		goto finish_toolbox_mousemove;
	}

	//also release capture if cursor is over subtoolbox window, contextmenu window or keyboard window
	if (1)
	{
		POINT cursor;
		GetCursorPos(&cursor);
		if (WindowFromPoint(cursor)!=this)
		{
			if (m_IsSubtoolbox==-4)
			{
				m_IsSubtoolbox=0;
				ShowWindow(SW_HIDE);
				pMainView->RepaintTheView();
				
			}
			//if  (this->m_IsToolbar) 
			{
				if ((StaticMessageWindow) && (StaticMessageWindow->IsWindowVisible()) && (ToolbarHelpTimer>1))
				{
					StaticMessageWindow->ShowWindow(SW_HIDE);
					InvalidateRect(NULL,1);
				}
				ToolbarHelpTimer=0;
				if ((ToolbarSelectedItem>-1) && (Toolbox->Toolbar))
				{
					ToolbarSelectedItem=-1;
					prevToolbarItem=-1;
					Toolbox->Toolbar->InvalidateRect(NULL,0);
					Toolbox->Toolbar->UpdateWindow();
				}
			}
			ReleaseCapture();
			goto finish_toolbox_mousemove;
		}
	}

	if (m_IsToolbar==0) ToolbarHelpTimer=0;

	//also release the cursor
	if ((!m_IsContextMenu) && (Toolbox->ContextMenu->IsWindowVisible()))
	{
		POINT cursor;
		GetCursorPos(&cursor);
		if (WindowFromPoint(cursor)==Toolbox->ContextMenu)
		{
			ReleaseCapture();
			goto finish_toolbox_mousemove;
		}
	}
	//otherwise, capture the mouse pointer
	SetCapture();

	if (m_IsMain)
	{
		if (point.y<ToolboxSize/2)
		{
			//mouse over main-toolbox header
			int xx=point.x*2/ToolboxSize;

			int Xres=point.x-(xx+1)*ToolboxSize/2;
			int Yres=point.y-ToolboxSize/2;

			m_FontModeElement=xx;
			font_element=xx;
			if (Xres+Yres>=-ToolboxSize/5)
				m_IsArrowSelected=1;
			else
				m_IsArrowSelected=0;
			m_SelectedElement=-1;
			PaintToolboxHeader(this->GetDC());

			if ((ClipboardExpression==NULL) && (ClipboardDrawing==NULL))
				if (m_FontModeElement!=prevFontElement)
			{
				ToolboxAutoopen=1;
			}
			ToolboxAutoopenX=point.x;
			ToolboxAutoopenY=point.y;
		}
		else if ((point.y<(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2))
		{
			//mouse over an main-toolbox element
			//calculating the element the mouse is pointing at
			if (m_FontModeElement!=-1)
			{
				m_FontModeElement=-1;
				PaintToolboxHeader(this->GetDC());
			}

			point.y-=ToolboxSize/2; //subsctract the header size
			int xx=point.x*2/ToolboxSize;
			int yy=point.y/m_ItemHeight;
			element=xx+yy*2;

			if (xx+yy*2<ToolboxNumMembers)
			{
				//int Cx,Cy;  //Centerpoints
				element=xx+yy*2;
				//Cx=xx*ToolboxSize/2+ToolboxSize/4;
				//Cy=yy*m_ItemHeight+m_ItemHeight/2+ToolboxSize/2;
				int Xres=point.x-(xx+1)*(ToolboxSize/2);
				int Yres=point.y-(yy+1)*(m_ItemHeight);
				m_SelectedElement=element;
				if (Xres+Yres>=-ToolboxSize/5-3)
				{
					//the mouse is pointing at subtoolbox opening arrow
					PaintToolboxElement(this->GetDC(),element,1);
					m_IsArrowSelected=1;
				}
				else
				{
					//the mouse is pointing at the toolbox element
					PaintToolboxElement(this->GetDC(),element,0);
					m_IsArrowSelected=0;
				}
				
			}

			if ((ClipboardExpression==NULL) && (ClipboardDrawing==NULL))
				if (/*(abs(ToolboxAutoopenX-point.x)>0) || (abs(ToolboxAutoopenY-point.y)>0) ||*/ (m_SelectedElement!=prevToolboxElement))
			{
				ToolboxAutoopen=1;
			}
			ToolboxAutoopenX=point.x;
			ToolboxAutoopenY=point.y;


		}
		else if ((point.y>=(ToolboxNumMembers+1)/2*m_ItemHeight+ToolboxSize/2+5))//mouse is pointing at colorbox or textcontrolbox
		{
			int y=point.y-(ToolboxNumMembers+1)/2*m_ItemHeight-ToolboxSize/2-5;
			if ((NumSelectedObjects) || (IsDrawingMode) || (KeyboardEntryObject==0))
			{
				//color box
				if (y<ToolboxSize/3) //selecting color
				{
					int len=ToolboxSize/5;
					m_SelectedColor=point.x/len;
				}
				else if (y<2*ToolboxSize/3) //selecting line width
				{
					int len=ToolboxSize/6;
					m_SelectedColor=point.x/len+10;
				}
				else if (y<ToolboxSize) //selecting line width
				{
					int len=ToolboxSize/3;
					m_SelectedColor=point.x/len+20;
				}
				else
					m_SelectedColor=-1;
				PaintColorbox(this->GetDC());
			}
			else
			{
				//text control box
				m_SelectedTextControl=0;
				if (y<ToolboxSize-ToolboxSize/3-ToolboxSize/4) 
					m_SelectedTextControl=1;
				else if (y<ToolboxSize-ToolboxSize/3)
				{
					int gg=(ToolboxSize+1)/5;
					if (point.x<gg) m_SelectedTextControl=5;
					else if (point.x<2*gg) m_SelectedTextControl=6;
					else if (point.x<3*gg) m_SelectedTextControl=7;
					else if (point.x<4*gg) m_SelectedTextControl=8;
					else m_SelectedTextControl=9;
				}
				else 
				{
					if (point.x<ToolboxSize/3) m_SelectedTextControl=2;
					else if (point.x<2*ToolboxSize/3) m_SelectedTextControl=3;
					else m_SelectedTextControl=4;
				}
				this->PaintTextcontrolbox(this->GetDC());
			}

		}
	}
	else if (m_IsSubtoolbox>0)
	{
		//moving cursor over subtoolbox window
		//calculating the element the mouse is pointing at
		int xx=point.x*2/ToolboxSize;
		int yy=point.y*3/ToolboxSize/2;

		if (xx*2+yy<ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers)
		{
			int Cx,Cy;  //Centerpoints
			element=xx*2+yy;
			Cx=xx*ToolboxSize/2+ToolboxSize/4;
			Cy=yy*2*ToolboxSize/3+ToolboxSize/3;
			int Xres=point.x-(xx)*(ToolboxSize/2);
			int Yres=point.y-(yy)*(2*ToolboxSize/3);
			m_SelectedElement=element;
			if (Xres+Yres<=ToolboxSize/5-1)
			{
				//the mouse is pointing at subtoolbox opening arrow
				PaintToolboxElement(this->GetDC(),element,1);
				m_IsArrowSelected=1;
			}
			else
			{
				//the mouse is pointing at the toolbox element
				PaintToolboxElement(this->GetDC(),element,0);
				m_IsArrowSelected=0;
			}
			
		}
	}
	else if (m_IsSubtoolbox==-4)
	{
		int xx=point.x*2/ToolboxSize;
		int yy=point.y*2/ToolboxSize;
		font_element=xx+yy*3;
		if (font_element<9)
		{
			theApp.m_pMainWnd->SetActiveWindow();
			element=m_SelectedElement=font_element;
			m_IsArrowSelected=0;
			PaintToolboxElement(this->GetDC(),font_element,0);
		}
	}
	else if (m_IsSubtoolbox==-2)
	{
		//subtoolbox for mixed fonts
		int row=point.y*3/ToolboxSize;
		int key=point.x*4/ToolboxSize;
		m_IsArrowSelected=0;
		if ((row==4) && (point.x>ToolboxSize*5)) m_IsArrowSelected=1;
		font_element=m_SelectedElement=row*100+key;
		PaintToolboxElement(this->GetDC(),row*100+key,0);
	}
	else if (m_IsSubtoolbox==-1)
	{
		//subtoolbox for uniform fonts
		int yy=point.y/(2*ToolboxSize/5);
		int delta_center=abs((yy*(2*ToolboxSize/5)+ToolboxSize/5)-point.y);
		if (yy<ToolboxFontFormating.NumFormats)
		{
			font_element=yy;

			//the mouse is pointing at the toolbox element
			if (point.x>2*ToolboxSize+delta_center) m_IsArrowSelected=1; else m_IsArrowSelected=0;
			m_SelectedElement=font_element;
			PaintToolboxElement(this->GetDC(),font_element,0);
		}
	}
	else if (m_IsContextMenu)
	{
		int TS=30+ToolboxSize/2;
		int xx;
		int yy=((point.y-TS/10)/(TS/4-1));
		xx=0;
		if (point.x>TS) 
		{	
			point.x-=TS;xx=1;
			if (point.x>7*TS/8) 
			{
				point.x-=7*TS/8;xx=2;
				if (point.x>10*TS/8) 
				{
					point.x-=10*TS/8;xx=3;
					if (point.x>10*TS/8) 
					{
						point.x-=10*TS/8;xx=4;
						if (point.x>11*TS/8) 
						{
							point.x-=11*TS/8;xx=5;
							if (point.x>5*TS/8) 
							{
								point.x-=5*TS/8;xx=6;
							}
						}
					}
				}
			}
		}

		if (yy>=0)
		{
			int is_header=0;
			int is_drawing=0;
			if (m_ContextMenuMember<0)
			{
				is_header=1;
			}
			else if (ToolboxMembers[m_ContextMenuMember].Above[m_ContextMenuSubmember]==-1)
			{
				is_drawing=1;
			}

			m_ContextMenuSelection=xx*23+yy;
			int exitid=0;
			for (int i=0;i<=m_ContextMenuSelection;i++)
			{
				if (ToolboxKeycodes[i].code==1) exitid=i;
				if (ToolboxKeycodes[i].code==0)
				{
					m_ContextMenuSelection=exitid;
					break;
				}
			}
			if (m_ContextMenuSelection>=0)
			if ((ToolboxKeycodes[m_ContextMenuSelection].code&0x400) && ((is_drawing) || (is_header)))
				{m_ContextMenuSelection=-1;}

			if (yy>=23) m_ContextMenuSelection=-1;
			if (prevContextMenuSelection!=m_ContextMenuSelection)
			{
				InvalidateRect(NULL,0);
				UpdateWindow();
				prevContextMenuSelection=m_ContextMenuSelection;
			}
		}
	}
	else if (m_IsKeyboard)
	{
		int i;
		for (i=0;i<ToolboxKeyboardElements.NumKeys;i++)
		{
			int X=ToolboxKeyboardElements.X[i]*ToolboxSize/30+3;
			int Y=ToolboxKeyboardElements.Y[i]*ToolboxSize/30+3;
			if ((point.x>X) && (point.x<X+ToolboxSize/3) &&
				(point.y>Y) && (point.y<Y+ToolboxSize/3))
			{
				m_KeyboardElement=i;
				element=i;
				//ToolboxKeyboardElements.Key[i]->SelectExpression(1);
				PaintKeyboardElement(this->GetDC(),i);
				break;
			}
		}
		
		if ((point.x>cr.right-20*ToolboxSize/64) && (point.y>cr.bottom-15*ToolboxSize/64) && (point.x<cr.right-2) && (point.y<cr.bottom-2))
		{
			element=255;
			if (AlternateVirtualKeyboard!=3) AlternateVirtualKeyboard=2;
			PaintKeyboardElement(this->GetDC(),element);
		}
		else if ((point.x<3) || (point.y<3) || (point.x>cr.right-2) || (point.y>cr.bottom-2))
		{
			if (AlternateVirtualKeyboard!=3) AlternateVirtualKeyboard=0;
			PaintKeyboardElement(this->GetDC(),255);
		}
		else
		{
			if (AlternateVirtualKeyboard!=3) AlternateVirtualKeyboard=1;
			PaintKeyboardElement(this->GetDC(),255);
		}
	}
	else if (m_IsToolbar)
	{
		if (!ToolbarHelpTimer) ToolbarHelpTimer=1;
		int x=ToolboxSize/8;
		ToolbarSelectedItem=-1;
		for (int i=0;i<ToolbarNumItems;i++)
		{
			int len=ToolbarConfig[i].pixel_len;

			if ((point.x>=x) && (point.x<=x+len))
			{
				ToolbarItemPos=x;
				ToolbarSelectedItem=i;
				break;
			}
			x+=ToolbarConfig[i].pixel_len+ToolbarConfig[i].pixel_separator;
		}
		if (prevToolbarItem!=ToolbarSelectedItem) 
		{
			if (ToolbarHelpTimer>=6) 
				Toolbox->Toolbar->ToolbarShowHelp();
			else
				ToolbarHelpTimer=1;
			Toolbox->Toolbar->InvalidateRect(NULL,0);
			Toolbox->Toolbar->UpdateWindow();
		}
		prevToolbarItem=ToolbarSelectedItem;
	}
	
	
finish_toolbox_mousemove:

	int help_element=-1;
	int help_subelement=-1;
	int help_font_element=-1;
	int help_font_subelement=-1;

	//clear the selection in main toolbox or subtoolbox if mouse is no more pointing at it
	if (m_IsMain)
	{
		if ((m_IsArrowSelected) && ((element>=0) || (font_element>=0)))
		{
			if (font_element==1)
				this->ShowHelptext("virtual\r\nkeyboard","","","",6051);
			else
				this->ShowHelptext("more...","","","",6050);
		}
		else if ((font_element!=prevFontElement) || (prevToolboxElement!=element))
			this->ShowHelptext("","","","",0);

		//clear selection on the element the mouse is no more pointing at
		if ((prevToolboxElement>=0) && (prevToolboxElement<ToolboxNumMembers) && 
			(prevToolboxElement!=element))
		{
			m_SelectedElement=element;
			PaintToolboxElement(this->GetDC(),prevToolboxElement,0);
		}
		prevToolboxElement=element;

		//clear selection on the element the mouse is no more pointing at
		if ((prevFontElement>=0) && (prevFontElement<3) &&  
			(prevFontElement!=font_element))
		{
			if (font_element==-1) m_FontModeElement=-1;
			PaintToolboxHeader(this->GetDC());

		}

		prevFontElement=font_element;
		help_font_element=font_element;
		if (font_element==0) help_font_subelement=ToolboxFontFormating.SelectedUniform;
		if (font_element==1) help_font_subelement=0;

		if (element>=0)
		{
			help_element=element;
			help_subelement=ToolboxMembers[element].SelectedSubmember;
		}

		if (prevSelectedTextControl>0)
		{
			this->PaintTextcontrolbox(this->GetDC());
		}
		prevSelectedTextControl=m_SelectedTextControl;

		//clear selection on the element the mouse is no more pointing at
		if (prevSelectedColor>=0)
		{
			PaintColorbox(this->GetDC());
		}
		prevSelectedColor=m_SelectedColor;
	}
	else if (m_IsSubtoolbox)
	{

		//clear selection on the element the mouse is no more pointing at
		if ((m_IsArrowSelected) && (element>=0))
		{
				this->ShowHelptext("place into\r\nmain menu","","","",6056);
		}
		else 
		if ((prevSubtoolboxElement!=element) || (prevFontElement!=font_element) || (m_IsArrowSelected)) 
			this->ShowHelptext("","","","",0);



		if ((prevSubtoolboxElement>=0) && (prevSubtoolboxElement!=element))
		{
			if (((m_IsSubtoolbox>0) && (m_IsSubtoolbox<=ToolboxNumMembers) && (prevSubtoolboxElement<ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers)) ||
				(m_IsSubtoolbox==-4))
			{
				m_SelectedElement=element;
				PaintToolboxElement(this->GetDC(),prevSubtoolboxElement,0);
			}
		}
		prevSubtoolboxElement=element;

		//clear selection on the element the mouse is no more pointing at
		if ((prevFontElement>=0) && 
			(((prevFontElement<ToolboxFontFormating.NumFormats) && (m_IsSubtoolbox==-1)) || (m_IsSubtoolbox==-2)) &&  
			(prevFontElement!=font_element))
		{
			if (font_element==-1) {m_SelectedElement=-1;}
			PaintToolboxElement(this->GetDC(),prevFontElement,0);
		}
		
		prevFontElement=font_element;
		
		help_font_element=-m_IsSubtoolbox-1;
		help_font_subelement=font_element;

		if (element>=0)
		{
			help_element=m_IsSubtoolbox-1;
			help_subelement=element;
		}
	}
	else if (m_IsKeyboard)
	{
		//clear selection on the element the mouse is no more pointing at
		if ((prevKeyboardElement>=0) /*&& (prevKeyboardElement<ToolboxKeyboardElements.NumKeys)*/ &&  
			(prevKeyboardElement!=element))
		{

			if (element==-1) {m_KeyboardElement=-1;}
			PaintKeyboardElement(this->GetDC(),prevKeyboardElement);
		}
		prevKeyboardElement=element;
	}

	if (((help_element>=0) && (help_subelement>=0)) ||
		((help_font_element>=0) && (help_font_subelement>=0)))
	{
		int langid=6000;
		char *text="";
		char *easycast="";
		short key;
		char *command="";
		char buff[32];
		strcpy(buff,"");
		if (!m_IsArrowSelected)
		{
		if (help_element>=0) 
		{
			key=ToolboxMembers[help_element].AcceleratorKey[help_subelement];
			easycast=ToolboxMembers[help_element].Keycodes[help_subelement];
			if (ToolboxMembers[help_element].Above[help_subelement]==-1)
			{
				CDrawing *drw=(CDrawing*)ToolboxMembers[help_element].Submembers[help_subelement];
				if (drw->OriginalForm==3) {text="Freehand";langid=6008;}
				if (drw->OriginalForm==1) {text="Rectangle";langid=6009;}
				if (drw->OriginalForm==2) {text="Line";langid=6010;}
				if (drw->OriginalForm==4) {text="Ellipse";langid=6011;}
				if (drw->OriginalForm==7) {text="Pencil\r\neraser";langid=6012;}
				if (drw->OriginalForm==6) {text="Eraser";langid=6012;}
				if (drw->OriginalForm==18) {text="Section\r\nline";langid=6013;}
				if (drw->OriginalForm==50) {text="Drawing Box";langid=6014;}
				if (drw->OriginalForm==51) {text="Function\r\nPlotter";langid=6015;}
				if (drw->OriginalForm==52) {text="Bitmap Box";langid=6016;}
			}
			else
			{
				CExpression *e=ToolboxMembers[help_element].Submembers[help_subelement];
				const char *cmd=GetCommandFromCreationCode(ToolboxMembers[help_element].CreationCode[help_subelement]);
				if (cmd) command=(char*)cmd;

				if ((command[0]==0) && (e) && (e->m_NumElements==1) && ((ToolboxMembers[help_element].userdef_mask&(1<<help_subelement))==0) &&
					(e->m_pElementList->Type==6) && (e->m_pElementList->pElementObject->Data2[0]==0x20) &&
					(e->m_pElementList->pElementObject->Expression2==NULL))
				{
					command=e->m_pElementList->pElementObject->Data1;
				}
			}
		}
		else
		{
			if (help_font_element==0) {key=ToolboxFontFormating.UniformAccKey[help_font_subelement];text="Uniform\r\nfont casting";langid=6006;}
			if (help_font_element==1) {key=ToolboxFontFormating.MixedAccKey;text="Mixed\r\nfont casting";langid=6007;}
		}
		int i=0;
		while (ToolboxKeycodes[i].code)
		{
			if (ToolboxKeycodes[i].code==key) break;
			i++;
		}
		if (ToolboxKeycodes[i].code) 
		{
			if (key&0x200) strcat(buff,"Ctrl+Shft+"); else strcat(buff,"Ctrl+");
			strcat(buff,ToolboxKeycodes[i].name);
			if (key&0x400) {strcpy(buff,"\'");strcat(buff,ToolboxKeycodes[i].name);strcat(buff,"\'");}
		}
		this->ShowHelptext(text,command,buff,easycast,langid);

		}
	}
	
	CWnd::OnMouseMove(nFlags, point);
	}
	catch(...)
	{
	}
}

#pragma optimize("s",on)
void CToolbox::OnRButtonDown(UINT nFlags, CPoint point)
{
	try
	{
	if (m_IsMain)
	{

		if (InsertIntoToolbox())
		{

		}
		else if (ContextMenu->IsWindowVisible())
		{
			ContextMenu->m_ContextMenuSelection=-1;
			ContextMenu->m_ContextMenuMember=-1;
			ContextMenu->ShowWindow(SW_HIDE);
		}
		else if (point.y<ToolboxSize/2)
		{
			//clicked on toolbox header
			if (ContextMenu)
			{
				ContextMenu->m_ContextMenuSelection=-1;
				prevContextMenuSelection=-1;
				ContextMenu->AdjustPosition();
				ContextMenu->m_ContextMenuMember=-m_FontModeElement-1;
				if (m_FontModeElement==0)
					ContextMenu->m_ContextMenuSubmember=ToolboxFontFormating.SelectedUniform;
				if (m_FontModeElement==1)
					ContextMenu->m_ContextMenuSubmember=-1;
				NoImageAutogeneration=2;
				ContextMenu->ShowWindow(SW_SHOW);
				ContextMenu->InvalidateRect(NULL,0);
				ContextMenu->UpdateWindow();
			}
		}
		else if ((point.y<=(ToolboxNumMembers+1)/2*2*ToolboxSize/3+ToolboxSize/2))
		{
			//clicked on toolbox element
			if ((m_SelectedElement<ToolboxNumMembers) && (m_SelectedElement>=0))
			{
				if (ContextMenu)
				{
					ContextMenu->m_ContextMenuSelection=-1;
					prevContextMenuSelection=-1;
					ContextMenu->AdjustPosition();
					ContextMenu->m_ContextMenuMember=m_SelectedElement;
					ContextMenu->m_ContextMenuSubmember=ToolboxMembers[m_SelectedElement].SelectedSubmember;
					NoImageAutogeneration=2;
					ContextMenu->ShowWindow(SW_SHOW);
					ContextMenu->InvalidateRect(NULL,0);
					ContextMenu->UpdateWindow();
				}
			}

		}
		else if (m_SelectedColor>=0)
		{
			if ((m_SelectedColor==20) || (m_SelectedColor==21) || (m_SelectedColor==22))
			{
				//the "edit nodes" option, the "use cross" option, the "select last object" option
				//handling is the same as for left button click
				OnLButtonDown(nFlags,point);
			}
			else
			{
				//line color and width adjustment
				int LW=-100;
				int CL=-100;
				if (m_SelectedColor>=10)
				{
					int width=m_SelectedColor-10;
					if (width==0) LW=65*DRWZOOM/100;
					if (width==1) LW=DRWZOOM;
					if (width==2) LW=3*DRWZOOM/2;
					if (width==3) LW=2*DRWZOOM;
					if (width==4) LW=3*DRWZOOM;
					if (width==5) LW=4*DRWZOOM;
				}
				if (m_SelectedColor<10) CL=m_SelectedColor;
				PaintColorbox(this->GetDC());

				//we are changing the properties of the last created object
				int repaint=0;
				if (NumSelectedObjects)
				{
					//if something is selected then act the same as if we left-clicked
					OnLButtonDown(nFlags,point);
				}
				else if (NumDocumentElements>0)
				{
					tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
					if (ds->Object) 
					{
						if (ds->Type==2)
						{
							if (CL!=-100) if (m_SelectedColor<10) ((CDrawing*)(ds->Object))->SetColor(CL);
							if (LW!=-100) if (m_SelectedColor>=10)((CDrawing*)(ds->Object))->SetLineWidth(LW);
							int x,y,w,h;
							((CDrawing*)(ds->Object))->AdjustCoordinates(&x,&y,&w,&h);
							ds->absolute_X+=x;
							ds->absolute_Y+=y;
							repaint=1;
						}
						if (ds->Type==1)
						{
							if (CL!=-100) ((CExpression*)(ds->Object))->SetColor(CL);
						}
					}
				}
				if (repaint) pMainView->RepaintTheView(); else pMainView->InvalidateRect(NULL,0);
			}
		}
		else 
		{
			OnLButtonDown(nFlags,point);
			//clicked on a colorbox or text control box
			/*if (NumSelectedObjects)
			{
				for (int i=0;i<NumDocumentElements;i++)
					if (TheDocument[i].MovingDotState==3) TheDocument[i].MovingDotState=0;
				pMainView->InvalidateRect(NULL,0);
				pMainView->UpdateWindow();
				NumSelectedObjects=0;
				NumSelectedDrawings=0;
			}
			else if (IsDrawingMode)
			{
				IsDrawingMode=0;
				pMainView->SetMousePointer();
			}
			else if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
			{
				((CExpression*)KeyboardEntryObject)->KeyboardStop();
				if ((KeyboardEntryBaseObject) && (KeyboardEntryBaseObject->Type==1))
				{
					CExpression *expr=((CExpression*)(KeyboardEntryBaseObject->Object));
					if (((expr->m_NumElements==1) && (expr->m_pElementList->Type==0)) || (expr->m_NumElements==0))
					{
						pMainView->DeleteDocumentObject(KeyboardEntryBaseObject);
					}
				}
				KeyboardEntryObject=NULL;
				KeyboardEntryBaseObject=NULL;
				pMainView->InvalidateRect(NULL,0);
				pMainView->UpdateWindow();
				pMainView->SetMousePointer();
				this->AdjustPosition();
				ShowHelptext("","","","",0);
				//this->InvalidateRect(NULL,1);
				//this->UpdateWindow();
	
			}
			*/
		}
	} 
	else if (m_IsSubtoolbox>0)
	{
		if (InsertIntoToolbox())
		{

		}
		else if (Toolbox->ContextMenu->IsWindowVisible())
		{
			Toolbox->ContextMenu->m_ContextMenuSelection=-1;
			Toolbox->ContextMenu->m_ContextMenuMember=-1;
			Toolbox->ContextMenu->ShowWindow(SW_HIDE);
		}
		else if ((m_SelectedElement<ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers) && (m_SelectedElement>=0))
		{
			if (Toolbox->ContextMenu)
			{				
				Toolbox->ContextMenu->m_ContextMenuSelection=-1;
				prevContextMenuSelection=-1;
				Toolbox->ContextMenu->AdjustPosition();
				Toolbox->ContextMenu->m_ContextMenuMember=m_IsSubtoolbox-1;
				Toolbox->ContextMenu->m_ContextMenuSubmember=m_SelectedElement;
				NoImageAutogeneration=2;
				Toolbox->ContextMenu->ShowWindow(SW_SHOW);
				Toolbox->ContextMenu->InvalidateRect(NULL,0);
				Toolbox->ContextMenu->UpdateWindow();
			}
		}
	}
	else if (m_IsSubtoolbox==-2) //mixed font individual key
	{
		ToolboxChangeIndividualKeyFont();
	}
	else if (m_IsSubtoolbox==-1) //uniform font selection subtoolbox
	{
		if (ClipboardExpression)
		{
			delete ClipboardExpression;
			ClipboardExpression=NULL;
		}
		else if (ClipboardDrawing)
		{
			delete ClipboardDrawing;
			ClipboardDrawing=NULL;
		}
		else if ((m_SelectedElement<ToolboxFontFormating.NumFormats) && (m_SelectedElement>=0))
		{
			if (Toolbox->ContextMenu)
			{				
				Toolbox->ContextMenu->m_ContextMenuSelection=-1;
				prevContextMenuSelection=-1;
				Toolbox->ContextMenu->AdjustPosition();
				Toolbox->ContextMenu->m_ContextMenuMember=m_IsSubtoolbox;
				Toolbox->ContextMenu->m_ContextMenuSubmember=m_SelectedElement;
				NoImageAutogeneration=2;
				Toolbox->ContextMenu->ShowWindow(SW_SHOW);
				Toolbox->ContextMenu->InvalidateRect(NULL,0);
				Toolbox->ContextMenu->UpdateWindow();
			}
		}
	}
	else if (m_IsSubtoolbox==-4) //hand drawing popup menu
	{
		IsDrawingMode=0;
		ShowWindow(SW_HIDE);
		if (pMainView)
		{
			pMainView->InvalidateRect(NULL,0);
			pMainView->UpdateWindow();
			pMainView->SetMousePointer();
		}

	}
	else if (m_IsContextMenu)
	{
		m_ContextMenuSelection=-1;
		m_ContextMenuMember=-1;
		ShowWindow(SW_HIDE);
	}
	else if (m_IsKeyboard)
	{
		if (ClipboardExpression)
		{
			delete ClipboardExpression;
			ClipboardExpression=NULL;
		}
		else if (ClipboardDrawing)
		{
			delete ClipboardDrawing;
			ClipboardDrawing=NULL;
		}
		else 
		{
			MessageBeep(0);
		}
	}
	pMainView->SetMousePointer();

	CWnd::OnRButtonDown(nFlags, point);
	}
	catch(...)
	{
	}
}

UINT CToolbox::GetUppercaseFormatting(int key_code, UINT formatting)
{
	//the lowercase key_code and formatting translates to uppercase formatting

	if ((m_FontModeSelection==0) || (textmodeactivated))
		return formatting; //if uniform mode is active, then just return the same formatting as for lowercase 
	else
	{
		//if mixed mode is active, we must check the key_code 
		int i;
		for(i=0;i<ToolboxKeyboardElements.NumKeys;i++)
			if (ToolboxKeyboardElements.code[i]==toupper(key_code))
			{
				//we found the key code
				if ((formatting&0xE0)==0x60)
				{
					UINT rv=ToolboxKeyboardElements.FormatingBigGreek2[i];
					rv=rv<<16;
					rv|=(ToolboxKeyboardElements.FormatingBigGreek[i]&0x1F)|0x60;
					return rv;
				}
				else
				{
					UINT rv=ToolboxKeyboardElements.FormatingBigCaps2[i];
					rv=rv<<16;
					rv|=ToolboxKeyboardElements.FormatingBigCaps[i];
					return rv;
				}
			}

		UINT rv=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->m_VMods;
		rv=rv<<16;
		rv|=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0];
		return rv;
	}
}

#pragma optimize("",on)
int prevAltData=0;

UINT CToolbox::GetUniformFormatting(void)
{
	UINT rv=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_VMods;
	rv=rv<<16;
	rv|=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0];
	return rv;
}
int CToolbox::GetUniformFormattingColor(void)
{
	return ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_Color;
}

UINT CToolbox::GetMixedFormatting(char key,char is_greek)
{
	int is_caps=0;
	if (key==toupper(key)) is_caps=1;

	int i;
	for(i=0;i<ToolboxKeyboardElements.NumKeys;i++)
		if (ToolboxKeyboardElements.code[i]==toupper(key))
		{
			if (is_caps)
			{
				if (is_greek) 
				{
					UINT rv=ToolboxKeyboardElements.FormatingBigGreek2[i];
					rv=rv<<16;
					rv|=ToolboxKeyboardElements.FormatingBigGreek[i];
					return rv; 
				}
				else 
				{
					UINT rv=ToolboxKeyboardElements.FormatingBigCaps2[i];
					rv=rv<<16;
					rv|=ToolboxKeyboardElements.FormatingBigCaps[i];
					return rv; 
				}
			}
			else
			{
				if (is_greek) 
				{
					UINT rv=ToolboxKeyboardElements.FormatingSmallGreek2[i];
					rv=rv<<16;
					rv|=ToolboxKeyboardElements.FormatingSmallGreek[i];
					return rv; 
				}
				else 
				{
					UINT rv=ToolboxKeyboardElements.FormatingSmallCaps2[i];
					rv=rv<<16;
					rv|=ToolboxKeyboardElements.FormatingSmallCaps[i];
					return rv; 
				}
			}
		}
	UINT retval=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->m_VMods;
	retval=retval<<16;
	retval|=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0];
	if ((key=='?') || (key=='$') || (key=='%')) retval&=~0x02; //some characters are never cast italic
	return retval;
}

extern CExpression *AutocompleteSource;
extern CExpression *prevAutocompleteSource;
extern int AutocompleteTriggered;
extern int prevAutocompleteTriggered;
UINT CToolbox::KeyboardHit(UINT code, UINT Flags)
{
	//Flags: bit 0 - CTRL; bit1 - SHIFT; bit2 - CAPS; bit3 - ALT; 
	if (!Toolbox) return 1;

	if (code==27)  //esc button
	{
		if (Toolbox->Subtoolbox->IsWindowVisible()) Toolbox->Subtoolbox->ShowWindow(SW_HIDE);
		if (Toolbox->ContextMenu->IsWindowVisible()) Toolbox->ContextMenu->ShowWindow(SW_HIDE);
		if (Popup->IsWindowVisible()) Popup->HidePopupMenu();
	}

	CExpression *ee=(CExpression*)KeyboardEntryObject;
	if ((ee) && (ee->m_IsKeyboardEntry>0) && (ee->m_IsKeyboardEntry<=ee->m_NumElements) && 
		((ee->m_pElementList+ee->m_IsKeyboardEntry-1)->pElementObject) &&
		((ee->m_pElementList+ee->m_IsKeyboardEntry-1)->pElementObject->m_Text))
		//(((ee->DetermineInsertionPointType(ee->m_IsKeyboardEntry-1)))
	{
		//the text mode is active at cursor position
		if (textmodeactivated==0) {textmodeactivated=1;CDC *DC=this->GetDC();PaintToolboxHeader(DC);this->ReleaseDC(DC);}
	}
	else
	{
		if (textmodeactivated) {textmodeactivated=0;CDC *DC=this->GetDC();PaintToolboxHeader(DC);this->ReleaseDC(DC);}
	}


	if (code==6) {return 1;}

	CDC *DC=this->GetDC();

	if ((code) && (this->m_IsMain))
	{
		if (AlternateVirtualKeyboard)
		{
			AlternateVirtualKeyboard=0;
			this->Keyboard->InvalidateRect(NULL,0);
			this->Keyboard->UpdateWindow();
		}
	}

	m_KeyboardSmallCaps=1;
	if (Flags&0x02) m_KeyboardSmallCaps=0;  //the SHIFT key
	if (Flags&0x04) m_KeyboardSmallCaps=!m_KeyboardSmallCaps;  //the CAPS LOCK key
	
	if (AlternateVirtualKeyboard==3) Flags|=0x10;

	if ((Flags&0x10) && (!(Flags&0x01))) // we should revert to greek keyboard
	{
		if (!greeksymbolsactivated)
		{
			if ((m_FontModeSelection==1) && (!textmodeactivated))
			{
				greeksymbolsactivated=2;
				prevAltData=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0];
				ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0]=(prevAltData&0x1F)|0x60;
			}
			else
			{
				greeksymbolsactivated=1;
				prevAltData=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0];
				ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0]=(prevAltData&0x1F)|0x60;
			}
			//PaintToolboxHeader(DC);
		}
	}
	else if (greeksymbolsactivated)
	{
		if (greeksymbolsactivated==2)
			ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0]=prevAltData;
		else
			ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0]=prevAltData;
		greeksymbolsactivated=0;
		if (m_FontModeSelection<0) m_FontModeSelection=1;
		if (m_FontModeSelection>2) m_FontModeSelection=1;
		PaintToolboxHeader(DC);
	}
	
	this->ReleaseDC(DC);

	AdjustKeyboardFont();

	if (!ViewOnlyMode)
	if ((Flags&0x01) && (!(Flags&0x08))) //the CTRL key was down - the accelertor table will be examined
	{
		if (Flags&0x02) code+=0x200; //also the SHIFT key was down 
		int ii,jj;
		for (ii=0;ii<ToolboxNumMembers;ii++)
			for (jj=0;jj<ToolboxMembers[ii].NumSubmembers;jj++)
				if ((ToolboxMembers[ii].AcceleratorKey[jj]==code) && (code))
				{
					if (ClipboardExpression)
					{
						delete ClipboardExpression;
						ClipboardExpression=NULL;
					}
					if (ClipboardDrawing)
					{

						delete ClipboardDrawing;
						ClipboardDrawing=NULL;
					}
					//found accelerator key in the table
					//we are copying the member into clipboard
					if (ToolboxMembers[ii].Above[jj]==-1)
					{
						if (ToolboxMembers[ii].userdef_mask&(1<<jj))
						{
							ClipboardDrawing=new CDrawing();
							ClipboardDrawing->CopyDrawing((CDrawing*)ToolboxMembers[ii].Submembers[jj]);
							ClipboardDrawing->FindBottomRightDrawingPoint(&MovingStartX,&MovingStartY);
							MovingStartX/=DRWZOOM;
							MovingStartY/=DRWZOOM;
							pMainView->RepaintTheView();

						}
						else
						{
							IsDrawingMode=((CDrawing*)ToolboxMembers[ii].Submembers[jj])->OriginalForm;
							//if (ToolbarEditNodes) {ToolbarEditNodes=0;if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();}

							pMainView->SetMousePointer();
						}
					}
					else
					{
						IsDrawingMode=0;
						ClipboardExpression=new CExpression(NULL,NULL,100);
						ClipboardExpression->CopyExpression(ToolboxMembers[ii].Submembers[jj],0,1,0);
						short l,a,b;
						ClipboardExpression->CalculateSize(this->GetDC(),100,&l,&a,&b);
						
						if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
						{
							CDC *DC=pMainView->GetDC();
							((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(DC,ViewZoom,6,0,0,0,0);
							((CExpression*)KeyboardEntryObject)->Autocomplete(0);
							if (AutocompleteSource) {prevAutocompleteSource=AutocompleteSource;prevAutocompleteTriggered=AutocompleteTriggered;pMainView->InvalidateRect(NULL,0);pMainView->UpdateWindow();}
							pMainView->ReleaseDC(DC);
						}
						else
						{
							pMainView->SetMousePointer();
							POINT cursor;
							GetCursorPos(&cursor);
							pMainView->ScreenToClient(&cursor);
							pMainView->PaintClipboard(cursor.x,cursor.y);
						}
					}
					break;
				}
		if ((ToolboxFontFormating.MixedAccKey==code) && (code))
		{
			m_prevFontModeSelection|=0x80;
			m_FontModeSelection=1;
			if ((m_FontModeSelection==1) && (textmodeactivated) && (KeyboardEntryObject))
			{
				//we are going to select the math typing mode becase user selected the mixed formatting (the mixed formatting is only possible in math typing mode)
				CDC *ddc=pMainView->GetDC();
				{int tmp=IsALTDown;IsALTDown=1;((CExpression*)KeyboardEntryObject)->KeyboardKeyHit(ddc,ViewZoom,' ',0,0,0,1);IsALTDown=tmp;}
				pMainView->ReleaseDC(ddc);
			}
			Toolbox->PaintToolboxHeader(Toolbox->GetDC());
			AdjustKeyboardFont();
			this->ReformatKeyboardSelection();
		}
		for (ii=0;ii<ToolboxFontFormating.NumFormats;ii++)
		{
			if ((ToolboxFontFormating.UniformAccKey[ii]==code) && (code))
			{
				m_FontModeSelection=0;
				ToolboxFontFormating.SelectedUniform=ii;
				if ((ToolboxFontFormating.UniformFormats2[ToolboxFontFormating.SelectedUniform]&0x01)==0) //if it is not a singleshot mode
				{
					m_prevFontModeSelection=ToolboxFontFormating.SelectedUniform;
				}
				AdjustKeyboardFont();
				this->ReformatKeyboardSelection();
				Toolbox->PaintToolboxHeader(Toolbox->GetDC());
			}
		}
	}
	else if (code>0)
	{
		//some key was hit, and we have to set return value

		int retval=0;

		if ((m_FontModeSelection==0) || (textmodeactivated))
		{
			//permanent keyboard mode or text entry box
			retval=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_VMods;
			retval=retval<<16;
			retval|=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0];

			if (ToolboxFontFormating.UniformFormats2[ToolboxFontFormating.SelectedUniform]&0x01) //this is a singleshot formating
			{
				if (greeksymbolsactivated) retval=(retval&0xFFFFFF1F)|0x60;
				if (m_prevFontModeSelection&0x80) 
					m_FontModeSelection=1;
				else
					m_FontModeSelection=0;
				
				ToolboxFontFormating.SelectedUniform=(m_prevFontModeSelection&0x7F);
				if (ToolboxFontFormating.SelectedUniform<0) ToolboxFontFormating.SelectedUniform=0;
				if (ToolboxFontFormating.SelectedUniform>ToolboxFontFormating.NumFormats-1) ToolboxFontFormating.SelectedUniform=0;
				
				
				if (m_FontModeSelection<0) m_FontModeSelection=0;
				if (m_FontModeSelection>1) m_FontModeSelection=0;
				AdjustKeyboardFont();
				Toolbox->PaintToolboxHeader(Toolbox->GetDC());
			}
		}
		else if (m_FontModeSelection==1)  //mixed keyboard mode
		{
			
			retval=GetMixedFormatting(code,greeksymbolsactivated);
			if (greeksymbolsactivated) {retval&=0xFFFFFF1F;retval|=0x60;}
/*
			int i;
			for(i=0;i<ToolboxKeyboardElements.NumKeys;i++)
				if (ToolboxKeyboardElements.code[i]==toupper(code))
				{
					//we found the key code
					retval=ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data2[0];
					break;
				}
			if ((i==ToolboxKeyboardElements.NumKeys))//not found key code - we will use the basic level mode
			{
				retval=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0];
				if ((code=='?') || (code=='$') || (code=='%')) retval&=~0x02; //never return italic for some symbols
			}
			*/
		}
		
		return retval;
	}

	return 0;
}

char CToolbox::IsUniformFormatting()
{
	if ((m_FontModeSelection==0) || (textmodeactivated)) return 1; else return 0;
}

void CToolbox::ReformatKeyboardSelection()
{
	int fcolor=-1;
	AlternateVirtualKeyboard=0;
	if ((KeyboardEntryObject) && (KeyboardEntryBaseObject))
	{
		char format;
		if (Toolbox->m_FontModeSelection==0) //permanent font mode
		{
			format=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0];
			fcolor=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_Color;
		}
		else if (Toolbox->m_FontModeSelection==1) //mixed font mode
		{
			format=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0];
		}
		else return;

		CExpression *e=(CExpression*)KeyboardEntryObject;
		int any=0;
		for (int kk=0;kk<e->m_NumElements;kk++)
		{
			tElementStruct *ts=e->m_pElementList+kk;
			if ((ts->pElementObject) && (ts->IsSelected==2)) ts->pElementObject->m_Color=fcolor;
			if ((ts->Type==1) && (ts->pElementObject) && (ts->IsSelected==2) && (ts->pElementObject->Data1[0]))
			{
				if (!any) ((CMainFrame*)theApp.m_pMainWnd)->UndoSave("font formatting",20218);
				for (int jj=0;jj<24;jj++)
					ts->pElementObject->Data2[jj]=format;
				ts->IsSelected=0;
				any=1;
			}
		}
		if ((any) && (Toolbox->m_FontModeSelection==0)) 
		{
			//just to revert oneshot formating back to previous state
			if (m_prevFontModeSelection&0x80) 
				m_FontModeSelection=1;
			else
				m_FontModeSelection=0;

			ToolboxFontFormating.SelectedUniform=(m_prevFontModeSelection&0x7F);
			if (ToolboxFontFormating.SelectedUniform<0) ToolboxFontFormating.SelectedUniform=0;
			if (ToolboxFontFormating.SelectedUniform>ToolboxFontFormating.NumFormats-1) ToolboxFontFormating.SelectedUniform=0;
			
			CDC *DC=Toolbox->GetDC();
			Toolbox->PaintToolboxHeader(DC);
			Toolbox->ReleaseDC(DC);
		}
	}
}

int CToolbox::GetFormattingColor(void)
{
	if ((m_FontModeSelection==1) && (textmodeactivated==0)) return -1;
	return ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_Color;
}

//defines acceleration code (first checks if it already exists)
int CToolbox::DefineAcceleratorCode(short code)
{
	int ii,jj;

	if (code==1) return 0; // the EXIT code
	if (code==2) //FONT popup menu
	{
		if (m_ContextMenuMember<0) //font definitions
		{
			CWnd *Owner=Toolbox;
			if (Toolbox->Subtoolbox->IsWindowVisible()) Owner=Toolbox->Subtoolbox;

			if (m_ContextMenuMember==-1)
			{
				
				ToolboxFontFormating.UniformFormats[m_ContextMenuSubmember]->SelectExpression(1);
				ToolboxFontFormating.UniformFormats[m_ContextMenuSubmember]->m_Selection=0;
				ToolboxDisableDeselect=1;
				FontAdditionalData=ToolboxFontFormating.UniformFormats2[m_ContextMenuSubmember];
				Popup->ShowPopupMenu(ToolboxFontFormating.UniformFormats[m_ContextMenuSubmember],Owner,0,m_ContextMenuSubmember);
				//the PopupCloses function will be called after popupmenu closes
			}
			if (m_ContextMenuMember==-2)
			{
				ToolboxFontFormating.MixedFormat->SelectExpression(1);
				ToolboxFontFormating.MixedFormat->m_Selection=0;
				FontAdditionalData=0;
				Popup->ShowPopupMenu(ToolboxFontFormating.MixedFormat,Owner,0,22);
				//the PopupCloses function will be called after popupmenu closes
			}
		}
		else
		{
			if ((m_ContextMenuMember>=0) && (ToolboxMembers[m_ContextMenuMember].Above[m_ContextMenuSubmember]!=-1))
			{
				//opening easycasts
				TheKeycodeString=&ToolboxMembers[m_ContextMenuMember].Keycodes[m_ContextMenuSubmember][0];
				EasycastListStart=0;
				FontAdditionalData=0;
				Popup->ShowPopupMenu(NULL,this,7,24);
			}
		}
		return 0;
	}
	if (code==3)
	{
		if ((m_ContextMenuMember>=0) && (ToolboxMembers[m_ContextMenuMember].userdef_mask&(1<<m_ContextMenuSubmember)))
		{
			//deleting m_ContextMenuMember i m_ContextMenuSubmember
			for (int i=m_ContextMenuSubmember;i<ToolboxMembers[m_ContextMenuMember].NumSubmembers-1;i++)
			{
				ToolboxMembers[m_ContextMenuMember].Submembers[i]=ToolboxMembers[m_ContextMenuMember].Submembers[i+1];
				ToolboxMembers[m_ContextMenuMember].Above[i]=ToolboxMembers[m_ContextMenuMember].Above[i+1];
				ToolboxMembers[m_ContextMenuMember].Below[i]=ToolboxMembers[m_ContextMenuMember].Below[i+1];
				ToolboxMembers[m_ContextMenuMember].Length[i]=ToolboxMembers[m_ContextMenuMember].Length[i+1];
				ToolboxMembers[m_ContextMenuMember].prevToolboxSize[i]=ToolboxMembers[m_ContextMenuMember].prevToolboxSize[i+1];
				ToolboxMembers[m_ContextMenuMember].zoom[i]=ToolboxMembers[m_ContextMenuMember].zoom[i+1];
				memcpy(&ToolboxMembers[m_ContextMenuMember].Keycodes[i][0],&ToolboxMembers[m_ContextMenuMember].Keycodes[i+1][0],9);
			}
			unsigned int tmp=ToolboxMembers[m_ContextMenuMember].userdef_mask;
			unsigned int tmp2=(1<<(m_ContextMenuSubmember))-1;
			tmp=tmp>>1;
			tmp=tmp&(~tmp2);
			ToolboxMembers[m_ContextMenuMember].userdef_mask&=tmp2;
			ToolboxMembers[m_ContextMenuMember].userdef_mask|=tmp;

			ToolboxMembers[m_ContextMenuMember].NumSubmembers--;
			if (ToolboxMembers[m_ContextMenuMember].SelectedSubmember>=ToolboxMembers[m_ContextMenuMember].NumSubmembers) 
				ToolboxMembers[m_ContextMenuMember].SelectedSubmember--;
			if (ToolboxMembers[m_ContextMenuMember].NumSubmembers==0)
			{
				for (int i=m_ContextMenuMember;i<ToolboxNumMembers-1;i++)
				{
					ToolboxMembers[i]=ToolboxMembers[i+1];
				}
				ToolboxNumMembers--;
			}
			Toolbox->AdjustPosition();
			Toolbox->InvalidateRect(NULL,0);
			Toolbox->UpdateWindow();
		}
		return 0; //separator
	}

	for (ii=0;ii<ToolboxNumMembers;ii++)
		for (jj=0;jj<ToolboxMembers[ii].NumSubmembers;jj++)
			if (ToolboxMembers[ii].AcceleratorKey[jj]==code) 
			{
				ToolboxMembers[ii].AcceleratorKey[jj]=0; 
				if ((ii==m_ContextMenuMember) && (jj==m_ContextMenuSubmember)) goto define_code_exit;
			}
	
	for (ii=0;ii<ToolboxFontFormating.NumFormats;ii++)
	{
		if (ToolboxFontFormating.UniformAccKey[ii]==code)
		{
			ToolboxFontFormating.UniformAccKey[ii]=0;
			if ((m_ContextMenuMember==-2) && (ii==m_ContextMenuSubmember)) goto define_code_exit;
		}
	}
	if (ToolboxFontFormating.MixedAccKey==code)
	{
		ToolboxFontFormating.MixedAccKey=0;
		if (m_ContextMenuMember==-3) goto define_code_exit;
	}

	if ((m_ContextMenuMember>=0) && (m_ContextMenuMember<ToolboxNumMembers) &&
		(m_ContextMenuSubmember>=0) && (m_ContextMenuSubmember<ToolboxMembers[m_ContextMenuMember].NumSubmembers))
	{
		ToolboxMembers[m_ContextMenuMember].AcceleratorKey[m_ContextMenuSubmember]=code;
	}
	if ((m_ContextMenuMember==-1) && (m_ContextMenuSubmember>=0) &&
		(m_ContextMenuSubmember<16))
		ToolboxFontFormating.UniformAccKey[m_ContextMenuSubmember]=code;
	if (m_ContextMenuMember==-2)
		ToolboxFontFormating.MixedAccKey=code;

define_code_exit:
	InvalidateRect(NULL,0);
	UpdateWindow();
	Sleep(200);
	return 1;
}
int prevSmallCaps=-1;
int CToolbox::AdjustKeyboardFont(void)
{
	char data=0;
	char vmods=0;
	char color=-1;

	if (this!=Toolbox) 
	{
		if (Toolbox) 
			Toolbox->AdjustKeyboardFont();
	}
	else
	{ 
		int i;
		int any_change=0;
		for (i=0;i<ToolboxKeyboardElements.NumKeys;i++)
		{
			if ((m_FontModeSelection==0) || (textmodeactivated)) //permanent font mode
			{
				data=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->Data2[0];
				vmods=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_VMods;
				color=ToolboxFontFormating.UniformFormats[ToolboxFontFormating.SelectedUniform]->m_pElementList->pElementObject->m_Color;
			}
			else if (m_FontModeSelection==1) //mixed font mode
			{
				if (greeksymbolsactivated)
				{
					if (m_KeyboardSmallCaps)
					{
						data=ToolboxKeyboardElements.FormatingSmallGreek[i];
						vmods=ToolboxKeyboardElements.FormatingSmallGreek2[i];
					}
					else
					{
						data=ToolboxKeyboardElements.FormatingBigGreek[i];
						vmods=ToolboxKeyboardElements.FormatingBigGreek2[i];
					}
					data=(data&0x1F)|0x60;
				}
				else
				{
					if (m_KeyboardSmallCaps)
					{
						data=ToolboxKeyboardElements.FormatingSmallCaps[i];
						vmods=ToolboxKeyboardElements.FormatingSmallCaps2[i];
					}
					else
					{
						data=ToolboxKeyboardElements.FormatingBigCaps[i];
						vmods=ToolboxKeyboardElements.FormatingBigCaps2[i];
					}
				}
				color=-1;
			}

			if (data!=ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data2[0])
			{
				ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data2[0]=data;
				any_change=1;
			}
			if (vmods!=ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->m_VMods)
			{
				ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->m_VMods=vmods;
				any_change=1;
			}
			if (color!=ToolboxKeyboardElements.Key[i]->m_Color)
			{
				ToolboxKeyboardElements.Key[i]->m_Color=color;
				any_change=1;
			}
			if (m_KeyboardSmallCaps)
				ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data1[0]=
				tolower(ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data1[0]);
			else
				ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data1[0]=
				toupper(ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data1[0]);

			if (prevSmallCaps!=m_KeyboardSmallCaps)
			{
				any_change=1;
				prevSmallCaps=m_KeyboardSmallCaps;
			}
		}

		if (any_change)
		{
			Keyboard->InvalidateRect(NULL,0);
			Keyboard->UpdateWindow();
		}
		
	}
	return 0;
}

void CToolbox::PaintKeyboardElement(CDC* dc, int element)
{
	if (element==255)
	{
		RECT ClientRect;
		GetClientRect(&ClientRect);

		dc->SelectObject(GetFontFromPool(3,0,0,13*ToolboxSize/64+1));
		
		dc->SetBkMode(TRANSPARENT);
		dc->SetTextAlign(TA_BOTTOM|TA_RIGHT);
		int aa=20*ToolboxSize/64;
		int bb=15*ToolboxSize/64;
		if (AlternateVirtualKeyboard==3)  //Greek letter set latched (selected)
		{
			dc->FillSolidRect(ClientRect.right-aa,ClientRect.bottom-bb,aa-1,bb-1,SHADOW_BLUE_COLOR);
			dc->SetTextColor(RGB(0,0,255));
			dc->TextOut(ClientRect.right-1,ClientRect.bottom-2,"abg",3);
		}
		else if (AlternateVirtualKeyboard==2) //Greek letter set button pointed at
		{
			dc->SetTextColor(RGB(0,0,255));
			dc->TextOut(ClientRect.right-1,ClientRect.bottom-2,"abg",3);
		}
		else if (AlternateVirtualKeyboard==1) //Greek letter set button drawn
		{
			dc->SetTextColor(RGB(128,128,255));
			dc->TextOut(ClientRect.right-1,ClientRect.bottom-2,"abg",3);
		}
		else //Greek letter set button not drawn (currsor outside virtual keyboard)
			dc->FillSolidRect(ClientRect.right-aa,ClientRect.bottom-bb,aa-1,bb-1,SHADOW_BLUE_COLOR2);
		return;
	}
	if (element>=ToolboxKeyboardElements.NumKeys) return;

	int i=element;
	int Cx,Cy;

	short l,a,b;
	Cx=ToolboxKeyboardElements.X[i]*ToolboxSize/30+ToolboxSize/6+3;
	Cy=ToolboxKeyboardElements.Y[i]*ToolboxSize/30+ToolboxSize/6+3;
	int Lx=ToolboxSize/6;
	int Ly=ToolboxSize/6;
	ToolboxKeyboardElements.Key[i]->CalculateSize(dc,80*ToolboxSize/64+1,&l,&a,&b,0,1);
	CBrush brush((m_KeyboardElement==element)?SHADOW_BLUE_COLOR2:SHADOW_BLUE_COLOR);
	dc->SelectObject(brush);
	dc->SelectObject(GetStockObject(WHITE_PEN));
	dc->Rectangle(Cx-Lx,Cy-Ly,Cx+Lx+1,Cy+Ly+1);
	ToolboxKeyboardElements.Key[i]->PaintExpression(dc,80*ToolboxSize/64+1,Cx-l/2,Cy+1);
}

#pragma optimize("s",on)
int CToolbox::PopupCloses(int UserParameter, int ExitCode)
{
	int i;
	ToolboxDisableDeselect=0;

	if ((UserParameter==22) && (ExitCode!=0))  //UserParametr 22 - individual key settings, ExitCode=0 - no change
	{
		for (i=0;i<ToolboxKeyboardElements.NumKeys;i++)
		{
			char ccc=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0];
			unsigned char vmods=ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->m_VMods;
			ToolboxKeyboardElements.FormatingBigCaps[i]=ccc;
			ToolboxKeyboardElements.FormatingSmallCaps[i]=ccc;
			ToolboxKeyboardElements.FormatingBigGreek[i]=ccc;
			ToolboxKeyboardElements.FormatingSmallGreek[i]=ccc;
			ToolboxKeyboardElements.FormatingBigCaps2[i]=vmods;
			ToolboxKeyboardElements.FormatingSmallCaps2[i]=vmods;
			ToolboxKeyboardElements.FormatingBigGreek2[i]=vmods;
			ToolboxKeyboardElements.FormatingSmallGreek2[i]=vmods;
			ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->Data2[0]=ccc;
			ToolboxKeyboardElements.Key[i]->m_pElementList->pElementObject->m_VMods=vmods;
			ToolboxKeyboardElements.Key[i]->DeselectExpression();
		}
		ToolboxFontFormating.MixedFormat->DeselectExpression();
		Toolbox->Keyboard->InvalidateRect(NULL,0);
		Toolbox->Keyboard->UpdateWindow();
	}
	if ((UserParameter==23) && (ExitCode!=0))
	{
		int row=Toolbox->Subtoolbox->m_SelectedElement/100;
		int key=Toolbox->Subtoolbox->m_SelectedElement%100;
		if ((key<ToolboxKeyboardElements.NumKeys) && (ToolboxCharacter))
		{
			//in case the individual key setting was changed, store it.
			char ccc=ToolboxCharacter->m_pElementList->pElementObject->Data2[0];
			unsigned char vmods=ToolboxCharacter->m_pElementList->pElementObject->m_VMods;
			if (row==0) ToolboxKeyboardElements.FormatingBigCaps[key]=ccc;
			if (row==1) ToolboxKeyboardElements.FormatingSmallCaps[key]=ccc;
			if (row==2 )ToolboxKeyboardElements.FormatingBigGreek[key]=ccc;
			if (row==3) ToolboxKeyboardElements.FormatingSmallGreek[key]=ccc;
			if (row==0) ToolboxKeyboardElements.FormatingBigCaps2[key]=vmods;
			if (row==1) ToolboxKeyboardElements.FormatingSmallCaps2[key]=vmods;
			if (row==2 )ToolboxKeyboardElements.FormatingBigGreek2[key]=vmods;
			if (row==3) ToolboxKeyboardElements.FormatingSmallGreek2[key]=vmods;	
		}
	}
	for (i=0;i<ToolboxFontFormating.NumFormats;i++)
	{
		ToolboxFontFormating.UniformFormats[i]->DeselectExpression();
	}
	if ((UserParameter<12) && (FontAdditionalData&0x80) &&(ExitCode!=0)) //uniform formatting some option changed
	{
		ToolboxFontFormating.UniformFormats2[UserParameter]=(FontAdditionalData&0x01)|0x080;
		FontAdditionalData=0;
	}
	Toolbox->AdjustKeyboardFont();
	Toolbox->PaintToolboxHeader(Toolbox->GetDC());
	if (Toolbox->Subtoolbox->IsWindowVisible()) {Toolbox->Subtoolbox->InvalidateRect(NULL,0);Toolbox->Subtoolbox->UpdateWindow();}

	return 0;
}

#pragma optimize("s",on)
void CToolbox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_IsSubtoolbox==-4)
	{
		int member=-1;
		if (nChar=='1') member=0;
		if (nChar=='2') member=1;
		if (nChar=='3') member=2;
		if (nChar=='4') member=3;
		if (nChar=='5') member=4;
		if (nChar=='6') member=5;
		if (nChar=='7') member=6;
		if (nChar=='8') member=7;
		if (nChar=='9') member=8;
		if (nChar==0x0D) 
		{
			m_IsSubtoolbox=0;
			ShowWindow(SW_HIDE);
		}
		int m=-1,s=0;
		if (member>=0) {m=SpecialDrawingToolbox[member].member;s=SpecialDrawingToolbox[member].submember;}
		if (m>=0)
		{
			this->PickUpElementFromToolbox(m,s);
			return;
		}

	}

	if (((Toolbox->Keyboard->IsWindowVisible()) && (Toolbox->Keyboard->GetActiveWindow())) ||
		((Toolbox->Subtoolbox->IsWindowVisible()) && (Toolbox->Subtoolbox->GetActiveWindow())))
	{
		if (Toolbox->Subtoolbox->IsWindowVisible()) {Toolbox->m_SelectedElement=-1;Toolbox->Subtoolbox->ShowWindow(SW_HIDE);}
		theApp.m_pMainWnd->SetActiveWindow();
		pMainView->OnChar(nChar,nRepCnt,nFlags);
		//pMainView->SetActiveWindow();
	}
	if (nChar==27)
	{
		int clear_clp=1;
		if (Toolbox->Subtoolbox->IsWindowVisible()) {Toolbox->Subtoolbox->ShowWindow(SW_HIDE);clear_clp=0;}
		if (Toolbox->ContextMenu->IsWindowVisible()) {Toolbox->ContextMenu->ShowWindow(SW_HIDE);clear_clp=0;}
		if (Popup->IsWindowVisible()) {Popup->HidePopupMenu();clear_clp=0;}
		if (clear_clp)
		{
			if (ClipboardExpression)
			{
				delete ClipboardExpression;
				ClipboardExpression=NULL;
				//SetCursor(::LoadCursor(NULL,IDC_ARROW));
				pMainView->SetMousePointer();
			}
		}
	}
	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CToolbox::OnClose()
{

	if (m_IsKeyboard) {NoImageAutogeneration=1;ShowWindow(SW_HIDE);return;}
	CWnd::OnClose();
}


extern char FontFacenames[4][32];
extern char FontAdjustedSizes[4];
extern unsigned char FontCharSet[4];
extern unsigned int FontWeight[4];
#pragma optimize("s",on)
int CToolbox::SaveSettings(char * filename)
{
	//saves toolbox setings and menu options

	char DefaultFilename[512];
	if (filename==NULL) 
	{
		int j;		
		strcpy(DefaultFilename,GetCommandLine());	
		DefaultFilename[511]=0;

		int find_quot=0;
		if (DefaultFilename[0]=='"') {j=1;find_quot=1;} else j=0;
		while (DefaultFilename[j])
		{
			if (DefaultFilename[j]=='"') break;
			if (!find_quot) if (DefaultFilename[j]==' ') break;
			if (j>511) break;
			j++;
		}
		j--;
		for (;j>=0;j--)
		{
			if (DefaultFilename[j]=='\\') break;
			if (DefaultFilename[j]=='"') break;
			if (j==0) break;
		}
		DefaultFilename[j]=0;
		DefaultFilename[j+1]=0;
		if (j>find_quot) strcat(DefaultFilename+find_quot,"\\");
		strcat(DefaultFilename+find_quot,"Mathomir.set");
		filename=DefaultFilename+find_quot;
	}
	FILE *fil;
	fil=fopen(filename,"w+b");
	if (fil)
	{
		char dummy[32*9];
		memset(dummy,0,32*9);

		int i;
		for (i=0;i<24;i++)
		{
			fwrite(&(ToolboxMembers[i].AcceleratorKey[0]),sizeof(short),32,fil);
			fwrite(&(ToolboxMembers[i].SelectedSubmember),sizeof(short),1,fil);
		}
		for (i=0;i<6;i++) //for compatibility with earilier versions
		{
			fwrite(dummy,sizeof(short),32,fil);
			fwrite(dummy,sizeof(short),1,fil);
		}
		fwrite(dummy,sizeof(short),32,fil);

		fwrite(&(ToolboxFontFormating.UniformAccKey[0]),sizeof(short),32,fil);
		fwrite(&(ToolboxFontFormating.MixedAccKey),sizeof(short),1,fil);
		fwrite(&(ToolboxFontFormating.SelectedUniform),sizeof(short),1,fil);
		fwrite(dummy,sizeof(short),1,fil);
		for (i=0;i<32;i++)
		{
			if (i<ToolboxFontFormating.NumFormats)
			{
			   char ch;
			   ch=ToolboxFontFormating.UniformFormats2[i]&0x01; //the singleshot settings
			   int clr=ToolboxFontFormating.UniformFormats[i]->m_pElementList->pElementObject->m_Color;
			   clr+=1;
			   clr=clr&0x07;
			   clr*=2;
			   ch|=clr; //lowest bit = singleshoot settings; next three bits = color settings
			   ch|=0x80; //highest bit always set
			   fwrite(&ch,1,1,fil);
			   ch=ToolboxFontFormating.UniformFormats[i]->m_pElementList->pElementObject->Data2[0];
			   fwrite(&ch,1,1,fil);
			}
			else
			{
				char ch=0;
			    fwrite(&ch,1,1,fil);
			   	fwrite(&ch,1,1,fil);
			}
		}
		fwrite(&(ToolboxKeyboardElements.FormatingBigCaps[0]),sizeof(char),48,fil);
		fwrite(&(ToolboxKeyboardElements.FormatingSmallCaps[0]),sizeof(char),48,fil);		
		fwrite(&CenterParentheses_not_used,sizeof(int),1,fil);
		fwrite(&DefaultParentheseType,sizeof(int),1,fil);
		fwrite(&FrameSelections,sizeof(int),1,fil);
		fwrite(&FixFontForNumbers,sizeof(int),1,fil);
		fwrite(&UseALTForExponents,sizeof(int),1,fil);
		fwrite(&PaperHeight,sizeof(int),1,fil);
		fwrite(&PaperWidth,sizeof(int),1,fil);
		fwrite(&ToolboxSize,sizeof(int),1,fil);
		theApp.m_pMainWnd->GetWindowRect(&MainWindowRect);
		fwrite(&MainWindowRect,sizeof(RECT),1,fil);
		fwrite(&ImageSize,sizeof(int),1,fil);
		fwrite(&ForceHighQualityImage,sizeof(int),1,fil);
		fwrite(&ForceHalftoneImage,sizeof(int),1,fil);
		fwrite(&DefaultFontSize,sizeof(int),1,fil);
		fwrite(&MovingDotSize,sizeof(int),1,fil);
		fwrite(&MovingDotPermanent,sizeof(int),1,fil);
		fwrite(&GRID,sizeof(int),1,fil);
		fwrite(&IsShowGrid,sizeof(int),1,fil); //for compatibility (it was 'SnapToGrid' before)
		fwrite(&IsShowGrid,sizeof(int),1,fil);
		fwrite(&IsSimpleVariableMode,sizeof(int),1,fil);
		fwrite(&IsMathDisabled,sizeof(int),1,fil);
		fwrite(&ImaginaryUnit,sizeof(char),1,fil);
		fwrite(&ShadowSelection,sizeof(int),1,fil);
		fwrite(&AutosaveOption,sizeof(int),1,fil);
		fwrite(&MoveCursorOnWheel,sizeof(int),1,fil);
		fwrite(&EnableMenuShortcuts,sizeof(int),1,fil);

		fwrite(&FontFacenames[0][0],4*32,1,fil);
		fwrite(&FontAdjustedSizes[0],4,1,fil);
		fwrite(&FontCharSet[0],4,1,fil);
		fwrite(&FontWeight[0],4*sizeof(int),1,fil);

		fwrite(&F1SetsZoom,sizeof(int),1,fil);
		fwrite(&PrintTextAsImage,sizeof(int),1,fil);
		fwrite(&UseCommaAsDecimal,sizeof(int),1,fil);
		fwrite(dummy,sizeof(int),1,fil);
		fwrite(&SnapToGuidlines,sizeof(int),1,fil);
		fwrite(&UseCTRLForZoom,sizeof(int),1,fil);
		
		//store mixed font formating
		fwrite(ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2,sizeof(char),1,fil);
		fwrite(&m_FontModeSelection,sizeof(int),1,fil);
		
		fwrite(&(ToolboxKeyboardElements.FormatingBigGreek[0]),sizeof(char),32,fil);
		fwrite(&(ToolboxKeyboardElements.FormatingSmallGreek[0]),sizeof(char),32,fil);	

		//here we are writting user-defined items from the toolbox
		for (int i=0;i<ToolboxNumMembers;i++)
			for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
			{
				if (ToolboxMembers[i].userdef_mask&(1<<j))
				{
					//we found a user-defined toolbox item
					int len;
					if (ToolboxMembers[i].Above[j]!=-1)
						len=ToolboxMembers[i].Submembers[j]->XML_output(NULL,0,1);
					else
						len=((CDrawing*)ToolboxMembers[i].Submembers[j])->XML_output(NULL,0,1);
					char *data=(char*)malloc(len+256);

					if (ToolboxMembers[i].Above[j]!=-1)
						ToolboxMembers[i].Submembers[j]->XML_output(data,0,0);
					else
						((CDrawing*)ToolboxMembers[i].Submembers[j])->XML_output(data,0,0);
					
					int pos_data=i; 
					if (ToolboxMembers[i].userdef_mask&0x01) //the completely user defined submenu
						pos_data=(j==0)?256:257;
					if (ToolboxMembers[i].Above[j]==-1) pos_data+=1024;
					fwrite(&pos_data,sizeof(int),1,fil);
					fwrite(&len,sizeof(int),1,fil);
					fwrite(data,len,1,fil);
					free(data);
				}
			}
		{
			int pos_data=-1;
			fwrite(&pos_data,sizeof(int),1,fil); //mark end of the user-defined items region
		}
		//storing all keycode sequences (easycasts)
		for (i=0;i<24;i++)
		{
			fwrite(&(ToolboxMembers[i].Keycodes[0]),sizeof(char),32*9,fil);
		}
		for (i=0;i<6;i++) //reserved for future use
		{
			fwrite(dummy,sizeof(char),32*9,fil);
		}

		fwrite(&UseToolbar,sizeof(int),1,fil);
		fwrite(&ToolbarEditNodes,sizeof(int),1,fil);
		fwrite(&ToolbarUseCross,sizeof(int),1,fil);
		fwrite(&UseCapsLock,sizeof(int),1,fil);
		fwrite(&MouseWheelDirection,sizeof(int),1,fil);
		fwrite(&DefaultZoom,sizeof(int),1,fil);
		fwrite(&RightButtonTogglesWheel,sizeof(int),1,fil);
		fwrite(&WheelScrollingSpeed,sizeof(int),1,fil);
		fwrite(&PageNumeration,sizeof(int),1,fil);
		fwrite(&AutoResizeToolbox,sizeof(int),1,fil);
		fwrite(&UseComplexIndexes,sizeof(int),1,fil);

		fwrite(&(ToolboxKeyboardElements.FormatingBigGreek2[0]),sizeof(char),32,fil);
		fwrite(&(ToolboxKeyboardElements.FormatingSmallGreek2[0]),sizeof(char),32,fil);	
		fwrite(&(ToolboxKeyboardElements.FormatingBigCaps2[0]),sizeof(char),32,fil);
		fwrite(&(ToolboxKeyboardElements.FormatingSmallCaps2[0]),sizeof(char),32,fil);	

		fclose(fil);
	}
	return 0;
}


#pragma optimize("s",on)
int CToolbox::LoadSettings(char * filename)
{
	int is_first_load=0;
	if (origToolboxNumMembers==0)
	{
		is_first_load++;
		//first time call - we are storing original values;
		origToolboxNumMembers=ToolboxNumMembers;
		for(int i=0;i<ToolboxNumMembers;i++)
			ToolboxMembers[i].origNumSubmembers=ToolboxMembers[i].NumSubmembers;
	}
	else
	{
		//non-first time call - restoring original values
		for (int i=0;i<ToolboxNumMembers;i++)
		{
			for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
				if (ToolboxMembers[i].userdef_mask&(1<<j))
				{
					if (ToolboxMembers[i].Above[j]!=-1)
						delete ToolboxMembers[i].Submembers[j];
					else
						delete ((CDrawing*)ToolboxMembers[i].Submembers[j]);

					ToolboxMembers[i].Above[j]=0;
					ToolboxMembers[i].Below[j]=0;
					ToolboxMembers[i].Keycodes[j][0]=0;
					ToolboxMembers[i].Length[j]=0;
					ToolboxMembers[i].prevToolboxSize[j]=0;
					ToolboxMembers[i].zoom[j]=0;
				}
			ToolboxMembers[i].userdef_mask=0;
			ToolboxMembers[i].NumSubmembers=ToolboxMembers[i].origNumSubmembers;
		}
		ToolboxNumMembers=origToolboxNumMembers;
	}

	//saves toolbox setings and menu options
	char *orig_filename=filename;

	char DefaultFilename[512];
	if (filename==NULL) 
	{
		is_first_load++;
		int j;		
		strcpy(DefaultFilename,GetCommandLine());
		
		DefaultFilename[511]=0;

		int find_quot=0;
		if (DefaultFilename[0]=='"') {j=1;find_quot=1;} else j=0;
		while (DefaultFilename[j])
		{
			if (DefaultFilename[j]=='"') break;
			if (!find_quot) if (DefaultFilename[j]==' ') break;
			if (j>511) break;
			j++;
		}
		j--;
		for (;j>=0;j--)
		{
			if (DefaultFilename[j]=='\\') break;
			if (DefaultFilename[j]=='"') break;
			if (j==0) break;
		}
		DefaultFilename[j]=0;
		DefaultFilename[j+1]=0;
		if (j>find_quot) strcat(DefaultFilename+find_quot,"\\");
		filename=DefaultFilename+find_quot;



		//loading language strings (from the application folder, file Language.csv)
		if (LanguageStrings==NULL)
		{
			strcat(filename,"Language.csv");
			FILE *fil;
			fil=fopen(filename,"r+b");
			if (fil)
			{
				char line[255];
				int UsedSize=0;
				while(1)
				{
					char ch=0;
					int len=0;
					while (fread(&ch,1,1,fil))
					{
						if ((ch==0x0D) || (ch==0x0A)) break;
						if ((len==0) && (ch<=32)) continue;
						if (len<254) line[len++]=ch;
					}
					line[len]=0;
					if ((ch==0) && (len==0)) break; //end of file, probably
					if ((len>3) && (line[0]!=';'))
					{
						char text[255];
						int id=-1;
						text[0]=0;
						line[250]=0;
						int first_comma=0;
						for (int i=0;i<len;i++)
							if ((line[i]==',') || (line[i]==';')) {first_comma=i;break;}
						int tlen=0;
						int lastchar=0;
						int quot=0;
						for (int i=first_comma+1;i<len+1;i++)
						{
							if (line[i]=='"') 
							{
								if (!quot) {tlen=0;quot=1;}
								else {text[tlen]=0;break;}
								continue;
							}
							if ((line[i]<=32) && (tlen==0) && (!quot)) continue;
							if ((line[i]=='\\') && (line[i+1]=='\\')) 
							{text[tlen++]=0x0D;text[tlen++]=0x0A;i++;
							continue;}
							if ((((line[i]==',') || (line[i]==';')) && (!quot)) || (line[i]==0)) {text[lastchar]=0;break;}
							text[tlen++]=line[i];
							if (line[i]>32) lastchar=tlen;
						}
						text[tlen]=0;

						sscanf(line,"%d",&id);
						if ((id>=0) && (id<36000) && (text[0]))
						{
							if (LanguageStrings==NULL)
							{
								LanguagePointers=(unsigned short*)malloc(sizeof(unsigned short)*36000);
								memset(LanguagePointers,0xFF,sizeof(unsigned short)*36000);
								LanguageStrings=(char*)malloc(32768);
							}
							LanguagePointers[id]=(unsigned short)UsedSize;
							strcpy(LanguageStrings+UsedSize,text);
							UsedSize+=(int)strlen(text)+1;
							if (UsedSize>32000) break; //no more space memory
						}
					}
				}
				fclose(fil);
			}
			
			filename[strlen(filename)-12]=0;
		}

		strcat(filename,"Mathomir.set");
	}

	FILE *fil;
	fil=fopen(filename,"r+b");
	if (fil)
	{
		short dummy[16*9];
		int i;
		for (i=0;i<24;i++)
		{
			fread(&ToolboxMembers[i].AcceleratorKey[0],sizeof(short),28,fil);
			fread(dummy,sizeof(short),4,fil); //for compatibility with earlier version
			fread(&ToolboxMembers[i].SelectedSubmember,sizeof(short),1,fil);
		}
		for (i=0;i<6;i++)
		{
			fread(dummy,sizeof(short),32,fil); //for compatibility with earlier version
			fread(dummy,sizeof(short),1,fil); //for compatibility with earlier version
		}
		fread(dummy,sizeof(short),16,fil);
		fread(dummy,sizeof(short),16,fil); //for compatibility with earlier version
		fread(&(ToolboxFontFormating.UniformAccKey[0]),sizeof(short),12,fil);
		fread(dummy,sizeof(short),4,fil);
		fread(dummy,sizeof(short),16,fil); //for compatibility with earlier version
		fread(&(ToolboxFontFormating.MixedAccKey),sizeof(short),1,fil);
		fread(&(ToolboxFontFormating.SelectedUniform),sizeof(short),1,fil);
		if (ToolboxFontFormating.SelectedUniform>7) ToolboxFontFormating.SelectedUniform=0;

		fread(dummy,sizeof(short),1,fil);
		for (i=0;i<32;i++)
		{
			if (i<ToolboxFontFormating.NumFormats)
			{
			   char ch;
			   if (fread(&ch,1,1,fil))
			   {
					int clr=-1;
					char tmp=0;
					if (ch&0x80) //only if the highest bit is set then the rest of settings is ok (compatibility)
					{
						clr=((ch>>1)&0x07)-1; //getting out the color information
						if (ch&0x01) tmp|=0x01; //getting out the singleshot information
						tmp|=0x80; //always set the high bit if the configuration is vaild
					}
					ToolboxFontFormating.UniformFormats2[i]=tmp; //always turn on the highest bit
					ToolboxFontFormating.UniformFormats[i]->m_pElementList->pElementObject->m_Color=clr;
			   }
			   if (fread(&ch,1,1,fil))
			   {
					ToolboxFontFormating.UniformFormats[i]->m_pElementList->pElementObject->Data2[0]=ch;
					if (ToolboxFontFormating.UniformFormats2[i]==0) //unknown configuration for color and singleshot - initialize (compatibility)
					{
						ToolboxFontFormating.UniformFormats2[i]|=0x80;
						if ((ch&0x1C) || ((ch&0xE0)==0x60))
							ToolboxFontFormating.UniformFormats2[i]|=0x01;
					}
			   }
			}
			else
			{
				char ch;
			    fread(&ch,1,1,fil);
			   	fread(&ch,1,1,fil);
			}
		}		



		fread(&(ToolboxKeyboardElements.FormatingBigCaps[0]),sizeof(char),32,fil);
		fread(dummy,sizeof(char),16,fil); //for compatibility with earlier version
		fread(&(ToolboxKeyboardElements.FormatingSmallCaps[0]),sizeof(char),32,fil);
		fread(dummy,sizeof(char),16,fil); //for compatibility with earlier version
		AdjustKeyboardFont();
		fread(&CenterParentheses_not_used,sizeof(int),1,fil);
		fread(&DefaultParentheseType,sizeof(int),1,fil);
		fread(&FrameSelections,sizeof(int),1,fil);
		fread(&FixFontForNumbers,sizeof(int),1,fil);
		fread(&UseALTForExponents,sizeof(int),1,fil);
		fread(&PaperHeight,sizeof(int),1,fil);
		if (PaperHeight==0) PaperHeight=1100;
		fread(&PaperWidth,sizeof(int),1,fil);
		fread(&ToolboxSize,sizeof(int),1,fil);
		if (ToolboxSize<60) ToolboxSize=60;
		if (ToolboxSize>150) ToolboxSize=150;
		BaseToolboxSize=ToolboxSize;
		RECT tmpRect;
		fread(&tmpRect,sizeof(RECT),1,fil);
		if (orig_filename==NULL) MainWindowRect=tmpRect;
		fread(&ImageSize,sizeof(int),1,fil);
		fread(&ForceHighQualityImage,sizeof(int),1,fil);
		fread(&ForceHalftoneImage,sizeof(int),1,fil);
		fread(&DefaultFontSize,sizeof(int),1,fil);
			if (DefaultFontSize>150) DefaultFontSize=150;
			if (DefaultFontSize<85) DefaultFontSize=80;
		fread(&MovingDotSize,sizeof(int),1,fil);
		fread(&MovingDotPermanent,sizeof(int),1,fil);
		fread(&GRID,sizeof(int),1,fil);
		fread(&IsShowGrid,sizeof(int),1,fil); //for compatibility (it was 'SnapToGrid' before)
		fread(&IsShowGrid,sizeof(int),1,fil);
		fread(&IsSimpleVariableMode,sizeof(int),1,fil);
		fread(&IsMathDisabled,sizeof(int),1,fil);
		fread(&ImaginaryUnit,sizeof(char),1,fil);
		fread(&ShadowSelection,sizeof(int),1,fil);
		fread(&AutosaveOption,sizeof(int),1,fil);
		fread(&MoveCursorOnWheel,sizeof(int),1,fil);
		fread(&EnableMenuShortcuts,sizeof(int),1,fil);
		fread(&FontFacenames[0][0],4*32,1,fil);
		fread(&FontAdjustedSizes[0],4,1,fil);
		fread(&FontCharSet[0],4,1,fil);
		fread(&FontWeight[0],4*sizeof(int),1,fil);
		fread(&F1SetsZoom,sizeof(int),1,fil);
		fread(&PrintTextAsImage,sizeof(int),1,fil);
		fread(&UseCommaAsDecimal,sizeof(int),1,fil);
		fread(&dummy,sizeof(int),1,fil);
		fread(&SnapToGuidlines,sizeof(int),1,fil);
		fread(&UseCTRLForZoom,sizeof(int),1,fil);

		//read mixed font format
		if (!fread(ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2,sizeof(char),1,fil))
			ToolboxFontFormating.MixedFormat->m_pElementList->pElementObject->Data2[0]=ToolboxKeyboardElements.FormatingSmallCaps[0];
		fread(&m_FontModeSelection,sizeof(int),1,fil); if ((m_FontModeSelection>2) || (m_FontModeSelection<1)) m_FontModeSelection=1;

		memcpy(&ToolboxKeyboardElements.FormatingBigGreek[0],&ToolboxKeyboardElements.FormatingBigCaps[0],32);
		memcpy(&ToolboxKeyboardElements.FormatingSmallGreek[0],&ToolboxKeyboardElements.FormatingSmallCaps[0],32);
		fread(&(ToolboxKeyboardElements.FormatingBigGreek[0]),sizeof(char),32,fil);
		fread(&(ToolboxKeyboardElements.FormatingSmallGreek[0]),sizeof(char),32,fil);	

		m_prevFontModeSelection=0;
		if ((ToolboxFontFormating.UniformFormats2[ToolboxFontFormating.SelectedUniform]&0x01)==0) //if it is not a singleshot mode
		{
			m_prevFontModeSelection=ToolboxFontFormating.SelectedUniform;
		}
		if (m_FontModeSelection==1) m_prevFontModeSelection|=0x80; //high bit set if mixed formatting is activated

		//read user-defined toolbox items
		while (1)
		{
			int is_drawing=0;
			int pos_data=-1;
			int len=-1;
			fread(&pos_data,sizeof(int),1,fil);
			if (pos_data>1024) {pos_data-=1024;is_drawing=1;}
			if (pos_data<0) break;
			if (pos_data>=260) break;
			fread(&len,sizeof(int),1,fil);
			if (len==-1) break;
			if (len>0xFFFF) break;
			char *data=(char*)malloc(len+256);
			fread(data,len,1,fil);

			CExpression *tmp;
			if (!is_drawing)
			{
				tmp=new CExpression(NULL,NULL,100);
				tmp->XML_input(data);
			}
			else
			{
				tmp=(CExpression*)(new CDrawing());
				((CDrawing*)tmp)->XML_input(data);
			}
			if (pos_data==256) //adding a new member
			{
				pos_data=ToolboxNumMembers;
				ToolboxNumMembers++;
			}
			if (pos_data==257)
			{
				pos_data=ToolboxNumMembers-1;
			}
			if (pos_data<256)
			{
				ToolboxMembers[pos_data].Submembers[ToolboxMembers[pos_data].NumSubmembers]=tmp;
				ToolboxMembers[pos_data].CreationCode[ToolboxMembers[pos_data].NumSubmembers]=0;
				ToolboxMembers[pos_data].Above[ToolboxMembers[pos_data].NumSubmembers]=(is_drawing)?-1:0;
				ToolboxMembers[pos_data].userdef_mask|=1<<ToolboxMembers[pos_data].NumSubmembers;
				ToolboxMembers[pos_data].NumSubmembers++;
			}
			free(data);
		}
		//reading all keycode sequences
		for (i=0;i<24;i++)
		{
			fread(&(ToolboxMembers[i].Keycodes[0]),sizeof(char),32*9,fil);
		}
		for (i=0;i<6;i++)
		{
			fread(dummy,sizeof(char),32*9,fil);
		}

		fread(&UseToolbar,sizeof(int),1,fil);
		fread(&ToolbarEditNodes,sizeof(int),1,fil);
		fread(&ToolbarUseCross,sizeof(int),1,fil);
		fread(&UseCapsLock,sizeof(int),1,fil);
		fread(&MouseWheelDirection,sizeof(int),1,fil);
		fread(&DefaultZoom,sizeof(int),1,fil); 
			if ((DefaultZoom!=150) && (DefaultZoom!=120) && (DefaultZoom!=100) && (DefaultZoom!=80)) DefaultZoom=100;
			if (NumDocumentElements==0) ViewZoom=DefaultZoom;
		fread(&RightButtonTogglesWheel,sizeof(int),1,fil);
		fread(&WheelScrollingSpeed,sizeof(int),1,fil);
		fread(&PageNumeration,sizeof(int),1,fil);
		fread(&AutoResizeToolbox,sizeof(int),1,fil);
		fread(&UseComplexIndexes,sizeof(int),1,fil);

		fread(&(ToolboxKeyboardElements.FormatingBigGreek2[0]),sizeof(char),32,fil);
		fread(&(ToolboxKeyboardElements.FormatingSmallGreek2[0]),sizeof(char),32,fil);	
		fread(&(ToolboxKeyboardElements.FormatingBigCaps2[0]),sizeof(char),32,fil);
		fread(&(ToolboxKeyboardElements.FormatingSmallCaps2[0]),sizeof(char),32,fil);	

		AdjustPosition();
		fclose(fil);
	}
	else if (is_first_load==2) //the mathomir.set is not found - load the FirstRun help
	{
		filename[strlen(filename)-12]=0;
		strcat(filename,"FirstRun.mom");
		fil=fopen(filename,"rb");
		if (fil)
		{
			fclose(fil);
		}
		else
		{
			filename[strlen(filename)-12]=0;
			strcat(filename,"FirstRun_.mom");
			fil=fopen(filename,"rb");
			if (fil)
			{
				fclose(fil);
				char filename2[512];
				strcpy(filename2,filename);
				filename2[strlen(filename2)-5]=0;
				strcat(filename2,".mom");

				if (!rename(filename,filename2))
				{
					
					pMainView->GetDocument()->OpenMOMFile(filename2);
					HelpTutorLoaded=1;
				}
		
			}
		}
	}
		
			


	ClearFontPool(); 
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	return 0;
}
#pragma optimize("s",off)

void CToolbox::OnSaveoptionsSaveasdefault()
{
	//not used
}

void CToolbox::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	if (((Toolbox->Keyboard->IsWindowVisible()) && (Toolbox->Keyboard->GetActiveWindow())) ||
		((Toolbox->Subtoolbox->IsWindowVisible()) && (Toolbox->Subtoolbox->GetActiveWindow())))
	{
		theApp.m_pMainWnd->SetActiveWindow();
		pMainView->OnKeyDown(nChar,nRepCnt,nFlags);
		//pMainView->SetActiveWindow();
	}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CToolbox::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (((Toolbox->Keyboard->IsWindowVisible()) && (Toolbox->Keyboard->GetActiveWindow())) ||
		((Toolbox->Subtoolbox->IsWindowVisible()) && (Toolbox->Subtoolbox->GetActiveWindow())))
	{
		theApp.m_pMainWnd->SetActiveWindow();
		pMainView->OnSysKeyDown(nChar,nRepCnt,nFlags);
		//pMainView->SetActiveWindow();
	}
	CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

unsigned int DoublestrokesTimer;
CExpression *CToolbox::CheckForKeycodes(char *keystrokes, int *len)
{
	//first we check for double-strokes
	int l2=*len;
	if (l2<1) return NULL;

	if ((!KeyboardEntryObject) || (((CExpression*)KeyboardEntryObject)->m_pPaternalElement==NULL) || (((CExpression*)KeyboardEntryObject)->m_pPaternalElement->m_Type!=1)) //do not use double-strokes in indexes
	if (l2>=2)
	{
		unsigned int ttt=750;
		if (IsHighQualityRendering) ttt+=70;
		if (IsHalftoneRendering) ttt+=70;
		if (ViewZoom>280) ttt+=(ttt-200)/3;
		if (GetTickCount()-DoublestrokesTimer<ttt) 
		for (int i=0;i<ToolboxNumMembers;i++)
		{
			for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
			{
				short keycode=(ToolboxMembers[i].AcceleratorKey[j]);
				if (keycode&0x400)
				{ 
					keycode&=0xFF;
					if ((keystrokes[l2-2]==keycode) && (keystrokes[l2-1]==keycode)) 
					{
						*len=2;
						return ToolboxMembers[i].Submembers[j];
					}
				}
			}
		}
	}
	DoublestrokesTimer=GetTickCount();


	//then we check for easycast strokes - we are searching for the longest mach
	int candidate_member;
	int candidate_submember;
	int candidate_len=-1;
	for (int i=0;i<ToolboxNumMembers;i++)
	{
		for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
		{
			char *keycode=&ToolboxMembers[i].Keycodes[j][0];

			int l=(int)strlen(keycode);
			if (l<1) continue;

			if (memcmp(keystrokes+l2-l,keycode,l)==0)
			{
				//we found a mach
				if (l>candidate_len)
				{
					candidate_member=i;
					candidate_submember=j;
					candidate_len=l;
				}
			}
		}
	}
	if (candidate_len>0)
	{
		*len=candidate_len;
		return ToolboxMembers[candidate_member].Submembers[candidate_submember];
	}
	return NULL;
}

CExpression* CToolbox::ReturnKeycode(int keycode_order,char **Keycode)
{
	int k=0;
	for (int i=0;i<ToolboxNumMembers;i++)
	{
		for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
		{
			if (ToolboxMembers[i].Keycodes[j][0])
			{
				if (k==keycode_order)
				{
					*Keycode=ToolboxMembers[i].Keycodes[j];
					return ToolboxMembers[i].Submembers[j];
				}
				k++;

			}
		}
	}
	*Keycode=NULL;
	return NULL;
}


int prev_lang_code;
void CToolbox::ShowHelptext(char *text,char *command, char *accelerator, char *easycast, int language_code)
{
	if (!Toolbox->IsWindowVisible()) return;
	if (ToolboxSize<1) return;
	CMainFrame *mf=(CMainFrame*)theApp.m_pMainWnd;
	char buff[256];
	CopyTranslatedString(buff,text,language_code,256);
	RECT r;

	Toolbox->GetClientRect(&r);
	CDC *DC=mf->GetDC();

	int yy=r.bottom+ToolboxSize/12;
	if (Toolbox->Subtoolbox->IsWindowVisible())
	{
		RECT r2;
		Toolbox->GetWindowRect(&r);
		Toolbox->Subtoolbox->GetWindowRect(&r2);
		if (r2.bottom>r.bottom) yy+=r2.bottom-r.bottom;
	}

	if (prev_lang_code!=language_code+yy)
		DC->FillSolidRect(0,yy,ToolboxSize,ToolboxSize*2,RGB(128,128,128));

	prev_lang_code=language_code+yy;
	if (!language_code) 
	{
		mf->ReleaseDC(DC);
		return;
	}

	DC->SetBkMode(OPAQUE);
	DC->SetBkColor(RGB(128,128,128));
	CSize size;

	CRgn myrgn;
	myrgn.CreateRectRgn(0,yy,ToolboxSize,yy+ToolboxSize);
	DC->SelectClipRgn(&myrgn);

	DC->SetTextColor(RGB(204,204,204));
	if (buff[0])
	{
		DC->SelectObject(GetFontFromPool(4,0,0,4+ToolboxSize/7));
		int len=(int)strlen(buff);
		int i=0;
		int last=0;
		while (i<=len)
		{
			if (buff[i]==0x0A) {last=i+1;}
			if ((buff[i]==0x0D) || (i==len))
			{
				buff[i]=0;
				DC->TextOut(2,yy,buff+last);
				last=i+1;
				yy+=ToolboxSize/6+2;
			}
			i++;
		}
	}

	if (accelerator[0])
	{
		DC->SetTextColor(RGB(240,192,192));
		DC->SelectObject(GetFontFromPool(4,0,0,ToolboxSize/5));
		size=DC->GetTextExtent(accelerator);if (size.cx>ToolboxSize-1) DC->SelectObject(GetFontFromPool(4,0,0,ToolboxSize/6));
		DC->TextOut(2,yy,accelerator);
		yy+=ToolboxSize/6+1;
	}

	if (easycast[0])
	{
		char bbf[48];
		strcpy(bbf,"\"");
		strcat(bbf,easycast);
		strcat(bbf,"\"");
		DC->SetTextColor(RGB(240,192,192));
		DC->SelectObject(GetFontFromPool(4,0,0,ToolboxSize/5));
		size=DC->GetTextExtent(bbf);if (size.cx>ToolboxSize-1) DC->SelectObject(GetFontFromPool(4,0,0,ToolboxSize/6));
		DC->TextOut(2,yy,bbf);
		yy+=ToolboxSize/6+1;

	}

	if (command[0])
	{
		char bbf[48];
		strcpy(bbf,"\\");
		strcat(bbf,command);
		DC->SetTextColor(RGB(192,192,192));
		DC->SelectObject(GetFontFromPool(4,0,0,(ToolboxSize>70)?ToolboxSize/5:ToolboxSize/4));
		size=DC->GetTextExtent(bbf);if (size.cx>ToolboxSize-1) DC->SelectObject(GetFontFromPool(4,0,0,ToolboxSize/5));
		DC->TextOut(2,yy,bbf);
	}

	DC->SelectClipRgn(NULL);
	mf->ReleaseDC(DC);
}

extern int CalcStructuralChecksumOnly;
void CToolbox::PickUpElementFromToolbox(int member, int submember)
{
	IsDrawingMode=0;
	if (ClipboardExpression)
	{
		delete ClipboardExpression;
		ClipboardExpression=NULL;
	}

	if (this->m_IsMain)
	{
		Subtoolbox->m_IsSubtoolbox=-1;
		Subtoolbox->ShowWindow(SW_HIDE);
	}
	else
	{
		m_IsSubtoolbox=-1;
		ShowWindow(SW_HIDE);
	}
	Sleep(10);

	if (ToolboxMembers[member].Above[submember]==-1) //drawing element
	{
		if (ToolboxMembers[member].userdef_mask&(1<<submember))
		{
			//user defined drawing was clicked
			if (ClipboardDrawing) delete ClipboardDrawing;
			ClipboardDrawing=new CDrawing();
			ClipboardDrawing->CopyDrawing((CDrawing*)ToolboxMembers[member].Submembers[submember]);
			ClipboardDrawing->FindBottomRightDrawingPoint(&MovingStartX,&MovingStartY);
			MovingStartX/=DRWZOOM;
			MovingStartY/=DRWZOOM;
		}
		else
		{
			IsDrawingMode=((CDrawing*)ToolboxMembers[member].Submembers[submember])->OriginalForm;
			//if (ToolbarEditNodes) {ToolbarEditNodes=0;if ((UseToolbar) && (Toolbox->Toolbar)) Toolbox->Toolbar->ConfigureToolbar();}
			((CDrawing*)ToolboxMembers[member].Submembers[submember])->SelectDrawing(0);
		}
	}
	else
	{
		ClipboardExpression=new CExpression(NULL,NULL,100);
		ClipboardExpression->CopyExpression(ToolboxMembers[member].Submembers[submember],0);
		short l,a,b;
		ClipboardExpression->CalculateSize(this->GetDC(),100,&l,&a,&b);
		ToolboxMembers[member].Submembers[submember]->DeselectExpression();
	}

	pMainView->SetMousePointer();
}

#pragma optimize("s",on)
int CToolbox::InsertIntoToolbox(void)
{
	int member=ToolboxNumMembers;
	int submember=0;

	if (!this->m_IsMain)
	{
		member=m_IsSubtoolbox-1;
		submember=ToolboxMembers[member].NumSubmembers;

	}


	if (ClipboardExpression)
	{
		int ok=1;

		CalcStructuralChecksumOnly=1;
		int chksm=ClipboardExpression->CalcChecksum();
		for (int i=0;i<ToolboxNumMembers;i++)
			if (ok) for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
				if (ToolboxMembers[i].Above[j]!=-1)
				if (ToolboxMembers[i].Submembers[j]->CalcChecksum()==chksm) {ok=0;break;}
		CalcStructuralChecksumOnly=0;

		if (this->m_IsMain)
		{
			if (ToolboxNumMembers>=24) ok=0;
		}
		else
		{
			if (ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers>=32) ok=0;
		}
		
		if (ok)
		{
			char bff[128];
			CopyTranslatedString(bff,"Insert into toolbox?",5080,127);
			if (AfxMessageBox(bff,MB_YESNO)!=IDYES) ok=0;
		}
		if (ok)
		{
			ClipboardExpression->DeselectExpression();
			ToolboxMembers[member].Submembers[submember]=ClipboardExpression;
			ToolboxMembers[member].userdef_mask|=1<<submember;
			ToolboxMembers[member].Above[submember]=0;
			if (m_IsMain) 
			{
				ToolboxMembers[member].NumSubmembers=1;
				ToolboxMembers[member].SelectedSubmember=0;
				ToolboxNumMembers++; 
			}
			else 
				ToolboxMembers[member].NumSubmembers++;
			ClipboardExpression=NULL;
			AdjustPosition();
			InvalidateRect(NULL,0);
			UpdateWindow();
		}
		else 
		{
			delete ClipboardExpression;
			ClipboardExpression=NULL;
		}
		return 1;
	}
	else if (ClipboardDrawing)
	{
		int ok=1;
		int chksm=ClipboardDrawing->CalcChecksum();
		for (int i=0;i<ToolboxNumMembers;i++)
			if (ok) for (int j=0;j<ToolboxMembers[i].NumSubmembers;j++)
				if (ToolboxMembers[i].Above[j]==-1)
				if (((CDrawing*)ToolboxMembers[i].Submembers[j])->CalcChecksum()==chksm) {ok=0;break;}

		if (this->m_IsMain)
		{
			if (ToolboxNumMembers>=24) ok=0;
		}
		else
		{
			if (ToolboxMembers[m_IsSubtoolbox-1].NumSubmembers>=32) ok=0;
		}

		//we must temporarely store the drawing because it might get deleted while the message box is displayed
		CDrawing *cd=ClipboardDrawing;
		ClipboardDrawing=NULL;

		if (ok)
		{
			char bff[128];
			CopyTranslatedString(bff,"Insert into toolbox?",5080,127);
			if (AfxMessageBox(bff,MB_YESNO)!=IDYES) ok=0;
		}
		if (ok)
		{
			cd->SelectDrawing(0);
			ToolboxMembers[member].Submembers[submember]=(CExpression*)cd;
			ToolboxMembers[member].userdef_mask|=1<<submember;
			ToolboxMembers[member].Above[submember]=-1;
			if (m_IsMain) 
			{
				ToolboxMembers[member].NumSubmembers=1;
				ToolboxMembers[member].SelectedSubmember=0;
				ToolboxNumMembers++; 
			}
			else 
				ToolboxMembers[member].NumSubmembers++;
			AdjustPosition();
			InvalidateRect(NULL,0);
			UpdateWindow();
		}
		else
		{
			delete cd;
		}
		return 1;
	}

	return 0;
}
#pragma optimize("",on)

void CToolbox::AutoResize()
{
	//this functions resizes toolbox to fit the main window size

	if (ToolboxSize) //only if toolbox is shown
	{
		RECT r;
		theApp.m_pMainWnd->GetClientRect(&r);
		int sizes[10]={120,102,84,60,0};
		int i=0;
		int pts=ToolboxSize;
		while (sizes[i])
		{
			int bottom=sizes[i]*((ToolboxNumMembers+7)/3);
			if (sizes[i]>=100) bottom+=100;
			if (sizes[i]<=60) bottom-=60;
			if ((bottom<r.bottom) && ((sizes[i]*6<r.right) || (!UseToolbar)))
			{
				ToolboxSize=BaseToolboxSize=sizes[i];
				break;
			}
			i++;
		}
		if (!sizes[i]) ToolboxSize=BaseToolboxSize=60;
		if (pts!=ToolboxSize)
		{
				Toolbox->AdjustPosition();
				Toolbox->RedrawWindow();
				pMainView->AdjustPosition();
				((CMainFrame*)theApp.m_pMainWnd)->AdjustMenu();
				if (UseToolbar) Toolbox->Toolbar->AdjustPosition();
		}
	}
}