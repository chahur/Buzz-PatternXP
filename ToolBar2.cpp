// ToolBar2.cpp : implementation file
//

#include "stdafx.h"
#include "ToolBar2.h"

#include "afxpriv.h" //BWC!!
// CToolBar2

 
IMPLEMENT_DYNAMIC(CToolBar2, CToolBar)

CToolBar2::CToolBar2()
{

}

CToolBar2::~CToolBar2()
{
}



BEGIN_MESSAGE_MAP(CToolBar2, CToolBar)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


BOOL CToolBar2::OnToolTipNotify(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
   // need to handle both ANSI and UNICODE versions of the message
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    TCHAR szFullText[256];
    CString strTipText;
    UINT nID = pNMHDR->idFrom;
    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
      pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
      // idFrom is actually the HWND of the tool
      nID = ::GetDlgCtrlID((HWND)nID);
    }

    if (nID != 0) // will be zero on a separator
	{
		// don't handle the message if no string resource found
		if (AfxLoadString(nID, szFullText) == 0)
			return FALSE;
		// this is the command id, not the button index
//		AfxExtractSubString(strTipText, szFullText, 1, '\n');
	}

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, szFullText, _countof(pTTTA->szText));
	else
		mbstowcs(pTTTW->szText, szFullText, _countof(pTTTW->szText));
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, szFullText, _countof(pTTTA->szText));
	else
		lstrcpyn(pTTTW->szText, szFullText, _countof(pTTTW->szText));
#endif
    *pResult = 0;

    return TRUE;    // message was handled
 }


LRESULT CToolBar2::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	if (IsWindowVisible())
    {
		CFrameWnd* pParent = (CFrameWnd*)GetParent();
        if (pParent)
			OnUpdateCmdUI(pParent, (BOOL)wParam);
    }
	return 0L;
}
