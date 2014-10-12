#pragma once

#include <functional>
#include "../../buzz/MachineInterface.h"
#include "ActionStack.h"
#include "RecQueue.h"

int const DefaultMidiVelocity = 100;

enum InternalParameter 
{ 
	SPGlobalTrigger = -10,
	SPGlobalEffect1 = -11,
	SPGlobalEffect1Data = -12,
	
	FirstInternalTrackParameter = -101,
	SPTrackTrigger = -110,
	SPTrackEffect1 = -111,
	SPTrackEffect1Data = -112,

	FirstMidiTrackParameter = -128,
	MidiNote = -128,
	MidiVelocity = -129,
	MidiNoteDelay = -130,
	MidiNoteCut = -131,
	MidiPitchWheel = -132,
	MidiCC = -133

};

CMachineParameter const ipMIDINote = { pt_note, "MIDI Note", "MIDI Note", NOTE_MIN, NOTE_MAX, NOTE_NO, 0, 0 };
CMachineParameter const ipMIDIVelocity = { pt_byte, "MIDI Velocity", "MIDI Velocity", 1, 127, 255, 0, 0 };
CMachineParameter const ipMIDINoteDelay = { pt_byte, "MIDI Note Delay", "MIDI Note Delay", 0, 95, 255, 0, 0 };
CMachineParameter const ipMIDINoteCut = { pt_byte, "MIDI Note Cut", "MIDI Note Cut", 0, 95, 255, 0, 0 };
CMachineParameter const ipMIDIPitchWheel = { pt_word, "MIDI Pitch Wheel", "MIDI Pitch Wheel", 0, 0x3fff, 0xffff, 0, 0 };
CMachineParameter const ipMIDICC = { pt_word, "MIDI CC", "MIDI CC", 0, 0x7fff, 0xffff, 0, 0 };

CMachineParameter const ipSPGlobalTrigger = { pt_byte, "SPTrigger G", "SubPattern Global Trigger", 0, 254, 255, 0, 0 };
CMachineParameter const ipSPGlobalEffect1 = { pt_byte, "Effect 1 G", "SubPattern Global Effect 1", 0, 254, 255, 0, 0 };
CMachineParameter const ipSPGlobalEffect1Data = { pt_byte, "Effect 1 Data G", "SubPattern Global Effect 1 Data", 0, 255, 0, 0, 0 };

CMachineParameter const ipSPTrackTrigger = { pt_byte, "SPTrigger T", "SubPattern Track Trigger", 0, 254, 255, 0, 0 };
CMachineParameter const ipSPTrackEffect1 = { pt_byte, "Effect 1 T", "SubPattern Track Effect 1", 0, 254, 255, 0, 0 };
CMachineParameter const ipSPTrackEffect1Data = { pt_byte, "Effect 1 Data T", "SubPattern Track Effect 1 Data", 0, 255, 0, 0, 0 };

#define EFFECT_PLAY_MODE				0x01
#define EFFECT_OFFSET					0x02
#define EFFECT_DIATONIC_TRANSPOSE_UP	0x11
#define EFFECT_DIATONIC_TRANSPOSE_DOWN	0x12

#define BUZZ_TICKS_PER_BEAT		4

typedef pair<CMachine *, int> MacIntPair;
typedef vector<MacIntPair> MacParamPairVector;

typedef map<int, int> MapIntToInt;
typedef vector<MapIntToInt> MapIntToIntVector;

typedef set<MacIntPair> MacTrackSet;

inline pair<int, int> GetRowSubtickRange(int rowinbeat, int rpb, int subtickspertick)
{
	double x = BUZZ_TICKS_PER_BEAT * rowinbeat / (double)rpb;
	double subtick = x * subtickspertick;
	int firstSubtick = (int)(subtick + 0.5) % subtickspertick;

	x = BUZZ_TICKS_PER_BEAT * (rowinbeat + 1)  / (double)rpb;
	subtick = x * subtickspertick;
	int lastSubtick = (int)(subtick + 0.5) % subtickspertick;
	if (lastSubtick == 0) lastSubtick = subtickspertick;

	return pair<int, int>(firstSubtick, lastSubtick);
}


class CValue
{
public:
	CValue() {}
	CValue(int v) : value(v) {}

	void Write(CMachineDataOutput * const po) const
	{
		po->Write(value);
	}

	void Read(CMachineDataInput * const pi, byte ver)
	{
		pi->Read(value);
	}

	operator int () const { return value; }

private:
	int value;
};

typedef map<int, CValue> MapIntToValue;
typedef vector<MapIntToValue> MapIntToValueVector;


struct CTrackID
{
	CTrackID() {}
	CTrackID(CMachine *pmac, int g, int t)
	{
		pMachine = pmac;
		groupAndTrack = (g << 16) | t;
	}

	bool operator < (CTrackID const &x) const
	{
		// NOTE: machines are not processed in the order they are displayed because of this
		if ((int)pMachine < (int)x.pMachine) return true;
		else if ((int)pMachine > (int)x.pMachine) return false;
		else return groupAndTrack < x.groupAndTrack;
	}
		
	CMachine *pMachine;
	int groupAndTrack;
};

class CModulators
{
public:
	CModulators()
	{
		diatonicTransposeAmount = 0;
		diatonicTransposeKey = 0;
	}

	void Add(int param, int mod)
	{
		paramModulators[param] = mod;
	}

	int DiatonicTransposition(int note)
	{
		if (diatonicTransposeAmount == 0)
			return note;

		static byte const c2d[12] = { 0, 0 | 128, 1, 1 | 128, 2, 3, 3 | 128, 4, 4 | 128, 5, 5 | 128, 6 };
		static byte const d2c[7] = { 0, 2, 4, 5, 7, 9, 11 };
		note -= diatonicTransposeKey - 12;

		int dtn = c2d[note % 12];
		bool sharp = (dtn & 128) != 0;
		dtn &= 127;
		dtn += diatonicTransposeAmount + 700;
		int oct = (int)(note / 12 + (dtn / 7.0f - 100));
		dtn %= 7;
		note = d2c[dtn] + oct * 12;
		if (sharp) note++;

		return note + diatonicTransposeKey - 12;
	}

	MapIntToInt paramModulators;
	int diatonicTransposeKey;
	int diatonicTransposeAmount;

};

class CSubPatternControl
{
public:
	CSubPatternControl(int spi, int to)
	{
		subPatternIndex = spi;
		trackOffset = to;
		effectCount = 0;
		effectDataCount = 0;
		modulators = make_shared<CModulators>();
	}

	void AddModulator(int param, int mod)
	{
		modulators->Add(param, mod);
	}

	void AddEffect(int value)
	{
		effects[effectCount++] = value;
	}

	void AddEffectData(int value)
	{
		effectData[effectDataCount++] = value;
	}

	int subPatternIndex;
	int trackOffset;
	int effectCount;
	int effectDataCount;
	int effects[2];
	int effectData[2];
	shared_ptr<CModulators> modulators;
};

struct ActiveMidiNote
{
	enum State { note_on_pending, playing, note_off_pending, recording, pw_or_cc };

	State state;
	int note;
	int velocity;
	int previousNote;
	int delaytime;
	int cuttime;
	int pw;
	int cc;
	int rowInBeat;
	int RPB;
};

class CMachinePattern;

typedef map<MacIntPair, ActiveMidiNote> MapMacAndTrackToActiveMidiNote;

class CGlobalData
{
public:
	vector<CMachinePattern *> patternList;
	MapMacAndTrackToActiveMidiNote activeMidiNotes;
	MapMacAndTrackToActiveMidiNote recordingMidiNotes;
	int currentRPB;
	int currentRowInBeat;

	int currentSubtick;
	int subticksPerTick;
};

typedef map<CTrackID, shared_ptr<CSubPatternControl>> MapTrackIDToSPControl;

inline int DecodeNote(int x) { return (x >> 4) * 12 + ((x & 15) - 1); }
inline int EncodeNote(int x) { return ((x / 12) << 4) + (x % 12) + 1; }

inline int EncodeMidi(int status, int data1, int data2) { return (int)(status | (data1 << 8) | (data2 << 16)); }
inline int EncodeMidiNoteOn(int channel, int note, int velocity) { return EncodeMidi(0x90 + channel, note, velocity); }
inline int EncodeMidiNoteOff(int channel, int note) { return EncodeMidi(0x80 + channel, note, 0); }

class CColumn
{
public:
	CColumn()
	{
		pMachine = NULL;
		pParam = NULL;
		graphical = false;
	}

	CColumn(CColumn *pc, bool copydata, bool inctrack = false)
	{
		machineName = pc->machineName;
		paramIndex = pc->paramIndex;
		paramGroup = pc->paramGroup;
		paramTrack = pc->paramTrack + (inctrack ? 1 : 0);
		pMachine = pc->pMachine;
		pParam = pc->pParam;
		numGlobalParameters = pc->numGlobalParameters;
		graphical = false;

		if (copydata)
			events = pc->events;
	}

