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
#include "expression.h"
#include "MathomirDoc.h"
#include ".\MathomirDoc.h"
#include "toolbox.h"
#include "mainfrm.h"
#include "drawing.h"
#include "diffiehellman.h"
#include "passworddlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int RandomNumber;
char XMLFileVersion;

#include "MathomirView.h"
extern CMathomirView *pMainView;
extern CToolbox *Toolbox;
extern DWORD AutosaveTime;
extern DWORD AutosavePoints;

extern void DetermineTillensData(int mouseY);
extern CExpression* TheKeyboardClipboard;

IMPLEMENT_DYNCREATE(CMathomirDoc, CDocument)

BEGIN_MESSAGE_MAP(CMathomirDoc, CDocument)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
END_MESSAGE_MAP()


CMathomirDoc::CMathomirDoc()
{
	RandomNumber=GetTickCount()%99999; //used to name clipboard data during exam session (to prevent copying between two applications)
}


BOOL CMathomirDoc::OnNewDocument()
{
	AutosavePoints=AutosaveTime=0;
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}


#ifdef _DEBUG
void CMathomirDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMathomirDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


#pragma optimize("s",on)
BOOL CMathomirDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	//called by CMathomirDoc whenever needs to save document
	SaveMOMFile((char*)lpszPathName,TheFileType);
	return 1;
}

#pragma optimize("s",on)
BOOL CMathomirDoc::SaveModified()
{	
	//called by CMathomirDoc whenever a modifed document needs to be closed (should return 0 if not to be closed)
	if (!IsModified()) return 1;
	if (NumDocumentElements==0) return 1;

	char str[120];
	CopyTranslatedString(str,"Save changes?",5020,119);
	int ret=AfxMessageBox(str,MB_YESNOCANCEL | MB_ICONQUESTION,NULL);
	if (ret==IDCANCEL) return 0; //should not be clossed
	if (ret==IDYES) OnFileSave(); //save the document
	return 1;
}

#pragma optimize("s",on)
void CMathomirDoc::OnFileSave()
{
	if (TheFileType=='v') return; //the view-only files are not saved (but can be saved using Save-As)

	//main menu: Save
	CString path;
	path=GetPathName();

	//if the document does not have a name (file) yet, call the save-as dialog
	if ((path!="") && (path.Right(9)!="\\Untitled"))
		SaveMOMFile(path.GetBuffer(),TheFileType);
	else
		OnFileSaveAs();
}

#pragma optimize("s",on)
void CMathomirDoc::OnFileOpen()
{
	DetermineTillensData(0x7FFFFFFF); //to prevend crash when a document is double-clicked to load
	char str[120];
	if (!SaveModified()) return;
	int i=0;
	char filter[120];
	CopyTranslatedString(str,"MOM files",5010,40);
	strcpy(filter,str);strcat(filter,"|*.mom|");
	CopyTranslatedString(str,"All files",5011,40);
	strcat(filter,str);strcat(filter,"|*.*||\0");
	CFileDialog fd(TRUE,"mom",NULL,OFN_HIDEREADONLY,filter,theApp.m_pMainWnd,0);
	if (fd.DoModal()==IDOK)
	{
		OpenMOMFile(fd.m_pOFN->lpstrFile);
	}
	IsDrawingMode=0;
	pMainView->RepaintTheView(1);
}

