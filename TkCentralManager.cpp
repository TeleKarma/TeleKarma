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
#ifndef __LINUX__
#include <conio.h>
#endif

#include "TkCentralManager.h"
//#include "TkCommandLineView.h"
#include "TkOpalManager.h"
#include "Timestamp.h"


TkCentralManager::TkCentralManager() : PProcess("TeleKarma"), opal(NULL),
		console(new PConsoleChannel(PConsoleChannel::StandardInput)) {
	cout << "Welcome to TeleKarma!" << endl << endl;
	PString logFName("log");
	Timestamp::appendTo(logFName);
	logFName += ".txt";
	PTrace::Initialise(5, logFName);
	PTRACE(3, "TkCentralManager constructed.");
	state = 'x';
}


TkCentralManager::~TkCentralManager() {
	if (opal != NULL) {
		delete opal;
	}
	if (console != NULL) {
		delete console;
	}
	cout << "Thank you for using TeleKarma." << endl;
	PTRACE(3, "TkCentralManager destroyed.");
}


void TkCentralManager::Main() {

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
#ifdef __LINUX__
		c = getchar();
#else
		c = getch();
#endif
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

	opal = new TkOpalManager();
	opal->Initialise(stunServer, user);
	
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
		cout << "connected." << endl << endl;
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
	help << "  s   : Put call on hold and start Anti_IVR" << endl;
	help << "  d   : Disconnect the call" << endl;
	help << "  z   : Toggle recording" << endl;
	help << "  p   : Play audio file test" << endl;
	help << "  q   : Exit" << endl;
	help << "  ?   : Help" << endl;

	cout << help << endl;
	PString recFName;
	char ch;

	bool inputExists;
	bool reprintCmdPrompt;
	PString line;

	cout << "Command ? " << flush;

	while (true) {
#ifdef WIN32
		// character-by-character processing (non-blocking)
		inputExists = _kbhit();
		if (inputExists != 0) {
			ch = _getch();
		} else {
			PThread::Sleep(1);
			continue;
		}
#else
		// line-by-line processing (blocks!!)
		ch = (char)console->peek();
		if (console->eof()) {
			cout << endl << "Console gone - menu disabled" << endl;
			return;
		}
		(*console) >> line;
		line = line.LeftTrim();
		ch = line[0];
		line = line.Mid(1).Trim();
#endif

		PTRACE(3, "Console user input is " << ch);
		reprintCmdPrompt = true;
		switch (tolower(ch)) {
		case 'x' :
		case 'q' :
			cout << ch << endl;
			return;

		case '?' :
			cout << ch << endl << help << endl;
			break;

		case 'z':
			recFName = "rec";
			Timestamp::appendTo(recFName);
			recFName += ".wav";
			cout << ch << endl << opal->ToggleRecording(recFName) << endl;
			break;

		case 'd' :
			if (opal->EndCurrentCall()) {
				cout << ch << endl << "Call terminated." << endl;
			} else {
				cout << ch << endl << "Not connected." << endl;
			}
			break;

		case 'p' :
			if (opal->HasActiveCall()) {
				opal->SendAudioFile("\\TeleKarma\\test.wav");
				cout << ch << endl << "Theoretically sending audio..." << endl;
			} else {
				cout << ch << endl << "Not connected." << endl;
			}
			break;

		case 'c' :
			if (opal->HasActiveCall()) {
				cout << ch << endl << "Disconnect the active call before making another call." << endl;
			} else {
				PString dest = "sip:500@ekiga.net";
				cout << ch << endl << "Please enter the SIP address to call [" << dest << "]: " << flush;
				(*console) >> line;
				line = line.Trim();
				if (!line.IsEmpty()) {
					dest = line;
				}
				cout << "Calling " << dest << "... " << flush;
				opal->StartCall(dest);
				cout << "connected." << endl << endl;
				cout << help << endl;
			}
			break;

		case 's':
			opal->WaitForHuman();
			break;
		//case 'f' :
		//	SendTone('x');
		//	break;

		default:
			if (isdigit(ch) || ch == '*' || ch == '#') {
				opal->SendTone(ch);
				cout << ch << endl << "Sent DTML tone for '" << ch << "'" << endl;
			} else {
				reprintCmdPrompt = false;
			}
			break;
		}
		
		if (reprintCmdPrompt) {
			cout << "Command ? " << flush;
		}
	}
	
}

// End of File ///////////////////////////////////////////////////////////////