	CColumn(CMICallbacks *pcb, MacIntPair const &mpp, int track)
	{
		pMachine = mpp.first;
		paramIndex = mpp.second;
		paramTrack = track;
		machineName = pcb->GetMachineName(pMachine);
		graphical = false;

		Init(pcb);
	}

	void Init(CMICallbacks *pcb, bool loadedpattern = false)
	{
		ASSERT(machineName.GetLength() > 0);

		char buf[256];
		strcpy_s(buf, sizeof(buf), machineName);
		pcb->RemapLoadedMachineName(buf, sizeof(buf));		
		machineName = buf;

		pMachine = pcb->GetMachine(machineName);
		CMachineInfo const *pmi = pcb->GetMachineInfo(pMachine);
		numGlobalParameters = pmi->numGlobalParameters;

		if (paramIndex >= 0)
		{
			if (loadedpattern)
			{
				paramIndex = pcb->RemapLoadedMachineParameterIndex(pMachine, paramIndex);
				if (paramIndex < 0) paramIndex = 0;		// TODO: create a 'missing' internal parameter
			}

			pParam = pmi->Parameters[paramIndex];
			paramGroup = paramIndex < pmi->numGlobalParameters ? 0 : 1;
		}
		else
		{
			switch((InternalParameter)paramIndex)
			{
			case MidiNote: pParam = &ipMIDINote; break;
			case MidiVelocity: pParam = &ipMIDIVelocity; break;
			case MidiPitchWheel: pParam = &ipMIDIPitchWheel; break;
			case MidiCC: pParam = &ipMIDICC; break;
			case MidiNoteDelay: pParam = &ipMIDINoteDelay; break;
			case MidiNoteCut: pParam = &ipMIDINoteCut; break;

			case SPGlobalTrigger: pParam = &ipSPGlobalTrigger; break;
			case SPGlobalEffect1: pParam = &ipSPGlobalEffect1; break;
			case SPGlobalEffect1Data: pParam = &ipSPGlobalEffect1Data; break;
			
			case SPTrackTrigger: pParam = &ipSPTrackTrigger; break;
			case SPTrackEffect1: pParam = &ipSPTrackEffect1; break;
			case SPTrackEffect1Data: pParam = &ipSPTrackEffect1Data; break;
			default: pParam = NULL;
			}
			
			paramGroup = paramIndex > FirstInternalTrackParameter ? 0 : 1;
		}

	}

	void MachineRenamed(CMICallbacks *pcb)
	{
		machineName = pcb->GetMachineName(pMachine);
	}

	void Write(CMachineDataOutput * const po)
	{
		po->Write(machineName);
		po->Write(paramIndex);
		po->Write(paramTrack);
		po->Write(graphical);
		po->Write((int)events.size());

		for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
		{
			po->Write((*i).first);
			(*i).second.Write(po);
		}
	}

	void Read(CMachineDataInput * const pi, byte ver)
	{
		machineName = pi->ReadString();
		pi->Read(paramIndex);
		pi->Read(paramTrack);

		if (ver >= 3) 
			pi->Read(graphical);

		int count;
		pi->Read(count);
		
		for (int i = 0; i < count; i++)
		{
			int first;
			CValue second;
			pi->Read(first);
			second.Read(pi, ver);
			events[first] = second;
		}
	}

	bool HasValue(int row) const
	{
		MapIntToValue::const_iterator i = events.find(row);
		return i != events.end();
	}

	int GetValue(int row) const
	{
		MapIntToValue::const_iterator i = events.find(row);
		if (i == events.end())
			return GetNoValue();
		else
			return (*i).second;
	}

	int GetValueClamp(int row, int mini, int maxi) const
	{
		MapIntToValue::const_iterator i = events.find(row);
		if (i == events.end())
			return GetNoValue();
		else
			return min(max((*i).second, mini), maxi);
	}

	void SetValue(int row, int value)
	{
		if (GetParamType() == pt_internal)
			events[row] = value;
		else if (value == GetNoValue())
			ClearValue(row);
		else if (GetParamType() == pt_note)
		{
			if (value == NOTE_OFF)
				events[row] = value;
			else
				events[row] = EncodeNote(min(max(DecodeNote(value), 0), 9 * 12 + 11));
		}
		else if (GetParamType() == pt_switch)
			events[row] = min(max(value, 0), 1);
		else
			events[row] = Clamp(value);
	}

    void SetValueSpecial(int row, int value)
	{
		if (GetParamType() == pt_internal)
			events[row] = value;
		else if (value == GetNoValue())
			ClearValue(row);
		else if (GetParamType() == pt_note)
		{
			if (value == NOTE_OFF){
				if (GetValue(row) == NOTE_NO) {
				  events[row] = NOTE_OFF;
				}			
			}
			else
				events[row] = EncodeNote(min(max(DecodeNote(value), 0), 9 * 12 + 11));
		}
		else if (GetParamType() == pt_switch)
			events[row] = min(max(value, 0), 1);
		else
			events[row] = Clamp(value);
	}
	
		
	double GetValueNormalized(int row)
	{
		if (GetParamType() == pt_internal) return 0;

		double value;

		if (GetParamType() == pt_note)
			value = DecodeNote(GetValue(row)) / (10.0 * 12);
		else if (GetParamType() == pt_switch)
			value = GetValue(row) != 0 ? 1.0 : 0.0;
		else
			value = (double)(GetValue(row) - GetMinValue()) / ((GetMaxValue() - GetMinValue()) + 1);

		return value;
	}

	void SetValueNormalized(int row, double value)
	{
		if (GetParamType() == pt_internal) return;

		value = min(max(value, 0), 0.9999999);

		if (GetParamType() == pt_note)
			SetValue(row, EncodeNote((int)(value * 10 * 12)));
		else if (GetParamType() == pt_switch)
			SetValue(row, value < 0.5 ? 0 : 1);
		else
			SetValue(row, GetMinValue() + (int)(value * ((GetMaxValue() - GetMinValue()) + 1)));

	}

	void SetValueNormalized(int row, double value, int minval, int maxval)
	{
		if (GetParamType() == pt_internal) return;

		value = min(max(value, 0), 0.9999999);

		if (GetParamType() == pt_note)
			SetValue(row, EncodeNote(DecodeNote(minval) + (int)(value * ((DecodeNote(maxval) - DecodeNote(minval)) + 1))));
		else if (GetParamType() == pt_switch)
			SetValue(row, value < 0.5 ? 0 : 1);
		else
			SetValue(row, minval + (int)(value * ((maxval - minval) + 1)));

	}

	void ShiftValue(int row, int delta)
	{
		if (GetParamType() == pt_internal) return;

		int value = GetValue(row);

		if (!IsNormalValue(value))
			return;

		if (GetParamType() == pt_note)
			value = EncodeNote(min(max(DecodeNote(value) + delta, 0), 9 * 12 + 11));
		else if (GetParamType() == pt_switch)
			value = min(max(value + delta, 0), 1);
		else
			value = Clamp(value + delta);

		SetValue(row, value);

	}

	void ClearNoteOff(int row)
	{
		if (GetParamType() == pt_note) {
			int value = GetValue(row);
			if (value == NOTE_OFF){
				ClearValue(row);
				}
		}		
	}
	
	void ClearValue(int row)
	{
		MapIntToValue::iterator i = events.find(row);
		if (i != events.end())
			events.erase(i);
	}

	void Insert(int row, int length)
	{
		MapIntToValue e;

		for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
		{
			if ((*i).first < row)
				e[(*i).first] = (*i).second;
			else if ((*i).first < length - 1)
				e[(*i).first + 1] = (*i).second;
		}

		events = e;
	}

	void Delete(int row)
	{
		MapIntToValue e;

		for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
		{
			if ((*i).first < row)
				e[(*i).first] = (*i).second;
			else if ((*i).first > row)
				e[(*i).first - 1] = (*i).second;
		}

		events = e;
	}

	void Rotate(int row, int length, bool reverse)
	{
		MapIntToValue e;

		if (reverse)
		{
			for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
			{
				if ((*i).first < row || (*i).first >= row + length)
					e[(*i).first] = (*i).second;
				else if ((*i).first > row)
					e[(*i).first - 1] = (*i).second;
				else if ((*i).first == row)
					e[row + length - 1] = (*i).second;
			}
		}
		else
		{
			for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
			{
				if ((*i).first < row || (*i).first >= row + length)
					e[(*i).first] = (*i).second;
				else if ((*i).first < row + length - 1)
					e[(*i).first + 1] = (*i).second;
				else if ((*i).first == row + length - 1)
					e[row] = (*i).second;
			}
		}

		events = e;
	}

	void Trim(int length)
	{
		events.erase(events.lower_bound(length), events.end());
	}

	void GetEventRange(MapIntToInt &out, int firstrow, int lastrow)
	{
		for (MapIntToValue::iterator i = events.lower_bound(firstrow); i != events.upper_bound(lastrow); i++)
			out[(*i).first - firstrow] = (*i).second;

	}

	void Clear()
	{
		events.clear();
	}