#pragma optimize("s",on)
void CMathomirDoc::OnFileNew()
{
	DetermineTillensData(0x7FFFFFFF); //to prevend crash when a shell command is used
	if (!SaveModified()) return;
	AutosavePoints=AutosaveTime=0;
	((CMainFrame*)(theApp.m_pMainWnd))->ClearDocument();
	SetPathName("\\Untitled",0); //ClearPathName does not exist??
	SetModifiedFlag(0);
	Toolbox->LoadSettings(NULL);
	ViewOnlyMode=0;
	IsDrawingMode=0;
	ToolboxSize=(BaseToolboxSize)?BaseToolboxSize:60;
	if ((theApp.m_pMainWnd) && (theApp.m_pMainWnd->IsWindowVisible()))
		if (Toolbox) Toolbox->ShowWindow(SW_SHOW);
	((CMainFrame*)(theApp.m_pMainWnd))->UndoInit();
	((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
	pMainView->AdjustPosition();
	pMainView->RepaintTheView(1);
	Toolbox->InvalidateRect(NULL,1);
}

#pragma optimize("s",on)
void CMathomirDoc::OnFileSaveAs()
{
	char str[41];
	char filter[200];
	int i=0;
#ifdef TEACHER_VERSION
	if (TheFileType=='r')
	{
		CopyTranslatedString(str,"MOM exam result file",5005,40);
		strcpy(filter,str);strcat(filter,"|*.mom|All files|*.*||\0");
	}
	else
	{
		CopyTranslatedString(str,"MOM file (1.x)",5000,40);strcpy(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM file (2.x)",5001,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM encrypted file",5002,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM exam file",5003,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM view-only file",5004,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"All files",5011,40);strcat(filter,str);strcat(filter,"|*.*||\0");
	}
#else
		CopyTranslatedString(str,"MOM file",5000,40);strcpy(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM compressed file",5001,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM encrypted file",5002,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM exam file",5003,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"MOM view-only file",5004,40);strcat(filter,str);strcat(filter,"|*.mom|");
		CopyTranslatedString(str,"All files",5011,40);strcat(filter,str);strcat(filter,"|*.*||\0");
#endif
	CFileDialog fd(FALSE,"mom",GetTitle(),0,filter,theApp.m_pMainWnd,0);
	if (TheFileType=='2') fd.m_pOFN->nFilterIndex=2;
	else if (TheFileType=='s') fd.m_pOFN->nFilterIndex=3;
	else if (TheFileType=='e') fd.m_pOFN->nFilterIndex=4;
	else fd.m_pOFN->nFilterIndex=1;
	if (fd.DoModal()==IDOK)
	{
		char type=0;
		if (fd.m_pOFN->nFilterIndex==1) type=0;
		if (fd.m_pOFN->nFilterIndex==2) type='2';
		if (fd.m_pOFN->nFilterIndex==3) type='s';
		if (fd.m_pOFN->nFilterIndex==4) type='e';
		if (fd.m_pOFN->nFilterIndex==5) type='v';
		if ((fd.m_pOFN->nFilterIndex==1) && (TheFileType=='r')) type='r';
		SaveMOMFile((char*)fd.m_pOFN->lpstrFile,type);
		if (fd.m_pOFN->nFilterIndex!=4)
		{
			SetPathName(fd.m_pOFN->lpstrFile,1);
			SetTitle(fd.m_pOFN->lpstrFile);
			SetModifiedFlag(0);
		}
	}
}

#pragma optimize("s",on)
BOOL CMathomirDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DetermineTillensData(0x7FFFFFFF);

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE; //the file will be removed from the list of recent files

	theApp.m_nCmdShow=-1; //tricking the MFC not to resize window
	IsDrawingMode=0;
	OpenMOMFile((char*)lpszPathName);

	CString name=GetPathName();
	strcpy((char*)lpszPathName,name);
	return TRUE;
}

//opens MOM file from disk or clipboard (reads from clipboard if filename==NULL)
extern char dont_empty_clipboard;
#pragma optimize("s",on)
int CMathomirDoc::OpenMOMFile(char * filename) 
{
	XMLFileVersion=(filename)?1:2;
	int OrigNumElements;
	char *file_buffer=NULL;
	char *file_pointer;
	FILE *fil=NULL;
	HANDLE clipb_data=NULL;
	int len;

	if (filename!=NULL)
	{
		//reading from file
		fil=fopen(filename,"r+b");
		if (fil==NULL) 
		{
			char str[280];
			CopyTranslatedString(str,"Cannot open file!",5030,79);
			strcat(str,"\r\n(");
			strcat(str,filename);
			strcat(str,")");
			AfxMessageBox(str,MB_OK|MB_ICONWARNING,NULL);
			return 0;
		}
		fseek(fil,0,SEEK_END);
		len=ftell(fil);
		fseek(fil,0,SEEK_SET);
		AutosavePoints=AutosaveTime=0;
	}
	else
	{
		//reading from clipboard

		//first deselect everything 
		//(when reading from clipboard we do not delete previous content, but simply adding to it)
		int i;
		for (i=0;i<NumDocumentElements;i++)
		{
			if (TheDocument[i].MovingDotState!=5)
				TheDocument[i].MovingDotState&=0x80; //clear everything except msb

			if (!dont_empty_clipboard)
			if (TheDocument[i].Object)
			{
				if (TheDocument[i].Type==1)
					((CExpression*)(TheDocument[i].Object))->DeselectExpressionExceptKeyboardSelection();
				if (TheDocument[i].Type==2)
					((CDrawing*)(TheDocument[i].Object))->SelectDrawing(0);
			}
		}

		//opening the clipobard and accessing data
		if ((theApp.m_pMainWnd) && (theApp.m_pMainWnd->OpenClipboard()))
		{
			UINT format;
#ifdef TEACHER_VERSION
			if (TheFileType=='r')
			{
				char ff[32];
				sprintf(ff,"MATHOMIR_%d",RandomNumber);
				format=RegisterClipboardFormat(ff);
			}
			else
#endif
			{
				format=RegisterClipboardFormat("MATHOMIR_DOC");
			}
			clipb_data=GetClipboardData(format);
			if (clipb_data==NULL) {CloseClipboard();return 0;}
			len=(int)GlobalSize(clipb_data);
		}
		else 
			return 0;
	}

	//reserve memory where file will be loaded
	file_buffer=(char*)malloc(len+1);
	if (file_buffer==NULL) 
	{
		if (filename==NULL) CloseClipboard();
		return 0; //failed
	}

	if (filename!=NULL)
	{
		//clear the entire documment
		((CMainFrame*)(theApp.m_pMainWnd))->ClearDocument();

		//get data from file
		fread(file_buffer,len,1,fil);
		fclose(fil);
		len=UnscrambleMOMFile(&file_buffer,len);
		//check if file is valid
		int i=0;
		for (i=0;i<len;i++)
			if (((unsigned char)file_buffer[i])>32) break;
		if ((i==len) || (file_buffer[i]!='<'))
		{
			if (len) AfxMessageBox("Invalid file - cannot continue",MB_OK | MB_ICONSTOP);
			SetPathName("\\Untitled",0);
			SetTitle("Untitled");
			SetModifiedFlag(0);
			free(file_buffer);
			return 0;
		}

		//set new document name
		SetPathName(filename,1);
		SetTitle(filename);

		//adjust menu and init undo buffer
		if (theApp.m_pMainWnd)
		{
			((CMainFrame*)(theApp.m_pMainWnd))->UndoInit();
			((CMainFrame*)(theApp.m_pMainWnd))->AdjustMenu();
		}
	}
	else
	{
		//gata from clipboard
		LPVOID pntr=GlobalLock(clipb_data);
		if (pntr==NULL){CloseClipboard();return 0;}
		memcpy(file_buffer,pntr,len);
		GlobalUnlock(clipb_data);
		CloseClipboard();

		OrigNumElements=NumDocumentElements;
	}

	file_pointer=file_buffer;
	file_pointer[len]=0;

	//parsing file  -  object by object  -  the MOM file has XML structure
	{
		int type=0;
		int x=0,y=0;
		while (1)
		{
			file_pointer=((CMainFrame*)(theApp.m_pMainWnd))->XML_search("",file_pointer);
			if (file_pointer==0) goto openMOMfile_end; //no more objects, we finished

			if (*file_pointer=='o')
			{
				try
				{
					file_pointer++;while ((*file_pointer!=' ') && (*file_pointer!='>')) file_pointer++; //jumps over 'obj' or 'o' tags

					int lock=0;
					char attribute[128];
					char value[128];
					do 
					{
						file_pointer=((CMainFrame*)(theApp.m_pMainWnd))->XML_read_attribute(attribute,value,file_pointer,128);
						if (file_pointer==0) goto openMOMfile_end;  //unexpected end of file

						if ((strcmp(attribute,"type")==0) || (strcmp(attribute,"t")==0)) type=atoi(value);
						if (strcmp(attribute,"ver")==0) 
						{
							XMLFileVersion=atoi(value);
							if (filename) if (XMLFileVersion>1) TheFileType='2';
						}
						if (strcmp(attribute,"X")==0) x=atoi(value);
						if (strcmp(attribute,"Y")==0) y=atoi(value);
						if (strcmp(attribute,"lock")==0) lock=atoi(value);
						if ((filename) && (strcmp(attribute,"page_w")==0)) PaperWidth=atoi(value);
						if ((filename) && (strcmp(attribute,"page_h")==0)) PaperHeight=atoi(value);
						if ((filename) && (strcmp(attribute,"numbering")==0)) PageNumeration=atoi(value);
					} while (attribute[0]);

					if (type)
					{
						if (AddDocumentObject(type,x,y)==0) goto openMOMfile_end; //irregular end!
						tDocumentStruct *ds=TheDocument+NumDocumentElements-1;
						ds->MovingDotState=(lock)?5:1;

						if (type==1) //object of type==1 - the expression
						{
							CExpression *exp=new CExpression(NULL,NULL,100);
							if (!exp) {NumDocumentElements--;goto openMOMfile_end;} //irregular end!
							ds->Object=(CObject*)exp;

							//calling the CExpression for parsing the object
							file_pointer=exp->XML_input(file_pointer);
							if (file_pointer==NULL) {NumDocumentElements--;delete exp;goto openMOMfile_end;} //irregular end!
						}

						if (type==2) //object of type==2 - the drawing
						{
							CDrawing *drw=new CDrawing();
							if (!drw) {NumDocumentElements--;goto openMOMfile_end;} //irregular end!
							ds->Object=(CObject*)drw;

							//calling the CExpression for parsing the object
							file_pointer=drw->XML_input(file_pointer);
							if (file_pointer==NULL) {NumDocumentElements--;delete drw;goto openMOMfile_end;} //irregular end!
						}

						//calculate size of the drawing
						//(when started with given filename in command line, the main window will not be yet created
						// so it is not possible to call GetDC)
						if (theApp.m_pMainWnd) 
						{
							CDC *DC=theApp.m_pMainWnd->GetDC();
							short l=0,a=0,b=0;
							if (type==1) 
								((CExpression*)ds->Object)->CalculateSize(DC,ViewZoom,&l,&a,&b); 
							else
								((CDrawing*)ds->Object)->CalculateSize(DC,ViewZoom,&l,&b);
							ds->Length=l*100/ViewZoom;
							ds->Above=a*100/ViewZoom;
							ds->Below=b*100/ViewZoom;
							if (ds->MovingDotState!=5) ds->MovingDotState=(filename==NULL)?3:0; //automaticaly select object pasted from clipboard
							theApp.m_pMainWnd->ReleaseDC(DC);
						}
					}
				}
				catch(...)
				{
				}
			}
		}
	}

openMOMfile_end:
	if (filename==NULL) //paste operation
	{
		if (TheKeyboardClipboard) {delete TheKeyboardClipboard;TheKeyboardClipboard=NULL;}
		if ((NumDocumentElements-OrigNumElements==1) &&
			(TheDocument[OrigNumElements].Type==1) &&
			(KeyboardEntryObject))
		{
			//we are pasting into the keyboard clipboard			
			TheKeyboardClipboard=new CExpression(NULL,NULL,100);
			CExpression *exp=(CExpression*)TheDocument[OrigNumElements].Object;
			TheKeyboardClipboard->CopyExpression(exp,0);
			delete exp;
			NumDocumentElements--;
		}
		else
		{
			//adjusting position of objects pasted from clipboard (depending on the mouse pointer position)
			int i;
			int MinX=0x7FFFFFFF,MinY=0x7FFFFFFF;
			int MaxX=-MinX;
			int MaxY=-MinY;
			tDocumentStruct *ds=TheDocument+OrigNumElements;
			for (i=OrigNumElements;i<NumDocumentElements;i++,ds++)
			{
				if (ds->absolute_X<MinX) MinX=ds->absolute_X;
				if (ds->absolute_Y-ds->Above<MinY) MinY=ds->absolute_Y-ds->Above;
				if (ds->absolute_X+ds->Length>MaxX) MaxX=ds->absolute_X+ds->Length;
				if (ds->absolute_Y+ds->Below>MaxY) MaxY=ds->absolute_Y+ds->Below;
			}
			POINT p;
			GetCursorPos(&p);
			RECT r;
			pMainView->ScreenToClient(&p);
			pMainView->GetClientRect(&r);
			if (p.x<0) p.x=0;
			if (p.y<0) p.y=0;
			if (p.x>r.right) p.x=r.right;
			if (p.y>r.bottom) p.y=r.bottom;
			p.x=p.x*100/ViewZoom+ViewX;
			p.y=p.y*100/ViewZoom+ViewY;

			int DeltaX=(MinX+MaxX)/2-p.x;
			int DeltaY=(MinY+MaxY)/2-p.y;
			for (i=OrigNumElements;i<NumDocumentElements;i++)
			{
				TheDocument[i].absolute_X-=DeltaX;
				TheDocument[i].absolute_Y-=DeltaY;
			}
		}
	}

	//clear memory
	if (file_buffer) free(file_buffer);

	if (filename) 
	{
		SetModifiedFlag(0);
#ifdef TEACHER_VERSION
		if (TheFileType=='r')
		{
			//if an exam was loaded, we change the name so that the file does not accidentaly overwrite the original exam file
			SetTitle("Untitled");
			SetPathName("Untitled");
		}
#endif
	}
	else
		SetModifiedFlag(1); //when pasting 

	if (!dont_empty_clipboard) //this flag is set if it is called from OnActivate()
	{
		if (theApp.m_pMainWnd)
		{
			pMainView->RepaintTheView(0);
			if ((!ViewOnlyMode) && (filename)) DetermineTillensData(-1);

			if (filename)
			{
				Toolbox->AdjustPosition();
				pMainView->AdjustPosition();
			}
			((CMainFrame*)theApp.m_pMainWnd)->AdjustMenu();
		}
		else 
			if (ViewOnlyMode) ViewOnlyMode=2; //we don't even show the menu

	}

	return 1;
}

// saves into MOM file or clipboard
// (if filename==NULL the saves into clipboard)
#pragma optimize("s",on)
int CMathomirDoc::SaveMOMFile(char * filename,char filetype)
{
	if ((filetype=='2') || (filename==NULL)) XMLFileVersion=2; else XMLFileVersion=1;
	char dummy[128];
	int len=0;
	int i;
	char save_keyboard_clipboard=0;
	if ((filename==(char*)TheKeyboardClipboard) &&
		(TheKeyboardClipboard))
		{filename=NULL;save_keyboard_clipboard=1;TheFileType=0;}

#ifdef TEACHER_VERSION
	if ((filename) && (TheFileType=='r'))
	{
		char tmpbuf[128];
		CExpression *exp=new CExpression(NULL,NULL,80);
		int maxy=0;
		for (int jj=0;jj<NumDocumentElements;jj++)
			if (TheDocument[jj].absolute_Y+TheDocument[jj].Below>maxy)
				maxy=TheDocument[jj].absolute_Y+TheDocument[jj].Below;
		AddDocumentObject(1,20,maxy+30);
		TheDocument[NumDocumentElements-1].Object=(CObject*)exp;
		exp->InsertEmptyElement(0,1,'-');
		exp->InsertEmptyElement(1,1,'-');
		exp->InsertEmptyElement(2,11,0);
		exp->InsertEmptyElement(3,1,'-');
		exp->InsertEmptyElement(4,11,0);
		exp->InsertEmptyElement(5,1,'-');
		exp->m_ParentheseShape='b';
		exp->m_ParenthesesFlags=0x01;
		exp->m_Color=3;
		SYSTEMTIME st;
		GetLocalTime(&st);
		sprintf(tmpbuf,"%04d-%02d-%02d",st.wYear,st.wMonth,st.wDay);
		strcpy((exp->m_pElementList+0)->pElementObject->Data1,tmpbuf);
		sprintf(tmpbuf,"%02d:%02d:%02d",st.wHour,st.wMinute,st.wSecond);
		strcpy((exp->m_pElementList+1)->pElementObject->Data1,tmpbuf);
		DWORD lnn=23;
		GetUserName(tmpbuf,&lnn);
		strcpy((exp->m_pElementList+3)->pElementObject->Data1,tmpbuf);
		lnn=23;
		GetComputerName(tmpbuf,&lnn);
		strcpy((exp->m_pElementList+5)->pElementObject->Data1,tmpbuf);
	}
#endif


	//first, calculate briefly the length of the output data

	if (save_keyboard_clipboard)
		len+=((CExpression*)TheKeyboardClipboard)->XML_output(dummy,1,1);
	else
	{
		tDocumentStruct *ds=TheDocument;
		for (i=0;i<NumDocumentElements;i++,ds++)
		{
			try
			{
				if ((filename) || (ds->MovingDotState==3))
				{
					if (ds->Type==1) len+=((CExpression*)(ds->Object))->XML_output(dummy,0,1);
					else if (ds->Type==2) len+=((CDrawing*)(ds->Object))->XML_output(dummy,0,1);
				}
			}
			catch(...)
			{

			}
		}
	}


	//reserve memory for output buffer
	char *file_buffer;
	char *file_pointer;
	int alloc_len=len+1024+NumDocumentElements*64;
	file_buffer=(char*)malloc(alloc_len);
	if (file_buffer==0)
	{
		if (filename) AfxMessageBox("Cannot reserve memory for file saving!",MB_OK|MB_ICONWARNING,NULL);
		return 0;
	}
	file_pointer=file_buffer;


	//start outputing data into output buffer
	len=0;
	strcpy(file_buffer,"<?xml version=\"1.0\"?>\r\n<mathomir>\r\n");
	file_pointer+=strlen(file_buffer);
	len+=(int)strlen(file_buffer);


	if (save_keyboard_clipboard)
	{
			int tmp;
			sprintf(file_pointer,"<obj type=\"1\" X=\"0\" Y=\"0\">\r\n");				
			tmp=(int)strlen(file_pointer);
			file_pointer+=tmp;
			len+=tmp;

			tmp=0;
			tmp=((CExpression*)TheKeyboardClipboard)->XML_output(file_pointer,1,0);
			len+=tmp;
			file_pointer+=tmp;

			strcpy(file_pointer,"</obj>\r\n");
			file_pointer+=8;
			len+=8;
	}
	else
	{
		tDocumentStruct *ds=TheDocument;
		int type=0, x=0, y=0;
		for (i=0;i<NumDocumentElements;i++,ds++)
		{
			try
			{
				if (((filename) || (ds->MovingDotState==3)) && (ds->absolute_Y<2000000) && (ds->absolute_Y>-10000))
				{
					if (XMLFileVersion==1)
					{
						type=0; x=0; y=0;
						strcpy(file_pointer,"<obj");file_pointer+=4;len+=4;
						strcpy(file_pointer," type=\"1\"");file_pointer[7]=ds->Type+'0';file_pointer+=9;len+=9;
					}
					else
					{
						strcpy(file_pointer,"<o");file_pointer+=2;len+=2;
						if (type!=ds->Type) {type=ds->Type;strcpy(file_pointer," t=\"1\"");file_pointer[4]=type+'0';file_pointer+=6;len+=6;}
					}
					if (x!=ds->absolute_X) {x=ds->absolute_X;sprintf(dummy," X=\"%d\"",x);int ll=(int)strlen(dummy);memcpy(file_pointer,dummy,ll+1);file_pointer+=ll;len+=ll;}
					if (y!=ds->absolute_Y) {y=ds->absolute_Y;sprintf(dummy," Y=\"%d\"",y);int ll=(int)strlen(dummy);memcpy(file_pointer,dummy,ll+1);file_pointer+=ll;len+=ll;}
					if (ds->MovingDotState==5) {strcpy(file_pointer," lock=\"1\"");file_pointer+=9;len+=9;}
					if (i==0) 
					{
						//only the first object has some global attributs
						sprintf(dummy," page_w=\"%d\" page_h=\"%d\" numbering=\"%d\" ver=\"%d\"",PaperWidth,PaperHeight,PageNumeration,XMLFileVersion);
						int ll=(int)strlen(dummy);memcpy(file_pointer,dummy,ll+1);file_pointer+=ll;len+=ll;
					}
					strcpy(file_pointer,">\r\n");file_pointer+=3;len+=3;

					int tmp=0;
					if (ds->Type==1)
						tmp=((CExpression*)(ds->Object))->XML_output(file_pointer,0,0);
					else
						tmp=((CDrawing*)(ds->Object))->XML_output(file_pointer,0,0);
					len+=tmp;
					file_pointer+=tmp;

					if (XMLFileVersion==1)
					{
						strcpy(file_pointer,"</obj>\r\n");file_pointer+=8;len+=8;
					}
					else
					{
						strcpy(file_pointer,"</o>\r\n");file_pointer+=6;len+=6;
					}
				}
			}
			catch(...)
			{
			}
		}
	}

	strcpy(file_pointer,"</mathomir>\r\n");
	file_pointer+=13;
	len+=13;

#ifdef TEACHER_VERSION
	if ((TheFileType=='r') && (filename))
	{
		NumDocumentElements--;
		delete ((CExpression*)(TheDocument[NumDocumentElements].Object));
	}
#endif
	if (filename)
	{
		//encrypt, compress if necessary
		len=ScrambleMOMFile(&file_buffer,len,filetype);
		if (len==0) {free(file_buffer); return 0;}
		//save to file
		FILE *fil;
		fil=fopen(filename,"w+b");
		if (fil)
		{
			fwrite(file_buffer,len,1,fil);
			fclose(fil);
			if (strcmp(filename,"autosave.mom")!=0) SetModifiedFlag(0);
			AutosavePoints=AutosaveTime=0;
		}
		else
		{
			char str[80];
			CopyTranslatedString(str,"Cannot create file!",5031,79);
			AfxMessageBox(str,MB_OK|MB_ICONWARNING,NULL);
		}
	}
	else
	{
		//save to clipboard

		if ((theApp.m_pMainWnd->OpenClipboard()) && (EmptyClipboard()))
		{
			HANDLE hmem=GlobalAlloc(GMEM_ZEROINIT,alloc_len);
			LPVOID pntr=GlobalLock(hmem);
			memcpy(pntr,file_buffer,alloc_len);
			GlobalUnlock(hmem);
			UINT format;
#ifdef TEACHER_VERSION
			if (TheFileType=='r')
			{
				char ff[32];
				sprintf(ff,"MATHOMIR_%d",RandomNumber);
				format=RegisterClipboardFormat(ff);
			}
			else
#endif	
			{
				format=RegisterClipboardFormat("MATHOMIR_DOC");
			}
			HANDLE ret=SetClipboardData(format,hmem);
			CloseClipboard();
		}
	}
	free(file_buffer);
	return 1;
}



#ifdef TEACHER_VERSION
tPublicKey *PublicKey;
unsigned char TheTimeLimit;
unsigned char TheMathFlags;
DWORD TheExamStartTime;
unsigned char DisableEditing;
unsigned char WarningDisplayed;
#endif
char TheFileType;
tPasswordDlgStruct *PasswordDlgStruct;

//this function compresses and/or scrambles the xml file
#pragma optimize("s",on)
int CMathomirDoc::ScrambleMOMFile(char **buffer, int len, char type)
{
	if ((type==0) || (type=='2')) {TheFileType=0; return len;} //standard file type - no need for compression/encryption

	char passw[25];
	passw[0]=0;
	if ((type=='s') || (type=='e'))	 
	{
		//encrypted MOM file or exam file - ask for password
		PasswordDlgStruct=new tPasswordDlgStruct;
		PasswordDlgStruct->is_exam=0;
		if (type=='e')
		{
			PasswordDlgStruct->is_exam=1;
			PasswordDlgStruct->disable_math=0;
			PasswordDlgStruct->disable_symbolic_math=1;
			PasswordDlgStruct->time_limit=30;
		}
		CPasswordDlg *psw;
		psw=new CPasswordDlg(theApp.m_pMainWnd);
		psw->DoModal();
		delete psw;
		if (PasswordDlgStruct->canceled) {delete PasswordDlgStruct;PasswordDlgStruct=NULL;return 0;}
		strcpy(passw,PasswordDlgStruct->password);
	}

	
	int olen=len;
	char *buf=*buffer;

	//step 1. remove spaces and change common strings with shortcuts
	int j=0;
	int inside_quotation=0;
	for (int i=0;i<len;i++)
	{
		char c=buf[i];
		if (c=='"') 
		{
			if (inside_quotation) inside_quotation=0; else inside_quotation=1;
		}
		if (inside_quotation==0)
		{
			if (c==9) continue;
			/*if (strncmp(buf+i,"<obj type=",10)==0) {i+=10-1;c=1;}
			else if (strncmp(buf+i,"</obj>",6)==0) {i+=6-1;c=2;}
			else if (strncmp(buf+i,"<draw type=",11)==0) {i+=11-1;c=3;}
			else if (strncmp(buf+i,"<group",6)==0) {i+=6-1;c=4;}
			else if (strncmp(buf+i,"</group>\r\n",10)==0) {i+=10-1;c=5;}
			else if (strncmp(buf+i," width=",7)==0) {i+=7-1;c=6;}
			else if (strncmp(buf+i,"<expr fnt_h=",12)==0) {i+=12-1;c=8;}
			else if (strncmp(buf+i," brack=",7)==0) {i+=7-1;c=9;}
			else if (strncmp(buf+i,"<elm tp=\"1\" tx=",15)==0) {i+=15-1;c=11;}
			else if (strncmp(buf+i,"<elm tp=",8)==0) {i+=8-1;c=12;}
			else if (strncmp(buf+i," fnt=",5)==0) {i+=5-1;c=14;}
			else if (strncmp(buf+i,"</expr>\r\n",9)==0) {i+=9-1;c=15;}
			else if (strncmp(buf+i,"</elm>\r\n",8)==0) {i+=8-1;c=31;}
			else if (strncmp(buf+i," stp=",5)==0) {i+=5-1;c=16;}
			else if (strncmp(buf+i,">\r\n",3)==0) {i+=3-1;c=17;}
			else if (strncmp(buf+i," txt=",5)==0) {i+=5-1;c=19;}
			else if (strncmp(buf+i," alig=",6)==0) {i+=6-1;c=20;}
			else if (strncmp(buf+i," Exp1=",6)==0) {i+=6-1;c=21;}
			else if (strncmp(buf+i," Exp2=",6)==0) {i+=6-1;c=22;}
			else if (strncmp(buf+i," X=",3)==0) {i+=3-1;c=23;}
			else if (strncmp(buf+i," Y=",3)==0) {i+=3-1;c=24;}
			else if (strncmp(buf+i," X1=",4)==0) {i+=4-1;c=25;}
			else if (strncmp(buf+i," Y1=",4)==0) {i+=4-1;c=26;}
			else if (strncmp(buf+i," X2=",4)==0) {i+=4-1;c=27;}
			else if (strncmp(buf+i," Y2=",4)==0) {i+=4-1;c=28;}
			else if (strncmp(buf+i," X3=",4)==0) {i+=4-1;c=29;}
			else if (strncmp(buf+i," Y3=",4)==0) {i+=4-1;c=30;}
			else if (strncmp(buf+i,"<bmp bits=",10)==0)
			{
				i+=10-1;c=(char)251;}
			else if (strncmp(buf+i," fnthq_h=",9)==0) {i+=9-1;c=(char)252;}
			else if (strncmp(buf+i," b_shape=",9)==0) {i+=9-1;c=(char)255;}
			else if (strncmp(buf+i," b_height=",10)==0) {i+=10-1;c=(char)254;}
			else if (strncmp(buf+i," b_data=",8)==0) {i+=8-1;c=(char)253;}*/
		}
		else if (c!='"')
		{
			{
				int k=0;
				while (k<200)
				{
					if ((buf[i+k]!=buf[i+k+2]) || (buf[i+k+1]!=buf[i+k+3])) break;
					k+=2;
				}
				if (k>5)
				{
					buf[j++]=18;
					buf[j++]=(k+1)/2;
					buf[j++]=buf[i];
					buf[j++]=buf[i+1];
					i+=k+1;
					continue;
				}
			}
			if (strncmp(buf+i,"000\"",4)==0) {i+=4-1;c=7;inside_quotation=0;}
		}	
		buf[j++]=c;
	}
	len=j;



#ifdef TEACHER_VERSION
	unsigned char *pkey=NULL;
	if ((type=='r') || (type=='e'))
	{
		//if this is an exam or exam result
		pkey=(unsigned char*)malloc(1026);
		CDiffieHellman *DH=new CDiffieHellman();
		__int64 N,X,Y,key;

		if (type=='e') 
		{
			for (int i=0;i<64;i++) 
			{
				DH->DerivePublicKey(passw,&N,&X); 
				(*(__int64 *)(pkey+16*i))=N;
				(*(__int64 *)(pkey+16*i+8))=X;
			}
			passw[0]=0;
			pkey[1024]=PasswordDlgStruct->time_limit;
			pkey[1025]=PasswordDlgStruct->disable_math+2*PasswordDlgStruct->disable_symbolic_math;
		}
		else
		{
			memset(passw,0,24);
			for (int i=0;i<64;i++) 
			{
				DH->CreateEncryptionKey(PublicKey[i].N,PublicKey[i].X,&key,&Y); 
				(*(__int64 *)(pkey+16*i))=PublicKey[i].N;
				(*(__int64 *)(pkey+16*i+8))=Y;
				(*(__int64 *)(&passw[i%16]))+=key;
			}
			for (int i=0;i<16;i++) if (passw[i]==0) passw[i]=1;
			passw[16]=0;
		}
		delete DH;
	}
#endif


	//scramble with password
	if ((passw) && (strlen(passw)>=1))
	{
		int passlen=(int)strlen(passw);
		
		//add some random numbers
		unsigned int random_numbers=(unsigned int)((unsigned int)((GetTickCount())%64)+passlen);
		char *buf2;
		buf2=(char*)malloc(len+random_numbers+12+1024+NumDocumentElements*64);
		buf=(char*)malloc(len+random_numbers+12+1024+NumDocumentElements*64);
		len+=random_numbers;
		memcpy(buf+random_numbers,*buffer,len);
		free(*buffer);
		*buffer=buf;

		buf[0]=(char)random_numbers;
		Sleep(10);
		for (int i=1;i<(int)random_numbers;i++)
		{buf[i]=(char)(GetTickCount()%64);buf[i]+=rand();Sleep(10);}

		//hash the text with the rest of the password characters
		for (int j=0;j<passlen;j++)
		{
			int startpointer=0;
			int pointer=0;
			char d=passw[j];
			for (int i=0;i<len;i++)
			{
				unsigned char x=(unsigned char)passw[j];
				buf2[i]=buf[pointer];
				buf2[i]=buf2[i]^x;
				buf2[i]+=d;
				d=buf2[i];

				pointer+=x;
				if (pointer>=len) pointer=++startpointer;
			}
			memcpy(buf,buf2,len);
		}
		free(buf2);
	}

#ifdef TEACHER_VERSION
	if (pkey)
	{
		len+=1026;
		buf=(char*)malloc(len+1024+NumDocumentElements*64+16);
		memcpy(buf+1026,*buffer,len);
		memcpy(buf,pkey,1026);
		free(*buffer);
		*buffer=buf;
		free(pkey);
	}
#endif

	//store the original length at the begining of file
	memmove(buf+8,buf,len);
	strcpy(buf,"MOM");
	buf[3]=type;
	*((int*)(buf+4))=olen;

	if (PasswordDlgStruct)
	{
		delete PasswordDlgStruct;
		PasswordDlgStruct=NULL;
	}

	if ((type!='e') && (type!='v')) TheFileType=type; //change the file type according to saved format
	
	return len+8;
}


//this function compresses the xml file
#pragma optimize("s",on)
int CMathomirDoc::UnscrambleMOMFile(char ** buffer, int len)
{
	ViewOnlyMode=0;
	ToolboxSize=(BaseToolboxSize)?BaseToolboxSize:60;
	if (Toolbox) Toolbox->ShowWindow(SW_SHOW);

	if (strncmp(*buffer,"MOM",3)) {TheFileType=0;return len;} //not a compressed MOM file

	char passw[24];
	passw[0]=0;
	char type=*(*buffer+3);

#ifdef TEACHER_VERSION
	TheTimeLimit=0;
	TheMathFlags=0;
	if (type=='e') 
	{
		//exam file - create a structure to hold the public key
		PublicKey=(tPublicKey*)calloc(sizeof(tPublicKey),64);
	}
#endif
	if ((type=='r') || (type=='s'))
	{
		//exam result file or scrambled (encrypted) file - ask for password
		PasswordDlgStruct=new tPasswordDlgStruct;
		PasswordDlgStruct->is_exam=0;
		CPasswordDlg *psw;
		psw=new CPasswordDlg(theApp.m_pMainWnd);
		psw->DoModal();
		delete psw;
		if (PasswordDlgStruct->canceled) {delete PasswordDlgStruct;PasswordDlgStruct=NULL;return 0;}
		strcpy(passw,PasswordDlgStruct->password);
		delete PasswordDlgStruct;
		PasswordDlgStruct=NULL;
		

	}
	
	int olen=*((int*)(*buffer+4)); //original file length

	//remove header
	len-=8;
	memmove(*buffer,(*buffer+8),len);

#ifdef TEACHER_VERSION
	if ((type=='e') || (type=='r'))
	{
		//if this is an exam or exam result
		CDiffieHellman *DH=new CDiffieHellman();
		__int64 N,Y,key;
	
		if (type=='r')
		{
			DH->DerivePublicKey(passw,&N,NULL);
			memset(passw,0,24);
			for (int i=0;i<64;i++) 
			{
				N=(*(__int64 *)(*buffer+16*i));
				Y=(*(__int64 *)(*buffer+16*i+8));
				DH->CreateDecryptionKey(Y,N,&key); 
				(*(__int64 *)(&passw[i%16]))+=key;
			}
			for (int i=0;i<16;i++) if (passw[i]==0) passw[i]=1;
			passw[16]=0;

		}
		else
		{
			for (int i=0;i<64;i++) 
			{
				PublicKey[i].N=(*(__int64 *)(*buffer+16*i));
				PublicKey[i].X=(*(__int64 *)(*buffer+16*i+8));
			}
			TheTimeLimit=*(unsigned char*)(*buffer+1024);
			TheMathFlags=*(unsigned char*)(*buffer+1025);	
		}
		len-=1026;
		memmove(*buffer,(*buffer+1026),len);
		delete DH;
	}
#endif

	if ((passw) && (strlen(passw)>=1))
	{
		//if password is defined, then we unscramble this file
		char *buf2;
		buf2=(char*)malloc(len+1);

		int passlen=(int)strlen(passw);
		for (int j=passlen-1;j>=0;j--)
		{			
			char d;
			unsigned char x=(unsigned char)passw[j];
			for (int i=0;i<len;i++)
			{
				int startpointer=0;
				int pointer=0;
				int jj=0;
				int num=0;
				for (int kk=0;kk<x;kk++)
				{
					if (i>=kk)
						if (((i-kk)%x)==0) {jj=num+(i-kk)/x;break;}
					num+=(len-kk-1)/x+1;
				}
				if (jj==0) d=x; else d=(*buffer)[jj-1];
				buf2[i]=(*buffer)[jj]-d;
				buf2[i]=buf2[i]^x;
				d=(*buffer)[jj];
			}
			memcpy(*buffer,buf2,len);
		}
		free(buf2);

		//remove random numbers (salt)
		unsigned int num_randoms=(unsigned char)*buffer[0];
		len-=num_randoms;
		if (len<0) len=0;
		memmove(*buffer,*buffer+num_randoms,len);
	}
	
	//we procede by decompressing the file
	char *buf=(char*)malloc(olen+1);
	int j=0;
	int inside_quotation=0;
	for (int i=0;i<len;i++)
	{
		char c=*((*buffer)+i);
		if ((i==0) && (c!='<')) break;
		if (c=='"') 
		{
			if (inside_quotation) inside_quotation=0; else inside_quotation=1;
		}
		if (inside_quotation==0)
		{
			if (c!=13)
			if (c!=10)
			if ((c<31) || (c>250))
				c+=0;
			if (c==1) {strcpy(buf+j,"<obj type=");j+=10;continue;}
			if (c==2) {strcpy(buf+j,"</obj>");j+=6;continue;}
			if (c==3) {strcpy(buf+j,"<draw type=");j+=11;continue;}
			if (c==4) {strcpy(buf+j,"<group");j+=6;continue;}
			if (c==5) {strcpy(buf+j,"</group>\r\n");j+=10;continue;}
			if (c==6) {strcpy(buf+j," width=");j+=7;continue;}
			if (c==8) {strcpy(buf+j,"<expr fnt_h=");j+=12;continue;}
			if (c==9) {strcpy(buf+j," brack=");j+=7;continue;}
			if (c==11) {strcpy(buf+j,"<elm tp=\"1\" tx=");j+=15;continue;}
			if (c==12) {strcpy(buf+j,"<elm tp=");j+=8;continue;}
			if (c==14) {strcpy(buf+j," fnt=");j+=5;continue;}
			if (c==15) {strcpy(buf+j,"</expr>\r\n");j+=9;continue;}
			if (c==31) {strcpy(buf+j,"</elm>\r\n");j+=8;continue;}
			if (c==16) {strcpy(buf+j," stp=");j+=5;continue;}
			if (c==17) {strcpy(buf+j,">\r\n");j+=3;continue;}

			if (c==19) {strcpy(buf+j," txt=");j+=5;continue;}
			if (c==20) {strcpy(buf+j," alig=");j+=6;continue;}
			if (c==21) {strcpy(buf+j," Exp1=");j+=6;continue;}
			if (c==22) {strcpy(buf+j," Exp2=");j+=6;continue;}
			if (c==23) {strcpy(buf+j," X=");j+=3;continue;}
			if (c==24) {strcpy(buf+j," Y=");j+=3;continue;}

			if (c==25) {strcpy(buf+j," X1=");j+=4;continue;}
			if (c==26) {strcpy(buf+j," Y1=");j+=4;continue;}
			if (c==27) {strcpy(buf+j," X2=");j+=4;continue;}
			if (c==28) {strcpy(buf+j," Y2=");j+=4;continue;}
			if (c==29) {strcpy(buf+j," X3=");j+=4;continue;}
			if (c==30) {strcpy(buf+j," Y3=");j+=4;continue;}
			if (c==(char)251) {strcpy(buf+j,"<bmp bits=");j+=10;continue;}
			if (c==(char)252) {strcpy(buf+j," fnthq_h=");j+=9;continue;}
			if (c==(char)255) {strcpy(buf+j," b_shape=");j+=9;continue;}
			if (c==(char)254) {strcpy(buf+j," b_height=");j+=10;continue;}
			if (c==(char)253) {strcpy(buf+j," b_data=");j+=8;continue;}
		}
		else
		{
			if (c==18)
			{
				int k=*((*buffer)+i+1);
				for (int n=0;n<k+1;n++)
				{
					buf[j++]=*((*buffer)+i+2);
					buf[j++]=*((*buffer)+i+3);
				}
				i+=3;
				continue;
			}
			if (c==7) {strcpy(buf+j,"000\"");j+=4;inside_quotation=0;continue;}
		}	
		buf[j++]=c;
	}
	len=j;	
	free(*buffer);
	*buffer=buf;

	if (type=='e') TheFileType='r';
	else if (type=='r') TheFileType=0;
	else TheFileType=type;

	if (TheFileType=='v') {ViewOnlyMode=1;} 
#ifdef TEACHER_VERSION
	if ((len) && (TheFileType=='r')) {TheExamStartTime=GetTickCount();DisableEditing=0;WarningDisplayed=0;}
#endif
	return len;
}
int FatalErrorActivated=0;
void FatalErrorHandling()
{
	if (!FatalErrorActivated)
	{
		FatalErrorActivated=1;
		if (AutosaveOption==0)	pMainView->GetDocument()->SaveMOMFile("Autosave.mom",TheFileType);

		MessageBox(theApp.m_pMainWnd->m_hWnd,"FATAL ERROR!\r\nYou can try to save your work now, then restart the software.\r\nUpon restart, you can also check the 'Autosave.mom' file.","Math-o-mir",MB_ICONSTOP|MB_OK);
	}
}
#pragma optimize("",on)