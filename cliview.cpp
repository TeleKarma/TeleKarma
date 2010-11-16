#include "cliview.h"

#ifdef WIN32
#include <conio.h>			// for MS-specific _kbhit() and _getch()
#endif
#include "action.h"
#include "clicontext.h"
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
	controller->Resume();

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
//	controller->Backspace(24);
	cout << "done.";
//	controller->Space(19);
	cout << endl;
//	cout << "Identifying STUN client type... " << controller->GetSTUNType() << "." << endl;
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

	/* XXX HACK If we pass false to start() it won't create a new thread,
	 * so this while true won't be necessary. */
	while(true);
}

void CLIView::OnReceivedLine(Arguments & line) {

	/*XXX: Maybe use a state pattern here? */
	switch(inputState) {
	case CLIVIEW_INPUT_DEST:
		dest = parseArgument(line, PString(DEST));
		Dial(dest);
		EnterState(CLIVIEW_INPUT_AUTO);
		break;
	default:
		PCLIStandard::OnReceivedLine(line);
	}
}

void CLIView::EnterState(CLIViewInputState state) {

	/*XXX: Use a state pattern here. */
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

PString CLIView::parseArgument(Arguments & line, PString defaultValue)
{
	if (line.GetCount() == 0) {
		return defaultValue;
	} else {
		return line[0];
	}
}

void CLIView::Register(const PString & registrar, const PString & user, const PString & password) {
	DoAction(new RegisterAction(registrar, user, password, 0));
}

void CLIView::Dial(PString & dest) {
	DoAction(new DialAction(dest, 0));
}

void CLIView::Dial(PCLI::Arguments & args, INT) {
	EnterState(CLIVIEW_INPUT_DEST);
}

void CLIView::Hold(PCLI::Arguments & args, INT) {
	DoAction(new HoldAction(0));
}

void CLIView::AutoHold(PCLI::Arguments & args, INT) {
	DoAction(new AutoHoldAction(0));
}

void CLIView::Retrieve(PCLI::Arguments & args, INT) {
	DoAction(new RetrieveAction(0));
}

void CLIView::Disconnect(PCLI::Arguments & args, INT) {
	DoAction(new DisconnectAction(0));
}

void CLIView::Quit(PCLI::Arguments & args, INT) {
	DoAction(new QuitAction(0));
}

PCLI::Context * CLIView::CreateContext()
{
	return new CLIContext(*this);
}
