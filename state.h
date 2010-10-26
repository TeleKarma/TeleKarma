/*
 * state.h
 *
 * State handler delegate classes of the TeleKarma class.
 *
 */

#ifndef _STATE_H_
#define _STATE_H_

/** Identifier for the {@link ExitStateHandler}. */
#define EXIT              0
/** Identifier for the {@link RegisterStateHandler}. */
#define REGISTER          1
/** Identifier for the {@link MenuStateHandler}. */
#define MENU              2
/** Identifier for the {@link DialStateHandler}. */
#define DIAL              3
/** Identifier for the {@link ConnectedStateHandler}. */
#define CONNECTED         4
/** Identifier for the {@link DisconnectStateHandler}. */
#define DISCONNECT        5
/** Identifier for the {@link AutoHoldStateHandler}. Also known as human detection hold. */
#define AUTO_HOLD         6
/** Identifier for the {@link ExitStateHandler}. Also known as standard hold. */
#define HOLD              7

/* The number of System States. */
#define STATE_COUNT       8

/* How many times to call RegisterStateHandler.In() before failing */
#define REG_ITER_LIMIT  100

class TeleKarma;

/**
 * <p>
 * Abstract superclass that defines the interface for all
 * state handler delegate classes.
 * </p><p>
 * {@link TeleKarma} delegates state handling to subclasses
 * of this class. State handlers are responsible for detecting
 * environmental events (user input, telephony events),
 * changing the current state handler in {@link TeleKarma},
 * and carrying out actions via the {@link TeleKarma}
 * interface.
 * </p>
 */
class StateHandler
{
	public:

		/**
		 * Constructor. Retains a pointer to the {@link TeleKarma}
		 * instance for access to that class's methods.
		 */
		StateHandler(TeleKarma & tk);

		/** Destructor. No action. */
		virtual ~StateHandler();

		/**
		 * Called when a state handler is designated the active state
		 * handler in {@link TeleKarma}.
		 */
		virtual void Enter();

		/**
		 * Called from within {@link TeleKarma}'s main control loop
		 * upon each iteration for as long as this state handler is the
		 * active state handler in {@link TeleKarma}.
		 */
		virtual void In();

		/**
		 * Called by {@link TeleKarma} when this state handler is the
		 * active state handler but is about to be replaced as active
		 * state handler by another state handler class instance.
		 */
		virtual void Exit();

	protected:
		TeleKarma * tk;

	private:
		StateHandler(const StateHandler & orig);
		StateHandler & operator=(const StateHandler & rhs);

};


/**
 * This class does nothing. However, the termination condition
 * in the main control loop in {@link TeleKarma} is an instance
 * of this class designated as the active state handler.
 */
class ExitStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. */
		ExitStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~ExitStateHandler();
		/** Does nothing. */
		void Enter();
		/** Does nothing. */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		ExitStateHandler(const ExitStateHandler & orig);
		ExitStateHandler & operator=(const ExitStateHandler & rhs);

};


/**
 * This class is responsible for initializing telephony and
 * registering the user with an SIP registrar, including gathering
 * SIP account information from the user, initiating registration,
 * and verifying successful registration.
 */
class RegisterStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. */
		RegisterStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~RegisterStateHandler();
		/**
		 * Acquires registration data from user, calls {@link TeleKarma}'s
		 * {@link TeleKarma#Initialize(const PString &, const PString &)}
		 * method, and initiates registration.
		 */
		void Enter();
		/**
		 * Checks registration state until timeout. Forwards to menu
		 * state upon success, or exit state if registration fails.
		 */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		RegisterStateHandler(const RegisterStateHandler & orig);
		RegisterStateHandler & operator=(const RegisterStateHandler & rhs);
		int iterCount;

};


/**
 * This class is responsible for displaying the simple menu
 * presenting 'call' or 'quit' options and responding to
 * either command.
 */
class MenuStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. Creates menu string. */
		MenuStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~MenuStateHandler();
		/** Prints UI menu. */
		void Enter();
		/**
		 * Listens for user command. Forwards to dial or
		 * exit state as commanded.
		 */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		MenuStateHandler(const MenuStateHandler & orig);
		MenuStateHandler & operator=(const MenuStateHandler & rhs);
		PStringStream menu;

};


/**
 * This class is responsible for obtaining a SIP number
 * from the user, initiating dialing, and confirming
 * success or failure of the connection.
 */
class DialStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. */
		DialStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~DialStateHandler();
		/** Acquires number to dial; initates dialing. */
		void Enter();
		/**
		 * Detects still dialing, disconnected, or connected.
		 * Forwards to menu or connected.
		 */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		DialStateHandler(const DialStateHandler & orig);
		DialStateHandler & operator=(const DialStateHandler & rhs);

};


/**
 * This class is responsible for user options and connection
 * state handling when in a normal call connection (as
 * opposed to one of the hold modes).
 */
class ConnectedStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. Creates menu. */
		ConnectedStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~ConnectedStateHandler();
		/** Prints UI menu. */
		void Enter();
		/**
		 * Listens for user commands and checks connection state.
		 * Forwards to menu (on remote disconnect), standard hold,
		 * human detection hold, disconnect.
		 */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		ConnectedStateHandler(const ConnectedStateHandler & orig);
		ConnectedStateHandler & operator=(const ConnectedStateHandler & rhs);
		PStringStream menu;

};


/**
 * This class is responsible for placing a connected call
 * on standard hold, including recording, playing a wav
 * loop, presenting user options, detecting user commands
 * and responding those commands and connection state
 * appropriately.
 */
class HoldStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. Creates menu. */
		HoldStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~HoldStateHandler();
		/** Prints UI menu. Starts wav playback, recording, etc. */
		void Enter();
		/**
		 * Listens for user commands and checks connection state.
		 * Forwards to connected, disconnected (menu) or disconnect.
		 */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		HoldStateHandler(const HoldStateHandler & orig);
		HoldStateHandler & operator=(const HoldStateHandler & rhs);
		PStringStream menu;
		int iterCount;

};

/**
 * This class is responsible for placing a connected call
 * on detect human hold, including recording, muting mic,
 * playing a wav loop, detecting a DTMF tone, alerting the
 * user if a human is detected, monitoring call connection
 * state, and restoring the connection and mic upon
 * retrieval or human detection.
 */
class AutoHoldStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. Creates menu string. */
		AutoHoldStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~AutoHoldStateHandler();
		/**
		 * Starts wav playback, recording, disables mic, clears
		 * touchtone queue, plays sound to user and prints
		 * UI menu to console.
		 */
		void Enter();
		/**
		 * Detects user commands, call state, and for receipt
		 * of designed DTMF tone. Forwards to the appropriate
		 * state upon detection of relevant input or conditions.
		 * Responsible for alerting user if human is detected.
		 */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		// prohibit copy construction and assignment
		AutoHoldStateHandler(const AutoHoldStateHandler & orig);
		AutoHoldStateHandler & operator=(const AutoHoldStateHandler & rhs);
		PStringStream menu;
		int iterCount;
		bool mute;

};


/**
 * This class is responsible for disconnecting a call.
 * Forwards to the menu state.
 */
class DisconnectStateHandler : public StateHandler
{
	public:
		/** Constructor. Calls superclass constructor. */
		DisconnectStateHandler(TeleKarma & tk);
		/** Destructor. No action. */
		virtual ~DisconnectStateHandler();
		/** Initiate disconnection. */
		void Enter();
		/** Detect connected state; forward to menu state when disconnected. */
		void In();
		/** Does nothing. */
		void Exit();

	private:
		DisconnectStateHandler(const DisconnectStateHandler & orig);
		DisconnectStateHandler & operator=(const DisconnectStateHandler & rhs);

};


#endif // _STATE_H_
