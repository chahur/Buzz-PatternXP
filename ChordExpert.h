#pragma once

#include "resource.h"
#include "afxwin.h"
#include "ScrollWnd.h"
#include "PatEd.h"

class CEditorWnd;

class CCEGrid : public CScrollWnd
{
	DECLARE_DYNAMIC(CCEGrid)

public:
	
public:
	CCEGrid();
	virtual ~CCEGrid();

	void Create(CWnd *parent);
	virtual void OnDraw(CDC *pDC);
	void OnDrawCell(CDC *pDC, int col, int row);
	void GetCellText(LPSTR sText, int col, int row);
	void RefreshCanvasSize();
	void CursorSelect(int dx, int dy);


protected:
	DECLARE_MESSAGE_MAP()	
public:
	int ColCount;
	int ColWidth;
	int RowCount;
	int RowHeight;
	int CurrentCol;
	int CurrentRow;
	int ChordRow;

	grid_vector NotesGrid;
	row_vector_vector SortedChords;

	BOOL OnKeyDown(UINT nChar);
	CEditorWnd *pew;
	CWnd *pced;

	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);


};

class CChordExpertDialog : public CDialog
{
	DECLARE_DYNAMIC(CChordExpertDialog)

public:
	CChordExpertDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChordExpertDialog();

// Dialog Data
	enum { IDD = IDD_CHORD_EXPERT_DIALOG };

	CEditorWnd *pew;
	CCEGrid ceGrid;
	int CursorRow;
	int BaseOctave;
//	int ChordRow;
	string ChordText;

private:

protected:
	void UpdateWindowSize();
	void SaveDialogPos();
	void InitNotesGrid();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
//	afx_msg void OnCbnEditupdateCombo1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

};