/*
 *
 * telekarma.cpp
 *
 * The TeleKarma main application.
 *
 */

#ifdef WIN32
	#include <conio.h>	// for MS-specific _kbhit() and _getch()
	#include <io.h>		// for access()
#else
	#include <unistd.h>	// for access()
#endif


#include <ptlib.h>

#include <sys/types.h>  // for struct stat
#include <sys/stat.h>   // for struct stat

#include "telekarma.h"

#ifdef WIN32
	#define ACCESS _access
#else
	#define ACCESS access
#endif

using namespace std;


TeleKarma::TeleKarma() :
	PProcess("TeleKarma"),
	phone(NULL),
	currentState(NULL)
{
	for (int i = 0; i < STATE_COUNT; ++i) states[i] = NULL;
}


TeleKarma::~TeleKarma()
{
	currentState = NULL;
	for (int i = 0; i < STATE_COUNT; ++i)
		if (states[i] != NULL) delete states[i];
	if (phone != NULL) delete phone;
	PThread::Sleep(EXIT_DELAY);
	cout << "Thank you for using TeleKarma." << endl;
}


void TeleKarma::Main() {

	cout << "Welcome to TeleKarma!" << endl << endl << flush;

	// verify existence and type of 'logs' and 'recordings' folders
	const char * strPath1 = "logs";
	struct stat status;
	stat(strPath1, &status);
	if ((ACCESS(strPath1, 0) == -1) || !(status.st_mode & S_IFDIR)){
		cout << "Please create a \"logs\" folder in your TeleKarma program folder." << endl << flush;
		PThread::Sleep(2000);
		exit(1);
	}
	const char * strPath2 = "recordings"; 
	stat(strPath2, &status);
	if ((ACCESS(strPath2, 0) == -1) || !(status.st_mode & S_IFDIR)){
		cout << "Please create a \"recordings\" folder in your TeleKarma program folder." << endl << flush;
		PThread::Sleep(2000);
		exit(1);
	}

	// create log file
	PTime now;
	PString logFName("logs/log");
	logFName += now.AsString("_yyyy.mm.dd_hh.mm.ss");
	logFName += ".txt";
	PTrace::Initialise(5, logFName);

	// initialize states
	states[REGISTER]       = new RegisterStateHandler(*this);
	states[MENU]           = new MenuStateHandler(*this);
	states[DIAL]           = new DialStateHandler(*this);
	states[CONNECTED]      = new ConnectedStateHandler(*this);
	states[HOLD]           = new HoldStateHandler(*this);
	states[AUTO_HOLD]      = new AutoHoldStateHandler(*this);
	states[MUTE_AUTO_HOLD] = new MuteAutoHoldStateHandler(*this);
	states[EXIT]           = new ExitStateHandler(*this);
	states[DISCONNECT]     = new DisconnectStateHandler(*this);

	// enter REGISTER state
	EnterState(REGISTER);

	// enter running loop
	while (currentState != states[EXIT]) {
		currentState->In();
		PThread::Sleep(SLEEP_DURATION);
	}

	// exit the application
	cout << "Cleaning up... (this may take a few moments)" << endl << flush;

}


/**
 * Exit the current state (if any) and enter a new
 * specified state.
 */
void TeleKarma::EnterState(int stateId)
{
	if (currentState == states[stateId]) return;
	PTRACE(3, "DEBUG: TeleKarma.EnterState(" << stateId << ")");
	if (currentState != NULL) currentState->Exit();
	currentState = states[stateId];
	currentState->Enter();
}


/** Initialize the telephony API. Blocks. */
void TeleKarma::Initialize(const PString & stun, const PString & user)
{
	phone = new TelephonyIfc();
	phone->Initialise(stun, user);
}


/** Output STUN type identification message. */
PString TeleKarma::GetSTUNType()
{
	PSTUNClient * stunClient = phone->GetSTUNClient();
	if (stunClient != NULL) {
		return stunClient->GetNatTypeName();
	} else {
		return "none";
	}
}


