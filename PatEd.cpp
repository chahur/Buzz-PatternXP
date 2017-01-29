// PatEd.cpp : implementation file
//

#include "stdafx.h"
#include "App.h"
#include "PatEd.h"
#include "EditorWnd.h"
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>



// shared by all instances
MapIntToIntVector CPatEd::clipboard;
int CPatEd::clipboardRowCount;
bool CPatEd::clipboardPersistentSelection;
CPatEd::SelectionMode CPatEd::clipboardSelMode;


// CPatEd

IMPLEMENT_DYNCREATE(CPatEd, CScrollWnd)

CPatEd::CPatEd()
{
	mouseWheelAcc = 0;
	cursorStep = 1;
	selection = false;
	pPlayingPattern = NULL;
	playPos = 0;
	drawPlayPos = -1;
	mouseSelecting = false;
	persistentSelection = false;
	selMode = column;
	invalidateInTimer = false;
	drawing = false;
	CheckRefreshChordCount=0;
	AnalyseChordRefresh=false;
	AnalysingChords = false;
}

CPatEd::~CPatEd()
{
}


BEGIN_MESSAGE_MAP(CPatEd, CScrollWnd)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_CREATE()
	ON_MESSAGE(CPatEd::WM_MIDI_NOTE, OnMidiNote)
END_MESSAGE_MAP()




void CPatEd::SetPattern(CMachinePattern *p)
{
	
}
static char const NibbleToHexText[] = "0123456789ABCDEF";

static void FieldToText(char *txt, CMPType type, bool ascii, bool hasval, void *fdata)
{
	switch(type)
	{
	case pt_note:
		{
			byte b = *(byte *)fdata;
			if (!hasval)
			{
				// empty
				txt[0] = '.';
				txt[1] = '.';
				txt[2] = '.';
			}
			else if (b == NOTE_OFF)
			{
				// key off
				txt[0] = 'o';
				txt[1] = 'f';
				txt[2] = 'f';
			}
			else
			{
				int octave = b >> 4;
				int note = (b & 15) -1;

				txt[0] = NoteToText[note*2+0];
				txt[1] = NoteToText[note*2+1];
				txt[2] = octave + '0';
			}
			txt[3] = 0;
		}
		break;
	case pt_byte:
		{
			byte b = *(byte *)fdata;
			if (ascii)
			{
				if (!hasval)
					txt[0] = '.';
				else
					txt[0] = b;

				txt[1] = 0;
			}
			else
			{
				if (!hasval)
				{
					txt[0] = '.';
					txt[1] = '.';
				}
				else
				{
					txt[0] = NibbleToHexText[b >> 4];
					txt[1] = NibbleToHexText[b & 15];
				}
				txt[2] = 0;
			}
		}
		break;
	case pt_word:
		{
			word w = *(word *)fdata;
			if (!hasval)
			{
				txt[0] = '.';
				txt[1] = '.';
				txt[2] = '.';
				txt[3] = '.';
			}
			else
			{
				txt[0] = NibbleToHexText[w >> 12];
				txt[1] = NibbleToHexText[(w >> 8) & 15];
				txt[2] = NibbleToHexText[(w >> 4) & 15];
				txt[3] = NibbleToHexText[w & 15];
			}
			txt[4] = 0;
		}
		break;
	case pt_switch:
		{
			byte b = *(byte *)fdata;
			if (hasval)
			{
				if (b == SWITCH_ON)
					txt[0] = '1';
				else if (b == SWITCH_OFF)
					txt[0] = '0';
			}
			else
			{
				txt[0] = '.';
			}
			txt[1] = 0;

		}
		break;

	}
}

static CString FieldToLongText(CMPType type, bool hasval, int v)
{
	CString s;

	switch(type)
	{
	case pt_note:
		{
			if (!hasval)
			{
			}
			else if (v == NOTE_OFF)
			{
				s = "note off";
			}
			else
			{
				int octave = v >> 4;
				int note = (v & 15) -1;

				s = NoteToText[note*2+0];
				s += NoteToText[note*2+1];
				s += (char)(octave + '0');
			}
		}
		break;
	default:
		if (v != -1)
		{
			if (v > 255)
				s.Format("%04X (%d)", v, v);
			else
				s.Format("%02X (%d)", v, v);
		}

		break;


	}

	return s;
}

static void FieldToTextExport(char *txt, CMPType type, bool hasval, void *fdata)
{
	switch(type)
	{
	case pt_note:
		{
			byte b = *(byte *)fdata;
			if (!hasval)
			{
				// empty
				txt[0] = 0;
			}
			else if (b == NOTE_OFF)
			{
				// key off
				txt[0] = 'o';
				txt[1] = 'f';
				txt[2] = 'f';
				txt[3] = 0;
			}
			else
			{
				int octave = b >> 4;
				int note = (b & 15) -1;

				txt[0] = NoteToText[note*2+0];
				txt[1] = NoteToText[note*2+1];
				txt[2] = octave + '0';
				txt[3] = 0;
			}
			
		}
		break;
	case pt_byte:
		{
			byte b = *(byte *)fdata;
			if (!hasval)
			{
				txt[0] = 0;
			}
			else
			{
				txt[0] = NibbleToHexText[b >> 4];
				txt[1] = NibbleToHexText[b & 15];
				txt[2] = 0;
			}
			
		}
		break;
	case pt_word:
		{
			word w = *(word *)fdata;
			if (!hasval)
			{
				txt[0] = 0;
			}
			else
			{
				txt[0] = NibbleToHexText[w >> 12];
				txt[1] = NibbleToHexText[(w >> 8) & 15];
				txt[2] = NibbleToHexText[(w >> 4) & 15];
				txt[3] = NibbleToHexText[w & 15];
				txt[4] = 0;
			}
			
		}
		break;
	case pt_switch:
		{
			byte b = *(byte *)fdata;
			if (hasval)
			{
				if (b == SWITCH_ON)
					txt[0] = '1';
				else if (b == SWITCH_OFF)
					txt[0] = '0';
				txt[1] = 0;
			}
			else
			{
				txt[0] = 0;
			}
			
		}
		break;

	}
}

int TxtToNote(char c)
{
	if ((c>='a') && (c<='g')) c= c - 'a' + 'A';

	switch(c)
	{
	case 'C' : return 0; break;
	case 'D' : return 2; break;
	case 'E' : return 4; break;
	case 'F' : return 5; break;
	case 'G' : return 7; break;
	case 'A' : return 9; break;
	case 'B' : return 11;break;
	default : return 0;
	}
}

int HexToInt(char c)
{
	int res = 0;
	
	if ((c>='0') && (c<='9')) res = (c -'0');
	else if ((c>='A') && (c<='Z')) res = (10 + c -'A');
	else if ((c>='a') && (c<='z')) res = (10 + c -'a');
	
	return res;
}

char IntToHex(int i)
{
	char res = '0';
	
	if ((i>=0) && (i<=9)) res = ('0'+i);
	else if ((i>=10) && (i<=35)) res = ('A'+i-10);

	return res;
}


void CPatEd::TextToFieldImport(char *txt, CColumn *pc, int irow)
{
//	char debugtxt[256];
	
	switch(pc->GetParamType())
	{
	case pt_note:
		{
		if (txt[0]==0) pc->ClearValue(irow);
		else if (txt[1]==0) pc->ClearValue(irow);
		else if (txt[2]==0) pc->ClearValue(irow);
		else
		if ((txt[0] == 'o') && (txt[1] == 'f')  && (txt[2] == 'f'))
			pc->SetValue(irow, NOTE_OFF);
		else
		{
			int octave = (txt[2] - '0');
			int note = TxtToNote(txt[0]);
			if (txt[1] =='#') note++; 
			pc->SetValue(irow, (octave << 4) + note + 1);
		}
	
		}
		break;
	case pt_byte:
		{
			if (txt[0]==0) pc->ClearValue(irow);
			else if (txt[1]==0) pc->SetValue(irow, HexToInt(txt[0]));
			else pc->SetValue(irow, 16*HexToInt(txt[0]) + HexToInt(txt[1]));
		}
		break;
	case pt_word:
		{
			if (txt[0]==0) pc->ClearValue(irow);
			else if (txt[1]==0) pc->SetValue(irow, HexToInt(txt[0]));
			else if (txt[2]==0) pc->SetValue(irow, 16*HexToInt(txt[0])+HexToInt(txt[1]));
			else if (txt[3]==0) pc->SetValue(irow, 16*(16*HexToInt(txt[0])+HexToInt(txt[1]))+HexToInt(txt[2]));
			else pc->SetValue(irow, 16*(16*(16*HexToInt(txt[0])+HexToInt(txt[1]))+HexToInt(txt[2])+HexToInt(txt[3])));
		}
		break;
	case pt_switch:
		{
			if (txt[0]==0) pc->ClearValue(irow);
			else if (txt[0]='0') pc->SetValue(irow, SWITCH_OFF);
			else pc->SetValue(irow, SWITCH_ON);
		}
		break;

	}

}


void CPatEd::SetPlayPos(MapIntToPlayingPattern &pp, CMasterInfo *pmi)
{
	MapIntToPlayingPattern::iterator i;

	for (i = pp.begin(); i != pp.end(); i++)
	{
		CPlayingPattern *p = (*i).second->GetFirstPlayingPattern(pew->pPattern);
		if (p != NULL)
		{
			pPlayingPattern = p->ppat;
			playPos = (int)(p->GetPlayPos() * pew->fontSize.cy);
		
			return;
		}

	}

	if (i == pp.end())
	{
		pPlayingPattern = NULL;
		playPos = 0;
	}
}

CColumn *CPatEd::GetCursorColumn()
{
	if (pew->pPattern == NULL)
		return NULL;

	if (cursor.column < 0 || cursor.column >= (int)pew->pPattern->columns.size())
		return NULL;

	return pew->pPattern->columns[cursor.column].get();
}

bool DialogFileName(LPSTR Suffix, LPSTR FileLibelle, LPSTR FileTitle, LPSTR InitFilename, LPSTR pathName, bool DoOpen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	char szFilters[255];
	char ssuf[255];
    sprintf(szFilters, "%s (*.%s)|*.%s|All Files (*.*)|*.*||", FileLibelle, Suffix, Suffix);
    sprintf(ssuf, "*.%s", Suffix);
	int OpenMode = (DoOpen ? 1 : 0);
	int OpenParams = (DoOpen ? OFN_HIDEREADONLY|OFN_FILEMUSTEXIST : OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT);

	CFileDialog dlgFile(OpenMode, Suffix, ssuf, OpenParams, szFilters);
	
	dlgFile.m_ofn.lpstrTitle = FileTitle;

	CString InitName(InitFilename);
	dlgFile.m_ofn.lpstrFile = InitName.GetBuffer(_MAX_PATH);

	if (dlgFile.DoModal() != IDOK)
		return false;

	sprintf(pathName, "%s", dlgFile.GetPathName());
	return true;	
}

void CPatEd::ExportPattern()
{
	char exportpathName[255];
	char pathName[255];
	pCB->GetProfileString("ExportPathName", exportpathName, "");

	if (!DialogFileName("csv", "Pattern file", "Export pattern", exportpathName, pathName, false))
		return;

	pCB->WriteProfileString("ExportPathName", pathName);
	
	CMachinePattern *ppat = pew->pPattern;
	int const firstrow = 0;
	int const lastrow = pew->pPattern->GetRowCount();

	ofstream expfile (pathName, ios::out | ios::trunc);  
	if (expfile)  
    {
		// Export the pattern in a csv format
		// First line : column header		
		for (int col = 0; col < (int)ppat->columns.size(); col++)
		{
			CColumn *pc = ppat->columns[col].get();
			if (col>0)
			  expfile << ";" << pc->GetName();
			else
			  expfile << pc->GetName();
		}
		expfile << endl;

		for (int y = firstrow; y < lastrow; y++)
		{
			
			for (int col = 0; col < (int)ppat->columns.size(); col++)
			{
				CColumn *pc = ppat->columns[col].get();
				MapIntToValue::const_iterator ei = pc->EventsBegin();

				while(ei != pc->EventsEnd() && (*ei).first < y) ei++;

				int data = pc->GetNoValue();
				bool hasvalue = false;
				if (ei != pc->EventsEnd() && (*ei).first == y)
				{
					hasvalue = true;
					data = (*ei).second;
					ei++;
				}
				if (pc->GetRealNoValue() < 0)
					hasvalue = true;

				char txt[6];
				FieldToTextExport(txt, pc->GetParamType(), hasvalue, (void *)&data);
				if (col>0)
				  expfile << ";" << txt;
				else
				  expfile << txt;
			}
			expfile << endl;
		}
		
		expfile.close();  
    }
}

