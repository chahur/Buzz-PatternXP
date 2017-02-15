#pragma once
#include "PatEd.h"
#include "EditorWnd.h"


struct One_chord_struct
{
	int position;
	int basenote;
	string chordname;
};

typedef std::vector<One_chord_struct> One_chord_vector;

struct Chords_progression_struct
{
	CString name;
	One_chord_vector chordlist;
};

typedef std::vector<Chords_progression_struct> Progressions_vector;


class CChordsProgression
{
public:
	CChordsProgression();
	~CChordsProgression();
	void LoadList(CComboBox *cb);
	void LoadChordsProgression(CListBox *list, CString aname);
	void SaveProgression(CString name, CEditorWnd *pew);
	int FindProgression(CString Aname);
	void Save(CMachineDataOutput * const po);
	void Init(CMachineDataInput * const pi);
	void GenerateChordsProgression(CString Aname, int baseoctave, CEditorWnd *pew);
	int GetCurrentProgressionChord_position(int index);
	int GetCurrentProgressionChord_note(int index);
	string GetCurrentProgressionChord_chord(int index);
	

protected:
	Progressions_vector progressionlist;
	int index_progression;
	void SaveProgression(int indexprogression, int first, int last, CEditorWnd *pew);
	void LoadChordsProgression(CListBox *list, int index);

};


extern CChordsProgression *gChordsProgression;
