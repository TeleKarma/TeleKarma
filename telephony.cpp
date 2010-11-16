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
#include <opal/opalmixer.h>


TelephonyIfc::TelephonyIfc() :
	pcssEP(NULL), 
	sipEP(NULL), 
	ivrEP(NULL), 
	aor(""),
	callToken(""),
	pcToken(""),
	wavToken(""),
	why(""),
	nextTone(0),
	dialing(false),
	ivrMode(PFalse)
{
	why.MakeEmpty();
	callToken.MakeEmpty();
	pcToken.MakeEmpty();
	wavToken.MakeEmpty();
	aor.MakeEmpty();
	ClearTones();
}


TelephonyIfc::~TelephonyIfc()
{
	Disconnect();
	Unregister();
}


void TelephonyIfc::Initialise()//const PString & stunAddr, const PString & user)
{

	///////////////////////////////////////
	// Disable video

#if OPAL_VIDEO
	SetAutoStartReceiveVideo(false);
	SetAutoStartTransmitVideo(false);
#endif

	///////////////////////////////////////
	// Optionally set STUN Server

	SetSTUNServer("stun.ekiga.net");

	///////////////////////////////////////
	// PC Sound System (PCSS) handler

	pcssEP = new TkPCSSEndPoint(*this);
	// XXX This may not be necessary.
	pcssEP->StartListeners(pcssEP->GetDefaultListeners());

	PTRACE(3, "Sound output device: \"" << pcssEP->GetSoundChannelPlayDevice() << "\"");
	PTRACE(3, "Sound  input device: \"" << pcssEP->GetSoundChannelRecordDevice() << "\"");

	///////////////////////////////////////
	// SIP protocol handler

	sipEP = new TkSIPEndPoint(*this);
//	sipEP->SetDefaultLocalPartyName("tstellar");
	sipEP->SetRetryTimeouts(10000, 30000);
	sipEP->StartListeners(sipEP->GetDefaultListeners());
	
	///////////////////////////////////////
	// IVR Endpoint Setup & Config

	ivrEP = new OpalIVREndPoint(*this);
	ivrEP->StartListeners(ivrEP->GetDefaultListeners());

	///////////////////////////////////////
	// Mixer Endpoint Setup
	mixerEP = new OpalMixerEndPoint(*this, "mcu");
	OpalMixerNodeInfo * mcuNodeInfo = new OpalMixerNodeInfo;
	mcuNodeInfo->m_name = "Telekarma";
	mixerEP->SetAdHocNodeInfo(mcuNodeInfo);
	mixerEP->StartListeners(mixerEP->GetDefaultListeners());

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
		Disconnect();
		sipEP->Unregister(aor);
		while (sipEP->IsRegistered(aor));
	}
	return PTrue;
}

void TelephonyIfc::StartRecording(const PString & fname) {
	if (recordToken.IsEmpty()) {
		PString VXML = "ivr:<vxml><form>"
		"<record name=\"msg\" dtmfterm=\"false\" dest=\"" +
					PURL(PFilePath(fname)).AsString() + "\"/>"
		"</form></vxml>";

		SetUpCall("mcu:*;Listen-Only=1", VXML, recordToken);
	}
}

void TelephonyIfc::StopRecording() {
	ClearCall(recordToken);
	recordToken.MakeEmpty();
}

PBoolean TelephonyIfc::IsRecording() {
	return IsCallEstablished(recordToken);
}

void TelephonyIfc::SendTone(const char tone) {
	if (!callToken.IsEmpty()) {
		PSafePtr<OpalCall> call = FindCallWithLock(callToken);
		if (call != NULL) {
			PSafePtr<OpalConnection> conn = call->GetConnection(0);
			while (conn != NULL) {
				conn->SendUserInputTone(tone, 180);
				// would be nice to hear this on speaker...
				++conn;
			}
		}
	}
}


void TelephonyIfc::Dial(const PString & dest)
{
	if (callToken.IsEmpty() && !dest.IsEmpty()) {
		dialing = true;
		ClearTones();
		why.MakeEmpty();
		SetUpCall("mcu:*", "pc:*", pcToken);
		SetUpCall("mcu:*", dest, callToken);
	}
}

PBoolean TelephonyIfc::IsDialing()
{
	return dialing;
}


void TelephonyIfc::OnEstablishedCall(OpalCall & call)
{
	if (callToken == call.GetToken()) dialing = false;
}


PBoolean TelephonyIfc::IsConnected()
{
	return IsCallEstablished(callToken);
}


/** Disconnect the call currently in progress */
void TelephonyIfc::Disconnect()
{
	PSafePtr<OpalCall> call = FindCallWithLock(callToken);
	if (call != NULL) {
		if (IsRecording()) StopRecording();
		call->Clear();
	}
	PSafePtr<OpalCall> ivr  = FindCallWithLock(wavToken);
	if (ivr != NULL) {
		if (IsRecording()) StopRecording();
		ivr->Clear();
	}
	PSafePtr<OpalCall> pc   = FindCallWithLock(pcToken);
	if (pc != NULL) {
		if(IsRecording()) StopRecording();
		pc->Clear();
	}
}


