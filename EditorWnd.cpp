#include "stdafx.h"
// #include "App.h"
#include "EditorWnd.h"
#include "PatEd.h"
#include "ImageList.h"

void CMachineDataOutput::Write(void *pbuf, int const numbytes) {}
void CMachineDataInput::Read(void *pbuf, int const numbytes) {}


IMPLEMENT_DYNAMIC(CEditorWnd, CWnd)

CEditorWnd::CEditorWnd()
{
	pPattern = NULL;

	MidiEditMode = false;
}

CEditorWnd::~CEditorWnd()
{
}


BEGIN_MESSAGE_MAP(CEditorWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_COLUMNS_BUTTON, OnColumns)
	ON_COMMAND(ID_SELECT_FONT, OnSelectFont)
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
	ON_BN_CLICKED(IDC_COLUMNS_BUTTON, CEditorWnd::OnBnClickedColumnsButton) //BWC!!!
	ON_BN_CLICKED(IDC_FONT_BUTTON, CEditorWnd::OnBnClickedFontButton) //BWC!!!
	ON_BN_CLICKED(IDC_MIDI_EDIT, CEditorWnd::OnBnClickedMidiEdit) //BWC!!!

	ON_BN_CLICKED(IDC_MISC_BUTTON, OnBnClickedMisc) //BWC
	ON_BN_CLICKED(IDC_CLEAR_BUTTON, CEditorWnd::OnBnClickedClearOff) //BWC
	ON_BN_CLICKED(IDC_IMPORT_BUTTON, CEditorWnd::OnBnClickedImport) //BWC
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, CEditorWnd::OnBnClickedExport) //BWC
	ON_BN_CLICKED(IDC_HELP_CHECK, OnBnClickedHelp) //BWC

	ON_CBN_SELENDOK(IDC_BAR_COMBO, OnBarComboSelect)
END_MESSAGE_MAP()



void CEditorWnd::Create(HWND parent)
{
	CWnd::Create(NULL, "editor", WS_CHILD | WS_VISIBLE, CRect(0, 0, 100, 100), CWnd::FromHandle(parent), 0);
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


//	reBar.Create(this, 0);
//	toolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	/*
	toolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	toolBar.LoadToolBar(IDR_TOOLBAR);

	CImageList il;
	CreateImageList(il, MAKEINTRESOURCE(IDB_TOOLBAR24), 16, 4, RGB(255,0,255));
	toolBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)il.m_hImageList);
	il.Detach();
*/

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

	ShowParamText =	pCB->GetProfileInt("ShowParamText", true);

	FontChanged();

	InitBarCombo();
	
	return 0;
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
		MACHINE_LOCK;
		pPattern->EnableColumns(dlg.enabledColumns, pCB);
		pPattern->SetRowsPerBeat(dlg.m_RPBValue);
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
}

void CEditorWnd::OnEditRedo()
{
	pPattern->actions.Redo(this);

	pCB->SetModifiedFlag();
	pe.ColumnsChanged();
	UpdateCanvasSize();
	Invalidate();
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

void CEditorWnd::OnUpdateClipboard(CCmdUI *pCmdUI) { pCmdUI->Enable(EnableCommandUI(pCmdUI->m_nID)); }

void CEditorWnd::OnEditCut() { pe.OnEditCut(); }
void CEditorWnd::OnEditCopy() { pe.OnEditCopy(); }
void CEditorWnd::OnEditPaste() { pe.OnEditPaste(); }
void CEditorWnd::OnEditPasteSpecial() { pe.OnEditPasteSpecial(); } //BWC
void CEditorWnd::OnClearNoteOff() { pe.OnClearNoteOff(); } //BWC



void CEditorWnd::OnBnClickedColumnsButton()
{
	OnColumns();
}

void CEditorWnd::OnBnClickedFontButton()
{
	OnSelectFont();
}

void CEditorWnd::OnBnClickedMidiEdit()
{
	CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_MIDI_EDIT);
	MidiEditMode = pc->GetCheck() == BST_CHECKED;
}

void CEditorWnd::OnBnClickedMisc()
{
	OnEditPasteSpecial();
}

void CEditorWnd::OnBnClickedClearOff()
{	
	OnClearNoteOff();
}

void CEditorWnd::OnBnClickedImport()
{
	pe.ImportPattern();
}

void CEditorWnd::OnBnClickedExport()
{	
	pe.ExportPattern();
}

	

void CEditorWnd::InitBarCombo()
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
}

void CEditorWnd::OnBarComboSelect()
{
	CComboBox *cb = (CComboBox *)dlgBar.GetDlgItem(IDC_BAR_COMBO);
	BarComboIndex = cb->GetCurSel();
	pCB->SetModifiedFlag();
	Invalidate();
}

static DWORD CALLBACK MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   CFile* pFile = (CFile*) dwCookie;

   *pcb = pFile->Read(pbBuff, cb);

   return 0;
}

void CEditorWnd::OnBnClickedHelp()
{
	CButton *pc = (CButton *)dlgBar.GetDlgItem(IDC_HELP_CHECK);
	helpvisible = pc->GetCheck() == BST_CHECKED;
	UpdateCanvasSize();
	UpdateWindowSizes();
	Invalidate();
	
	if (helpvisible) {
		helptext.SetFont(&font);
		helptext.SetReadOnly(TRUE);
		helptext.SetTargetDevice(NULL, 1);
		

		
		char txt[255];
		char path_buffer[_MAX_PATH];
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		GetModuleFileName(NULL, path_buffer, sizeof(path_buffer)); 		// Load file "Jeskola Pattern XP.txt"
		_splitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname,
                       _MAX_FNAME, ext, _MAX_EXT );
		sprintf(txt,"%s%s\\Gear\\Generators\\Jeskola Pattern XP.txt", drive, dir);

		CFile cFile(TEXT(txt), CFile::modeRead);
		EDITSTREAM es;
		es.dwCookie = (DWORD) &cFile;
		es.pfnCallback = MyStreamInCallback; 
		// helptext.StreamIn(SF_RTF, es);
		helptext.StreamIn(SF_TEXT, es); 
	}
	
}

int CEditorWnd::GetEditorPatternPosition()
{
	if (pPattern == NULL)
		return 0;
	
	return pe.cursor.row * 4 / pPattern->rowsPerBeat;
}
