#pragma once
#include "stdafx.h"

// CToolBar2

class CToolBarHint : public CToolBar
{
	DECLARE_DYNAMIC(CToolBarHint)

public:
	CToolBarHint();
	virtual ~CToolBarHint();

//	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

protected:
	DECLARE_MESSAGE_MAP()
public:
	CFont tbFont;
	CMICallbacks *pCB;

	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR,LRESULT *pResult);
//	afx_msg LRESULT OnMouseLeave(WPARAM,LPARAM);
//    afx_msg LRESULT OnMouseHover(WPARAM,LPARAM);
//	void OnMouseMove(UINT flags,CPoint point);

};


class CToolBar2 : public CToolBarHint
{
	DECLARE_DYNAMIC(CToolBar2)

public:
	CToolBar2();
	virtual ~CToolBar2();

//	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

protected:
public:
	CButton checkMidi;
	CStatic labelBar;
	CComboBox comboBar;
	CComboBox comboShrink;
	CButton checkHelp;
	CButton checkToolbar;
	CEdit editHumanize;
	CButton checkHumanizeEmpty;
//	CComboBox comboChords;
//	CButton checkChordsOnce;
	CComboBox comboInterpolate;
//	CStatic labelTonal;
//	CComboBox comboTonal;
//	CComboBox comboTranspose;
//	CComboBox comboArpeggio;

};


class CToolBarExt : public CToolBarHint
{
	DECLARE_DYNAMIC(CToolBarExt)

public:
	CToolBarExt();
	virtual ~CToolBarExt();

//	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

protected:
public:
/*	CButton checkMidi;
	CStatic labelBar;
	CComboBox comboBar;
	CComboBox comboShrink;
	CButton checkHelp;
	CButton checkToolbar;
	CEdit editHumanize;
	CButton checkHumanizeEmpty;
	*/
	CComboBox comboChords;
	CButton checkChordsOnce;
//	CComboBox comboInterpolate;
	CStatic labelTonal;
	CComboBox comboTonal;
	CComboBox comboTranspose;
	CComboBox comboArpeggio;

};


