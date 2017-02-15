#include "stdafx.h"
#include "EditorWnd.h"
#include "RecQueue.h"

#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifndef BWC
#define CHECK_BUILD_NUMBER
#endif

#ifdef CHECK_BUILD_NUMBER
static char const *BuildNumber =
#include "../../buildcount"
;
#endif

static bool PatternsByName(CMachinePattern *a, CMachinePattern * b) { return a->name < b->name; }

CMachineParameter const paraDummy = 
{ 
	pt_byte,										// type
	"Dummy",
	"Dummy",							// description
	0,												// MinValue	
	127,											// MaxValue
	255,												// NoValue
	MPF_STATE,										// Flags
	0
};


static CMachineParameter const *pParameters[] = { 
	// track
	&paraDummy,
};

#pragma pack(1)

class gvals
{
public:
	byte dummy;

};

#pragma pack()

CMachineInfo const MacInfo = 
{
	MT_GENERATOR,							// type
	MI_VERSION,
	MIF_PATTERN_EDITOR | MIF_PE_NO_CLIENT_EDGE | MIF_NO_OUTPUT | MIF_CONTROL_MACHINE,						// flags
	0,										// min tracks
	0,										// max tracks
	1,										// numGlobalParameters
	0,										// numTrackParameters
	pParameters,
	0, 
	NULL,
#ifdef PXPMOD
#ifdef _WIN64
	"Jeskola Pattern XP mod.x64",
#else
	"Jeskola Pattern XP mod",
#endif
	"Pattern XP mod",						// short name

#else
#ifdef _WIN64
	"Jeskola Pattern XP.x64",
#else
	"Jeskola Pattern XP",
#endif
	"Pattern XP",						// short name
#endif
	"Oskari Tammelin / Ronan Daniel", 	// author
	NULL
};

class mi;

class miex : public CMachineInterfaceEx
{
public:
	virtual void *CreatePatternEditor(void *parenthwnd);
	virtual void CreatePattern(CPattern *p, int numrows);
	virtual void CreatePatternCopy(CPattern *pnew, CPattern const *pold);
	virtual void DeletePattern(CPattern *p);
	virtual void RenamePattern(CPattern *p, char const *name);
	virtual void SetPatternLength(CPattern *p, int length);
	virtual void PlayPattern(CPattern *p, CSequence *s, int offset);
	virtual void SetEditorPattern(CPattern *p);
	virtual void AddTrack(); 
	virtual void DeleteLastTrack();
	virtual bool EnableCommandUI(int id);
	virtual void SetPatternTargetMachine(CPattern *p, CMachine *pmac);
	virtual bool ShowPatternProperties();
	virtual bool ImportPattern(CPattern *p);
	virtual void RecordControlChange(CMachine *pmac, int group, int track, int param, int value);
	virtual void MidiControlChange(int const ctrl, int const channel, int const value);
	virtual void BeginWriteToPlayingPattern(CMachine *pmac, int quantization, CPatternWriteInfo &outpwi);
	virtual void WriteToPlayingPattern(CMachine *pmac, int group, int track, int param, int value);
	virtual void EndWriteToPlayingPattern(CMachine *pmac);
	virtual int GetEditorPatternPosition();
	virtual void GotMidiFocus();
	virtual void LostMidiFocus();
	virtual bool ExportMidiEvents(CPattern *p, CMachineDataOutput *pout);
	virtual bool ImportMidiEvents(CPattern *p, CMachineDataInput *pin);
	virtual void UpdateWaveReferences(CPattern *p, byte const *remap);


public:
	mi *pmi;

};
class mi : public CMachineInterface
{
public:
	mi();
	virtual ~mi();

	virtual void Init(CMachineDataInput * const pi);
	virtual void Tick();
	virtual bool Work(float *psamples, int numsamples, int const mode);
	virtual void Save(CMachineDataOutput * const po);
	virtual void Stop();