void CPatEd::ImportPattern()
{
	char exportpathName[255];
	char pathName[255];
	pCB->GetProfileString("ExportPathName", exportpathName, "");

	if (!DialogFileName("csv", "Pattern file", "Import pattern", exportpathName, pathName, true))
		return;

	pCB->WriteProfileString("ExportPathName", pathName);

	CMachinePattern *ppat = pew->pPattern;
	ppat->actions.BeginAction(pew, "Import pattern");
	{
		MACHINE_LOCK;
	
		int lastrow = pew->pPattern->GetRowCount();
		int lastcol = (int)ppat->columns.size();

		ifstream expfile (pathName, ios::in);  
		if (expfile)  
		{
			string impRow;
			char txt[6];
			int itxt=0;
			int icol=0;
			int irow=0;
		
			// 1rst line is column header : just read it
			getline(expfile, impRow);

			while (getline(expfile, impRow))  
			{	// Import the pattern in a csv format
				// Extract each field, separator : ";"
	
				itxt=0;
				icol=0;

				if (pew->ImportAutoResize)
				{
					// check if enough rows in the pattern
					if (irow >= lastrow) 
					{
						lastrow = lastrow + BUZZ_TICKS_PER_BEAT;
						pCB->SetPatternLength(ppat->pPattern, lastrow);
					}
				}
				else
				if (irow >= lastrow) 
					break;


				for (int i=0; i < (int)impRow.length(); i++)
				{
					if (impRow[i] ==';') 
					{
						txt[itxt]=0;
						itxt=0;
						// Import the field
						if (icol<lastcol)
						{
							CColumn *pc = ppat->columns[icol].get();
							TextToFieldImport(txt, pc, irow);
						}
						icol++;
					}
					else {
						if (impRow[i]!=' ')
						{
							txt[itxt]= impRow[i];
							itxt++;
						}
					}


				}
				txt[itxt]=0; 
				itxt=0;
				// Import the last field
				if (icol<lastcol){
					CColumn *pc = ppat->columns[icol].get();
					TextToFieldImport(txt, pc, irow);
				}

				// Get next row
				irow++;
			}
		
			expfile.close();  
    
		}
	}
	pCB->SetModifiedFlag();
	pew->UpdateCanvasSize();
	pew->Invalidate();
	DoAnalyseChords();

}

void CPatEd::InflatePattern(int delta)
{
	if (delta > 0) {
	 
		// Expand pattern x delta
		CMachinePattern *ppat = pew->pPattern;
		
		ppat->actions.BeginAction(pew, "Inflate pattern");
		{
			MACHINE_LOCK;
			// Resize the pattern * delta
			pCB->SetPatternLength(ppat->pPattern, ppat->numBeats * delta * BUZZ_TICKS_PER_BEAT);

			for (int col = 0; col < (int)ppat->columns.size(); col++)
			{
				CColumn *pc = ppat->columns[col].get();

				MapIntToValue colbuf;
				MapIntToValue::const_iterator ei;

				// First, copy the data of the column to colbuf
				for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
				{					
					int y = (*ei).first;
					int data = (*ei).second;						
					colbuf[y] = data;					
				}

				// Clear the column
				pc->Clear();

				// Now move the data from y to delta*y
				for (ei = colbuf.begin(); ei != colbuf.end(); ei++)
				{					
					int y = (*ei).first;
					int data = (*ei).second;

					// Move value to row : y * delta
					pc->SetValue(delta*y, data);					
				}
			}
		}

		pCB->SetModifiedFlag();
		ColumnsChanged();
		pew->UpdateCanvasSize();
		pew->Invalidate();
	}
	else if (delta < 0) {
		// Collapse pattern / delta
		delta = -delta;
		CMachinePattern *ppat = pew->pPattern;
		
		// Do not collapse to death !
		if (ppat->numBeats / delta * BUZZ_TICKS_PER_BEAT >= 1) 
		{

			ppat->actions.BeginAction(pew, "Inflate pattern");
			{
				MACHINE_LOCK;
				// Expand rows per beats
				ppat->SetRowsPerBeat(ppat->rowsPerBeat * delta);

				for (int col = 0; col < (int)ppat->columns.size(); col++)
				{
					CColumn *pc = ppat->columns[col].get();

					MapIntToValue colbuf;
					MapIntToValue::const_iterator ei;

					// First, copy the data of the column to colbuf
					for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
					{					
						int y = (*ei).first;
						int data = (*ei).second;						
						colbuf[y] = data;					
					}

					// Clear the column
					pc->Clear();

					// Now move the data from y to y/delta
					for (ei = colbuf.begin(); ei != colbuf.end(); ei++)
					{					
						int y = (*ei).first;
						int data = (*ei).second;

						// Move value to row : y / delta
						pc->SetValue(y/delta, data);					
					}
				}

				// Resize the pattern / delta
				pCB->SetPatternLength(ppat->pPattern, ppat->numBeats / delta * BUZZ_TICKS_PER_BEAT);
			}
		}

		pCB->SetModifiedFlag();
		ColumnsChanged();
		pew->UpdateCanvasSize();
		pew->Invalidate();
	}
	DoAnalyseChords();

}

void CPatEd::OnDraw(CDC *pDC)
{

	CRect clr;
	GetClientRect(&clr);
	CRect cr = GetCanvasRect();
	CRect ur;
	ur.UnionRect(&cr, &clr);

	CRect cpr;
	pDC->GetClipBox(&cpr);

	bgcol = pew->pCB->GetThemeColor("PE BG");
	bgcoldark = pew->pCB->GetThemeColor("PE BG Dark");
	bgcolvdark = pew->pCB->GetThemeColor("PE BG Very Dark");
	bgsel = pew->pCB->GetThemeColor("PE Sel BG");
	COLORREF textcolor = pew->pCB->GetThemeColor("PE Text");

	COLORREF itextcolor = Blend(textcolor, RGB(255, 0, 0), 0.66f);
	COLORREF mtextcolor = Blend(textcolor, RGB(0, 0, 255), 0.66f);

//	COLORREF col = RGB(rand(), rand(), rand());
//	pDC->FillSolidRect(&ur, col);
	pDC->FillSolidRect(&ur, bgcol);


	CMachinePattern *ppat = pew->pPattern;

	if (ppat == NULL || ppat->columns.size() == 0)
		return;


	CObject *pOldFont = pDC->SelectObject(&pew->font);
	pDC->SetBkMode(OPAQUE);

	CPen pen(PS_SOLID, 1, Blend(textcolor, bgcol, 0.5f));
	CObject *pOldPen = pDC->SelectObject(&pen);

	int x = 0;
	CColumn *lastcolumn = NULL;

	for (int col = 0; col < (int)ppat->columns.size(); col++)
	{
		CColumn *pc = ppat->columns[col].get();

		if (lastcolumn != NULL && !pc->MatchMachine(*lastcolumn))
		{
			// draw a vertical separator between machines
			int sx = x - pew->fontSize.cx / 2;
			pDC->MoveTo(sx, 0);
			pDC->LineTo(sx, ppat->GetRowCount() * pew->fontSize.cy);
		}

		if (x >= cpr.right)
			break;

		int w = GetColumnWidth(col);
		
		if (x + w >= cpr.left)
		{
			COLORREF tc;

			if (pc->IsMidi())
				tc = mtextcolor;
			else if (pc->IsInternal())
				tc = itextcolor;
			else
				tc = textcolor;

			if (pew->pPattern->IsTrackMuted(pc))
				tc = Blend(tc, bgcol, 0.5f);

			DrawColumn(pDC, col, x, tc, cpr);
		}

		x += w;

		lastcolumn = pc;
	}

	DrawCursor(pDC);

	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldPen);

	if (drawPlayPos >= 0)
	{
		CRect ppr = cr;
		ppr.top = drawPlayPos;
		ppr.bottom = ppr.top + 1;

//		pDC->InvertRect(&ppr);

		if (pCB->GetStateFlags() & SF_RECORDING)
		{
			CBrush br(RGB(255, 0, 0));
			pDC->FillRect(&ppr, &br);
		}
		else
		{
			CBrush br(RGB(255, 255, 0));
			pDC->FillRect(&ppr, &br);
		}
	}


	UpdateStatusBar();
}

void CPatEd::DrawColumn(CDC *pDC, int col, int x, COLORREF textcolor, CRect const &clipbox)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[col].get();
	CColumn *pnc = NULL;

	if (col < (int)ppat->columns.size() - 1)
		pnc = ppat->columns[col + 1].get();

	bool muted = pew->pPattern->IsTrackMuted(pc);
	pDC->SetTextColor(textcolor);

	MapIntToValue::const_iterator ei = pc->EventsBegin();

	int const firstrow = max(0, clipbox.top / pew->fontSize.cy);
	int const lastrow = min(pew->pPattern->GetRowCount(), clipbox.bottom / pew->fontSize.cy + 1);

	while(ei != pc->EventsEnd() && (*ei).first < firstrow) ei++;

	for (int y = firstrow; y < lastrow; y++)
	{
		int data = pc->GetNoValue();
		bool hasvalue = false;

		if (ei != pc->EventsEnd() && (*ei).first == y)
		{
			hasvalue = true;
			data = (*ei).second;
			ei++;
		}

		if (pc->IsGraphical())
			DrawGraphicalField(pDC, col, pnc, data, x, y, muted, hasvalue, textcolor);
		else
			DrawField(pDC, col, pnc, data, x, y, muted, hasvalue, textcolor);
	}
}

bool DrawMeasureBar(int aRow, int arowsPerBeat, int aBarCnt) 
{
	if (aBarCnt==0) // Automatic mode
	{ 
		return ((aRow % (4 * arowsPerBeat)) == 0);
	}
	else
		return ((aRow % (aBarCnt * arowsPerBeat)) == 0);

}

int CPatEd::BeatsInMeasureBar()
{
	CMachinePattern *ppat = pew->pPattern;
	if ((pew->BarComboIndex<=0) || (pew->BarComboIndex>8)) // Automatic mode
	{ 
		int bar = 4;
	
		if (ppat->numBeats % 13 == 0) bar = 13;
		else if (ppat->numBeats % 11 == 0) bar = 11;
		else if (ppat->numBeats % 9 == 0) bar = 9;
		else if (ppat->numBeats % 7 == 0) bar = 7;
		else if (ppat->numBeats % 5 == 0) bar = 5;
		else if (ppat->numBeats % 3 == 0) bar = 3;

		return (bar * ppat->rowsPerBeat);
	}
	else
		return (pew->BarComboIndex * ppat->rowsPerBeat);
}

COLORREF CPatEd::GetFieldBackgroundColor(CMachinePattern *ppat, int row, int col, bool muted)
{
	COLORREF color;

	int bar = 4;
	if ((pew->BarComboIndex >0) && (pew->BarComboIndex <=8)) 
		bar = pew->BarComboIndex;
	else if (ppat->numBeats % 13 == 0) bar = 13;
	else if (ppat->numBeats % 11 == 0) bar = 11;
	else if (ppat->numBeats % 9 == 0) bar = 9;
	else if (ppat->numBeats % 7 == 0) bar = 7;
	else if (ppat->numBeats % 5 == 0) bar = 5;
	else if (ppat->numBeats % 3 == 0) bar = 3;

	if (InSelection(row, col))
	{
		color = bgsel;
	}
	else if (DrawMeasureBar(row, ppat->rowsPerBeat, bar))
	{
		if (muted)
			color = Blend(bgcolvdark, bgcol, 0.5f);
		else
			color = bgcolvdark;
	}
	else if ((row % ppat->rowsPerBeat) == 0)
	{
		if (muted)
			color = Blend(bgcoldark, bgcol, 0.5f);
		else
			color = bgcoldark;
	}
	else
	{
		color = bgcol;
	}

	return color;
}


bool CPatEd::CheckNoteInTonality(byte note)
{
	// If no tonality defined, return true
	if (pew->TonalComboIndex <= 0) return true;
	if (pew->TonalComboIndex > (int)pew->TonalityList.size()) return true;

	// Check if note is in the tonality defined for the pattern
	return (pew->TonalityList[pew->TonalComboIndex].notes.test(note));
}

bool CPatEd::CheckLeadingTone(byte note)
{
	// If no tonality defined, return false
	if (pew->TonalComboIndex <= 0) return false;
	if (pew->TonalComboIndex > (int)pew->TonalityList.size()) return false;

	// If Major tonality, return false
	if (pew->TonalityList[pew->TonalComboIndex].major) return false;

	// Test the "leading tone" of the minor mode (the seventh scale degree of the diatonic scale)
	int leadingtone = pew->TonalityList[pew->TonalComboIndex].base_note - 1;
	if (leadingtone < 0) leadingtone = leadingtone + 12;

	return (note == leadingtone);
}

bool CPatEd::IsMajorTonality()
{
	// If no tonality defined, return true
	if (pew->TonalComboIndex <=0) return true;
	if (pew->TonalComboIndex > (int)pew->TonalityList.size()) return true;

	// Check if the tonality defined for the pattern is in major mode
	return (pew->TonalityList[pew->TonalComboIndex].major);
}

void CPatEd::DrawField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue, COLORREF textcolor)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[col].get();

	if (pc->GetRealNoValue() < 0)
		hasvalue = true;

	char txt[6];
	FieldToText(txt, pc->GetParamType(), pc->IsAscii(), hasvalue, (void *)&data);
	
	bool SetTextColorBack = false;
	if (hasvalue && pc->GetParamType()==pt_note)
	{
		byte b = (byte)data;
		if (b != NOTE_OFF)
			if (CheckNoteInTonality((b & 15) - 1))
			{
				if (CheckLeadingTone((b & 15) - 1)) {
					COLORREF color;
					SetTextColorBack = true;
					color = RGB(0, 255, 0);
					if (muted) color = Blend(color, bgcol, 0.5f);
					pDC->SetTextColor(color);
				}
			}
			else 
			{
				SetTextColorBack = true;
				COLORREF color;
				color = RGB(255, 0, 0);
				if (muted) color = Blend(color, bgcol, 0.5f);
				pDC->SetTextColor(color);
			}
	}

	int len = (int)strlen(txt);
	if (pnc != NULL && pc->MatchGroupAndTrack(*pnc))
	{
		txt[len++] = ' ';
		txt[len] = 0;
	}

	CRect r;
	r.left = x;
	r.top = y * pew->fontSize.cy;
	r.right = x + len * pew->fontSize.cx;
	r.bottom = r.top + pew->fontSize.cy;

	if (pDC->RectVisible(&r))
	{
		pDC->SetBkColor(GetFieldBackgroundColor(ppat, y, col, muted));

		pDC->TextOut(x, y * pew->fontSize.cy, txt, len);

//		pDC->TextOut(x - pew->fontSize.cx, y * pew->fontSize.cy, "-");
	}
	if (SetTextColorBack) pDC->SetTextColor(textcolor);

}

