/*
 *
 * telephony.h
 *
 * The TeleKarma OpalManager (TelephonyIfc) component.
 *
 */

#ifndef _TELEPHONY_H_
#define _TELEPHONY_H_

#include <opal/manager.h>
#include <opal/pcss.h>
#include <sip/sipep.h>
#include <opal/ivr.h>

#ifndef OPAL_PTLIB_AUDIO
#error Cannot compile without PTLib sound channel support!
#endif

/* Max number of unique DTMF tones to track */
#define DTMF_TONE_MAX    20

class TkPCSSEndPoint;


class TelephonyIfc : public OpalManager {

	PCLASSINFO(TelephonyIfc, OpalManager);

	public:
		TelephonyIfc();
		virtual ~TelephonyIfc();

		void Initialise(const PString & stunAddr, const PString & user);
		virtual void Register(const PString & registrar, const PString & user, const PString & passwd);
		virtual PBoolean StartCall(const PString & ostr);
		virtual PBoolean SendTone(const char tone);
		// fix return value later... need tri-state return value
		virtual PString  ToggleRecording(const PString & fname);
		virtual PBoolean EndCurrentCall();
		virtual PBoolean Unregister();
		/** True if there is an active (not holding) call. */
		virtual PBoolean HasActiveCall();
		virtual PBoolean IsRegistered();

		/**
		 * Determines whether a DTMF tone has been received.
		 * DTMF tones are queued, this checks queue and clears
		 * it by default. Use clear = false to preserve the
		 * queue, for example to check for one of multiple
		 * keys of interest.
		 */
		bool ToneReceived(char key, bool clear = true);

		virtual void OnEstablishedCall(OpalCall & call);
		virtual void OnClearedCall(OpalCall & call);
		virtual PBoolean OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream);
		//virtual void OnUserInputString(OpalConnection & connection,	const PString & value);

		/**
		 * Callback invoked when DTMF tone is detected.
		 * Tones identified by a key (0-9, *, #). Array of
		 * tones holds record of unique tones detected since
		 * array was last cleared. This method populates
		 * the array of tones.
		 */
		void OnUserInputTone(OpalConnection& connection, char tone, int duration);

		void WaitForHuman();
		void SendAudioFile(const PString & path);

	protected:
		PString callToken;
		PString aor;
		char tones[DTMF_TONE_MAX];
		int nextTone;
		TkPCSSEndPoint * pcssEP;
		SIPEndPoint * sipEP;
		OpalIVREndPoint  * ivrEP;

		PSafePtr<OpalConnection> GetConnection(PSafePtr<OpalCall> call, bool user, PSafetyMode mode);

};


#endif // _TELEPHONY_H_


// End of File ///////////////////////////////////////////////////////////////
