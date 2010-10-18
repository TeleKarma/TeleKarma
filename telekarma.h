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

/* How long to wait (in milliseconds) before exiting */
#define EXIT_DELAY     1000

/* DTMF tone that signifies a human */
#define IS_HUMAN_TONE    '1'


class TeleKarma : public PProcess {

	PCLASSINFO(TeleKarma, PProcess);

	public:
		TeleKarma();
		virtual ~TeleKarma();
		void Main();
		void EnterState(int stateId);
		void Initialize(const PString & stun, const PString & user);
		void IdentifySTUNType();
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(const PString & destination);
		void Disconnect();
		bool IsRegistered();
		bool IsDialing();
		bool IsConnected();
		//UNKNOWN_TYPE GetConnectionState();
		bool IsPlayingWAV();
		void PlayWAV(const PString & src, bool onConnection = true, bool onSpeakers = true);		// details to go
		void SetMicVolume(int volume);
		void SetSpeakerVolume(int volume);
		void ToggleRecording();
		bool ToneReceived(char key, bool clear = true);
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
