#pragma once

#include "Element.h"

//#pragma pack(1)
typedef struct ELEMENT_STRUCT
{
	/* element types:
						0 - empty frame (just a visible placeholder; frame)
						1 - variable / constant / symbols like: infinite, three points...
						2 - operator (like: +, - , /)
						3 - power (superscript)
						4 - rational numbers: A/B
						5 - parentheses
						6 - functions (sin, cos,...)
						7 - sigma, pi, integral...
						8 - root
						9 - condition list
						10 - conditin list as element
						11 - column separator (matrices)
						12 - row seperator (matrices)
	*/
	CElement *pElementObject;
	short int X_pos;
	short int Y_pos;
	short int Length;
	short int Above;
	short int Below;
	char IsSelected;
	char Decoration; //0-none, 1-crossline, 2-encircled, 3-overline, 4-underline
	char Type;
} tElementStruct;

typedef struct MATRIX_ROWS
{
	short above,below;
	short y;
} tMatrixRows;

typedef struct MATRIX_COLUMNS
{
	short length;
	short x;
} tMatrixColumns;

typedef struct PURE_FACTORS
{
	double N1,N2;
	double N3,N4;
	int prec1,prec2;
	char is_frac1,is_frac2;
} tPureFactors;

typedef struct CELL_ATTRIBUTES
{
	char *top_border;
	char *bottom_border;
	char *left_border;
	char *right_border;
	char *alignment;
	char *data_known;
} tCellAttributes;


#define MAX_VARIABLES 20
#define MAX_SUMMANDS 32



class CExpression
{
	// structure VARIABLE_LIST holds all elements of an expression (it is filled by function
	// 'ExtractVariables'). It contains enough data so that the expression can be reconstructed from it
	typedef struct SUMMAND
	{
		double dorder;
		char order;			//order (power) of the variable (can be positive or negative)
		char style;			//how was variable written (bit 0-as a part of fraction; bit 1-has '/' preoperator)
	} tSummand;

	typedef struct VARIABLE
	{
		CExpression *exponent;  //residuum of the exponent (temporary created object)
		CExpression *variable;  //pointer to object that holds the variable
		int pos;                //position of the variable in the object
		int len;                //length of the variable
		tSummand summand[MAX_SUMMANDS];//for every summand in original expression holds order and style         
	} tVariable;

	typedef struct CONSTANT
	{
		double N1; //numerator
		double N2; //denominator
		int prec;  //precision
		int summand_pos;
	} tConstant;

	typedef struct VARIABLE_LIST
	{
		int NumVariables;
		tVariable Variables[MAX_VARIABLES]; //all variables in an expression are listed here
		tConstant Constants[MAX_SUMMANDS]; //constant factors of all summands of an expression are listed here
	} tVariableList;

public:
	CElement *m_pPaternalElement;
	CExpression *m_pPaternalExpression;

public:
	short int m_FontSize;  //FontSize is the number in % (100% is default topmost font-size)
	short int m_Selection; //selected insertion point
	short int m_IsKeyboardEntry; //the index of currently edited variable (or 0 if editing mode is not active)
	short int m_KeyboardCursorPos; //position of cursor within currently edited variable
	short int m_InternalInsertionPoint;

	char m_ParentheseShape; //contains '(', '[', '{'...
	char m_ParenthesesFlags; //bit 0 - force parentheses by user
							//bit 1 - automatic parentheses - paretheses automatically shown under certain circumstances (user cannot change this bit)
							//bit 2 - horizontal
							//bit 3 - no left parenthesis
							//bit 4 - no right parentheses
							//bit 8 - parentheses forcing (calculated)
	short m_ParentheseWidth; //calculated - width of the parenthese
	short m_ParenthesesAbove;//calculated
	short m_ParenthesesBelow;//calculated
	char m_ParenthesesSelected; //if parentheses are touched
	char m_DrawParentheses;  //calculated - are parentheses to be drawn or not

	short m_OverallLength; //overall length (in pixels) of the whole expression - calculated at every CalculateSize call
	short m_OverallAbove;
	short m_OverallBelow;

	short m_MaxNumRows,m_MaxNumColumns;
	short m_MarginX,m_MarginY;
	short m_RowSelection,m_ColumnSelection;
	char m_IsColumnInsertion;
	char m_IsRowInsertion;
	short m_IsMatrixElementSelected;
	char m_Alignment;
	char m_StartAsText;  // defines if the box is to be started in text typing mode
	char m_IsVertical;
	unsigned int m_IsComputed;
	char m_Color;
	int m_ElementListReservations;
	tMatrixRows *m_MatrixRows;
	tMatrixColumns *m_MatrixColumns;
	char m_IsPointerHover; //to paint faint lines within matrix/table when mouse pointer howers above matrix/table
	char m_IsHeadline;
	int m_NumElements;
	tElementStruct *m_pElementList;
	unsigned short m_ModeDefinedAt; //two high bits represent the mode (0=math, 1=text); rest is the position where it was defined (increased for 1); 0 for undefined

public:
	CExpression(CElement *PaternalElement,CExpression *PaternalExpression,short int FontSize);
	~CExpression(void);
	void CalculateSize(CDC *DC,short int zoom, short int * length, short int * above, short int * below,char HQR=-1,char optimize_for_readability=0);
	void PaintExpression(CDC * DC, short zoom, short X, short Y,RECT *ClipReg=NULL,int color=0);
	short GetActualFontSize(short zoom);
	int InsertEmptyElement(short position,short Type,char Operator, int color=-1);
	void SelectExpression(char Select);
	void DeselectExpressionExceptKeyboardSelection(void/*char preserve_keyboard_selection*/);
	void DeselectExpression(void);

