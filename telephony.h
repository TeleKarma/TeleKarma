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

/** This class is the interface between telekarama and the Opal telephony
 * library.
 */
class TelephonyIfc : public OpalManager {

	PCLASSINFO(TelephonyIfc, OpalManager);

	public:
		TelephonyIfc();
		virtual ~TelephonyIfc();

		/**
		 * Set up the TelephonyIfc class
		 * @param stunAddr Address for the stun server
		 * (e.g. stun.ekiga.net)
		 * @param user The username that is used to authenticate with
		 * the registrar that will be used
		 */
		void Initialise();//const PString & stunAddr, const PString & user);

		/**
		 * Register with a service provided
		 * @param registrar Address of the registrar (e.g. ekiga.net)
		 */
		void Register(const PString & registrar, const PString & user, const PString & passwd);

		/**
		 * Indicates whether the user is registered with a 
		 * SIP provider.
		 */
		PBoolean IsRegistered();

		/**
		 * Unregister from the current SIP provider.
		 */
		PBoolean Unregister();

		/**
		 * @return PTrue if the current call is being recorded
		 * @return PFalse if the current call is not being recorded
		 */
		PBoolean IsRecording();

		/**
		 * Begin recording the current call
		 * @param fname Path to the file where the recording will be
		 * saved.
		 */
		void StartRecording(const PString & fname);

		/**
		 * Stop recording the current call
		 */
		void StopRecording();

		/**
		 * Determines whether a DTMF tone has been received.
		 * DTMF tones are queued, this checks queue and clears
		 * it by default. Use clear = false to preserve the
		 * queue, for example to check for one of multiple
		 * keys of interest.
		 *
		 * @param key The DTMF tone (use '1' not 1 if you are looking
		 * for the tone that comes from pressing the one key on a
		 * phone).
		 * @param clear Clear all entries in the queue for tone key
		 */
		bool ToneReceived(char key, bool clear = true);

		/**
		 * Send a DTMF tone to the remote party if connected.
		 * @param tone Tone to send
		 */
		void SendTone(const char tone);

		/**
		 * Initiate a new call.
		 * @param ostr URL representing the party to call in the form of
		 * [proto:][alias@][transport$]address[:port]
		 * Here is an example using the SIP protocol:
		 * sip:telekarma@uoregon.edu
		 */
		void Dial(const PString & ostr);

		/**
		 * Determines whether there is a call being dialed, but
		 * not yet connected.
		 */
		PBoolean IsDialing();

		/**
		 * Callback invoked when call has been established.
		 * @param call The call that has been established.
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
		 * @param call The call that has been cleared
		 */
		void OnClearedCall(OpalCall & call);

		/**
		 * Returns a human-friendly description of the reason
		 * a call ended or NULL.
		 */
		PString DisconnectReason();

		/**
		 * Callback invoked when a media stream is opened for
		 * reading or playing. Used to detect end of IVR 
		 * mode.
		 * @param connection Connection that owns the stream
		 * @param stream The stream that has been opened
		 */
		PBoolean OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream);

		/**
		 * Callback invoked when DTMF tone is detected.
		 * Tones identified by a key (0-9, *, #). Array of
		 * tones holds record of unique tones detected since
		 * array was last cleared. This method populates
		 * the array of tones.
		 * @param connection The connection that recieved the tone
		 * @param tone The tone that was received
		 * @param duration of the tone in milliseconds
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
		 * @note You can only play one WAV file at a time.
		 * @param path WAV file to play
		 * @param repeat The number of times to repeat the file
		 * @param delay The time in milliseconds to wait between
		 * repeated playings of the file
		 */
		PBoolean PlayWAV(const PString & path, int repeat=0, int delay=0);

		/**
		 * Stop the WAV file that is currently playing
		 */
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

		/**
		 * Block all microphone input
		 */
		void TurnOffMicrophone();

		/**
		 * Un-Block all microphone input
		 */
		void TurnOnMicrophone();

	protected:
		/** Call token for the connection from telekarama to the remote
		 * party */
		PString callToken;
		/** Call token for the connection from telekarama to the PC
		 * sound system */
		PString pcToken;
		/** Call token used to reference the WAV file that is currently
		 * playing */
		PString wavToken;
		/** Call token for the connection from telekarma to the
		 * recording device */
		PString recordToken;
		PString aor;
		/** PTrue if telekarma is dialing a remote party otherwise
		 * PFalse*/
		bool dialing;
		/** PTrue if telekarma is in IVR mode otherwise PFalse */
		PBoolean ivrMode;
		/** Describes why call was disconnected */
		PString why;
		/** Queue of tones pressed by the remote user */
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