	void ClearEventRange(int firstrow, int lastrow)
	{
		events.erase(events.lower_bound(firstrow), events.upper_bound(lastrow));
	}

	void SetEventRange(MapIntToInt const &in, int firstrow, int lastrow, int patlength)
	{
		ClearEventRange(firstrow, lastrow);

		for (MapIntToInt::const_iterator i = in.begin(); i != in.end(); i++)
			if ((*i).first + firstrow < patlength)
				SetValue((*i).first + firstrow, (*i).second);

	}

	void SetEventRangeSpecial(MapIntToInt const &in, int firstrow, int lastrow, int patlength) //BWC
	{
		// Merge both : do not clear first
		// Do not copy 'off' if it override a note
		// ClearEventRange(firstrow, lastrow);

		for (MapIntToInt::const_iterator i = in.begin(); i != in.end(); i++)
			if ((*i).first + firstrow < patlength)
				SetValueSpecial((*i).first + firstrow, (*i).second);
	}
	
		
	void SetRowsPerBeat(int rpb, int oldrpb)
	{
		MapIntToValue e;

		for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
		{
			if ((*i).first * rpb % oldrpb == 0)
				e[(*i).first * rpb / oldrpb] = (*i).second;
		}

		events = e;
	}

	void Import(CPattern *p, int length, CMICallbacks *pcb)
	{
		if (paramIndex < 0)
			return;

		events.clear();

		for (int row = 0; row < length; row++)
		{
			int data = pcb->GetPatternData(p, row, paramGroup + 1, paramTrack, GetIndexInGroup());
//			if (data != pParam->NoValue)
			if (data != GetNoValue())
				events[row] = data;
		}
	}

	MapIntToValue::const_iterator EventsBegin() const { return events.begin(); }
	MapIntToValue::const_iterator EventsBeginAt(int offset) const { return events.lower_bound(offset); }
	MapIntToValue::const_iterator EventsEnd() const { return events.end(); }

	bool MatchMachine(CMachine *pmac) const { return pMachine == pmac; }
	bool MatchMachine(CColumn const &x) const { return pMachine == x.pMachine; }
	bool MatchGroup(CMachine *pmac, int group) const { return pMachine == pmac && paramGroup == group; }
	bool MatchGroup(CColumn const &x) const { return pMachine == x.pMachine && paramGroup == x.paramGroup; }
	bool MatchGroupAndTrack(CMachine *pmac, int group, int track) const { return pMachine == pmac && paramGroup == group && paramTrack == track; }
	bool MatchGroupAndTrack(CColumn const &x) const { return pMachine == x.pMachine && paramGroup == x.paramGroup && paramTrack == x.paramTrack; }

	bool Match(MacIntPair const &mpp) const { return pMachine == mpp.first && paramIndex == mpp.second; }
	bool Match(MacIntPair const &mpp, int track) const { return pMachine == mpp.first && paramIndex == mpp.second && paramTrack == track; }

	bool MatchBuzzParam(CMachine *pmac, int bgroup, int btrack, int bparam)
	{
		return pMachine == pmac && paramGroup == bgroup - 1 && paramTrack == btrack && GetIndexInGroup() == bparam;
	}

	bool IsTrackParam() const { return paramGroup == 1; }
	bool IsTrackParam(CMachine *pmac) { return pMachine == pmac && paramGroup == 1; }
	int GetTrack() const { assert(IsTrackParam()); return paramTrack; }

	CMachine *GetMachine() const { return pMachine; }
	CString GetMachineName() const { return machineName; }

	MacIntPair GetMachineAndTrack() const { assert(IsTrackParam()); return MacIntPair(pMachine, paramTrack); }

	int Clamp(int value) const { return min(max(value, GetMinValue()), GetMaxValue()); }

	void SendControlChanges(CMICallbacks *pcb) { if (paramIndex >= 0) pcb->SendControlChanges(pMachine); }

	bool IsInternal() const { return paramIndex < 0; }
	bool IsMidi() const { return paramIndex <= FirstMidiTrackParameter; }
	bool IsMidiNote() const { return paramIndex == MidiNote; }
	bool IsMidiPW() const { return paramIndex == MidiPitchWheel; }
	bool IsMidiCC() const { return paramIndex == MidiCC; }

public:
	int GetDigitCount() const
	{
		switch(GetParamType())
		{
		case pt_note: return 3;
		case pt_switch: return 1;
		case pt_byte: return IsAscii() ? 1 : 2;
		case pt_word: return 4;
		case pt_internal: return 2;
		default: ASSERT(false); return 0;
		}
	}

	int GetWidth() const
	{
		if (graphical)
			return 10;
		else
			return GetDigitCount() + (IsTiedToNext() ? 0 : 1);
	}
	
	bool IsNormalValue(int value) const
	{
		if (GetParamType() == pt_internal)
			return false;
		else if (value == GetNoValue())
			return false;
		else if (GetParamType() == pt_note && value == NOTE_OFF)
			return false;

		return true;
	}

	void WriteState(CMICallbacks *pcb, int row)
	{
		if (GetParamType() == pt_internal) return;

		assert(pParam != NULL);

		if (!(pParam->Flags & MPF_STATE))
			return;

		SetValue(row, pcb->GetParameterState(pMachine, (paramGroup + 1), paramTrack, GetIndexInGroup()));
	}

	int GetIndex() const { return paramIndex; }

	int GetIndexInGroup() const
	{
		assert(paramIndex >= 0);

		if (paramIndex < numGlobalParameters)
			return paramIndex;
		else
			return paramIndex - numGlobalParameters;

	}

	int GetModulatedValue(int value, int delta)
	{
		if (GetParamType() == pt_internal) return value;

		if (!IsNormalValue(value))
			return value;

		if (GetParamType() == pt_note)
			value = EncodeNote(min(max(DecodeNote(value) + delta, 0), 9 * 12 + 11));	
		else if (GetParamType() == pt_switch)
			value = min(max(value + delta, 0), 1);
		else
			value = Clamp(value + delta);

		return value;
	}

	bool PlayRow(CMICallbacks *pcb, int row, MapTrackIDToSPControl *sptv, int trackoffset, shared_ptr<CModulators> modulators, CGlobalData &gdata)
	{
		MapIntToValue::const_iterator i = events.find(row);
		if (i == events.end())
			return false;

		CTrackID const tid = CTrackID(pMachine, paramGroup, paramTrack);

		if (paramIndex >= 0)
		{
			int mod = 0;

			if (modulators)
			{
				MapIntToInt::const_iterator mit = modulators->paramModulators.find(paramIndex);
				if (mit != modulators->paramModulators.end())
					mod = (*mit).second;
			}

			if (sptv != NULL && sptv->find(tid) != sptv->end())
			{
				int newmod;
				if (GetParamType() == pt_note)
					newmod = DecodeNote((*i).second) - 48;
				else
					newmod = (*i).second;

				(*sptv)[tid]->AddModulator(paramIndex, mod + newmod);
				return false;
			}
			else
			{
				int value = (*i).second;
					
				if (modulators)
				{
					if (GetParamType() == pt_note && IsNormalValue(value))
						value = EncodeNote(min(max(modulators->DiatonicTransposition(DecodeNote(value)), 0), 9 * 12 + 11));

				}

				pcb->ControlChange(pMachine, (paramGroup + 1) | 16, paramTrack + trackoffset, GetIndexInGroup(), GetModulatedValue(value, mod));
				return true;
			}
		}
		else
		{
			InternalParameter ip = (InternalParameter)paramIndex;

			if (ip <= FirstMidiTrackParameter)
			{
				if (ip == MidiNote)
				{
					if ((*i).second == NOTE_OFF)
					{
						auto n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
						if (n != gdata.activeMidiNotes.end())
						{
							(*n).second.cuttime = 0;
							(*n).second.state = ActiveMidiNote::note_off_pending;
						}
					}
					else
					{
						ActiveMidiNote n;
						n.note = DecodeNote((*i).second);
						if (modulators) n.note = min(max(modulators->DiatonicTransposition(n.note), 0), 9 * 12 + 11);
						n.velocity = DefaultMidiVelocity;
						n.delaytime = 0;
						n.cuttime = 0x7fffffff;
						n.pw = n.cc = -1;
						n.state = ActiveMidiNote::note_on_pending;
						n.rowInBeat = gdata.currentRowInBeat;
						n.RPB = gdata.currentRPB;

						auto amn = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
						
						if (amn != gdata.activeMidiNotes.end() && (*amn).second.state > ActiveMidiNote::note_on_pending)
							n.previousNote = (*amn).second.note;
						else
							n.previousNote = -1;

						gdata.activeMidiNotes[MacIntPair(pMachine, paramTrack)] = n;
					}
				}
				else if (ip == MidiVelocity)
				{
					auto n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
					if (n != gdata.activeMidiNotes.end())
					{
						if ((*n).second.state == ActiveMidiNote::note_on_pending)
							(*n).second.velocity = (*i).second;
					}
				}
				else if (ip == MidiPitchWheel)
				{
					auto n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
					if (n == gdata.activeMidiNotes.end())
					{
						gdata.activeMidiNotes[MacIntPair(pMachine, paramTrack)] = ActiveMidiNote();
						n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
						(*n).second.state = ActiveMidiNote::pw_or_cc;
						(*n).second.cc = -1;
						(*n).second.delaytime = 0;
						(*n).second.cuttime = 0x7fffffff;
					}

					(*n).second.rowInBeat = gdata.currentRowInBeat;
					(*n).second.RPB = gdata.currentRPB;
					(*n).second.pw = (*i).second;
				}
				else if (ip == MidiCC)
				{
					auto n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
					if (n == gdata.activeMidiNotes.end())
					{
						gdata.activeMidiNotes[MacIntPair(pMachine, paramTrack)] = ActiveMidiNote();
						n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
						(*n).second.state = ActiveMidiNote::pw_or_cc;
						(*n).second.pw = -1;
						(*n).second.delaytime = 0;
						(*n).second.cuttime = 0x7fffffff;
					}

					(*n).second.rowInBeat = gdata.currentRowInBeat;
					(*n).second.RPB = gdata.currentRPB;
					(*n).second.cc = (*i).second;
				}
				else if (ip == MidiNoteDelay)
				{
					auto n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
					if (n != gdata.activeMidiNotes.end())
					{
						if ((*n).second.state == ActiveMidiNote::note_on_pending || (*n).second.pw >= 0 || (*n).second.cc >= 0)
							(*n).second.delaytime = (*i).second;
						else if ((*n).second.state == ActiveMidiNote::note_off_pending)
							(*n).second.cuttime = (*i).second;
					}
				}
				else if (ip == MidiNoteCut)
				{
					auto n = gdata.activeMidiNotes.find(MacIntPair(pMachine, paramTrack));
					if (n != gdata.activeMidiNotes.end())
					{
						if ((*n).second.state == ActiveMidiNote::note_on_pending)
							(*n).second.cuttime = (*i).second;
					}
				}
			}

			if (sptv != NULL)
			{
				if (ip == SPGlobalTrigger || ip == SPTrackTrigger)
				{
					(*sptv)[tid] = make_shared<CSubPatternControl>((*i).second, paramTrack);
				}
				else
				{
					MapTrackIDToSPControl::iterator ci = sptv->find(tid);
					if (ci != sptv->end())
					{
						if (ip == SPGlobalEffect1 || ip == SPTrackEffect1)
							(*ci).second->AddEffect((*i).second);
						else if (ip == SPGlobalEffect1Data || ip == SPTrackEffect1Data)
							(*ci).second->AddEffectData((*i).second);
					}
				}
				return false;	
			}


		}
		

		return false;
	}



