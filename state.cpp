
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

/** Hopefully these are never called! **/
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
//May be used for future development.

ExitStateHandler::ExitStateHandler(TeleKarma & tk) : StateHandler(tk) { }
ExitStateHandler::~ExitStateHandler() { }
void ExitStateHandler::Enter() { }
void ExitStateHandler::In() { }
void ExitStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Register
//

RegisterStateHandler::RegisterStateHandler(TeleKarma & tk) : StateHandler(tk), iterCount(0) { }

/** Nothing extra to do in the RegisterState deconstructor. **/
RegisterStateHandler::~RegisterStateHandler() { }

/** Enter the register state and prompt for the user's info to register. **/
void RegisterStateHandler::Enter() {
	PString registrar(REGISTRAR);
	PString stunServer(STUN);
	PString user(ACCOUNT);
	PString passwd(PASSWORD);
	PString line;

	//Very self-explanitory.
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
#else //Had a lot of problems with this else being blocking...we need non-blocking.
		c = getchar();
#endif
	}
	if (!line.IsEmpty()) passwd = line;
	cout << endl << endl << flush;

	//May be useful for debugging and/or future use.
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

/** Attempt to register when we are in the RegisterState **/
void RegisterStateHandler::In() {
	++iterCount;
	if (tk->IsRegistered()) {
		// registration succeeded
		cout << "done." << endl << endl << flush;
		tk->EnterState(MENU);
	} else if (iterCount > REG_ITER_LIMIT) {
		// registration failed
		cout << "failed." << endl << flush;
		cout << "Could not register...exiting program." << endl << flush;
		tk->EnterState(EXIT); //go to the ExitState
	}
}

/** Nothing extra do to when we exit the RegisterState **/
void RegisterStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Menu
//

/** Initialize and print the menu. **/
MenuStateHandler::MenuStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
	menu << "  c   : Place a call" << endl;
	menu << "  q   : Quit" << endl;
	menu << endl;
}
/** Nothing extra to do in the deconstructor **/
MenuStateHandler::~MenuStateHandler() { }

/** Just a print statement when we enter the MenuState **/
void MenuStateHandler::Enter()
{
	cout << menu << "Command? " << flush;
}

/** Basic handler while inside the MenuState **/
void MenuStateHandler::In()
{
	char ch = tk->GetChar(); //basic commands as usual
	PTRACE(3, "User command in Menu state: " << ch);
	if (ch == 'q') { //"quit"
		cout << endl << endl << flush;
		tk->EnterState(EXIT);
	} else if (ch == 'c') { //or call.
		cout << endl << endl << flush;
		tk->EnterState(DIAL);
	}
}

/** No action. */
void MenuStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Dial
//

/** Nothing extra to do here. **/
DialStateHandler::DialStateHandler(TeleKarma & tk) : StateHandler(tk) { }

/** Nothing extra to do in the deconstructor. **/
DialStateHandler::~DialStateHandler() { }

/** Basic handler when we enter the DialState **/
void DialStateHandler::Enter() {
	PString dest(DEST);
	PString line;

	// obtain SIP address to call
	cout << "Enter SIP address [" << dest << "]: " << flush;
	cin >> line;
	line = line.Trim();
	if (!line.IsEmpty()) dest = line;
	// May be used in future development.
	//if (!dest.MatchesRegEx("^sip:")) {
	//	dest = "sip:" + dest;
	//}
	cout << endl << "Dialing... " << flush;

	// attempt to call the provided SIP address (non-blocking)
	tk->Dial(dest);

}

