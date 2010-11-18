#include "cliview.h"

#ifdef WIN32
#include <conio.h>			// for MS-specific _kbhit() and _getch()
#endif
#include <assert.h>

#include "action.h"
#include "clicontext.h"
#include "conf.h"
#include "eventqueue.h"
#include "model.h"
#include "controller.h"
#include "telekarma.h"

CLIViewInputHandler::CLIViewInputHandler(CLIView & cli, PString defaultValue) :
	cli(cli),
	inputValue(defaultValue)
	{ }

void CLIViewInputHandler::WaitForInput()
{
	cli.SetPrompt("Telekarma>");
	cli.SetInputHandler(NULL);
}

void CLIViewInputHandler::ReceiveInput(PString input)
{
	if (!input.IsEmpty()) {
		inputValue = input;
	}
}

CLIViewPasswordInputHandler::CLIViewPasswordInputHandler(CLIView & cli, PString defaultValue) :
	CLIViewInputHandler(cli, defaultValue)
	{ }

void CLIViewPasswordInputHandler::WaitForInput()
{
	cli.SetPrompt("Please enter your SIP password:");
}

void CLIViewPasswordInputHandler::ReceiveInput(PString input)
{
	CLIViewInputHandler::ReceiveInput(input);
	cli.Register(cli.registrar, cli.user, inputValue);
	cli.SetInputHandler(cli.defaultInputHandler);
}


CLIViewDestInputHandler::CLIViewDestInputHandler(CLIView & cli, PString defaultValue) :
	CLIViewInputHandler(cli, defaultValue)
	{ }

void CLIViewDestInputHandler::WaitForInput()
{
	cli.SetPrompt("Enter SIP address [" + inputValue + "]: ");
}

void CLIViewDestInputHandler::ReceiveInput(PString input)
{
	CLIViewInputHandler::ReceiveInput(input);
	cli.Dial(inputValue);
	cli.SetInputHandler(cli.defaultInputHandler);
}

CLIView::CLIView() :
	View(),
	destInputHandler(NULL),
	defaultInputHandler(NULL),
	currentInputHandler(NULL)
	{ }

void CLIView::Main() {

	registrar = REGISTRAR;
	PString stunServer(STUN);
	user = ACCOUNT;
	PString passwd(PASSWORD);
	PString line;


	model = new Model();
	controller = new TeleKarma(model);
	controller->Resume();

	/* Setup input handlers. */
	defaultInputHandler = new CLIViewInputHandler(*this);
	passwordInputHandler = new CLIViewPasswordInputHandler(*this);
	destInputHandler = new CLIViewDestInputHandler(*this, DEST);

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



	SetCommand("c", PCREATE_NOTIFIER(Dial), "Place a call");
	SetCommand("q", PCREATE_NOTIFIER(Quit), "Quit");
	SetCommand("r", PCREATE_NOTIFIER(Retrieve), "Retrieve call");
//	menu << "  0-9 : touch tones" << endl;
//	menu << "  *,# : touch tones" << endl;
	SetCommand("h", PCREATE_NOTIFIER(Hold), "Hold call");
	SetCommand("w", PCREATE_NOTIFIER(AutoHold), "Hold until human detected");
	SetCommand("d", PCREATE_NOTIFIER(Disconnect), "Disconnect");

	SetInputHandler(passwordInputHandler);

	Start();

	/* XXX HACK If we pass false to start() it won't create a new thread,
	 * so this while true won't be necessary. */
	while(true);
}

void CLIView::OnReceivedLine(Arguments & line)
{
	if (currentInputHandler) {
		if (line.GetCount() == 0) {
			currentInputHandler->ReceiveInput(PString::Empty());
		} else {
			currentInputHandler->ReceiveInput(line[0]);
		}
	} else {
		PCLIStandard::OnReceivedLine(line);
	}
}

void CLIView::SetInputHandler(CLIViewInputHandler * handler)
{
	currentInputHandler = handler;
	if (currentInputHandler) {
		handler->WaitForInput();
	}
}

void CLIView::Register(const PString & registrar, const PString & user, const PString & password) {
	DoAction(new RegisterAction(registrar, user, password, 0));
}

void CLIView::Dial(PString & dest) {
	DoAction(new DialAction(dest, 0));
}

void CLIView::Dial(PCLI::Arguments & args, INT) 
{
	SetInputHandler(destInputHandler);
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
