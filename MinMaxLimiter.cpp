// MinMaxLimiter.cpp : implementation file
//

#include "stdafx.h"
#include "MinMaxLimiter.h"
#include "EditorWnd.h"
#include <io.h>




//-----------------------------------------------------------------------------
// CMinMaxLimiterDialog dialog

IMPLEMENT_DYNAMIC(CMinMaxLimiterDialog, CDialog)

CMinMaxLimiterDialog::CMinMaxLimiterDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMinMaxLimiterDialog::IDD, pParent)
{

}

CMinMaxLimiterDialog::~CMinMaxLimiterDialog()
{
}


BEGIN_MESSAGE_MAP(CMinMaxLimiterDialog, CDialog)
//	ON_WM_PAINT()
//	ON_BN_CLICKED(5, OnBnClickedUp)
  ON_CBN_SELENDOK(IDC_COMBO5, OnComboPresetSelect)

END_MESSAGE_MAP()


BOOL CMinMaxLimiterDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	cbOctaveMin = (CComboBox *)GetDlgItem(IDC_COMBO1);
	cbOctaveMin->AddString("0");
	cbOctaveMin->AddString("1");
	cbOctaveMin->AddString("2");
	cbOctaveMin->AddString("3");
	cbOctaveMin->AddString("4");
	cbOctaveMin->AddString("5");
	cbOctaveMin->AddString("6");
	cbOctaveMin->AddString("7");
	cbOctaveMin->AddString("8");
	cbOctaveMin->AddString("9");

	cbNoteMin = (CComboBox *)GetDlgItem(IDC_COMBO2);
	
	cbNoteMin->AddString("C");
	cbNoteMin->AddString("C#");
	cbNoteMin->AddString("D");
	cbNoteMin->AddString("D#");
	cbNoteMin->AddString("E");
	cbNoteMin->AddString("F");
	cbNoteMin->AddString("F#");
	cbNoteMin->AddString("G");
	cbNoteMin->AddString("G#");
	cbNoteMin->AddString("A");
	cbNoteMin->AddString("A#");
	cbNoteMin->AddString("B");

	cbOctaveMax = (CComboBox *)GetDlgItem(IDC_COMBO3);
	cbOctaveMax->AddString("0");
	cbOctaveMax->AddString("1");
	cbOctaveMax->AddString("2");
	cbOctaveMax->AddString("3");
	cbOctaveMax->AddString("4");
	cbOctaveMax->AddString("5");
	cbOctaveMax->AddString("6");
	cbOctaveMax->AddString("7");
	cbOctaveMax->AddString("8");
	cbOctaveMax->AddString("9");

	cbNoteMax = (CComboBox *)GetDlgItem(IDC_COMBO4);
	cbNoteMax->AddString("C");
	cbNoteMax->AddString("C#");
	cbNoteMax->AddString("D");
	cbNoteMax->AddString("D#");
	cbNoteMax->AddString("E");
	cbNoteMax->AddString("F");
	cbNoteMax->AddString("F#");
	cbNoteMax->AddString("G");
	cbNoteMax->AddString("G#");
	cbNoteMax->AddString("A");
	cbNoteMax->AddString("A#");
	cbNoteMax->AddString("B");

	int i = pew->pCB->GetProfileInt("CMinMaxLimiterDialog.OctaveMin", 3);
	cbOctaveMin->SetCurSel(i);
	i = pew->pCB->GetProfileInt("CMinMaxLimiterDialog.NoteMin", 0);
	cbNoteMin->SetCurSel(i);

	i = pew->pCB->GetProfileInt("CMinMaxLimiterDialog.OctaveMax", 6);
	cbOctaveMax->SetCurSel(i);
	i = pew->pCB->GetProfileInt("CMinMaxLimiterDialog.NoteMax", 0);
	cbNoteMax->SetCurSel(i);

	// Load Presets
	// Load chords filename
	IniOK = false;

	char pathName[255];
	pew->GeneratorFileName(pathName, "transpose.prs");
	// Check if the file exists
	if (_access(pathName, 0) == 0) {
		CComboBox *cb = (CComboBox *)GetDlgItem(IDC_COMBO5);
		cb->ResetContent();
		CStringList* myStringList;
		POSITION pos;

		m_IniReader.setINIFileName(pathName);
		IniOK = true;
		
		myStringList = m_IniReader.getSectionNames();

		for (pos = myStringList->GetHeadPosition(); pos != NULL; ) {
			CString txt = myStringList->GetNext(pos);
			cb->AddString(txt);
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMinMaxLimiterDialog::SaveParams()
{
	int i = cbOctaveMin->GetCurSel();
	pew->pCB->WriteProfileInt("CMinMaxLimiterDialog.OctaveMin", i);
	i = cbNoteMin->GetCurSel();
	pew->pCB->WriteProfileInt("CMinMaxLimiterDialog.NoteMin", i);
	i = cbOctaveMax->GetCurSel();
	pew->pCB->WriteProfileInt("CMinMaxLimiterDialog.OctaveMax", i);
	i = cbNoteMax->GetCurSel();
	pew->pCB->WriteProfileInt("CMinMaxLimiterDialog.NoteMax", i);
	
}

void CMinMaxLimiterDialog::OnComboPresetSelect()
{
	if (IniOK) {
		char PrsName[255];
		CComboBox *cb = (CComboBox *)GetDlgItem(IDC_COMBO5);
		int index = cb->GetCurSel();
		cb->GetLBText(index, PrsName);

		CString val_minO = m_IniReader.getKeyValue("minOctave", PrsName);
		cbOctaveMin = (CComboBox *)GetDlgItem(IDC_COMBO1);
		cbOctaveMin->SelectString(0, val_minO);

		CString val_minN = m_IniReader.getKeyValue("minNote", PrsName);
		cbNoteMin = (CComboBox *)GetDlgItem(IDC_COMBO2);
		cbNoteMin->SelectString(0, val_minN);

		CString val_maxO = m_IniReader.getKeyValue("maxOctave", PrsName);
		cbOctaveMax = (CComboBox *)GetDlgItem(IDC_COMBO3);
		cbOctaveMax->SelectString(0, val_maxO);

		CString val_maxN = m_IniReader.getKeyValue("maxNote", PrsName);
		cbNoteMax = (CComboBox *)GetDlgItem(IDC_COMBO4);
		cbNoteMax->SelectString(0, val_maxN);
	}

}

void CMinMaxLimiterDialog::OnOK()
{
	// Check Min - Max validity : must be at least 1 octave
	int OctaveMin = cbOctaveMin->GetCurSel(); 
	int NoteMin = cbNoteMin->GetCurSel();
	int ValMin = (OctaveMin << 4) + NoteMin + 1;
	int CompMin = OctaveMin*12 + NoteMin;

	int OctaveMax = cbOctaveMax->GetCurSel();
	int NoteMax = cbNoteMax->GetCurSel();
	int ValMax = (OctaveMax << 4) + NoteMax + 1;
	int CompMax = OctaveMax * 12 + NoteMax;

	if (CompMax - CompMin +1 < 12)	{ 
		// Invalid parameters
		char txt[255];
		sprintf(txt, "Must be at least 1 octave.");
		AfxMessageBox(txt, MB_OK);
		return;
	}
	else	{
		pew->pe.Transpose(ValMin, ValMax);

	}

	SaveParams();
	CDialog::OnOK();
}

void CMinMaxLimiterDialog::OnCancel()
{
	CDialog::OnCancel();
}

