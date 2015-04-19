#pragma once

#include "MachinePattern.h"
#include "PatEd.h"
#include "EmptyWnd.h"
#include "RowNumWnd.h"
#include "TopWnd.h"
// #include "FuBar.h"
#include "ToolBar2.h"
#include "afxext.h"
#include <afxpriv.h>
#include "ColumnDialog.h"
#include "ActionStack.h"
#include <bitset>


static char const NoteTo2char[]     = "C-C#D-EbE-F-F#G-G#A-BbB-";
static char const NoteToText[]      = "C-C#D-D#E-F-F#G-G#A-A#B-";
static char const NoteToTextStrip[] = "C C#D D#E F F#G G#A A#B ";

#define MAX_ARPEGGIO_ROWS 64

class CEditorWnd : public CWnd
{
	DECLARE_DYNAMIC(CEditorWnd)

public:
	CEditorWnd();
	virtual ~CEditorWnd();

	void Create(HWND parent);
	void SetPattern(CMachinePattern *p);

	void SetPatternLength(CMachinePattern *p, int length);
	void SetPlayPos(MapIntToPlayingPattern &pp, CMasterInfo *pmi);

	void AddTrack(int n = 1);
	void DeleteLastTrack();

	bool EnableCommandUI(int id);

	void UpdateCanvasSize();
	int  GetComboBoxInflate();

	int GetEditorPatternPosition();
	void ShowParamTextChanged();
	void OnUpdateClipboard();
	void OnUpdateSelection();
	void OnUpdatePosition(); 
	void AnalyseChords();
	void SetComboBoxTonal(int index);
	void SetComboBoxArpeggio(int index);



private:
	bool UpdatingToolbar;
	void UpdateWindowSizes();
	void FontChanged();
	void ToolbarChanged();
	void ReadParamProfile();


	void InitToolbarData();
	void DoShowHelp();
	void UpdateButtons();
	void InitChords();
	void InitTonal();
	void InitTonality(LPSTR txt, int basenote, bool major, int sharpCount);
	void InitArpeggio();

	void GeneratorFileName(LPSTR FullFilename, LPSTR AFilename);

	bool GetCheckBoxHelp();
	void SetCheckBoxHelp(bool AValue);

	bool GetCheckBoxMidiEdit();
	void SetCheckBoxMidiEdit(bool AValue);

	int  GetEditBoxDelta();	
	void SetEditBoxDelta(int AValue);

	int  GetComboBoxBar();

	int GetComboBoxTonal();
	int GetComboBoxArpeggio();

	void GetComboBoxArpeggioText(LPSTR AValue);


	int GetComboBoxTranspose();

	bool GetCheckBoxHumanizeEmpty();
	void SetCheckBoxHumanizeEmpty(bool AValue);

	int	 GetComboBoxChords();
	bool GetCheckBoxChordOnce();
	void SetCheckBoxChordOnce(bool AValue);

	int GetComboBoxInterpolate();


protected:
	DECLARE_MESSAGE_MAP()
			
public:
	CMICallbacks *pCB;
	CMachine *pMachine;
	CGlobalData *pGlobalData;
//	CParamWnd pw;
	CPatEd pe;
	CRowNumWnd leftwnd;
	CTopWnd topwnd;
	CEmptyWnd topleftwnd;

	CRichEditCtrl helptext;
	int helpwidth;
	bool helpvisible;
	bool toolbarvisible;
	bool ChordExpertvisible;
	bool Closing;

	CMachinePattern *pPattern;
	CFont font;
	CSize fontSize;
	int rowNumWndWidth;
	int topWndHeight;

	// Chords 
	string_vector Chords;
	chord_vector ChordsBase;
	row_vector RowNotes;
	int minChordNotes;
	int maxChordNotes;

	// Tonality
	tonality_vector TonalityList;
	int tonality_notes[12];

	// Arpeggios
	int ArpeggioComboIndex;
	int ArpeggioDefaultCount;
	int ArpeggioRowCount;
	char ArpeggioRows[MAX_ARPEGGIO_ROWS][256]; // MAX_ARPEGGIO_ROWS rows of 256 cols
	CStringList* SLArpeggio;

	CToolBar2 toolBar;
	CToolBarExt toolBarExt;

	CDialogBar dlgBar;

	bool MidiEditMode;
	int BarComboIndex;
	int TonalComboIndex;
	int TransposeComboIndex;
	int DeltaHumanize;
	bool HumanizeEmpty;
	int ChordsComboIndex() {return GetComboBoxChords();};
	int InterpolateComboIndex() {return GetComboBoxInterpolate();};
	char ChordPathName[255];
	char ArpeggioPathName[255];	
	bool InsertChordOnce;
	bool ShowParamText;
	bool ShowTrackToolbar;
	int LINEAR_INTERPOLATE_PARAM;

	bool PersistentSelection;
	bool PersistentPlayPos;
	bool ImportAutoResize;
	bool AutoChordExpert;
	bool PgUpDownDisabled;
	bool HomeDisabled;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnColumns();
	afx_msg void OnSelectFont();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditUndo(CCmdUI *pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI *pCmdUI);

	afx_msg void OnUpdateClipboard(CCmdUI *pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditPaste();
	afx_msg void OnEditPasteSpecial();
	afx_msg void OnClearNoteOff();
	afx_msg void OnParameters();
	afx_msg void OnChordExpert();


protected:
//	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedColumnsButton();
	afx_msg void OnBnClickedFontButton();
	afx_msg void OnCheckedMidiEdit();
	
	afx_msg void OnBnClickedMisc();
	afx_msg void OnBnClickedClearOff();
	afx_msg void OnBnClickedAddOff();
	afx_msg void OnBnClickedUpOff();
	afx_msg void OnBnClickedDownOff();
	
	afx_msg void OnBnClickedImport();
	afx_msg void OnBnClickedExport();
	afx_msg void OnButtonShrink();
	afx_msg void OnButtonExpand();
	afx_msg void OnButtonHumanize();
	afx_msg void OnChangeHumanize(); 
	afx_msg void OnCheckedHumanizeEmpty();
	
	afx_msg void OnCheckedHelp();
	afx_msg void OnComboBarSelect();
	afx_msg void OnComboShrinkSelect();

	afx_msg void OnButtonInsertChord();
	afx_msg void OnButtonSelectChordFile();
	afx_msg void OnCheckedChordOnce();
	afx_msg void OnButtonInterpolate();
	afx_msg void OnComboInterpolateSelect();
	
	afx_msg void OnButtonReverse();
	afx_msg void OnButtonMirror();
	afx_msg void OnButtonInsertRow();
	afx_msg void OnButtonDeleteRow();

	afx_msg void OnButtonTonality();
	afx_msg void OnComboTonalSelect();
	afx_msg void OnComboTransposeSelect();
	afx_msg void OnButtonTransposeUp();
	afx_msg void OnButtonTransposeDown();

	afx_msg void OnComboArpeggioSelect();
	afx_msg void OnArpeggioSave();
	afx_msg void OnButtonDldChord();
	afx_msg void OnButtonSelectArpeggioFile();
	
	

	
	


};


