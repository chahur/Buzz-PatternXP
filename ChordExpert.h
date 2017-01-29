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
	void DisplaySelectedChord(int col, int row);


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
	CStatic *labelChord;

	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);


};

#define SORT_UNDEFINED 0
#define SORT_BYNOTE 1
#define SORT_BYDISTANCE 2

#define ARP_WIDTH 150
#define ARP_WIDTH_MIN 80
#define SPLITTER_WIDTH 6
#define GRID_WIDTH_MIN 80



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
	CMICallbacks *pCB;


	int CursorRow;
	int BaseOctave;
	int BaseNote;
//	int ChordRow;
	string ChordText;
	int SortBy;
	CComboBox *comboOctave;
	int ArpWidth;
	int ArpPosX;
	int Current_Cursor;
	int New_Cursor;
	BOOL Move_Splitter;

private:

protected:
	void UpdateWindowSize();
	void SaveDialogPos();
	void InitNotesGrid();
	void InitGrid(int SortType);


	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL CChordExpertDialog::OnKeyDown(UINT nChar);

protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
//	afx_msg void OnCbnEditupdateCombo1();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedUp();
	afx_msg void OnBnClickedDown();
	afx_msg void OnBnClickedClear();
	afx_msg void OnListBoxSelectionChange();
	afx_msg void OnBnClickedSortByNote();
	afx_msg void OnBnClickedSortByDistance();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnPaint();
};
