#pragma once


class CElement
{
public:
	void *operator new(size_t size);
	//void operator delete(void *p);

public:
	CObject *m_pPaternalExpression;
	short *Data3;
	CObject *Expression1;
	CObject *Expression2;
	CObject *Expression3;
	short ParenthesesBelow;
	short ParenthesesAbove;

	char m_Type; //type of Element
	char m_Color;
	char m_Text;
	unsigned char m_VMods; //variable modificators (overhead dash, hat, arrow, dot, double dot, measurement unit)

	//the below codes must have fixed structure (see the 'operator new' and 'CopyElement')
	short E1_length,E1_above,E1_below,E1_posX,E1_posY;
	char Data2[4];
	short E2_length,E2_above,E2_below,E2_posX,E2_posY;
	short E3_length,E3_above,E3_below,E3_posX,E3_posY;
	char Data1[4]; 

public:
	CElement(void);
	~CElement(void);
	void CalculateSize(CDC *DC,short int zoom, short int * length, short int * above, short int * below,short paternal_position,char HQR);
	void PaintExpression(CDC * DC, short zoom, short X, short Y, char IsBlue,int ActualSize,RECT *ClipReg=NULL,int color=0);
	void Empty(char oper);
	void CopyElement(const CElement* Element);
	int FontSizeForType(int subelement_no);

	CObject *SelectAtPoint(CDC* DC, short zoom, short X, short Y, short* IsExpression, char* IsParenthese, short paternal_position);
	int XML_output(char * output, int num_tabs, char only_calculate);
	int LaTeX_output(char * output, char only_calculate);
	char * XML_input(char * file,void *element_struct);
	int CalcChecksum(void);

	int IsMeasurementUnit();
	int SetColor(int color);
	int IsDifferential(int only_nonparentheses=0);

	CElement *GetPreviousElement();
	CElement *GetNextElement();
	int GetPaternalPosition();
	int ContainsBlinkingCursor();
	void CalculateSizeReadjust(short zoom,short *length,short *above,short *below,char align);
};