	void SelectElement(char Select,int position);

	CObject* SelectObjectAtPoint(CDC* DC, short zoom, short X, short Y,short *IsExpression, char *IsParenthese,char ForceInsertionPoint=0);
	// Copies all data from original expression
	int CopyExpression(const CExpression* Original,const char OnlySelected,const char selection_type=1,const char update_clipboard=1);
	void Delete(void);
	int InsertElement(const tElementStruct* Element, int position);
	int MoveElementInto(const tElementStruct* Element, int position);
	//int MovePortionInto(int destination_pos, CExpression *Original, int source_pos, int num_elements);
	CExpression *CopyAtPoint(CDC* DC, short zoom, short X, short Y, CExpression* Original,int keyboard_select=1);
	int DeleteElement(short position);
	CExpression *AdjustSelection(char keyboard_select=1);
	int KeyboardStart(CDC* DC, short zoom);
	int KeyboardRefocus(CExpression* new_focus, int position);
	int KeyboardPopupClosed(int user_param,int exit_code);
	int RemoveEmptyVariable(CDC *DC,tElementStruct *theElement,char no_recomutation=0);
	int ResolveKnownFunctions(CDC* DC, short zoom, UINT nChar, UINT nRptCnt, UINT nFlags,int fcolor, tElementStruct *theElement);

	int KeyboardKeyHit(CDC* DC, short zoom, UINT nChar, UINT nRptCnt, UINT nFlags, int fcolor, char extern_call);
	// only to be called from KeyboardHit function!!
	tElementStruct* KeyboardSplitVariable(void);
	int PaintParentheses(CDC * DC, short zoom, short X1, short Y1, short X2, short Y2, short ParentheseWidth, char Type,short data, char IsBlue,int color=0);
	int PaintHorizontalParentheses(CDC * DC, short zoom, short X1, short Y1, short X2, short Y2, short ParentheseWidth, char Type,short data, char IsBlue,int color=0);

	int KeyboardStop(void);
	int XML_output(char * output, int num_tabs, char only_calculate);
	char * XML_input(char * file);
	//int MathML_output(char * output, int num_tabs, char only_calculate,char output_type);
	int LaTeX_output(char * output, char only_calculate);

	int CalcChecksum(void);
	CObject* KeyboardFindEntryPos();
	int CopyToWindowsClipboard(void);
	// select element of the matris at (row,column)
	int SelectMatrixElement(int row, int column,char select_type=1);
	// returns the postion of the first element at (row,column), also expands matrix if neccessary
	int FindMatrixElement(int row, int column,int expand_flag);
	int InsertMatrixColumn(int position);
	int InsertMatrixRow(int position);
	int AdjustMatrix(void);
	int DeleteSelection(char selection_type=1);
	int PaintDecoration(CDC *DC,short zoom,int X, int Y,int LastDecorationElement,int i, int LastDecoration,int color=0);
	// direction<0 find positions above x,y;  direction>0 find positions below x,y
	int KeyboardStartAt(int X, int Y, char direction,char between=0);
	int KeyboardQuickType(CDC* DC, short zoom, UINT nChar, UINT nRepCnt, UINT nFlags,int fcolor, int *x, int *y);
	int KeyboardInsertNewEquation(CDC *DC, short zoom, UINT nChar,CExpression *orig,int TypingMode);
	int GetKeyboardCursorPos(int * X, int * Y);
	int ChangeFontSize(float factor);
	int GetElementLen(const unsigned int StartPos, const unsigned int EndPos, const unsigned int Level, char * element_type, char*has_preoperator);
	int FindLowestOperatorLevel(const unsigned int StartPos,const unsigned int EndPos, char default_operator=0);
	int FindLowestOperatorLevel(char default_oper=0);
	int DetermineInsertionPointType(int position); //returns 0 if this is a math or 1 if this is a text position
	int Autocomplete(int is_internal);
	tElementStruct *GetElementStruct(CElement *element);
	CElement *DecodeInternalInsertionPoint(); //returns NULL if no insertion point found, otherwise the element (type=1) with the insertion point
	int ContainsBlinkingCursor();
	CExpression *GetLabel();


