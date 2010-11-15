#include "cliview.h"

#include "action.h"
#include "conf.h"
#include "eventqueue.h"
#include "model.h"
#include "controller.h"
#include "telekarma.h"

CLIView::CLIView() :
	View(),
	inputState(CLIVIEW_INPUT_AUTO),
	dest(PString(DEST))
	{ }

void CLIView::Main() {

	PString registrar(REGISTRAR);
	PString stunServer(STUN);
	PString user(ACCOUNT);
	PString passwd(PASSWORD);
	PString line;


	model = new Model();
	controller = new TeleKarma(model);

	cout << "Welcome to TeleKarma!" << endl << endl << flush;

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
	controller->Initialize(stunServer, user);
	controller->Backspace(24);
	cout << "done.";
	controller->Space(19);
	cout << endl;
	cout << "Identifying STUN client type... " << controller->GetSTUNType() << "." << endl;
	cout << flush;

	// Initiate registration (non-blocking)
	cout << "Registering with " << registrar << " as " << user << "... " << flush;
	Register(registrar, user, passwd);

	EnterState(CLIVIEW_INPUT_AUTO);
	SetCommand("c", PCREATE_NOTIFIER(Dial), "Place a call");
	SetCommand("q", PCREATE_NOTIFIER(Quit), "Quit");
	SetCommand("r", PCREATE_NOTIFIER(Retrieve), "Retrieve call");
//	menu << "  0-9 : touch tones" << endl;
//	menu << "  *,# : touch tones" << endl;
	SetCommand("h", PCREATE_NOTIFIER(Hold), "Hold call");
	SetCommand("w", PCREATE_NOTIFIER(AutoHold), "Hold until human detected");
	SetCommand("d", PCREATE_NOTIFIER(Disconnect), "Disconnect");
	Start();
}

void CLIView::OnReceivedLine(Arguments & line) {

	cout << "Line='" << line[0] << "'\n";
	/*XXX: Maybe use a state pattern here? */
	switch(inputState) {
	case CLIVIEW_INPUT_DEST:
		Dial(dest);
		EnterState(CLIVIEW_INPUT_AUTO);
		break;
	default:
		PCLIStandard::OnReceivedLine(line);
	}
}

void CLIView::EnterState(CLIViewInputState state) {

	/*XXX: Use a state pattern here. */
	cout << "Enter state = " << state << "\n";
	switch(state) {
	case CLIVIEW_INPUT_DEST:
		SetPrompt("Enter SIP address [" DEST "]: ");
		break;

	default:
		SetPrompt("TeleKarma>");
		break;
	}
	inputState = state;
}

void CLIView::Register(const PString & registrar, const PString & user, const PString & password) {
	DoAction(new RegisterAction(registrar, user, password));
}

void CLIView::Dial(PString & dest) {
	DoAction(new DialAction(dest));
}

void CLIView::Dial(PCLI::Arguments & args, INT) {
	EnterState(CLIVIEW_INPUT_DEST);
}

void CLIView::Hold(PCLI::Arguments & args, INT) {
	DoAction(new HoldAction());
}

void CLIView::AutoHold(PCLI::Arguments & args, INT) {
	DoAction(new AutoHoldAction());
}

void CLIView::Retrieve(PCLI::Arguments & args, INT) {
	DoAction(new RetrieveAction());
}

void CLIView::Disconnect(PCLI::Arguments & args, INT) {
	DoAction(new DisconnectAction());
}

void CLIView::Quit(PCLI::Arguments & args, INT) {
	DoAction(new QuitAction());
}

