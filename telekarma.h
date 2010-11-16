/*
 *
 * telekarma.h
 *
 * The TeleKarma main application thread.
 *
 */

#ifndef _TELEKARMA_H_
#define _TELEKARMA_H_

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include "controller.h"
#include "model.h"
#include "telephony.h"
#include "state.h"

class DialAction;
class EventQueue;
class RegisterAction;
class View;

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

	// PTLib macro for setting up methods required by the PProcess class.
	//PCLASSINFO(TeleKarma, PProcess);

	public:

		/**
		 * Constructor. Initializes fields.
		 */
		TeleKarma(Model * model);

		/**
		 * Destructor. Heap memory cleanup and delay of {@link EXIT_DELAY}
		 * milliseconds before returning.
		 */
		virtual ~TeleKarma();

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

		/**
		 * Called by state handler to transition to a new state. Calls
		 * the current state handler's {@link StateHandler#Exit()} method,
		 * then replaces the current state handler with an instance of
		 * the new state handler and calls that object's
		 * {@link StateHandler#Enter()} method.
		 * @param stateId one of the state identifiers from state.h.
		 */
		void EnterState(int stateId);

		/**
		 * Instantiates the {@link TelephonyIfc}. Expected to be called
		 * from a state handler object. Must be called prior to
		 * calling {@link #Register(const PString &, const PString &, const Pstring &)}.
		 * This is a blocking method.
		 * @param stun STUN server address (may be set to empty)
		 * @param user required username
		 */
		void Initialize(const PString & stun, const PString & user);

		/**
		 * Returns a description of the STUN server type.
		 * Not meaningful until {@link #Initialize(const PString &, const PString &)}
		 * has been called.
		 * @return a description of the STUN server type.
		 */
		PString GetSTUNType();

		/**
		 * Registers the user with an SIP registrar. This method is
		 * non-blocking. Registration process continues in another
		 * thread. Caller must use {@link #IsRegistered()} to verify
		 * success. Registration can take time.
		 * @param registrar an SIP registrar
		 * @param user user name registered with the registrar
		 * @param password user's password
		 */
		void Register(RegisterAction * params);

		/**
		 * Dials an SIP phone number. This method is non-blocking. Initiation
		 * of the call takes place in a separate thread. The caller is responsible
		 * for determing call state using {@link #IsDialing()} and {@link #IsConnected()}.
		 * @param destination an SIP-formatted address, including sip: prefix
		 */
		void Dial(DialAction * params);

		/**
		 * Disconnects a phone call. This method is non-blocking. Disconnection
		 * is carried out in a separate thread. The caller is responsible
		 * for determing call state using {@link #IsConnected()}.
		 */
		void Disconnect();

		/**
		 * Returns true if the user is registered with an SIP service. This
		 * method reflects actual registration state.
		 * @return true if the user is registered with their SIP registrar,
		 *         false otherwise.
		 */
		bool IsRegistered();

		/**
		 * Returns true if a call is in the process of being dialed. This
		 * method will return false once {@link #IsConnected()} begins
		 * returning true.
		 * @return true if a call is in the process of being dialed.
		 */
		bool IsDialing();

		/**
		 * Returns true if a call is connected.
		 * @return true if a call is connected, false if not connected
		 *         or dialing.
		 */
		bool IsConnected();

		/**
		 * Returns a user-friendly description of the reason why the
		 * most recent call in the current session was disconnected.
		 * @return a description of the reason for the most recent
		 *         call disconnection.
		 */
		PString DisconnectReason();

		/**
		 * Indicates whether a WAV is or has been played since
		 * the current call began.
		 * @param onLine true by default; false not supported.
		 * @param onSpeakers ignored.
		 * @return true if the current call has had a WAV played on it.
		 */
		bool IsPlayingWAV(bool onLine = true, bool onSpeakers = false);

		/**
		 * Plays a WAV file over the current call.
		 * @param src path (relative to telekarma.exe folder) and filename
		 *            of wav to play.
		 * @param repeat number of times to play the WAV. Defaults to zero.
		 * @param delay how long (in milliseconds) to wait between repeats.
		 *            defaults to zero.
		 */
		void PlayWAV(const PString & src, int repeat = 0, int delay = 0);

		/**
		 * Inoperative. Reserved for future development.
		 */
		void SetMicVolume(unsigned int volume);

		/**
		 * Inoperative. Reserved for future development.
		 */
		void SetSpeakerVolume(unsigned int volume);

		/**
		 * Called upon entry of standard hold and detect human hold modes.
		 * Disables the microphone, plays a notice of recording, begins
		 * recording, and calls {@link #PlayWav(const PString &, int, int)}.
		 * @param fname path (relative to telekarma.exe folder) and filename
		 *              of wav to play.
		 */
		void StartIVR(const PString &fname);

		/**
		 * Called upon termination of standard hold and detect human hold modes.
		 * Enables the microphone and calls {@link #StopWav()}.
		 */
		void StopIVR();

		/**
		 * Invokes {@link TelephonyIfc#StopWav()}.
		 */
		void StopWAV();

		/**
		 * Inverts the recording state. If recording, recording is stopped. If
		 * no recording, recording is initiated. Provided for debugging purposes
		 * only.
		 */
		void ToggleRecording();

		/**
		 * Begins recording of the call. Recordings are generated in WAV format
		 * and dumped in a timestamped file in the recordings subdirectory of
		 * the folder that holds the telekarma executable.
		 */
		void StartRecording();

		/**
		 * Indicates whether the touch tone associated with the given character
		 * has been received since the last time the touch tone queue was cleared,
		 * and optionally clears the touch tone queue.
		 * @param key the label of the key associated with a touch tone
		 * @param clear true (default) to clear the touch tone queue.
		 */
		bool ToneReceived(char key, bool clear = true);

		/**
		 * Clears the contents of the touch tone queue. Prevents historical touch
		 * tones from corrupting current input detection.
		 */
		void ClearTones();

		/**
		 * Sends the DTMF tone associated with the given key to the remote party.
		 * @param key the label of the key associated with a touch tone
		 */
		void SendTone(char key);

		/**
		 * Obtains a single character of input from the console. Non-blocking. If
		 * there is no input available on the console, returns NULL. Consumes the
		 * input.
		 * @return one character of input from console or NULL.
		 */
		char GetChar();

		/**
		 * Prints the given number of backspaces to the console.
		 * @param n the number of backspaces to print
		 */
		void Backspace(int n);

		/**
		 * Prints the given number of spaces to the console.
		 * @param n the number of spaces to print
		 */
		void Space(int n);

		/**
		 * XXX This should be private.
		 */
		void ProcessNextEvent();
		EventQueue * eventQueue;

	private:
		TelephonyIfc * phone;					// telephony services

		// disabled assignment operator & copy constructor
		TeleKarma & operator=(const TeleKarma & rhs);
		TeleKarma(const TeleKarma & rhs);

};


#endif // _TELEKARMA_H_

// End of File ///////////////////////////////////////////////////////////////
