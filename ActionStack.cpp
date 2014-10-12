#include "StdAfx.h"
#include "ActionStack.h"
#include "EditorWnd.h"

CActionStack::CActionStack(void)
{
	position = 0;
}

CActionStack::~CActionStack(void)
{
}

class CMemDataOutput : public CMachineDataOutput
{
public:
	CMemDataOutput(ByteVector *p) { pdata = p; }

	virtual void Write(void *pbuf, int const numbytes)
	{
		int oldsize = (int)pdata->size();
		pdata->resize(oldsize + numbytes);
		memcpy(&(*pdata)[oldsize], pbuf, numbytes);		// try to avoid: &pdata[oldsize]
	}

	ByteVector *pdata;
};

class CMemDataInput : public CMachineDataInput
{
public:
	CMemDataInput(ByteVector *p) { pdata = p; pos = 0;}

	virtual void Read(void *pbuf, int const numbytes)
	{
		ASSERT(pos + numbytes <= (int)pdata->size());
		memcpy(pbuf, &(*pdata)[pos], numbytes);
		pos += numbytes;
	}

	ByteVector *pdata;
	int pos;
};


void CActionStack::BeginAction(CEditorWnd *pew, char const *name)
{
	shared_ptr<CState> s = shared_ptr<CState>(new CState());
	s->name = name;
	SaveState(pew, *s);
	states.resize(position++);
	states.push_back(s);
	pew->pCB->SetModifiedFlag();
}

void CActionStack::Undo(CEditorWnd *pew)
{
	if (position < 1)
		return;

	if (position == (int)states.size())
		SaveState(pew, unmodifiedState);

	position--;
	RestoreState(pew);
}

void CActionStack::Redo(CEditorWnd *pew)
{
	if (position >= (int)states.size())
		return;

	position++;
	RestoreState(pew);
}

void CActionStack::SaveState(CEditorWnd *pew, CState &s)
{
	s.state.clear();
	CMemDataOutput mdo(&s.state);
	pew->pPattern->Write(&mdo);
	s.patternLength = pew->pCB->GetPatternLength(pew->pPattern->pPattern);
	s.cursorpos = pew->pe.cursor;
	s.scrollpos = pew->pe.GetScrollPos();
}

void CActionStack::RestoreState(CEditorWnd *pew)
{
	CState *ps;

	if (position == (int)states.size())
		ps = &unmodifiedState;
	else
		ps = states[position].get();

	MACHINE_LOCK_PCB(pew->pCB);
	{
		CMemDataInput mdi(&ps->state);
		pew->pPattern->Read(&mdi, PATTERNXP_DATA_VERSION);
		pew->pPattern->Init(pew->pCB, ps->patternLength);
		pew->pe.cursor = ps->cursorpos;
		pew->pCB->SetPatternLength(pew->pPattern->pPattern, ps->patternLength);
		pew->pe.ScrollTo(ps->scrollpos);

	}
}