	CMPType GetParamType() const 
	{ 
		assert(pParam != NULL);
		return pParam->Type; 
	}

	int GetRealNoValue() const
	{
		assert(pParam != NULL);
		return pParam->NoValue;
	}

	int GetNoValue() const
	{
		assert(pParam != NULL);

		if (pParam->NoValue < 0)
			return GetMinValue();	
		else
			return pParam->NoValue;
	}

	int GetMinValue() const 
	{ 
		assert(pParam != NULL); 
		return pParam->MinValue; 
	}
	
	int GetMaxValue() const 
	{ 
		assert(pParam != NULL); 
		return pParam->MaxValue; 
	}

	bool IsWaveParameter() const 
	{ 
		assert(pParam != NULL); 
		return (pParam->Flags & MPF_WAVE) != 0; 
	}

	bool IsTiedToNext() const 
	{ 
		assert(pParam != NULL); 
		return (pParam->Flags & MPF_TIE_TO_NEXT) != 0; 
	}

	bool IsAscii() const 
	{ 
		assert(pParam != NULL); 
		return (pParam->Flags & MPF_ASCII) != 0; 
	}

	char const *GetName() const 
	{
		assert(pParam != NULL); 
		return pParam->Name;
	}

	bool NameEqualsIgnoreCase(char const *str) const 
	{
		assert(pParam != NULL); 
		return ::_stricmp(pParam->Name, str) == 0;
	}

	bool NameStartsWithIgnoreCase(char const *str) const 
	{
		assert(pParam != NULL); 
		return ::_strnicmp(pParam->Name, str, strlen(str)) == 0;
	}

	char const *GetDescription() const 
	{
		assert(pParam != NULL); 
		return pParam->Description; 
	}


	char const *DescribeValue(int value, CMICallbacks *pcb)	
	{ 
		if (paramIndex >= 0)
		{
			assert(pParam != NULL); 
			return pcb->DescribeValue(pMachine, paramIndex, value); 
		}
		else
		{
			return NULL;
		}
	}

	void UpdatePatternReferences(MapIntToInt const &remap)
	{
		InternalParameter ip = (InternalParameter)paramIndex;
		if (ip != SPGlobalTrigger && ip != SPTrackTrigger)
			return;

		for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
		{
			int pr = (*i).second;
			MapIntToInt::const_iterator rmi = remap.find(pr);
			if (rmi != remap.end())
				(*i).second = (*rmi).second;
		}

	}

	void UpdateWaveReferences(byte const *remap)
	{
		if (!IsWaveParameter()) return;

		for (MapIntToValue::iterator i = events.begin(); i != events.end(); i++)
		{
			int wr = (*i).second;
			if (wr >= WAVE_MIN && wr <= WAVE_MAX && remap[wr] >= WAVE_MIN && remap[wr] <= WAVE_MAX)
				(*i).second = remap[wr];
		}

	}

	bool IsGraphical() const { return graphical; }
	void ToggleGraphicalMode() { graphical ^= true; }

private:
	CString machineName;

	int paramIndex;
	int paramGroup;
	int paramTrack;

	CMachine *pMachine;
	CMachineParameter const *pParam;
	MapIntToValue events;

	int numGlobalParameters;
	bool graphical;

};

typedef vector<shared_ptr<CColumn>> ColumnVector;

inline int BuzzTicksToBeats(int x)
{
	return (x + BUZZ_TICKS_PER_BEAT - 1) / BUZZ_TICKS_PER_BEAT;
}

class CMachinePattern
{
public:
	CMachinePattern()
	{
		CycleTestFrame = 0;
		pPattern = NULL;
		rowsPerBeat = 4;
	}


	CMachinePattern(CMICallbacks *pcb, CPattern *p, int numrows)
	{
		CycleTestFrame = 0;
		pPattern = p;
		numBeats = BuzzTicksToBeats(numrows);
		rowsPerBeat = 4;
		name = pcb->GetPatternName(p);
	}

	CMachinePattern(CMICallbacks *pcb, CPattern *p, CMachinePattern *pold, int numrows, bool copydata)
	{
		CycleTestFrame = 0;
		pPattern = p;
		rowsPerBeat = pold->rowsPerBeat;
		name = pcb->GetPatternName(p);

		if (copydata)
			numBeats = pold->numBeats;
		else
			numBeats = BuzzTicksToBeats(numrows);

		for (ColumnVector::iterator i = pold->columns.begin(); i != pold->columns.end(); i++)
			columns.push_back(make_shared<CColumn>((*i).get(), copydata));
	}

	void Init(CMICallbacks *pcb, int numrows, bool loadedpattern = false)
	{
		numBeats = BuzzTicksToBeats(numrows);
		name = pcb->GetPatternName(pPattern);

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->Init(pcb, loadedpattern);
	}

	void SetLength(int l, CMICallbacks *pCB)
	{
		MACHINE_LOCK;

		numBeats = BuzzTicksToBeats(l);

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->Trim(GetRowCount());
	}

	void Rename(char const *newname)
	{
		name = newname;
	}

	void Write(CMachineDataOutput * const po)
	{
		po->Write(rowsPerBeat);
		po->Write((int)columns.size());

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->Write(po);
	}

	void Read(CMachineDataInput * const pi, byte ver)
	{
		if (ver > 1)
			pi->Read(rowsPerBeat);

		int count;
		pi->Read(count);

		columns.clear();

		for (int i = 0; i < count; i++)
		{
			auto pc = make_shared<CColumn>();
			pc->Read(pi, ver);
			columns.push_back(pc);
		}
	}

	void DeleteMachine(CMachine *pmac)
	{
		ColumnVector cv;

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
		{
			if ((*i)->MatchMachine(pmac))
				deletedColumns.push_back(*i);
			else
				cv.push_back(*i);
		}

		columns = cv;
	}

	void UndeleteMachine(CMachine *pmac)
	{
		ColumnVector dcv;

		for (ColumnVector::iterator i = deletedColumns.begin(); i != deletedColumns.end(); i++)
		{
			if ((*i)->MatchMachine(pmac))
				columns.push_back(*i);
			else
				dcv.push_back(*i);
		}

		deletedColumns = dcv;
	}


	void RenameMachine(CMachine *pmac, CMICallbacks *pcb)
	{
		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			if ((*i)->MatchMachine(pmac))
				(*i)->MachineRenamed(pcb);

	}