void CPatEd::DrawGraphicalField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue, COLORREF textcolor)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[col].get();

	int w = pc->GetWidth(pCB);
	bool extraspace = pnc != NULL && pc->MatchGroupAndTrack(*pnc);
	if (!extraspace)
		w--;

	CRect r;
	r.left = x;
	r.top = y * pew->fontSize.cy;
	r.right = x + w * pew->fontSize.cx;
	r.bottom = r.top + pew->fontSize.cy;

	if (pDC->RectVisible(&r))
	{
		COLORREF bgc = GetFieldBackgroundColor(ppat, y, col, muted);
		pDC->FillSolidRect(&r, bgc);

		r.left += 2;
		r.top += 2;
		r.bottom -= 2;
		r.right -= 2 + (extraspace ? pew->fontSize.cx : 0);

		pDC->FillSolidRect(&r, hasvalue ? textcolor : Blend(textcolor, bgc, 0.5f));

		r.left += 1;
		r.top += 1;
		r.right -= 1;
		r.bottom -= 1;

		double v = pc->GetValueNormalized(y);

		if (hasvalue)
			r.left = min(r.right, r.left + (int)(v * ((r.right - r.left) + 1)));

		pDC->FillSolidRect(&r, bgc);

	}

}


bool CPatEd::CheckNoteCol()
{
	if (pew->pPattern == NULL)
		return false;

	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();
	if (pc->GetParamType() != pt_note)
		return false;

	return true;
}

bool CPatEd::CanInsertChord()
{
	if (pew->pPattern == NULL)
		return false;

	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();
	if (pc->GetParamType() != pt_note)
		return false;

	// Is there a note here ?
	int value = pc->GetValue(cursor.row);
	return ((pc->HasValue(cursor.row)) && (value != pc->GetNoValue()));

}

void CPatEd::InsertChordNote(int note, int ChordIndex)
{
	if (pew->pPattern == NULL)
		return;
	// Insert note
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	if (pc->GetParamType() != pt_note)
		return;

	ppat->actions.BeginAction(pew, "Insert chord and base");
	{
		MACHINE_LOCK;
		pc->SetValue(cursor.row, note);	
		DoInsertChord(ChordIndex);
	}	
}

void CPatEd::InsertChord(int ChordIndex)
{
	if (CanInsertChord())
	{
		CMachinePattern *ppat = pew->pPattern;
		CColumn *pc = ppat->columns[cursor.column].get();
		ppat->actions.BeginAction(pew, "Insert chord");
		{
			MACHINE_LOCK;
			DoInsertChord(ChordIndex);
		}
	}
}

	
void CPatEd::DoInsertChord(int ChordIndex)
{
	// Insert a chord on the current row if there is a note in the current col
	// The notes of the chord ar inserted one by one in the tracks on the right.		
	
	// Must be on a column note
	// Is there a note here ?
	if (CanInsertChord())
	{
		int ChordNotes[256];
		int ChordNotesIndex = 0;
		for (int i=0; i<256; i++) ChordNotes[i] = 0;

		CMachinePattern *ppat = pew->pPattern;
		CColumn *pc = ppat->columns[cursor.column].get();

		// Is there a selection to limit the generation of the chord ?
		int LastCol=0;
		if (selection){
			CRect r= GetSelRect();
			if (r.right > cursor.column)
				LastCol = r.right;
		}

		// Keep the root note of the chord
		int value = pc->GetValue(cursor.row);
		ChordNotes[ChordNotesIndex] = value;
		ChordNotesIndex++;

		int rootChord = value;

		bool stopChord = false;
		bool cleanTracks = false;
		int iChord = 0;
		int delta;
		int lastnote;
		char c;
		// Get current chord description
		char SelectedChord[20];
		if (ChordIndex <0) ChordIndex = pew->ChordsComboIndex();
		strcpy(SelectedChord, pew->Chords[ChordIndex].c_str());
			
		// Get next track, next note column
		int icol = cursor.column+1;
		while (!stopChord) 
		{
			while ((icol<(int)ppat->columns.size()) && (ppat->columns[icol]->GetParamType() != pt_note))
				icol++;
		
			if (icol<(int)ppat->columns.size())
			{ 
				// Check column limit
				if ((LastCol<=0) || (icol<=LastCol))
				{
					if (cleanTracks)
					{
						if (pew->ArpeggioComboIndex<=0)	ppat->columns[icol]->ClearValue(cursor.row);
						icol++;
					}
					else
					{
						// Note column found
						c = SelectedChord[iChord]; 
						if (c==0)
						{	// Restart chord with base note, add 1 octave
							if (pew->InsertChordOnce)
								cleanTracks=true;
							else {
								value = EncodeNote(DecodeNote(rootChord) + 12);
								/* ADD a param here
								while (value<=lastnote) {
									value = EncodeNote(DecodeNote(value) + 12);
								}
								*/
								rootChord = value;
								iChord = 0;
							}
						}
						else 
						{	// Get next note of chord
							delta = HexToInt(c);
							value = EncodeNote(DecodeNote(rootChord) + delta);
							iChord++;
						}	
							
						if (cleanTracks)
						{
							if (pew->ArpeggioComboIndex<=0)	ppat->columns[icol]->ClearValue(cursor.row);
						}
						else {
							// Check if the note is valid
							int octave = value >> 4;
							int note = (value & 15) -1;
							// Last note is B-9
							if ((!stopChord) && (octave<10)) {
								if (pew->ArpeggioComboIndex<=0)	ppat->columns[icol]->SetValue(cursor.row, value);	
								lastnote = value;
								ChordNotes[ChordNotesIndex] = value;
								ChordNotesIndex++;
							}
							else 
								cleanTracks = true;
						}
						icol++;
					}
				}
				else 
				// Chords generated in the selection, don't clean the tracks outside
					stopChord = true;
			}
			else
			{	// No more track : end of the chord generation
				stopChord = true;
			}

		}

		// Generate the notes in the pattern according to the selected arpeggio 
		// 1rst column is : cursor.column (contains the root note)
		icol = cursor.column;
		int irow = cursor.row;

		// Use default arpeggio ?
		if (pew->ArpeggioComboIndex>0)
		{
			for (int ArpeggioRow=0; ArpeggioRow < pew->ArpeggioRowCount; ArpeggioRow++)
			{
				// Clean the row (irow+ArpeggioRow)
				int columnsize = (int)ppat->columns.size();
				if ((LastCol>0) && (LastCol<columnsize)) columnsize = LastCol+1;

				for (int c = 0; c < columnsize ; c++)
					if (ppat->columns[c]->GetParamType() == pt_note)
						ppat->columns[c]->ClearValue(irow+ArpeggioRow);

				// Insert the NoteOff 
				// ArpeggioNoteOffRows[ArpeggioRow][] gives the NoteOff to insert in the row
				for (int iArpeggioCol = 0; pew->ArpeggioNoteOffRows[ArpeggioRow][iArpeggioCol] != 0; iArpeggioCol++)
				{
					int iNote = HexToInt(pew->ArpeggioNoteOffRows[ArpeggioRow][iArpeggioCol]);
					// Insert noteOff (if iNote < ChordNotesIndex) at the iNoteth note column 
					if (iNote <= ChordNotesIndex)
					{
						icol = cursor.column;
						int icolcount;
						// Get the column iNoteth
						for (icolcount = 0; (icolcount<iNote) && (icol<columnsize); icolcount++)
						{
							while ((icol<columnsize) &&
								(ppat->columns[icol]->GetParamType() != pt_note))
								icol++;
							if (icolcount<iNote - 1) icol++;
						}
						if ((icol < columnsize) && (icolcount == iNote))
						{
							ppat->columns[icol]->SetValue(irow + ArpeggioRow, NOTE_OFF);
						}

					}

				}
				// Insert the notes 
				// ArpeggioRows[ArpeggioRow][] gives the notes to insert in the row
				for (int iArpeggioCol=0; pew->ArpeggioRows[ArpeggioRow][iArpeggioCol]!=0; iArpeggioCol++)
				{
					int iNote = HexToInt(pew->ArpeggioRows[ArpeggioRow][iArpeggioCol]);
					// Insert note ChordNotes[] (if iNote < ChordNotesIndex) at the iNoteth note column 
					if (iNote <= ChordNotesIndex)
					{
						icol = cursor.column;
						int icolcount;
						// Get the column iNoteth
						for (icolcount=0; (icolcount<iNote) && (icol<columnsize); icolcount++)
						{
							while ((icol<columnsize) && 
								   (ppat->columns[icol]->GetParamType() != pt_note))
								icol++;
							if (icolcount<iNote-1) icol++;
						}
						if ((icol < columnsize) && (icolcount==iNote))
						{
							ppat->columns[icol]->SetValue(irow+ArpeggioRow, ChordNotes[iNote-1]);
						}

					}

				}
			}
		}
		
		
		pCB->SetModifiedFlag();
		ColumnsChanged();
		pew->UpdateCanvasSize();
		pew->Invalidate();
		DoAnalyseChords();
	}	
}

bool CPatEd::SaveArpeggio()
{
	// Copy data from the selection to the internal structure of arpeggio
	if (selection)
	{
		CRect r= GetSelRect();
		CMachinePattern *ppat = pew->pPattern;

		pew->ArpeggioRowCount=r.bottom-r.top+1;

		for (int row=r.top; row<=r.bottom; row++)
		{
			char ArpBuf[256];
			char ArpNoteOffBuf[256];
			int iArpBuf = 0;
			int iArpNoteOffBuf = 0;
			int iCol=0;

			for (int col=r.left; col<=r.right; col++)
			{
				CColumn *pc = ppat->columns[col].get();
				
				if (pc->GetParamType() == pt_note) {
					iCol++;
					int val = pc->GetValue(row);
					if (val != pc->GetNoValue()) {
						if (val != NOTE_OFF) {
							ArpBuf[iArpBuf] = IntToHex(iCol);
							iArpBuf++;
						}
						else {
							ArpNoteOffBuf[iArpNoteOffBuf] = IntToHex(iCol);
							iArpNoteOffBuf++;
						}

					}
				}
			}
			ArpBuf[iArpBuf]=0;
			ArpNoteOffBuf[iArpNoteOffBuf] = 0;
			strcpy(pew->ArpeggioRows[row-r.top], ArpBuf);	
			strcpy(pew->ArpeggioNoteOffRows[row - r.top], ArpNoteOffBuf);
			
		}
	}
	else
		return false;
	return true;
}

	
int CPatEd::TestChords(note_bitset n, int ir)
// Return value
//  0 : found
//  1 : not enough notes
// -1 : too much notes
// -2 : not found
{
	// Test each 12 base notes for each chord
	// first base note is C
	if ((int)n.count() < pew->minChordNotes)
		// Not enough notes to make a chord
		return 1;
	else if ((int)n.count() > pew->maxChordNotes)
		// Too much notes to make a chord
		return -1;
	else {
		int octave = pew->RowNotes[ir].base_data >> 4;
		int note = (pew->RowNotes[ir].base_data & 15) - 1;

		// Start with base note (should be the root of the chord)
		for (int ib = 0; ib < note; ib++) {
			bool n0 = n[0];
			n = (n >> 1);
			n.set(11, n0);
		}

		// Test the 12 root notes chords
		for (int ib = note; ib<note + 12; ib++) {
			for (int ic = 0; ic< (int)pew->ChordsBase.size(); ic++) {
			if (n == pew->ChordsBase[ic].notes) {
				pew->RowNotes[ir].chord_index = ic;
				pew->RowNotes[ir].base_note = ib % 12;
				pew->RowNotes[ir].base_octave = octave;
				return 0;
				}
			}
			// No match, test next base note
			bool n0 = n[0];
			n = (n>>1);
			n.set(11, n0);
		}
		// No chord found, set a special value to chord_index
		pew->RowNotes[ir].chord_index = -2;
		return -2;
	}
}
	
void CPatEd::DoAnalyseChords()
{
	if (pew->AutoChordExpert && pew->ChordExpertvisible) 
		AnalyseChordRefresh= true;
}

void CPatEd::DoManualAnalyseChords()
{
	if (pew->ChordExpertvisible) 
		AnalyseChordRefresh= true;
}

void CPatEd::CheckRefreshChords()
{
	if (!pew->ChordExpertvisible) return;
	if (!AnalyseChordRefresh) return;

	CheckRefreshChordCount++;
	if (CheckRefreshChordCount>10)
	{
		CheckRefreshChordCount = 0;
		AnalyseChordRefresh = false;
		
		AnalyseChords();
		pew->leftwnd.Invalidate();
	}
}

