#include <ptlib.h>
#include "state.h"

State::State(StateID id, int turn) :
	id(id),
	turn(turn), 
	status(STATUS_UNSPECIFIED),
	message("")
{
}

State::State(StateID id, int turn, StatusID status) :
	id(id),
	turn(turn), 
	status(status),
	message("")
{
}

State::State(StateID id, int turn, StatusID status, const char * msg) :
	id(id),
	turn(turn), 
	status(status),
	message(msg)
{
}

State::State(StateID id, int turn, StatusID status, const PString & msg) :
	id(id),
	turn(turn), 
	status(status),
	message(msg)
{
}

State * State::Clone() const
{
	State * obj = new State(id, turn, status, message);
	return obj;
}