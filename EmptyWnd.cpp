// EmptyWnd.cpp : implementation file
//

#include "stdafx.h"
#include "EmptyWnd.h"
#include "EditorWnd.h"


// CEmptyWnd

IMPLEMENT_DYNAMIC(CEmptyWnd, CScrollWnd)

CEmptyWnd::CEmptyWnd()
{
	memset(&lf, 0, sizeof(LOGFONT)); // clear out structure.
	lf.lfHeight = 90;
	lf.lfCharSet = SYMBOL_CHARSET;
	strcpy(lf.lfFaceName,_T("Webdings"));
}

CEmptyWnd::~CEmptyWnd()
{
}

BEGIN_MESSAGE_MAP(CEmptyWnd, CScrollWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP() 
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEMOVE, OnMouseMove)
	

END_MESSAGE_MAP()


// CEmptyWnd message handlers

void CEmptyWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	COLORREF bgcolor = pCB->GetThemeColor("PE BG");

   // Create symbols font
	CFont myWingFont;
	myWingFont.CreatePointFontIndirect(&lf, &dc);  

	CRect cr;
	GetClientRect(&cr);
	dc.FillSolidRect(&cr, bgcolor);
	dc.SetBkMode(TRANSPARENT);

	CString s;
	
	CObject *pOldFont = dc.SelectObject(&myWingFont);

	COLORREF textcolor = pew->pCB->GetThemeColor("PE Text");
	dc.SetTextColor(textcolor);

	// Show / hide Chord expert button
	CRect crce;
	GetClientRect(&crce);
	crce.right = 14;
	if (pew->ChordExpertvisible) {
		// Show refresh chords button
		CRect crrb = crce;
		crrb.right = 45;
		crrb.bottom = 13;
		if (pew->ShowTrackToolbar) crrb.bottom = crrb.bottom+6;
		crce.top = crrb.bottom;
		crce.right = crrb.right; 
		if (pew->AutoChordExpert)
			;
		else
			dc.DrawEdge(crrb, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT|BF_SOFT);
		s = (char)0x71; 
		if (!pew->ShowTrackToolbar) crrb.top = crrb.top - 1;
		dc.DrawText(s, &crrb, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
	}
	dc.DrawEdge(crce, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);
	if (pew->ChordExpertvisible) s = (char)0x33;//0x76;
	else s = (char)0x34;//0x77;
	dc.DrawText(s, &crce, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
	

	// Show / hide Tracks toolbar button
	CRect crtb;
	GetClientRect(&crtb);
	crtb.left = crce.right;
	crtb.bottom = 13;
	if (pew->ShowTrackToolbar) {
		s = (char)0x35;//0x72;
		crtb.bottom = crtb.bottom+6;
	}
	else s = (char)0x36;//0x73;
	dc.DrawEdge(crtb, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);		
	crtb.top = crtb.top - 2;
	dc.DrawText(s, &crtb, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
	crtb.top = crtb.top + 2;
	
	// Show / hide Params text button
	cr.left = crce.right;
	cr.top = crtb.bottom;
	dc.DrawEdge(cr, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);

	if (pew->ShowParamText) {
		s = (char)0x35;//72;
	}
	else s = (char)0x36;//73;
	cr.top = cr.top - 2;
	dc.DrawText(s, &cr, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
	
	dc.SelectObject(pOldFont);
}

void CEmptyWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CScrollWnd::OnLButtonDblClk(nFlags, point);
}

void CEmptyWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT tme;
	if (!pew->AutoChordExpert) {
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = this->m_hWnd;
		::_TrackMouseEvent(&tme);
	}

	int pointy = 14;
	if (pew->ShowTrackToolbar) pointy = pointy+6;

	if ((pew->ChordExpertvisible) && (point.y <pointy) && (point.x<45)) {
	//	bool ctrldown = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;
	//	if (ctrldown) 
	//		pew->OnChordExpert();
	//	else 
		if (!pew->AutoChordExpert)
			DrawRefreshChordButton(true);
	}
	else
	{
		int leftMargin = 14;
		if (pew->ChordExpertvisible) leftMargin = 45;

		if (point.x < leftMargin)
		{
			pew->ChordExpertvisible = !pew->ChordExpertvisible;
			pew->AnalyseChords();
		}
		else if (point.y <pointy) 
			pew->ShowTrackToolbar = !pew->ShowTrackToolbar;
		else 
			pew->ShowParamText = !pew->ShowParamText;	
	
		pew->ShowParamTextChanged();
	}

	CScrollWnd::OnLButtonDown(nFlags, point);
}

void CEmptyWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!pew->AutoChordExpert) {
		int pointy = 14;
		if (pew->ShowTrackToolbar) pointy = pointy+6;

		if ((pew->ChordExpertvisible) && (point.y <pointy) && (point.x<45)) {
				pew->AnalyseChords();
			}
	

		DrawRefreshChordButton(false);
	}

	CScrollWnd::OnLButtonUp(nFlags, point);
}


void CEmptyWnd::DrawRefreshChordButton(bool down)
{
	int pointy = 14;
	if (pew->ShowTrackToolbar) pointy = pointy+6;

	if (pew->ChordExpertvisible)
	{
		CClientDC dc(this); // device context for painting

		// Create symbols font
		CFont myWingFont;
		myWingFont.CreatePointFontIndirect(&lf, &dc);  
		CObject *pOldFont = dc.SelectObject(&myWingFont);

		CRect crrb;
		GetClientRect(&crrb);
		COLORREF bgcolor = pCB->GetThemeColor("PE BG");
		crrb.right = 45;
		crrb.bottom = 13;
		if (pew->ShowTrackToolbar) crrb.bottom = crrb.bottom+6;
		dc.FillSolidRect(&crrb, bgcolor);
		dc.SetBkMode(TRANSPARENT);
		if (!down) dc.DrawEdge(crrb, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT|BF_SOFT);
		RefreshChordsButtonDown = down;

		CString s;
		s = (char)0x71;
		if (!pew->ShowTrackToolbar) crrb.top = crrb.top - 1;
		dc.DrawText(s, &crrb, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
		dc.SelectObject(pOldFont);
	}
}

LRESULT CEmptyWnd::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	DrawRefreshChordButton(false);
	return TRUE;
}

LRESULT CEmptyWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	if (RefreshChordsButtonDown) {
		// Check if we are leaving the button
		int xPos = GET_X_LPARAM(lParam); 
		int yPos = GET_Y_LPARAM(lParam); 

		if (!pew->AutoChordExpert) {
			int pointy = 14;
			if (pew->ShowTrackToolbar) pointy = pointy+6;

			if ((pew->ChordExpertvisible) && (yPos <pointy) && (xPos<45)) {
				DrawRefreshChordButton(false);
			}
		}
	}
	return TRUE;
}