/**
 * Initiate registration with a SIP service provider.
 * This method is non-blocking.
 */
void TeleKarma::Register(const PString & registrar, const PString & user, const PString & password)
{
	if (phone != NULL) phone->Register(registrar, user, password);
}


/**
 * Determine whether user is registered with SIP service.
 */
bool TeleKarma::IsRegistered()
{
	return (phone != NULL && phone->IsRegistered());
}


/** Dial the indicated SIP address. Format sipAddr as sip:user@domain. */
void TeleKarma::Dial(const PString & sipAddr)
{
	if (phone != NULL) phone->Dial(sipAddr);
}


/** Disconnect the current call. */
void TeleKarma::Disconnect()
{
	if (phone != NULL) phone->Disconnect();
}


/** 
 * Determine whether a DTMF tone has been recieved and
 * optionally reset the array of received tones.
 */
bool TeleKarma::ToneReceived(char key, bool clear)
{
	if (phone != NULL)
		return phone->ToneReceived(key, clear);
	return false;
}


/** Play a DTMF tone over phone connection. */
void TeleKarma::SendTone(char key)
{
	if (phone != NULL) phone->SendTone(key);
}


/** Play a WAV file over phone connection. */
void TeleKarma::PlayWAV(const PString & filename, bool onLine, bool onSpeaker)
{
	if (phone != NULL && onLine) phone->SendAudioFile(filename);
//	if (onSpeaker) // TO GO
}


void TeleKarma::ToggleRecording()
{
	PTime now;
	PString recFName("recordings/rec");
	recFName += now.AsString("_yyyy.mm.dd_hh.mm.ss");
	recFName += ".wav";
	phone->ToggleRecording(recFName);
}


/**
 * Obtain one character of keyboard input if present.
 * Convert to lowercase and return. If no input, return
 * the NULL character. Guaranteed not to block.
 */
 char TeleKarma::GetChar()
{
	char ch;
#ifdef WIN32
	if (_kbhit() == 0) {
		ch = NULL;
	} else {
		ch = tolower(_getch());
	}
#else
	/* XXX TO DO for Tom
	 * Why do we need to do non-blocking IO? 
	 * #error NON-BLOCKING INPUT REQUIRED BUT MISSING FOR LINUX */
	PString line;
	cin >> line;
	line = line.LeftTrim();
	if (line.GetLength() > 0)
		ch = line[0];
#endif
	return ch;
}


/** Print n backspaces to stdout. Does not flush. */
void TeleKarma::Backspace(int n)
{
	for (int i = 0; i < n; ++i) cout << '\b';
}


/** Print n spaces to stdout. Does not flush. */
void TeleKarma::Space(int n)
{
	for (int i = 0; i < n; ++i) cout << ' ';
}

bool TeleKarma::IsDialing()
{
	if (phone != NULL) return phone->IsDialing();
	return false;
}

bool TeleKarma::IsConnected()
{
	if (phone != NULL) return phone->IsConnected();
	return false;
}

PString TeleKarma::DisconnectReason()
{
	if (phone != NULL) return phone->DisconnectReason();
	return NULL;
}


bool TeleKarma::IsPlayingWAV(bool onLine, bool onSpeakers)
{
	if (onLine && onSpeakers) {
		return false;	// TO GO, since we currently don't play WAVs over speaker
	} else if (onLine) {
		return (phone != NULL && phone->InIVRMode());
	} else if (onSpeakers) {
		return false;	// TO GO, since we currently don't play WAVs over speaker
	} else {
		return false;
	}
}

void TeleKarma::SetMicVolume(int volume)
{
	// TO GO
	cerr << "Unimplemented: TeleKarma.SetMicVolume()" << endl;
}

void TeleKarma::SetSpeakerVolume(int volume)
{
	// TO GO
	cerr << "Unimplemented: TeleKarma.SetSpeakerVolume()" << endl;
}


// End of File ///////////////////////////////////////////////////////////////
