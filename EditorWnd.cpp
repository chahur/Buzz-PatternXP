#include "stdafx.h"
// #include "App.h"
#include "EditorWnd.h"
#include "PatEd.h"
#include "ImageList.h"
#include <iostream>
#include <string>
#include <fstream>
#include <io.h>
#include "Parameters.h"
#include "ChordExpert.h"

HHOOK g_hHook = 0;

void CMachineDataOutput::Write(void *pbuf, int const numbytes) {}
void CMachineDataInput::Read(void *pbuf, int const numbytes) {}


IMPLEMENT_DYNAMIC(CEditorWnd, CWnd)

CEditorWnd::CEditorWnd()
{
	pPattern = NULL;

	Closing = false;
	MidiEditMode = false;
	BarComboIndex = 0;
	TonalComboIndex  = 0;
	PgUpDownDisabled = false;
	HomeDisabled = false;
}

CEditorWnd::~CEditorWnd()
{
}


BEGIN_MESSAGE_MAP(CEditorWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()

	ON_COMMAND(ID_COLUMNS_BUTTON, OnColumns)
	ON_BN_CLICKED(IDC_COLUMNS_BUTTON, CEditorWnd::OnBnClickedColumnsButton) 
	ON_COMMAND(ID_PARAMETERS_BUTTON, OnParameters)
	ON_BN_CLICKED(IDC_PARAMETERS_BUTTON, OnParameters)
	ON_COMMAND(ID_SELECT_FONT, OnSelectFont)
	ON_BN_CLICKED(IDC_FONT_BUTTON, CEditorWnd::OnBnClickedFontButton)	

	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTE_SPECIAL, OnEditPasteSpecial) //BWC
	
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateClipboard)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateClipboard)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateClipboard)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_SPECIAL, OnUpdateClipboard) //BWC
	
	ON_BN_CLICKED(IDC_MIDI_EDIT, OnCheckedMidiEdit) 
	ON_BN_CLICKED(ID_CHECK_MIDI, OnCheckedMidiEdit) 

	ON_BN_CLICKED(IDC_MISC_BUTTON, OnBnClickedMisc) //BWC
	ON_BN_CLICKED(IDC_PASTE_BUTTON, OnEditPaste) //BWC
	ON_BN_CLICKED(IDC_CUT_BUTTON, OnEditCut) //BWC
	ON_BN_CLICKED(IDC_COPY_BUTTON, OnEditCopy) //BWC
	
	ON_COMMAND(ID_BT_PASTE, OnEditPaste) //BWC
	ON_COMMAND(ID_BT_CUT, OnEditCut) //BWC
	ON_COMMAND(ID_BT_COPY, OnEditCopy) //BWC
	ON_COMMAND(ID_BT_MERGE, OnBnClickedMisc) //BWC

	
	ON_BN_CLICKED(IDC_CLEAR_BUTTON, CEditorWnd::OnBnClickedClearOff) //BWC
	ON_COMMAND(ID_BT_CLEAROFF, OnBnClickedClearOff) //BWC
	ON_BN_CLICKED(IDC_ADDOFF_BUTTON, CEditorWnd::OnBnClickedAddOff) //BWC
	ON_COMMAND(ID_BT_ADDOFF, OnBnClickedAddOff) //BWC
	ON_BN_CLICKED(IDC_UPOFF_BUTTON, CEditorWnd::OnBnClickedUpOff) //BWC
	ON_BN_CLICKED(IDC_DOWNOFF_BUTTON, CEditorWnd::OnBnClickedDownOff) //BWC
	ON_COMMAND(ID_BT_UPOFF, OnBnClickedUpOff) //BWC
	ON_COMMAND(ID_BT_DOWNOFF, OnBnClickedDownOff) //BWC

	ON_BN_CLICKED(IDC_IMPORT_BUTTON, OnBnClickedImport) //BWC
	ON_COMMAND(ID_BT_IMPORT, OnBnClickedImport) //BWC
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, OnBnClickedExport) //BWC
	ON_COMMAND(ID_BT_EXPORT, OnBnClickedExport) //BWC
	
	ON_COMMAND(ID_BT_SHRINK, OnButtonShrink) //BWC
	ON_BN_CLICKED(IDC_SHRINK_BUTTON, OnButtonShrink) //BWC
	ON_COMMAND(ID_BT_EXPAND, OnButtonExpand) //BWC
	ON_BN_CLICKED(IDC_EXPAND_BUTTON, OnButtonExpand) //BWC
	
	ON_BN_CLICKED(IDC_HELP_CHECK, OnCheckedHelp) //BWC
	ON_BN_CLICKED(ID_CHECK_HELP, OnCheckedHelp) //BWC
		
	ON_COMMAND(ID_BT_HUMANIZE, OnButtonHumanize) //BWC
	ON_EN_CHANGE(ID_HUMANIZE_DELTA, OnChangeHumanize)
	ON_EN_CHANGE(IDC_HUMANIZE_EDIT, OnChangeHumanize)
	ON_BN_CLICKED(IDC_HUMANIZE_BUTTON, OnButtonHumanize)

	ON_BN_CLICKED(ID_CHECK_HUMANIZE_EMPTY, OnCheckedHumanizeEmpty) //BWC
	ON_BN_CLICKED(IDC_HUMANIZE_EMPTY, OnCheckedHumanizeEmpty) //BWC
		
	ON_CBN_SELENDOK(IDC_BAR_COMBO, OnComboBarSelect)
	ON_CBN_SELENDOK(ID_COMBO_BAR, OnComboBarSelect)

	ON_COMMAND(ID_BT_INSERT_CHORD, OnButtonInsertChord)
	ON_BN_CLICKED(IDC_INSERT_CHORD, OnButtonInsertChord)
	ON_COMMAND(ID_BT_CHORD_FILE, OnButtonSelectChordFile)
	ON_BN_CLICKED(IDC_CHORD_FILE_BUTTON, OnButtonSelectChordFile) 
	
	ON_BN_CLICKED(ID_CHECK_CHORD_ONCE, OnCheckedChordOnce) 
	ON_BN_CLICKED(IDC_CHECK_CHORD_ONCE, OnCheckedChordOnce) 

	ON_COMMAND(ID_BT_INTERPOLATE, OnButtonInterpolate)
	ON_BN_CLICKED(IDC_INTERPOLATE_BUTTON, OnButtonInterpolate) 
	ON_CBN_SELENDOK(ID_INTERPOLATE_PARAM, OnComboInterpolateSelect)
	ON_CBN_SELENDOK(IDC_INTERPOLATE_COMBO, OnComboInterpolateSelect)
	
	ON_COMMAND(ID_BT_REVERSE, OnButtonReverse)
	ON_BN_CLICKED(IDC_REVERSE_BUTTON, OnButtonReverse) 
	ON_COMMAND(ID_BT_MIRROR, OnButtonMirror)
	ON_BN_CLICKED(IDC_MIRROR_BUTTON, OnButtonMirror) 

	ON_COMMAND(ID_BT_INSROW, OnButtonInsertRow)
	ON_BN_CLICKED(IDC_BT_INSROW, OnButtonInsertRow) 
	ON_COMMAND(ID_BT_DELROW, OnButtonDeleteRow)
	ON_BN_CLICKED(IDC_BT_DELROW, OnButtonDeleteRow) 
	
	ON_CBN_SELENDOK(IDC_TONAL_COMBO, OnComboTonalSelect)
	ON_CBN_SELENDOK(ID_COMBO_TONAL, OnComboTonalSelect)
	ON_COMMAND(ID_TONAL_BT, OnButtonTonality)
	ON_BN_CLICKED(IDC_TONAL_BUTTON, OnButtonTonality) 