void TelephonyIfc::OnClearedCall(OpalCall & call)
{
	if (wavToken == call.GetToken()) {
		wavToken.MakeEmpty();
	} else if (pcToken == call.GetToken()) {
		pcToken.MakeEmpty();
	} else if (callToken == call.GetToken()) {
		// update connectivity status
		dialing = false;
		callToken.MakeEmpty();
		// update disconnect reason
		PString remoteName = call.GetPartyB();
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
			why = remoteName + "Call with " + remoteName + " completed";
		}
	}
	ClearTones();
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
	if (prefix == "pc" || prefix == "pots") {
		PTRACE(3, "Started " << (stream.IsSink() ? "playing " : "grabbing ") << stream.GetMediaFormat());
	} else if (prefix == "ivr") {
		PTRACE(3, "Started " << (stream.IsSink() ? "streaming " : "recording ") << stream.GetMediaFormat());
		ivrMode = PTrue;
	} else {
		PTRACE(3, "Started" << (stream.IsSink() ? "sending " : "receiving ") << stream.GetMediaFormat()
	 		 << (stream.IsSink() ? " to " : " from ")<< prefix);
	}

	return PTrue;
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


void TelephonyIfc::ClearTones()
{
	for (int i = 0; i < DTMF_TONE_MAX; ++i)
		tones[i] = NULL;
	nextTone = 0;
}


bool TelephonyIfc::ToneReceived(char key, bool clear)
{
	bool r = false;
	for (int i = 0; i < DTMF_TONE_MAX; ++i)
		if (tones[i] == key) r = true;
	if (clear) ClearTones();
	return r;
}

PBoolean TelephonyIfc::PlayWAV(const PString & path, int repeat, int delay)
{
	if (!wavToken.IsEmpty()) {
		cout << "Only one wav file can be played at a time.\n";
		return PFalse;
	}
	PStringStream ivrString;
	ivrString << "ivr:repeat=" << repeat << ";delay=" << delay << ";" << PURL(PFilePath(path));
	SetUpCall("mcu:*", ivrString, wavToken);
	return PTrue;
}

void TelephonyIfc::StopWAV()
{
	ClearCall(wavToken);
	wavToken.MakeEmpty();
}

PBoolean TelephonyIfc::IsPlayingWav()
{
	return IsCallEstablished(wavToken);
}

void TelephonyIfc::SetMicVolume(unsigned int gain)
{
	PSafePtr<OpalCall> pc = FindCallWithLock(pcToken);
	if (pc != NULL) {
		PSafePtr<OpalConnection> connection = pc->GetConnection(0, PSafeReference);
		if (connection != NULL) {
			connection->SetAudioVolume(PTrue, gain);
			cerr << endl << "Set PC Mic Volume @ " << gain << "%" << endl;
		}
	}
	PSafePtr<OpalCall> call = FindCallWithLock(callToken);
	if (call != NULL) {
		PSafePtr<OpalConnection> connection = call->GetConnection(0, PSafeReference);
		if (connection != NULL) {
			connection->SetAudioVolume(PTrue, gain);
			cerr << endl << "Set Call Mic Volume @ " << gain << "%" << endl;
		}
	}
	PSafePtr<OpalCall> ivr = FindCallWithLock(wavToken);
	if (ivr != NULL) {
		PSafePtr<OpalConnection> connection = ivr->GetConnection(0, PSafeReference);
		if (connection != NULL) {
			connection->SetAudioVolume(PTrue, gain);
			cerr << endl << "Set IVR Mic Volume @ " << gain << "%" << endl;
		}
	}
}


void TelephonyIfc::SetSpeakerVolume(unsigned int gain)
{
	PSafePtr<OpalCall> pc = FindCallWithLock(pcToken);
	if (pc != NULL) {
		PSafePtr<OpalConnection> connection = pc->GetConnection(0, PSafeReference);
		if (connection != NULL) {
			connection->SetAudioVolume(PFalse, gain);
			cerr << endl << "Set Speaker Volume @ " << gain << "%" << endl;
		}
	}
	PSafePtr<OpalCall> call = FindCallWithLock(callToken);
	if (call != NULL) {
		PSafePtr<OpalConnection> connection = call->GetConnection(0, PSafeReference);
		if (connection != NULL) {
			connection->SetAudioVolume(PFalse, gain);
			cerr << endl << "Set Call Speaker Volume @ " << gain << "%" << endl;
		}
	}
	PSafePtr<OpalCall> ivr = FindCallWithLock(wavToken);
	if (ivr != NULL) {
		PSafePtr<OpalConnection> connection = ivr->GetConnection(0, PSafeReference);
		if (connection != NULL) {
			connection->SetAudioVolume(PFalse, gain);
			cerr << endl << "Set IVR Speaker Volume @ " << gain << "%" << endl;
		}
	}
}

void TelephonyIfc::TurnOffMicrophone()
{
	ClearCall(pcToken);
	//SetupCall("mcu:*", "pc:*", pcToken);
	SetUpCall("mcu:*;Listen-Only=1", "pc:*", pcToken);
}

void TelephonyIfc::TurnOnMicrophone(){
	ClearCall(pcToken);
	SetUpCall("mcu:*", "pc:*", pcToken);
}

// End of File ///////////////////////////////////////////////////////////////
