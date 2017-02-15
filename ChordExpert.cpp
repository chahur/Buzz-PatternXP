// ChordExpert.cpp : implementation file
//

#include "stdafx.h"
#include "ChordExpert.h"
#include "EditorWnd.h"


//-----------------------------------------------------------------------------
// CCEGrid : Chords display grid

IMPLEMENT_DYNAMIC(CCEGrid, CScrollWnd)

CCEGrid::CCEGrid()
{
	ColCount = 5;
	ColWidth = 80;
	RowCount = 5;
	RowHeight = 23;
	CurrentCol = 0;
	CurrentRow = 0;
}

CCEGrid::~CCEGrid()
{
}


BEGIN_MESSAGE_MAP(CCEGrid, CScrollWnd)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()

END_MESSAGE_MAP()


int CCEGrid::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CCEGrid::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	pced = pParentWnd;
	return CScrollWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

void CCEGrid::RefreshCanvasSize()
{
	SetCanvasSize(CSize(ColCount*ColWidth, RowCount*RowHeight));
}

void CCEGrid::OnDraw(CDC *pDC)
{
	CRect clr;
	GetClientRect(&clr);
	CRect cr = GetCanvasRect();
	CRect ur;
	ur.UnionRect(&cr, &clr);

	CRect cpr;
	pDC->GetClipBox(&cpr);

	COLORREF bgcol = pew->pCB->GetThemeColor("PE BG");
/*	bgcoldark = pew->pCB->GetThemeColor("PE BG Dark");
	bgcolvdark = pew->pCB->GetThemeColor("PE BG Very Dark");
	bgsel = pew->pCB->GetThemeColor("PE Sel BG");
	*/
	COLORREF textcolor = pew->pCB->GetThemeColor("PE Text");
	pDC->FillSolidRect(&ur, bgcol);
	pDC->SetTextColor(textcolor);

	CObject *pOldFont = pDC->SelectObject(&pew->font);
	pDC->SetBkMode(TRANSPARENT);

	CPen pen(PS_SOLID, 1, Blend(textcolor, bgcol, 0.5f));
	CObject *pOldPen = pDC->SelectObject(&pen);

	for (int col = 0; col < ColCount; col++)
		for (int row = 0; row < RowCount; row++)
			OnDrawCell(pDC, col, row);

	DisplaySelectedChord(CurrentCol, CurrentRow);

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);

}


void CCEGrid::OnDrawCell(CDC *pDC, int col, int row)
{
	CRect cellrect;
	cellrect.top    = row * RowHeight;
	cellrect.left   = col * ColWidth;
	cellrect.bottom = cellrect.top  + RowHeight;
	cellrect.right  = cellrect.left + ColWidth;

	if (col==CurrentCol && row==CurrentRow)
	{
		COLORREF bgsel = pew->pCB->GetThemeColor("PE Sel BG");
		pDC->FillSolidRect(&cellrect, bgsel);

		pDC->DrawEdge(cellrect, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);
	}
	else
		pDC->DrawEdge(cellrect, BDR_RAISEDINNER, BF_RECT|BF_FLAT);
	char s[48];
	GetCellText(s, col, row);
	pDC->DrawText(s, &cellrect, DT_VCENTER|DT_SINGLELINE|DT_CENTER);

}

void CCEGrid::GetCellText(LPSTR sText, int col, int row)
{
	if (row < (int)SortedChords[col].size()) 
	{
		row_struct rs = SortedChords[col][row];
		char s_basenote[3];
		string s_c;
		s_basenote[0] = NoteTo2char[rs.base_note*2+0];
		s_basenote[1] = NoteTo2char[rs.base_note*2+1];
		s_basenote[2] = 0;
		s_c = s_basenote + pew->ChordsBase[rs.chord_index].name;
		strcpy(sText, s_c.c_str());
	}
	else
		sText[0] = 0;
}

