
#include "action.h"

#include "telephony.h"

RegisterAction::RegisterAction(const PString & registrar, const PString & user, const PString & passwd) :
	Action(ACTION_REGISTER),
	registrar(registrar),
	user(user),
	passwd(passwd)
	{ }

DialAction::DialAction(const Pstring & dest) : dest(dest) { }

DialAction::Do(TelephonyIfc * phone) {
	phone.Dial(dest);
}

HoldAction::Do(TelephonyIfc * phone) {
	phone.
