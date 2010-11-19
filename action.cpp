#include <iostream>
#include <ptlib.h>
#include "action.h"

using namespace std;

RegisterAction::RegisterAction(const PString & registrar, const PString & user, const PString & password, int turn) :
	Action(ACTION_REGISTER, turn),
	registrar(registrar),
	user(user),
	password(password)
{ }
