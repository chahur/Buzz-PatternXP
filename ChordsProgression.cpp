#include "stdafx.h"
#include "ChordsProgression.h"


// Global pointer to the ChordsProgression object
CChordsProgression *gChordsProgression = NULL;


void CChordsProgression::LoadList(CComboBox *cb)
{
	cb->Clear();
	for (int i = 0; i < (int)progressionlist.size(); i++) {
		cb->AddString(progressionlist[i].name);
	}
}

void CChordsProgression::LoadChordsProgression(CListBox *list, CString aname)
{
	int indexprogression = -1;
	for (int i = 0; i < (int)progressionlist.size(); i++)
		if (aname.CompareNoCase(progressionlist[i].name) == 0) {
			indexprogression = i;
			break;
		}
	if (indexprogression >= 0) {
		LoadChordsProgression(list, indexprogression);
	}
}

void CChordsProgression::LoadChordsProgression(CListBox *list, int index)
{
	// index point to a valid ChordsProgression element
	if ((index < 0) || (index >(int)progressionlist.size())) {
		return;
	}

	index_progression = index;

	list->ResetContent();
	for (int i = 0; i < (int)progressionlist[index].chordlist.size(); i++) {
		char txt[255];
		char s_basenote[3];
		s_basenote[0] = NoteTo2char[progressionlist[index].chordlist[i].basenote * 2 + 0];
		s_basenote[1] = NoteTo2char[progressionlist[index].chordlist[i].basenote * 2 + 1];
		s_basenote[2] = 0;
		sprintf(txt,"[%d] %s%s", progressionlist[index].chordlist[i].position, s_basenote, progressionlist[index].chordlist[i].chordname.c_str());

		list->AddString(txt);
	}
		
}

int CChordsProgression::GetCurrentProgressionChord_position(int index)
{
	if ((index >= 0) && (index < (int)progressionlist[index_progression].chordlist.size()))
		return progressionlist[index_progression].chordlist[index].position;
	else
		return -1;
}

int CChordsProgression::GetCurrentProgressionChord_note(int index)
{
	if ((index >= 0) && (index < (int)progressionlist[index_progression].chordlist.size()))
		return progressionlist[index_progression].chordlist[index].basenote;
	else
		return -1;
}

string CChordsProgression::GetCurrentProgressionChord_chord(int index)
{
	if ((index >= 0) && (index < (int)progressionlist[index_progression].chordlist.size()))
		return progressionlist[index_progression].chordlist[index].chordname;
	else
		return "";
}

int CChordsProgression::FindProgression(CString Aname)
{
	// Search the progression.
	for (int i = 0; i < (int)progressionlist.size(); i++)
		if (Aname.CompareNoCase(progressionlist[i].name) == 0) {	
			return (i);
		}
	return (-1);
}

void CChordsProgression::SaveProgression(CString Aname, CEditorWnd *pew)
{
	// Search the progression.
	int indexprogression = FindProgression(Aname);

	if (indexprogression < 0) {
		// Create the progression in the list
		indexprogression = progressionlist.size();
		progressionlist.resize(indexprogression + 1);
		progressionlist[indexprogression].name = Aname;
	}

	CRect r = pew->pe.GetSelOrAll();
	SaveProgression(indexprogression, r.top, r.bottom, pew);

};


void CChordsProgression::GenerateChordsProgression(CString Aname, int baseoctave, CEditorWnd *pew)
{
	// For each chord of the progression
	// Set the cursor to the right row
	// Find the chord to be used in the pew->Chords vector
	// Generate the arpeggio

	// Search the progression.
	int indexprogression = FindProgression(Aname);

	if (indexprogression >= 0) {
		for (int i = 0; i < (int)progressionlist[indexprogression].chordlist.size(); i++) {
			// Move the cursor to the row : position
			pew->pe.MoveCursorRow(progressionlist[indexprogression].chordlist[i].position);
			
			int chord_index = -1;
			for (int j = 0; j < (int)pew->ChordsBase.size(); j++) {
				if (pew->ChordsBase[j].name == progressionlist[indexprogression].chordlist[i].chordname) {
					chord_index = j;
					break;
				}
			}
			if (chord_index>=0)
				pew->pe.InsertChordNote((baseoctave << 4) + progressionlist[indexprogression].chordlist[i].basenote + 1, chord_index);
		}
	}

}