void CCEGrid::MoveToChord(int note, string name)
{
	for (int col = 0; col < ColCount; col++)
		for (int row = 0; row < RowCount; row++)
			if (row < (int)SortedChords[col].size()) {
				row_struct rs = SortedChords[col][row];
				if ((note == rs.base_note) && (name == pew->ChordsBase[rs.chord_index].name)) {
					int dx = col - CurrentCol;
					int dy = row - CurrentRow;
					CursorSelect(dx, dy);
					return;
				}
			}
}

void CCEGrid::CursorSelect(int dx, int dy)
{
	CurrentRow = max(min(CurrentRow + dy, RowCount-1), 0);
	CurrentCol = max(min(CurrentCol + dx, ColCount-1), 0);

	MakeVisible((CurrentCol+dx)*ColWidth, (CurrentRow+dy)*RowHeight);

	Invalidate();
}

BOOL CCEGrid::OnKeyDown(UINT nChar)
{
	bool ctrldown = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;
	bool shiftdown = (GetKeyState(VK_SHIFT) & (1 << 15)) != 0;
	bool altdown = (GetKeyState(VK_MENU) & (1 << 15)) != 0;

	if (ctrldown || altdown || shiftdown) return false;

	switch(nChar)
	{
	case VK_UP:    CursorSelect(0, -1); break;
	case VK_DOWN:  CursorSelect(0, 1);  break;
	case VK_LEFT:  CursorSelect(-1, 0); break;
	case VK_RIGHT: CursorSelect(1, 0);  break;
	default : return false;
	}
	return true;
}

void CCEGrid::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	
	int col = (GetScrollPos().x + point.x) / ColWidth;
	int row = (GetScrollPos().y + point.y) / RowHeight;
	if (col >=0 && col < ColCount && row >=0 && row < RowCount) {
		CurrentRow = row;
		CurrentCol = col;
		Invalidate();
	}

	CScrollWnd::OnLButtonDown(nFlags, point);
}

void CCEGrid::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CScrollWnd::OnLButtonDblClk(nFlags, point);

	// Send OK message to dialog
	WPARAM WParam = MAKEWPARAM(IDOK, BN_CLICKED);
    pced->PostMessage(WM_COMMAND, WParam, NULL); 
}


void CCEGrid::DisplaySelectedChord(int col, int row)
{
	char sText[48];
	sText[0] = 0;

	if ((col >=0) && (col < (int)SortedChords.size()) &&
		(row >=0) && (row <(int)SortedChords[col].size()))
	{
		row_struct rs = SortedChords[col][row];
		char s_basenote[3];
		string s_c;
		s_basenote[0] = NoteTo2char[rs.base_note*2+0];
		s_basenote[1] = NoteTo2char[rs.base_note*2+1];
		s_basenote[2] = 0;

		char SelectedChord[20];
		strcpy(SelectedChord, pew->Chords[rs.chord_index].c_str());

		sprintf(sText, "Selected : %s %s",s_basenote, SelectedChord);
	}
	
	if (labelChord!=NULL)
		labelChord->SetWindowText(sText);
}



//-----------------------------------------------------------------------------
// CChordExpertDialog dialog

IMPLEMENT_DYNAMIC(CChordExpertDialog, CDialog)

CChordExpertDialog::CChordExpertDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CChordExpertDialog::IDD, pParent)
{

}

CChordExpertDialog::~CChordExpertDialog()
{
}


BEGIN_MESSAGE_MAP(CChordExpertDialog, CDialog)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(5, OnBnClickedUp)
	ON_BN_CLICKED(6, OnBnClickedDown) 
	ON_BN_CLICKED(7, OnBnClickedClear) 
	ON_LBN_SELCHANGE(9, OnListBoxSelectionChange)
	ON_BN_CLICKED(IDD_SORTBYNOTE, OnBnClickedSortByNote) 
	ON_BN_CLICKED(IDD_SORTBYDISTANCE, OnBnClickedSortByDistance) 
	ON_CBN_SELENDOK(IDC_COMBO1, OnComboChordsProgressionSelect)
	ON_BN_CLICKED(IDC_GENERATE, OnBnClickedGenerate)
	ON_LBN_DBLCLK(IDC_LIST1, OnListBoxChordsProgressionDblClick)
