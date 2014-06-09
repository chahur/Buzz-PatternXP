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
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};