void CChordsProgression::SaveProgression(int indexprogression, int first, int last, CEditorWnd *pew)
{
	if ((indexprogression < 0) || (indexprogression >(int)progressionlist.size()) ) {
		return; 
	}

	// indexprogression is a valid element of the list
	// Clear the list
	progressionlist[indexprogression].chordlist.clear();
	int count_chords = 0;

	for (int y = first; y < last; y++)
	{
		if (y < (int)pew->RowNotes.size())
		{
			if (pew->RowNotes[y].chord_index >= 0)
			{
				progressionlist[indexprogression].chordlist.resize(count_chords + 1);
				progressionlist[indexprogression].chordlist[count_chords].position = y - first;
					progressionlist[indexprogression].chordlist[count_chords].chordname =
					pew->ChordsBase[pew->RowNotes[y].chord_index].name;
				progressionlist[indexprogression].chordlist[count_chords].basenote = 
					pew->RowNotes[y].base_note;

				count_chords++;
			}
			else
				if (pew->RowNotes[y].chord_index == -2)
				{
					// Skip unknown chords
				}

		}
	}
}

#define PROGRESSION_MAGIC_NUMBER 22303
#define EMPTY_MAGIC_NUMBER 22304
#define MAX_PROGRESSION_SIZE 500
#define MAX_CHORDS_COUNT 500

void CChordsProgression::Save(CMachineDataOutput * const po)
{
	// 0. Magic number
	po->Write(PROGRESSION_MAGIC_NUMBER);
	// 1. Count of chords progressions
	po->Write((int)progressionlist.size());

	for (int ip = 0; ip < (int)progressionlist.size(); ip++) {
		// 2.1 Name of current chords progression
		po->Write(progressionlist[ip].name);
		// 2.2 Count of Chords in the progression
		po->Write((int)progressionlist[ip].chordlist.size());

		for (int i = 0; i < (int)progressionlist[ip].chordlist.size(); i++) {
			// 3. Current chord
			po->Write(progressionlist[ip].chordlist[i].basenote);
			po->Write(progressionlist[ip].chordlist[i].position);
			po->Write(progressionlist[ip].chordlist[i].chordname.c_str());
		}
	}
}

void CChordsProgression::Init(CMachineDataInput * const pi)
{

	// 0. Magic number
	int magic_number;
	pi->Read(magic_number);
	if (magic_number == EMPTY_MAGIC_NUMBER) return;

	// Reset the list
	if (!progressionlist.empty()) progressionlist.clear();

	if (magic_number != PROGRESSION_MAGIC_NUMBER) return;
	
	// 1. Count of chords progressions
	int prog_size;
	pi->Read(prog_size);
	if ((prog_size>0) && (prog_size < MAX_PROGRESSION_SIZE)){
		progressionlist.resize(prog_size);
		for (int ip = 0; ip < prog_size; ip++) {
			// 2.1 Name of current chords progression
			CString prog_name = pi->ReadString();
			progressionlist[ip].name = prog_name;
			// 2.2 Count of Chords in the progression
			int chords_count;
			pi->Read(chords_count);
			if ((chords_count > 0) && (chords_count < MAX_CHORDS_COUNT)) {
				progressionlist[ip].chordlist.resize(chords_count);

				for (int i = 0; i < chords_count; i++) {
					// 3. Current chord
					pi->Read(progressionlist[ip].chordlist[i].basenote);
					pi->Read(progressionlist[ip].chordlist[i].position);
					CString chord_name = pi->ReadString();
					progressionlist[ip].chordlist[i].chordname = chord_name;
				}
			}
			else {
				// Something goes wrong
				return;
			}
		}
	}
}


CChordsProgression::CChordsProgression()
{


}


CChordsProgression::~CChordsProgression()
{

}

