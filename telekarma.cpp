/*
 *
 * telekarma.cpp
 *
 * Implementation of TeleKarma main application.
 *
 * See telekarma.h for full API documentation.
 *
 */

#ifdef WIN32
	#include <conio.h>			// for MS-specific _kbhit() and _getch()
	#include <io.h>				// for access()
#else
	#include <unistd.h>			// for access()
#endif


#include <ptlib.h>
#include <sys/types.h>  		// for struct stat
#include <sys/stat.h>   		// for struct stat
#include "telekarma.h"

#include "action.h"
#include "cliview.h"
#include "eventqueue.h"
#include "state.h"

#ifdef WIN32
	#define ACCESS _access		// MSFT proprietary version of C-standard access function
#else
	#define ACCESS access		// C-standard access function (for linux)
#endif

using namespace std;

// Constructor - best practices in field initialization.
TeleKarma::TeleKarma(Model * model) :
	Controller(model),
	phone(NULL)
	{ }

// Destructor - heap memory management, delay prior to exit,
// and final message to console.
TeleKarma::~TeleKarma()
{
	if (phone != NULL) delete phone;
	cout << "Thank you for using TeleKarma." << endl << flush;
	PThread::Sleep(EXIT_DELAY);
}


// Main program
void TeleKarma::Main() {


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
	logFName += now.AsString("_yyyy.MM.dd_hh.mm.ss");
	logFName += ".txt";
	PTrace::Initialise(5, logFName);

	phone = new TelephonyIfc();
	phone->Initialise();
	fprintf(stderr, "Initialized...\n");
	// exit the application
//	cout << "Cleaning up... (this may take a few moments)" << endl << flush;

	while (true) {
		ProcessNextEvent();
		PThread::Sleep(3000);
		if (phone && phone->IsRegistered()) {
			fprintf(stderr, "Registered\n");
		}
		if (phone && phone->IsConnected()) {
			fprintf(stderr, "Connected\n");
		}
	}
}


/**
 * Exit the current state (if any) and enter a new
 * specified state.
 */
void TeleKarma::EnterState(int stateId)
{
}



/** Initialize the telephony API. Blocks. */
void TeleKarma::Initialize(const PString & stun, const PString & user)
{
//	phone = new TelephonyIfc();
//	phone->Initialise(stun, user);
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
void TeleKarma::Register(RegisterAction * params)
{
	if (phone != NULL) {
		phone->Register(params->registrar, params->user, params->password);
	}
}


/**
 * Determine whether user is registered with SIP service.
 */
bool TeleKarma::IsRegistered()
{
	return (phone != NULL && phone->IsRegistered());
}


/** Dial the indicated SIP address. Format sipAddr as sip:user@domain. */
void TeleKarma::Dial(DialAction * params)
{
	if (phone != NULL) {
		phone->Dial(params->dest);
	}
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


/**
 * Resets the array of received tones.
 */
void TeleKarma::ClearTones()
{
	if (phone != NULL) phone->ClearTones();
}


/** Play a DTMF tone over phone connection. */
void TeleKarma::SendTone(char key)
{
	if (phone != NULL) phone->SendTone(key);
}


/** Play a WAV file over phone connection. */
void TeleKarma::PlayWAV(const PString & filename, int repeat, int delay)
{
	if (phone != NULL) {
		phone->PlayWAV(filename, repeat, delay);
	}
}

void TeleKarma::StopWAV()
{
	if (phone != NULL) {
		phone->StopWAV();
	}
}

void TeleKarma::StartIVR(const PString &fname)
{
	if (phone != NULL) {
		PString assuranceName = "assurance.wav";
		PlayWAV(assuranceName, 0,0);
		cout << "Please wait while a mandatory prompt is played..." << flush;
		PThread::Sleep(4500);
		StopWAV();
		cout << "finished.\n" << flush;
		StartRecording();
		phone->PlayWAV(fname, IVR_REPEATS, PAUSE_TIME);
		phone->TurnOffMicrophone();
	}
}

void TeleKarma::StopIVR()
{
	if (phone != NULL) {
		phone->StopWAV();
		phone->TurnOnMicrophone();
	}
}

//Only used for debugging.  Potentially for later features also.
void TeleKarma::ToggleRecording()
{
	if (!phone->IsRecording()) {
		StartRecording();
	} else {
		phone->StopRecording();
	}
}

void TeleKarma::StartRecording() {
	PTime now;
	PString recFName("recordings/rec");
	recFName += now.AsString("_yyyy.MM.dd_hh.mm.ss");
	recFName += ".wav";
	phone->StartRecording(recFName);
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
	/* Thanks to:
	 * http://cc.byexamples.com/2007/04/08/non-blocking-user-input-in-loop-without-ncurses/
	 * for linux kbhit implementation*/
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
	if(FD_ISSET(0, &fds)) {
		ch = tolower(getchar());
	} else {
		ch = NULL;
	}
#endif
	return ch;
}


void TeleKarma::Backspace(int n)
{
	for (int i = 0; i < n; ++i) cout << '\b';
}


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
		return (phone != NULL && phone->IsPlayingWav());
	} else if (onSpeakers) {
		return false;	// TO GO, since we currently don't play WAVs over speaker
	} else {
		return false;
	}
}

void TeleKarma::SetMicVolume(unsigned int volume)
{
	// not functioning per Opal spec
	if (phone != NULL) return phone->SetMicVolume(volume);
#ifdef WIN32
	if (volume > 0) {
		//UnMuteMic();
	} else {
		//MuteMic();
	}

#endif
}

void TeleKarma::SetSpeakerVolume(unsigned int volume)
{
	// not functioning per Opal spec
	if (phone != NULL) return phone->SetSpeakerVolume(volume);
}

void TeleKarma::ProcessNextEvent() {
	Action * action = model->DequeueAction();
	
	if (!action) {
		return;
	}

	switch(action->id) {

	case ACTION_REGISTER:
	{
		RegisterAction * registerAction = dynamic_cast<RegisterAction *>(action);
		/* XXX Handle this better? */
		if (!registerAction) {
			return;
		}
		Register(registerAction);
		break;
	}

	case ACTION_DIAL:
	{
		DialAction * dialAction = dynamic_cast<DialAction *>(action);
		if (!dialAction) {
			return;
		}
		Dial(dialAction);
		break;
	}

	case ACTION_HOLD:
		StartIVR(HOLD_WAV);
		break;

	case ACTION_AUTOHOLD:
		StartIVR(AUTO_HOLD_WAV);
		break;

	case ACTION_RETRIEVE:
		StopIVR();
		break;

	case ACTION_DISCONNECT:
		Disconnect();
		break;

	case ACTION_QUIT:
		/*XXX Do something here? */
		break;
	}

	delete action;
}

// End of File ///////////////////////////////////////////////////////////////
