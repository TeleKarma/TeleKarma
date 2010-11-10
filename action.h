
#ifndef _ACTION_H_
#define _ACTION_H_

class PString;

class TelephonyIfc;

enum ActionID {
	ACTION_REGISTER,
	ACTION_DIAL,
	ACTION_HOLD,
	ACTION_AUTOHOLD,
	ACTION_RETRIEVE,
	ACTION_DISCONNECT,
	ACTION_QUIT
};

class Action {
	public:
		Action(const enum ActionID id):id(id) { }
		const enum ActionID id;
};

class RegisterAction : public Action {
	public:
		RegisterAction(const PString & registrar, const PString & user, const PString & password) :
			Action(ACTION_REGISTER),
			registrar(registrar),
			user(user),
			password(password)
			{ }

		const PString & registrar;
		const PString & user;
		const PString & password;
};

class DialAction : public Action {
	public:
		DialAction(const PString & dest) :
			Action(ACTION_DIAL),
			dest(dest)
			{ }

		const PString & dest;
};

class HoldAction : public Action {
	public:
		HoldAction() : Action(ACTION_HOLD) { }
};

class AutoHoldAction : public Action {
	public:
		AutoHoldAction() : Action(ACTION_AUTOHOLD) { }
};

class RetrieveAction : public Action {
	public:
		RetrieveAction() : Action(ACTION_RETRIEVE) { }
};

class DisconnectAction : public Action {
	public:
		DisconnectAction() : Action(ACTION_DISCONNECT) { }
};

class QuitAction : public Action {
	public:
		QuitAction() : Action(ACTION_QUIT) { }
};

#endif // _ACTION_H_
