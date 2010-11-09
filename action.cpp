
#include "action.h"

#include "telephony.h"

RegisterAction::RegisterAction(const PString & registrar, const PString & user, const PString & passwd) :
	Action(ACTION_REGISTER),
	registrar(registrar),
	user(user),
	passwd(passwd)
	{ }

void RegisterAction::Do(TelephonyIfc & phone ) {
	phone.Register(registrar, user, passwd);
}
