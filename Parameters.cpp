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
//	ON_CBN_EDITUPDATE(IDC_COMBO1, CParametersDialog::OnCbnEditupdateCombo1) 
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

	if (pew->ImportAutoResize) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_IMPORT_AUTO_RESIZE);
	pc->SetCheck(BST_VAL);

	if (pew->AutoChordExpert) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_CHORD_AUTO_REFRESH);
	pc->SetCheck(BST_VAL);
	
	if (pew->PgUpDownDisabled) BST_VAL=BST_CHECKED; else BST_VAL=BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_PGUPDOWN_DISABLED);
	pc->SetCheck(BST_VAL);

	if (pew->HomeDisabled) BST_VAL = BST_CHECKED; else BST_VAL = BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_HOME_DISABLED);
	pc->SetCheck(BST_VAL);

	if (pew->DrawRowNumberButton) BST_VAL = BST_CHECKED; else BST_VAL = BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_DRAW_ROW_BUTTON);
	pc->SetCheck(BST_VAL);	

	CEdit *pet = (CEdit*)GetDlgItem(ID_GRAPHICALWIDTH);
	CString sgw;
	int gw = pew->pCB->GetProfileInt("GraphicalWidth", 10);
	if ((gw >= 0) && (gw <= 100))
		sgw.Format(_T("%d"), gw);
	else 
		sgw.Format(_T("%d"), 10);
	pet->SetWindowText(sgw);


	if (pew->UpdateGraphicalRow) BST_VAL = BST_CHECKED; else BST_VAL = BST_UNCHECKED;
	pc = (CButton *)GetDlgItem(ID_GRAPHICALROW);
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

	pc = (CButton *)GetDlgItem(ID_IMPORT_AUTO_RESIZE);
	pew->pCB->WriteProfileInt("ImportAutoResize", pc->GetCheck() == BST_CHECKED);
	pew->ImportAutoResize= pew->pCB->GetProfileInt("ImportAutoResize", true)!=0;
	
	pc = (CButton *)GetDlgItem(ID_CHORD_AUTO_REFRESH);
	pew->pCB->WriteProfileInt("AutoChordExpert", pc->GetCheck() == BST_CHECKED);
	pew->AutoChordExpert= pew->pCB->GetProfileInt("AutoChordExpert", true)!=0;

	pc = (CButton *)GetDlgItem(ID_PGUPDOWN_DISABLED);
	pew->pCB->WriteProfileInt("PgUpDownDisabled", pc->GetCheck() == BST_CHECKED);
	pew->PgUpDownDisabled= pew->pCB->GetProfileInt("PgUpDownDisabled", false)!=0;

	pc = (CButton *)GetDlgItem(ID_HOME_DISABLED);
	pew->pCB->WriteProfileInt("HomeDisabled", pc->GetCheck() == BST_CHECKED);
	pew->HomeDisabled = pew->pCB->GetProfileInt("HomeDisabled", false) != 0;

	pc = (CButton *)GetDlgItem(ID_DRAW_ROW_BUTTON);
	pew->pCB->WriteProfileInt("DrawRowNumberButton", pc->GetCheck() == BST_CHECKED);
	pew->DrawRowNumberButton = pew->pCB->GetProfileInt("DrawRowNumberButton", true) != 0;

	CEdit *pet = (CEdit*)GetDlgItem(ID_GRAPHICALWIDTH);
	CString sgw;
	pet->GetWindowText(sgw);
	int gw = atoi(sgw);
	if ((gw >= 0) && (gw <= 100))
		pew->pCB->WriteProfileInt("GraphicalWidth", gw);

	pc = (CButton *)GetDlgItem(ID_GRAPHICALROW);
	pew->pCB->WriteProfileInt("UpdateGraphicalRow", pc->GetCheck() == BST_CHECKED);
	pew->UpdateGraphicalRow = pew->pCB->GetProfileInt("UpdateGraphicalRow", true) != 0;


	CDialog::OnOK();
}

