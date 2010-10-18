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
	currentState(NULL),
	nextTone(0)
{
	for (int i = 0; i < STATE_COUNT; ++i) states[i] = NULL;
	for (int i = 0; i < DTMF_TONE_MAX; ++i) tones[i] = NULL;
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
	cout << "DEBUG: TeleKarma.EnterState(" << stateId << ")" << endl;
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
void TeleKarma::IdentifySTUNType()
{
	PSTUNClient * stunClient = phone->GetSTUNClient();
	if (stunClient != NULL) {
		cout << "STUN client type is '" << stunClient->GetNatTypeName() << "'." << endl;
	} else {
		cout << "Not using a STUN client." << endl;
	}
}


/**
 * Initiate registration with a SIP service provider.
 * This method is non-blocking.
 */
void TeleKarma::Register(const PString & registrar, const PString & user, const PString & password)
{
	phone->Register(registrar, user, password);
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
	phone->StartCall(sipAddr);
}


/** Disconnect the current call. */
void TeleKarma::Disconnect()
{
	phone->EndCurrentCall();
}


/** 
 * Determine whether a DTMF tone has been recieved and
 * optionally reset the array of received tones.
 */
bool TeleKarma::ToneReceived(char key, bool clear)
{
	bool r = false;
	for (int i = 0; i < DTMF_TONE_MAX; ++i)
		if (tones[i] == key) r = true;
	if (clear)
		for (int i = 0; i < DTMF_TONE_MAX; ++i)
			tones[i] = NULL;
	return r;
}


/** Add a DTMF tone to the array of received tones. */
void TeleKarma::OnReceiveTone(char key)
{
	for (int i = 0; i < DTMF_TONE_MAX; ++i)
		if (tones[i] == key) return;
	tones[nextTone] = key;
	++nextTone;
	if (nextTone > DTMF_TONE_MAX)
		nextTone = 0;
}


/** Play a DTMF tone over phone connection. */
void TeleKarma::SendTone(char key)
{
	phone->SendTone(key);
}


/** Play a WAV file over phone connection. */
void TeleKarma::PlayWAV(const PString & filename, bool onLine, bool onSpeaker)
{
	if (onLine)	phone->SendAudioFile(filename);
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
	cerr << "Unimplemented: TeleKarma.IsDialing()" << endl;
	return false;
}

bool TeleKarma::IsConnected()
{
	cerr << "Unimplemented: TeleKarma.IsConnected()" << endl;
	return false;
}

//UNKNOWN_TYPE GetConnectionState();

bool TeleKarma::IsPlayingWAV()
{
	cerr << "Unimplemented: TeleKarma.IsPlayingWAV()" << endl;
	return false;
}

void TeleKarma::SetMicVolume(int volume)
{
	cerr << "Unimplemented: TeleKarma.SetMicVolume()" << endl;
}

void TeleKarma::SetSpeakerVolume(int volume)
{
	cerr << "Unimplemented: TeleKarma.SetSpeakerVolume()" << endl;
}


// End of File ///////////////////////////////////////////////////////////////
