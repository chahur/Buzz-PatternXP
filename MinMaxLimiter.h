#pragma once

#include "resource.h"
#include "afxwin.h"
#include "ScrollWnd.h"
#include "PatEd.h"
#include "INI.h"

class CEditorWnd;


class CMinMaxLimiterDialog : public CDialog
{
	DECLARE_DYNAMIC(CMinMaxLimiterDialog)

public:
	CMinMaxLimiterDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMinMaxLimiterDialog();

// Dialog Data
	enum { IDD = IDD_MINMAXLIMITER_DIALOG };

	CEditorWnd *pew;
	CMICallbacks *pCB;

private:
	CIniReader m_IniReader;
	BOOL IniOK;


protected:

	DECLARE_MESSAGE_MAP()
public:
	CComboBox *cbOctaveMin;
	CComboBox *cbOctaveMax;
	CComboBox *cbNoteMin;
	CComboBox *cbNoteMax;
	virtual BOOL OnInitDialog();
	void SaveParams();


protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
//	afx_msg void OnCbnEditupdateCombo1();
	afx_msg void OnComboPresetSelect();

};
