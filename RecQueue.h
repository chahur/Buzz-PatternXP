#pragma once

class CMachine;

class CRecQueue
{
public:
	struct Event
	{
		Event(CMachine *pmac, int group, int track, int param, int value)
		{
			this->pmac = pmac;
			this->group = group;
			this->track = track;
			this->param = param;
			this->value = value;
		}

		CMachine *pmac;
		int group;
		int track;
		int param;
		int value;

	};

	void Push(Event const &e)
	{
		cs.Lock();
		queue.push(e);
		cs.Unlock();
	}

	void Pop(vector<Event> &ev)
	{
		cs.Lock();

		while (!queue.empty())
		{
			ev.push_back(queue.front());
			queue.pop();
		}

		cs.Unlock();
	}

	bool IsEmpty() const { return queue.empty(); }

private:
	queue<Event> queue;
	CCriticalSection cs;
};