	int GetTrackCount(CMachine *pmac) const
	{
		int ht = -1;

		for (ColumnVector::const_iterator i = columns.begin(); i != columns.end(); i++)
			if ((*i)->IsTrackParam(pmac) && (*i)->GetTrack() > ht)
				ht = (*i)->GetTrack();

		return ht + 1;
	}

	int GetColumnsPerTrack(CMachine *pmac) const
	{
		int c = 0;

		// count the columns of the first track
		for (ColumnVector::const_iterator i = columns.begin(); i != columns.end(); i++)
			if ((*i)->IsTrackParam(pmac) && (*i)->GetTrack() == 0)
				c++;

		return c;
	}

	int GetColumnIndex(CMachine *pmac, int group, int param) const
	{
		int index = 0;
		int pc = 0;

		for (ColumnVector::const_iterator i = columns.begin(); i != columns.end(); i++, index++)
		{
			if ((*i)->MatchGroup(pmac, group))
			{
				if (param == pc++)
					return index;
			}
		}

		return -1;
	}

	bool ColumnsInSameGroupAndTrack(int a, int b) const
	{
		return columns[a]->MatchGroupAndTrack(*columns[b]);
	}

	int GetFirstColumnOfTrackByColumn(int column) const
	{
		for (int c = column - 1; c >= 0; c--)
		{
			if (!ColumnsInSameGroupAndTrack(c, column))
				return c + 1;
		}

		return 0;
	}

	bool ColumnsInSameGroup(int a, int b) const
	{
		return columns[a]->MatchGroup(*columns[b]);
	}

	int GetFirstColumnOfGroupByColumn(int column) const
	{
		for (int c = column - 1; c >= 0; c--)
		{
			if (!ColumnsInSameGroup(c, column))
				return c + 1;
		}

		return 0;
	}

	int GetGroupColumnCount(int column) const
	{
		int fc = GetFirstColumnOfTrackByColumn(column);
		int c;
		for (c = fc; c < (int)columns.size(); c++)
		{
			if (!ColumnsInSameGroupAndTrack(c, fc))
				break;
		}

		return c - fc;
	}

	void AddTrack(CMachine *pmac, CMICallbacks *pcb)
	{
		if (pmac == NULL)
			return;

		int cpt = GetColumnsPerTrack(pmac);
		if (cpt == 0)
			return;

		int tc = GetTrackCount(pmac);
		if (tc < 1)
		{
			ASSERT(false);
			return;
		}

		int lastcoli = GetColumnIndex(pmac, 1, 0) + cpt * (tc - 1);

		for (int i = 0; i < cpt; i++)
		{
			shared_ptr<CColumn> pc = make_shared<CColumn>(columns[lastcoli+i].get(), false, true);
			columns.insert(columns.begin() + lastcoli + cpt + i, pc);
		}

		int nmt = pcb->GetNumTracks(pmac);
		if (nmt < tc + 1)
			pcb->SetNumTracks(pmac, tc + 1);

	}

	void DeleteLastTrack(CMachine *pmac, CMICallbacks *pcb)
	{
		if (pmac == NULL)
			return;

		int cpt = GetColumnsPerTrack(pmac);
		if (cpt == 0)
			return;

		int tc = GetTrackCount(pmac);
		if (tc < 2)
			return;

		int lastcoli = GetColumnIndex(pmac, 1, 0) + cpt * (tc - 1);
		columns.erase(columns.begin() + lastcoli, columns.begin() + lastcoli + cpt);

		pcb->SetNumTracks(pmac, tc - 1);

	}

	shared_ptr<CColumn> GetColumn(MacIntPair const &mpp, int track) const
	{
		for (auto i = columns.begin(); i != columns.end(); i++)
			if ((*i)->Match(mpp, track))
				return (*i);

		return shared_ptr<CColumn>();
	}

	shared_ptr<CColumn> GetColumn(function<bool (shared_ptr<CColumn> const &)> match) const
	{
		for (auto i = columns.begin(); i != columns.end(); i++)
			if (match(*i))
				return (*i);

		return shared_ptr<CColumn>();
	}

	void GetColumns(ColumnVector &cv, MacIntPair const &mpp)
	{
		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			if ((*i)->Match(mpp))
				cv.push_back(*i);
	}

	bool IsGlobalParameter(MacIntPair const &mpp, CMICallbacks *pcb)
	{
		if (mpp.second >= 0)
		{
			CMachineInfo const *pmi = pcb->GetMachineInfo(mpp.first);
			return mpp.second < pmi->numGlobalParameters;
		}
		else
		{
			return mpp.second > FirstInternalTrackParameter;
		}
	}

	shared_ptr<CColumn> CreateColumn(MacIntPair const &mpp, int track, CMICallbacks *pcb)
	{
		return make_shared<CColumn>(pcb, mpp, track);
	}

	void EnableColumns(MacParamPairVector const &_ec, CMICallbacks *pCB, int mintracks = 1, int rpb = -1)
	{
		ColumnVector cv;

		MacParamPairVector ecg;
		MacParamPairVector ect;

		for (MacParamPairVector::const_iterator i = _ec.begin(); i != _ec.end(); i++)
		{
			if (IsGlobalParameter(*i, pCB))
				ecg.push_back(*i);
			else
				ect.push_back(*i);
		}

		for (MacParamPairVector::const_iterator i = ecg.begin(); i != ecg.end(); i++)
		{
			shared_ptr<CColumn> c = GetColumn(*i, 0);
			if (!c) c = CreateColumn(*i, 0, pCB);
			cv.push_back(c);
		}

		for (MacParamPairVector::const_iterator i = ect.begin(); i != ect.end(); i++)
		{
			vector<ColumnVector> tv;

			int maxtracks = mintracks;
			maxtracks = max(maxtracks, GetTrackCount((*i).first));

			MacParamPairVector::const_iterator j = i;
			do
			{
				tv.push_back(ColumnVector());
				GetColumns(tv.back(), *j);
				maxtracks = max(maxtracks, (int)tv.back().size());
				j++;
			} while(j != ect.end() && (*j).first == (*i).first);

			for (int t = 0; t < maxtracks; t++)
			{
				for (int c = 0; c < (int)tv.size(); c++)
				{
					if (tv[c].size() > 0)
						cv.push_back(tv[c][t]);
					else
						cv.push_back(shared_ptr<CColumn>(CreateColumn(MacIntPair(*(i + c)), t, pCB)));
				}
			}

			i = i + tv.size() - 1;
		}

		MACHINE_LOCK;
		columns = cv;

		if (rpb > 0)
			SetRowsPerBeat(rpb);

	}

	void PlayRow(CMICallbacks *pcb, int row, bool immediate, MapTrackIDToSPControl *sptv, int trackoffset, shared_ptr<CModulators> mod, CGlobalData &gdata)
	{
		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
		{
			if (IsTrackMuted((*i).get()))
				continue;
			
			(*i)->PlayRow(pcb, row, sptv, trackoffset, mod, gdata);

			if (immediate)
				(*i)->SendControlChanges(pcb);
		}

	}

	void PlayRow(CMICallbacks *pcb, CColumn *pc, int row, bool immediate, CGlobalData &gdata)
	{
		if (IsTrackMuted(pc))
			return;

		bool sentcc = false;

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
		{
			if ((*i)->MatchGroupAndTrack(*pc))
				sentcc |= (*i)->PlayRow(pcb, row, NULL, 0, shared_ptr<CModulators>(), gdata);
		}

		if (immediate && sentcc)
			pc->SendControlChanges(pcb);

	}

	void SetTargetMachine(CMachine *pmac, CMICallbacks *pcb)
	{
		if (columns.size() > 0)
			return;		// if columns were already copied from previous pattern

		CMachineInfo const *pmi = pcb->GetMachineInfo(pmac);

		bool midinotes = (pmi->Flags & MIF_PREFER_MIDI_NOTES) != 0;

		MacParamPairVector v;

		int n = midinotes ? pmi->numGlobalParameters : (pmi->numGlobalParameters + pmi->numTrackParameters);

		for (int i = 0; i < n; i++)
			v.push_back(MacIntPair(pmac, i));

		if (midinotes)
		{
			v.push_back(MacIntPair(pmac, MidiNote));
			v.push_back(MacIntPair(pmac, MidiVelocity));
			v.push_back(MacIntPair(pmac, MidiNoteDelay));
			v.push_back(MacIntPair(pmac, MidiNoteCut));
		}

		EnableColumns(v, pcb, pcb->GetNumTracks(pmac));
	}

	void SetRowsPerBeat(int rpb)
	{
		if (rpb == rowsPerBeat)
			return;

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->SetRowsPerBeat(rpb, rowsPerBeat);

		for (ColumnVector::iterator i = deletedColumns.begin(); i != deletedColumns.end(); i++)
			(*i)->SetRowsPerBeat(rpb, rowsPerBeat);

		rowsPerBeat = rpb;
	}

	int GetRowCount() const { return numBeats * rowsPerBeat; }

	void Import(CMICallbacks *pcb)
	{
		SetRowsPerBeat(4);

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->Import(pPattern, GetRowCount(), pcb);
	}