void CPatEd::AnalyseTonality()
{

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL) return;
	
	// Create the array of notes
	for (int i=0; i<12; i++) pew->tonality_notes[i]=0;

	// Fill the note rows 
	for (int col = 0; col<(int)ppat->columns.size(); col++)
	{
		CColumn *pc = ppat->columns[col].get();
			
		if (pc->GetParamType()==pt_note)
		{
			MapIntToValue::const_iterator ei;

			for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
			{					
				int y = (*ei).first;
				byte data = (byte)(*ei).second;
				if (data != NOTE_OFF)
				{
					// int octave = data >> 4;
					int note = (data & 15) -1;
					pew->tonality_notes[note]++;
				}
			}
		}
	}
}

void CPatEd::AnalyseChordsMeasure(int rStart, int rStop)
// From rStart to rStop (not included)
{
	__try
	{
		CMachinePattern *ppat = pew->pPattern;
		note_bitset n;

		// Check if quitting the analyse quickly
		if (pew->Closing) __leave;

		n = 0;
		int bn = 1000;
		// Get the notes of the interval
		for (int ir = rStart; ir < rStop; ir++)	{
			n = n | pew->RowNotes[ir].notes;
			bn = min(bn, pew->RowNotes[ir].base_data);
		}
		// Test if it's a chord
		int res = TestChords(n, rStart);

		if (res == 0) // Chord found
		{
			pew->RowNotes[rStart].base_data = bn;
			return; 
		}
		if (res > 0) return;  // Not enough notes to make a chord : no need to test smaller interval

		// res < 0, try with a subset
		// 1. BeatsInMeasureBar();
		// 2. ppat->rowsPerBeat;
		// 3. 1 row
		int bimb = BeatsInMeasureBar();

		if (rStop - rStart > bimb)
		{
			int nbBIMB = ((rStop - rStart) / bimb);
			for (int i = 0; i<nbBIMB; i++) {
				AnalyseChordsMeasure(rStart, rStart + bimb);
				rStart = rStart + bimb;
			}
		}
		else if (rStop - rStart > ppat->rowsPerBeat)
		{
			int nbM = ((rStop - rStart) / ppat->rowsPerBeat);
			for (int i = 0; i<nbM; i++) {
				AnalyseChordsMeasure(rStart, rStart + ppat->rowsPerBeat);
				rStart = rStart + ppat->rowsPerBeat;
			}

		}
		else
		{
			for (int i = rStart; i<rStop; i++) {
				TestChords(pew->RowNotes[i].notes, i);
			}
		}
		
	}
	__finally
	{
	}
}

void CPatEd::AnalyseChords()
{
	if (AnalysingChords) return;

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL) return;
	
	if (!pew->ChordExpertvisible) return;
	
	__try
	{
		AnalysingChords = true;

		// Check if quitting the analyse quickly
		if (pew->Closing) __leave;

		// Create the rows of the vector
		pew->RowNotes.clear();
		row_struct rs;
		rs.base_octave=12;
		rs.chord_index=-1;
		rs.base_note = 0;
		rs.base_data = 255;
		rs.notes = 0;
		pew->RowNotes.resize(pew->pPattern->GetRowCount(), rs);

		// Fill the note rows 
		for (int col = 0; col<(int)ppat->columns.size(); col++)
		{
			CColumn *pc = ppat->columns[col].get();
			
			if (pc->GetParamType()==pt_note)
			{
				MapIntToValue::const_iterator ei;

				for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
				{					
					// Check if quitting the analyse quickly
					if (pew->Closing) __leave;

					int y = (*ei).first;
					byte data = (byte)(*ei).second;
					if (data != NOTE_OFF)
					{
						// Keep the lowest note of the row
						pew->RowNotes[y].base_data = min(pew->RowNotes[y].base_data, data);
						
						// Keep the lowest octave of the row
//						int octave = data >> 4;
//						pew->RowNotes[y].base_octave = min(pew->RowNotes[y].base_octave, octave);
						
						// Set the current note in the array of the row
						int note = (data & 15) -1;
						pew->RowNotes[y].notes.set(note, true); 
					}
				}
			}
		}

		AnalyseChordsMeasure(0, (int)pew->RowNotes.size());

	}
	__finally
	{
	AnalysingChords = false;
	}
}


int CPatEd::GetColumnWidth(int column)
{
	CMachinePattern *ppat = pew->pPattern;
	return (ppat->columns[column]->GetWidth(pCB)) * pew->fontSize.cx;
}

int CPatEd::GetFirstColumnofTrack(int column)
{
	CMachinePattern *ppat = pew->pPattern;
	return ppat->GetFirstColumnOfTrackByColumn(column);
}

int CPatEd::GetTrackWidth(int column)
{
	CMachinePattern *ppat = pew->pPattern;
	int fc = ppat->GetFirstColumnOfTrackByColumn(column);
	int cc = ppat->GetGroupColumnCount(column);
	int w = 0;
	for (int i = 0; i < cc; i++)
		w += GetColumnWidth(fc + i);
	return w;
}

int CPatEd::GetColumnX(int column)
{
	CMachinePattern *ppat = pew->pPattern;
	int x = 0;

	for (int col = 0; col < min(column, (int)ppat->columns.size() - 1); col++)
		x += GetColumnWidth(col);

	return x;
}

int CPatEd::GetColumnAtX(int x)
{
	CMachinePattern *ppat = pew->pPattern;

	if (ppat == NULL)
		return -1;

	if (x < 0)
		return 0;

	int colx = 0;

	for (int col = 0; col < (int)ppat->columns.size(); col++)
	{
		colx += GetColumnWidth(col);
		if (x < colx - (ppat->columns[col]->IsTiedToNext() ? 0 : pew->fontSize.cx))
			return col;

	}

	return (int)ppat->columns.size() - 1;
}

int CPatEd::GetDigitAtX(int x)
{
	int cx = GetColumnX(GetColumnAtX(x));
	return (x - cx) / pew->fontSize.cx;
}

int CPatEd::GetRowY(int y)
{
	if (y < 0)
		return 0;

	CMachinePattern *ppat = pew->pPattern;
	return min(y / pew->fontSize.cy, ppat->GetRowCount() - 1);
}


CRect CPatEd::GetCursorRect(CCursorPos const &c)
{
	CMachinePattern *ppat = pew->pPattern;

	CRect r;
	r.left = GetColumnX(c.column) + c.digit * pew->fontSize.cx;
	r.top = c.row * pew->fontSize.cy;
	r.right = r.left + pew->fontSize.cx;
	r.bottom = r.top + pew->fontSize.cy;

	return r;
}

void CPatEd::DrawCursor(CDC *pDC)
{
	CRect r = GetCursorRect(cursor);
	pDC->InvertRect(&r);

}

void CPatEd::MoveCursor(CCursorPos newpos, bool killsel)
{
	if (pew->pPattern == NULL)
		return;

	if (pew->pPattern->columns.size() > 0)
	{
		newpos.row = min(max(newpos.row, 0), pew->pPattern->GetRowCount() - 1);
		newpos.column = min(max(newpos.column, 0), (int)pew->pPattern->columns.size() - 1);
		newpos.digit = min(max(newpos.digit, 0), pew->pPattern->columns[newpos.column]->GetDigitCount() - 1);
	}
	else
	{
		newpos.row = newpos.column = newpos.digit = 0;
	}

	if (newpos == cursor)
		return;

	CRect oldr = GetCursorRect(cursor);
	CRect newr = GetCursorRect(newpos);
	CRect ur;
	ur.UnionRect(&oldr, &newr);
	cursor = newpos;

	if (killsel && !persistentSelection)
		KillSelection();

	InvalidateRect(CanvasToClient(ur));
	MakeVisible(GetColumnX(cursor.column) / pew->fontSize.cx + cursor.digit, cursor.row);
	UpdateStatusBar();
	pew->OnUpdatePosition();
}

void CPatEd::InvalidateField(int row, int column)
{
	CMachinePattern *ppat = pew->pPattern;
	CRect r;
	r.left = GetColumnX(column);
	r.right = r.left + GetColumnWidth(column); // - pew->fontSize.cx;
	r.top = row * pew->fontSize.cy;
	r.bottom = r.top + pew->fontSize.cy;
	InvalidateRect(CanvasToClient(r));
}

void CPatEd::InvalidateGroup(int row, int column)
{
	CMachinePattern *ppat = pew->pPattern;

	int fc = ppat->GetFirstColumnOfTrackByColumn(column);
	int cc = ppat->GetGroupColumnCount(column);

	int w = 0;
	
	for (int i = 0; i < cc; i++)
//		w += ppat->columns[fc + i]->GetDigitCount() + 1;
		w += GetColumnWidth(fc + i);

	CRect r;
	r.left = GetColumnX(fc);
	r.right = r.left + w;
	r.top = row * pew->fontSize.cy;
	r.bottom = ppat->GetRowCount() * pew->fontSize.cy;
	InvalidateRect(CanvasToClient(r));
}


void CPatEd::MoveCursorUpDown(int dy)
{
	MoveCursorDelta(0, dy);
}

void CPatEd::MoveCursorPgUpDown(int dy)
{
	CMachinePattern *ppat = pew->pPattern;
	if (dy < 0)
	{
		if (pew->PgUpDownDisabled) MoveCursorDelta(0, -ppat->rowsPerBeat); 
		else MoveCursorDelta(0, -BeatsInMeasureBar()); 
	}
	else if (dy > 0)
	{
		if (pew->PgUpDownDisabled) MoveCursorDelta(0, ppat->rowsPerBeat); 
		else MoveCursorDelta(0, BeatsInMeasureBar());
	}
}


void CPatEd::MoveCursorDelta(int dx, int dy)
{
	CMachinePattern *ppat = pew->pPattern;

	CCursorPos p = cursor;
	p.row = max(0, min(ppat->GetRowCount() - 1, p.row + dy));

	while (dx < 0)
	{
		if (p.digit > 0)
		{
			if (p.digit == 2 && ppat->columns[p.column]->GetParamType() == pt_note)
				p.digit = 0;	// skip middle digit of note
			else
				p.digit--;
		}
		else
		{
			if (p.column > 0)
			{
				p.column--;
				p.digit = ppat->columns[p.column]->GetDigitCount() - 1;
			}
		}
		dx++;
	}

	while (dx > 0)
	{
		if (p.digit < ppat->columns[p.column]->GetDigitCount() - 1)
		{
			if (p.digit == 0 && ppat->columns[p.column]->GetParamType() == pt_note)
				p.digit = 2;	// skip middle digit of note
			else
				p.digit++;
		}
		else
		{
			if (p.column < (int)ppat->columns.size() - 1)
			{
				p.column++;
				p.digit = 0;
			}
		}
		dx--;
	}

	MoveCursor(p);

}

void CPatEd::PatternChanged()
{
	if (!pew->PersistentSelection)
		KillSelection(); // BWC : Keep selection persistent
	MoveCursor(cursor);
	Invalidate();
	UpdateStatusBar();
	pew->AnalyseChords();
}


void CPatEd::ColumnsChanged()
{
	KillSelection();
	MoveCursor(cursor);
	Invalidate();
	UpdateStatusBar();
}


void CPatEd::Tab()
{
	CMachinePattern *ppat = pew->pPattern;

	for (int c = cursor.column; c < (int)ppat->columns.size() - 1; c++)
	{
		if (!ppat->columns[c]->MatchGroupAndTrack(*ppat->columns[cursor.column]))
		{
			CCursorPos p = cursor;
			p.column = c;
			p.digit = 0;
			MoveCursor(p);
			break;
		}
	}
}

void CPatEd::ShiftTab()
{
	CMachinePattern *ppat = pew->pPattern;

	int c;
	for (c = cursor.column - 1; c >= 0; c--)
	{
		if (!ppat->columns[c]->MatchGroupAndTrack(*ppat->columns[cursor.column - 1]))
		{
			CCursorPos p = cursor;
			p.column = c + 1;
			p.digit = 0;
			MoveCursor(p);
			break;
		}
	}

	if (c < 0)
	{
		CCursorPos p = cursor;
		p.column = 0;
		p.digit = 0;
		MoveCursor(p);
	}

}

void CPatEd::Home()
{
	CCursorPos p = cursor;
	p.row = 0;
	MoveCursor(p);
}

void CPatEd::HomeTop()
{
	CCursorPos p = cursor;
	p.row = 0;
	p.column = 0;
	p.digit = 0;
	MoveCursor(p);
}

void CPatEd::HomeOld()
{
	// make it work like buzz :
	// if 1rst col of track but not 1rst track goto 1rst col of first track
	// else if 1rst col of 1rst track goto 1rst col of row
	// else if 1rst col of row goto 1rst col of 1rst row
	// else goto 1rst col of current track
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	int c;
	c = cursor.column - 1;

	if (c<0)
	{	// First col of params, goto 1rst col of 1rst row
		CCursorPos p = cursor;
		p.column = 0;
		p.row = 0;
		p.digit = 0;
		MoveCursor(p);
		return;
	}
	else
	if (!ppat->columns[c]->MatchGroupAndTrack(*ppat->columns[cursor.column]))
	{  // First col of track
		if (ppat->columns[c]->IsTrackParam())
		{  // but not 1rst track, goto 1rst col of first track
			CCursorPos p = cursor;
			p.column = ppat->GetFirstColumnOfTrackByColumn(cursor.column) - pc->GetTrack() * ppat->GetGroupColumnCount(cursor.column);
			p.digit = 0;
			MoveCursor(p);
			return;
		}
		else
		{  //  1rst col of 1rst track goto 1rst col of row
			CCursorPos p = cursor;
			p.column = 0;
			p.digit = 0;
			MoveCursor(p);
			return;
		}

	}
	else
	{ // Not first col of track, goto 1rst col of current track
		CCursorPos p = cursor;
		p.column = ppat->GetFirstColumnOfTrackByColumn(cursor.column);
		p.digit = 0;
		MoveCursor(p);
		return;		

	}
}

