// TopWnd.cpp : implementation file
//

#include "stdafx.h"
#include "TopWnd.h"
#include "EditorWnd.h"
#include <io.h>

// CTopWnd

IMPLEMENT_DYNAMIC(CTopWnd, CScrollWnd)

CTopWnd::CTopWnd()
{

}

CTopWnd::~CTopWnd()
{
}


BEGIN_MESSAGE_MAP(CTopWnd, CScrollWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


void CTopWnd::OnDraw(CDC *pDC)
{
	CRect clr;
	GetClientRect(&clr);
	CRect cr = GetCanvasRect();

	CRect ur;
	ur.UnionRect(&cr, &clr);

	
	COLORREF bgcolor = pew->pCB->GetThemeColor("PE BG");
	COLORREF textcolor = pew->pCB->GetThemeColor("PE Text");

	pDC->FillSolidRect(&ur, bgcolor);

	CMachinePattern *ppat = pew->pPattern;

	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CObject *pOldFont = pDC->SelectObject(&pew->font);

	CPen pen(PS_SOLID, 1, Blend(textcolor, bgcolor, 0.5f));
	CObject *pOldPen = pDC->SelectObject(&pen);

	// Create Vertical font
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT)); // clear out structure.
	lf.lfHeight = 70; // request a 7-pixel-height font
	lf.lfEscapement = 900; // Rotate 90°
	_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, _T("Tahoma"), 7);

	CFont myVerticalFont;
	VERIFY(myVerticalFont.CreatePointFontIndirect(&lf, pDC));   

	LOGFONT lfw;
	memset(&lfw, 0, sizeof(LOGFONT)); // clear out structure.
	lfw.lfHeight = 90;
	lfw.lfCharSet = SYMBOL_CHARSET;
	strcpy(lfw.lfFaceName, _T("Webdings"));
	CFont myWingFont;
	myWingFont.CreatePointFontIndirect(&lfw, pDC);

	CColumn *lastcolumn = ppat->columns[0].get();
	int startx = 0;
	int macstartx = 0;
	int x = 0;

	for (int col = 0; col <= (int)ppat->columns.size(); col++)
	{
		CColumn *pc = NULL;
		if (col < (int)ppat->columns.size())
			pc = ppat->columns[col].get();

		if (col == 0)
		{ // Show / hide toolbars button
			CRect r;
			r.left = 0;
			r.right = pew->pe.GetColumnWidth(0);
			r.top = 0;
			r.bottom = pew->fontSize.cy;
			
			CString s;
			if (!pew->toolbarhidden) {
				s = (char)0x35;//72;
			}
			else s = (char)0x36;//73;

			CFont *pOldFont = pDC->SelectObject(&myWingFont);
		
			pDC->DrawText(s, &r, DT_VCENTER | DT_SINGLELINE | DT_CENTER);

			pDC->SelectObject(pOldFont);

			pDC->DrawEdge(r, BDR_RAISEDINNER | BDR_RAISEDOUTER, BF_RECT);

		}
		
		if (pc == NULL || ((x - startx) > 0 && !pc->MatchGroupAndTrack(*lastcolumn)))
		{
			if (lastcolumn->IsTrackParam())
			{
				// draw track number
				if (pew->pPattern->IsTrackMuted(lastcolumn))
					pDC->SetTextColor(Blend(textcolor, bgcolor, 0.5f));
				else
					pDC->SetTextColor(textcolor);

				CRect r;
				r.left = startx;
				r.right = x - pew->fontSize.cx;
				r.top = pew->fontSize.cy;
				r.bottom = r.top + pew->fontSize.cy;

				CString s;
				s.Format("%d", lastcolumn->GetTrack());

				pDC->DrawText(s, &r, DT_CENTER);
				InflateRect(r,1,1);
				pDC->DrawEdge(r, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);

				pDC->SetTextColor(textcolor);
				pDC->SetBkMode(TRANSPARENT);

				if (pew->ShowTrackToolbar) {
					int rl = r.left;
					int rr = r.right;
					char tbc[5] = "CXPM";
					r.top = r.bottom + 1;
					r.bottom = r.bottom + 16;
					pDC->FillSolidRect(&r, bgcolor);
					int tbbw = r.Width() / 4;
					r.right = r.left + tbbw;
					for (int i=0; i<4; i++){
						pDC->DrawEdge(r, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);
						if ((i>1) && (!pew->pe.CanPaste()))
							pDC->SetTextColor(Blend(textcolor, bgcolor, 0.5f));
						else
							pDC->SetTextColor(textcolor);
						char txt[2];
						txt[0] = tbc[i];
						txt[1] = 0;
						pDC->DrawText(txt, &r, DT_CENTER);
						r.left = r.left + tbbw;
						r.right = r.right + tbbw;
					}
					r.left = rl;
					r.right = rr;
				}

				if (pew->ShowParamText) {
					r.top = r.bottom + 1;
					r.bottom = r.bottom + 72;
					pDC->DrawEdge(r, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);
				}

			}
			else
			{
				CRect r;
				r.left = startx;
				r.right = x - pew->fontSize.cx;
				r.top = pew->fontSize.cy;
				r.bottom = r.top + pew->fontSize.cy;

				CString s;
				s.Format("Global");

				pDC->DrawText(s, &r, DT_CENTER);			
			}
 
			startx = x;
		}

		if (pc == NULL || !pc->MatchMachine(*lastcolumn))
		{
			// draw machine name
			pDC->SetTextColor(textcolor);

			CRect r;
			r.left = macstartx;
			r.right = x - pew->fontSize.cx;
			r.top = 0;
			r.bottom = pew->fontSize.cy;
			pDC->DrawText(ppat->columns[col-1]->GetMachineName(), &r, DT_CENTER | DT_WORD_ELLIPSIS);

			if (pc != NULL)
			{
				// draw a vertical separator between machines
				int sx = x - pew->fontSize.cx / 2;
				pDC->MoveTo(sx, 0);
				pDC->LineTo(sx, clr.bottom);
			}

			macstartx = x;
		}

		if ((pew->ShowParamText) && (pc != NULL))
		{   // Draw param description
			// x : start of col

			CRect r;
			r.left = x;
			r.right = x + pew->pe.GetColumnWidth(col);
			r.top = 2*pew->fontSize.cy + 4;
			r.bottom = r.top + 77;
			if (pew->ShowTrackToolbar) r.bottom = r.bottom + 16; 
			
			if (pew->pPattern->IsTrackMuted(pc))
				pDC->SetTextColor(Blend(textcolor, bgcolor, 0.5f));
			else 
				pDC->SetTextColor(textcolor);

			// Use vertical font
			CFont* pOldFont = pDC->SelectObject(&myVerticalFont);

			CString s;
			s = pc->GetName();
			pDC->DrawText(s, &r, DT_BOTTOM|DT_SINGLELINE);

			// Restore prev font
			pDC->SelectObject(pOldFont);
		}

		if (pc == NULL)
			break;

//		x += (pc->GetDigitCount() + 1) * pew->fontSize.cx;
		x += pew->pe.GetColumnWidth(col);
		
		lastcolumn = pc;

	}

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
}

void CTopWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	point = ClientToCanvas(point);
	if (point.y < pew->fontSize.cy)
	{
		int col = pew->pe.GetColumnAtX(point.x);
		if (col <= 0) // Button to hide / show the toolbars
			return;

		bool ctrldown = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;

		if (ctrldown) {
			// Help file for this machine
			char path_buffer[_MAX_PATH];
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char FullFilename[_MAX_FNAME];
			char ext[_MAX_EXT];

			HMODULE hm = (HMODULE)pew->pCB->GetMachineModuleHandle(pew->pPattern->columns[col]->GetMachine());
			GetModuleFileName(hm, path_buffer, sizeof(path_buffer));
			_splitpath_s( path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, fname,
							_MAX_FNAME, ext, _MAX_EXT );
			sprintf(FullFilename,"%s%s%s%s", drive, dir, fname, ".htm");
			// Check if the file exists
			if (_access (FullFilename, 0) == 0) 
				ShellExecute(NULL, _T("open"), FullFilename, NULL, NULL, SW_SHOWNORMAL);
			else {
				sprintf(FullFilename,"%s%s%s%s", drive, dir, fname, ".html");
				if (_access (FullFilename, 0) == 0) 
					ShellExecute(NULL, "open", FullFilename, NULL, NULL, SW_SHOWNORMAL);
				else {
					sprintf(FullFilename,"%s%s%s%s", drive, dir, fname, ".txt");
					if (_access (FullFilename, 0) == 0) 
						ShellExecute(NULL, "open", FullFilename, NULL, NULL, SW_SHOWNORMAL);
					else
						pew->pCB->MessageBox("Sorry, no help for this machine.");
				}
			}

		}
		else
			// Show machine window
			pew->pCB->ShowMachineWindow(pew->pPattern->columns[col]->GetMachine(), true);
	}
	else if (pew->ShowTrackToolbar) 
	{
		if (point.y < 2*pew->fontSize.cy) MuteTrack(point);
		else if (point.y > 16 +2*pew->fontSize.cy) MuteTrack(point);
		else TrackToolbarButton(point);
	}
	else
		MuteTrack(point);

	CScrollWnd::OnLButtonDblClk(nFlags, point);
}

void CTopWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	point = ClientToCanvas(point);

	if (point.y < pew->fontSize.cy)
	{
		int col = pew->pe.GetColumnAtX(point.x);
		if (col <= 0)
		{
			pew->toolbarhidden = !pew->toolbarhidden;
			pew->ToolbarChanged();
			return;
		}
		return;
	}

	if (pew->ShowTrackToolbar) 
	{

		if (point.y < 2*pew->fontSize.cy) MuteTrack(point);
		else if (point.y > 16 +2*pew->fontSize.cy) MuteTrack(point);
		else TrackToolbarButton(point);
	}
	else
	MuteTrack(point);

	CScrollWnd::OnLButtonDown(nFlags, point);
}

void CTopWnd::TrackToolbarButton(CPoint point)
{
	int col = pew->pe.GetColumnAtX(point.x);
	if (col < 0)
		return;

	CColumn *pc = pew->pPattern->columns[col].get();

	if (pc->IsTrackParam()) {
		int fc = pew->pe.GetFirstColumnofTrack(col);
		int cl = pew->pe.GetColumnX(fc);
		int btw = pew->pe.GetTrackWidth(col) / 4;
		
		if (point.x < cl+btw) {
			// button copy
			pew->pe.SelectTrackByNo(col);
			pew->pe.OnEditCopy();
		}
		else if (point.x < cl+2*btw) {
			// button cut
			pew->pe.SelectTrackByNo(col);
			pew->pe.OnEditCut();
		}
		else if (point.x < cl+3*btw) {
			// button paste
			int scol = pew->pe.cursor.column;
			int srow = pew->pe.cursor.row;
			pew->pe.cursor.column = fc;
			pew->pe.cursor.row = 0;
			pew->pe.OnEditPaste();
			pew->pe.cursor.column = scol;
			pew->pe.cursor.row = srow;
		}
		else if (point.x < cl+4*btw) {
			// button merge
			int scol = pew->pe.cursor.column;
			int srow = pew->pe.cursor.row;
			pew->pe.cursor.column = fc;
			pew->pe.cursor.row = 0;
			pew->pe.OnEditPasteSpecial();
			pew->pe.cursor.column = scol;
			pew->pe.cursor.row = srow;
		}	
	}
	
	Invalidate();
	pew->pe.Invalidate();

}

	
void CTopWnd::MuteTrack(CPoint point)
{
	int col = pew->pe.GetColumnAtX(point.x);
	if (col < 0)
		return;

	CColumn *pc = pew->pPattern->columns[col].get();
	if (pc->IsTrackParam())
		pew->pPattern->ToggleTrackMute(pc);

	Invalidate();
	pew->pe.Invalidate();

}
