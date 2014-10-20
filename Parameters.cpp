// Parameters.cpp : implementation file
//

#include "stdafx.h"
#include "Parameters.h"
#include "EditorWnd.h"

// CParametersDialog dialog

IMPLEMENT_DYNAMIC(CParametersDialog, CDialog)

CParametersDialog::CParametersDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CParametersDialog::IDD, pParent)
{


}

CParametersDialog::~CParametersDialog()
{
}


BEGIN_MESSAGE_MAP(CParametersDialog, CDialog)
//	ON_CBN_EDITUPDATE(IDC_COMBO1, CColumnDialog::OnCbnEditupdateCombo1) 
END_MESSAGE_MAP()


BOOL CParametersDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	int BST_VAL;		
		
	if (pew->PersistentSelection) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;
	CButton *pc = (CButton *)GetDlgItem(ID_PERSISTENT_SELECTION);
	pc->SetCheck(BST_VAL);
	
	if (pew->PersistentPlayPos) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_PERSISTENT_PLAYPOS);
	pc->SetCheck(BST_VAL);

	if (pew->toolbarvisible) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_USE_TOOLBAR);
	pc->SetCheck(BST_VAL);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CParametersDialog::OnOK()
{
	CButton *pc = (CButton *)GetDlgItem(ID_PERSISTENT_SELECTION);
	pew->pCB->WriteProfileInt("PersistentSelection", pc->GetCheck() == BST_CHECKED);
	pew->PersistentSelection= pew->pCB->GetProfileInt("PersistentSelection", true)!=0;
	
	pc = (CButton *)GetDlgItem(ID_PERSISTENT_PLAYPOS);
	pew->pCB->WriteProfileInt("PersistentPlayPos", pc->GetCheck() == BST_CHECKED);
	pew->PersistentPlayPos= pew->pCB->GetProfileInt("PersistentPlayPos", true)!=0;
	
	pc = (CButton *)GetDlgItem(ID_USE_TOOLBAR);
	pew->toolbarvisible = (pc->GetCheck() == BST_CHECKED);

	CDialog::OnOK();
}