	bool AddMachineEvent(CMachine *pmac) { return true; }
	bool DeleteMachineEvent(CMachine *pmac)
	{ 
		// delete pattern columns for the deleted machine
		for (MapPatternToMachinePattern::iterator i = patterns.begin(); i != patterns.end(); i++)
			(*i).second->DeleteMachine(pmac);

		patEd->pe.ColumnsChanged();

		if (targetMachine != NULL && pmac == targetMachine)
			targetMachineDeleted = true;

		for (auto _i = globalData.activeMidiNotes.begin(); _i != globalData.activeMidiNotes.end();)
		{
			auto i = _i++;
			if ((*i).first.first == pmac) globalData.activeMidiNotes.erase(i);
		}

		return true; 
	}

	bool UndeleteMachineEvent(CMachine *pmac)
	{ 
		// restore deleted pattern columns
		for (MapPatternToMachinePattern::iterator i = patterns.begin(); i != patterns.end(); i++)
			(*i).second->UndeleteMachine(pmac);

		patEd->pe.ColumnsChanged();

		if (targetMachine != NULL && pmac == targetMachine)
			targetMachineDeleted = false;

		return true; 
	}


	bool RenameMachineEvent(CMachine *pmac)
	{
		for (MapPatternToMachinePattern::iterator i = patterns.begin(); i != patterns.end(); i++)
			(*i).second->RenameMachine(pmac, pCB);

		return true;
	}

	bool DClickMachine(void *)
	{
		pCB->SetPatternEditorMachine(patEd->pMachine, true);
		return true;
	}

	void UpdatePatternList()
	{
		vector<CMachinePattern *> newlist;

		for (MapPatternToMachinePattern::iterator i = patterns.begin(); i != patterns.end(); i++) 
			newlist.push_back((*i).second.get());

		std::sort(newlist.begin(), newlist.end(), PatternsByName);

		MapIntToInt remap;

		for (int i = 0; i < (int)globalData.patternList.size(); i++)
			for (int j = 0; j < (int)newlist.size(); j++)
				if (i != j && globalData.patternList[i] == newlist[j])
					remap[i] = j;

		globalData.patternList = newlist;

		for (int i = 0; i < (int)globalData.patternList.size(); i++)
			globalData.patternList[i]->UpdatePatternReferences(remap);

	}

	virtual void MidiNote(int const channel, int const value, int const velocity);

	void Update(CSubTickInfo const *psti);

	void GotMidiFocus();
	void LostMidiFocus();

	CMachine *GetMidiTargetMachine();

	void DoMidiPlayback();
	void ReleaseMidiNotes();

public:
	miex ex;
	MapPatternToMachinePattern patterns;
	MapStringToMachinePattern loadedPatterns;
	MapIntToPlayingPattern playingPatterns;

	CGlobalData globalData;

	CEditorWnd *patEd;
	int CycleTestFrame;
	CRecQueue recQueue;
	CCriticalSection patternCS;
	int writeRow;
	bool writingToPattern;
	bool ignoreExternalWriteToPattern;

	set<int> activeMidiNotes;
	CCriticalSection amnCS;

	CMachine *targetMachine;
	bool targetMachineDeleted;

	gvals gval;

};

void miex::GotMidiFocus() { pmi->GotMidiFocus(); }
void miex::LostMidiFocus() { pmi->LostMidiFocus(); }
bool miex::ExportMidiEvents(CPattern *p, CMachineDataOutput *pout) { return pmi->patterns[p]->ExportMidiEvents(pout, pmi->targetMachine); }
bool miex::ImportMidiEvents(CPattern *p, CMachineDataInput *pin) 
{ 
	auto pmp = pmi->patterns[p];
	
	if (pmi->patEd->pPattern) // BWC
	{
		pmp->actions.BeginAction(pmi->patEd, "Add Track");
	}
	pmi->patEd->AddTrack(0xffff);

	return pmp->ImportMidiEvents(pin, pmi->targetMachine, pmi->pCB); 
}



DLL_EXPORTS_AFX

mi::mi()
{
	ex.pmi = this;
	GlobalVals = &gval;
	TrackVals = NULL;
	AttrVals = NULL;
	patEd = new CEditorWnd();
	CycleTestFrame = 0;
	writingToPattern = false;
	targetMachine = NULL;
	targetMachineDeleted = false;
}

mi::~mi()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	delete patEd;
}

