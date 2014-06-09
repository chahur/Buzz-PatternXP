#pragma once

#include "ScrollWnd.h"

class CEditorWnd;

// CRowNumWnd

class CRowNumWnd : public CScrollWnd
{
	DECLARE_DYNAMIC(CRowNumWnd)

public:
	CRowNumWnd();
	virtual ~CRowNumWnd();

	virtual void OnDraw(CDC *pDC);


	CEditorWnd *pew;

protected:
	DECLARE_MESSAGE_MAP()
public:

};