END_MESSAGE_MAP()

/* Windows hook to receive the ToolTips messages */
static LRESULT CALLBACK PreTranslateGetMsgProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState()); 

    ASSERT (g_hHook);
 
    if ( (nCode < 0) )      // doc says do not process
        return CallNextHookEx (g_hHook, nCode, wParam, lParam);
 
	// Pretend that this is CWinApp::Run() and call PreTranslateMessage
    // as in a normal MFC app
    MSG *pMsg = (MSG *) lParam;
    CWnd *pWnd = CWnd::FromHandlePermanent(pMsg->hwnd);
    if (pWnd)    // a CWnd exists
	{
        pWnd->PreTranslateMessage(pMsg);
	}
 
    return CallNextHookEx (g_hHook, nCode, wParam, lParam);
}


void CEditorWnd::Create(HWND parent)
{
	CWnd::Create(NULL, "editor", WS_CHILD | WS_VISIBLE, CRect(0, 0, 100, 100), CWnd::FromHandle(parent), 0);
	
	// Install WindowsHook only once
	if (g_hHook==0) g_hHook = SetWindowsHookEx (WH_GETMESSAGE, PreTranslateGetMsgProc, AfxGetInstanceHandle(), GetCurrentThreadId());
}

void CEditorWnd::SetPattern(CMachinePattern *p)
{
	if (p == pPattern)
		return;

	pPattern = p;
	pe.PatternChanged();
	UpdateCanvasSize();

}

void CEditorWnd::SetPatternLength(CMachinePattern *p, int length)
{
	p->SetLength(length, pCB);

	if (p == pPattern)
	{
		pe.PatternChanged();
		UpdateCanvasSize();
	}
}

void CEditorWnd::SetPlayPos(MapIntToPlayingPattern &pp, CMasterInfo *pmi)
{
	pe.SetPlayPos(pp, pmi);
}

void CEditorWnd::UpdateCanvasSize()
{
	if (pPattern == NULL)
		return;

	int height = pPattern->GetRowCount();

	int width = -1;
	for (ColumnVector::iterator i = pPattern->columns.begin(); i != pPattern->columns.end(); i++)
		width += (*i)->GetWidth();
	
	pe.SetCanvasSize(CSize(width, height));
	leftwnd.SetCanvasSize(CSize(5, height));
	topwnd.SetCanvasSize(CSize(width, 2));
	
}

void CEditorWnd::OnClose()
{
	Closing = true;
}

int CEditorWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	byte *data;
	int nbytes;
	pCB->GetProfileBinary("Font", &data, &nbytes);
	if (nbytes == sizeof(LOGFONT))
	{
		font.CreateFontIndirect((LOGFONT *)data);
		pCB->FreeProfileBinary(data);
	}
	else
	{
		font.CreatePointFont(90, "Fixedsys");
	}
	
	CRect rect;
	UpdatingToolbar = false;
	DeltaHumanize=10;
	
	// Create Toolbar

	toolBar.CreateEx(this, 0, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	toolBar.LoadToolBar(IDR_TOOLBAR); 
	
	// Set transparent color Magenta : RGB(255,0,255)
	CImageList il;
	CreateImageList(il, MAKEINTRESOURCE(IDR_TOOLBAR), 16, 4, RGB(255,0,255));
	toolBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)il.m_hImageList);
	il.Detach();

	// Create font for Toolbar
	toolBar.tbFont.CreatePointFont(80, "MS Shell Dlg");
	toolBar.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 
	
	toolBar.SendMessage(TB_SETMAXTEXTROWS, 0, 0);
   
	toolBar.EnableToolTips(true);
	EnableToolTips(true);
	
	// Insert "midi edit" check box
	int index = toolBar.CommandToIndex(ID_CHECK_MIDI_BT);
    toolBar.SetButtonInfo(index, ID_CHECK_MIDI_BT, TBBS_SEPARATOR, 60);
    toolBar.GetItemRect(index, &rect);
	rect.top++;
	toolBar.checkMidi.Create("Midi edit", BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_CHECK_MIDI);
	toolBar.checkMidi.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 
	if (MidiEditMode) toolBar.checkMidi.SetCheck(BST_CHECKED);

	// Insert Bar Combo
	index = toolBar.CommandToIndex(ID_COMBO_BAR_LABEL_BT);
    toolBar.SetButtonInfo(index, ID_COMBO_BAR_LABEL_BT, TBBS_SEPARATOR, 24);  
	toolBar.GetItemRect(index, &rect);
	rect.top = 5;
	toolBar.labelBar.Create("Bar", SS_CENTER|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_COMBO_BAR_LABEL);
	toolBar.labelBar.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 	

	index = toolBar.CommandToIndex(ID_COMBO_BAR_BT);
    toolBar.SetButtonInfo(index, ID_COMBO_BAR_BT, TBBS_SEPARATOR, 48); 
	toolBar.GetItemRect(index, &rect);
	rect.top = 1;
    rect.bottom = rect.top + 80;
	toolBar.comboBar.Create(CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_TABSTOP | WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_COMBO_BAR);
	toolBar.comboBar.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 

	// Insert Shrink Combo
	index = toolBar.CommandToIndex(ID_COMBO_SHRINK_BT);
    toolBar.SetButtonInfo(index, ID_COMBO_SHRINK_BT, TBBS_SEPARATOR, 32); 
	toolBar.GetItemRect(index, &rect);
	rect.top = 1;  
    rect.bottom = rect.top + 80;
	toolBar.comboShrink.Create(CBS_DROPDOWNLIST | CBS_AUTOHSCROLL | WS_TABSTOP | WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_COMBO_SHRINK);
	toolBar.comboShrink.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 

	// Insert Humanize factor edit
	index = toolBar.CommandToIndex(ID_HUMANIZE_DELTA_BT);
    toolBar.SetButtonInfo(index, ID_HUMANIZE_DELTA_BT, TBBS_SEPARATOR, 24);
    toolBar.GetItemRect(index, &rect);
	rect.top = 2;
	rect.bottom = rect.bottom - 2; 
	toolBar.editHumanize.Create(ES_NUMBER|WS_CHILD|WS_BORDER|WS_VISIBLE, rect, &toolBar, ID_HUMANIZE_DELTA);
	toolBar.editHumanize.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 
	toolBar.editHumanize.SetMargins(2, 2);

	// Insert "Humanize empty" check box
	index = toolBar.CommandToIndex(ID_CHECK_HUMANIZE_EMPTY_BT);
    toolBar.SetButtonInfo(index, ID_CHECK_HUMANIZE_EMPTY_BT, TBBS_SEPARATOR, 72);
    toolBar.GetItemRect(index, &rect);
	rect.left= rect.left+2;
	rect.top++;
	toolBar.checkHumanizeEmpty.Create("and empty", BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_CHECK_HUMANIZE_EMPTY);
	toolBar.checkHumanizeEmpty.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 
 
	// Insert Chords Combo
	index = toolBar.CommandToIndex(ID_COMBO_CHORD_BT);
    toolBar.SetButtonInfo(index, ID_COMBO_CHORD_BT, TBBS_SEPARATOR, 64); 
	toolBar.GetItemRect(index, &rect);
	rect.top = 1;  
    rect.bottom = rect.top + 80;
	toolBar.comboChords.Create(CBS_DROPDOWNLIST|CBS_AUTOHSCROLL|WS_VSCROLL|WS_TABSTOP|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_COMBO_CHORD);
	toolBar.comboChords.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 

	// Insert "Insert chord once" check box
	index = toolBar.CommandToIndex(ID_CHECK_CHORD_ONCE_BT);
    toolBar.SetButtonInfo(index, ID_CHECK_CHORD_ONCE_BT, TBBS_SEPARATOR, 48);
    toolBar.GetItemRect(index, &rect);
	rect.top++;
	toolBar.checkChordsOnce.Create("Once", BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_CHECK_CHORD_ONCE);
	toolBar.checkChordsOnce.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 
	
	// Insert interpolate Combo
	index = toolBar.CommandToIndex(ID_INTERPOLATE_PARAM_BT);
    toolBar.SetButtonInfo(index, ID_INTERPOLATE_PARAM_BT, TBBS_SEPARATOR, 64); 
	toolBar.GetItemRect(index, &rect);
	rect.top = 1;  
    rect.bottom = rect.top + 80;
	toolBar.comboInterpolate.Create(CBS_DROPDOWNLIST|CBS_AUTOHSCROLL|WS_VSCROLL|WS_TABSTOP|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_INTERPOLATE_PARAM);
	toolBar.comboInterpolate.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 

	// Insert Tonal combo and button
	index = toolBar.CommandToIndex(ID_LABEL_TONAL_BT);
    toolBar.SetButtonInfo(index, ID_LABEL_TONAL_BT, TBBS_SEPARATOR, 36);  
	toolBar.GetItemRect(index, &rect);
	rect.top = 5;
	toolBar.labelTonal.Create("Tonal", SS_CENTER|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_LABEL_TONAL);
	toolBar.labelTonal.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 	

	index = toolBar.CommandToIndex(ID_COMBO_TONAL_BT);
    toolBar.SetButtonInfo(index, ID_COMBO_TONAL_BT, TBBS_SEPARATOR, 64); 
	toolBar.GetItemRect(index, &rect);
	rect.top = 1;
    rect.bottom = rect.top + 80;
	toolBar.comboTonal.Create(CBS_DROPDOWNLIST|CBS_AUTOHSCROLL|WS_VSCROLL|WS_TABSTOP|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_COMBO_TONAL);
	toolBar.comboTonal.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 


	// Insert "Help" check box
	index = toolBar.CommandToIndex(ID_CHECK_HELP_BT);
    toolBar.SetButtonInfo(index, ID_CHECK_HELP_BT, TBBS_SEPARATOR, 48);
    toolBar.GetItemRect(index, &rect);
	rect.top++;
	toolBar.checkHelp.Create("Help", BS_AUTOCHECKBOX|WS_CHILD|WS_VISIBLE, rect, &toolBar, ID_CHECK_HELP);
	toolBar.checkHelp.SendMessage(WM_SETFONT, (WPARAM)HFONT(toolBar.tbFont),TRUE); 
   
	// CDialogBar : Buttons bar
	dlgBar.Create(this, IDD_DIALOGBAR, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, AFX_IDW_TOOLBAR);
	
	// Help static panel
	helpwidth = 250;
	helpvisible = false;
	helptext.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL, CRect(0, 0, 10, 10), this, 1);
	

	// CPatEd : Pattern editor
	pe.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	pe.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	pe.AlwaysShowVerticalScrollBar(true);
	pe.AlwaysShowHorizontalScrollBar(true);
	pe.pew = this;
	pe.pCB = pCB;

	// CTopWnd : Track header
	topwnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	topwnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	topwnd.AlwaysShowVerticalScrollBar(true);
	topwnd.HideHorizontalScrollBar(true);
	topwnd.pew = this;

	// CRowNumWnd : Row numbers
	leftwnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	leftwnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	leftwnd.AlwaysShowHorizontalScrollBar(true);
	leftwnd.HideVerticalScrollBar(true);
	leftwnd.pew = this;

	// CEmptyWnd : Top Left corner (empty)
	topleftwnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	topleftwnd.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	topleftwnd.pCB = pCB;
	topleftwnd.pew = this;

	pe.linkVert = &leftwnd;
	pe.linkHorz = &topwnd;

	ReadParamProfile();

	ToolbarChanged();

	FontChanged();

	InitToolbarData();
	
	UpdateButtons();
	
	DoShowHelp();

	return 0;
}

void CEditorWnd::ReadParamProfile()
{
	ShowParamText =	pCB->GetProfileInt("ShowParamText", true)!=0;
	ShowTrackToolbar = pCB->GetProfileInt("ShowTrackToolbar", true)!=0;
	toolbarvisible = pCB->GetProfileInt("ToolbarVisible", true)!=0;
	ChordExpertvisible = pCB->GetProfileInt("ChordExpertvisible", true)!=0;

	DeltaHumanize = pCB->GetProfileInt("DeltaHumanize", 10);
	HumanizeEmpty = pCB->GetProfileInt("HumanizeEmpty", true)!=0;
	helpvisible = pCB->GetProfileInt("helpvisible", false)!=0;
	InsertChordOnce = pCB->GetProfileInt("InsertChordOnce", false)!=0;
	
	PersistentSelection= pCB->GetProfileInt("PersistentSelection", true)!=0;
	PersistentPlayPos= pCB->GetProfileInt("PersistentPlayPos", true)!=0;
	ImportAutoResize= pCB->GetProfileInt("ImportAutoResize", true)!=0;
	AutoChordExpert= pCB->GetProfileInt("AutoChordExpert", false)!=0;

	PgUpDownDisabled= pCB->GetProfileInt("PgUpDownDisabled", false)!=0;
	HomeDisabled= pCB->GetProfileInt("HomeDisabled", false)!=0;

	ChordPathName[0]=0;
	pCB->GetProfileString("ChordPathName", ChordPathName, "");
}


void CEditorWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CWnd::OnShowWindow(bShow, nStatus);

	Closing = false;

	// If the configuration of the toolbar have been changed in another instance of patternXP, init it now.
	ReadParamProfile();

	ToolbarChanged();

	DoShowHelp();

//	AnalyseChords();

}

/* ---- Checkbox "Help" -----*/
bool CEditorWnd::GetCheckBoxHelp()
{
	if (toolbarvisible)  
		return toolBar.checkHelp.GetCheck() == BST_CHECKED;
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_HELP_CHECK);
		return pc->GetCheck() == BST_CHECKED;
	}
}

void CEditorWnd::SetCheckBoxHelp(bool AValue)
{
	int BST_VAL;
	if (AValue) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;

	if (toolbarvisible)  
		toolBar.checkHelp.SetCheck(BST_VAL);
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_HELP_CHECK);
		pc->SetCheck(BST_VAL);
	}
}

/* ---- Checkbox "MidiEdit" -----*/
bool CEditorWnd::GetCheckBoxMidiEdit()
{
	if (toolbarvisible)  
		return toolBar.checkMidi.GetCheck() == BST_CHECKED;
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_MIDI_EDIT);
		return pc->GetCheck() == BST_CHECKED;
	}
}

void CEditorWnd::SetCheckBoxMidiEdit(bool AValue)
{
	int BST_VAL;
	if (AValue) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;

	if (toolbarvisible)  
		toolBar.checkMidi.SetCheck(BST_VAL);
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_MIDI_EDIT);
		pc->SetCheck(BST_VAL);
	}
}

/* ---- EditBox "Delta" humanize-----*/
int CEditorWnd::GetEditBoxDelta()
{
	CString sdelta;
		
	if (toolbarvisible)  
		toolBar.editHumanize.GetWindowText(sdelta);
	else {
		CEdit *ce = (CEdit *)dlgBar.GetDlgItem(IDC_HUMANIZE_EDIT);
		ce->GetWindowText(sdelta);
	}

	int x = atoi(sdelta);
	if ((x>=0) && (x<=100))
		return x;
	else
		return -1; /* incorrect value */
}

void CEditorWnd::SetEditBoxDelta(int AValue)
{
	CString sdelta;
	sdelta.Format(_T("%d"), AValue);

	if (toolbarvisible)  
		toolBar.editHumanize.SetWindowText(sdelta);
	else {
		CEdit *ce = (CEdit *)dlgBar.GetDlgItem(IDC_HUMANIZE_EDIT);
		ce->SetWindowText(sdelta);
	}
}

/*---- Combo box "Inflate" factor ----*/
int CEditorWnd::GetComboBoxInflate()
{	
	int inflateComboIndex;
	if (toolbarvisible)  
		inflateComboIndex = toolBar.comboShrink.GetCurSel();
	else {
		CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_INFLATE_COMBO);
		inflateComboIndex = cb->GetCurSel();
	}
	
	if (inflateComboIndex>=0)
		return inflateComboIndex+2;
	else
		return -1; /* Incorrect value */
}

/*---- Combo box "Measure Bar" ----*/
int CEditorWnd::GetComboBoxBar()
{
	if (toolbarvisible)  
		return toolBar.comboBar.GetCurSel();
	else {
		CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_BAR_COMBO);
		return cb->GetCurSel();
	}
}

/*---- Combo box "Tonal Bar" ----*/
int CEditorWnd::GetComboBoxTonal()
{
	if (toolbarvisible)  
		return toolBar.comboTonal.GetCurSel();
	else {
		CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_TONAL_COMBO);
		return cb->GetCurSel();
	}
}

/* ---- Checkbox "Humanize empty" -----*/
bool CEditorWnd::GetCheckBoxHumanizeEmpty()
{
	if (toolbarvisible)  
		return toolBar.checkHumanizeEmpty.GetCheck() == BST_CHECKED;
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_HUMANIZE_EMPTY);
		return pc->GetCheck() == BST_CHECKED;
	}
}

void CEditorWnd::SetCheckBoxHumanizeEmpty(bool AValue)
{
	int BST_VAL;
	if (AValue) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;

	if (toolbarvisible)  
		toolBar.checkHumanizeEmpty.SetCheck(BST_VAL);
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_HUMANIZE_EMPTY);
		pc->SetCheck(BST_VAL);
	}
}

// Combobox Chords
int CEditorWnd::GetComboBoxChords()
{
	if (toolbarvisible)  
		return toolBar.comboChords.GetCurSel();
	else {
		CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_CHORD_COMBO);
		return cb->GetCurSel();
	}
}

bool CEditorWnd::GetCheckBoxChordOnce()
{
	if (toolbarvisible)  
		return toolBar.checkChordsOnce.GetCheck() == BST_CHECKED;
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_CHECK_CHORD_ONCE);
		return pc->GetCheck() == BST_CHECKED;
	}
}

void CEditorWnd::SetCheckBoxChordOnce(bool AValue)
{
	int BST_VAL;
	if (AValue) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;

	if (toolbarvisible)  
		toolBar.checkChordsOnce.SetCheck(BST_VAL);
	else {
		CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_CHECK_CHORD_ONCE);
		pc->SetCheck(BST_VAL);
	}
}


int CEditorWnd::GetComboBoxInterpolate()
{
	if (toolbarvisible)  
		return toolBar.comboInterpolate.GetCurSel();
	else {
		CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_INTERPOLATE_COMBO);
		return cb->GetCurSel();
	}
}



void CEditorWnd::ToolbarChanged()
{
	/* Disable controls action */
	UpdatingToolbar = true;

	if (toolbarvisible) 
	{
		toolBar.ShowWindow(SW_SHOWNORMAL);
		dlgBar.ShowWindow(SW_HIDE);
	}
	else
	{
		dlgBar.ShowWindow(SW_SHOWNORMAL);
		toolBar.ShowWindow(SW_HIDE);
	}

	SetCheckBoxHelp(helpvisible);
	SetEditBoxDelta(DeltaHumanize);
	SetCheckBoxMidiEdit(MidiEditMode);
	SetCheckBoxHumanizeEmpty(HumanizeEmpty);
	SetCheckBoxChordOnce(InsertChordOnce);

	Invalidate(true);
	UpdateCanvasSize();
	UpdateWindowSizes();

	/* Enable controls action */	
	UpdatingToolbar = false;

	pCB->WriteProfileInt("ToolbarVisible", toolbarvisible);	
}

void CEditorWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	UpdateWindowSizes();
}

void CEditorWnd::UpdateWindowSizes()
{	
	CRect cr;
//	GetClientRect(&cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	int cx = cr.Width();
	int cy = cr.Height();

	int top = cr.top + topWndHeight;
	int left = rowNumWndWidth;

	if (ChordExpertvisible)
		left = left + 60;

	if (helpvisible) {
		helptext.MoveWindow(cr.right-helpwidth, cr.top, helpwidth, cr.bottom -cr.top);
		cr.right = cr.right-helpwidth; }
	else {
		helptext.MoveWindow(cr.right, cr.top, 0, cr.bottom - cr.top);
	}

	topwnd.MoveWindow(left, cr.top, cr.right - left, top - cr.top);
	leftwnd.MoveWindow(0, top, left, cr.bottom - top);
	topleftwnd.MoveWindow(0, cr.top, left, top - cr.top);
	pe.MoveWindow(left, top, cr.right - left, cr.bottom - top);
}

void CEditorWnd::FontChanged()
{
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SelectObject(&font);
	fontSize = dc.GetOutputTextExtent("A");
	rowNumWndWidth = fontSize.cx * 5 + 4;
	topWndHeight = 2 * fontSize.cy + 4; 
	if (ShowParamText) topWndHeight = topWndHeight + 72; //BWC
	if (ShowTrackToolbar) topWndHeight = topWndHeight + 16; //BWC

	pe.SetLineSize(fontSize);
	leftwnd.SetLineSize(fontSize);
	topwnd.SetLineSize(fontSize);
	UpdateCanvasSize();
	UpdateWindowSizes();
	Invalidate();
}

void CEditorWnd::ShowParamTextChanged()
{
	FontChanged();
	pCB->WriteProfileInt("ShowParamText", ShowParamText);
	pCB->WriteProfileInt("ShowTrackToolbar", ShowTrackToolbar);	
	pCB->WriteProfileInt("ChordExpertvisible", ChordExpertvisible);	
	
}


void CEditorWnd::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	pe.SetFocus();
}

void CEditorWnd::OnSelectFont()
{
	LOGFONT lf;
	font.GetLogFont(&lf);

	CFontDialog dlg(&lf, CF_FIXEDPITCHONLY | CF_SCREENFONTS);
	if (dlg.DoModal() != IDOK)
		return;

	dlg.GetCurrentFont(&lf);

	pCB->WriteProfileBinary("Font", (byte *)&lf, sizeof(LOGFONT));

	font.DeleteObject();
	font.CreateFontIndirect(&lf);
	
	FontChanged();
}

void CEditorWnd::AddTrack(int n)
{
	if (pPattern == NULL || (int)pPattern->columns.size() == 0)
		return;

	CMachine *pmac = pe.GetCursorColumn()->GetMachine();
	CMachineInfo const *pmi = pCB->GetMachineInfo(pmac);

	if (pPattern->GetTrackCount(pmac) >= pmi->maxTracks)
		return;

	pPattern->actions.BeginAction(this, "Add Track");
	{
		MACHINE_LOCK;

		for (int i = 0; i < n; i++)
		{
			if (pPattern->GetTrackCount(pmac) >= pmi->maxTracks) break;
			pPattern->AddTrack(pmac, pCB);
		}
	}

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
}

void CEditorWnd::DeleteLastTrack()
{
	if (pPattern == NULL || (int)pPattern->columns.size() == 0)
		return;

	CMachine *pmac = pe.GetCursorColumn()->GetMachine();
	CMachineInfo const *pmi = pCB->GetMachineInfo(pmac);

	if (pPattern->GetTrackCount(pmac) <= pmi->minTracks)
		return;

	pPattern->actions.BeginAction(this, "Delete Last Track");
	{
		MACHINE_LOCK;
		pPattern->DeleteLastTrack(pe.GetCursorColumn()->GetMachine(), pCB);
	}

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();

	if (AutoChordExpert) AnalyseChords();
}

void CEditorWnd::OnChordExpert()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (pPattern == NULL)
		return;

	CChordExpertDialog dlg(this);
	dlg.pew = this;
	dlg.CursorRow = pe.cursor.row;
	if (dlg.DoModal() == IDOK)
	{
		
	}	
}

void CEditorWnd::OnParameters()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (pPattern == NULL)
		return;

	CParametersDialog dlg(this);
	dlg.pew = this;
	if (dlg.DoModal() == IDOK)
	{
		ToolbarChanged();
	}	
}

void CEditorWnd::OnColumns()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (pPattern == NULL)
		return;

	CColumnDialog dlg(this);
	dlg.pew = this;
	dlg.m_NameValue = pCB->GetPatternName(pPattern->pPattern);
	dlg.m_LengthValue = pPattern->numBeats;
	dlg.m_RPBValue = pPattern->rowsPerBeat;

	if (dlg.DoModal() != IDOK)
		return;

	pPattern->actions.BeginAction(this, "Change Properties");
	{
		pPattern->EnableColumns(dlg.enabledColumns, pCB, 1, dlg.m_RPBValue);
	}

	pCB->SetPatternName(pPattern->pPattern, dlg.m_NameValue);
	pCB->SetPatternLength(pPattern->pPattern, dlg.m_LengthValue * BUZZ_TICKS_PER_BEAT);


	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
}

void CEditorWnd::OnEditUndo()
{
	pPattern->actions.Undo(this);

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
	if (AutoChordExpert) AnalyseChords();
}

void CEditorWnd::OnEditRedo()
{
	pPattern->actions.Redo(this);

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
	if (AutoChordExpert) AnalyseChords();
}

void CEditorWnd::OnUpdateEditUndo(CCmdUI *pCmdUI) { pCmdUI->Enable(pPattern != NULL && pPattern->actions.CanUndo()); }
void CEditorWnd::OnUpdateEditRedo(CCmdUI *pCmdUI) { pCmdUI->Enable(pPattern != NULL && pPattern->actions.CanRedo()); }

bool CEditorWnd::EnableCommandUI(int id)
{
	switch(id)
	{
	case ID_EDIT_CUT: return pe.CanCut();
	case ID_EDIT_COPY: return pe.CanCopy();
	case ID_EDIT_PASTE: return pe.CanPaste();
	case ID_EDIT_PASTE_SPECIAL: return pe.CanPaste();
	case ID_EDIT_UNDO: return pPattern != NULL && pPattern->actions.CanUndo();
	case ID_EDIT_REDO: return pPattern != NULL && pPattern->actions.CanRedo();
	}

	return false;
}

void CEditorWnd::OnUpdateClipboard(CCmdUI *pCmdUI) 
{ 
	pCmdUI->Enable(EnableCommandUI(pCmdUI->m_nID)); 
	UpdateButtons();
}

