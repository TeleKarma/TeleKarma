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

		virtual void OnEstablishedCall(OpalCall & call);
		virtual void OnClearedCall(OpalCall & call);
		virtual PBoolean OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream);
		//virtual void OnUserInputString(OpalConnection & connection,	const PString & value);
		void OnUserInputTone(OpalConnection& connection, char tone, int duration);
		void WaitForHuman();
		void SendAudioFile(const PString & path);

	protected:
		PString currentCallToken;
//		PString heldCallToken;
		PString aor;
		TkPCSSEndPoint * pcssEP;
		SIPEndPoint * sipEP;
		OpalIVREndPoint  * ivrEP;
//		PSafePtr<OpalPCSSConnection> activeCall;

		PSafePtr<OpalConnection> GetConnection(PSafePtr<OpalCall> call, bool user, PSafetyMode mode);

};


#endif // _TELEPHONY_H_


// End of File ///////////////////////////////////////////////////////////////
