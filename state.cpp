
#ifdef WIN32
#include <conio.h>
#endif
#include <ptlib.h>
#include <iostream>
#include "state.h"
#include "conf.h"
#include "telekarma.h"

using namespace std;


StateHandler::StateHandler(TeleKarma & app) : tk(&app) { }
StateHandler::~StateHandler() { }
void StateHandler::Enter()
{
	cerr << endl << "Something has gone dreadfully wrong (StateHandler::Enter)." << endl;
	exit(1);
}
void StateHandler::In()
{
	cerr << endl << "Something has gone dreadfully wrong (StateHandler::IN)." << endl;
	exit(1);
}
void StateHandler::Exit()
{
	cerr << endl << "Something has gone dreadfully wrong (StateHandler::Exit)." << endl;
	exit(1);
}

/////////////////////////////////////////////////////////////////////////
// Exit
//

ExitStateHandler::ExitStateHandler(TeleKarma & tk) : StateHandler(tk) { }
ExitStateHandler::~ExitStateHandler() { }
void ExitStateHandler::Enter() { }
void ExitStateHandler::In() { }
void ExitStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Register
//

RegisterStateHandler::RegisterStateHandler(TeleKarma & tk) : StateHandler(tk), iterCount(0) { }

RegisterStateHandler::~RegisterStateHandler() { }

void RegisterStateHandler::Enter() {
	PString registrar(REGISTRAR);
	PString stunServer(STUN);
	PString user(ACCOUNT);
	PString passwd(PASSWORD);
	PString line;

	cout << "Please enter your SIP Registrar's address [" << registrar << "]: ";
	line.MakeEmpty();
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) registrar = line;

	cout << "Please enter your SIP Registrar's STUN address [" << stunServer << "]: ";
	line.MakeEmpty();
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) stunServer = line;

	cout << "Please enter your SIP user name" << flush;
	if (user.IsEmpty()) {
		cout << ": ";
	} else {
		cout << " [" << user << "]: ";
	}
	line.MakeEmpty();
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) user = line;

	cout << "Please enter your SIP password";
	if (passwd.IsEmpty()) {
		cout << ": ";
	} else {
		cout << " [";
		for (int i = 0; i < passwd.GetLength(); ++i) {
			cout << "*";
		}
		cout << "]: ";
	}
	cout << flush;
	line.MakeEmpty();
	int c = 0;
	int a = 0;
	while (c != 10 && c != 13)	{
		// protect the cast
		if (c >= 32 && c <= 255) {
			line += (char)c;
			cout << "*" << flush;
			++a;
		} else if (c == 8 && a > 0) {
			// backspace
			cout << "\b \b" << flush;
			--a;
			line = line.Left(a);
		}
#ifdef WIN32
		c = _getch();
#else
		c = getchar();
#endif
	}
	if (!line.IsEmpty()) passwd = line;
	cout << endl << endl << flush;

	/*
	cout << "=============================================================" << endl;
	cout << "SIP Server Address: '" << registrar << "'" << endl;
	cout << "STUN Server Address: '" << stunServer << "'" << endl;
	cout << "User name: '" << user << "'" << endl;
	cout << "Password: '" << passwd << "'" << endl;
	cout << "=============================================================" << endl;
	*/

	cout << "Initializing telephony system... (this may take a moment)" << flush;
	tk->Initialize(stunServer, user);
	tk->Backspace(24);
	cout << "done.";
	tk->Space(19);
	cout << endl;
	cout << "Identifying STUN client type... " << tk->GetSTUNType() << "." << endl;
	cout << flush;
	
	// Initiate registration (non-blocking)
	cout << "Registering with " << registrar << " as " << user << "... " << flush;
	tk->Register(registrar, user, passwd);
}

void RegisterStateHandler::In() {
	++iterCount;
	if (tk->IsRegistered()) {
		// registration succeeded
		cout << "done." << endl << endl << flush;
		tk->EnterState(MENU);
	} else if (iterCount > REG_ITER_LIMIT) {
		// registration failed
		cout << "failed." << endl << flush;
		tk->EnterState(EXIT);
	}
}

void RegisterStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Menu
//

MenuStateHandler::MenuStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
	menu << "  c   : Place a call" << endl;
	menu << "  q   : Quit" << endl;
	menu << endl;
}

MenuStateHandler::~MenuStateHandler() { }

void MenuStateHandler::Enter()
{
	cout << menu << "Command? " << flush;
}

void MenuStateHandler::In()
{
	char ch = tk->GetChar();
	PTRACE(3, "User command in Menu state: " << ch);
	if (ch == 'q') {
		cout << endl << endl << flush;
		tk->EnterState(EXIT);
	} else if (ch == 'c') {
		cout << endl << endl << flush;
		tk->EnterState(DIAL);
	}
}

