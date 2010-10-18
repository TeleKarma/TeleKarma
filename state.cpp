
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
}
void StateHandler::In()
{
	cerr << endl << "Something has gone dreadfully wrong (StateHandler::IN)." << endl;
}
void StateHandler::Exit()
{
	cerr << endl << "Something has gone dreadfully wrong (StateHandler::Exit)." << endl;
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
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) registrar = line;

	cout << "Please enter your SIP Registrar's STUN address [" << stunServer << "]: ";
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) stunServer = line;

	cout << "Please enter your SIP user name" << flush;
	if (user.IsEmpty()) {
		cout << ": ";
	} else {
		cout << " [" << user << "]: ";
	}
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
	int c = 0;
	int a = 0;
	while (c != 10 && c != 13)	{
		// protect the cast
		if (c >= 32 && c <= 255) {
			passwd += (char)c;
			cout << "*" << flush;
			++a;
		} else if (c == 8 && a > 0) {
			// backspace
			cout << "\b \b" << flush;
			--a;
			passwd = passwd.Left(a);
		}
#ifdef WIN32
		c = _getch();
#else
		c = getchar();
#endif
	}
	cout << endl << flush;

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
	tk->Space(19);
	cout << "done." << endl;
	tk->IdentifySTUNType();
	cout << flush;
	
	// Initiate registration (non-blocking)
	cout << "Registering with " << registrar << " as " << user << "... " << flush;
	tk->Register(registrar, user, passwd);
}

void RegisterStateHandler::In() {
	++iterCount;
	if (tk->IsRegistered()) {
		// registration succeeded
		cout << " done." << endl << flush;
		tk->EnterState(DIAL);
	} else if (iterCount > REG_ITER_LIMIT) {
		// registration failed
		cout << " failed." << endl << flush;
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
		cout << endl << flush;
		tk->EnterState(EXIT);
	} else if (ch == 'c') {
		cout << endl << flush;
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
	cout << "Enter an SIP address [" << dest << "]: " << flush;
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) dest = line;
	// TO DO: figure out how regex matcher works (this doesn't work)
	//if (!dest.MatchesRegEx("^sip:")) {
	//	dest = "sip:" + dest;
	//}
	cout << endl << flush;

	// attempt to call the provided SIP address (non-blocking)
	tk->Dial(dest);
}

/** ... */
void DialStateHandler::In() {
	// TO GO perform call state detection
	// Present disconnect & quit options?
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
		cout << "Connection terminated by remote party." << endl;
		tk->EnterState(MENU);
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in Connected state: " << ch);
		switch (ch) {
			case 'x':
				cout << endl;
				tk->EnterState(DISCONNECT);
				break;
			case 'z':	// for dev & test purposes only
				// TO GO user feedback message improvement
				cout << endl << "Recording toggled." << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
			case 'h':
				cout << endl;
				tk->EnterState(HOLD);
				break;
			case 'w':
				cout << endl;
				tk->EnterState(AUTO_HOLD);
				break;
			case 'p':	// for dev & test purposes only
				tk->PlayWAV("\\TeleKarma\\test.wav");
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
HoldStateHandler::HoldStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
//	menu << "  z   : Toggle recording" << endl;
	menu << "  r   : Retrieve call" << endl;
//	menu << "  q   : Quit" << endl;
	menu << "  x   : Disconnect" << endl;
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
		cout << "Connection terminated by remote party." << endl;
		tk->EnterState(MENU);
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in Hold state: " << ch);
		switch (ch) {
			case 'x':
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
AutoHoldStateHandler::AutoHoldStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
//	menu << "  z   : Toggle recording" << endl;
	menu << "  r   : Retrieve call" << endl;
//	menu << "  q   : Quit" << endl;
	menu << "  x   : Disconnect" << endl;
	menu << "  m   : Mute mic & speakers" << endl;
	menu << endl;
}

/** ... */
AutoHoldStateHandler::~AutoHoldStateHandler() { }

/** ... */
void AutoHoldStateHandler::Enter()
{
	cout << menu << "Command? " << flush;
}

/** ... */
void AutoHoldStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << endl;
		cout << "Connection terminated by remote party." << endl;
		tk->EnterState(MENU);
	} else if (tk->ToneReceived(IS_HUMAN_TONE)) {
		// TO GO: play alarm WAV
		// tk->PlayWav("src", false, true);
		cout << endl;
		cout << "ALERT: Human auto-detected; call retrieved & active." << endl << flush;
		tk->EnterState(CONNECTED);
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in AutoHold state: " << ch);
		switch (ch) {
			case 'x':
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
			case 'm':
				tk->EnterState(MUTE_AUTO_HOLD);
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
	// TO GO - if in IVR mode, transfer back to PCSS mode
}



/////////////////////////////////////////////////////////////////////////
// MuteAutoHold
//

/** ... */
MuteAutoHoldStateHandler::MuteAutoHoldStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
//	menu << "  z   : Toggle recording" << endl;
	menu << "  r   : Retrieve call" << endl;
//	menu << "  q   : Quit" << endl;
	menu << "  x   : Disconnect" << endl;
	menu << "  m   : Un-mute mic & speakers" << endl;
	menu << endl;
}

/** ... */
MuteAutoHoldStateHandler::~MuteAutoHoldStateHandler() { }

/** ... */
void MuteAutoHoldStateHandler::Enter()
{
	tk->SetMicVolume(0);
	tk->SetSpeakerVolume(0);
	cout << menu << "Command? " << flush;
}

/** ... */
void MuteAutoHoldStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << endl;
		cout << "Connection terminated by remote party." << endl;
		tk->EnterState(MENU);
	} else if (tk->ToneReceived(IS_HUMAN_TONE)) {
		// TO GO: play alarm WAV
		// tk->PlayWav("src", false, true);
		cout << endl;
		cout << "ALERT: Human auto-detected; call retrieved & active." << endl << flush;
		tk->EnterState(CONNECTED);
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in MuteAutoHold state: " << ch);
		switch (ch) {
			case 'x':
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
			case 'm':
				tk->EnterState(AUTO_HOLD);
				break;
//			case 'q':
//				cout << endl << flush;
//				tk->EnterState(EXIT);
//				break;
		}
	}

}

/** ... */
void MuteAutoHoldStateHandler::Exit()
{
	// TO GO - if in IVR mode, transfer back to PCSS mode
	tk->SetMicVolume(0);
	tk->SetSpeakerVolume(0);
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
	cout << "Disconnecting..." << endl << flush;
	tk->Disconnect();
}

/** ... */
void DisconnectStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << "done." << endl << flush;
		tk->EnterState(MENU);
	}
}

/** ... */
void DisconnectStateHandler::Exit() { }