void CPatEd::End()
{
	CCursorPos p = cursor;
	p.row = pew->pPattern->GetRowCount() - 1;
	MoveCursor(p);
}

void CPatEd::EndBottom()
{
	CMachinePattern *ppat = pew->pPattern;

	CCursorPos p = cursor;
	p.row = pew->pPattern->GetRowCount() - 1;
	p.column = (int)ppat->columns.size() - 1;
	MoveCursor(p);
}

void CPatEd::EditNote(int n, bool canplay)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();
	if (pc->GetParamType() != pt_note)
		return;

	int value;
	if (n <= -9999)
	{
		value = NOTE_OFF;
	}
	else
	{
		n += pew->pCB->GetBaseOctave() * 12;
		int octave = n / 12;
		int note = n % 12;

		if (octave > 9)
		{
			octave = 9;
			note = 11;
		}

		value = (octave << 4) + note + 1;
	}

	ppat->actions.BeginAction(pew, "Edit Note");
	{
		MACHINE_LOCK;

		pc->SetValue(cursor.row, value);
		InvalidateField(cursor.row, cursor.column);

		if (cursor.column < (int)ppat->columns.size() - 1 && ppat->columns[cursor.column + 1]->IsWaveParameter())
		{
			ppat->columns[cursor.column + 1]->SetValue(cursor.row, n < 0 ? WAVE_NO : pew->pCB->GetSelectedWave());
			InvalidateField(cursor.row, cursor.column + 1);
		}
	}


	if (canplay && pCB->GetPlayNotesState())
	{
		pew->pGlobalData->currentRowInBeat = 0;
		pew->pGlobalData->currentRPB = ppat->rowsPerBeat;

		ppat->PlayRow(pCB, pc, cursor.row, true, *pew->pGlobalData);
	}

	MoveCursorDelta(0, cursorStep);

	DoAnalyseChords();

}

void CPatEd::PlayRow(bool allcolumns)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	pew->pGlobalData->currentRowInBeat = 0;
	pew->pGlobalData->currentRPB = ppat->rowsPerBeat;

	if (allcolumns)
		ppat->PlayRow(pCB, cursor.row, true, NULL, 0, shared_ptr<CModulators>(), *pew->pGlobalData);
	else
		ppat->PlayRow(pCB, pc, cursor.row, true, *pew->pGlobalData);

	MoveCursorDelta(0, cursorStep);

}


void CPatEd::EditOctave(int oct)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	int value = pc->GetValue(cursor.row);
	if (value != NOTE_NO && value != NOTE_OFF)
	{
		ppat->actions.BeginAction(pew, "Edit Octave");
		{
			MACHINE_LOCK;
			pc->SetValue(cursor.row, (value & 15) | (oct << 4));
		}

		InvalidateField(cursor.row, cursor.column);
		if (pew->AutoChordExpert) pew->AnalyseChords();
	}

	MoveCursorDelta(0, cursorStep);
}

void CPatEd::EditByte(int n)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	int value = pc->GetValue(cursor.row);

	if (value == pc->GetNoValue())
		value = (n << ((cursor.digit^1)*4));
	else
		value = (value & (0x0f << (cursor.digit*4))) | (n << ((cursor.digit^1)*4));

	value = pc->Clamp(value);

	ppat->actions.BeginAction(pew, "Edit Byte");
	{
		MACHINE_LOCK;

		pc->SetValue(cursor.row, value);

		if (pc->IsWaveParameter())
		{
			if (pew->pCB->GetWave(value) != NULL)
			{
				pew->pCB->SelectWave(value);
			}
		}
	}

	InvalidateField(cursor.row, cursor.column);
	MoveCursorDelta(0, cursorStep);
}

void CPatEd::EditWord(int n)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	int value = pc->GetValue(cursor.row);

	if (value == pc->GetNoValue())
		value = (n << ((3 - cursor.digit)*4));
	else
		value = (value & (~(0x0f << ((3-cursor.digit)*4)))) | (n << ((3 - cursor.digit)*4));

	value = pc->Clamp(value);

	ppat->actions.BeginAction(pew, "Edit Word");
	{
		MACHINE_LOCK;
		pc->SetValue(cursor.row, value);
	}

	InvalidateField(cursor.row, cursor.column);
	MoveCursorDelta(0, cursorStep);
}

void CPatEd::EditSwitch(int sw)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	ppat->actions.BeginAction(pew, "Edit Switch");
	{
		MACHINE_LOCK;
		pc->SetValue(cursor.row, sw);
	}

	InvalidateField(cursor.row, cursor.column);
	MoveCursorDelta(0, cursorStep);
}

void CPatEd::EditAscii(char val)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	ppat->actions.BeginAction(pew, "Edit ASCII");
	{
		MACHINE_LOCK;
		pc->SetValue(cursor.row, val);
	}

	InvalidateField(cursor.row, cursor.column);
	MoveCursorDelta(0, cursorStep);
}

void CPatEd::Clear()
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();

	ppat->actions.BeginAction(pew, "Clear Field");
	{
		MACHINE_LOCK;

		pc->ClearValue(cursor.row);

		if (pc->GetParamType() == pt_note && cursor.column < (int)ppat->columns.size() - 1 && ppat->columns[cursor.column + 1]->IsWaveParameter())
		{
			ppat->columns[cursor.column + 1]->ClearValue(cursor.row);
			InvalidateField(cursor.row, cursor.column + 1);
		}
	}
	if (pc->GetParamType() == pt_note)
		DoAnalyseChords();

	InvalidateField(cursor.row, cursor.column);
	MoveCursorDelta(0, cursorStep);
}

void CPatEd::Insert()
{
	CMachinePattern *ppat = pew->pPattern;

	ppat->actions.BeginAction(pew, "Insert");
	{
		MACHINE_LOCK;

		int fc = ppat->GetFirstColumnOfTrackByColumn(cursor.column);
		int cc = ppat->GetGroupColumnCount(cursor.column);

		for (int c = fc; c < fc + cc; c++)
			ppat->columns[c]->Insert(cursor.row, ppat->GetRowCount());
	}

	InvalidateGroup(cursor.row, cursor.column);
	DoAnalyseChords();
}

void CPatEd::InsertRow()
{
	CMachinePattern *ppat = pew->pPattern;

	ppat->actions.BeginAction(pew, "Insert Row");
	{
		MACHINE_LOCK;

		int cc = (int)ppat->columns.size();

		for (int c = 0; c < cc; c++)
			ppat->columns[c]->Insert(cursor.row, ppat->GetRowCount());
	}

	Invalidate();
	DoAnalyseChords();
}


void CPatEd::Delete()
{
	CMachinePattern *ppat = pew->pPattern;

	ppat->actions.BeginAction(pew, "Delete");
	{
		MACHINE_LOCK;

		int fc = ppat->GetFirstColumnOfTrackByColumn(cursor.column);
		int cc = ppat->GetGroupColumnCount(cursor.column);

		for (int c = fc; c < fc + cc; c++)
			ppat->columns[c]->Delete(cursor.row);
	}

	InvalidateGroup(cursor.row, cursor.column);
	DoAnalyseChords();
}

void CPatEd::DeleteRow()
{
	CMachinePattern *ppat = pew->pPattern;

	ppat->actions.BeginAction(pew, "Delete Row");
	{
		MACHINE_LOCK;
		int cc = (int)ppat->columns.size();

		for (int c = 0; c < cc; c++)
			ppat->columns[c]->Delete(cursor.row);
	}

	Invalidate();
	DoAnalyseChords();
}

void CPatEd::ClearRow()
{
	CMachinePattern *ppat = pew->pPattern;

	ppat->actions.BeginAction(pew, "Clear Row");
	{
		MACHINE_LOCK;
		int cc = (int)ppat->columns.size();

		for (int c = 0; c < cc; c++)
			ppat->columns[c]->ClearValue(cursor.row);
	}

	Invalidate();
	DoAnalyseChords();
}

void CPatEd::Rotate(bool reverse)
{
	CMachinePattern *ppat = pew->pPattern;

	ppat->actions.BeginAction(pew, "Rotate");
	{
		MACHINE_LOCK;

		CRect r = GetSelOrAll();

		for (int c = r.left; c <= r.right; c++)
			ppat->columns[c]->Rotate(r.top, r.bottom - r.top + 1, reverse);
	}

	Invalidate();
	DoAnalyseChords();
}

void CPatEd::ToggleGraphicalMode()
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[cursor.column].get();
	
	ppat->actions.BeginAction(pew, "Toggle Graphical Mode");
	{
		pc->ToggleGraphicalMode();
	}

	pew->UpdateCanvasSize();
	pew->topwnd.Invalidate();
	Invalidate();
}


void CPatEd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	static int NoteScanCodes[] = { 44, 31, 45, 32, 46, 47, 34, 48, 35, 49, 36, 50, 16, 3, 17, 4, 18, 19, 6, 20, 7, 21, 8, 22, 23, 10, 24, 11, 25 };

	int scancode = MapVirtualKey(nChar, 0);
	if (nFlags & 256) scancode = 0;

	bool ctrldown = (GetKeyState(VK_CONTROL) & (1 << 15)) != 0;
	bool shiftdown = (GetKeyState(VK_SHIFT) & (1 << 15)) != 0;
	bool altdown = (GetKeyState(VK_MENU) & (1 << 15)) != 0;
    
	if (ctrldown && altdown)
	{
		switch(nChar)
		{
		case 'I': ImportOld(); break;
		case 'X': ExportPattern(); break; //BWC
		case 'P': ImportPattern(); break; //BWC
		case VK_SUBTRACT : {
			int inflateFactor = pew->GetComboBoxInflate();
			if (inflateFactor>=0)
				InflatePattern(-inflateFactor); 
			else	
				InflatePattern(-2); 
			break; //BWC
			}
		case VK_ADD : {
			int inflateFactor = pew->GetComboBoxInflate();
			if (inflateFactor>=0)
				InflatePattern(inflateFactor); 
			else
				InflatePattern(2); 
			break; // BWC
			}
		}
	}
	else if (ctrldown)
	{
		switch(nChar)
		{
		case 'R': if (shiftdown) Humanize(pew->DeltaHumanize, pew->HumanizeEmpty); else Randomize(); break;
		case 'I': Interpolate(shiftdown); break;
		case 'T': if (shiftdown) SelectTrack(); else WriteState(); break;
		case 'M': MuteTrack(); break;
		case 'B': OldSelect(true); break;
		case 'E': OldSelect(false); break;
		case 'U': KillSelection(); break;
		case 'W': Rotate(shiftdown); break;
		case 'G': ToggleGraphicalMode(); break;
		case 'V': if (shiftdown) OnEditPasteSpecial(); break; //BWC
		case 'F': if (shiftdown) OnAddNoteOff(); else OnClearNoteOff(); break; //BWC
		case 'J': if (shiftdown) OnUpNoteOff(); else OnDownNoteOff(); break; //BWC
		case 'A': SelectAll(); break; //BWC
		case VK_PRIOR: Home(); break; //BWC
		case VK_NEXT: End(); break; //BWC
		case VK_HOME: HomeTop(); break; //BWC
		case VK_END: EndBottom(); break; //BWC
		case 'H': if (shiftdown) pew->OnButtonDldChord(); else pew->OnButtonInsertChord(); break;
		case 'D': if (shiftdown) Mirror(); else Reverse(); break;
		case 'P': if (shiftdown) DeleteRow(); else InsertRow(); break;
		case VK_SUBTRACT: if (shiftdown) ShiftValues(-12, false); break; 
		case VK_ADD: if (shiftdown) ShiftValues(12, false); break; 
		}

		if (nChar >= '0' && nChar <= '9')
			cursorStep = nChar - '0';
	}
	else if (shiftdown)
	{
		switch(nChar)
		{
		case VK_TAB: ShiftTab(); break;
		case VK_UP: CursorSelect(0, -1); break;
		case VK_DOWN: CursorSelect(0, 1); break;
		case VK_LEFT: CursorSelect(-1, 0); break;
		case VK_RIGHT: CursorSelect(1, 0); break;
		case VK_PRIOR: CursorSelect(0, -BeatsInMeasureBar()); break;
		case VK_NEXT: CursorSelect(0, BeatsInMeasureBar()); break; 
		case VK_HOME: CursorSelect(0, -(1 << 24)); break;
		case VK_END: CursorSelect(0, (1 << 24)); break;
		case VK_SUBTRACT: ShiftValues(-1, false); break;
		case VK_ADD: ShiftValues(1, false); break;
		}

	}
	else
	{
		switch(nChar)
		{
		case VK_UP: MoveCursorDelta(0, -1); break;
		case VK_DOWN: MoveCursorDelta(0, 1); break;	
		case VK_PRIOR: MoveCursorPgUpDown(-1); break; //Use BarComboIndex to compute the delta of prior/next page
		case VK_NEXT:  MoveCursorPgUpDown(1); break;
		case VK_RIGHT: MoveCursorDelta(1, 0); break;
		case VK_LEFT: MoveCursorDelta(-1, 0); break;
		case VK_TAB: Tab(); break;
		case VK_HOME: if (pew->HomeDisabled) Home(); else HomeOld(); break;
		case VK_END: End(); break;
		case VK_OEM_PERIOD: Clear(); break;
		case VK_INSERT: Insert(); break;
		case VK_DELETE: Delete(); break;
		}
	
		CColumn *pc = ppat->columns[cursor.column].get();

		if (pc->GetParamType() == pt_note && cursor.digit == 0)
		{
			if (nChar == '1')
			{
				EditNote(-9999);
			}
			else if (nChar == '4')
			{
				PlayRow(false);
			}
			else if (nChar == '8')
			{
				PlayRow(true);
			}
			else
			{
				int i = 0;
				while(NoteScanCodes[i] != 0)
				{
					if (scancode == NoteScanCodes[i])
					{
						EditNote(i);
						break;
					}
					i++;
				}
			}
		}
		else if (pc->GetParamType() == pt_note && cursor.digit == 2)
		{
			if (nChar >= '0' && nChar <= '9')
				EditOctave(nChar - '0');
		}
		else if (pc->GetParamType() == pt_byte)
		{
			if (!pc->IsAscii())
			{
				if (nChar >= '0' && nChar <= '9')
					EditByte(nChar - '0');
				else if (nChar == 'A') EditByte(0xA);
				else if (nChar == 'B') EditByte(0xB);
				else if (nChar == 'C') EditByte(0xC);
				else if (nChar == 'D') EditByte(0xD);
				else if (nChar == 'E') EditByte(0xE);
				else if (nChar == 'F') EditByte(0xF);
			}
		}
		else if (pc->GetParamType() == pt_word)
		{
			if (nChar >= '0' && nChar <= '9')
				EditWord(nChar - '0');
			else if (nChar == 'A') EditWord(0xA);
			else if (nChar == 'B') EditWord(0xB);
			else if (nChar == 'C') EditWord(0xC);
			else if (nChar == 'D') EditWord(0xD);
			else if (nChar == 'E') EditWord(0xE);
			else if (nChar == 'F') EditWord(0xF);
			
		}
		else if (pc->GetParamType() == pt_switch)
		{
			if (nChar == '0')
				EditSwitch(SWITCH_OFF);
			else if (nChar == '1')
				EditSwitch(SWITCH_ON);
		}
	}
		

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CPatEd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CColumn *pc = ppat->columns[cursor.column].get();

	if (pc->GetParamType() == pt_byte && pc->IsAscii())
	{
		if (nChar != '.')
		{
			char ch = (char)nChar;
			bool valid = pCB->IsValidAsciiChar(pc->GetMachine(), pc->GetIndex(), ch);
			
			if (!valid && nChar >= 'a' && nChar <= 'z')
			{
				ch += 'A' - 'a';
				valid = pCB->IsValidAsciiChar(pc->GetMachine(), pc->GetIndex(), ch);
			}
			else if (!valid && nChar >= 'A' && nChar <= 'Z')
			{
				ch += 'a' - 'A';
				valid = pCB->IsValidAsciiChar(pc->GetMachine(), pc->GetIndex(), ch);
			}

			if (ch < pc->GetMinValue() || ch > pc->GetMaxValue())
				valid = false;

			if (valid)
				EditAscii(ch);
		}
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}