END_MESSAGE_MAP()


void CChordExpertDialog::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect cr;
//	GetClientRect(&cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	UpdateWindowSize();
}

// To trap the WM_KEYDOWN message : overwrite the PreTranslateMessage of CDialog.
BOOL CChordExpertDialog::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message==WM_KEYDOWN)  {
		if (ceGrid.OnKeyDown(pMsg->wParam)) return true;
		else if (OnKeyDown(pMsg->wParam)) return true;
    }
 
    return CDialog::PreTranslateMessage(pMsg);  
}

BOOL CChordExpertDialog::OnKeyDown(UINT nChar)
{
	bool ctrldown = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;
	bool shiftdown = (GetKeyState(VK_SHIFT) & (1 << 15)) != 0;
	bool altdown = (GetKeyState(VK_MENU) & (1 << 15)) != 0;

	if (ctrldown ) 
	{
		switch(nChar)
		{
		case VK_UP:      OnBnClickedUp(); break;
		case VK_DOWN:    OnBnClickedDown();  break;
		case VK_PRIOR:   pew->pe.MoveCursorPgUpDown(-1); 
			CursorRow = pew->pe.cursor.row;
			InitGrid(SortBy);
			break;
		case VK_NEXT:    pew->pe.MoveCursorPgUpDown(1); 
			CursorRow = pew->pe.cursor.row;
			InitGrid(SortBy);
			break;
		case VK_DELETE:  pew->pe.ClearRow(); break;		

		default : return false;
		}
		return true;
	}
	else
		switch (nChar)	{
		case VK_F5:  pew->pCB->Play(); break;
		// case VK_F6:  pew->pCB->Play(); break;
		case VK_F8:  pew->pCB->Stop(); break;
		default: return false;
		}
	return true;
}

void CChordExpertDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChordExpertDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	CPoint p = point;
	int d;

	if (Move_Splitter) {
		d = ArpPosX - p.x;
		if (d != 0) {
			ArpWidth = ArpWidth + d;
			UpdateWindowSize();
			UpdateWindow();
		}
		
	}
	else if (Move_SplitterProg) {
		d = ProgPosX - p.x;
		if (d != 0) {
			ChordsProgressionWidth = ChordsProgressionWidth - d;
			UpdateWindowSize();
			UpdateWindow();
		}
	}
	else if (p.y > 21) New_Cursor = 0; // Not in the upper part of the dialog
	else if ((p.x > ArpPosX - SPLITTER_WIDTH-2)  && (p.x < ArpPosX)) New_Cursor = 1; // Arpeggio splitter
	else if ((p.x > ProgPosX - SPLITTER_WIDTH-2) && (p.x < ProgPosX)) New_Cursor = 2; // Chords progression splitter
	else New_Cursor = 0; // IDC_ARROW
		
	CDialog::OnMouseMove(nFlags, point);
}

void CChordExpertDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	Move_Splitter = (Current_Cursor == 1);
	Move_SplitterProg = (Current_Cursor == 2);
	CDialog::OnLButtonDown(nFlags, point);
}

void CChordExpertDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	Move_Splitter = false;
	Move_SplitterProg = false;
	CDialog::OnLButtonUp(nFlags, point);
}

BOOL CChordExpertDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (nHitTest != HTCLIENT)
		if (!Move_Splitter && !Move_SplitterProg) New_Cursor = 0;

	Current_Cursor = New_Cursor;
	if (New_Cursor != 0) {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		return TRUE;
	}
	else return CDialog::OnSetCursor(pWnd, nHitTest, message);		
}