	bool IsTrackMuted(CColumn *pc) const
	{
		return pc->IsTrackParam() && mutedTracks.find(pc->GetMachineAndTrack()) != mutedTracks.end();
	}

	void ToggleTrackMute(CColumn *pc)
	{
		if (IsTrackMuted(pc))
			mutedTracks.erase(pc->GetMachineAndTrack());
		else
			mutedTracks.insert(pc->GetMachineAndTrack());
	}
	
	void UpdatePatternReferences(MapIntToInt const &remap)
	{
		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->UpdatePatternReferences(remap);

		for (ColumnVector::iterator i = deletedColumns.begin(); i != deletedColumns.end(); i++)
			(*i)->UpdatePatternReferences(remap);
	}

	void UpdateWaveReferences(byte const *remap)
	{
		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			(*i)->UpdateWaveReferences(remap);

		for (ColumnVector::iterator i = deletedColumns.begin(); i != deletedColumns.end(); i++)
			(*i)->UpdateWaveReferences(remap);
	}

	int AllocateMidiTrack(CMachine *pmac, int row, int delay, MapMacAndTrackToActiveMidiNote &notes, int tc)
	{
		for (int t = 0; t < tc; t++)
		{
			MapMacAndTrackToActiveMidiNote::iterator i;
			
			for (i = notes.begin(); i != notes.end(); i++)
				if ((*i).first.second == t) break;

			if (i == notes.end())
			{
				auto pwcol = GetColumn(MacIntPair(pmac, MidiPitchWheel), t).get();
				if (pwcol != NULL && pwcol->HasValue(row)) continue;
				auto cccol = GetColumn(MacIntPair(pmac, MidiCC), t).get();
				if (cccol != NULL && cccol->HasValue(row)) continue;

				auto notecol = GetColumn(MacIntPair(pmac, MidiNote), t).get();
				if (notecol != NULL)
				{
					if (notecol->HasValue(row)) 
					{
						if (delay == 0 && notecol->GetValue(row) == NOTE_OFF)
						{
							// note off can be overwritten except when it's delayed or cut

							auto delaycol = GetColumn(MacIntPair(pmac, MidiNoteDelay), t).get();
							auto cutcol = GetColumn(MacIntPair(pmac, MidiNoteCut), t).get();
						
							bool dval = delaycol != NULL && delaycol->HasValue(row);
							bool cval = cutcol != NULL && cutcol->HasValue(row);

							if (!dval && !cval) return t;
						}
					}
					else
					{
						return t;
					}
				}
			}
		}

		return tc > 0 ? 0 : -1;
	}

	int FreeMidiTrack(CMachine *pmac, int note, MapMacAndTrackToActiveMidiNote &notes)
	{
		for (auto i = notes.begin(); i != notes.end(); i++)
		{
			if ((*i).first.first == pmac && (*i).second.note == note)
			{
				int t = (*i).first.second;
				notes.erase(i);
				return t;
			}
		}
	
		return -1;
	}


	void RecordMidiNoteOns(int row, CRecQueue::Event const &e, CGlobalData &gdata)
	{
		assert(e.value > 0);

		if (!HasMidiNoteColumn()) return;
		int tc = GetTrackCount(e.pmac);
		if (tc < 1) return;

		auto range = GetRowSubtickRange(row % gdata.currentRPB, gdata.currentRPB, gdata.subticksPerTick);
		int delay = min(max((gdata.currentSubtick - range.first) * 96 / (range.second - range.first), 0), 95);

		int rectrack = AllocateMidiTrack(e.pmac, row, delay, gdata.recordingMidiNotes, tc);
		if (rectrack < 0) return;

		auto notecol = GetColumn(MacIntPair(e.pmac, MidiNote), rectrack).get();
		if (notecol == NULL) return;

		notecol->SetValue(row, EncodeNote(e.param));

		auto velcol = GetColumn(MacIntPair(e.pmac, MidiVelocity), rectrack).get();
		if (velcol != NULL)	velcol->SetValue(row, e.value);

		auto delaycol = GetColumn(MacIntPair(e.pmac, MidiNoteDelay), rectrack).get();
		if (delaycol != NULL && delay != 0) delaycol->SetValue(row, delay);

		ActiveMidiNote n;
		n.note = e.param;
		n.velocity = e.value;
		n.state = ActiveMidiNote::recording;
		gdata.recordingMidiNotes[MacIntPair(e.pmac, rectrack)] = n;

	}

	void RecordMidiNoteOffs(int row, CRecQueue::Event const &e, CGlobalData &gdata)
	{
		assert(e.value == 0);

		if (!HasMidiNoteColumn()) return;
		int tc = GetTrackCount(e.pmac);
		if (tc < 1) return;

		int rectrack = FreeMidiTrack(e.pmac, e.param, gdata.recordingMidiNotes);
		if (rectrack < 0) return;

		auto notecol = GetColumn(MacIntPair(e.pmac, MidiNote), rectrack).get();
		if (notecol == NULL) return;

		if (notecol->HasValue(row))
		{
			auto cutcol = GetColumn(MacIntPair(e.pmac, MidiNoteCut), rectrack).get();
			if (cutcol != NULL)
			{
				auto range = GetRowSubtickRange(row % gdata.currentRPB, gdata.currentRPB, gdata.subticksPerTick);
				int delay = min(max((gdata.currentSubtick - range.first) * 96 / (range.second - range.first), 0), 95);
				if (delay != 0) cutcol->SetValue(row, delay);
			}

		}
		else
		{
			notecol->SetValue(row, NOTE_OFF);

			auto delaycol = GetColumn(MacIntPair(e.pmac, MidiNoteDelay), rectrack).get();
			if (delaycol != NULL)
			{
				auto range = GetRowSubtickRange(row % gdata.currentRPB, gdata.currentRPB, gdata.subticksPerTick);
				int delay = min(max((gdata.currentSubtick - range.first) * 96 / (range.second - range.first), 0), 95);
				if (delay != 0) delaycol->SetValue(row, delay);
			}
		}

	}

	int AllocateMidiCCTrack(CMachine *pmac, int row, int delay, MapMacAndTrackToActiveMidiNote &notes, int tc)
	{
		for (int t = 0; t < tc; t++)
		{
			MapMacAndTrackToActiveMidiNote::iterator i;
			
			for (i = notes.begin(); i != notes.end(); i++)
				if ((*i).first.second == t) break;

			if (i == notes.end())
			{
				auto notecol = GetColumn(MacIntPair(pmac, MidiNote), t).get();
				auto pwcol = GetColumn(MacIntPair(pmac, MidiPitchWheel), t).get();
				auto cccol = GetColumn(MacIntPair(pmac, MidiCC), t).get();
				auto delaycol = GetColumn(MacIntPair(pmac, MidiNoteDelay), t).get();
				auto cutcol = GetColumn(MacIntPair(pmac, MidiNoteCut), t).get();

				bool gotstuff = notecol != NULL && notecol->HasValue(row);
				gotstuff |= pwcol != NULL && pwcol->HasValue(row);
				gotstuff |= cccol != NULL && cccol->HasValue(row);
				gotstuff |= cutcol != NULL && cutcol->HasValue(row);

				if (!gotstuff) return t;
			}
		}

		return tc > 0 ? 0 : -1;
	}

	void RecordMidiCCs(int row, CRecQueue::Event const &e, CGlobalData &gdata)
	{
		int tc = GetTrackCount(e.pmac);
		if (tc < 1) return;

		if (e.param == 255)
		{
			if (!HasMidiPWColumn()) return;
		}
		else if (e.param >= 0 && e.param <= 127)
		{
			if (!HasMidiCCColumn()) return;
		}
		else
		{
			return;
		}


		auto range = GetRowSubtickRange(row % gdata.currentRPB, gdata.currentRPB, gdata.subticksPerTick);
		int delay = min(max((gdata.currentSubtick - range.first) * 96 / (range.second - range.first), 0), 95);

		int rectrack = AllocateMidiCCTrack(e.pmac, row, delay, gdata.recordingMidiNotes, tc);
		if (rectrack < 0) return;

		if (e.param == 255)
		{
			auto pwcol = GetColumn(MacIntPair(e.pmac, MidiPitchWheel), rectrack).get();
			if (pwcol == NULL) return;
			pwcol->SetValue(row, e.value);
		}
		else
		{
			auto cccol = GetColumn(MacIntPair(e.pmac, MidiCC), rectrack).get();
			if (cccol == NULL) return;
			cccol->SetValue(row, (e.param << 8) | e.value);
		}

		auto delaycol = GetColumn(MacIntPair(e.pmac, MidiNoteDelay), rectrack).get();
		if (delaycol != NULL && delay != 0) delaycol->SetValue(row, delay);


	}