CCursorPos CPatEd::GetDigitAtPoint(CPoint p)
{
	CCursorPos cp;
	cp.row = GetRowY(p.y);
	cp.column = GetColumnAtX(p.x);
	cp.digit = GetDigitAtX(p.x);

	CColumn *pc = pew->pPattern->columns[cp.column].get();

	if (pc->GetParamType() == pt_note && cp.digit == 1)
		cp.digit = 0;

	return cp;
}


void CPatEd::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	if (nFlags & MK_CONTROL)
	{
		ppat->actions.BeginAction(pew, "Draw Values");
		CCursorPos cp = GetDigitAtPoint(ClientToCanvas(point));
		drawColumn = cp.column;
		drawing = true;
		SetCapture();
		Draw(point);
	}
	else
	{
		mouseSelectStartPos = GetDigitAtPoint(ClientToCanvas(point));
		mouseSelecting = true;
		MoveCursor(mouseSelectStartPos);
		SetCapture();
	}
	

	CWnd::OnLButtonDown(nFlags, point);
}

void CPatEd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (mouseSelecting)
	{
		mouseSelecting = false;
		ReleaseCapture();
	}
	else if (drawing)
	{
		drawing = false;
		ReleaseCapture();
	}


	CWnd::OnLButtonUp(nFlags, point);
}

void CPatEd::Draw(CPoint point)
{
	CMachinePattern *ppat = pew->pPattern;
	if (!drawing || ppat == NULL || ppat->columns.size() == 0)
		return;

	point = ClientToCanvas(point);
	
	CColumn *pc = ppat->columns[drawColumn].get();
	int w = (pc->GetWidth(pCB) - 1) * pew->fontSize.cx - 4;
	int x = max(0, min(w, point.x - (GetColumnX(drawColumn) + 2)));

	double v = (double)x / w;

	CCursorPos cp = GetDigitAtPoint(point);
	pc->SetValueNormalized(cp.row, v);

	InvalidateField(cp.row, drawColumn);

	if (pew->UpdateGraphicalRow) MoveCursor(cp, false);

}

void CPatEd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (mouseSelecting)
	{
		CCursorPos cp = GetDigitAtPoint(ClientToCanvas(point));
		if (cp != mouseSelectStartPos)
		{
			persistentSelection = false;
			selection = true;
			selStart.x = mouseSelectStartPos.column;
			selStart.y = mouseSelectStartPos.row;
			selEnd.x = cp.column;
			selEnd.y = cp.row;
			pew->OnUpdateSelection();
			// TODO: invalidate less
			Invalidate();
		}
	}
	else if (drawing)
	{
		Draw(point);
	}
	
	CWnd::OnMouseMove(nFlags, point);
}


BOOL CPatEd::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{

	return CScrollWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


BOOL CPatEd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (pew->pPattern == NULL || pew->pPattern->columns.size() == 0)
		return CScrollWnd::OnMouseWheel(nFlags, zDelta, pt);

	mouseWheelAcc += zDelta;

	while (mouseWheelAcc > 120)
	{
		mouseWheelAcc -= 120;
		MoveCursorDelta(0, -1);
	}

	while (mouseWheelAcc < 120)
	{
		mouseWheelAcc += 120;
		MoveCursorDelta(0, 1);
	}

	return CScrollWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CPatEd::UpdateStatusBar()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;
	
	CColumn *pc = ppat->columns[cursor.column].get();
	CString s;

	if (pc->IsTrackParam())
		s.Format("%s, Tr. %d", pc->GetMachineName(), pc->GetTrack());
	else
		s = pc->GetMachineName();

	pew->pCB->SetPatternEditorStatusText(0, s);


	int value = pc->GetValue(cursor.row);

	if (value != pc->GetNoValue())
	{
		s = FieldToLongText(pc->GetParamType(), pc->HasValue(cursor.row), value);

		char const *desc = pc->DescribeValue(value, pCB);

		if (value != pc->GetNoValue() && desc != NULL && strlen(desc) > 0)
			s += (CString)" " + desc;
	}
	else
	{
		s = "";
	}

	pew->pCB->SetPatternEditorStatusText(1, s);
	pew->pCB->SetPatternEditorStatusText(2, pc->GetDescription());
}


CRect CPatEd::GetSelRect()
{
	if (!selection)
		return CRect(0, 0, 0, 0);

	CRect r;
	r.left = min(selStart.x, selEnd.x);
	r.top = min(selStart.y, selEnd.y);
	r.right = r.left + abs(selStart.x - selEnd.x);
	r.bottom = r.top + abs(selStart.y - selEnd.y);

	return r;
}

CRect CPatEd::GetSelOrCursorRect()
{
	CRect r;

	if (!selection)
	{
		r.left = r.right = cursor.column;
		r.top = r.bottom = cursor.row;
		return r;
	}

	r.left = min(selStart.x, selEnd.x);
	r.top = min(selStart.y, selEnd.y);
	r.right = r.left + abs(selStart.x - selEnd.x);
	r.bottom = r.top + abs(selStart.y - selEnd.y);

	return r;
}

CRect CPatEd::GetSelOrAll()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return CRect(0, 0, 0, 0);

	CRect r;

	if (!selection)
	{
		r.left = 0;
		r.right = (int)ppat->columns.size() - 1;
		r.top = 0;
		r.bottom = ppat->GetRowCount() - 1;
		return r;
	}

	r.left = min(selStart.x, selEnd.x);
	r.top = min(selStart.y, selEnd.y);
	r.right = r.left + abs(selStart.x - selEnd.x);
	r.bottom = r.top + abs(selStart.y - selEnd.y);

	return r;
}

void CPatEd::KillSelection()
{
	if (selection)
	{
		selection = false;
		pew->OnUpdateSelection();
		Invalidate();
	}
}

void CPatEd::CursorSelect(int dx, int dy)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CColumn *pc = ppat->columns[cursor.column].get();

	if (!selection)
	{
		selStart.x = cursor.column;
		selStart.y = cursor.row;
		selection = true;
		pew->OnUpdateSelection();
	}

	CCursorPos oldpos = cursor;

	CCursorPos cp;
	cp.column = min(max(0, cursor.column + dx), (int)ppat->columns.size() - 1);
	cp.row = min(max(0, cursor.row + dy), ppat->GetRowCount() - 1);
	cp.digit = 0;
	MoveCursor(cp, false);

	selEnd.x = cursor.column;
	selEnd.y = cursor.row;

	if (cursor == oldpos)
	{
		switch(selMode)
		{
		case column: selMode = track; break;
		case track: if (pc->IsTrackParam()) selMode = group; else selMode = all; break;
		case group: selMode = all; break;
		case all: selMode = column; break;
		}

		switch(selMode)
		{
		case column:
			selStart.x = selEnd.x = cursor.column;
			break;
		case track:
			selStart.x = ppat->GetFirstColumnOfTrackByColumn(cursor.column);
			selEnd.x = selStart.x + ppat->GetGroupColumnCount(cursor.column) - 1;
			break;
		case group:
			selStart.x = ppat->GetFirstColumnOfTrackByColumn(cursor.column) - pc->GetTrack() * ppat->GetGroupColumnCount(cursor.column);
			selEnd.x = selStart.x + ppat->GetGroupColumnCount(cursor.column) * ppat->GetTrackCount(pc->GetMachine()) - 1;
			break;
		case all:
			selStart.x = 0;
			selEnd.x = (int)ppat->columns.size() - 1;
			break;
		}

	}

	persistentSelection = false;
	Invalidate();
}

void CPatEd::SelectAll()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r;

	selStart.x = 0;
	selEnd.x = (int)ppat->columns.size() - 1;
	selStart.y = 0;
	selEnd.y = ppat->GetRowCount() - 1;

	selection = true;
	pew->OnUpdateSelection();
	persistentSelection = false;
	Invalidate();
}

void CPatEd::SelectTrack()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	selStart.x = ppat->GetFirstColumnOfTrackByColumn(cursor.column);
	selStart.y = 0;

	selEnd.x = selStart.x + ppat->GetGroupColumnCount(cursor.column) - 1;
	selEnd.y = ppat->GetRowCount() - 1;

	selection = true;
	pew->OnUpdateSelection();
	persistentSelection = false;
	Invalidate();
}

//int CPatEd::

void CPatEd::SelectMesure(int r)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	if (r < 0) r = cursor.row;

	int mbegin;
	int mend;
	int bim = BeatsInMeasureBar();
	if (bim <= 0) bim = 1;

	mbegin = (r / bim) * bim;
	mend = mbegin + bim - 1;

	selStart.x = 0;
	selStart.y = mbegin;

	selEnd.x = (int)ppat->columns.size() - 1;
	selEnd.y = mend;

	selection = true;
	pew->OnUpdateSelection();
	persistentSelection = false;
	Invalidate();
}

void CPatEd::SelectBeat(int r)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	if (r < 0) r = cursor.row;

	int mbegin;
	int mend;
	int bim = ppat->rowsPerBeat;
	if (bim <= 0) bim = 1;

	mbegin = (r / bim) * bim;
	mend = mbegin + bim - 1;

	selStart.x = 0;
	selStart.y = mbegin;

	selEnd.x = (int)ppat->columns.size() - 1;
	selEnd.y = mend;

	selection = true;
	pew->OnUpdateSelection();
	persistentSelection = false;
	Invalidate();
}

void CPatEd::SelectRow(int r)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	if (r < 0) r = cursor.row;

	selStart.x = 0;
	selStart.y = r;

	selEnd.x = (int)ppat->columns.size() - 1;
	selEnd.y = r;

	selection = true;
	pew->OnUpdateSelection();
	persistentSelection = false;
	Invalidate();
}

void CPatEd::SelectTrackByNo(int col)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	selStart.x = ppat->GetFirstColumnOfTrackByColumn(col);
	selStart.y = 0;

	selEnd.x = selStart.x + ppat->GetGroupColumnCount(col) - 1;
	selEnd.y = ppat->GetRowCount() - 1;

	selection = true;
	pew->OnUpdateSelection();
	persistentSelection = false;
	Invalidate();
}

