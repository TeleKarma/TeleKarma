/*
 *
 * telephony.cpp
 *
 * The Telephony API interface component.
 *
 */

#include "telephony.h"
#include "pcss.h"
#include "sip.h"
#include <opal/ivr.h>


TelephonyIfc::TelephonyIfc() :
	pcssEP(NULL), 
	sipEP(NULL), 
	ivrEP(NULL), 
	aor(NULL),
	nextTone(0),
	dialing(false),
	connected(false),
	why(NULL)
{
	for (int i = 0; i < DTMF_TONE_MAX; ++i) tones[i] = NULL;
}


TelephonyIfc::~TelephonyIfc()
{
	if (!callToken.IsEmpty()) Disconnect();
	Unregister();
}


void TelephonyIfc::Initialise(const PString & stunAddr, const PString & user)
{

	///////////////////////////////////////
	// Disable video

#if OPAL_VIDEO
	SetAutoStartReceiveVideo(false);
	SetAutoStartTransmitVideo(false);
#endif

	///////////////////////////////////////
	// Optionally set STUN Server

	if (!stunAddr.IsEmpty()) SetSTUNServer(stunAddr);

	///////////////////////////////////////
	// PC Sound System (PCSS) handler

	pcssEP = new TkPCSSEndPoint(*this);

	PTRACE(3, "Sound output device: \"" << pcssEP->GetSoundChannelPlayDevice() << "\"");
	PTRACE(3, "Sound  input device: \"" << pcssEP->GetSoundChannelRecordDevice() << "\"");

	///////////////////////////////////////
	// SIP protocol handler

	sipEP = new TkSIPEndPoint(*this);
	sipEP->SetSendUserInputMode(OpalConnection::SendUserInputAsTone);
	sipEP->SetDefaultLocalPartyName(user);
	sipEP->SetRetryTimeouts(10000, 30000);
	sipEP->StartListeners(sipEP->GetDefaultListeners());
	
	///////////////////////////////////////
	// IVR Endpoint Setup & Config

	ivrEP = new OpalIVREndPoint(*this);

}


void TelephonyIfc::Register(const PString & registrar, const PString & user, const PString & passwd) {
	if (!sipEP->IsRegistered(aor)) {
		SIPRegister::Params params;
		params.m_registrarAddress = registrar;
		params.m_addressOfRecord = user;
		params.m_password = passwd;
		sipEP->Register(params, aor);
	}
}


PBoolean TelephonyIfc::IsRegistered()
{
	return sipEP->IsRegistered(aor);
}


PBoolean TelephonyIfc::Unregister() {
	if (sipEP->IsRegistered(aor)) {
		sipEP->Unregister(aor);
		while (sipEP->IsRegistered(aor));
	}
	return PTrue;
}


PString TelephonyIfc::ToggleRecording(const PString & fname) {
	if (callToken.IsEmpty()) {
		return "Cannot start or stop recording without a call in progress.";
	} else if (IsRecording(callToken)) {
		StopRecording(callToken);
		return "Recording stopped.";
	} else {
		StartRecording(callToken, fname);
		return "Recording started.";
	}
}

