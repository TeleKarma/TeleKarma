#ifndef _ACTION_H_
#define _ACTION_H_

class PString;

enum ActionID
{
	ACTION_INITIALIZE,
	ACTION_REGISTER,
	ACTION_DIAL,
	ACTION_HOLD,
	ACTION_AUTOHOLD,
	ACTION_MUTE,
	ACTION_RETRIEVE,
	ACTION_SEND_TONE,
	ACTION_DISCONNECT,
	ACTION_QUIT,
	ACTION_PLAY_SOUND
};

class Action
{
	public:
		Action(enum ActionID id, int turn):id(id), turn(turn) { }
		virtual ~Action() { }
		const enum ActionID id;
		const int turn;
};

class InitializeAction : public Action
{
	public:
		InitializeAction(const PString & stunServer, int turn) :
			Action(ACTION_INITIALIZE, turn),
			stunServer(stunServer)
			{ }

		const PString stunServer;
};

class RegisterAction : public Action
{
	public:
		RegisterAction(const PString & registrar, const PString & user, const PString & password, int turn);

		const PString registrar;
		const PString user;
		const PString password;
};

class DialAction : public Action
{
	public:
		DialAction(const PString & dest, int turn) :
			Action(ACTION_DIAL, turn),
			dest(dest)
			{ }

		const PString dest;
};

class HoldAction : public Action
{
	public:
		HoldAction(int turn) : Action(ACTION_HOLD, turn) { }
};

class AutoHoldAction : public Action
{
	public:
		AutoHoldAction(int turn) : Action(ACTION_AUTOHOLD, turn) { }
};

class RetrieveAction : public Action
{
	public:
		RetrieveAction(int turn) : Action(ACTION_RETRIEVE, turn) { }
};

class SendToneAction : public Action
{
	public:
		SendToneAction(int turn, char tone) : Action(ACTION_SEND_TONE, turn), tone(tone) { }
		const char tone;
};

class DisconnectAction : public Action
{
	public:
		DisconnectAction(int turn) : Action(ACTION_DISCONNECT, turn) { }
};

class QuitAction : public Action
{
	public:
		QuitAction(int turn) : Action(ACTION_QUIT, turn) { }
};

class PlaySoundAction : public Action
{
	public:
		PlaySoundAction(const PString & fname, int turn) :
			Action(ACTION_PLAY_SOUND, turn),
			fname(fname)
			{ }

		const PString fname;
};

#endif // _ACTION_H_