void CPatEd::OldSelect(bool start)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CColumn *pc = ppat->columns[cursor.column].get();

	bool nochange = false;

	if (start)
	{
		if (selStart.y == cursor.row)
			nochange = true;

		selStart.y = cursor.row;

		if (!selection)
		{
			selEnd = selStart;
			selection = true;
		}
	}
	else
	{
		if (selEnd.y == cursor.row)
			nochange = true;

		selEnd.y = cursor.row;

		if (!selection)
		{
			selStart = selEnd;
			selection = true;
		}

	}

	if (nochange)
	{
		switch(selMode)
		{
		case column: selMode = track; break;
		case track: if (pc->IsTrackParam()) selMode = group; else selMode = all; break;
		case group: selMode = all; break;
		case all: selMode = column; break;
		}
	}

	switch(selMode)
	{
	case column:
		selStart.x = selEnd.x = cursor.column;
		break;
	case track:
		selStart.x = ppat->GetFirstColumnOfTrackByColumn(cursor.column);
		selEnd.x = selStart.x + ppat->GetGroupColumnCount(cursor.column) - 1;
		break;
	case group:
		selStart.x = ppat->GetFirstColumnOfTrackByColumn(cursor.column) - pc->GetTrack() * ppat->GetGroupColumnCount(cursor.column);
		selEnd.x = selStart.x + ppat->GetGroupColumnCount(cursor.column) * ppat->GetTrackCount(pc->GetMachine()) - 1;
		break;
	case all:
		selStart.x = 0;
		selEnd.x = (int)ppat->columns.size() - 1;
		break;
	}

	persistentSelection = true;
	pew->OnUpdateSelection();
	Invalidate();
}


bool CPatEd::InSelection(int row, int column)
{
	if (!selection)
		return false;

	CRect r = GetSelRect();
	return (row >= r.top && row <= r.bottom && column >= r.left && column <= r.right);
}

bool CPatEd::CanCut() { return selection; }
bool CPatEd::CanCopy() { return selection; }
bool CPatEd::CanPaste() { return pew->pPattern != NULL && clipboard.size() > 0; }


void CPatEd::OnEditCut()
{
	if (!selection)
		return;

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelRect();
	clipboard.clear();
	clipboardRowCount = (r.bottom - r.top) + 1;
	clipboardPersistentSelection = persistentSelection;
	clipboardSelMode = selMode;

	ppat->actions.BeginAction(pew, "Cut");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
		{
			clipboard.push_back(MapIntToInt());
			ppat->columns[col]->GetEventRange(clipboard.back(), r.top, r.bottom);
			ppat->columns[col]->ClearEventRange(r.top, r.bottom);
		}
	}

	pew->OnUpdateClipboard();
	Invalidate();
	DoAnalyseChords();

}

void CPatEd::OnEditCopy()
{
	if (!selection)
		return;

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelRect();
	clipboard.clear();
	clipboardRowCount = (r.bottom - r.top) + 1;
	clipboardPersistentSelection = persistentSelection;
	clipboardSelMode = selMode;

	for (int col = r.left; col <= r.right; col++)
	{
		clipboard.push_back(MapIntToInt());
		ppat->columns[col]->GetEventRange(clipboard.back(), r.top, r.bottom);
	}
	
	pew->OnUpdateClipboard();
}

void CPatEd::OnEditPaste()
{
	if (clipboard.size() == 0)
		return;

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	int col = cursor.column;
	if (clipboardPersistentSelection)
	{
		if (clipboardSelMode == track)
			col = ppat->GetFirstColumnOfTrackByColumn(col);
		else if (clipboardSelMode == group)
			col = ppat->GetFirstColumnOfGroupByColumn(col);
		else if (clipboardSelMode == all)
			col = 0;
	}

	ppat->actions.BeginAction(pew, "Paste");
	{
		MACHINE_LOCK;

		for (MapIntToIntVector::iterator i = clipboard.begin(); i != clipboard.end(); i++, col++)
		{
			if (col >= (int)ppat->columns.size())
				break;

			ppat->columns[col]->SetEventRange(*i, cursor.row, cursor.row + clipboardRowCount - 1, ppat->GetRowCount());

		}
	}

	Invalidate();
	DoAnalyseChords();

}

void CPatEd::OnEditPasteSpecial()
{
	// Merge clipboard content with destination
	if (clipboard.size() == 0)
		return;

	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	int col = cursor.column;
	if (clipboardPersistentSelection)
	{
		if (clipboardSelMode == track)
			col = ppat->GetFirstColumnOfTrackByColumn(col);
		else if (clipboardSelMode == group)
			col = ppat->GetFirstColumnOfGroupByColumn(col);
		else if (clipboardSelMode == all)
			col = 0;
	}

	ppat->actions.BeginAction(pew, "Paste merge");
	{
		MACHINE_LOCK;

		for (MapIntToIntVector::iterator i = clipboard.begin(); i != clipboard.end(); i++, col++)
		{
			if (col >= (int)ppat->columns.size())
				break;
			
			ppat->columns[col]->SetEventRangeSpecial(*i, cursor.row, cursor.row + clipboardRowCount - 1, ppat->GetRowCount());

		}
	}

	Invalidate();
	DoAnalyseChords();

}

void CPatEd::OnAddNoteOff()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Add note off");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++){
			CColumn *pc = ppat->columns[col].get();
			if (pc->GetParamType()==pt_note)
				for (int row = r.top; row < r.bottom; row++)  // Last row is ignored (don't add after selection)
				{
					int val= pc->GetValue(row);
					if ((val != pc->GetNoValue())&&(val != NOTE_OFF)) {
						// Current row is a note
						val= pc->GetValue(row+1);
						if (val == pc->GetNoValue())
							// Next row is empty, add a note off
							pc->SetValue(row+1, NOTE_OFF);
					}
				}
		}
		
	}

	Invalidate();
}

void CPatEd::OnUpNoteOff()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Up note off");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++){
			CColumn *pc = ppat->columns[col].get();
			if (pc->GetParamType()==pt_note)
			{  
				int minDelta=INT_MAX;
				int maxDelta=0;
				int delta=-1;
				// First, try to guess how far the note off are from the notes
				for (int row = r.top; row <= r.bottom; row++)
				{
					int val= pc->GetValue(row);

					if (val != pc->GetNoValue()){
						if (val != NOTE_OFF){
							delta=0;
						}
						else {
							delta++;
							if (delta<minDelta) minDelta=delta;
							if (delta>maxDelta) maxDelta=delta;
							delta=-1; // Nothing to do until a new note
						}
					}
					else
					{
						if (delta>=0) delta++;
					}
				}

				// Assume minDelta is the regular distance between note and note off
				if (minDelta < r.bottom-r.top)
				{	
					int prevval = pc->GetNoValue();
					for (int row = r.top; row <= r.bottom; row++)
					{					
						int val= pc->GetValue(row);

						if (val != pc->GetNoValue()){
							if (val == NOTE_OFF) {
								// Current row is a note off
								// Clear it
								pc->ClearValue(row);
								if (row > r.top) { // Don't write outside the selection
									if (prevval == pc->GetNoValue()) {
										// Prev row is empty, set the note off there
										pc->SetValue(row-1, NOTE_OFF);
									}
								}
							}
							else
							{	// It's a note, could be a hidden note off
								if (row > r.top) 
								{	// Don't write outside the selection
									// Check if there is a note at minDelta distance
									if (row-minDelta >0)
									{
										int deltaval = pc->GetValue(row-minDelta);
										if ((deltaval != pc->GetNoValue()) && (deltaval != NOTE_OFF))
										{
											// A note at minDelta of the current note : can be a hidden note off
											if (prevval == pc->GetNoValue()) {
												// Prev row is empty, set the note off there
												pc->SetValue(row-1, NOTE_OFF);
											}
										}
									}
								}
							}

						}
						prevval = val;
					}
				}

			}
		}
		
	}

	Invalidate();
}

void CPatEd::OnDownNoteOff()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Down note off");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++){
			CColumn *pc = ppat->columns[col].get();
			if (pc->GetParamType()==pt_note)
				for (int row = r.top; row <= r.bottom; row++)
				{
					int val= pc->GetValue(row);
					if ((val != pc->GetNoValue())&&(val == NOTE_OFF)) {
						// Current row is a note off
						// Clear it
						pc->ClearValue(row);
						if (row < r.bottom) { // Don't write outside the selection
							val= pc->GetValue(row+1);
							if (val == pc->GetNoValue()) {
								// Next row is empty, set the note off there
								pc->SetValue(row+1, NOTE_OFF);
								row++; // Skip it now
							}
						}
					}
				}
		}
		
	}

	Invalidate();
}

void CPatEd::OnClearNoteOff()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Clear off");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
			for (int row = r.top; row <= r.bottom; row++)
			{
				ppat->columns[col]->ClearNoteOff(row);
			}
		
	}

	Invalidate();
}

inline double frand()
{
	static long stat = GetTickCount();
	stat = (stat * 1103515245 + 12345) & 0x7fffffff;
	return (double)stat * (1.0 / 0x7fffffff);
}

void CPatEd::Humanize(int delta, bool hEmpty)
// delta : from 0 to 100
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	if (delta <0) delta = 0;
	if (delta >100) delta = 100;

	ppat->actions.BeginAction(pew, "Humanize");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
		{
			CColumn *pc = ppat->columns[col].get();
			// Do nothing on notes or switch columns
			CMPType curtype = pc->GetParamType();

			if ((curtype!=pt_note) && (curtype!=pt_switch))
			{
				int curvalue=-1;
				int minvalue= pc->GetMinValue();
				int maxvalue= pc->GetMaxValue();
				int hdelta = (delta * (maxvalue-minvalue)) /100;

				// array to read the value of the row
				MapIntToValue::const_iterator ei = pc->EventsBegin();
				while(ei != pc->EventsEnd() && (*ei).first < r.top) ei++;
				
				for (int row = r.top; row <= r.bottom; row++)
				{
					
					// Get current value of the row
					int data = pc->GetNoValue();
					bool hasvalue = false;

					if (ei != pc->EventsEnd() && (*ei).first == row)
					{
						hasvalue = true;
						data = (*ei).second;
						ei++;
					}
					if (hasvalue) 
						curvalue = data;
					else
					{
						if (!hEmpty) curvalue = -1;
					}

					// if curvalue is not initialised, do nothing
					if (curvalue >=0)
					{
						// Use curvalue to set the humanized value of the row
						// use hdelta to have the min, max possible values around curvalue
						int hmin= curvalue - (hdelta/2);
						// randomly select a value between min and max
						int hvalue = hmin + (rand() % hdelta); 
						if (hvalue<minvalue) hvalue = minvalue;
						if (hvalue>maxvalue) hvalue = maxvalue;
						pc->SetValue(row, hvalue);
					}
				}
			}
		}

	}

	Invalidate();
}

void CPatEd::Randomize()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Randomize");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
			for (int row = r.top; row <= r.bottom; row++)
				ppat->columns[col]->SetValueNormalized(row, frand());

	}

	Invalidate();

}

inline double ExpIntp(double const x, double const y, double const a)
{
	assert((x > 0 && y > 0) || (x < 0 && y < 0));
	double const lx = log(x);
	double const ly = log(y);
	return exp(lx + a * (ly - lx));
}

inline double RootIntp(double const a, int param, int ipMax)
{
	double factor = 1.0/4.0;
	if (param > 1) {
		factor = (((1.0-(1.0/4.0)) / (ipMax+1)) * param) + (1.0/4.0);
	}

	return pow(a, factor);
}

inline double PowerIntp(double const a, int param, int ipMax)
{
	double factor = 4.0;
	if (param > 1) {
		factor = 4.0 - ((3.0*param) / (ipMax+1));
	}

	return pow(a, factor);
}

inline double Interpolate(double a, double v1, double v2, bool expintp, int InterpolateParam, int ipMax)
{
	if (expintp) {
		if ((v1 > 0 && v2 > 0) || (v1 < 0 && v2 < 0))
			return (ExpIntp(v1, v2, a) - v1) / (v2 -v1);
		else 
			return a;
	}
	else
	{
		if (InterpolateParam == 0)
			return a;
		else if (InterpolateParam < 0) 
		{
			if (v1<v2)
				return (v2*RootIntp(a, -InterpolateParam, ipMax) - v1) / (v2 -v1);
			else
				return (v1*RootIntp(a, -InterpolateParam, ipMax) - v2) / (v1 -v2);
		}
		else // if (InterpolateParam > 0) 
		{
			if (v1<v2)
				return (v2*PowerIntp(a, InterpolateParam, ipMax) - v1) / (v2 -v1);
			else
				return (v1*PowerIntp(a, InterpolateParam, ipMax) - v2) / (v1 -v2);
		}
			
	}
}