	int CompareExpressions(int StartPos, int EndPos, CExpression* Other, int StartPos2, int EndPos2);
	int Compute(int StartPos, int EndPos,int ComputationType,int OutsideLevel=-1);
	int ExecuteComputation(int StartPos, int EndPos, char element_type, int StartPos2, int EndPos2, char element_type2,int ComputationType);
	int IsPureNumber(int StartPos,int len, double * number,int *precision);
	int GenerateASCIINumber(double number_dbl,long long number_int, char is_integer, int precision, int position);
	int CompareElement(const tElementStruct * ts1,const tElementStruct * ts2);
	//int GetOperatorLevel(char op);
	// multiplies this expression with the multiplier
	int ComputeExponent(int Position, char element_type, int ComputationType);
	int FactorizeExpression(int force_factorization=0);
	int ComputeRoot(int Position, char element_type, int ComputationType);
	int ExtractVariables(int StartPos, int EndPos, double order,int summand_no, tVariableList *VarList, CExpression * exponent=NULL,char style=0);
	int Polynomize(CExpression * variable,char *alternative_variable=NULL,char is_greek=0);
	int DividePolynome(CExpression * Q, int orderQ,CExpression * result, CExpression * variable);
	int ComputeFraction(int Position, char element_type, int ComputationType);
	int MultiplyElements(int StartPos, int EndPos, int inv, CExpression * Other, int StartPos22, int EndPos2, int inv2,int ComputationType);
	int ReduceTwoNumbers(double * N1, double * N2);
	int RemoveSequence(int Level, int StartPos, int EndPos=-1);
	int InsertSequence(char element_type, int Position, CExpression * Source, int StartPos, int EndPos);
	int PROFILERClear(void);
	int PROFILEREnd(void);
	int StrikeoutCommonFactors(int StartPos, int EndPos, int inv, CExpression * Other, int StartPos2, int EndPos2, int inv2, tPureFactors *PureFactors=NULL,int UseDeepCalculation=0);
	int StrikeoutRemove(int StartPos, int EndPos,char test_value=0);
	char GetDefaultElementType(int Level);
	static int InitCalculator(void);// can be called only once, before any symbolic-calculator function is used
	int MakeExpressionBeautiful(void);// changes the expression so it becomes more beautiful (deletes '+' at the begginig, adds '*' between constants...)
	int IsSuitableForComputation(int autocorrect=0);
	int ComputeLog(int Position, char element_type, int ComputationType);
	int ContainsVariable(int StartPos, int EndPos, CExpression * variable, int VarPos, int VarLen,char *alternative_variable=NULL,char is_greek=0);
	int ExtractVariable(CExpression* variable, int VarPos,int VarLen,int ComputationLevel);
	int SynthetizeExpression(int Position, void * VarList, int summand_no);
	int ComputeMatrixDeterminant(int ComputationType);
	int ComputeParentheses(int Position, char element_type, int ComputationType,int Level);

	// //returns zero if matrix is non-regular (different number of columns per row)
	int GetMatrixSize(int* rows, int* columns);
	int MatrixInvert(int ComputationType);
	int GenerateASCIIFraction(int Position,double num, double denom, int precision , int is_fraction);
	int ComputeSinCos(int Position, char element_type, int ComputationType);
	int ComputeASinCos(int Position, char element_type, int ComputationType);
	int ComputeHSinCos(int Position, char element_type, int ComputationType);


	int Derivate(CExpression * variable,int internall_call=0);

	void *PlotterPrepareVariablePositions(void *VarPos=NULL);
	double PlotterCalculateFunctionValue(double X,void *VarPos);
	int PlotterReleaseVariablePositions(void *VarPos);
	int PlotterGetEquationInfo(CExpression **variable,int *position,int *starting_point,char *equation_type);
	int SetColor(int color);
	int MakeSubstitution(CExpression *Substitute, CExpression *Variable);
	void *GenerateVariableList(int StartPos,int EndPos,int *summand_no, tVariableList *VarList=NULL);
	void FreeVariableList(tVariableList *VarList);
	int FindReplace(int StartPos,int EndPos,CExpression *Find,CExpression *Replace);
	int SolveSystemOfEquations(CExpression *System[],int *NumEquations, CObject *ThePopupMenu);
	int CountVariablesInSystem(CExpression *System[],int NumEquations,void *VariableStorage);
	int CodeDecodeUnitsOfMeasurement(int StartPos,int EndPos);
	int ComputeUnits();
	int SearchForString(char *str);
	int ConvertToPlainText(int buffer_size,char *str,char force_parentheses=0);
	int AutowrapText(CDC *DC,int width,int rewrap_all);
	int IsTextContained(int position, char unmark_at_line_start=0);
	int GetCellAttributes(int row, int column, tCellAttributes *attributes);
	//int GetCellAttributes(int row, int column,char *align, char *top, char *bottom, char *left, char *right);
	int SetCellAttributes(int row, int column,char align, char top, char bottom, char left, char right);
	//int FuseDifferentials(void);
	void CalculateSizeReadjust(short zoom,short *length,short *above,short *below);
	int InsertTabSpacer(int position, char stop_on_equality);
	void AddToStackClipboard(int add_forcefully);
	//void StoreBackspaceStorage();
};