void mi::Init(CMachineDataInput * const pi)
{
#ifdef CHECK_BUILD_NUMBER
	int hostbn = pCB->GetBuildNumber();
	int pxpbn = atoi(BuildNumber);
	if (hostbn != pxpbn)
	{
		CString s;
		s.Format("Incompatible build (Host %d, Pattern XP %d)", hostbn, pxpbn);
		::MessageBox(NULL, s, "Pattern XP", MB_OK | MB_ICONWARNING);
	}

#endif

	pCB->SetMachineInterfaceEx(&ex);
	patEd->pCB = pCB;
	patEd->pMachine = pCB->GetThisMachine();
	patEd->pGlobalData = &globalData;
	pCB->SetEventHandler(patEd->pMachine, gAddMachine, (EVENT_HANDLER_PTR)&mi::AddMachineEvent, NULL);
	pCB->SetEventHandler(patEd->pMachine, gDeleteMachine, (EVENT_HANDLER_PTR)&mi::DeleteMachineEvent, NULL);
	pCB->SetEventHandler(patEd->pMachine, gUndeleteMachine, (EVENT_HANDLER_PTR)&mi::UndeleteMachineEvent, NULL);
	pCB->SetEventHandler(patEd->pMachine, gRenameMachine, (EVENT_HANDLER_PTR)&mi::RenameMachineEvent, NULL);
	pCB->SetEventHandler(patEd->pMachine, DoubleClickMachine, (EVENT_HANDLER_PTR)&mi::DClickMachine, NULL);

	if (pi != NULL)
	{
		byte version;
		pi->Read(version);
		if (version < 1 || version > PATTERNXP_DATA_VERSION)
		{
			AfxMessageBox("invalid data");
			return;
		}

		int numpat;
		pi->Read(numpat);

		for (int i = 0; i < numpat; i++)
		{
			CString name = pi->ReadString();
			shared_ptr<CMachinePattern> p(new CMachinePattern());
			p->Read(pi, version);
			loadedPatterns[name] = p;
		}

		// Add at the end, read the new params
		byte ci = 0;
		pi->Read(ci);
		if ((ci>=0)&&(ci<=8))
		  patEd->BarComboIndex = ci;
		else
		  patEd->BarComboIndex = 0;

		pi->Read(ci);
		if ((ci>=0)&&(ci<=30))
		  patEd->TonalComboIndex = ci;
		else
		  patEd->TonalComboIndex = 0;

		// Load the Chords Progression datas
		if (gChordsProgression == NULL) {
			// First PatternXP loaded, create the global ChrodsProgression object
			gChordsProgression = new CChordsProgression();
		}
		gChordsProgression->Init(pi);
	}

}

void mi::Save(CMachineDataOutput * const po)
{
	byte version = PATTERNXP_DATA_VERSION;
	po->Write(version);
	po->Write((int)patterns.size());

	for (MapPatternToMachinePattern::iterator i = patterns.begin(); i != patterns.end(); i++)
	{
		char const *name = pCB->GetPatternName((*i).first);
		po->Write(name);
		(*i).second->Write(po);
	}

	// Add at the end, write the new params
	byte ci;
	ci = patEd->BarComboIndex;
	po->Write(ci);

	ci = patEd->TonalComboIndex;
	po->Write(ci);

	// Save the Chords Progression datas
	gChordsProgression->Save(po);
}

void *miex::CreatePatternEditor(void *parenthwnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pmi->patEd->Create((HWND)parenthwnd);
	return pmi->patEd->GetSafeHwnd();
}

void miex::CreatePattern(CPattern *p, int numrows)
{
	MapStringToMachinePattern::iterator i = pmi->loadedPatterns.find(pmi->pCB->GetPatternName(p));
	if (i != pmi->loadedPatterns.end())
	{
		(*i).second->pPattern = p;
		pmi->patterns[p] = (*i).second;
		(*i).second->Init(pmi->pCB, numrows, true);
		pmi->loadedPatterns.erase(i);
	}
	else
	{
		if (pmi->patEd->pPattern != NULL)
			pmi->patterns[p] = shared_ptr<CMachinePattern>(new CMachinePattern(pmi->pCB, p, pmi->patEd->pPattern, numrows, false));		// copy columns from current pattern
		else
			pmi->patterns[p] = shared_ptr<CMachinePattern>(new CMachinePattern(pmi->pCB, p, numrows));
	}

	pmi->UpdatePatternList();
}

