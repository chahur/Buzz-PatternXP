#pragma once


// CScrollWnd

class CScrollWnd : public CWnd
{
	DECLARE_DYNAMIC(CScrollWnd)

public:
	CScrollWnd();
	virtual ~CScrollWnd();

	void SetLineSize(CSize s) { lineSize = s; }
	void SetPageSize(CSize s) { pageSize = s; }
	void SetCanvasSize(CSize s);
	CPoint GetScrollPos() const { return scrollPos; }

	CRect GetCanvasRect()
	{
		return CRect(0, 0, (canvasSize.cx + 1) * lineSize.cx, (canvasSize.cy + 1) * lineSize.cy);
	}

	CRect CanvasToClient(CRect &r)
	{
		CRect cr = r;
		cr.OffsetRect(CPoint(-scrollPos.x * lineSize.cx, -scrollPos.y * lineSize.cy));
		return cr;
	}

	CPoint ClientToCanvas(CPoint p)
	{
		CPoint cp = p;
		cp.Offset(CPoint(scrollPos.x * lineSize.cx, scrollPos.y * lineSize.cy));
		return cp;
	}

	virtual void OnDraw(CDC *pDC) {}
	virtual void PreScrollWindow() {}

	void HideVerticalScrollBar(bool hide);
	void HideHorizontalScrollBar(bool hide);

	void AlwaysShowVerticalScrollBar(bool show);
	void AlwaysShowHorizontalScrollBar(bool show);

	CScrollWnd *linkVert;
	CScrollWnd *linkHorz;

	void MakeVisible(int x, int y);
	void ScrollTo(CPoint pos);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	void ScrollDelta(CPoint dpos);

private:
	CPoint scrollPos;
	CSize canvasSize;
	CSize windowSize;
	CSize lineSize;
	CSize pageSize;

	bool hideVert;
	bool hideHorz;
	
	bool alwaysVert;
	bool alwaysHorz;

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


