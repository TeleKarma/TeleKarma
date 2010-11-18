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

CLIView::InputHandler::InputHandler(CLIView & cli, PString defaultValue) :
	cli(cli),
	inputValue(defaultValue)
	{ }

void CLIView::InputHandler::WaitForInput()
{
	cli.SetPrompt("Telekarma>");
	cli.SetInputHandler(NULL);
}

void CLIView::InputHandler::ReceiveInput(PString input)
{
	if (!input.IsEmpty()) {
		inputValue = input;
	}
}

CLIView::STUNInputHandler::STUNInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::STUNInputHandler::WaitForInput()
{
	cli.SetPrompt("Please enter your SIP Registrar's STUN address [" + inputValue + "]: ");
}

void CLIView::STUNInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.SetInputHandler(cli.registrarInputHandler);
}

CLIView::RegistrarInputHandler::RegistrarInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::RegistrarInputHandler::WaitForInput()
{
	cli.SetPrompt("Please enter your SIP Registrar's address [" + inputValue + "]: ");

}

void CLIView::RegistrarInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.SetInputHandler(cli.userInputHandler);
}

CLIView::UserInputHandler::UserInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::UserInputHandler::WaitForInput()
{
	cli.SetPrompt("Please enter your SIP user name: ");
}

void CLIView::UserInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.SetInputHandler(cli.passwordInputHandler);
}

CLIView::PasswordInputHandler::PasswordInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::PasswordInputHandler::WaitForInput()
{
	cli.SetPrompt("Please enter your SIP password:");
}

void CLIView::PasswordInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.Register(cli.registrarInputHandler->inputValue,
		     cli.userInputHandler->inputValue,
		     this->inputValue);
	cli.SetInputHandler(cli.defaultInputHandler);
}


CLIView::DestInputHandler::DestInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::DestInputHandler::WaitForInput()
{
	cli.SetPrompt("Enter SIP address [" + inputValue + "]: ");
}

void CLIView::DestInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
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

	model = new Model();
	controller = new TeleKarma(model);
	controller->Resume();

	/* Setup input handlers. */
	defaultInputHandler = new InputHandler(*this);
	stunInputHandler = new STUNInputHandler(*this, STUN);
	registrarInputHandler = new RegistrarInputHandler(*this, REGISTRAR);
	userInputHandler = new UserInputHandler(*this, ACCOUNT);
	passwordInputHandler = new PasswordInputHandler(*this);
	destInputHandler = new DestInputHandler(*this, DEST);

	SetCommand("c", PCREATE_NOTIFIER(Dial), "Place a call");
	SetCommand("q", PCREATE_NOTIFIER(Quit), "Quit");
	SetCommand("r", PCREATE_NOTIFIER(Retrieve), "Retrieve call");
//	menu << "  0-9 : touch tones" << endl;
//	menu << "  *,# : touch tones" << endl;
	SetCommand("h", PCREATE_NOTIFIER(Hold), "Hold call");
	SetCommand("w", PCREATE_NOTIFIER(AutoHold), "Hold until human detected");
	SetCommand("d", PCREATE_NOTIFIER(Disconnect), "Disconnect");

	SetInputHandler(stunInputHandler);

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

void CLIView::SetInputHandler(InputHandler * handler)
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
