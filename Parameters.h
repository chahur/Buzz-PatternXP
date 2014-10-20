#pragma once

#include "resource.h"

#include "afxwin.h"

class CEditorWnd;

// CColumnDialog dialog

class CParametersDialog : public CDialog
{
	DECLARE_DYNAMIC(CParametersDialog)

public:
	CParametersDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CParametersDialog();

// Dialog Data
	enum { IDD = IDD_PARAMETERS_DIALOG };

	CEditorWnd *pew;


private:

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
//	afx_msg void OnCbnEditupdateCombo1();
};
