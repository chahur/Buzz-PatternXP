#pragma once

#include "MachinePattern.h"
#include "ScrollWnd.h"
#include "CursorPos.h"
#include <bitset>


class CEditorWnd;

int HexToInt(char c);
bool DialogFileName(LPSTR Suffix, LPSTR FileLibelle, LPSTR FileTitle, LPSTR InitFilename, LPSTR pathName, bool DoOpen);

typedef bitset<12> note_bitset;

struct chord_struct
{
	string name;
	note_bitset notes;
};

struct tonality_struct
{
	string name;
	int base_note;
	bool major;
	int sharp_flat;
	note_bitset notes;
};

struct row_struct
{
	note_bitset notes;
	int chord_index;
	int base_data;
	int base_note;
	int base_octave;
};

struct grid_struct
{
	note_bitset notes;
	int chord_index;
	int base_note;
	int delta;
};

typedef std::vector<std::string> string_vector;
typedef std::vector<row_struct> row_vector;
typedef std::vector<grid_struct> grid_vector;
typedef std::vector<chord_struct> chord_vector;
typedef std::vector<tonality_struct> tonality_vector;

typedef std::vector<row_vector> row_vector_vector;

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

//	bool DialogFileName(LPSTR Suffix, LPSTR FileLibelle, LPSTR FileTitle, LPSTR InitFilename, LPSTR pathName, bool DoOpen);

	void InsertChord(int ChordIndex = -1);
	void InsertChordNote(int note, int ChordIndex);

	void Reverse();
	void Mirror();
	void InsertRow();
	void DeleteRow();
	void ClearRow();
	void SelectRow(int r);
	void SelectBeat(int r);
	void SelectMesure(int r);


	void DoAnalyseChords();
	void DoManualAnalyseChords();
	void DoInsertChord(int ChordIndex);
	void MoveCursorUpDown(int dy);
	void MoveCursorPgUpDown(int dy);
	bool SaveArpeggio();

	void AnalyseTonality();

	void Interpolate(bool expintp);

	bool CanCut();
	bool CanCopy();
	bool CanPaste();
	bool CanInsertChord();
	bool CheckNoteCol();

	int GetColumnAtX(int x);
	int GetColumnX(int column);
	void ShiftValues(int delta, bool OnlyNotes);
	int GetRowY(int y);
	void Transpose(int MinValue, int MaxValue);
	int TestTranspose(int MinValue, int MaxValue, int delta, int *count_min, int *count_max);


	void InvalidateInTimer() { invalidateInTimer = true; }

	int GetColumnWidth(int column);
	int GetFirstColumnofTrack(int column);
	int GetTrackWidth(int column);
	
	void SelectTrackByNo(int col); //BWC
private:
	void TextToFieldImport(char *txt, CColumn *pc, int irow);
	void DrawColumn(CDC *pDC, int col, int x, COLORREF textcolor, CRect const &clipbox);
	void DrawField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue, COLORREF textcolor);
	void DrawGraphicalField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue, COLORREF textcolor);
	void DrawCursor(CDC *pDC);
	bool CheckNoteInTonality(byte note);
	bool CheckLeadingTone(byte note);

	bool IsMajorTonality();

	CRect GetCursorRect(CCursorPos const &p);
	void MoveCursor(CCursorPos newpos, bool killsel = true);
	void MoveCursorDelta(int dx, int dy);
	void Tab();
	void ShiftTab();
	int GetDigitAtX(int x);
	
	void UpdateStatusBar();
	void InvalidateField(int row, int column);
	void InvalidateGroup(int row, int column);
	void EditNote(int note, bool canplay = true);
	void EditOctave(int oct);
	void EditByte(int n);
	void EditWord(int n);
	void EditSwitch(int n);
	void EditAscii(char n);
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

	int TestChords(note_bitset n, int ir);
	void AnalyseChordsMeasure(int rStart, int rStop);
	void CheckRefreshChords();
	void AnalyseChords();

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
	bool AnalyseChordRefresh;
	int CheckRefreshChordCount;
	bool AnalysingChords;

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
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
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


