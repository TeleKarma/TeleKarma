/*
 *
 * TkCentralManager.cpp
 *
 * The TeleKarma Central Manager component.
 *
 */

#include <ptlib.h>
#include <opal/buildopts.h>
#include <string.h>
#include <conio.h>

#include "TkCentralManager.h"
//#include "TkCommandLineView.h"
#include "TkOpalManager.h"


TkCentralManager::TkCentralManager() : PProcess("TeleKarma"), opal(NULL) {
	cout << "Welcome to TeleKarma!" << endl << endl;
	PTrace::Initialise(5, "ptrace_log");
	PTRACE(3, "TkCentralManager constructed.");
	state = 'x';
}


TkCentralManager::~TkCentralManager() {
	if (opal != NULL) {
		delete opal;
	}
	cout << "Thank you for using TeleKarma." << endl;
	PTRACE(3, "TkCentralManager destroyed.");
}


void TkCentralManager::Main() {

	PConsoleChannel * console = new PConsoleChannel(PConsoleChannel::StandardInput);
	
	PString registrar = "ekiga.net";
	PString stunServer = "stun.ekiga.net";
	PString user = "";
	PString passwd = "";
	PString dest = "sip:500@ekiga.net";
	//PString SIP_ADDRESS = "sip:*0131800xxxxxxx@ekiga.net";
	PString line;

	cout << "Please enter your SIP Registrar's address [" << registrar << "]: " << flush;
	(*console) >> line;
	line = line.Trim();
	if (!line.IsEmpty()) {
		registrar = line;
	}

	cout << "Please enter your SIP Registrar's STUN address [" << stunServer << "]: " << flush;
	(*console) >> line;
	line = line.Trim();
	if (!line.IsEmpty()) {
		stunServer = line;
	}

	cout << "Please enter your SIP user name: " << flush;
	(*console) >> line;
	line = line.Trim();
	if (!line.IsEmpty()) {
		user = line;
	}

	cout << "Please enter your SIP password: " << flush;
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
		c = getch();
		// break the tight loop
		PThread::Sleep(1);
	}
	cout << endl;

	cout << "Please enter the SIP address to call [" << dest << "]: " << flush;
	(*console) >> line;
	line = line.Trim();
	if (!line.IsEmpty()) {
		dest = line;
	}

	delete console;

	cout << endl;
	
	/*
	cout << "=============================================================" << endl;
	cout << "SIP Server Address: '" << registrar << "'" << endl;
	cout << "STUN Server Address: '" << stunServer << "'" << endl;
	cout << "User name: '" << user << "'" << endl;
	cout << "Password: '" << passwd << "'" << endl;
	cout << "SIP Destination: '" << dest << "'" << endl;
	cout << "=============================================================" << endl;
	*/

	cout << "Initializing telephony system... (this may take a moment)" << flush;

	opal = new TkOpalManager(stunServer, user);
	
	for (int i = 0; i < 24; ++i) {
		cout << "\b";
	}
	cout << "done.";
	for (int i = 0; i < 19; ++i) {
		cout << " ";
	}
	cout << endl;
	
	PSTUNClient * stunClient = opal->GetSTUNClient();
	if (stunClient != NULL) {
		cout << "STUN type is '" << stunClient->GetNatTypeName() << "'." << endl;
	}


	cout << "Registering with " << registrar << " [" << stunServer << "] as user " << user << "... " << flush;
	if (!(opal->Register(registrar, user, passwd))) {
		cout << " failed." << endl;
		PThread::Sleep(2000);
	} else {
		PThread::Sleep(125);								// just 'cause
		cout << " done." << endl;
		cout << "Calling " << dest << "... " << flush;
		opal->StartCall(dest);
		cout << "done." << endl << endl;
		Console();
		opal->EndCurrentCall();
	}
	cout << "Cleaning up... (this may take a few moments)" << endl;
}


void TkCentralManager::Console() {

	//cout << "Press ? for help." << endl;

	PStringStream help;

	help << "Select:" << endl;
	help << "  0-9 : send touch tone" << endl;
	help << "  *,# : send touch tone" << endl;
	help << "  h   : Put call on hold" << endl;
	help << "  r   : Retrieve call from hold" << endl;
	help << "  d   : Disconnect the call" << endl;
	help << "  z   : Toggle recording" << endl;
	help << "  q   : Exit" << endl;
	help << "  ?   : Help" << endl;

	PConsoleChannel console(PConsoleChannel::StandardInput);
	cout << help << endl;
	while (true) {
		cout << "Command ? " << flush;
		char ch = (char)console.peek();
		if (console.eof()) {
			cout << endl << "Console gone - menu disabled" << endl;
			return;
		}

		PString line;
		console >> line;
		line = line.LeftTrim();
		ch = line[0];
		line = line.Mid(1).Trim();

		PTRACE(3, "console in audio test is " << ch);
		switch (tolower(ch)) {
		case 'x' :
		case 'q' :
			cout << endl;
			return;

		case '?' :
			cout << endl << help << endl;
			break;

		case 'z':
			cout << endl << opal->ToggleRecording("recording.wav") << endl;
			break;

		case 'd' :
			if (opal->EndCurrentCall()) {
				cout << endl << "Call terminated." << endl;
			} else {
				cout << endl << "Not connected." << endl;
			}
			break;

		//case 'c' :
		//	StartCall(line);
		//	break;

		case 'r' :
			if (opal->RetrieveCallOnHold()) {
				cout << endl << "Call is now active." << endl;
			} else {
				cout << endl << "Not on hold or holding call lost." << endl;
			}
			break;

		case 'h':
			if (opal->HoldCurrentCall()) {
				cout << endl << "Call placed on hold." << endl;
			} else {
				cout << endl << "Not connected." << endl;
			}
			break;

		//case 'f' :
		//	SendTone('x');
		//	break;

		default:
			if (isdigit(ch) || ch == '*' || ch == '#') {
				opal->SendTone(ch);
				cout << endl << "Sent DTML tone for '" << ch << "'" << endl;
			}
			break;
		}
	}
	


}

// End of File ///////////////////////////////////////////////////////////////