/** No action. */
void MenuStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Dial
//

DialStateHandler::DialStateHandler(TeleKarma & tk) : StateHandler(tk) { }

DialStateHandler::~DialStateHandler() { }

void DialStateHandler::Enter() {
	PString dest(DEST);
	PString line;

	// obtain SIP address to call
	cout << "Enter SIP address [" << dest << "]: " << flush;
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) dest = line;
	// TO DO: figure out how regex matcher works (this doesn't work)
	//if (!dest.MatchesRegEx("^sip:")) {
	//	dest = "sip:" + dest;
	//}
	cout << endl << "Dialing... " << flush;

	// attempt to call the provided SIP address (non-blocking)
	tk->Dial(dest);

}

/** ... */
void DialStateHandler::In() {
	if (tk->IsConnected()) {
		// pending call has connected
		cout << "connected." << endl << endl << flush;
		tk->EnterState(CONNECTED);
	} else if (!tk->IsDialing()) {
		// pending call was cleared
		PString why = tk->DisconnectReason();
		if (why != NULL && !why.IsEmpty()) {
			cout << "failed to connect." << endl;
			cout << why << endl << endl << flush;
		} else {
			cout << "failed to connect." << endl;
			cout << "Call failed for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU);
	} else {
		// TO GO
		// Present disconnect & quit options?
	}
}

/** ... */
void DialStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Connected
//

/** ... */
ConnectedStateHandler::ConnectedStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
	menu << "  0-9 : touch tones" << endl;
	menu << "  *,# : touch tones" << endl;
	menu << "  h   : Hold call" << endl;
	menu << "  w   : Hold until human detected" << endl;
//	menu << "  q   : Quit" << endl;
	menu << "  x   : Disconnect" << endl;
	menu << "  d   : Disconnect" << endl;
	menu << "----------------------------------------" << endl;
	menu << "  z   : [FOR TESTING] Toggle recording" << endl;
	menu << "  p   : [FOR TESTING] Play WAV file" << endl;
	menu << "----------------------------------------" << endl;
	menu << endl;
}

/** ... */
ConnectedStateHandler::~ConnectedStateHandler() { }

/** ... */
void ConnectedStateHandler::Enter()
{
	cout << menu << "Command? " << flush;
}

/** ... */
void ConnectedStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << endl;
		PString why = tk->DisconnectReason();
		if (why != NULL && !why.IsEmpty()) {
			cout << "Call ended." << endl;
			cout << why << endl << endl << flush;
		} else {
			cout << "Call ended for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU);
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in Connected state: " << ch);
		switch (ch) {
			case 'x':
			case 'd':
				cout << endl << endl;
				tk->EnterState(DISCONNECT);
				break;
			case 'z':	// for dev & test purposes only
				// TO GO user feedback message improvement
				cout << endl << endl << "Recording toggled." << endl << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
			case 'h':
				cout << endl << endl;
				tk->EnterState(HOLD);
				break;
			case 'w':
				cout << endl << endl;
				tk->EnterState(AUTO_HOLD);
				break;
			case 'p':	// for dev & test purposes only
				tk->PlayWAV("test.wav");
				break;
//			case 'q':
//				cout << endl << flush;
//				tk->EnterState(EXIT);
//				break;
			default:
				if (isdigit(ch) || ch == '*' || ch == '#') {
					tk->SendTone(ch);
				}
				break;
		}
	}

}

/** No action. */
void ConnectedStateHandler::Exit() { }




/////////////////////////////////////////////////////////////////////////
// Hold
//

/** ... */
HoldStateHandler::HoldStateHandler(TeleKarma & tk) : 
	StateHandler(tk),
	iterCount(0)
{
	menu << "Select:" << endl;
//	menu << "  z   : Toggle recording" << endl;
	menu << "  r   : Retrieve call" << endl;
//	menu << "  q   : Quit" << endl;
	menu << "  x   : Disconnect" << endl;
	menu << "  d   : Disconnect" << endl;
	menu << endl;
}

/** ... */
HoldStateHandler::~HoldStateHandler() { }

/** ... */
void HoldStateHandler::Enter()
{
	tk->SetMicVolume(0);
	tk->SetSpeakerVolume(0);
	// TO GO play WAV file repeatedly
	cout << menu << "Command? " << flush;
}

