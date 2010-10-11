/*
 *
 * TkOpalManager.cpp
 *
 * The TeleKarma OpalManager component.
 *
 */

#include "TkOpalManager.h"
#include "TkPCSSEndPoint.h"
#include "TkSIPEndPoint.h"


TkOpalManager::TkOpalManager(const PString & stunAddr, const PString & user) {
	pcssEP = NULL;
	sipEP  = NULL;
	aor    = NULL;


	///////////////////////////////////////
	// Disable video
	// TODO: Configuring opal with --disable-video should make these calls unnecessary.
#ifndef DISABLE_VIDEO
	SetAutoStartReceiveVideo(false);
	SetAutoStartTransmitVideo(false);
#endif
	///////////////////////////////////////
	// STUN Server

	SetSTUNServer(stunAddr);

	///////////////////////////////////////
	// PC Sound System (PCSS) handler

	pcssEP = new TkPCSSEndPoint(*this);

	PTRACE(3, "Sound output device: \"" << pcssEP->GetSoundChannelPlayDevice() << "\"");
	PTRACE(3, "Sound  input device: \"" << pcssEP->GetSoundChannelRecordDevice() << "\"");
#ifndef DISABLE_VIDEO
	PTRACE(3, "Video output device: \"" << GetVideoOutputDevice().deviceName << "\"");
	PTRACE(3, "Video  input device: \"" << GetVideoInputDevice().deviceName << '"');
#endif
	///////////////////////////////////////
	// SIP protocol handler

	sipEP = new TkSIPEndPoint(*this);
	sipEP->SetSendUserInputMode(OpalConnection::SendUserInputAsTone);
	sipEP->SetDefaultLocalPartyName(user);
	sipEP->SetRetryTimeouts(10000, 30000);
	sipEP->StartListeners(sipEP->GetDefaultListeners());

}


TkOpalManager::~TkOpalManager() {
	if (!currentCallToken.IsEmpty()) {
		EndCurrentCall();
	}
	if (!heldCallToken.IsEmpty()) {
		RetrieveCallOnHold();
		EndCurrentCall();
	}
	Unregister();
}


PBoolean TkOpalManager::Register(const PString & registrar, const PString & user, const PString & passwd, const PString & domain) {
	if (sipEP->IsRegistered(aor)) {
		return PTrue;
	} else {
		SIPRegister::Params params;
		params.m_registrarAddress = registrar;
		params.m_addressOfRecord = user;
		params.m_password = passwd;
		sipEP->Register(params, aor);
		for (int i = 80; i > 0; i--) {
			if (sipEP->IsRegistered(aor)) {
				return PTrue;
			}
			PThread::Sleep(250);
		}
		return PFalse;
	}
}


PBoolean TkOpalManager::Unregister() {
	if (sipEP->IsRegistered(aor)) {
		sipEP->Unregister(aor);
		while (sipEP->IsRegistered(aor));
	}
	return PTrue;
}


PString TkOpalManager::ToggleRecording(const PString & fname) {
	if (currentCallToken.IsEmpty()) {
		return "Cannot start or stop recording without a call in progress.";
	} else if (IsRecording(currentCallToken)) {
		StopRecording(currentCallToken);
		return "Recording stopped.";
	} else {
		StartRecording(currentCallToken, fname);
		return "Recording started.";
	}
}


PBoolean TkOpalManager::HoldCurrentCall() {
	PBoolean result = PFalse;
	if (currentCallToken.IsEmpty() && heldCallToken.IsEmpty()) {
		// no call in progress
	} else {
		if (heldCallToken.IsEmpty()) {
			PSafePtr<OpalCall> call = FindCallWithLock(currentCallToken);
			if (call == NULL) {
				// current call disappeared
			} else if (call->Hold()) {
				heldCallToken = currentCallToken;
				currentCallToken.MakeEmpty();
				result = PTrue;
			}
		} else {
			// there is already a call on hold
		}
	}
	return result;
}


PBoolean TkOpalManager::RetrieveCallOnHold() {
	PBoolean result = PFalse;
	if (currentCallToken.IsEmpty() && heldCallToken.IsEmpty()) {
		// no call in progress
	} else {
		if (heldCallToken.IsEmpty()) {
			// no call on hold
		} else {
			PSafePtr<OpalCall> call = FindCallWithLock(heldCallToken);
			if (call == NULL) {
				// current call disappeared
			} else if (call->Retrieve()) {
				currentCallToken = heldCallToken;
				heldCallToken.MakeEmpty();
				result = PTrue;
			}
		}
	}
	return result;
}


PBoolean TkOpalManager::SendTone(const char tone) {
	if (currentCallToken.IsEmpty()) {
		// no call in progress
		return PFalse;
	} else {
		PSafePtr<OpalCall> call = FindCallWithLock(currentCallToken);
		if (call == NULL) {
			// no call in progress
			return PFalse;
		} else {
			PSafePtr<OpalConnection> conn = call->GetConnection(0);
			while (conn != NULL) {
				conn->SendUserInputTone(tone, 180);
				// would be nice to hear this on speaker...
				++conn;
			}
			return PTrue;
		}
	}
}


PBoolean TkOpalManager::StartCall(const PString & dest) {
	if (!currentCallToken.IsEmpty() || dest.IsEmpty()) {
		// cannot call while current call exists
		// cannot call without specifying destination
		return PFalse;
	} else {
		return SetUpCall("pc:*", dest, currentCallToken);
	}
}


/** Disconnect the call currently in progress */
PBoolean TkOpalManager::EndCurrentCall() {
	PString & token = currentCallToken.IsEmpty() ? heldCallToken : currentCallToken;
	PSafePtr<OpalCall> call = FindCallWithLock(token);
	if (call == NULL) {
		// no call in progress
		return PFalse;
	} else {
		if (IsRecording(currentCallToken)) {
			StopRecording(currentCallToken);
		}
		call->Clear();
		token.MakeEmpty();
		return PTrue;
	}
}


PBoolean TkOpalManager::HasCallHolding() {
	return (heldCallToken.IsEmpty()) ? PFalse : PTrue;
}


PBoolean TkOpalManager::HasActiveCall() {
	return (currentCallToken.IsEmpty()) ? PFalse : PTrue;
}


void TkOpalManager::OnEstablishedCall(OpalCall & call) {
	currentCallToken = call.GetToken();
}


void TkOpalManager::OnClearedCall(OpalCall & call) {
	if (currentCallToken == call.GetToken())
		currentCallToken.MakeEmpty();
	else if (heldCallToken == call.GetToken())
		heldCallToken.MakeEmpty();
	PString remoteName = call.GetPartyB();
	cout << "Call with " << remoteName << " has ended because "
						<< call.GetCallEndReasonText();
	OpalManager::OnClearedCall(call);
}


PBoolean TkOpalManager::OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream) {
	if (!OpalManager::OnOpenMediaStream(connection, stream)) {
		return PFalse;
	}

	PCaselessString prefix = connection.GetEndPoint().GetPrefixName();
	if (prefix == "pc" || prefix == "pots")
		PTRACE(3, "Started" << (stream.IsSink() ? "playing " : "grabbing ") << stream.GetMediaFormat());
	else if (prefix == "ivr")
		PTRACE(3, "Started" << (stream.IsSink() ? "streaming " : "recording ") << stream.GetMediaFormat());
	else
		PTRACE(3, "Started" << (stream.IsSink() ? "sending " : "receiving ") << stream.GetMediaFormat()
	 		 << (stream.IsSink() ? " to " : " from ")<< prefix);

	return PTrue;
}


// End of File ///////////////////////////////////////////////////////////////
