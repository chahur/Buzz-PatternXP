// ColumnDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ColumnDialog.h"
#include "EditorWnd.h"
#include "ImageList.h"

CString CColumnDialog::LastLength = "16";


// CColumnDialog dialog

IMPLEMENT_DYNAMIC(CColumnDialog, CDialog)

CColumnDialog::CColumnDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CColumnDialog::IDD, pParent)
{

	m_NameValue = "";
	m_LengthValue = 0;
	m_RPBValue = 0;
	m_Copy = false;

}

CColumnDialog::~CColumnDialog()
{
}

void CColumnDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NP_NAME, m_Name);
	DDX_Control(pDX, IDC_NP_LENGTH, m_Length);
	DDX_Control(pDX, IDC_COMBO1, m_RPB);
}


BEGIN_MESSAGE_MAP(CColumnDialog, CDialog)
	ON_EN_UPDATE(IDC_NP_NAME, CColumnDialog::OnEnUpdateNpName) //BWC!!!
	ON_CBN_EDITUPDATE(IDC_NP_LENGTH, CColumnDialog::OnCbnEditupdateNpLength) //BWC!!!
	ON_CBN_EDITUPDATE(IDC_COMBO1, CColumnDialog::OnCbnEditupdateCombo1) //BWC!!!
END_MESSAGE_MAP()


BOOL CColumnDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
		
	CRect r;
	GetDlgItem(IDC_TREE_STATIC)->GetWindowRect(&r);
	ScreenToClient(&r);
	tree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | WS_BORDER, r, this, 0);
//	imageList.Create(IDB_TREE_IMAGES, 16, 9, RGB(0,255,255));
	CreateImageList(imageList, MAKEINTRESOURCE(IDB_TREE_IMAGES), 16, 9, RGB(0,255,255));
	
	tree.SetImageList(&imageList, TVSIL_NORMAL);
	tree.Initialize(TRUE).SetSmartCheckBox(TRUE).SetImages(TRUE);
 
	BuildTree();
	ValidLength = true;
	ValidName = true;
	ValidRPB = true;

	if (m_LengthValue != 0)
	{
		char buf[256];
		itoa(m_LengthValue, buf, 10);
		m_Length.SetWindowText(buf);
	}
	else
	{
		m_Length.SetWindowText(LastLength);
	}

	if (m_RPBValue != 0)
	{
		char buf[256];
		itoa(m_RPBValue, buf, 10);
		m_RPB.SetWindowText(buf);
	}
	else
	{
		m_RPB.SetWindowText("4");
	}

	if (m_NameValue != "")
	{
		m_Name.SetWindowText(m_NameValue);
	}
	else
	{
		int count = 0;
		CString name;
		do
		{
			name.Format("%02d", count++);
		} while(pew->pCB->GetPatternByName(pew->pMachine, name) != NULL);

		m_Name.SetWindowText(name);
	}

	if (m_Copy)
	{
		m_Length.EnableWindow(false);
		m_RPB.EnableWindow(false);
		SetWindowText("Create copy of pattern");
	}
	else if (m_LengthValue != 0)
		SetWindowText("Pattern Properties");		


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


class CMachineNameReader : public CMachineDataOutput
{
public:
	CMachineNameReader(CColumnDialog *p) { pwnd = p; }

	virtual void Write(void *pbuf, int const numbytes)
	{
		pwnd->AddMachine((char const *)pbuf);
	}

	CColumnDialog *pwnd;
};

void CColumnDialog::BuildTree()
{

	tree.SetRedraw(FALSE);
	tree.CollapseAll();
	tree.DeleteAllItems();

	CMachineNameReader mnr(this);
	pew->pCB->GetMachineNames(&mnr);

	tree.SortChildren(NULL);

	tree.SetRedraw(TRUE);

}

static void AddInternalParameter(CEditorWnd *pew, CMachine *pmac, CXHtmlTree &tree, HTREEITEM h, char const *name, InternalParameter ip, bool enable)
{
	HTREEITEM hi = tree.InsertItem(name, h);
	tree.SetItemImage(hi, TV_NOIMAGE, TV_NOIMAGE);
	tree.SetItemData(hi, (DWORD_PTR)(ip));
	tree.EnableItem(hi, enable);

	if (pew->pPattern != NULL)
		tree.SetCheck(hi, !pew->pPattern->GetColumn(MacIntPair(pmac, ip), 0) ? FALSE : TRUE);
}

