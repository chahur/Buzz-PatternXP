// PatEd.cpp : implementation file
//

#include "stdafx.h"
#include "App.h"
#include "PatEd.h"
#include "EditorWnd.h"
#include <iostream>
#include <string>
#include <fstream>



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
}

CPatEd::~CPatEd()
{
}


BEGIN_MESSAGE_MAP(CPatEd, CScrollWnd)
	ON_WM_KEYDOWN()
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
static char const NoteToText[] = "C-C#D-D#E-F-F#G-G#A-A#B-";
static char const NibbleToHexText[] = "0123456789ABCDEF";

static void FieldToText(char *txt, CMPType type, bool hasval, void *fdata)
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
	else if ((c>='A') && (c<='F')) res = (10 + c -'A');
	else if ((c>='a') && (c<='f')) res = (10 + c -'a');
	
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
//	pCB->WriteLine(debugtxt);

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


void CPatEd::ExportPattern()
{
	// Get filename
	TCHAR szFilters[]= _T("Pattern file (*.csv)|*.csv|All Files (*.*)|*.*||");

	CFileDialog dlgFile(TRUE, _T("csv"), _T("*.csv"), OFN_HIDEREADONLY, szFilters);
	dlgFile.m_ofn.lpstrTitle = "Export pattern";

	if (dlgFile.DoModal() != IDOK)
		return;
	
	CString pathName = dlgFile.GetPathName();
	
	CMachinePattern *ppat = pew->pPattern;
	int const firstrow = 0;
	int const lastrow = pew->pPattern->GetRowCount();

	ofstream expfile (pathName, ios::out | ios::trunc);  
	if (expfile)  
    {
		// Export the pattern in a csv format
		// Fiste line : column header		
		for (int col = 0; col < (int)ppat->columns.size(); col++)
		{
			CColumn *pc = ppat->columns[col].get();
			if (col>0)
			  expfile << ";" << pc->GetShortDescription();
			else
			  expfile << pc->GetShortDescription();
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
	// Get filename
	TCHAR szFilters[]= _T("Pattern file (*.csv)|*.csv|All Files (*.*)|*.*||");

	CFileDialog dlgFile(TRUE, _T("csv"), _T("*.csv"), OFN_HIDEREADONLY, szFilters);
	dlgFile.m_ofn.lpstrTitle = "Import pattern";
	if (dlgFile.DoModal() != IDOK)
		return;
	
	CString pathName = dlgFile.GetPathName();
	
	CMachinePattern *ppat = pew->pPattern;
	
	int const lastrow = pew->pPattern->GetRowCount();
	int const lastcol = (int)ppat->columns.size();

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
			if (irow > lastrow) 
				break;
        }
		
		expfile.close();  
    }
	
	pCB->SetModifiedFlag();
	Invalidate();
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
			DrawField(pDC, col, pnc, data, x, y, muted, hasvalue);
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
	if (pew->BarComboIndex==0) // Automatic mode
	{ 
		return (4 * ppat->rowsPerBeat);
	}
	else
		return (pew->BarComboIndex * ppat->rowsPerBeat);
}

COLORREF CPatEd::GetFieldBackgroundColor(CMachinePattern *ppat, int row, int col, bool muted)
{
	COLORREF color;
	// BWC pew.BarComboIndex
	if (InSelection(row, col))
	{
		color = bgsel;
	}
	else if (DrawMeasureBar(row, ppat->rowsPerBeat, pew->BarComboIndex))
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


void CPatEd::DrawField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[col].get();

	if (pc->GetRealNoValue() < 0)
		hasvalue = true;

	char txt[6];
	FieldToText(txt, pc->GetParamType(), hasvalue, (void *)&data);
	
	int len = strlen(txt);
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

}

