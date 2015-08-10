// RowNumWnd.cpp : implementation file
//

#include "stdafx.h"
#include "RowNumWnd.h"
#include "EditorWnd.h"

// CRowNumWnd

IMPLEMENT_DYNAMIC(CRowNumWnd, CScrollWnd)

CRowNumWnd::CRowNumWnd()
{

}

CRowNumWnd::~CRowNumWnd()
{
}


BEGIN_MESSAGE_MAP(CRowNumWnd, CScrollWnd)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


void CRowNumWnd::OnDraw(CDC *pDC)
{
	CRect clr;
	GetClientRect(&clr);
	CRect cr = GetCanvasRect();

	CRect ur;
	ur.UnionRect(&cr, &clr);

	CRect clipbox;
	pDC->GetClipBox(&clipbox);
	
	COLORREF bgcolor = pew->pCB->GetThemeColor("PE BG");
	COLORREF textcolor = pew->pCB->GetThemeColor("PE Text");

	pDC->FillSolidRect(&ur, bgcolor);

	if (pew->pPattern == NULL)
		return;

	CObject *pOldFont = pDC->SelectObject(&pew->font);

	pDC->SetTextColor(textcolor);

	int const firstrow = max(0, clipbox.top / pew->fontSize.cy);
	int const lastrow = min(pew->pPattern->GetRowCount(), clipbox.bottom / pew->fontSize.cy + 1);
	int const lmargin = pew->ChordExpertvisible ? 60 : 0;
	for (int y = firstrow; y < lastrow; y++)
	{
		if (pew->ChordExpertvisible)
		{ 
			// Draw chords value 
			if (y < (int)pew->RowNotes.size()) 
			{
				if (pew->RowNotes[y].chord_index >= 0)
				{
					string s_chord;
					char s_basenote[3];
					s_basenote[0] = NoteTo2char[pew->RowNotes[y].base_note*2+0];
				    s_basenote[1] = NoteTo2char[pew->RowNotes[y].base_note*2+1];
					s_basenote[2] = 0;
					s_chord =  s_basenote + pew->ChordsBase[pew->RowNotes[y].chord_index].name;
					pDC->TextOut(0, y * pew->fontSize.cy, s_chord.c_str(), s_chord.size());
				}
				else
				if (pew->RowNotes[y].chord_index == -2)
				{
					pDC->TextOut(0, y * pew->fontSize.cy, "unknown", 7);
				}

			}
		}
		CRect r;
		r.left = lmargin;
		r.right = clr.right;
		r.top = y * pew->fontSize.cy;
		r.bottom = r.top + pew->fontSize.cy +1;
		InflateRect(r, -1, -1);

		char buf[10];
		sprintf(buf, "%5d", y);
//		pDC->TextOut(lmargin, y * pew->fontSize.cy, buf, 5);
		pDC->DrawText(buf, &r, DT_RIGHT);

		if (pew->DrawRowNumberButton) {
			InflateRect(r, 1, 1);
			pDC->DrawEdge(r, BDR_RAISEDINNER | BDR_RAISEDOUTER, BF_RECT);
		}
		
	}

	pDC->SelectObject(pOldFont); 
}


// CRowNumWnd message handlers

void CRowNumWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int const lmargin = pew->ChordExpertvisible ? 60 : 0;

	point = ClientToCanvas(point);

	if (point.x >= lmargin) {

		int row = pew->pe.GetRowY(point.y);
		if (row >= 0) {

			bool ctrldown = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;
			bool shiftdown = (GetKeyState(VK_SHIFT) & (1 << 15)) != 0;

			if (!ctrldown)	{
				if (!shiftdown)	pew->pe.SelectRow(row);
				else pew->pe.SelectBeat(row);
			}
			else pew->pe.SelectMesure(row);
		}
	}
	CScrollWnd::OnLButtonDblClk(nFlags, point);
}