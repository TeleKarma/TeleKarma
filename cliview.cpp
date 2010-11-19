#include "cliview.h"

#include "action.h"
#include "clicontext.h"
#include "conf.h"
#include "controller.h"
#include "eventqueue.h"
#include "model.h"
#include "state.h"
#include "telekarma.h"

CLIView::InputHandler::InputHandler(CLIView & cli, PString defaultValue) :
	cli(cli),
	inputValue(defaultValue)
	{ }

void CLIView::InputHandler::WaitForInput()
{
	cli.SetPrompt("Input your command here (press ? for help)>");
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
	cli.PrintMessage("\nInitializing telephony system (this may take a moment)...");
	cli.Initialize(inputValue);
	if (cli.WaitForState(STATE_INITIALIZED, 15000)) {
		cli.PrintMessage("done.\n\n");
		cli.SetInputHandler(cli.registrarInputHandler);
	} else {
		cli.PrintMessage("failed.\n\n");
		cli.SetInputHandler(cli.stunInputHandler);
	}
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
	cli.SetPrompt("Please enter your SIP password: ");
}

void CLIView::PasswordInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.PrintMessage("\nRegistering with " +
			 cli.registrarInputHandler->inputValue +
			 " as " + cli.userInputHandler->inputValue + "...");
	cli.Register(cli.registrarInputHandler->inputValue,
		     cli.userInputHandler->inputValue,
		     this->inputValue);
	if (cli.WaitForState(STATE_REGISTERED, 15000)) {
		cli.PrintMessage("done.\n\n");
		cli.SetInputHandler(cli.defaultInputHandler);
	} else {
		cli.PrintMessage("failed\n\n");
		cli.SetInputHandler(cli.registrarInputHandler);
	}
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
	cli.PrintMessage("\nDialing " + inputValue + " ...");
	cli.Dial(inputValue);
	if (cli.WaitForState(STATE_CONNECTED, 15000)) {
		cli.PrintMessage("done.\n\n");
	} else {
		cli.PrintMessage("failed.\n\n");
	}
	cli.SetInputHandler(cli.defaultInputHandler);
}

CLIView::CLIView() :
	View(),
	destInputHandler(NULL),
	defaultInputHandler(NULL),
	currentInputHandler(NULL),
	state(NULL),
	stateMutex(1,1)
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

	while(true) {
		State * state = model->DequeueState();
		if(state) {
			SetState(state);
		}
		PThread::Sleep(100);
	}
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

void CLIView::PrintMessage(PString message)
{
	cout << message << flush;
}

bool CLIView::WaitForState(enum StateID stateToWaitFor, int timeout)
{
	bool ret = false;
	int timeWaited = 0;
	int sleepTime = 100;
	do {
		State * currentState = GetStateWithLock();
		if (currentState->status == STATUS_FAILED) {
			ret = false;
			goto cleanup;
		}
		if (currentState->id == stateToWaitFor) {
			ret = true;
			goto cleanup;
		}
		ReleaseStateLock();
		PThread::Sleep(sleepTime);
	} while(timeWaited < timeout);
	goto exit;
cleanup:
	ReleaseStateLock();
exit:
	return ret;
}

void CLIView::SetState(State * newState)
{
	stateMutex.Wait();
	if (state) {
		delete state;
	}
	state = newState;
	stateMutex.Signal();
}

State * CLIView::GetStateWithLock()
{
	stateMutex.Wait();
	return state;
}

void CLIView::ReleaseStateLock()
{
	stateMutex.Signal();
}

int CLIView::GetTurn()
{
	int currentTurn;
	stateMutex.Wait();
	if (state) {
		currentTurn = state->turn;
	} else {
		currentTurn = 0;
	}
	stateMutex.Signal();
	return currentTurn;
}

void CLIView::Initialize(PString & stunServer)
{
	DoAction(new InitializeAction(stunServer, GetTurn()));
}

void CLIView::Register(const PString & registrar, const PString & user, const PString & password) {
	DoAction(new RegisterAction(registrar, user, password, GetTurn()));
}

void CLIView::Dial(PString & dest) {
	DoAction(new DialAction(dest, GetTurn()));
}

void CLIView::Dial(PCLI::Arguments & args, INT)
{
	SetInputHandler(destInputHandler);
}

void CLIView::Hold(PCLI::Arguments & args, INT) {
	DoAction(new HoldAction(GetTurn()));
}

void CLIView::AutoHold(PCLI::Arguments & args, INT) {
	DoAction(new AutoHoldAction(GetTurn()));
}

void CLIView::Retrieve(PCLI::Arguments & args, INT) {
	DoAction(new RetrieveAction(GetTurn()));
}

void CLIView::Disconnect(PCLI::Arguments & args, INT) {
	DoAction(new DisconnectAction(GetTurn()));
}

void CLIView::Quit(PCLI::Arguments & args, INT) {
	DoAction(new QuitAction(GetTurn()));
}

PCLI::Context * CLIView::CreateContext()
{
	return new CLIContext(*this);
}
