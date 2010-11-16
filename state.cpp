#include "state.h"

State::State(int id, int turn):id(id), turn(turn)
{
}

State * State::Clone() const
{
	State * obj = new State(id, turn);
	return obj;
}