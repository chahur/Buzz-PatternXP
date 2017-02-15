#pragma once

#include "resource.h"
#include "afxwin.h"

class CEditorWnd;

// CSaveArpeggioDialog dialog

class CSaveArpeggioDialog : public CDialog
{
	DECLARE_DYNAMIC(CSaveArpeggioDialog)

public:
	CSaveArpeggioDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSaveArpeggioDialog();

// Dialog Data
	enum { IDD = IDD_SAVE_ARPEGGIO_DIALOG };

	CEditorWnd *pew;
	CString SaveName;

private:

protected:
	DECLARE_MESSAGE_MAP()
public:
	CString Caption;
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
//	afx_msg void OnCbnEditupdateCombo1();
};