void CChordExpertDialog::UpdateWindowSize()
{
	CRect cr;
//	GetClientRect(&cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	int cx = cr.Width();
	int cy = cr.Height();

	// Validate the width of the Arpeggio list
	if (ArpWidth < ARP_WIDTH_MIN) ArpWidth = ARP_WIDTH_MIN;
	if (ArpWidth > cx - GRID_WIDTH_MIN) ArpWidth = cx - GRID_WIDTH_MIN;

	// Validate the width of the Progression list
	if (ChordsProgressionWidth < ARP_WIDTH_MIN) ChordsProgressionWidth = ARP_WIDTH_MIN;
	if (ChordsProgressionWidth > cx - GRID_WIDTH_MIN) ChordsProgressionWidth = cx - GRID_WIDTH_MIN;

	// The chords grid
	ceGrid.MoveWindow(ChordsProgressionWidth + SPLITTER_WIDTH, 21, cx - ArpWidth - ChordsProgressionWidth - 2* SPLITTER_WIDTH, cy - MARGIN_HEIGHT);

	// The buttons
	CButton *pc = (CButton *)GetDlgItem(1); // Insert
	if (pc!=NULL) pc->MoveWindow(cx - 145, cy-32, 60, 23, 1);
	
	pc = (CButton *)GetDlgItem(2);  // Close
	if (pc!=NULL) pc->MoveWindow(cx -75, cy-32, 60, 23, 1);

	pc = (CButton *)GetDlgItem(5);  // Up
	if (pc!=NULL) pc->MoveWindow(cx -265, cy-32, 32, 23, 1);

	pc = (CButton *)GetDlgItem(6);  // Down
	if (pc!=NULL) pc->MoveWindow(cx -227, cy-32, 38, 23, 1);

	pc = (CButton *)GetDlgItem(7);  // Clear
	if (pc != NULL) pc->MoveWindow(cx - 185, cy - 32, 32, 23, 1);

	pc = (CButton *)GetDlgItem(IDC_GENERATE);  // Generate
	if (pc != NULL) pc->MoveWindow(4, cy - 32, 64, 23, 1);

	CStatic *pt = (CStatic *)GetDlgItem(12);  // Octave label (l 45)
	if (pt != NULL) pt->MoveWindow(ChordsProgressionWidth + 12, cy - 28, 45, 23);

	CComboBox * pcb = (CComboBox *)GetDlgItem(13);  // Current octave (l 34)
	if (pcb != NULL) pcb->MoveWindow(ChordsProgressionWidth + 60, cy - 32, 34, 23);

	pt = (CStatic *)GetDlgItem(3);  // Current chord (l 150)
	if (pt != NULL) pt->MoveWindow(ChordsProgressionWidth + 100, cy - 28, 150, 23);

	ceGrid.labelChord = (CStatic *)GetDlgItem(4);  // Selected chord (l 150)
	if (ceGrid.labelChord != NULL) ceGrid.labelChord->MoveWindow(ChordsProgressionWidth + 245, cy - 28, 150, 23);

	// 10 Sort by note
	pc = (CButton *)GetDlgItem(10);
	if (pc != NULL) pc->MoveWindow(ChordsProgressionWidth +30, 0, 100, 21);
	// 11 Sort by distance
	pc = (CButton *)GetDlgItem(11);
	if (pc != NULL) pc->MoveWindow(ChordsProgressionWidth + 130, 0, 120, 21);

	// Splitters
	pt = (CStatic *)GetDlgItem(14);  // Separator
	if (pt != NULL) pt->MoveWindow(cx - ArpWidth - SPLITTER_WIDTH - 8, 3, 20, 17);

	pt = (CStatic *)GetDlgItem(15);  // Separator
	if (pt != NULL) pt->MoveWindow(ChordsProgressionWidth - SPLITTER_WIDTH, 3, 20, 17);

	// Arpeggio list
	pt = (CStatic *)GetDlgItem(8);  // Arpeggio
	if (pt!=NULL) pt->MoveWindow(cx - ArpWidth + 40, 5, 60, 23);

	CListBox * pl = (CListBox *)GetDlgItem(9);  // List of Arpeggio 
	if (pl!=NULL) pl->MoveWindow(cx - ArpWidth -1, 21, ArpWidth, cy - 58);
	ArpPosX = cx - ArpWidth;

	// Chord progression selection
	comboChordsProgression = (CComboBox *)GetDlgItem(IDC_COMBO1); 
	if (comboChordsProgression != NULL) comboChordsProgression->MoveWindow(1, 22, ChordsProgressionWidth - 1, 20);

	// Chord progression description
	listChordsProgression = (CListBox *)GetDlgItem(IDC_LIST1);
	if (listChordsProgression != NULL) listChordsProgression->MoveWindow(1, 44, ChordsProgressionWidth - 1, cy - MARGIN_HEIGHT - 22);
	ProgPosX = ChordsProgressionWidth+4;
}