void CEditorWnd::OnUpdateClipboard() 
{ 
	UpdateButtons();
}

void CEditorWnd::OnUpdateSelection() 
{ 
	UpdateButtons();
}

void CEditorWnd::OnUpdatePosition() 
{ 
	UpdateButtons();
}

BOOL EnableToolbarButtonByIndex(CToolBar *pToolbar, const int iIndex, const BOOL bEnabled)
{
	UINT nNewStyle;
	if (pToolbar == NULL) return FALSE;

	nNewStyle = pToolbar->GetButtonStyle(iIndex) & ~TBBS_DISABLED;
	if (!bEnabled)
	{
		nNewStyle |= TBBS_DISABLED;
		// If a button is currently pressed and then is disabled
		// COMCTL32.DLL does not unpress the button, even after the mouse
		// button goes up! We work around this bug by forcing TBBS_PRESSED
		// off when a button is disabled.
		nNewStyle &= ~TBBS_PRESSED;
	}

	if (nNewStyle & TBBS_SEPARATOR) return FALSE;
	pToolbar->SetButtonStyle(iIndex, nNewStyle);
	return TRUE;
}

BOOL EnableToolbarButtonByCommand(CToolBar *pToolbar, const int iCommand, const BOOL bEnabled)
{
	int	iIndex;
	if (pToolbar == NULL) return FALSE;
	if ((iIndex = pToolbar->CommandToIndex(iCommand)) < 0) return FALSE;
	return EnableToolbarButtonByIndex(pToolbar, iIndex, bEnabled);
}

void CEditorWnd::UpdateButtons() 
{ 
	EnableToolbarButtonByCommand(&toolBar, ID_BT_PASTE, pe.CanPaste());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_MERGE, pe.CanPaste());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_CUT, pe.CanCut());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_COPY, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_CLEAROFF, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_ADDOFF, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_UPOFF, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_DOWNOFF, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_INSERT_CHORD, pe.CanInsertChord() || (ChordExpertvisible && pe.CheckNoteCol()));
	EnableToolbarButtonByCommand(&toolBar, ID_BT_INTERPOLATE, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_REVERSE, pe.CanCopy());
	EnableToolbarButtonByCommand(&toolBar, ID_BT_MIRROR, pe.CanCopy());
}

void CEditorWnd::OnEditCut() { pe.OnEditCut(); pe.SetFocus();}
void CEditorWnd::OnEditCopy() { pe.OnEditCopy(); pe.SetFocus();}
void CEditorWnd::OnEditPaste() { pe.OnEditPaste(); pe.SetFocus();}
void CEditorWnd::OnEditPasteSpecial() { pe.OnEditPasteSpecial(); } //BWC
void CEditorWnd::OnClearNoteOff() { 
	pe.OnClearNoteOff(); 
	pe.SetFocus();
} 
void CEditorWnd::OnBnClickedAddOff() { 
	pe.OnAddNoteOff(); 
	pe.SetFocus();
} 
void CEditorWnd::OnBnClickedUpOff() { 
	pe.OnUpNoteOff(); 
	pe.SetFocus();
}
void CEditorWnd::OnBnClickedDownOff() { 
	pe.OnDownNoteOff(); 
	pe.SetFocus();
} 


void CEditorWnd::OnBnClickedColumnsButton()
{
	OnColumns();
}

void CEditorWnd::OnBnClickedFontButton()
{
	OnSelectFont();
}

void CEditorWnd::OnCheckedMidiEdit()
{
	if (UpdatingToolbar) return;
	MidiEditMode = GetCheckBoxMidiEdit();
	pe.SetFocus();
}


void CEditorWnd::OnBnClickedMisc()
{
	OnEditPasteSpecial();
	pe.SetFocus();
}

void CEditorWnd::OnBnClickedClearOff()
{	
	OnClearNoteOff();
	pe.SetFocus();
}

void CEditorWnd::OnBnClickedImport()
{
	pe.ImportPattern();
	pe.SetFocus();
}

void CEditorWnd::OnBnClickedExport()
{	
	pe.ExportPattern();
	pe.SetFocus();
}
	
void CEditorWnd::OnButtonShrink()
{	
	int inflateFactor = GetComboBoxInflate();
	if (inflateFactor>=0)
		pe.InflatePattern(-inflateFactor);
	pe.SetFocus();
}
	
void CEditorWnd::OnButtonExpand()
{	
	int inflateFactor = GetComboBoxInflate();
	if (inflateFactor>=0)
		pe.InflatePattern(inflateFactor);
	pe.SetFocus();
}

void CEditorWnd::OnChangeHumanize() 
{
	if (UpdatingToolbar) return;

	int x=GetEditBoxDelta();
	if (x<0) {	
		// incorrect value in edit box, reset last value
		SetEditBoxDelta(DeltaHumanize);
	}
	else
	{
		DeltaHumanize=x;
		pCB->WriteProfileInt("DeltaHumanize", DeltaHumanize);
	}
}

void CEditorWnd::OnButtonHumanize()
{	
	pe.Humanize(DeltaHumanize, HumanizeEmpty);
	pe.SetFocus();
}
	

