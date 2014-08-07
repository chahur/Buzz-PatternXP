// EmptyWnd.cpp : implementation file
//

#include "stdafx.h"
#include "EmptyWnd.h"
#include "EditorWnd.h"


// CEmptyWnd

IMPLEMENT_DYNAMIC(CEmptyWnd, CScrollWnd)

CEmptyWnd::CEmptyWnd()
{

}

CEmptyWnd::~CEmptyWnd()
{
}

BEGIN_MESSAGE_MAP(CEmptyWnd, CScrollWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CEmptyWnd message handlers

void CEmptyWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	COLORREF bgcolor = pCB->GetThemeColor("PE BG");

// Create Vertical font
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT)); // clear out structure.
	lf.lfHeight = 70;
	lf.lfCharSet = SYMBOL_CHARSET;
	strcpy(lf.lfFaceName,_T("Wingdings 3"));
	CFont myWingFont;
	myWingFont.CreatePointFontIndirect(&lf, &dc);  

	CRect cr;
	GetClientRect(&cr);
	dc.FillSolidRect(&cr, bgcolor);
	dc.SetBkMode(TRANSPARENT);

	CString s;
	
	// Show / hide Tracks toolbar button
	CRect crtb;
	GetClientRect(&crtb);
	InflateRect(crtb, -2, 0);
	crtb.bottom = 13;
	dc.DrawEdge(crtb, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);
		
	if (pew->ShowTrackToolbar) {s = 0x72;}
	else {s = 0x73;}
	
	CObject *pOldFont = dc.SelectObject(&myWingFont);
	dc.DrawText(s, &crtb, DT_BOTTOM|DT_SINGLELINE|DT_CENTER);
	
	// Show / hide Params text button
	InflateRect(cr, -2, 0);
	cr.top = cr.top + 13;
	dc.DrawEdge(cr, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);

	if (pew->ShowParamText) {
		s = 0x72;
		cr.bottom = cr.bottom-6;
	}
	else {
		s = 0x73;
	}
		
	dc.DrawText(s, &cr, DT_BOTTOM|DT_SINGLELINE|DT_CENTER);
	
	dc.SelectObject(pOldFont);
}

void CEmptyWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CScrollWnd::OnLButtonDblClk(nFlags, point);
}

void CEmptyWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (point.y <14) {
		pew->ShowTrackToolbar = !pew->ShowTrackToolbar;
	}
	else {
		pew->ShowParamText = !pew->ShowParamText;	
	}
	pew->ShowParamTextChanged();

	CScrollWnd::OnLButtonDown(nFlags, point);
}