void CChordExpertDialog::OnBnClickedSortByNote()
{
	SortBy = SORT_BYNOTE;
	InitGrid(SORT_BYNOTE);
}

void CChordExpertDialog::OnBnClickedSortByDistance()
{
	SortBy = SORT_BYDISTANCE;
	InitGrid(SORT_BYDISTANCE);
}

void CChordExpertDialog::InitGrid(int SortType)
{
/*
#define SORT_UNDEFINED 0
#define SORT_BYNOTE 1
#define SORT_BYDISTANCE 2
*/
	// Get the previous chord of the pattern
	int ir = CursorRow-1;
	while (ir>=0 && pew->RowNotes[ir].chord_index < 0) ir--;
	
	CButton *pc;
	pc = (CButton *)GetDlgItem(IDD_SORTBYDISTANCE);
	pc->EnableWindow(TRUE);

	if ((pew->TonalComboIndex >0) && (pew->TonalComboIndex < (int)pew->TonalityList.size() ))
		// Start with Tonality root note of major mode
		if (pew->TonalityList[pew->TonalComboIndex].major)
			BaseNote = pew->TonalityList[pew->TonalComboIndex].base_note;
		else
		{
			BaseNote = pew->TonalityList[pew->TonalComboIndex].base_note - 3;
			if (BaseNote<0) BaseNote = BaseNote+12;
		}
	else
		BaseNote=0; // Start with C

	if (ir >=0)
	{
		BaseOctave = pew->RowNotes[ir].base_octave;
	
		char s_basenote[3];
		string s_c;
		s_basenote[0] = NoteTo2char[pew->RowNotes[ir].base_note*2+0];
		s_basenote[1] = NoteTo2char[pew->RowNotes[ir].base_note*2+1];
		s_basenote[2] = 0;
		s_c = s_basenote + pew->ChordsBase[pew->RowNotes[ir].chord_index].name;
		ChordText =  "Current chord : " + s_c;
	}
	else
	{
		pc = (CButton *)GetDlgItem(IDD_SORTBYDISTANCE);
		pc->EnableWindow(FALSE);
		int i = CursorRow;
		while (i>=0 && pew->RowNotes[i].base_octave > 10)
			i--;
		if (i > 0) BaseOctave = pew->RowNotes[i].base_octave;
		else BaseOctave = 4;

		ChordText = "Current chord : Unknown";
	}

	if ((ir >=0) && (SortType==SORT_UNDEFINED || SortType==SORT_BYDISTANCE))
	{
		pc = (CButton *)GetDlgItem(IDD_SORTBYDISTANCE);
		pc->SetCheck(BST_CHECKED);
		pc = (CButton *)GetDlgItem(IDD_SORTBYNOTE);
		pc->SetCheck(BST_UNCHECKED);

		ceGrid.ChordRow = ir;
		// Initialize the grid of chords
		InitNotesGrid();
	}
	else 
	{
		pc = (CButton *)GetDlgItem(IDD_SORTBYNOTE);
		pc->SetCheck(BST_CHECKED);
		pc = (CButton *)GetDlgItem(IDD_SORTBYDISTANCE);
		pc->SetCheck(BST_UNCHECKED);

		ceGrid.ChordRow = -1;
		// Empty the grid of chords
		InitNotesGrid();

	}
	ceGrid.RefreshCanvasSize();

	// Display current chord
	CStatic *pt = (CStatic *)GetDlgItem(3);
	if (pt!=NULL)
		pt->SetWindowText(ChordText.c_str());
	// Display current base octave
	comboOctave->SetCurSel(BaseOctave-1);

	UpdateWindowSize();
	ceGrid.Invalidate();
	ceGrid.SetFocus();
}

