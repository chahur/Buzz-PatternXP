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
	lf.lfHeight = 120;
	lf.lfCharSet = SYMBOL_CHARSET;
	strcpy(lf.lfFaceName,_T("Wingdings 3"));
	CFont myWingFont;
	myWingFont.CreatePointFontIndirect(&lf, &dc);  

	CRect cr;
	GetClientRect(&cr);
	dc.FillSolidRect(&cr, bgcolor);
	// dc.Draw3dRect(cr, RGB(255, 0, 0), RGB(0, 255, 0));
	InflateRect(cr, -2, -2);
	dc.DrawEdge(cr, BDR_RAISEDINNER| BDR_RAISEDOUTER, BF_RECT);

	CString s;
	if (pew->ShowParamText) {
		s = 0x72;
		cr.bottom = cr.bottom-6;
	}
	else {
		s = 0x73;
		cr.bottom = cr.bottom-2;
	}
	
	
	CObject *pOldFont = dc.SelectObject(&myWingFont);
	
	dc.DrawText(s, &cr, DT_BOTTOM|DT_SINGLELINE|DT_CENTER);
	
	dc.SelectObject(pOldFont);
}

void CEmptyWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CScrollWnd::OnLButtonDblClk(nFlags, point);
}

void CEmptyWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	pew->ShowParamText = !pew->ShowParamText;
	pew->ShowParamTextChanged();

	CScrollWnd::OnLButtonDown(nFlags, point);
}