/** ... */
void HoldStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << endl;
		PString why = tk->DisconnectReason();
		if (why != NULL && !why.IsEmpty()) {
			cout << why << endl << endl << flush;
		} else {
			cout << "Call ended for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU);
	} else {
		if (iterCount == 0) {
			tk->PlayWAV(HOLD_WAV);
			++iterCount;
		} else {
			if (!tk->IsPlayingWAV()) ++iterCount;
			if (iterCount > PAUSE_ITER_LIMIT) iterCount = 0;
		}
		char ch = tk->GetChar();
		PTRACE(3, "User command in Hold state: " << ch);
		switch (ch) {
			case 'x':
			case 'd':
				cout << endl;
				tk->EnterState(DISCONNECT);
				break;
			case 'z':	// for dev & test purposes only
				// TO GO user feedback message improvement
				cout << endl << "Recording toggled." << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
			case 'r':
				cout << endl;
				tk->EnterState(CONNECTED);
				break;
//			case 'q':
//				cout << endl << flush;
//				tk->EnterState(EXIT);
//				break;
		}
	}

}

/** Restore volume. */
void HoldStateHandler::Exit()
{
	tk->SetMicVolume(100);
	tk->SetSpeakerVolume(100);
}




/////////////////////////////////////////////////////////////////////////
// AutoHold
//

/** ... */
AutoHoldStateHandler::AutoHoldStateHandler(TeleKarma & tk) : 
	StateHandler(tk),
	iterCount(0),
	mute(false)
{
	menu << "Select:" << endl;
//	menu << "  z   : Toggle recording" << endl;
	menu << "  r   : Retrieve call" << endl;
//	menu << "  q   : Quit" << endl;
	menu << "  x   : Disconnect" << endl;
	menu << "  d   : Disconnect" << endl;
	menu << "  m   : Mute/Unmute Speakers" << endl;
	menu << endl;
}

/** ... */
AutoHoldStateHandler::~AutoHoldStateHandler() { }

/** ... */
void AutoHoldStateHandler::Enter()
{
	// clear the queue of received dtmf tones (slightly hacky)
	tk->ToneReceived('0', true);
	// Mute the microphone
	tk->SetMicVolume(0);
	//cerr << endl << "DEBUG: Wait " << PAUSE_ITER_LIMIT << " between cycles." << endl;
	cout << menu << "Command? " << flush;
}

/** ... */
void AutoHoldStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << endl;
		PString why = tk->DisconnectReason();
		if (why != NULL && !why.IsEmpty()) {
			cout << "Call ended." << endl << why << endl << endl << flush;
		} else {
			cout << "Call ended for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU);
	} else if (tk->ToneReceived(IS_HUMAN_TONE)) {
		// TO GO: play alarm WAV
		// tk->PlayWav("src", false, true);
		cout << endl;
		cout << "ALERT: Human auto-detected; call retrieved & active." << endl << flush;
		// TO GO: actually return to PCSS mode
		tk->EnterState(CONNECTED);
	} else {
		if (iterCount == 0) {
			tk->PlayWAV(AUTO_HOLD_WAV);
			++iterCount;
		} else {
			if (!tk->IsPlayingWAV()) ++iterCount;
			if (iterCount > PAUSE_ITER_LIMIT) iterCount = 0;
		}
		char ch = tk->GetChar();
		if (ch == 0) return;
		PTRACE(3, "User command in AutoHold state: " << ch);
		switch (ch) {
			case 'x':
			case 'd':
				cout << endl;
				tk->EnterState(DISCONNECT);
				break;
			case 'z':	// for dev & test purposes only
				// TO GO user feedback message improvement
				cout << endl << "Recording toggled." << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
			case 'r':
				cout << endl;
				// TO GO... haven't actually transferred away from IVR
				tk->EnterState(CONNECTED);
				break;
			case 'm':
				//tk->EnterState(MUTE_AUTO_HOLD);
				if (mute) {
					tk->SetSpeakerVolume(100);
				} else {
					tk->SetSpeakerVolume(0);
				}
				mute = !mute;
				break;
//			case 'q':
//				cout << endl << flush;
//				tk->EnterState(EXIT);
//				break;
		}
	}

}

/** ... */
void AutoHoldStateHandler::Exit()
{
	tk->SetMicVolume(100);
	tk->SetSpeakerVolume(100);
	// TO GO - if in IVR mode, transfer back to PCSS mode
}


/////////////////////////////////////////////////////////////////////////
// Disconnect
//

/** ... */
DisconnectStateHandler::DisconnectStateHandler(TeleKarma & tk) : StateHandler(tk) { }

/** ... */
DisconnectStateHandler::~DisconnectStateHandler() { }

/** ... */
void DisconnectStateHandler::Enter()
{
	cout << "Disconnecting... " << flush;
	tk->Disconnect();
}

/** ... */
void DisconnectStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << "done." << endl << endl << flush;
		tk->EnterState(MENU);
	}
}

/** ... */
void DisconnectStateHandler::Exit() { }
