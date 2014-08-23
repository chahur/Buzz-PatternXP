#pragma once

#include "MachinePattern.h"
#include "ScrollWnd.h"
#include "CursorPos.h"

class CEditorWnd;


// CPatEd

class CPatEd : public CScrollWnd
{
	DECLARE_DYNCREATE(CPatEd)

public:
	static int const WM_MIDI_NOTE = WM_USER + 1;

public:
	CPatEd();
	virtual ~CPatEd();

	void Create(CWnd *parent);
	void SetPattern(CMachinePattern *p);

	void SetPlayPos(MapIntToPlayingPattern &pp, CMasterInfo *pmi);

	virtual void OnDraw(CDC *pDC);

	void PatternChanged();
	void ColumnsChanged();

	CColumn *GetCursorColumn();

	void OnEditCut();
	void OnEditCopy();
	void OnEditPaste();
	void OnEditPasteSpecial(); //BWC
	void OnClearNoteOff(); //BWC
	void OnAddNoteOff(); //BWC
	void OnUpNoteOff(); //BWC
	void OnDownNoteOff(); //BWC
	
	int BeatsInMeasureBar(); //BWC

	void ExportPattern(); //BWC
	void ImportPattern(); //BWC
	void InflatePattern(int delta); //BWC 
	void Humanize(int delta, bool hEmpty);

	bool CanCut();
	bool CanCopy();
	bool CanPaste();

	int GetColumnAtX(int x);
	int GetColumnX(int column);
	
	void InvalidateInTimer() { invalidateInTimer = true; }

	int GetColumnWidth(int column);
	int GetFirstColumnofTrack(int column);
	int GetTrackWidth(int column);
	
	void SelectTrackByNo(int col); //BWC
private:
	void TextToFieldImport(char *txt, CColumn *pc, int irow);
	void DrawColumn(CDC *pDC, int col, int x, COLORREF textcolor, CRect const &clipbox);
	void DrawField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue);
	void DrawGraphicalField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue, COLORREF textcolor);
	void DrawCursor(CDC *pDC);
	
	CRect GetCursorRect(CCursorPos const &p);
	void MoveCursor(CCursorPos newpos, bool killsel = true);
	void MoveCursorDelta(int dx, int dy);
	void Tab();
	void ShiftTab();
	int GetDigitAtX(int x);
	
	int GetRowY(int y);
	void UpdateStatusBar();
	void InvalidateField(int row, int column);
	void InvalidateGroup(int row, int column);
	void EditNote(int note, bool canplay = true);
	void EditOctave(int oct);
	void EditByte(int n);
	void EditWord(int n);
	void EditSwitch(int n);
	void Clear();
	void Insert();
	void Delete();
	void Home();
	void HomeOld();//BWC
	void HomeTop();//BWC
	void EndBottom();//BWC
	void End();
	CRect GetSelRect();
	CRect GetSelOrCursorRect();
	CRect GetSelOrAll();
	void KillSelection();
	void CursorSelect(int dx, int dy);
	void SelectAll(); //BWC
	void SelectTrack(); //BWC
	
	bool InSelection(int row, int column);
	void Randomize();

	void Interpolate(bool expintp);
	void ShiftValues(int delta);
	void WriteState();
	void MuteTrack();
	CCursorPos GetDigitAtPoint(CPoint p);
	void PlayRow(bool allcolumns);
	void OldSelect(bool start);
	void Rotate(bool reverse);
	void ToggleGraphicalMode();
	COLORREF GetFieldBackgroundColor(CMachinePattern *ppat, int row, int col, bool muted);
	void Draw(CPoint point);
	void ImportOld();

	COLORREF bgcol;
	COLORREF bgcoldark;
	COLORREF bgcolvdark;
	COLORREF bgsel;

	int mouseWheelAcc;
	int cursorStep;

	bool selection;
	CPoint selStart;
	CPoint selEnd;

	CMachinePattern *pPlayingPattern;
	int playPos;
	int drawPlayPos;

	CCursorPos mouseSelectStartPos;
	bool mouseSelecting;
	bool persistentSelection;

	enum SelectionMode { column, track, group, all };
	SelectionMode selMode;

	bool invalidateInTimer;

	bool drawing;
	int drawColumn;

	static MapIntToIntVector clipboard;
	static int clipboardRowCount;
	static bool clipboardPersistentSelection;
	static SelectionMode clipboardSelMode;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	CCursorPos cursor;

	CEditorWnd *pew;
	CMICallbacks *pCB;
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	afx_msg LRESULT OnMidiNote(WPARAM wParam, LPARAM lParam);

};


