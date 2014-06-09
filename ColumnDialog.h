#pragma once

#include "resource.h"
#include "tree/XHtmlTree.h"
#include "MachinePattern.h"
#include "afxwin.h"

class CEditorWnd;

// CColumnDialog dialog

class CColumnDialog : public CDialog
{
	DECLARE_DYNAMIC(CColumnDialog)

public:
	CColumnDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CColumnDialog();

// Dialog Data
	enum { IDD = IDD_COLUMN_DIALOG };

	CXHtmlTree tree;
	CEditorWnd *pew;

	void AddMachine(char const *name);

	MacParamPairVector enabledColumns;

private:
	void BuildTree();

	CImageList imageList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	CEdit m_Name;
	CComboBox m_Length;
	CComboBox m_RPB;

	bool ValidLength;
	bool ValidName;
	bool ValidRPB;
	static CString LastLength;

	CString m_NameValue;
	int m_LengthValue;
	int m_RPBValue;

	bool m_Copy;

	afx_msg void OnEnUpdateNpName();
	afx_msg void OnCbnEditupdateNpLength();
	afx_msg void OnCbnEditupdateCombo1();
};
