#pragma once

#include "resource.h"
#include "afxwin.h"
#include "PatEd.h"

class CEditorWnd;


class CTonalDialog : public CDialog
{
	DECLARE_DYNAMIC(CTonalDialog)

public:
	CTonalDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTonalDialog();

// Dialog Data
	enum { IDD = IDD_TONAL_DIALOG };

	CEditorWnd *pew;

private:
	int indexmajor;
	int indexminor;
protected:

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();

protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

};