void miex::CreatePatternCopy(CPattern *pnew, CPattern const *pold)
{
	pmi->patterns[pnew] = shared_ptr<CMachinePattern>(new CMachinePattern(pmi->pCB, pnew, pmi->patterns[(CPattern *)pold].get(), 0, true));
	pmi->UpdatePatternList();
}

void miex::DeletePattern(CPattern *p)
{
	pmi->patternCS.Lock();

	CMachinePattern *pmp = pmi->patterns[p].get();

	for (MapIntToPlayingPattern::iterator i = pmi->playingPatterns.begin(); i != pmi->playingPatterns.end();)
	{
		MapIntToPlayingPattern::iterator t = i++;

		if ((*t).second->ppat == pmp)
			pmi->playingPatterns.erase(t);
		else
			(*t).second->Stop(pmp);		// stop subpatterns
	}


	pmi->patterns.erase(pmi->patterns.find(p));
	pmi->UpdatePatternList();

	pmi->patternCS.Unlock();
}

void miex::RenamePattern(CPattern *p, char const *name)
{
	pmi->patterns[p].get()->Rename(name);
	pmi->UpdatePatternList();
}

void miex::SetPatternLength(CPattern *p, int length)
{
	pmi->patEd->SetPatternLength(pmi->patterns[p].get(), length);
}

void miex::PlayPattern(CPattern *p, CSequence *s, int offset)
{
	int sc = pmi->pCB->GetSequenceColumn(s);
	MapIntToPlayingPattern npp;

	for (MapIntToPlayingPattern::iterator i = pmi->playingPatterns.begin(); i != pmi->playingPatterns.end(); i++)
	{
		int c = pmi->pCB->GetSequenceColumn((*i).second->pseq);

		if (c != sc)
			npp[c] = (*i).second;
	}

	if (p != NULL)
		npp[sc] = shared_ptr<CPlayingPattern>(new CPlayingPattern(pmi->patterns[p].get(), s, offset - 1, 0, shared_ptr<CModulators>()));
 
	pmi->playingPatterns = npp;
}
 
void mi::Stop()
{
	patternCS.Lock();
	playingPatterns.clear();
	ReleaseMidiNotes();
	patternCS.Unlock();
}



void miex::SetEditorPattern(CPattern *p)
{
	if (p == NULL)
		pmi->patEd->SetPattern(NULL);
	else
		pmi->patEd->SetPattern(pmi->patterns.find(p)->second.get());
}

void miex::AddTrack() { pmi->patEd->AddTrack(); }
void miex::DeleteLastTrack() { pmi->patEd->DeleteLastTrack(); }

// this is called after CreatePattern to set the default target machine
void miex::SetPatternTargetMachine(CPattern *p, CMachine *pmac)
{
	pmi->patterns[p]->SetTargetMachine(pmac, pmi->pCB);
	pmi->targetMachine = pmac;
}


bool miex::ShowPatternProperties()
{
	pmi->patEd->OnColumns();
	return true;
}

bool miex::ImportPattern(CPattern *p)
{
	pmi->patterns[p]->Import(pmi->pCB);
	return true;
}

void miex::UpdateWaveReferences(CPattern *p, byte const *remap)
{
	pmi->patterns[p]->UpdateWaveReferences(remap);
}

bool miex::EnableCommandUI(int id)
{
	return pmi->patEd->EnableCommandUI(id);
}

void miex::RecordControlChange(CMachine *pmac, int group, int track, int param, int value)
{
	pmi->recQueue.Push(CRecQueue::Event(pmac, group, track, param, value));
}

CMachine *mi::GetMidiTargetMachine()
{
	CColumn *pc = patEd->pe.GetCursorColumn();
	if (pc != NULL)
		return pc->GetMachine();
	else
		return targetMachineDeleted ? NULL : targetMachine;
}