/** Basic handler while in the DialState */
void DialStateHandler::In() {
	if (tk->IsConnected()) {
		// pending call has connected
		cout << "connected." << endl << endl << flush;
		tk->EnterState(CONNECTED);
	} else if (!tk->IsDialing()) {
		// pending call was cleared
		PString why = tk->DisconnectReason();
		if (!why.IsEmpty()) {
			cout << "failed to connect." << endl;
			cout << why << endl << endl << flush;
		} else {
			cout << "failed to connect." << endl;
			cout << "Call failed for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU);
	} else {
		// Will be used in future development.
	}
}

/** ... */
void DialStateHandler::Exit() { }



/////////////////////////////////////////////////////////////////////////
// Connected
//

/** Initialize and send things to the menu. */
ConnectedStateHandler::ConnectedStateHandler(TeleKarma & tk) : StateHandler(tk)
{
	menu << "Select:" << endl;
	menu << "  0-9 : touch tones" << endl;
	menu << "  *,# : touch tones" << endl;
	menu << "  h   : Hold call" << endl;
	menu << "  w   : Hold until human detected" << endl;
	menu << "  d   : Disconnect" << endl;
//	menu << "----------------------------------------" << endl;
//	menu << "  z   : [FOR TESTING] Toggle recording" << endl;
//	menu << "  p   : [FOR TESTING] Play WAV file" << endl;
//	menu << "----------------------------------------" << endl;
	menu << endl;
}

/** Nothing extra done in the deconstructor */
ConnectedStateHandler::~ConnectedStateHandler() { }

/** Just a print statement when you enter the ConnectedState */
void ConnectedStateHandler::Enter()
{
	cout << menu << "Command? " << flush;
}

/** This is the main block of code for the ConnectedState.  Here
*** the basic menu is displayed which provides our main functionality
*/
void ConnectedStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << endl;
		PString why = tk->DisconnectReason();
		if (!why.IsEmpty()) {
			cout << "Call ended." << endl;
			cout << why << endl << endl << flush;
		} else {
			cout << "Call ended for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU);
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in Connected state: " << ch);
		switch (ch) { //the usual messages
			case 'd': //d for disconnect
				cout << endl << endl;
				tk->EnterState(DISCONNECT);
				break;
			case 'h': //h for regular hold
				cout << endl << endl;
				tk->EnterState(HOLD);
				break;
			case 'w': //w for "AutoHold"
				cout << endl << endl;
				tk->EnterState(AUTO_HOLD);
				break;
			case 'z':	// for dev & test purposes only
				cout << endl << endl << "Recording toggled." << endl << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
			/*case 'p':	// for dev & test purposes only
				tk->PlayWAV("test.wav");
				break;*/
			default: //lastly, if nothing else got caught, check if it's supposed to be a tone
				if (isdigit(ch) || ch == '*' || ch == '#') {
					tk->SendTone(ch); //Use DTMF to send the tone pressed
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

/** Initialize and display the menu. */
HoldStateHandler::HoldStateHandler(TeleKarma & tk) :
	StateHandler(tk),
	iterCount(0)
{
	menu << "Select:" << endl;
	menu << "  r   : Retrieve call" << endl;
	menu << "  d   : Disconnect" << endl;
	menu << endl;
}

/** Nothing extra to do in the deconstructor */
HoldStateHandler::~HoldStateHandler() { }

/** When we enter the HoldState, start the IVR and play the hold wav prompt. */
void HoldStateHandler::Enter()
{
	iterCount = 0;
	tk->StartIVR(HOLD_WAV);
	cout << menu << "Command? " << flush;
}

/** Very similar to the AutoHoldStateHandler but sadly the code was not easily
*** modulated from AutoHoldStateHandler nor from here to AutoHoldStateHandler.
*** This is the main code for dealing with the HoldState.  A wav prompt is played
*** letting the person on hold know that the user will be returning shortly.
*/
void HoldStateHandler::In()
{
	if (!tk->IsConnected()) { //if we are NOT connected.
		cout << endl;
		PString why = tk->DisconnectReason(); //display the disconnect reason if there was one
		if (!why.IsEmpty()) {
			cout << why << endl << endl << flush;
		} else {
			cout << "Call ended for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU); //Enter the MenuState if we were disconnected
	} else {
		char ch = tk->GetChar();
		PTRACE(3, "User command in Hold state: " << ch);
		switch (ch) {
			case 'd':
				cout << endl;
				tk->EnterState(DISCONNECT);
				break;
			case 'r': // take control back from the HoldState
				cout << endl;
				tk->EnterState(CONNECTED);
				break;
			case 'z':	// for dev & test purposes only
				// TO GO user feedback message improvement
				cout << endl << "Recording toggled." << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
		}
	}

}
/** Just stop the IVR when exiting the HoldState **/

void HoldStateHandler::Exit()
{
	tk->StopIVR();
}




/////////////////////////////////////////////////////////////////////////
// AutoHold
//

/** This the main state for our "AutoHold" feature where a continual loop of
*** of audio is played and then listens for a dial-tone response of the number 1 key.
**/
AutoHoldStateHandler::AutoHoldStateHandler(TeleKarma & tk) :
	StateHandler(tk),
	iterCount(0),
	mute(false)
{
	menu << "Select:" << endl;
	menu << "  r   : Retrieve call" << endl;
	menu << "  d   : Disconnect" << endl;
	//menu << "  m   : Mute/Unmute Speakers" << endl;
	//The above line may be useful for further development
	menu << endl;
}

/** Nothin to deconstruct. */
AutoHoldStateHandler::~AutoHoldStateHandler() { }

/** ... */
void AutoHoldStateHandler::Enter()
{
	iterCount = 0;
	// clear the queue of received dtmf tones
	tk->ClearTones();
	// clear the queue of received dtmf tones (slightly hacky)
	tk->ToneReceived('0', true);
	//Start the IVR with the wav's filename.
	tk->StartIVR(AUTO_HOLD_WAV);
	cout << menu << "Command? " << flush;
#ifdef WIN32
	// Win32 library call
	// Play an Alert to let the user know they have entered AutoHold
	PlaySound(TEXT("alert2.wav"), NULL, SND_FILENAME);
#endif
}

/** This is our AutoHold Handler.  This takes care of all the main functionality
*** for AutoHold.
*/
void AutoHoldStateHandler::In()
{
	if (!tk->IsConnected()) { // if the user is NOT connected
		cout << endl;
		PString why = tk->DisconnectReason(); // get the disconnect reason and print.
		if (!why.IsEmpty()) { // if there was no reason.
			cout << "Call ended." << endl << why << endl << endl << flush;
		} else { //if there was a reason.
			cout << "Call ended for unknown reasons." << endl << endl << flush;
		}
		tk->EnterState(MENU); // get out of AutoHold.
	} else if (tk->ToneReceived(IS_HUMAN_TONE)) { //If a human was detected
		tk->StopIVR(); //Stop the IVR
#ifdef WIN32
		PlaySound(TEXT("userconnected.wav"), NULL, SND_FILENAME); // applause please.
		cout << endl;
		cout << "ALERT: Human auto-detected; call retrieved & active." << endl << flush;
		// Win32 library call
		PlaySound(TEXT("alert.wav"), NULL, SND_FILENAME); //Alert that you are going back to "Connected" state.
#endif

		tk->EnterState(CONNECTED);
	} else {
		char ch = tk->GetChar();
		if (ch == 0) return;
		PTRACE(3, "User command in AutoHold state: " << ch);
		switch (ch) {

			case 'd':
				cout << endl;
				tk->EnterState(DISCONNECT);
				break;

			case 'r': // take back control from AutoHold
				cout << endl;
				tk->StopIVR();
				tk->EnterState(CONNECTED);
				break;
			case 'z':	// For dev & test purposes only
				// Room for imporvement on the feedback given here.
				cout << endl << "Recording toggled." << endl << "Command? " << flush;
				tk->ToggleRecording();
				break;
			// Maybe used in future development.
			/*case 'm':
				if (mute) {
					//tk->SetSpeakerVolume(100);
				} else {
					//tk->SetSpeakerVolume(0);
				}
				mute = !mute;
				break;*/
		}
	}

}

/** No extra actions taken when exiting AutoHold. */
void AutoHoldStateHandler::Exit()
{
}


/////////////////////////////////////////////////////////////////////////
// Disconnect
//

/** ... */
DisconnectStateHandler::DisconnectStateHandler(TeleKarma & tk) : StateHandler(tk) { }

/** ... */
DisconnectStateHandler::~DisconnectStateHandler() { }

/** Just call Disonnect when you enter the DisconnectState */
void DisconnectStateHandler::Enter()
{
	cout << "Disconnecting... " << flush;
	tk->Disconnect();
}

/** Finish disonnecting. */
void DisconnectStateHandler::In()
{
	if (!tk->IsConnected()) {
		cout << "done." << endl << endl << flush;
		tk->EnterState(MENU); //Enter the Menu state again.
	}
}

/** No extra actions taken when Exiting the DisconnectState */
void DisconnectStateHandler::Exit() { }
