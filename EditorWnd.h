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

	int GetEditorPatternPosition();
	void ShowParamTextChanged();
	void OnUpdateClipboard();
	void OnUpdateSelection();
	void OnUpdatePosition(); 


private:
	bool UpdatingToolbar;
	void UpdateWindowSizes();
	void FontChanged();
	void ToolbarChanged();

	void InitToolbarData();
	void DoShowHelp();
	void UpdateButtons();
	void InitChords();
	void GeneratorFileName(LPSTR FullFilename, LPSTR AFilename);


	bool GetCheckBoxToolbar();
	void SetCheckBoxToolbar(bool AValue);

	bool GetCheckBoxHelp();
	void SetCheckBoxHelp(bool AValue);

	bool GetCheckBoxMidiEdit();
	void SetCheckBoxMidiEdit(bool AValue);

	int  GetEditBoxDelta();	
	void SetEditBoxDelta(int AValue);

	int  GetComboBoxInflate();

	int  GetComboBoxBar();

	bool GetCheckBoxHumanizeEmpty();
	void SetCheckBoxHumanizeEmpty(bool AValue);

	int	 GetComboBoxChords();
	bool GetCheckBoxChordOnce();
	void SetCheckBoxChordOnce(bool AValue);




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

	CMachinePattern *pPattern;
	CFont font;
	CSize fontSize;
	int rowNumWndWidth;
	int topWndHeight;

	char Chords[100][20];

//	CFuBar reBar;
	CToolBar2 toolBar;
	CDialogBar dlgBar;

	bool MidiEditMode;
	int BarComboIndex;
	int DeltaHumanize;
	bool HumanizeEmpty;
	int ChordsComboIndex() {return GetComboBoxChords();};
	char ChordPathName[255];
	bool InsertChordOnce;
	bool ShowParamText;
	bool ShowTrackToolbar;


	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	
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

	afx_msg void OnCheckedToolbar();

	afx_msg void OnButtonInsertChord();
	afx_msg void OnButtonSelectChordFile();
	afx_msg void OnCheckedChordOnce();


};


