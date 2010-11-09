
#ifndef _ACTION_H_
#define _ACTION_H_

class PString;

class TelephonyIfc;

enum ActionID {
	ACTION_REGISTER
};

class Action {
	public:
		Action(const enum ActionID id):id(id) { }
		const enum ActionID id;

		virtual void Do(TelephonyIfc & phone) = 0;
};

class RegisterAction : public Action {
	public:
		RegisterAction(const PString & registrar, const PString & user, const PString & passwd);
		~RegisterAction();
		void Do(TelephonyIfc & phone);
	private:
		const PString & registrar;
		const PString & user;
		const PString & passwd;
};

#endif // _TELEKARMA_H_
