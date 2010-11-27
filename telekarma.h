/*
 *
 * telekarma.h
 *
 * TeleKarma's Controller.
 *
 */

#ifndef _TELEKARMA_H_
#define _TELEKARMA_H_

#include <ptlib.h>
#include "controller.h"

class Model;
class TelephonyIfc;
class Action;

/**
 * Defines the minimum amount of time, in milliseconds, for the
 * TeleKarma main application thread to sleep between control
 * loop iterations.
 */
#define SLEEP_DURATION   50

/**
 * Defines the amount of dead time between iterations of audio
 * file playback when in hold and human detection hold modes.
 * In these modes, audio files are played in a repeating loop.
 */
#define PAUSE_TIME        0

/**
 * Defines the number of times an audio file should be played,
 * at most, while in hold and human detection hold modes. The
 * aim here is for essentially endless playback looping, so
 * this number should be large.
 */
#define IVR_REPEATS   10000

/**
 * Defines how long to wait to close the program after
 * initiating termination. Longer values give users more time
 * to read the console and thus understand why the program is
 * exiting.
 */
#define EXIT_DELAY     1000

/**
 * Defines which DTMF tone (using the associated key label)
 * that is interpreted to mean that the remote party is a
 * human while in human detection hold mode. The audio file
 * played while in human detection hold mode should ask the
 * remote party to press this key.
 */
#define IS_HUMAN_TONE    '1'

/**
 * Defines the filename and path (relative to the telekarma
 * executable file's location) of the audio file played
 * to the remote party while in human detection hold mode.
 * The audio contained in this file should ask the remote
 * party to press the key defined by IS_HUMAN_TONE. Brevity
 * is desireable in the audio clip. Format: PCM-16 8Khz Mono
 * WAV.
 */
#define AUTO_HOLD_WAV   "press1.wav"

/**
 * Defines the filename and path (relative to the telekarma
 * executable file's location) of the audio file played
 * to the remote party while in standard hold mode.
 * Brevity is desireable in the audio clip.
 * Format: PCM-16 8Khz Mono WAV.
 */
#define HOLD_WAV        "pleasehold.wav"

/**
 * Defines the minimum timeout duration in milliseconds for registration.
 */
#define REGISTRATION_TIMEOUT (60*1000)

// XXX need to update the documentation...
/**
 * <p>
 * The TeleKarma class provides the "main" function for the
 * TeleKarma application and provides the interface through
 * which state handler objects access information and take
 * action.
 * </p><p>
 * The Main function, which is invoked by PTLib tools from
 * main.cpp, sets up state handler objects, sets the initial
 * state, and enters a loop that invokes the {@link StateHandler#In()}
 * method and sleeps for {@link #SLEEP_DURATION} milliseconds.
 * </p><p>
 * All other methods support state handler methods, which for
 * the sake of modularity have access to this class, but are
 * not provided direct access to the telephony interface or
 * other resources.
 * </p>
 */
class TeleKarma : public Controller {

	public:

		/**
		 * Constructor. Initializes fields.
		 * @param model
		 */
		TeleKarma(Model * model);

		/**
		 * <p>
		 * Main program logic. Verifies existance of required subfolders,
		 * sets up log files, instantiate state handler objects, sets
		 * the initial state handler and enters main control loop.
		 * </p><p>
		 * Main control loop simply calls the {@link StateHandler#In()}
		 * method, then sleeps, then repeats, until the state handler
		 * is set to an instance of {@link ExitStateHandler}. States
		 * call the {@link #EnterState(int)} method to change
		 * TeleKarma's state.
		 * </p>
		 */
		void Main();

	private:

		//
		// fields
		//

		int countdown;							// timeout support
		TelephonyIfc * phone;					// telephony services

		//
		// disabled assignment operator & copy constructor
		//

		TeleKarma & operator=(const TeleKarma & rhs);
		TeleKarma(const TeleKarma & rhs);

		//
		// helper methods
		//

		State * SetState(State * s);
		State * UpdateState(State * s);
		State * DoAction(Action * a, State * s);
		State * Initialize(Action * a, State * s);
		State * Register(Action * a, State * s);
		State * Dial(Action * a, State * s);
		State * Hold(Action * a, State * s);
		State * AutoHold(Action * a, State * s);
		State * MuteAutoHold(Action * a, State * s);
		State * Retrieve(Action * a, State * s);
		State * SendTone(Action * a, State * s);
		State * Disconnect(Action * a, State * s);
		State * Quit(Action * a, State * s);
		bool    IsConnectedState(State * s);
		bool    IsHoldingState(State * s);
		void StartRecording(State * currentState);

};


#endif // _TELEKARMA_H_

// End of File ///////////////////////////////////////////////////////////////