void CEditorWnd::InitToolbarData()
{
	CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_BAR_COMBO);
	cb->AddString("Auto");
	cb->AddString("1");
	cb->AddString("2");
	cb->AddString("3");
	cb->AddString("4");
	cb->AddString("5");
	cb->AddString("6");
	cb->AddString("7");
	cb->AddString("8");	

	cb->SetCurSel(BarComboIndex);

	toolBar.comboBar.AddString("Auto");
	toolBar.comboBar.AddString("1");
	toolBar.comboBar.AddString("2");
	toolBar.comboBar.AddString("3");
	toolBar.comboBar.AddString("4");
	toolBar.comboBar.AddString("5");
	toolBar.comboBar.AddString("6");
	toolBar.comboBar.AddString("7");
	toolBar.comboBar.AddString("8");	

	toolBar.comboBar.SetCurSel(BarComboIndex);

	CComboBox *cb2 = (CComboBox *)dlgBar.GetDlgItem(IDC_INFLATE_COMBO);
	cb2->AddString("2");
	cb2->AddString("3");
	cb2->AddString("4");
	cb2->AddString("5");
	cb2->AddString("6");
	cb2->AddString("7");
	cb2->AddString("8");

	cb2->SetCurSel(0);

	toolBar.comboShrink.AddString("2");
	toolBar.comboShrink.AddString("3");
	toolBar.comboShrink.AddString("4");
	toolBar.comboShrink.AddString("5");
	toolBar.comboShrink.AddString("6");
	toolBar.comboShrink.AddString("7");
	toolBar.comboShrink.AddString("8");

	toolBar.comboShrink.SetCurSel(0);

    InitChords();

	toolBar.comboInterpolate.AddString("Up-max");
	toolBar.comboInterpolate.AddString("Up-5"); 
	toolBar.comboInterpolate.AddString("Up-4");
	toolBar.comboInterpolate.AddString("Up-3");
	toolBar.comboInterpolate.AddString("Up-2");
	toolBar.comboInterpolate.AddString("Up-1");
	toolBar.comboInterpolate.AddString("Linear");
	toolBar.comboInterpolate.AddString("Down-1");
	toolBar.comboInterpolate.AddString("Down-2");
	toolBar.comboInterpolate.AddString("Down-3");
	toolBar.comboInterpolate.AddString("Down-4");
	toolBar.comboInterpolate.AddString("Down-5");
	toolBar.comboInterpolate.AddString("Down-max");

	
	CComboBox *cb3 = (CComboBox *)dlgBar.GetDlgItem(IDC_INTERPOLATE_COMBO);
	cb3->AddString("4th root");
	cb3->AddString("Up-5");
	cb3->AddString("Up-4");
	cb3->AddString("Up-3");
	cb3->AddString("Up-2");
	cb3->AddString("Up-1");
	cb3->AddString("Linear");
	cb3->AddString("Down-1");
	cb3->AddString("Down-2");
	cb3->AddString("Down-3");
	cb3->AddString("Down-4");
	cb3->AddString("Down-5");
	cb3->AddString("4th power");

	LINEAR_INTERPOLATE_PARAM = 6;
	int indexInterpolate = pCB->GetProfileInt("IndexInterpolate", LINEAR_INTERPOLATE_PARAM);

	toolBar.comboInterpolate.SetCurSel(indexInterpolate);
	cb3->SetCurSel(indexInterpolate);


	InitTonal();
	toolBar.comboTonal.SetCurSel(TonalComboIndex);
	CComboBox *cb4 = (CComboBox *)dlgBar.GetDlgItem(IDC_TONAL_COMBO);
	cb4->SetCurSel(TonalComboIndex);

}

note_bitset GetBaseChord(LPSTR txt)
{
	note_bitset res;
	// First bit always set to true (note base of the chord)
	res.set(0, true); 
	for (int i=0; txt[i]>0; i++)
	{
		char c = txt[i]; 
		int delta = HexToInt(c);
		int d = delta - ((delta /12)*12);
		res.set(d, true); 
	}
	return res;
}

void CEditorWnd::InitChords()
{
	// Load chords filename
	char pathName[255];
	if (ChordPathName[0]==0)
		GeneratorFileName(pathName, "Basic1.chords");
	else
	{
		strcpy(pathName, ChordPathName);
		// Check if the file exists
		if (_access (pathName, 0) != 0) 
			// File doesn't exists (0 if exists)
			// Get default chord file
			GeneratorFileName(pathName, "Basic1.chords");
	}
	
	string impChord;
	char txt[25];
	int itxt=0;
	int iChord=0;
	bool getval;

	int cbindex = GetComboBoxChords();

	CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_CHORD_COMBO);

	// First, empty the combo
	cb->ResetContent();
	toolBar.comboChords.ResetContent();
	// Empty the Chords vector
	Chords.clear();
	ChordsBase.clear();
	minChordNotes=12;
	maxChordNotes=0;


	ifstream expfile (pathName, ios::in);  
	if (expfile)  
    {
		 while (getline(expfile, impChord))  
        {
			// Skip comment lines (starting with ;)
			if (((int)impChord.length()>0) && (impChord[0] !=';'))
			{
				chord_struct cs;

				txt[0]=0;
				itxt=0;
				getval=false;
				for (int i=0; i < (int)impChord.length(); i++)
				{
					if (impChord[i] ==';') 
					{
						txt[itxt]=0;
						itxt=0;
						// First field : name of the chord
						// Add it in the combo box.
						toolBar.comboChords.AddString(txt);
						cb->AddString(txt);
						cs.name = txt;
						getval=true;
					}
					else 
					{
						// Read only valid data
						if ((!getval) || 
							((impChord[i]>='0')&&(impChord[i]<='9')) ||
							((impChord[i]>='A')&&(impChord[i]<='Z')) ||
							((impChord[i]>='a')&&(impChord[i]<='z'))
							){
							txt[itxt]= impChord[i];
							itxt++;
						}
					}
				}
			
				txt[itxt]=0; 
				itxt=0;
				// Second field : interval of the notes
				// Keep it in the vector of chords
				Chords.push_back(txt);
				cs.notes = GetBaseChord(txt);
				ChordsBase.push_back(cs);
				int csnc = cs.notes.count();
				if (minChordNotes>csnc) minChordNotes=csnc;
				if (maxChordNotes<csnc) maxChordNotes=csnc;

				iChord++;
			}
		}
	}

	if (cbindex < 0) cbindex = 0;
	if (cbindex >= iChord) cbindex = 0;
	toolBar.comboChords.SetCurSel(cbindex);
	cb->SetCurSel(cbindex);
}

void CEditorWnd::OnButtonSelectChordFile()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	// Get filename
	char chordsName[255];
	if (ChordPathName[0]==0)
		GeneratorFileName(chordsName, "Basic1.chords");
	else
	{
		strcpy(chordsName, ChordPathName);
		// Check if the file exists
		if (_access (chordsName, 0) != 0) 
			// File doesn't exists (0 if exists)
			// Get default chord file
			GeneratorFileName(chordsName, "Basic1.chords");
	}

	if (pe.DialogFileName("chords", "Chords", "Select chords file", chordsName, ChordPathName, true))
	{
		pCB->WriteProfileString("ChordPathName", ChordPathName);
		InitChords();
		AnalyseChords();
	}
	pe.SetFocus();
}

note_bitset GetTonality(int count)
{
	// Init in C major (no Flat or Sharp)
//	note_bitset res("101011010101");
	note_bitset res("101010110101"); // Bitsets are set from right to left

	if (count <0)
	{ 
		// Bémol    : si, mi, la, ré, sol, do, fa
		// Flat     : B,  E,  A,  D,  G,   C,  F
		// Flat pos : 11, 4,  9,  2,  7,   0,  5 
		res.set(11, false); res.set(10, true); 
		
		if (count<-1) {	res.set(4, false); res.set(3, true); }
		if (count<-2) {	res.set(9, false); res.set(8, true); }
		if (count<-3) {	res.set(2, false); res.set(1, true); }
		if (count<-4) {	res.set(7, false); res.set(6, true); }
		if (count<-5) {	res.set(0, false); res.set(11, true); }
		if (count<-6) {	res.set(5, false); res.set(4, true); }
	}
	else if (count >0)
	{ 
		// Dièse     : fa, do, sol, ré, la, mi, si
		// Sharp     : F,  C,  G,   D,  A,  E,  B
		// Sharp pos : 5,  0,  7,   2,  9,  4, 11   
		res.set(5, false); res.set(6, true); 
		
		if (count>1) {	res.set(0, false); res.set(1, true); }
		if (count>2) {	res.set(7, false); res.set(8, true); }
		if (count>3) {	res.set(2, false); res.set(3, true); }
		if (count>4) {	res.set(9, false); res.set(10, true); }
		if (count>5) {	res.set(4, false); res.set(5, true); }
		if (count>6) {	res.set(11, false); res.set(0, true); }
	}
	else // Count == 0
	{
		// Leave as it is
	}

	return res;
}

