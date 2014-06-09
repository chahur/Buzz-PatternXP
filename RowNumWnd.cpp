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

	for (int y = firstrow; y < lastrow; y++)
	{
		char buf[8];
		sprintf(buf, "%5d  ", y);
		pDC->TextOut(0, y * pew->fontSize.cy, buf, 5);
	}

	pDC->SelectObject(pOldFont);
}



// CRowNumWnd message handlers


