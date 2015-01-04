#pragma once
#include "stdafx.h"

// CToolBar2

class CToolBar2 : public CToolBar
{
	DECLARE_DYNAMIC(CToolBar2)

public:
	CToolBar2();
	virtual ~CToolBar2();

//	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

protected:
	DECLARE_MESSAGE_MAP()
public:
//	bool MouseOver_;
	CFont tbFont;
	CButton checkMidi;
	CStatic labelBar;
	CComboBox comboBar;
	CComboBox comboShrink;
	CButton checkHelp;
	CButton checkToolbar;
	CEdit editHumanize;
	CButton checkHumanizeEmpty;
	CComboBox comboChords;
	CButton checkChordsOnce;
	CComboBox comboInterpolate;
	CStatic labelTonal;
	CComboBox comboTonal;

	CMICallbacks *pCB;
	afx_msg LRESULT CToolBar2::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR,LRESULT *pResult);
//	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
//    afx_msg LRESULT OnMouseHover(WPARAM,LPARAM);
//	void OnMouseMove(UINT flags,CPoint point);

};