void CEditorWnd::InitTonality(LPSTR txt, int sharpCount)
{
	chord_struct cs;
	toolBar.comboTonal.AddString(txt);
	CComboBox *cb4 = (CComboBox *)dlgBar.GetDlgItem(IDC_TONAL_COMBO);
	cb4->AddString(txt);

	cs.name = txt;	
	if (sharpCount >-999)
		cs.notes = GetTonality(sharpCount);
	else
		cs.notes.reset();

	TonalityList.push_back(cs);
}

void CEditorWnd::InitTonal()
{
	// Init the notes of each tonality.
	TonalityList.clear();

	// First entry : no tonality
	InitTonality("None", -999);

	InitTonality("C-Maj", 0);
	InitTonality("C-min", -3);
	InitTonality("C#-Maj", 7);
	InitTonality("C#-min", 4);
	InitTonality("Db-Maj", -5);
	InitTonality("D-Maj", 2);
	InitTonality("D-min", -1);
	InitTonality("D#-min", 6);
	InitTonality("Eb-Maj", -3);
	InitTonality("Eb-min", -6);
	InitTonality("E-Maj", 4);
	InitTonality("E-min", 1);
	InitTonality("F-Maj", -1);
	InitTonality("F-min", -4);
	InitTonality("F#-Maj", 6);
	InitTonality("F#-min", 3);
	InitTonality("Gb-Maj", -6);
	InitTonality("G-Maj", 1);
	InitTonality("G-min", -2);
	InitTonality("G#-min", 5);
	InitTonality("Ab-Maj", -4);
	InitTonality("Ab-min", -7);
	InitTonality("A-Maj", 3);
	InitTonality("A-min", 0);
	InitTonality("A#-min", 7);
	InitTonality("Bb-Maj", -2);
	InitTonality("Bb-min", -5);
	InitTonality("B-Maj", 5);
	InitTonality("B-min", 2);
	InitTonality("Cb-Maj", -7);

}

void CEditorWnd::OnCheckedChordOnce()
{
	if (UpdatingToolbar) return;
	InsertChordOnce = GetCheckBoxChordOnce();
	pCB->WriteProfileInt("InsertChordOnce", InsertChordOnce);
	pe.SetFocus();
}

void CEditorWnd::OnComboBarSelect()
{
	BarComboIndex = GetComboBoxBar();
	pCB->SetModifiedFlag();
	Invalidate();
	pe.SetFocus();
}

void CEditorWnd::OnComboTonalSelect()
{
	TonalComboIndex = GetComboBoxTonal();
	pCB->SetModifiedFlag();
	Invalidate();
	pe.SetFocus();
}

void CEditorWnd::OnButtonTonality()
{
	// Analyse the pattern to determine the tonality
	::MessageBox(NULL, "Analyses the pattern to determine the tonality ... not done yet.", "Pattern XP", MB_OK | MB_ICONWARNING);

	pe.SetFocus();
}



void CEditorWnd::OnCheckedHumanizeEmpty()
{
	if (UpdatingToolbar) return;
	HumanizeEmpty = GetCheckBoxHumanizeEmpty();
	pCB->WriteProfileInt("HumanizeEmpty", HumanizeEmpty);
	pe.SetFocus();
}

void CEditorWnd::OnCheckedHelp()
{
	if (UpdatingToolbar) return;
	helpvisible = GetCheckBoxHelp();
	pCB->WriteProfileInt("helpvisible", helpvisible);
	DoShowHelp();
	pe.SetFocus();
}

static DWORD CALLBACK MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   CFile* pFile = (CFile*) dwCookie;
   *pcb = pFile->Read(pbBuff, cb);
   return 0;
}

void CEditorWnd::GeneratorFileName(LPSTR FullFilename, LPSTR AFilename)
{
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	GetModuleFileName(NULL, path_buffer, sizeof(path_buffer)); 
	_splitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname,
                    _MAX_FNAME, ext, _MAX_EXT );
	if (AFilename[0]==0) 
		sprintf(FullFilename,"%s%sGear\\Generators\\PatternXP", drive, dir);
	else
		sprintf(FullFilename,"%s%sGear\\Generators\\PatternXP\\%s", drive, dir, AFilename);
}

void CEditorWnd::DoShowHelp()
{
	UpdateCanvasSize();
	UpdateWindowSizes();
	Invalidate();
	
	if (helpvisible) 
	{
		helptext.SetFont(&font);
		helptext.SetReadOnly(TRUE);
		helptext.SetTargetDevice(NULL, 1);
		
		char txt[255];
		GeneratorFileName(txt, "Jeskola Pattern XP.txt");

		CFile cFile(TEXT(txt), CFile::modeRead);
		EDITSTREAM es;
		es.dwCookie = (DWORD) &cFile;
		es.pfnCallback = MyStreamInCallback; 
		// helptext.StreamIn(SF_RTF, es);
		helptext.StreamIn(SF_TEXT, es); 
	}
	
}

void CEditorWnd::OnButtonInsertChord()
{
	if (pe.CheckNoteCol()) 
		if (pe.CanInsertChord())
			pe.InsertChord();
		else
			OnChordExpert();
	pe.SetFocus();
}

void CEditorWnd::OnButtonInterpolate()
{
	pe.Interpolate(false);
	pe.SetFocus();
}

void CEditorWnd::OnComboInterpolateSelect()
{
	pCB->WriteProfileInt("IndexInterpolate", GetComboBoxInterpolate());
	pe.SetFocus();
}


void CEditorWnd::OnButtonReverse()
{
	pe.Reverse();
	pe.SetFocus();
}

void CEditorWnd::OnButtonMirror()
{
	pe.Mirror();
	pe.SetFocus();
}

void CEditorWnd::OnButtonInsertRow()
{
	pe.InsertRow();
	pe.SetFocus();
}

void CEditorWnd::OnButtonDeleteRow()
{
	pe.DeleteRow();
	pe.SetFocus();
}

void CEditorWnd::AnalyseChords()
{
	if (ChordExpertvisible) {
		// Do it asynchronous
		pe.DoManualAnalyseChords();
	}
}

int CEditorWnd::GetEditorPatternPosition()
{
	if (pPattern == NULL)
		return 0;
	
	return pe.cursor.row * 4 / pPattern->rowsPerBeat;
}