PBoolean TelephonyIfc::SendTone(const char tone) {
	if (callToken.IsEmpty()) {
		// no call in progress
		return PFalse;
	} else {
		PSafePtr<OpalCall> call = FindCallWithLock(callToken);
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


PBoolean TelephonyIfc::Dial(const PString & dest)
{
	if (!callToken.IsEmpty() || dest.IsEmpty()) {
		// cannot call while on a call
		// cannot call without specifying destination
		return PFalse;
	} else {
		connected = true;
		dialing = true;
		why = NULL;
		return SetUpCall("pc:*", dest, callToken);
	}
}

PBoolean TelephonyIfc::IsDialing()
{
	return dialing;
}


void TelephonyIfc::OnEstablishedCall(OpalCall & call)
{
	dialing = false;
	connected = true;
	callToken = call.GetToken();
}


PBoolean TelephonyIfc::IsConnected()
{
	return connected;
}


/** Disconnect the call currently in progress */
PBoolean TelephonyIfc::Disconnect()
{
	dialing = false;
	connected = false;
	PSafePtr<OpalCall> call = FindCallWithLock(callToken);
	if (call == NULL) {
		// no call in progress
		return PFalse;
	} else {
		if (IsRecording(callToken))
			StopRecording(callToken);
		call->Clear();
		return PTrue;
	}
}


void TelephonyIfc::OnClearedCall(OpalCall & call)
{
	dialing = false;
	if (callToken == call.GetToken()) callToken.MakeEmpty();
	PString remoteName = call.GetPartyB();
	//bool printTime = false;
	switch (call.GetCallEndReason()) {
	case OpalConnection::EndedByRemoteUser :
		why = remoteName + " has ended the call";
		break;
	case OpalConnection::EndedByCallerAbort :
		why = remoteName + " has hung up";
		break;
	case OpalConnection::EndedByRefusal :
		why = remoteName + " did not accept your call";
		break;
	case OpalConnection::EndedByNoAnswer :
		why = remoteName + " did not answer your call";
		break;
	case OpalConnection::EndedByTransportFail :
		why = remoteName + "Call with " + remoteName + " ended abnormally";
		break;
	case OpalConnection::EndedByCapabilityExchange :
		why = remoteName + "Could not find common codec with " + remoteName;
		break;
	case OpalConnection::EndedByNoAccept :
		why = remoteName + "Did not accept incoming call from " + remoteName;
		break;
	case OpalConnection::EndedByAnswerDenied :
		why = remoteName +  "Refused incoming call from " + remoteName;
		break;
	case OpalConnection::EndedByNoUser :
		why = remoteName + "Gatekeeper or registrar could not find user " + remoteName;
		break;
	case OpalConnection::EndedByNoBandwidth :
		why = remoteName + "Call to " + remoteName + " aborted, insufficient bandwidth";
		break;
	case OpalConnection::EndedByUnreachable :
		why = remoteName + remoteName + " could not be reached";
		break;
	case OpalConnection::EndedByNoEndPoint :
		why = remoteName + "No phone running for " + remoteName;
		break;
	case OpalConnection::EndedByHostOffline :
		why = remoteName + remoteName + " is not online";
		break;
	case OpalConnection::EndedByConnectFail :
		why = remoteName + "Transport error calling " + remoteName;
		break;
	default :
		//printTime = false;
		why = remoteName + "Call with " + remoteName + " completed";
	}
	/*
	if (printTime) {
		PTime now;
		cout << ", on " << now.AsString("w h:mma") << ". Duration "
			<< setprecision(0) << setw(5) << (now - call.GetStartTime())
			<< "s." << endl;
	}
	*/
	OpalManager::OnClearedCall(call);
}


PString TelephonyIfc::DisconnectReason()
{
	return why;
}


PBoolean TelephonyIfc::OnOpenMediaStream(OpalConnection & connection, OpalMediaStream & stream) {
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

void TelephonyIfc::WaitForHuman() {
}

void TelephonyIfc::OnUserInputTone(OpalConnection& connection, char tone, int duration)
{
	for (int i = 0; i < DTMF_TONE_MAX; ++i)
		if (tones[i] == tone) return;
	tones[nextTone] = tone;
	++nextTone;
	if (nextTone > DTMF_TONE_MAX)
		nextTone = 0;
}

bool TelephonyIfc::ToneReceived(char key, bool clear)
{
	/* TO GO - clear array upon disconnect */
	bool r = false;
	for (int i = 0; i < DTMF_TONE_MAX; ++i)
		if (tones[i] == key) r = true;
	if (clear)
		for (int i = 0; i < DTMF_TONE_MAX; ++i)
			tones[i] = NULL;
	return r;
}

PSafePtr<OpalConnection> TelephonyIfc::GetConnection(PSafePtr<OpalCall> call, bool user, PSafetyMode mode)
{
	if (call == NULL) 
		return NULL;

	PSafePtr<OpalConnection> connection = call->GetConnection(0, PSafeReference);
	while (connection != NULL && connection->IsNetworkConnection() == user)
		++connection;

	return connection.SetSafetyMode(mode) ? connection : NULL;
}

void TelephonyIfc::SendAudioFile(const PString & path)
{
	
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, "******************************************************************");
	PTRACE(3, path);

	PSafePtr<OpalCall> call = FindCallWithLock(callToken);
	if (call == NULL) {
		PTRACE(3, "Attempted to send WAV file failed: no active call.");
		return;
	}

	PSafePtr<OpalPCSSConnection> connection = PSafePtrCast<OpalConnection, OpalPCSSConnection>(GetConnection(call, true, PSafeReadOnly));
	if (connection == NULL) {
		PTRACE(3, "Attempted to send WAV file failed: no pcss connection.");
		return;
	}

	PStringStream ivrXML;
	ivrXML << "ivr:<?xml version=\"1.0\"?>"
		"<vxml version=\"1.0\">"
			"<form id=\"PlayFile\">"
				"<transfer bridge=\"false\" dest=\"pc:*;Auto-Answer=1\">"
					"<audio src=\"" << PURL(PFilePath(path)) << "\"/>"
				"</transfer>"
			"</form>"
		"</vxml>";

	if (!call->Transfer(ivrXML, connection))
		PTRACE(3, "Failed to send WAV file.");

	/* TO GO: also play over speakers; use the PSoundChannel classes from PTLib */

}

// End of File ///////////////////////////////////////////////////////////////
