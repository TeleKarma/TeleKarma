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
class OpalMixerEndPoint;

class TelephonyIfc : public OpalManager {

	PCLASSINFO(TelephonyIfc, OpalManager);

	public:
		TelephonyIfc();
		virtual ~TelephonyIfc();

		void Initialise(const PString & stunAddr, const PString & user);
		void Register(const PString & registrar, const PString & user, const PString & passwd);

		/**
		 * Indicates whether the user is registered with a 
		 * SIP provider.
		 */
		PBoolean IsRegistered();

		PBoolean Unregister();

		// fix return value later... need tri-state return value
		PString  ToggleRecording(const PString & fname);
		
		/**
		 * Determines whether a DTMF tone has been received.
		 * DTMF tones are queued, this checks queue and clears
		 * it by default. Use clear = false to preserve the
		 * queue, for example to check for one of multiple
		 * keys of interest.
		 */
		bool ToneReceived(char key, bool clear = true);

		/**
		 * Send a DTMF tone to the remote party if connected.
		 */
		void SendTone(const char tone);

		/**
		 * Initiate a new call.
		 */
		void Dial(const PString & ostr);

		/**
		 * Determines whether there is a call being dialed, but
		 * not yet connected.
		 */
		PBoolean IsDialing();

		/**
		 * Callback invoked when call has been established.
		 */
		void OnEstablishedCall(OpalCall & call);

		/**
		 * Determines whether there is a connected call.
		 */
		PBoolean IsConnected();

		/**
		 * Disconnect the current call if it exists.
		 */
		void Disconnect();
		
		/**
		 * Callback invoked when call has been disconnected.
		 */
		void OnClearedCall(OpalCall & call);

		/**
		 * Returns a human-friendly discription of the reason
		 * a call ended or NULL.
		 */
		PString DisconnectReason();

		/**
		 * Callback invoked when a media stream is opened for
		 * reading or playing. Used to detect end of IVR 
		 * mode.
		 */
		PBoolean OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream);

		/**
		 * Callback invoked when DTMF tone is detected.
		 * Tones identified by a key (0-9, *, #). Array of
		 * tones holds record of unique tones detected since
		 * array was last cleared. This method populates
		 * the array of tones.
		 */
		void OnUserInputTone(OpalConnection& connection, char tone, int duration);

		/**
		 * Clears the array that holds a record of unique 
		 * tones received since the last time the array was
		 * cleared.
		 */
		void ClearTones();

		/**
		 * Transmit a WAV file to the remote party.
		 */
		PBoolean PlayWAV(const PString & path, int repeat=0, int delay=0);
		void StopWAV();

		PBoolean IsPlayingWav();

		/**
		 * Retrieve the call from any form of IVR mode.
		 */
		void Retrieve();

		/**
		 * Set the gain (volume) of the microphone.
		 */
		void SetMicVolume(unsigned int gain);

		/**
		 * Set the gain (volume) of the pc's speakers or
		 * other sound output device.
		 */
		void SetSpeakerVolume(unsigned int gain);

		void TurnOffMicrophone();
		void TurnOnMicrophone();

	protected:
		PString callToken;
		PString pcToken;
		PString wavToken;
		PString aor;
		bool dialing;
		PBoolean ivrMode;
		PString why;	// describes why call was disconnected
		char tones[DTMF_TONE_MAX];
		int nextTone;
		TkPCSSEndPoint * pcssEP;
		SIPEndPoint * sipEP;
		OpalIVREndPoint  * ivrEP;
		OpalMixerEndPoint * mixerEP;

		void OnAudioFileSent();

};


#endif // _TELEPHONY_H_


// End of File ///////////////////////////////////////////////////////////////
