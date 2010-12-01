
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
	STATUS_AUTO_RETRIEVE,		// human detected, automatically exiting autohold
	STATUS_NOTIFY_RECORD,
	STATUS_RECORDING,
	STATUS_DONE_RECORDING,
	STATUS_TURN_MISMATCH,
	STATUS_RETRIEVE				// user-initiated exit from hold/autohold/muteautohold
};

class State
{
	public:
		/**
		 * Serial identifier used to conditionally accept action requests 
		 * based on the state in which they were generated.
		 */
		const int turn;
		/**
		 * Uniquely identifies the state type.
		 */
		const StateID id;
		/**
		 * Identifies the status associated with this state.
		 */
		const StatusID status;
		/** 
		 * Empty string or a message that may be useful to display to
		 * the user relating to the status.
		 */
		const PString message;
		/**
		 * Constructor.
		 * @param id   the type of this state
		 * @param turn previous state's id + 1 if previous state not of same
		 *             type as this state; otherwise, previous state's turn
		 */
		State(StateID id, int turn);
		/**
		 * Constructor.
		 * @param id   the type of this state
		 * @param turn previous state's id + 1 if previous state not of same
		 *             type as this state; otherwise, previous state's turn
		 * @param status the status to associate with this state
		 */
		State(StateID id, int turn, StatusID status);
		/**
		 * Constructor.
		 * @param id     the type of this state
		 * @param turn   previous state's id + 1 if previous state not of same
		 *               type as this state; otherwise, previous state's turn
		 * @param status the status to associate with this state
		 * @param msg    user-friendly elaboration on the status
		 */
		State(StateID id, int turn, StatusID status, const char * msg);
		/**
		 * Constructor.
		 * @param id     the type of this state
		 * @param turn   previous state's id + 1 if previous state not of same
		 *               type as this state; otherwise, previous state's turn
		 * @param status the status to associate with this state
		 * @param msg    user-friendly elaboration on the status
		 */
		State(StateID id, int turn, StatusID status, const PString & msg);
		/**
		 * Creates a copy of this class. Subclasses must implement this
		 * method.
		 */
		virtual State * Clone() const;
	// disable copy & assignment later...
};

#endif //_STATE_H
