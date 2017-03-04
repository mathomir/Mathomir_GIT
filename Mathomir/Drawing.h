#pragma once
#include "expression.h"

//scalling factor that defines drawings inner precision
#define DRWZOOM 32
#define halfDRWZOOM 16
#define tDrwXY int
typedef struct DRAWING_ITEM
{
	void *pSubdrawing;
	tDrwXY X1,Y1,X2,Y2;
	short LineWidth;
	char Type; //0-subdrawing, 1-line, 2-subexpression
} tDrawingItem;


class CDrawing
{
public:
	tDrawingItem *Items;
	void *SpecialData;
	tDrwXY NodeX;
	tDrwXY NodeY;
	unsigned short NumItems;
	unsigned short NumItemsReserved;

	char IsSelected;
	char m_Color;
	char OriginalForm; //used from toolbox (to store the purpose of this drawing)
	char IsSpecialDrawing;

	CDrawing(void);
	~CDrawing(void);
	int StartCreatingItem(int ItemForm);
	int EndCreatingItem(int *X, int *Y, int absX=0x7FFFFFFF, int absY=0x7FFFFFFF);  //return X and Y coordinates of the upper left corner
	int UpdateCreatingItem(int X, int Y, int absX, int absY);
	int Delete(void);
	int InsertEmptyElement(int form, int Cx, int Cy);
	int CalculateSize(CDC* DC, short zoom, short *width, short *height);
	void PaintDrawing(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg=NULL,int color=0);
	void SelectDrawing(char select);
	int CalcChecksum(void);
	int CopyDrawing(CDrawing* Original);
	CObject* SelectObjectAtPoint(CDC* DC, short zoom, short X, short Y, int *NodeEdit,int internal_call=0);
	int XML_output(char * output, int num_tabs, char only_calculate);
	char *XML_input(char * file);

	// Erases the square drawing part
	int EraseSquare(int X1, int Y1, int X2, int Y2, CDrawing *parent);
	// finds crospoint of an drawing item with vertical line
	int FindCrosspointX(tDrawingItem * di, int X, int Y1, int Y2, int * pX, int * pY);
	// finds crosspoint of an drawing item with horizontal line
	int FindCrosspointY(tDrawingItem * di, int Y, int X1, int X2, int * pX, int * pY);
	int InsertItemAt(int pos);
	int Combine(void);
	int BreakApart(tDrawingItem* di,CDrawing *parent);
	int CopyDrawingIntoSubgroup(CDrawing* Original, int x, int y);
	int CopyExpressionIntoSubgroup(CExpression* Original, int x, int y, int widht, int height);
	int SetLineWidth(int width);
	int ScaleForFactor(float factorx,float factory);
	int RotateForAngle(float angle, int centerX, int centerY, int* newX1, int* newY1, int* newW, int* newH);
	int MoveNodeCoordinate(int X, int Y);
	int AdjustCoordinates(int* x1, int* y1, int* w, int* h,int absX=0x7FFFFFFF, int absY=0x7FFFFFFF);
	int SetNodeEdit(int is_edit);
	int AnyNodeSelected(void);
	// returns coordinates of the real upper left corner
	int FindRealCorner(int* X, int* Y,int *X2=NULL,int *Y2=NULL);
	int CopyToWindowsClipboard(void);
	int SplitLineAtPos(int X, int Y);
	int MouseClick(int X, int Y);
	int MouseMove(CDC *DC,int X, int Y,UINT flags);
	// returns lenght of the diagonal from given point to drawing lines
	int FindDiagonalLength(int X, int Y, int* l1, int* l2,int direction);
	int SetColor(int color);
	int IsOpenPath(int close_path,char *is_closed_path=NULL,LPPOINT points=NULL,char *num_points_found=NULL);
	int	MakeDashed(char dash_dot);
	int FindNerbyPoint(int *X, int *Y, CDrawing *drw,int X0,int Y0,int X1,int Y1);
	void FindBottomRightDrawingPoint(int *X, int *Y);
	int AllowQuickEditNodes(void);
};


class CDrawingBox
{
public:
	CDrawing *Base; //this is a pointer to main document item that cointains Drawing box frame and other data
	char IsToolboxShown;
	char TheState;
	int ToolboxX;
	int ToolboxY;
	int ToolboxHeight;
	int ToolboxLength;
	int ToolboxSelectedItem;
	CExpression *CommandLine;
	int prevDrawingBoxData;

	CDrawingBox(CDrawing *BaseItem);
	~CDrawingBox(void);

	int Paint(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg,int no_background);
	int MouseMove(CDC *DC,int X, int Y, UINT flags);
	int CopyFrom(CDrawing *Original);
	int MouseClick(int X, int Y);
	int XML_output(char * output, int num_tabs, char only_calculate);
	char *XML_input(char * file);

	int ExecuteCommandLine(short X, short Y, int absX, int absY);
	void GetDrawingBoxGrid(int *unit_size_x, int *unit_size_y, int *startx, int *starty);
};

class CFunctionPlotter
{
public:
	CDrawing *Base; //this is a pointer to main document item that cointains Drawing box frame and other data
	CBitmap *Plot;
	unsigned char is_y_log;
	unsigned char is_x_log;
	unsigned char abort_request;
	unsigned char show_no_scale;
	unsigned char calc_y;
	unsigned char analyze;
	unsigned char any_function_defined;
	int m_X1,m_Y1,m_X2,m_Y2;
	DWORD ThreadID;
	HANDLE ThreadHandle;
	char TheState;
	int MX,MY; //size of the gray areas

	CFunctionPlotter(CDrawing *BaseItem);
	~CFunctionPlotter(void);

	int Paint(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg);
	int MouseMove(CDC *DC,int X, int Y, UINT flags);
	int CopyFrom(CDrawing *Original);
	int MouseClick(int X, int Y);
	int XML_output(char * output, int num_tabs, char only_calculate);
	char *XML_input(char * file);

	int PlotFunction(int reset_plot, CDC *PrintDC=NULL,short zoom=0);
	int PlotFunctionGetBondaries(double *Xmin, double *Xmax, double *Ymin, double *Ymax);
	int ShowNumberWithPrecision(double number, double precision,char *string);
};

class CBitmapImage
{
public:
	CDrawing *Base; //this is a pointer to main document item that cointains Drawing box frame and other data
	char *Image;
	int imgsize;
	unsigned char ShowMenu,SelectedItem;
	int MenuX,MenuY;
	char editing;

	CBitmapImage(CDrawing *BaseItem);
	~CBitmapImage(void);

	int Paint(CDC * DC, short zoom, short X, short Y,int absX,int absY,RECT *ClipReg);
	int MouseMove(CDC *DC,int X, int Y, UINT flags);
	int CopyFrom(CDrawing *Original);
	int MouseClick(int X, int Y);
	int XML_output(char * output, int num_tabs, char only_calculate);
	char *XML_input(char * file);

	int LoadImageFromFile(CObject *dwg, char *fname);
	int SaveImageToFileForEditing(CObject *dwg);
};