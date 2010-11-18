
#ifndef _STATE_H_
#define _STATE_H_

class PString;

enum StateID
{
	STATE_UNINITIALIZED,
	STATE_INITIALIZING,
	STATE_INITIALIZED,
	STATE_REGISTERING,
	STATE_REGISTERED,
	STATE_DIALING,
	STATE_CONNECTED,
	STATE_DISCONNECTING,
	STATE_DISCONNECTED,
	STATE_HOLD,
	STATE_AUTOHOLD,
	STATE_MUTEAUTOHOLD,
	STATE_TERMINATING,
	STATE_TERMINATED,
	STATE_ERROR
};

enum StatusID
{
	STATUS_UNSPECIFIED,			// default: nothing to add
	STATUS_FAILED,				// failed
	STATUS_AUTO_RETRIEVE		// human detected, automatically exiting autohold
};

class State
{
	public:
		const int turn;
		const StateID id;
		const StatusID status;
		const PString message;
		State(StateID id, int turn);
		State(StateID id, int turn, StatusID status);
		State(StateID id, int turn, StatusID status, const char * msg);
		State(StateID id, int turn, StatusID status, const PString & msg);
		virtual State * Clone() const;
	// disable copy & assignment later...
};


#endif //_STATE_H
