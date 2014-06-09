#pragma once

#include "CursorPos.h"

typedef vector<byte> ByteVector;

class CState
{
public:
	CString name;
	ByteVector state;
	int patternLength;
	CCursorPos cursorpos;
};

typedef vector<shared_ptr<CState>> StateVector;

class CEditorWnd;

class CActionStack
{
public:
	CActionStack();
	~CActionStack();

	void BeginAction(CEditorWnd *pew, char const *name);

	void Undo(CEditorWnd *pew);
	void Redo(CEditorWnd *pew);

	bool CanUndo() const { return position > 0; }
	bool CanRedo() const { return position < (int)states.size(); }

private:
	void RestoreState(CEditorWnd *pew);
	void SaveState(CEditorWnd *pew, CState &s);

private:
	int position;
	StateVector states;
	CState unmodifiedState;

};