void CColumnDialog::AddMachine(char const *name)
{
	CString thisname = pew->pCB->GetMachineName(pew->pMachine);
	if (thisname == name)
		return;

	HTREEITEM mac = tree.InsertItem(name);

	CMachine *pmac = pew->pCB->GetMachine(name);
	CMachineInfo const *pmi = pew->pCB->GetMachineInfo(pmac);

	int img;
	switch(pmi->Type)
	{
	case MT_MASTER: img = 0; break;
	case MT_GENERATOR: img = 1; break;
	case MT_EFFECT: img = 2; break;
	}

	tree.SetItemImage(mac, img, img);
	tree.SetItemData(mac, (DWORD_PTR)pmac);

	bool gotgp = pmi->numGlobalParameters > 0;
	bool gottp = pmi->numTrackParameters > 0;

	HTREEITEM internalitem = tree.InsertItem("Pattern XP", mac);
	AddInternalParameter(pew, pmac, tree, internalitem, "MIDI Note", MidiNote, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "MIDI Velocity", MidiVelocity, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "MIDI Pitch Wheel", MidiPitchWheel, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "MIDI CC", MidiCC, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "MIDI Note Delay", MidiNoteDelay, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "MIDI Note Cut", MidiNoteCut, gottp);
	
	AddInternalParameter(pew, pmac, tree, internalitem, "SubPattern Global Trigger", SPGlobalTrigger, gotgp);
	AddInternalParameter(pew, pmac, tree, internalitem, "SubPattern Global Effect 1", SPGlobalEffect1, gotgp);
	AddInternalParameter(pew, pmac, tree, internalitem, "SubPattern Global Effect 1 Data", SPGlobalEffect1Data, gotgp);
	AddInternalParameter(pew, pmac, tree, internalitem, "SubPattern Track Trigger", SPTrackTrigger, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "SubPattern Track Effect 1", SPTrackEffect1, gottp);
	AddInternalParameter(pew, pmac, tree, internalitem, "SubPattern Track Effect 1 Data", SPTrackEffect1Data, gottp);

	HTREEITEM paramitem = tree.InsertItem("Parameters", mac);

	tree.SetItemImage(internalitem, TV_NOIMAGE, TV_NOIMAGE);
	tree.SetItemImage(paramitem, TV_NOIMAGE, TV_NOIMAGE);

	for (int i = 0; i < pmi->numGlobalParameters + pmi->numTrackParameters; i++)
	{
		CMachineParameter const *pp = pmi->Parameters[i];
		HTREEITEM h = tree.InsertItem(pp->Name, paramitem);
		tree.SetItemData(h, (DWORD_PTR)(i));

		if (pp->Type == pt_note) img = 4;
		else if (pp->Flags & MPF_WAVE) img = 6;
		else if (pp->Flags & MPF_STATE)	img = 3;
		else img = 5;
		
		tree.SetItemImage(h, img, img);

		if (pew->pPattern != NULL)
			tree.SetCheck(h, !pew->pPattern->GetColumn(MacIntPair(pmac, i), 0) ? FALSE : TRUE);

//		if (pmi->numGlobalParameters > 0 && i == pmi->numGlobalParameters - 1 && pmi->numTrackParameters > 0)
//			tree.InsertSeparator(h);

	}
	
//	if (tree.GetChildrenCheckedCount(mac) > 0)
//		tree.Expand(mac, TVE_EXPAND);
}

void CColumnDialog::OnOK()
{
	MacParamPairVector &v = enabledColumns;
	v.clear();

	HTREEITEM h = tree.GetRootItem();
	while (h != NULL)
	{
		CMachine *pmac = (CMachine *)tree.GetItemData(h);

		HTREEITEM h2 = tree.GetNextItem(h, TVGN_CHILD);
		while (h2 != NULL)
		{
			HTREEITEM hp = tree.GetNextItem(h2, TVGN_CHILD);
			while(hp != NULL)
			{
				if (tree.GetCheck(hp) != FALSE)
					v.push_back(MacIntPair(pmac, (int)tree.GetItemData(hp)));
				
				hp = tree.GetNextItem(hp, TVGN_NEXT);
			}

			h2 = tree.GetNextItem(h2, TVGN_NEXT);
		}

		h = tree.GetNextItem(h, TVGN_NEXT);
	}

	m_Name.GetWindowText(m_NameValue);

	CString t;
	m_Length.GetWindowTextA(t);
	m_LengthValue = atoi(t);
	m_RPB.GetWindowTextA(t);
	m_RPBValue = atoi(t);

	CDialog::OnOK();
}


void CColumnDialog::OnEnUpdateNpName()
{
	CString name;
	m_Name.GetWindowText(name);
	
	if (m_NameValue != "" && name == m_NameValue)
		ValidName = true;
	else if (pew->pCB->GetPatternByName(pew->pMachine, name) != NULL || name.GetLength() <= 0)
		ValidName = false;
	else 
		ValidName = true;
	
	GetDlgItem(IDOK)->EnableWindow(ValidName && ValidLength && ValidRPB);

}

void CColumnDialog::OnCbnEditupdateNpLength()
{
	CString str;
	m_Length.GetWindowText(str);
	int len = atoi(str);

	if (len <= 0 || len > 8192)
		ValidLength = false;
	else
		ValidLength = true;

	GetDlgItem(IDOK)->EnableWindow(ValidName && ValidLength && ValidRPB);

}

void CColumnDialog::OnCbnEditupdateCombo1()
{
	CString str;
	m_RPB.GetWindowText(str);
	int rpb = atoi(str);

	if (rpb <= 0 || rpb > 96)
		ValidRPB = false;
	else
		ValidRPB = true;

	GetDlgItem(IDOK)->EnableWindow(ValidName && ValidLength && ValidRPB);
}