void mi::MidiNote(int const channel, int const value, int const velocity)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CColumn *pc = patEd->pe.GetCursorColumn();
	CMachine *pmac = GetMidiTargetMachine();

	if (pmac != NULL)
	{
		if (pCB->GetStateFlags() & SF_RECORDING)
		{
			recQueue.Push(CRecQueue::Event(pmac, -1, channel, value, velocity));
			patEd->pe.InvalidateInTimer();
		}

		amnCS.Lock();

		pCB->SendMidiNote(pmac, channel, value, velocity);

		if (pc != NULL && channel == 0 && patEd->MidiEditMode && !(pCB->GetStateFlags() & SF_RECORDING))
			::PostMessage(patEd->pe.GetSafeHwnd(), CPatEd::WM_MIDI_NOTE, value, velocity);

		if (velocity > 0)
			activeMidiNotes.insert((channel << 16) | value);
		else
			activeMidiNotes.erase((channel << 16) | value);

		amnCS.Unlock();
	}

}

void miex::MidiControlChange(int const ctrl, int const channel, int const value)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CMachine *pmac = pmi->GetMidiTargetMachine();
	if (pmac != NULL)
	{
		if (pmi->pCB->GetStateFlags() & SF_RECORDING)
		{
			pmi->recQueue.Push(CRecQueue::Event(pmac, -2, channel, ctrl, value));
			pmi->patEd->pe.InvalidateInTimer();
		}

		pmi->pCB->SendMidiControlChange(pmac, ctrl, channel, value);
	}
}

void mi::GotMidiFocus()
{

}

void mi::LostMidiFocus()
{
	CMachine *pmac = GetMidiTargetMachine();

	amnCS.Lock();

	if (pmac != NULL)
	{
		for (set<int>::iterator i = activeMidiNotes.begin(); i != activeMidiNotes.end(); i++)
			pCB->SendMidiNote(pmac, *i >> 16, *i & 0xffff, 0);
	
		// NOTE: could also send All Notes Off here
	}


	activeMidiNotes.clear();

	amnCS.Unlock();
}


void miex::BeginWriteToPlayingPattern(CMachine *pmac, int quantization, CPatternWriteInfo &outpwi) 
{
	pmi->patternCS.Lock();

	if (pmi->writingToPattern)
		return;

	if (pmi->playingPatterns.size() == 0)
		return;
	
	float pp = pmi->playingPatterns.begin()->second->GetPlayPos();

	if (quantization > 0)
	{
		float pos = floor(pp * quantization + 0.5f) / quantization;
		outpwi.BuzzTickPosition = (pos - (int)pos) * 4.0f / pmi->playingPatterns.begin()->second->ppat->rowsPerBeat;
		pmi->writeRow = (int)pos;
		
	}
	else
	{
		pmi->writeRow = (int)pp;
		outpwi.BuzzTickPosition = 0;
	}

	outpwi.Row = pmi->writeRow;

	pmi->ignoreExternalWriteToPattern = pmi->playingPatterns.begin()->second->ppat->HasMidiNoteColumn();
	pmi->writingToPattern = true;
}

void miex::WriteToPlayingPattern(CMachine *pmac, int group, int track, int param, int value) 
{
	if (!pmi->writingToPattern)
		return;

	if (!pmi->ignoreExternalWriteToPattern)
		pmi->playingPatterns.begin()->second->ppat->SetValue(pmi->writeRow, pmac, group, track, param, value);
}

void miex::EndWriteToPlayingPattern(CMachine *pmac) 
{
	pmi->patternCS.Unlock();

	if (!pmi->writingToPattern)
		return;

	pmi->patEd->pe.InvalidateInTimer();

	pmi->writingToPattern = false;
}

int miex::GetEditorPatternPosition()
{
	return pmi->patEd->GetEditorPatternPosition();
}


void mi::Tick()
{
	CSubTickInfo const *psti = pCB->GetSubTickInfo();
	
	if (psti != NULL)
	{
		globalData.currentSubtick = 0;
		globalData.subticksPerTick = psti->SubTicksPerTick;
	}
	
	Update(NULL);
}

