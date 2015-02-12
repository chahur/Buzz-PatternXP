// TonalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TonalDlg.h"
#include "EditorWnd.h"


//-----------------------------------------------------------------------------
// CTonalDialog dialog

IMPLEMENT_DYNAMIC(CTonalDialog, CDialog)

CTonalDialog::CTonalDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTonalDialog::IDD, pParent)
{

}

CTonalDialog::~CTonalDialog()
{
}


BEGIN_MESSAGE_MAP(CTonalDialog, CDialog)
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
//	ON_BN_CLICKED(5, OnBnClickedUp)
//	ON_BN_CLICKED(6, OnBnClickedDown)

END_MESSAGE_MAP()


void CTonalDialog::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}


BOOL CTonalDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Analyse the pattern
	pew->pe.AnalyseTonality();

	char txt[255];
	CStatic *pt;

	for (int i=0; i<12; i++)
	{
		pt = (CStatic *)GetDlgItem(IDD_FIRST_NOTE+i);
		if (pt!=NULL) 
		{
			char s_basenote[3];
			s_basenote[0] = NoteToTextStrip[i*2+0];
			s_basenote[1] = NoteToTextStrip[i*2+1];
			s_basenote[2] = 0;

			sprintf(txt, "%s : %d", s_basenote, pew->tonality_notes[i]);
			pt->SetWindowText(txt);
		}
	}
	// Dièse     : fa, do, sol, ré, la, mi, si
	// Sharp     : F,  C,  G,   D,  A,  E,  B
	// Sharp pos : 5,  0,  7,   2,  9,  4, 11   
	pt = (CStatic *)GetDlgItem(IDD_SHARP);
	if (pt!=NULL) 
	{
		sprintf(txt, "Sharp... F#[%d] C#[%d] G#[%d] D#[%d] A#[%d]", 
			pew->tonality_notes[6],
			pew->tonality_notes[1],
			pew->tonality_notes[8],
			pew->tonality_notes[3],
			pew->tonality_notes[10]
			);
		pt->SetWindowText(txt);
	}

	// Bémol    : si, mi, la, ré, sol, do, fa
	// Flat     : B,  E,  A,  D,  G,   C,  F
	// Flat pos : 11, 4,  9,  2,  7,   0,  5 
	pt = (CStatic *)GetDlgItem(IDD_FLAT); 
	if (pt!=NULL) 
	{
		sprintf(txt, "Flat... Bb[%d] Eb[%d] Ab[%d] Db[%d] Gb[%d]", 
			pew->tonality_notes[10],
			pew->tonality_notes[3],
			pew->tonality_notes[8],
			pew->tonality_notes[1],
			pew->tonality_notes[6]
			);
		pt->SetWindowText(txt);
	}
	
	// Verify if enough notes to determine the tonality
	txt[0]=0;
	if (pew->tonality_notes[6]==0 && pew->tonality_notes[5]==0) sprintf(txt,"%sF ", txt);
	if (pew->tonality_notes[1]==0 && pew->tonality_notes[0]==0) sprintf(txt,"%sC ", txt);
	if (pew->tonality_notes[8]==0 && pew->tonality_notes[7]==0) sprintf(txt,"%sG ", txt);
	if (pew->tonality_notes[3]==0 && pew->tonality_notes[2]==0) sprintf(txt,"%sD ", txt);
	if (pew->tonality_notes[10]==0 && pew->tonality_notes[9]==0) sprintf(txt,"%sA ", txt);

	if (txt[0]!=0)
	{
		// Tonality determination not possible
		pt = (CStatic *)GetDlgItem(IDD_MINOR); 
		if (pt!=NULL) 
		{
			pt->SetWindowText("Tonality undefined");
		}
		pt = (CStatic *)GetDlgItem(IDD_MAJOR); 
		if (pt!=NULL) 
		{
			sprintf(txt, "%s not defined", txt);
			pt->SetWindowText(txt);
		}
		CButton *pc;
		pc = (CButton *)GetDlgItem(IDD_RADIOMINOR);
		pc->SetCheck(BST_UNCHECKED);
		pc->EnableWindow(FALSE);
		pc = (CButton *)GetDlgItem(IDD_RADIOMAJOR);
		pc->SetCheck(BST_UNCHECKED);
		pc->EnableWindow(FALSE);

	}
	else
	{
		int sharpflat=0;
		if (pew->tonality_notes[6]>0 && pew->tonality_notes[6]>pew->tonality_notes[5]) sharpflat++;
		if (sharpflat>0 && pew->tonality_notes[1]>0 && pew->tonality_notes[1]>pew->tonality_notes[0]) sharpflat++;
		if (sharpflat>1 && pew->tonality_notes[8]>0 && pew->tonality_notes[8]>pew->tonality_notes[7]) sharpflat++;
		if (sharpflat>2 && pew->tonality_notes[3]>0 && pew->tonality_notes[3]>pew->tonality_notes[2]) sharpflat++;
		if (sharpflat>3 && pew->tonality_notes[10]>0 && pew->tonality_notes[10]>pew->tonality_notes[9]) sharpflat++;
	
		if (sharpflat==0 && pew->tonality_notes[10]>0 && pew->tonality_notes[10]>pew->tonality_notes[11]) sharpflat--;
		if (sharpflat<0  && pew->tonality_notes[3]>0 && pew->tonality_notes[3]>pew->tonality_notes[4]) sharpflat--;
		if (sharpflat<-1 && pew->tonality_notes[8]>0 && pew->tonality_notes[8]>pew->tonality_notes[9]) sharpflat--;
		if (sharpflat<-2 && pew->tonality_notes[1]>0 && pew->tonality_notes[1]>pew->tonality_notes[2]) sharpflat--;
		if (sharpflat<-3 && pew->tonality_notes[6]>0 && pew->tonality_notes[6]>pew->tonality_notes[7]) sharpflat--;
	
		// Now, look at the corresponding tonality
		indexmajor=0;
		indexminor=0;
		for (int i=0; i<(int)pew->TonalityList.size(); i++)
		{
			if (pew->TonalityList[i].sharp_flat == sharpflat)
			{
				if (pew->TonalityList[i].major) indexmajor=i; else indexminor=i;
			}
		}
		// We have the 2 possible tonalities
		pt = (CStatic *)GetDlgItem(IDD_MAJOR); 
		if (pt!=NULL) 
		{
			sprintf(txt, "Major : %s", pew->TonalityList[indexmajor].name.c_str());
			pt->SetWindowText(txt);
		}
		pt = (CStatic *)GetDlgItem(IDD_MINOR); 
		if (pt!=NULL) 
		{
			sprintf(txt, "Minor : %s", pew->TonalityList[indexminor].name.c_str());
			pt->SetWindowText(txt);
		}

		CButton *pc;
		pc = (CButton *)GetDlgItem(IDD_RADIOMINOR);
		pc->SetCheck(BST_CHECKED);

	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTonalDialog::OnOK()
{
	CButton *pc;
	pc = (CButton *)GetDlgItem(IDD_RADIOMINOR);

	if (pc->GetCheck() == BST_CHECKED)
	{  // Set the tonality to minor
		if (indexminor>0){
			pew->SetComboBoxTonal(indexminor);
			pew->Invalidate();
		}
	}
	else
	{ // Set the tonality to major
		pc = (CButton *)GetDlgItem(IDD_RADIOMAJOR);
		if (pc->GetCheck() == BST_CHECKED){
			if (indexmajor>0) {
				pew->SetComboBoxTonal(indexmajor);
				pew->Invalidate();
			}
		}
	};

	CDialog::OnOK();
}

void CTonalDialog::OnCancel()
{
	CDialog::OnCancel();
}
