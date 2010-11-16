
#ifndef _STATE_H_
#define _STATE_H_

class State
{
	public:
		const int turn;
		const int id;
		State(int id, int turn);
		virtual State * Clone() const;
};

#endif //_STATE_H