	void Record(int row, CRecQueue &rq, CGlobalData &gdata)
	{
		if (rq.IsEmpty())
			return;

		vector<CRecQueue::Event> ev;
		rq.Pop(ev);

		for (int ei = 0; ei < (int)ev.size(); ei++)
		{
			CRecQueue::Event const &e = ev[ei];
			if (e.group == -1 && e.value > 0)	RecordMidiNoteOns(row, e, gdata);
		}

		for (int ei = 0; ei < (int)ev.size(); ei++)
		{
			CRecQueue::Event const &e = ev[ei];
			if (e.group == -1 && e.value == 0) RecordMidiNoteOffs(row, e, gdata);
		}

		for (int ei = 0; ei < (int)ev.size(); ei++)
		{
			CRecQueue::Event const &e = ev[ei];
			if (e.group == -2) RecordMidiCCs(row, e, gdata);
		}


		for (int ei = 0; ei < (int)ev.size(); ei++)
		{
			CRecQueue::Event const &e = ev[ei];

			for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
			{
				if ((*i)->MatchBuzzParam(e.pmac, e.group, e.track, e.param))
					(*i)->SetValue(row, e.value);
			}
		}
	}

	void SetValue(int row, CMachine *pmac, int group, int track, int param, int value)
	{
		if (row < 0 || row >= GetRowCount())
			return;

		for (ColumnVector::iterator i = columns.begin(); i != columns.end(); i++)
		{
			if ((*i)->MatchBuzzParam(pmac, group, track, param))
				(*i)->SetValue(row, value);
		}
	}

	bool HasMidiNoteColumn() const
	{
		for (auto i = columns.begin(); i != columns.end(); i++)
			if ((*i)->IsMidiNote())
				return true;

		return false;
	}

	bool HasMidiPWColumn() const
	{
		for (auto i = columns.begin(); i != columns.end(); i++)
			if ((*i)->IsMidiPW())
				return true;

		return false;
	}

	bool HasMidiCCColumn() const
	{
		for (auto i = columns.begin(); i != columns.end(); i++)
			if ((*i)->IsMidiCC())
				return true;

		return false;
	}

	int ScaleToMidiTime(int row, int delay, int range) const
	{
		return (range * row + delay) * 960 * BUZZ_TICKS_PER_BEAT / (range * rowsPerBeat);
	}

	bool ExportMidiEvents(CMachineDataOutput *pout, CMachine *pmac) const
	{
		// if (!HasMidiNoteColumn()) return false;
		int tc = GetTrackCount(pmac);
		if (tc < 1) return false;

		vector<int> notes(tc);
		vector<CColumn *> notecol(tc);
		vector<CColumn *> velcol(tc);
		vector<CColumn *> delaycol(tc);
		vector<CColumn *> cutcol(tc);

		for (int i = 0; i < tc; i++) 
		{
			notes[i] = -1;
			notecol[i] = GetColumn(MacIntPair(pmac, MidiNote), i).get();
			if (notecol[i] == NULL)
			{
				notecol[i] = GetColumn([=] (shared_ptr<CColumn> const &c)
				{ 
					return c->GetParamType() == pt_note && c->IsTrackParam() && c->GetTrack() == i && c->NameEqualsIgnoreCase("note"); 
				}).get();
				if (notecol[i] == NULL) return false;
			}

			velcol[i] = GetColumn(MacIntPair(pmac, MidiVelocity), i).get();
			if (velcol[i] == NULL)
			{
				velcol[i] = GetColumn([=] (shared_ptr<CColumn> const &c)
				{ 
					return c->GetParamType() == pt_byte && c->IsTrackParam() && c->GetTrack() == i && (c->NameEqualsIgnoreCase("note velocity") || c->NameEqualsIgnoreCase("velocity")); 
				}).get();
			}

			delaycol[i] = GetColumn(MacIntPair(pmac, MidiNoteDelay), i).get();
			if (delaycol[i] == NULL)
			{
				delaycol[i] = GetColumn([=] (shared_ptr<CColumn> const &c)
				{ 
					return c->GetParamType() == pt_byte && c->IsTrackParam() && c->GetTrack() == i && c->NameEqualsIgnoreCase("note delay"); 
				}).get();
			}

			cutcol[i] = GetColumn(MacIntPair(pmac, MidiNoteCut), i).get();
			if (cutcol[i] == NULL)
			{
				cutcol[i] = GetColumn([=] (shared_ptr<CColumn> const &c)
				{ 
					return c->GetParamType() == pt_byte && c->IsTrackParam() && c->GetTrack() == i && c->NameEqualsIgnoreCase("note cut"); 
				}).get();
			}
		}

		vector<pair<int, int>> events;

		for (int row = 0; row < GetRowCount(); row++)
		{
			for (int t = 0; t < tc; t++)
			{
				int const delayrange = (delaycol[t] != NULL) ? (delaycol[t]->GetMaxValue() + 1) : 96;
				int const cutrange = (cutcol[t] != NULL) ? (cutcol[t]->GetMaxValue() + 1) : 96;

				int note = -1;
				int velocity = DefaultMidiVelocity;
				int delay = 0;
				int cut = cutrange;

				if (notecol[t] != NULL && notecol[t]->HasValue(row)) note = notecol[t]->GetValue(row);
				if (velcol[t] != NULL && velcol[t]->HasValue(row)) velocity =  velcol[t]->GetValueClamp(row, 1, 127);
				if (delaycol[t] != NULL && delaycol[t]->HasValue(row)) delay = delaycol[t]->GetValue(row);
				if (cutcol[t] != NULL && cutcol[t]->HasValue(row)) cut = cutcol[t]->GetValue(row);

				if (note >= 0 && note != NOTE_OFF && delay <= cut)
				{
					// note on is valid only if delay <= cut

					if (notes[t] >= 0)
					{
						// old note off
						events.push_back(pair<int, int>(
							ScaleToMidiTime(row, delay, delayrange),
							EncodeMidiNoteOff(0, DecodeNote(notes[t]))));
						notes[t] = -1;
					}

					// new note on
					events.push_back(pair<int, int>(
						ScaleToMidiTime(row, delay, delayrange),
						EncodeMidiNoteOn(0, DecodeNote(note), velocity)));
					
					notes[t] = note;

					if (cut < cutrange)
					{
						// new note cut
						events.push_back(pair<int, int>(
							ScaleToMidiTime(row, cut, cutrange),
							EncodeMidiNoteOff(0, DecodeNote(note))));
						notes[t] = -1;
					}
				}
				else if (note >= 0 && note == NOTE_OFF && notes[t] >= 0)
				{
					events.push_back(pair<int, int>(
						ScaleToMidiTime(row, min(delay, cut), delayrange),
						EncodeMidiNoteOff(0, DecodeNote(notes[t]))));
					notes[t] = -1;
				}



			}


		}

		if (events.size() == 0) return false;

		std::stable_sort(events.begin(), events.end(), [](pair<int, int> x, pair<int, int> y) { return x.first < y.first; });

		for (auto i = events.begin(); i != events.end(); i++)
		{
			pout->Write((*i).first);
			pout->Write((*i).second);
		}

		pout->Write(-1);

		return true;
	}

	bool ImportMidiEvents(CMachineDataInput * const pi, CMachine *pmac, CMICallbacks *pCB)
	{
		if (!HasMidiNoteColumn()) return false;


		int const tc = GetTrackCount(pmac);
		if (tc < 1) return false;

		vector<CColumn *> notecol(tc);
		vector<CColumn *> velcol(tc);
		vector<CColumn *> delaycol(tc);
		vector<CColumn *> cutcol(tc);

		for (int t = 0; t < tc; t++)
		{
			notecol[t] = GetColumn(MacIntPair(pmac, MidiNote), t).get();
			velcol[t] = GetColumn(MacIntPair(pmac, MidiVelocity), t).get();
			delaycol[t] = GetColumn(MacIntPair(pmac, MidiNoteDelay), t).get();
			cutcol[t] = GetColumn(MacIntPair(pmac, MidiNoteCut), t).get();
		}

		MACHINE_LOCK;

		for (int t = 0; t < tc; t++)
		{
			if (notecol[t] != NULL) notecol[t]->Clear();
			if (velcol[t] != NULL) velcol[t]->Clear();
			if (delaycol[t] != NULL) delaycol[t]->Clear();
			if (cutcol[t] != NULL) cutcol[t]->Clear();
		}

		MapMacAndTrackToActiveMidiNote notes;

		while(true)
		{
			int time;
			pi->Read(time);
			if (time < 0) break;

			time = time * rowsPerBeat / (10 * BUZZ_TICKS_PER_BEAT);

			int mididata;
			pi->Read(mididata);

			int row = time / 96;
			int delay = time % 96;

			if (row >= GetRowCount()) continue;

			int status = mididata & 0xff;
			int data1 = (mididata >> 8) & 0xff;
			int data2 = (mididata >> 16) & 0xff;

			if (status == 0x90)
			{
				int t = AllocateMidiTrack(pmac, row, delay, notes, tc);
				if (t < 0) continue;

				if (notecol[t] == NULL) continue;

				notecol[t]->SetValue(row, EncodeNote(data1));

				if (velcol[t] != NULL) velcol[t]->SetValue(row, data2);
				if (delaycol[t] != NULL && delay != 0) delaycol[t]->SetValue(row, delay);

				ActiveMidiNote n;
				n.note = data1;
				n.velocity = data2;
				n.state = ActiveMidiNote::recording;
				notes[MacIntPair(pmac, t)] = n;


			}
			else if (status == 0x80)
			{
				int t = FreeMidiTrack(pmac, data1, notes);
				if (t < 0) continue;

				if (notecol[t] == NULL) continue;

				if (notecol[t]->HasValue(row))
				{
					if (cutcol[t] != NULL && delay != 0) cutcol[t]->SetValue(row, delay);

				}
				else
				{
					notecol[t]->SetValue(row, NOTE_OFF);
					if (delaycol[t] != NULL && delay != 0) delaycol[t]->SetValue(row, delay);
				}


			}
			

		}


		return false;
	}


public:
	CPattern *pPattern;
	int numBeats;
	int rowsPerBeat;

