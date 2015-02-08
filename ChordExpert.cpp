// ChordExpert.cpp : implementation file
//

#include "stdafx.h"
#include "ChordExpert.h"
#include "EditorWnd.h"


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

	CObject *pOldFont = pDC->SelectObject(&pew->font);
	pDC->SetBkMode(TRANSPARENT);

	CPen pen(PS_SOLID, 1, Blend(textcolor, bgcol, 0.5f));
	CObject *pOldPen = pDC->SelectObject(&pen);

	for (int col = 0; col < ColCount; col++)
		for (int row = 0; row < RowCount; row++)
			OnDrawCell(pDC, col, row);

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
	if (row < (int)SortedChords[col].size()) {
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

void CCEGrid::CursorSelect(int dx, int dy)
{
	CurrentRow = max(min(CurrentRow + dy, RowCount-1), 0);
	CurrentCol = max(min(CurrentCol + dx, ColCount-1), 0);

	MakeVisible((CurrentCol+dx)*ColWidth, (CurrentRow+dy)*RowHeight);

	Invalidate();
}

BOOL CCEGrid::OnKeyDown(UINT nChar)
{
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

	CWnd::OnLButtonDown(nFlags, point);
}

void CCEGrid::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CScrollWnd::OnLButtonDblClk(nFlags, point);

	// Send OK message to dialog
	WPARAM WParam = MAKEWPARAM(IDOK, BN_CLICKED);
    pced->PostMessage(WM_COMMAND, WParam, NULL); 
}



//-----------------------------------------------------------------------------
// CParametersDialog dialog

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
    }
 
    return CDialog::PreTranslateMessage(pMsg);  
}

void CChordExpertDialog::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChordExpertDialog::UpdateWindowSize()
{
	CRect cr;
//	GetClientRect(&cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, cr);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	int cx = cr.Width();
	int cy = cr.Height();

	ceGrid.MoveWindow(0, 0, cx, cy - 36);

	CButton *pc = (CButton *)GetDlgItem(1);
	if (pc!=NULL)
		pc->MoveWindow(cx - 185, cy-32, 60, 23);
	
	pc = (CButton *)GetDlgItem(2);
	if (pc!=NULL)
		pc->MoveWindow(cx -85, cy-32, 60, 23);

	CStatic *pt = (CStatic *)GetDlgItem(3);
	if (pt!=NULL)
		pt->MoveWindow(25, cy-28, 150, 23);

}

BOOL CChordExpertDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	ceGrid.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 10, 10), this, 1, NULL);
	ceGrid.pew = pew;
	ceGrid.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);
	ceGrid.AlwaysShowVerticalScrollBar(true);
	ceGrid.AlwaysShowHorizontalScrollBar(true);

	ceGrid.ShowWindow(SW_SHOW);

	// Resize the dialog to the last saved datas
	CRect dialogrect;
	dialogrect.top = pew->pCB->GetProfileInt("ChordExpertDialog.top", 100);
	dialogrect.left = pew->pCB->GetProfileInt("ChordExpertDialog.left", 100);
	dialogrect.bottom = pew->pCB->GetProfileInt("ChordExpertDialog.bottom", 300);
	dialogrect.right = pew->pCB->GetProfileInt("ChordExpertDialog.right", 300);
	MoveWindow(dialogrect);

	// Get the previous chord of the pattern
	int ir = CursorRow;
	while (ir>=0 && pew->RowNotes[ir].chord_index < 0) ir--;
	
	if (ir >=0)	{
		BaseOctave = pew->RowNotes[ir].base_octave;

		ceGrid.ChordRow = ir;
		// Initialize the grid of chords
		InitNotesGrid();

		char s_basenote[3];
		string s_c;
		s_basenote[0] = NoteTo2char[pew->RowNotes[ir].base_note*2+0];
		s_basenote[1] = NoteTo2char[pew->RowNotes[ir].base_note*2+1];
		s_basenote[2] = 0;
		s_c = s_basenote + pew->ChordsBase[pew->RowNotes[ir].chord_index].name;
		ChordText =  "Current chord : " + s_c;
	}
	else {
		ir = CursorRow;
		while (ir>=0 && pew->RowNotes[ir].base_octave > 10)
			ir--;
		if (ir > 0) BaseOctave = pew->RowNotes[ir].base_octave;
		else BaseOctave = 4;

		ceGrid.ChordRow = -1;
		// Empty the grid of chords
		InitNotesGrid();

		ChordText = "Current chord : Unknown";
	}
	ceGrid.RefreshCanvasSize();

	CStatic *pt = (CStatic *)GetDlgItem(3);
	if (pt!=NULL)
		pt->SetWindowText(ChordText.c_str());


	UpdateWindowSize();
	ceGrid.SetFocus();
	
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
					ceGrid.NotesGrid[ir].delta = ib;
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
}

void CChordExpertDialog::OnOK()
{
	SaveDialogPos();
	// Insert the selected chord in the pattern
	if (ceGrid.CurrentRow < (int)ceGrid.SortedChords[ceGrid.CurrentCol].size()) 
	{
		row_struct rs = ceGrid.SortedChords[ceGrid.CurrentCol][ceGrid.CurrentRow];
		pew->pe.InsertChordNote((BaseOctave << 4) + rs.base_note + 1, rs.chord_index);
	}

	CDialog::OnOK();
}

void CChordExpertDialog::OnCancel()
{
	SaveDialogPos();
	CDialog::OnCancel();
}