bool mi::Work(float *psamples, int numsamples, int const)
{
	CSubTickInfo const *psti = pCB->GetSubTickInfo();

	if (pMasterInfo->PosInTick != 0 && psti != NULL && psti->PosInSubTick == 0)
	{
		if (psti != NULL)
		{
			globalData.currentSubtick = psti->CurrentSubTick;
			globalData.subticksPerTick = psti->SubTicksPerTick;
		}
		Update(psti);
	}

	DoMidiPlayback();

	return false;	
}

void mi::Update(CSubTickInfo const *psti)
{
	patternCS.Lock();

	globalData.currentRowInBeat = -1;

	bool const rec = !recQueue.IsEmpty();

	int index = 0;

	for (MapIntToPlayingPattern::iterator i = playingPatterns.begin(); i != playingPatterns.end(); index++)
	{
		MapIntToPlayingPattern::iterator t = i++;
		if (!(*t).second->Play(psti, globalData, ++CycleTestFrame, (index == 0) ? &recQueue : NULL, pCB))
			playingPatterns.erase(t);
	}

	patEd->SetPlayPos(playingPatterns, pMasterInfo);

	if (rec)
		patEd->pe.InvalidateInTimer();

	patternCS.Unlock();
}

void mi::DoMidiPlayback()
{
	if (globalData.activeMidiNotes.size() == 0) return;

	CSubTickInfo const *psti = pCB->GetSubTickInfo();
	int cst, stpt;

	if (psti != NULL)
	{
		cst = psti->CurrentSubTick;
		stpt = psti->SubTicksPerTick;
	
	}
	else
	{
		cst = 0;
		stpt = 1;
	}

	for (auto _i = globalData.activeMidiNotes.begin(); _i != globalData.activeMidiNotes.end();)
	{
		auto i = _i++;

		auto range = GetRowSubtickRange((*i).second.rowInBeat, (*i).second.RPB, stpt);

		int dtime = range.first + min((*i).second.delaytime * (range.second - range.first) / 96, stpt - 1);
		int ctime = (*i).second.cuttime < 96 ? range.first + min((*i).second.cuttime * (range.second - range.first) / 96, stpt - 1) : 0x7fffffff;
		
		if ((*i).second.state == ActiveMidiNote::note_on_pending && cst >= dtime)
		{
			// send note offs first to make monophonic mode synths work, except when the note number is the same

			if ((*i).second.previousNote >= 0 && (*i).second.previousNote == (*i).second.note)
				pCB->SendMidiNote((*i).first.first, 0, (*i).second.previousNote, 0);

			pCB->SendMidiNote((*i).first.first, 0, (*i).second.note, (*i).second.velocity);

			if ((*i).second.previousNote >= 0 && (*i).second.previousNote != (*i).second.note)
				pCB->SendMidiNote((*i).first.first, 0, (*i).second.previousNote, 0);

			(*i).second.state = ActiveMidiNote::playing;
		}
		
		if ((*i).second.pw >= 0 && cst >= dtime)
		{
			pCB->SendMidiControlChange((*i).first.first, 255, 0, (*i).second.pw);
			(*i).second.pw = -1;
		}

		if ((*i).second.cc >= 0 && cst >= dtime)
		{
			pCB->SendMidiControlChange((*i).first.first, ((*i).second.cc >> 8) & 0x7f, 0, (*i).second.cc & 0x7f);
			(*i).second.cc = -1;
		}
		
		if (cst >= ctime)
		{
			pCB->SendMidiNote((*i).first.first, 0, (*i).second.note, 0);
			globalData.activeMidiNotes.erase(i);
		}
		else if ((*i).second.state == ActiveMidiNote::pw_or_cc && (*i).second.pw < 0 && (*i).second.cc < 0)
		{
			globalData.activeMidiNotes.erase(i);
		}

	}
}

void mi::ReleaseMidiNotes()
{
	for (auto _i = globalData.activeMidiNotes.begin(); _i != globalData.activeMidiNotes.end();)
	{
		auto i = _i++;

		if ((*i).second.state == ActiveMidiNote::playing)
		{
			pCB->SendMidiNote((*i).first.first, 0, (*i).second.note, 0);
			globalData.activeMidiNotes.erase(i);
		}
	}
}

