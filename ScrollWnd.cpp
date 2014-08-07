// ScrollWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ScrollWnd.h"


// CScrollWnd

IMPLEMENT_DYNAMIC(CScrollWnd, CWnd)

CScrollWnd::CScrollWnd()
{
	linkHorz = NULL;
	linkVert = NULL;
	scrollPos = CPoint(0, 0);
	canvasSize = CSize(1, 1);
	windowSize = CSize(0, 0);
	lineSize = CSize(1, 1);
	pageSize = CSize(10, 10);

	hideVert = hideHorz = false;
	alwaysVert = alwaysHorz = false;
}

CScrollWnd::~CScrollWnd()
{
}


BEGIN_MESSAGE_MAP(CScrollWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()




void CScrollWnd::SetCanvasSize(CSize s)
{
	canvasSize = s;
//	ScrollTo(CPoint(0, 0));
	ScrollTo(scrollPos);}



void CScrollWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	windowSize = CSize(cx, cy);
//	ScrollTo(CPoint(0, 0));
	ScrollTo(scrollPos);
}


void CScrollWnd::ScrollTo(CPoint pos)
{
	if (!IsWindow(GetSafeHwnd()))
		return;
	
	PreScrollWindow();

	CSize lpp;
	lpp.cx = windowSize.cx / lineSize.cx;
	lpp.cy = windowSize.cy / lineSize.cy;

	pos.x = max(0, min(pos.x, canvasSize.cx - lpp.cx));
	pos.y = max(0, min(pos.y, canvasSize.cy - lpp.cy));
	
	ScrollWindowEx((scrollPos.x - pos.x) * lineSize.cx, (scrollPos.y - pos.y) * lineSize.cy, NULL, NULL, NULL, NULL, SW_INVALIDATE);
				   
	scrollPos = pos;

    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = lpp.cx;
    si.nMin = 0;
	si.nMax = canvasSize.cx - 1;     
	si.nPos = scrollPos.x;
	if (hideHorz) si.nMin = si.nMax = 0;
	if (alwaysHorz) si.fMask |= SIF_DISABLENOSCROLL;
    SetScrollInfo(SB_HORZ, &si, TRUE);
	if (alwaysHorz)	ShowScrollBar(SB_HORZ);

    si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	si.nPage = lpp.cy;
    si.nMin = 0;
	si.nMax = canvasSize.cy - 1;     
	si.nPos = scrollPos.y;
	if (hideVert) si.nMin = si.nMax = 0;
	if (alwaysVert) si.fMask |= SIF_DISABLENOSCROLL;
    SetScrollInfo(SB_VERT, &si, TRUE);
	if (alwaysVert)	ShowScrollBar(SB_VERT);

	UpdateWindow();

	if (linkHorz != NULL)
		linkHorz->ScrollTo(CPoint(pos.x, linkHorz->GetScrollPos().y));

	if (linkVert != NULL)
		linkVert->ScrollTo(CPoint(linkVert->GetScrollPos().x, pos.y));


}

void CScrollWnd::ScrollDelta(CPoint dpos)
{
	ScrollTo(scrollPos + dpos);
}

void CScrollWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int lpp = windowSize.cx / lineSize.cx;

	switch (nSBCode)
	{
	case SB_LINELEFT:       ScrollDelta(CPoint(-1, 0)); break;
	case SB_LINERIGHT:      ScrollDelta(CPoint(1, 0)); break;
    case SB_PAGELEFT:       ScrollDelta(CPoint(-lpp, 0)); break;
	case SB_PAGERIGHT:      ScrollDelta(CPoint(lpp, 0)); break;
    case SB_THUMBPOSITION:  ScrollTo(CPoint(nPos, scrollPos.y)); break;
    case SB_THUMBTRACK:     ScrollTo(CPoint(nPos, scrollPos.y)); break;
    case SB_LEFT:           ScrollTo(CPoint(0, scrollPos.y)); break;
    case SB_RIGHT:          ScrollTo(CPoint(MAXLONG, scrollPos.y)); break;
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CScrollWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int lpp = windowSize.cy / lineSize.cy;

	switch (nSBCode)
	{
	case SB_LINEUP:         ScrollDelta(CPoint(0, -1)); break;
	case SB_LINEDOWN:       ScrollDelta(CPoint(0, 1)); break;
    case SB_PAGEUP:         ScrollDelta(CPoint(0, -lpp)); break;
	case SB_PAGEDOWN:       ScrollDelta(CPoint(0, lpp)); break;
    case SB_THUMBPOSITION:  ScrollTo(CPoint(scrollPos.x, nPos)); break;
    case SB_THUMBTRACK:     ScrollTo(CPoint(scrollPos.x, nPos)); break;
    case SB_TOP:            ScrollTo(CPoint(scrollPos.x, 0)); break;
    case SB_BOTTOM:         ScrollTo(CPoint(scrollPos.x, MAXLONG)); break;
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CScrollWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.SetWindowOrg(scrollPos.x * lineSize.cx, scrollPos.y * lineSize.cy);

	CRect r;
	dc.GetClipBox(&r);		

	if (r.IsRectEmpty())
		return;

	CXMemDC memDC(dc, &r);
	CDC *pDC = &memDC.GetDC();
	OnDraw(pDC);
}

void CScrollWnd::HideVerticalScrollBar(bool hide)
{
	hideVert = hide;
	ScrollTo(scrollPos);
}

void CScrollWnd::HideHorizontalScrollBar(bool hide)
{
	hideHorz = hide;
	ScrollTo(scrollPos);
}

void CScrollWnd::AlwaysShowVerticalScrollBar(bool show)
{
	alwaysVert = show;
	ScrollTo(scrollPos);
}

void CScrollWnd::AlwaysShowHorizontalScrollBar(bool show)
{
	alwaysHorz = show;
	ScrollTo(scrollPos);
}


void CScrollWnd::MakeVisible(int x, int y)
{
	CPoint p = scrollPos;
	p.x = min(p.x, x);
	p.y = min(p.y, y);

	CSize lpp;
	lpp.cx = windowSize.cx / lineSize.cx;
	lpp.cy = windowSize.cy / lineSize.cy;

	if (x >= p.x + lpp.cx) p.x = max(0, x - lpp.cx + 1);
	if (y >= p.y + lpp.cy) p.y = max(0, y - lpp.cy + 1);
	
	ScrollTo(p);
}

int CScrollWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	return 0;
}
