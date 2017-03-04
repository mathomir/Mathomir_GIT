// EquationDoc.h : interface of the CMathomirDoc class
//


#pragma once

class CMathomirDoc : public CDocument
{
protected: // create from serialization only
	CMathomirDoc();
	DECLARE_DYNCREATE(CMathomirDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	//virtual void Serialize(CArchive& ar);

// Implementation
public:
	//virtual ~CMathomirDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnFileSaveAs();
	afx_msg BOOL SaveModified();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	int OpenMOMFile(char * filename);
	int SaveMOMFile(char * filename,char filetype);
	int ScrambleMOMFile(char ** bufer, int len,char type);
	int UnscrambleMOMFile(char ** bufer, int len);
};