void CPatEd::DrawGraphicalField(CDC *pDC, int col, CColumn *pnc, int data, int x, int y, bool muted, bool hasvalue, COLORREF textcolor)
{
	CMachinePattern *ppat = pew->pPattern;
	CColumn *pc = ppat->columns[col].get();

	int w = pc->GetWidth();
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

int CPatEd::GetColumnWidth(int column)
{
	CMachinePattern *ppat = pew->pPattern;
	return (ppat->columns[column]->GetWidth()) * pew->fontSize.cx;
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
		if (x < colx - pew->fontSize.cx)
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
}

void CPatEd::InvalidateField(int row, int column)
{
	CMachinePattern *ppat = pew->pPattern;
	CRect r;
	r.left = GetColumnX(column);
	r.right = r.left + GetColumnWidth(column) - pew->fontSize.cx;
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
//	KillSelection();
	MoveCursor(cursor);
	Invalidate();
	UpdateStatusBar();
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

{	CMachinePattern *ppat = pew->pPattern;
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
		case 'X': ExportPattern(); break;
		case 'P': ImportPattern(); break;
		}
	}
	else if (ctrldown)
	{
		switch(nChar)
		{
		case 'R': Randomize(); break;
		case 'I': Interpolate(shiftdown); break;
		case 'T': if (shiftdown) SelectTrack(); else WriteState(); break;
		case 'M': MuteTrack(); break;
		case 'B': OldSelect(true); break;
		case 'E': OldSelect(false); break;
		case 'U': KillSelection(); break;
		case 'W': Rotate(shiftdown); break;
		case 'G': ToggleGraphicalMode(); break;
		case 'V': if (shiftdown) OnEditPasteSpecial(); break; //BWC
		case 'F': OnClearNoteOff(); break; //BWC
		case 'A': SelectAll(); break; //BWC
		case VK_PRIOR: Home(); break; //BWC
		case VK_NEXT: End(); break; //BWC
		case VK_HOME: HomeTop(); break; //BWC
		case VK_END: EndBottom(); break; //BWC
			
			
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
		case VK_PRIOR: CursorSelect(0, -BeatsInMeasureBar()); break; // BWC
		case VK_NEXT: CursorSelect(0, BeatsInMeasureBar()); break; // BWC
		case VK_HOME: CursorSelect(0, -(1 << 24)); break;
		case VK_END: CursorSelect(0, (1 << 24)); break;
		case VK_SUBTRACT: ShiftValues(-1); break;
		case VK_ADD: ShiftValues(1); break;
		case VK_DIVIDE: ShiftValues(-12); break; //BWC
		case VK_MULTIPLY: ShiftValues(12); break; //BWC
		}


	}
	else
	{
		switch(nChar)
		{
		case VK_UP: MoveCursorDelta(0, -1); break;
		case VK_DOWN: MoveCursorDelta(0, 1); break;
		// BWC : use BarComboIndex to compute the delta of prior/next page
		case VK_PRIOR: MoveCursorDelta(0, -BeatsInMeasureBar()); break;
		case VK_NEXT: MoveCursorDelta(0, BeatsInMeasureBar()); break;
		case VK_RIGHT: MoveCursorDelta(1, 0); break;
		case VK_LEFT: MoveCursorDelta(-1, 0); break;
		case VK_TAB: Tab(); break;
		case VK_HOME: HomeOld(); break;
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
			if (nChar >= '0' && nChar <= '9')
				EditByte(nChar - '0');
			else if (nChar == 'A') EditByte(0xA);
			else if (nChar == 'B') EditByte(0xB);
			else if (nChar == 'C') EditByte(0xC);
			else if (nChar == 'D') EditByte(0xD);
			else if (nChar == 'E') EditByte(0xE);
			else if (nChar == 'F') EditByte(0xF);
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
	int w = (pc->GetWidth() - 1) * pew->fontSize.cx - 4;
	int x = max(0, min(w, point.x - (GetColumnX(drawColumn) + 2)));

	double v = (double)x / w;

	CCursorPos cp = GetDigitAtPoint(point);
	pc->SetValueNormalized(cp.row, v);

	InvalidateField(cp.row, drawColumn);
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

	char txt[6];
	FieldToText(txt, pc->GetParamType(), pc->HasValue(cursor.row), (void *)&value);

	if (value != pc->GetNoValue())
	{
		char const *desc = pc->DescribeValue(value, pCB);

		s = txt;
		if (value != pc->GetNoValue() && desc != NULL && strlen(desc) > 0)
			s += (CString)" (" + desc + ")";
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
		Invalidate();
	}
}

void CPatEd::CursorSelect(int dx, int dy)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	if (!selection)
	{
		selStart.x = cursor.column;
		selStart.y = cursor.row;
		selection = true;
	}

	CCursorPos cp;
	cp.column = min(max(0, cursor.column + dx), (int)ppat->columns.size() - 1);
	cp.row = min(max(0, cursor.row + dy), ppat->GetRowCount() - 1);
	cp.digit = 0;
	MoveCursor(cp, false);

	selEnd.x = cursor.column;
	selEnd.y = cursor.row;

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

	Invalidate();
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

inline double Interpolate(double a, double v1, double v2, bool expintp)
{
	if (expintp && ((v1 > 0 && v2 > 0) || (v1 < 0 && v2 < 0)))
	{
		return (ExpIntp(v1, v2, a) - v1) / (v2 -v1);
	}
	else
	{
		return a;
	}
}

void CPatEd::Interpolate(bool expintp)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelRect();

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
/*
					if (v1 < v2)
						ppat->columns[col]->SetValueNormalized(row, (double)(row - r.top) / (r.bottom - r.top), v1, v2);
					else
						ppat->columns[col]->SetValueNormalized(row, 1.0 - (double)(row - r.top) / (r.bottom - r.top), v2, v1);
						*/
					if (v1 < v2)
						ppat->columns[col]->SetValueNormalized(row, ::Interpolate((double)(row - r.top) / (r.bottom - r.top), v1, v2, expintp), v1, v2);
					else
						ppat->columns[col]->SetValueNormalized(row, ::Interpolate(1.0 - (double)(row - r.top) / (r.bottom - r.top), v1, v2, expintp), v2, v1);
				}
			}
			
		}
	}

	Invalidate();

}

void CPatEd::ShiftValues(int delta)
{
	CMachinePattern *ppat = pew->pPattern;
	if (ppat == NULL || ppat->columns.size() == 0)
		return;

	CRect r = GetSelOrCursorRect();

	ppat->actions.BeginAction(pew, "Shift Values");
	{
		MACHINE_LOCK;

		for (int col = r.left; col <= r.right; col++)
			for (int row = r.top; row <= r.bottom; row++)
				ppat->columns[col]->ShiftValue(row, delta);

	}

	Invalidate();

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

	int vel = lParam;
	int n = wParam - pew->pCB->GetBaseOctave() * 12;

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

}