void CPatEd::Interpolate(bool expintp)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelRect();

	int InterpolateParam;
	if (expintp) 
		InterpolateParam=0;
	else
	{
		InterpolateParam = pew->InterpolateComboIndex();
		if (InterpolateParam == pew->LINEAR_INTERPOLATE_PARAM) InterpolateParam = 0;
		else if (InterpolateParam < pew->LINEAR_INTERPOLATE_PARAM) InterpolateParam = -InterpolateParam -1;
		else if (InterpolateParam > pew->LINEAR_INTERPOLATE_PARAM) InterpolateParam = (2*pew->LINEAR_INTERPOLATE_PARAM - InterpolateParam) +1;
	}
	ppat->actions.BeginAction(pew, "Interpolate");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
		{
			int v1 = ppat->columns[col]->GetValue(r.top);
			int v2 = ppat->columns[col]->GetValue(r.bottom);
			if (ppat->columns[col]->IsNormalValue(v1) && ppat->columns[col]->IsNormalValue(v2))
			{
				for (int row = r.top + 1; row < r.bottom; row++)
				{
					if (v1 < v2)
						ppat->columns[col]->SetValueNormalized(row, ::Interpolate((double)(row - r.top) / (r.bottom - r.top), v1, v2, expintp, InterpolateParam, pew->LINEAR_INTERPOLATE_PARAM), v1, v2);
					else
						ppat->columns[col]->SetValueNormalized(row, ::Interpolate(1.0 - (double)(row - r.top) / (r.bottom - r.top), v1, v2, expintp, InterpolateParam, pew->LINEAR_INTERPOLATE_PARAM), v2, v1);
				}
			}
			
		}
	}

	Invalidate();

}


void CPatEd::Reverse()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelRect();

	ppat->actions.BeginAction(pew, "Reverse");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
		{
			CColumn *pc = ppat->columns[col].get();

			MapIntToValue colbuf;
			MapIntToValue::const_iterator ei;

			// First, copy the data of the column to colbuf
			// row y goes to r.bottom - (y - r.top)
			for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
			{					
				int y = (*ei).first;
				int data = (*ei).second;
				if ((y>=r.top) && (y<=r.bottom))
					colbuf[r.bottom - (y - r.top)] = data;	
				if (y>r.bottom) break;
			}

			// Then clear the datas between top and bottom
			pc->ClearEventRange(r.top, r.bottom);

			// Insert the saved datas at the reversed place
			for (ei = colbuf.begin(); ei != colbuf.end(); ei++)
			{					
				int y = (*ei).first;
				int data = (*ei).second;
				pc->SetValue(y, data);					
			}
		}
	}

	Invalidate();
	DoAnalyseChords();
}

double rounddouble(double d)
{
  return floor(d + 0.5);
}

int ModuloNote(int note, int &octave)
{
	if (note >= 12) {
		octave = octave+1;
		return (note - 12);
	}
	else if (note < 0) {
		octave = octave-1;
		return(note + 12);
	}
	else return note;
}

void CPatEd::Mirror()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelRect();

	ppat->actions.BeginAction(pew, "Mirror");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
		{
			CColumn *pc = ppat->columns[col].get();

			MapIntToValue::const_iterator ei;

			switch(pc->GetParamType())
			{
			case pt_note:
				{
				// Mirror notes is relative to first note of the selection
				byte ZeroVal=0;
				bool checkTonality = false;
	
				for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
				{					
					int y = (*ei).first;
					int data = (*ei).second;
					if (y>r.bottom) break;
					if ((y>=r.top) && (y<=r.bottom))
					{
						if (data != NOTE_OFF) 
						{
							ZeroVal = (byte)data;
							int octave = ZeroVal >> 4;
							int note;
							// If tonal, use base note as Zero
							if (pew->TonalComboIndex >0 && pew->TonalComboIndex <= (int)pew->TonalityList.size()){
								checkTonality = true;
								note = pew->TonalityList[pew->TonalComboIndex].base_note;
							}
							else
								note = (ZeroVal & 15) -1;
							ZeroVal = 12*octave + note;
							break;
						}
					}
				}
				if (ZeroVal>0)
				{ 
					// Then mirror each value according to ZeroVal
					for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
					{					
						int y = (*ei).first;
						byte data = (byte)(*ei).second;
						if (data != NOTE_OFF) 
						{
							int octave = data >> 4;
							int note = (data & 15) -1;
							bool inTonality;
							if (checkTonality) inTonality = CheckNoteInTonality(note);

							data = 12*octave + note;
							byte NewData = ZeroVal - (data-ZeroVal);
							octave = NewData / 12;
							note = NewData - (octave*12);
					//		bool MirrorUp = (data-ZeroVal<0);

							if (checkTonality) 
							{ // Use tonality
								int deltamirror;
								if (IsMajorTonality()) deltamirror=1; else deltamirror=-1;

								if (inTonality){
									// Note to mirror was in tonality
									if (!CheckNoteInTonality(note)) {
										// Mirror is not
										note = ModuloNote(note + deltamirror, octave);

									}
								}
								else
								{
									// Note to mirror was not in tonality
									if (CheckNoteInTonality(note)) {
										// Mirror is in tonality ... shift it
										note = ModuloNote(note + deltamirror, octave);
									}
									// Check it twice ... is enough
									if (CheckNoteInTonality(note)) {
										// Mirror is in tonality... shift it
										note = ModuloNote(note + deltamirror, octave);
									}
								}

							}
					
							if (y>r.bottom) break;
							if ((y>=r.top) && (y<=r.bottom))
								pc->SetValue(y, (octave << 4) + note + 1);
						}
					}
				}

				}
				break;
			case pt_byte:
			case pt_word:
				{
				int MinVal=MAXINT;
				int MaxVal=-1;
				double ZeroVal=0;
				// First, get the min and max values of the selection
				for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
				{					
					int y = (*ei).first;
					int data = (*ei).second;
					if (y>r.bottom) break;
					if ((y>=r.top) && (y<=r.bottom))
					{
						if (data > MaxVal) MaxVal = data;
						if (data < MinVal) MinVal = data;
					}
				}

				if (MaxVal>0)
				{ 
					ZeroVal = MinVal + ((double)(MaxVal-MinVal)/2);
					// Then mirror each value according to ZeroVal
					for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
					{					
						int y = (*ei).first;
						int data = (*ei).second;
						if (y>r.bottom) break;
						if ((y>=r.top) && (y<=r.bottom))
							pc->SetValue(y, rounddouble(ZeroVal - (data - ZeroVal)));
					}
				}
				}
				break;
			case pt_switch:
				{   // Invert the data
					for (ei = pc->EventsBegin(); ei != pc->EventsEnd(); ei++)
					{					
						int y = (*ei).first;
						int data = (*ei).second;
						if (y>r.bottom) break;
						if ((y>=r.top) && (y<=r.bottom))
							pc->SetValue(y, !data);					
					}
				}
				break;
			}

		}
	}

	Invalidate();
	DoAnalyseChords();
}

int CPatEd::TestTranspose(int MinValue, int MaxValue, int delta, int *count_min, int *count_max)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return 0;

	CRect r = GetSelOrCursorRect();

	*count_min = 0;
	*count_max = 0;
	int count = 0;

	for (int col = r.left; col <= r.right; col++) {
		CColumn *pc = ppat->columns[col].get();
		if (pc->GetParamType() == pt_note) {
			for (int row = r.top; row <= r.bottom; row++)
			{
				int val = pc->GetValue(row);
				// If it's a note
				if ((val != pc->GetNoValue()) && (val != NOTE_OFF)) {
					count++;
					val = val + delta;
					if (val < MinValue) *count_min = *count_min +1;
					if (val > MaxValue) *count_max = *count_max +1;
				}
			}
		}
	}
	return count;
}

void CPatEd::Transpose(int MinValue, int MaxValue)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	// Analyse first
	int count_min = 0;
	int count_max = 0;
	int count = TestTranspose(MinValue, MaxValue, 0, &count_min, &count_max);
	if (count == 0) return; // Nothing to do

	int delta_transpose = 0;

	if ((count_min > 0) && (count_max > 0)) {
		// Limit the note on both sides
	}
	else if (count_min > 0) {
		// Limit lower notes
		// Try to transpose up
		int count_min_d = 0;
		int count_max_d = 0;
		int delta = 16;
		delta_transpose++;
		int prev_count_min = count_min;
		count = TestTranspose(MinValue, MaxValue, delta, &count_min_d, &count_max_d);
		while ((count_max_d == 0) && (count_min_d > 0)) {
			delta = delta + 16;
			delta_transpose++;
			prev_count_min = count_min_d;
			count = TestTranspose(MinValue, MaxValue, delta, &count_min_d, &count_max_d);
		}
		if (prev_count_min < (count_min_d + count_max_d)) {
			// Was better before
			delta_transpose--;
		}
			
	}
	else if (count_max > 0) {
		// Limit upper notes
		// Try to transpose down
		int count_min_d = 0;
		int count_max_d = 0;
		int delta = -16;
		delta_transpose--;
		int prev_count_max = count_max;
		count = TestTranspose(MinValue, MaxValue, delta, &count_min_d, &count_max_d);
		while ((count_min_d == 0) && (count_max_d > 0)) {
			delta = delta - 16;
			delta_transpose--;
			prev_count_max = count_max_d;
			count = TestTranspose(MinValue, MaxValue, delta, &count_min_d, &count_max_d);
		}
		if (prev_count_max < (count_min_d + count_max_d)) {
			// Was better before
			delta_transpose++;
		}
	}
	else return; // Nothing to do


	ppat->actions.BeginAction(pew, "Transpose Values");
	{
		MACHINE_LOCK;
		for (int col = r.left; col <= r.right; col++)
		{
			CColumn *pc = ppat->columns[col].get();
			if (pc->GetParamType() == pt_note) {
				for (int row = r.top; row <= r.bottom; row++)
				{
					int val = pc->GetValue(row);
					// If it's a note
					if ((val != pc->GetNoValue()) && (val != NOTE_OFF)) {
						// First shift according to delta_transpose
						if (delta_transpose != 0) {
							ppat->columns[col]->ShiftValue(row, 12 * delta_transpose);
							val = pc->GetValue(row);
						}
							
						// Shift if out of limits
						while (val < MinValue) {
						  ppat->columns[col]->ShiftValue(row, 12);
						  val = pc->GetValue(row);
						}
						while (val > MaxValue) {
							ppat->columns[col]->ShiftValue(row, -12);
							val = pc->GetValue(row);
						}
					}
				}
			}

		}


	}

	Invalidate();
	DoAnalyseChords();
}
	

void CPatEd::ShiftValues(int delta, bool OnlyNotes)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Shift Values");
	{
		MACHINE_LOCK;

		int delta1;
		if (delta>0) delta1=1; else delta1=-1;

		for (int col = r.left; col <= r.right; col++)
		{
			CColumn *pc = ppat->columns[col].get();
			if (pc->GetParamType() == pt_note || !OnlyNotes) {
				// if Shift <12 and Pattern is tonal and column is note
				if ((delta!=12 && delta!=-12) && (pew->TonalComboIndex >0) && (pc->GetParamType()==pt_note))
				{
					for (int row = r.top; row <= r.bottom; row++) 
					{
						int val= pc->GetValue(row);
						// If it's a note
						if ((val != pc->GetNoValue()) && (val != NOTE_OFF)) {
							// if note in tonality
							if (CheckNoteInTonality((val & 15) -1)) 
							{
								ppat->columns[col]->ShiftValue(row, delta);
								val= pc->GetValue(row);
								// Stay in tonality
								if (!CheckNoteInTonality((val & 15) -1)) 
									ppat->columns[col]->ShiftValue(row, delta1);
							}
							else
							{
								ppat->columns[col]->ShiftValue(row, delta);
								val= pc->GetValue(row);
								// Stay out of tonality
								if (CheckNoteInTonality((val & 15) -1)) 
									ppat->columns[col]->ShiftValue(row, delta1);
								val= pc->GetValue(row);
								if (CheckNoteInTonality((val & 15) -1)) 
									ppat->columns[col]->ShiftValue(row, delta1);
							}
						}

					}
				}
				else
				for (int row = r.top; row <= r.bottom; row++)
					ppat->columns[col]->ShiftValue(row, delta);
			}
		}

	}

	Invalidate();
	DoAnalyseChords();

}

void CPatEd::WriteState()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Write State");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
			for (int row = r.top; row <= r.bottom; row++)
				ppat->columns[col]->WriteState(pCB, row);

	}

	Invalidate();
}

void CPatEd::MuteTrack()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CColumn *pc = ppat->columns[cursor.column].get();
	if (pc->IsTrackParam())
		ppat->ToggleTrackMute(pc);

	Invalidate();
	pew->topwnd.Invalidate();
}

void CPatEd::OnTimer(UINT_PTR nIDEvent)
{
	if (!IsWindow(GetSafeHwnd()))
		return;

	CRect r = GetCanvasRect();

	if (drawPlayPos >= 0 && drawPlayPos != playPos)
	{
		r.top = drawPlayPos;
		r.bottom = r.top + 1;
		if (!pew->PersistentPlayPos)
			drawPlayPos = -1;
		InvalidateRect(CanvasToClient(r));
	}

	if (pPlayingPattern == pew->pPattern)
	{
		drawPlayPos = playPos;		// playPos is written to in another thread so read it only once
		r.top = playPos;
		r.bottom = r.top + 1;
		InvalidateRect(CanvasToClient(r));
	}

	if (invalidateInTimer)
	{
		invalidateInTimer = false;
		Invalidate();
	}

	CheckRefreshChords();

	CScrollWnd::OnTimer(nIDEvent);

}

int CPatEd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetTimer(1, 50, NULL);

	return 0;
}

LRESULT CPatEd::OnMidiNote(WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int vel = (int)lParam;
	int n = (int)wParam - pew->pCB->GetBaseOctave() * 12;

	if (vel > 0)
		EditNote(n, false);

	return 0;
}

void CPatEd::ImportOld()
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	ppat->actions.BeginAction(pew, "Import Old Pattern");
	{
		MACHINE_LOCK;
		ppat->Import(pew->pCB);
	}

	Invalidate();
	DoAnalyseChords();

}