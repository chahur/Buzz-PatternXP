#pragma once

#include "MachinePattern.h"
#include "ScrollWnd.h"


// CEmptyWnd

class CEmptyWnd : public CScrollWnd
{
	DECLARE_DYNAMIC(CEmptyWnd)

public:
	CEmptyWnd();
	virtual ~CEmptyWnd();
	CEditorWnd *pew;
	
	CMICallbacks *pCB;

protected:
	LOGFONT lf;
	bool RefreshChordsButtonDown;
	void DrawRefreshChordButton(bool down);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
};




