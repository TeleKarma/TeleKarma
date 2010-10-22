/*
 *
 * telekarma.h
 *
 * The TeleKarma main application thread.
 *
 */

#ifndef _TELEKARMA_H_
#define _TELEKARMA_H_

#include <ptlib/pprocess.h>
#include "telephony.h"
#include "state.h"


/* How long (in milliseconds) to sleep between run loops */
#define SLEEP_DURATION   50

/* How long to wait between playing hold & autohold messages */
#define PAUSE_TIME     2000

/* How many times to loop the IVR message.  We want the IVR to repeat forever,
 * so this number should be large. */
#define IVR_REPEATS 10000

#define HOLD_REPEATS 10000

/* How long to wait (in milliseconds) before exiting */
#define EXIT_DELAY     1000

/* DTMF tone that signifies a human */
#define IS_HUMAN_TONE    '1'

/* Auto Hold WAV file */
#define AUTO_HOLD_WAV   "test.wav"

/* Regular Hold WAV file */
#define HOLD_WAV        "test.wav"


class TeleKarma : public PProcess {

	PCLASSINFO(TeleKarma, PProcess);

	public:
		TeleKarma();
		virtual ~TeleKarma();
		void Main();
		void EnterState(int stateId);
		void Initialize(const PString & stun, const PString & user);
		PString GetSTUNType();
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(const PString & destination);
		void Disconnect();
		bool IsRegistered();
		bool IsDialing();
		bool IsConnected();
		PString DisconnectReason();
		/* TO GO the following methods are not finished */
		bool IsPlayingWAV(bool onLine = true, bool onSpeakers = false);
		void PlayWAV(const PString & src, int repeat = 0, int delay = 0);		// details to go
		void SetMicVolume(unsigned int volume);
		void SetSpeakerVolume(unsigned int volume);
		void StartIVR();
		void StopIVR();
		void StartWAV();
		void StopWAV();
		/* End TO GO section */
		void ToggleRecording();
		void Retrieve();
		bool ToneReceived(char key, bool clear = true);
		void ClearTones();
		void SendTone(char key);
		// console I/O methods
		char GetChar();
		void Backspace(int n);
		void Space(int n);

	private:
		StateHandler * currentState;
		StateHandler * states[STATE_COUNT];
		TelephonyIfc * phone;

		// disabled assignment operator & copy constructor
		TeleKarma & operator=(const TeleKarma & rhs);
		TeleKarma(const TeleKarma & rhs);

};


#endif // _TELEKARMA_H_

// End of File ///////////////////////////////////////////////////////////////