	ColumnVector columns;
	ColumnVector deletedColumns;

	MacTrackSet mutedTracks;

	CString name;
	CActionStack actions;

	int CycleTestFrame;
	bool CycleTestFlag;

};

typedef map<CPattern *, shared_ptr<CMachinePattern>> MapPatternToMachinePattern;
typedef map<CString, shared_ptr<CMachinePattern>> MapStringToMachinePattern;

class CPlayingPattern;
typedef map<int, shared_ptr<CPlayingPattern>> MapIntToPlayingPattern;
typedef map<CTrackID, shared_ptr<CPlayingPattern>> MapTrackIDToPlayingPattern;




class CPlayingPattern
{
private:
	CPlayingPattern() {}

public:
	CPlayingPattern(CMachinePattern *p, CSequence *s, int pos, int trackofs, shared_ptr<CModulators> mod)
	{
		ppat = p;
		pseq = s;
		position = pos;
		currentRow = 0;
		trackOffset = trackofs;
		modulators = mod;
		playpos = 0;
		lastRPB = lastSTPT = 0;
		loop = false;
		reverse = false;
		offset = 0;
	}

	bool Play(CSubTickInfo const *psti, CGlobalData &gdata, int ctf, CRecQueue *prq, CMICallbacks *pcb)
	{
		if (ppat->CycleTestFrame != ctf) 
		{
			ppat->CycleTestFrame = ctf;
			ppat->CycleTestFlag = false;
		}
		else
		{
			if (ppat->CycleTestFlag)
				return true;
		}

		ppat->CycleTestFlag = true;
		
		if (psti == NULL)
			position++;
	
		int lengthInBuzzTicks = ppat->numBeats * BUZZ_TICKS_PER_BEAT;

		if (position >= lengthInBuzzTicks)
		{
			if (loop)
			{
				position = 0;
			}
			else
			{
				ppat->CycleTestFlag = false;
				return false;
			}
		}

		if (reverse)
			playpos = (lengthInBuzzTicks - GetSubTickPosition(psti)) / BUZZ_TICKS_PER_BEAT * ppat->rowsPerBeat - offset;
		else
			playpos = GetSubTickPosition(psti) / BUZZ_TICKS_PER_BEAT * ppat->rowsPerBeat + offset;

		UpdateMap(psti);

		MapTrackIDToSPControl sptv;

		int const rfm = GetRowInBeat(psti);
		if (rfm >= 0)
		{
			gdata.currentRPB = ppat->rowsPerBeat;
			gdata.currentRowInBeat = rfm;

			currentRow = position / BUZZ_TICKS_PER_BEAT * ppat->rowsPerBeat + rfm;
			if (reverse)
				currentRow = ppat->GetRowCount() - 1 - currentRow;

			currentRow += offset;
			if (currentRow < ppat->GetRowCount())
			{
				ppat->PlayRow(pcb, currentRow, true, &sptv, trackOffset, modulators, gdata);
			}
			else
			{
				ppat->CycleTestFlag = false;
				return false;
			}
		}

		if (prq != NULL) 
			ppat->Record(currentRow, *prq, gdata);

		PlaySubPatterns(sptv, psti, gdata, ctf, pcb);

		ppat->CycleTestFlag = false;
		return true;
	}

	CPlayingPattern *GetFirstPlayingPattern(CMachinePattern *p)
	{
		if (ppat == p)
			return this;

		for (MapTrackIDToPlayingPattern::iterator i = playingSubPatterns.begin(); i != playingSubPatterns.end(); i++)
		{
			CPlayingPattern *pp = (*i).second->GetFirstPlayingPattern(p);
			if (pp != NULL)
				return pp;
		}

		return NULL;
	}

	void Stop(CMachinePattern *p)
	{
		for (MapTrackIDToPlayingPattern::iterator i = playingSubPatterns.begin(); i != playingSubPatterns.end();)
		{
			MapTrackIDToPlayingPattern::iterator t = i++;
			if ((*t).second->ppat == p)
				playingSubPatterns.erase(t);
			else
				(*t).second->Stop(p);
		}
	}

	float GetPlayPos() const
	{
		return playpos;
	}
	


private:
	void PlaySubPatterns(MapTrackIDToSPControl &sptv, CSubTickInfo const *psti, CGlobalData &gdata, int ctf, CMICallbacks *pcb)
	{
		for (MapTrackIDToSPControl::iterator i = sptv.begin(); i != sptv.end(); i++)
		{
			int spi = (*i).second->subPatternIndex;
			int tofs = this->trackOffset + (*i).second->trackOffset;
			if (spi >= 0 && spi < (int)gdata.patternList.size())
			{
				auto p = make_shared<CPlayingPattern>(gdata.patternList[spi], nullptr, -1, tofs, (*i).second->modulators); 

				for (int ei = 0; ei < min((*i).second->effectCount, (*i).second->effectDataCount); ei++)
					p->ProcessEffect((*i).second->effects[ei], (*i).second->effectData[ei]);

				playingSubPatterns[(*i).first] = p;
			}
		}

		for (MapTrackIDToPlayingPattern::iterator i = playingSubPatterns.begin(); i != playingSubPatterns.end();)
		{
			MapTrackIDToPlayingPattern::iterator t = i++;
			if (!(*t).second->Play(psti, gdata, ctf, NULL, pcb))
				playingSubPatterns.erase(t);
		}

	}

	void ProcessEffect(int effect, int data)
	{
		switch(effect)
		{
		case EFFECT_PLAY_MODE:

			switch(data)
			{
			case 0x01: loop = true; break;
			case 0x02: reverse = true; break;
			case 0x03: loop = true; reverse = true; break;
			}

			break;

		case EFFECT_OFFSET:
			offset = data;
			break;

		case EFFECT_DIATONIC_TRANSPOSE_UP:
			modulators->diatonicTransposeKey = (data >> 4) % 12;
			modulators->diatonicTransposeAmount = data & 15;
			break;

		case EFFECT_DIATONIC_TRANSPOSE_DOWN:
			modulators->diatonicTransposeKey = (data >> 4) % 12;
			modulators->diatonicTransposeAmount = -(data & 15);
			break;

		}
	}

	void UpdateMap(CSubTickInfo const *psti)
	{
		if (psti == NULL)
			return;

		if (psti->SubTicksPerTick == lastSTPT && ppat->rowsPerBeat == lastRPB)
			return;

		lastSTPT = psti->SubTicksPerTick;
		lastRPB = ppat->rowsPerBeat;

		map.clear();

		for (int row = 0; row < ppat->rowsPerBeat; row++)
		{
			double x = BUZZ_TICKS_PER_BEAT * row / (double)ppat->rowsPerBeat;
			int tick = (int)x;
			double subtick = (x - tick) * psti->SubTicksPerTick;
			int isubtick = (int)(subtick + 0.5);	// it's possible that this gives isubtick == STPT but it doesn't matter here.
			map[(tick << 16) | isubtick] = row;
		}

	}

	float GetSubTickPosition(CSubTickInfo const *psti)
	{
		if (psti == NULL)
			return (float)position;
		else
			return position + (float)psti->CurrentSubTick / psti->SubTicksPerTick;
	}

	int GetRowInBeat(CSubTickInfo const *psti)
	{
		if (psti == NULL)
		{
			for (int row = 0; row < ppat->rowsPerBeat; row++)
			{
				double x = BUZZ_TICKS_PER_BEAT * row / (double)ppat->rowsPerBeat;
				int tick = (int)x;
				if ((position % BUZZ_TICKS_PER_BEAT) == tick && tick == x)
					return row;
			}

			return -1;
		}
		else
		{
			MapIntToInt::iterator i = map.find(((position % BUZZ_TICKS_PER_BEAT) << 16) | psti->CurrentSubTick);
			if (i != map.end())
				return (*i).second;
			else
				return -1;
		}
	}


public:
	CMachinePattern *ppat;
	CSequence *pseq;

private:
	int position;
	int currentRow;
	float playpos;
	int trackOffset;

	int lastRPB;
	int lastSTPT;
	
	MapIntToInt map;
	MapTrackIDToPlayingPattern playingSubPatterns;
	shared_ptr<CModulators> modulators;

	int offset;

	bool loop;
	bool reverse;

};

