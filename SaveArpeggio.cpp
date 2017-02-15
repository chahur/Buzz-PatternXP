// SaveArpeggio.cpp : implementation file
//

#include "stdafx.h"
#include "SaveArpeggio.h"
#include "EditorWnd.h"

// CSaveArpeggioDialog dialog

IMPLEMENT_DYNAMIC(CSaveArpeggioDialog, CDialog)

CSaveArpeggioDialog::CSaveArpeggioDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveArpeggioDialog::IDD, pParent)
{


}

CSaveArpeggioDialog::~CSaveArpeggioDialog()
{
}


BEGIN_MESSAGE_MAP(CSaveArpeggioDialog, CDialog)
END_MESSAGE_MAP()


BOOL CSaveArpeggioDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetWindowText(Caption); 

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CSaveArpeggioDialog::OnOK()
{
	CEdit * pt;
	pt = (CEdit *)GetDlgItem(4);  // Arpeggio
	if (pt!=NULL) pt->GetWindowTextA(SaveName);

	CDialog::OnOK();
}

