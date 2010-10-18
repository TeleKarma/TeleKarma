/*
 * state.h
 *
 * State handler delegate classes of the TeleKarma class.
 *
 */

#ifndef _STATE_H_
#define _STATE_H_

/* System States - used as array indices */
#define EXIT              0
#define REGISTER          1
#define MENU              2
#define DIAL              3
#define CONNECTED         4
#define DISCONNECT        5
#define AUTO_HOLD         6
#define MUTE_AUTO_HOLD    7
#define HOLD              8

/* Number of System States - array size */
#define STATE_COUNT       9

/* How many times to call RegisterStateHandler.In() before failing */
#define REG_ITER_LIMIT 100

class TeleKarma;


class StateHandler
{
	public:
		StateHandler(TeleKarma & tk);
		virtual ~StateHandler();
		virtual void Enter();
		virtual void In();
		virtual void Exit();

	protected:
		TeleKarma * tk;

	private:
		StateHandler(const StateHandler & orig);
		StateHandler & operator=(const StateHandler & rhs);

};


class ExitStateHandler : public StateHandler
{
	public:
		ExitStateHandler(TeleKarma & tk);
		virtual ~ExitStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		ExitStateHandler(const ExitStateHandler & orig);
		ExitStateHandler & operator=(const ExitStateHandler & rhs);

};


class RegisterStateHandler : public StateHandler
{
	public:
		RegisterStateHandler(TeleKarma & tk);
		virtual ~RegisterStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		RegisterStateHandler(const RegisterStateHandler & orig);
		RegisterStateHandler & operator=(const RegisterStateHandler & rhs);
		int iterCount;

};


class MenuStateHandler : public StateHandler
{
	public:
		MenuStateHandler(TeleKarma & tk);
		virtual ~MenuStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		MenuStateHandler(const MenuStateHandler & orig);
		MenuStateHandler & operator=(const MenuStateHandler & rhs);
		PStringStream menu;

};


class DialStateHandler : public StateHandler
{
	public:
		DialStateHandler(TeleKarma & tk);
		virtual ~DialStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		DialStateHandler(const DialStateHandler & orig);
		DialStateHandler & operator=(const DialStateHandler & rhs);

};


class ConnectedStateHandler : public StateHandler
{
	public:
		ConnectedStateHandler(TeleKarma & tk);
		virtual ~ConnectedStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		ConnectedStateHandler(const ConnectedStateHandler & orig);
		ConnectedStateHandler & operator=(const ConnectedStateHandler & rhs);
		PStringStream menu;

};


class HoldStateHandler : public StateHandler
{
	public:
		HoldStateHandler(TeleKarma & tk);
		virtual ~HoldStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		HoldStateHandler(const HoldStateHandler & orig);
		HoldStateHandler & operator=(const HoldStateHandler & rhs);
		PStringStream menu;

};


class AutoHoldStateHandler : public StateHandler
{
	public:
		AutoHoldStateHandler(TeleKarma & tk);
		virtual ~AutoHoldStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		AutoHoldStateHandler(const AutoHoldStateHandler & orig);
		AutoHoldStateHandler & operator=(const AutoHoldStateHandler & rhs);
		PStringStream menu;

};


class MuteAutoHoldStateHandler : public StateHandler
{
	public:
		MuteAutoHoldStateHandler(TeleKarma & tk);
		virtual ~MuteAutoHoldStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		MuteAutoHoldStateHandler(const MuteAutoHoldStateHandler & orig);
		MuteAutoHoldStateHandler & operator=(const MuteAutoHoldStateHandler & rhs);
		PStringStream menu;

};


class DisconnectStateHandler : public StateHandler
{
	public:
		DisconnectStateHandler(TeleKarma & tk);
		virtual ~DisconnectStateHandler();
		void Enter();
		void In();
		void Exit();

	private:
		DisconnectStateHandler(const DisconnectStateHandler & orig);
		DisconnectStateHandler & operator=(const DisconnectStateHandler & rhs);

};


#endif // _STATE_H_