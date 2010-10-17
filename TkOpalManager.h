/*
 *
 * TkOpalManager.h
 *
 * The TeleKarma OpalManager component.
 *
 */

#ifndef _TKOPALMANAGER_H_
#define _TKOPALMANAGER_H_

#include <opal/manager.h>
#include <opal/pcss.h>
#include <sip/sipep.h>
#include <opal/ivr.h>

#ifndef OPAL_PTLIB_AUDIO
#error Cannot compile without PTLib sound channel support!
#endif

class TkPCSSEndPoint;


class TkOpalManager : public OpalManager {

	PCLASSINFO(TkOpalManager, OpalManager);

	public:
		TkOpalManager();
		virtual ~TkOpalManager();

		void Initialise(const PString & stunAddr, const PString & user);
		virtual PBoolean Register(const PString & registrar, const PString & user, const PString & passwd, const PString & domain = "n/a");
		virtual PBoolean StartCall(const PString & ostr);
		virtual PBoolean SendTone(const char tone);
		// fix return value later... need tri-state return value
		virtual PString  ToggleRecording(const PString & fname);
		virtual PBoolean EndCurrentCall();
		virtual PBoolean Unregister();
		/** True if there is an active (not holding) call. */
		virtual PBoolean HasActiveCall();

		virtual void OnEstablishedCall(OpalCall & call);
		virtual void OnClearedCall(OpalCall & call);
		virtual PBoolean OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream);
		//virtual void OnUserInputString(OpalConnection & connection,	const PString & value);
		void TkOpalManager::OnUserInputTone(OpalConnection& connection, char tone, int duration);
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


#endif // _TKOPALMANAGER_H_

// End of File ///////////////////////////////////////////////////////////////