BOOL CChordExpertDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	New_Cursor = 0;
	Move_Splitter = false;
	Move_SplitterProg = false;

	ceGrid.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	ceGrid.pew = pew;
	ceGrid.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	ceGrid.AlwaysShowVerticalScrollBar(true);
	ceGrid.AlwaysShowHorizontalScrollBar(true);

	ceGrid.ShowWindow(SW_SHOW);

	ArpWidth = pew->pCB->GetProfileInt("ArpWidth", ARP_WIDTH);
	ChordsProgressionWidth = pew->pCB->GetProfileInt("ChordsProgressionWidth", ARP_WIDTH);

	// Resize the dialog to the last saved datas
	CRect dialogrect;
	dialogrect.top = pew->pCB->GetProfileInt("ChordExpertDialog.top", 100);
	dialogrect.left = pew->pCB->GetProfileInt("ChordExpertDialog.left", 100);
	dialogrect.bottom = pew->pCB->GetProfileInt("ChordExpertDialog.bottom", 300);
	dialogrect.right = pew->pCB->GetProfileInt("ChordExpertDialog.right", 300);
	MoveWindow(dialogrect);
	
	// Load Octave combo
	comboOctave = (CComboBox *)GetDlgItem(13);
	comboOctave->AddString("1");
	comboOctave->AddString("2");
	comboOctave->AddString("3");
	comboOctave->AddString("4");
	comboOctave->AddString("5");
	comboOctave->AddString("6");
	comboOctave->AddString("7");
	comboOctave->AddString("8");


	// Load the list of arpeggios
	CListBox * pl = (CListBox *)GetDlgItem(9);  // List of Arpeggio 
	if (pl!=NULL)  {
		POSITION pos;
		for (pos = pew->SLArpeggio->GetHeadPosition(); pos != NULL; ) {
			pl->AddString(pew->SLArpeggio->GetNext(pos));
		}
		// Set it to the current selected arpeggio
		pl->SetCurSel(pew->ArpeggioComboIndex);
	}
	
	// Initialize the chord grid according to the current chord
	InitGrid(SORT_UNDEFINED);

	// Load the list of Chords progression
	comboChordsProgression = (CComboBox *)GetDlgItem(IDC_COMBO1); 
	if (comboChordsProgression != NULL)
		gChordsProgression->LoadList(comboChordsProgression);

	listChordsProgression = (CListBox *)GetDlgItem(IDC_LIST1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CChordExpertDialog::InitNotesGrid()
{
	// Get current chord notes
	note_bitset nStartcur;
	if (ceGrid.ChordRow>=0)
	{
		nStartcur = pew->ChordsBase[pew->RowNotes[ceGrid.ChordRow].chord_index].notes;
		for (int i=0; i<pew->RowNotes[ceGrid.ChordRow].base_note; i++)
		{
			bool n0 = nStartcur[11];
			nStartcur = (nStartcur<<1);
			nStartcur.set(0, n0);
		}
	}
	else
	{
	}

	ceGrid.NotesGrid.clear();
	ceGrid.NotesGrid.resize(12*(int)pew->ChordsBase.size());
	int maxdelta = 0;
	if (ceGrid.ChordRow<0) maxdelta=11;
	int ir = 0;
	bool inTonality = true;

	for (int ic=0; ic<(int)pew->ChordsBase.size(); ic++) {
		// Read the notes of this chord
		note_bitset ncur = nStartcur;
		note_bitset n;
		n = pew->ChordsBase[ic].notes;
		for (int ib=0; ib<12; ib++) {
			ir = ic + ib*(int)pew->ChordsBase.size(); 
			ceGrid.NotesGrid[ir].notes = n;
			ceGrid.NotesGrid[ir].chord_index = ic;
			ceGrid.NotesGrid[ir].base_note = ib;

			// Check if the chord is in the current tonality
			inTonality = true;
			if ((pew->TonalComboIndex >0) && 
			    (pew->TonalComboIndex < (int)pew->TonalityList.size() )) 
			{
				note_bitset nor;
				nor = n;
				nor |= pew->TonalityList[pew->TonalComboIndex].notes;
				if (nor != pew->TonalityList[pew->TonalComboIndex].notes) 
					// not all notes of the chord are in the tonality
					inTonality = false;
			}

			// Compute the delta to the current chord
			if (ceGrid.ChordRow>=0)
			{
				if (ncur == n)
					ceGrid.NotesGrid[ir].delta = 0;
				else {
					int d1 = 0;
					int d2 = 0;
					int d3 = 0;
					for (int i = 0; i<12; i++) {
						// d1 : count of common notes
						if (ncur.test(i) && n.test(i)) d1++;
						// d2 : count of added notes
						if (!ncur.test(i) && n.test(i)) d2++;
						// d3 : count of suppressed notes
						if (ncur.test(i) && !n.test(i)) d3++;					
					}
					int delta = d2 + d3; // - d1;
					if (maxdelta < delta) maxdelta = delta;

					if (inTonality)
						ceGrid.NotesGrid[ir].delta = delta;
					else
						ceGrid.NotesGrid[ir].delta = 100;

				}
			}
			else
			{
				if (inTonality)
				{
					// Sort according to BaseNote
					ceGrid.NotesGrid[ir].delta = ib+BaseNote;
					if (ceGrid.NotesGrid[ir].delta>=12) 
						ceGrid.NotesGrid[ir].delta = ceGrid.NotesGrid[ir].delta-12;
				}
				else
					ceGrid.NotesGrid[ir].delta = 100;
			}
			
			// Next base note
			bool n0 = n[11];
			n = (n<<1);
			n.set(0, n0);

		}
	}
	 
	// Now, sort the NotesGrid by delta
	ceGrid.SortedChords.clear();
	ceGrid.SortedChords.resize(maxdelta+1);

	for (int i = 0; i < (int)ceGrid.NotesGrid.size(); i++) {
		row_struct cs;
		cs.base_note = ceGrid.NotesGrid[i].base_note;
		cs.chord_index = ceGrid.NotesGrid[i].chord_index;
		cs.notes = ceGrid.NotesGrid[i].notes;
		// Discard chords out of tonality
		if (ceGrid.NotesGrid[i].delta < 100)
			ceGrid.SortedChords[ceGrid.NotesGrid[i].delta].push_back(cs);
	}

	// Discard empty cols
	for (int i=maxdelta; i>=0 ; i--)
		if ((int)ceGrid.SortedChords[i].size()==0)
			ceGrid.SortedChords.erase(ceGrid.SortedChords.begin()+i);


	// Resize the grid to display the chords		
	int rc = 0;
	for (int i=0; i < (int)ceGrid.SortedChords.size(); i++)
		rc = max(rc, (int)ceGrid.SortedChords[i].size()); 
	ceGrid.RowCount = rc;
	ceGrid.ColCount = (int)ceGrid.SortedChords.size();

}

void CChordExpertDialog::SaveDialogPos()
{
	CRect dialogrect;
	GetWindowRect(dialogrect);
	pew->pCB->WriteProfileInt("ChordExpertDialog.top", dialogrect.top);
	pew->pCB->WriteProfileInt("ChordExpertDialog.left", dialogrect.left);
	pew->pCB->WriteProfileInt("ChordExpertDialog.bottom", dialogrect.bottom);
	pew->pCB->WriteProfileInt("ChordExpertDialog.right", dialogrect.right);

	pew->pCB->WriteProfileInt("ArpWidth", ArpWidth);
	pew->pCB->WriteProfileInt("ChordsProgressionWidth", ChordsProgressionWidth);
	

}

void CChordExpertDialog::OnOK()
{
	SaveDialogPos();
	// Insert the selected chord in the pattern
	if (ceGrid.CurrentRow < (int)ceGrid.SortedChords[ceGrid.CurrentCol].size()) 
	{
		row_struct rs = ceGrid.SortedChords[ceGrid.CurrentCol][ceGrid.CurrentRow];
		
		// Get octave to use
		int bo = comboOctave->GetCurSel()+1;
		if (bo < 1) bo = 1;
		if (bo > 8) bo = 8;
		
		// Insert the chord
		pew->pe.InsertChordNote((bo << 4) + rs.base_note + 1, rs.chord_index);

		// Refresh the chord expert
		if (!pew->AutoChordExpert) pew->AnalyseChords();

	}

//	CDialog::OnOK(); do not close the dialog
}

void CChordExpertDialog::OnCancel()
{
	SaveDialogPos();
	CDialog::OnCancel();
}

void CChordExpertDialog::OnBnClickedUp()
{
	// Move cursor 1 row up
	pew->pe.MoveCursorUpDown(-1);
	CursorRow = pew->pe.cursor.row;
	InitGrid(SortBy);
}

void CChordExpertDialog::OnBnClickedDown()
{
	// Move cursor 1 row down
	pew->pe.MoveCursorUpDown(1);
	CursorRow = pew->pe.cursor.row;
	InitGrid(SortBy);
}

void CChordExpertDialog::OnBnClickedClear()
{
	// Clear current pattern row
	pew->pe.ClearRow();
	InitGrid(SortBy);
}

void CChordExpertDialog::OnListBoxSelectionChange()
{
	CListBox * pl = (CListBox *)GetDlgItem(9);  // List of Arpeggio 
	if (pl!=NULL) {
		int index = pl->GetCurSel();	
		pew->SetComboBoxArpeggio(index);
		pew->OnComboArpeggioSelect();
	}
}

void CChordExpertDialog::OnComboChordsProgressionSelect()
{
	CString txt;
	comboChordsProgression->GetLBText(comboChordsProgression->GetCurSel(), txt);
	gChordsProgression->LoadChordsProgression(listChordsProgression, txt);

}

void CChordExpertDialog::OnBnClickedGenerate()
{
	CString txt;
	int i = comboChordsProgression->GetCurSel();
	if (i>=0) comboChordsProgression->GetLBText(i, txt);
	if ((i<0) || (txt == "")) {
		CString msg = "A chord progression must be selected.";
		AfxMessageBox(msg, MB_OK);
		return;
	}

	// Get octave to use
	int bo = comboOctave->GetCurSel() + 1;
	if (bo < 1) bo = 1;
	if (bo > 8) bo = 8;
	gChordsProgression->GenerateChordsProgression(txt, bo, pew);

}

void CChordExpertDialog::OnListBoxChordsProgressionDblClick()
{
	int index = listChordsProgression->GetCurSel();
	if (index >= 0) {
		// Move the cursor to the position defined in the progression
		int position = gChordsProgression->GetCurrentProgressionChord_position(index);
		if (position >=0)
			pew->pe.MoveCursorRow(position);

		// Select the current chord in the grid
		int note = gChordsProgression->GetCurrentProgressionChord_note(index);
		string chord = gChordsProgression->GetCurrentProgressionChord_chord(index);
		if ((note>=0) && (chord !=""))
		  ceGrid.MoveToChord(note, chord);
	}
}


