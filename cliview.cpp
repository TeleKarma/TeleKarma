#include "cliview.h"

#include <ptlib/sound.h>

#include "action.h"
#include "clicontext.h"
#include "conf.h"
#include "controller.h"
#include "model.h"
#include "sms.h"
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
	cli.currentContext->SetLocalEcho(false);
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
	cli.currentContext->SetLocalEcho(true);
	cli.commandMutex.Wait();
	if (cli.WaitForState(STATE_REGISTERED, 15000)) {
		cli.PrintMessage("done.\n\n");
		cli.SetInputHandler(cli.defaultInputHandler);
	} else {
		cli.PrintMessage("failed\n\n");
		cli.SetInputHandler(cli.registrarInputHandler);
	}
	cli.commandMutex.Signal();
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
	cli.commandMutex.Wait();
	if (cli.WaitForState(STATE_CONNECTED, 15000)) {
		cli.PrintMessage("done.\n\n");
	} else {
		cli.PrintMessage("failed.\n\n");
	}
	cli.commandMutex.Signal();
	cli.SetInputHandler(cli.defaultInputHandler);
}

CLIView::SMSDestInputHandler::SMSDestInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::SMSDestInputHandler::WaitForInput()
{
	cli.SetPrompt("Enter phone number for SMS message: ");
}

void CLIView::SMSDestInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.SetInputHandler(cli.smsMessageInputHandler);
}

CLIView::SMSMessageInputHandler::SMSMessageInputHandler(CLIView & cli, PString defaultValue) :
	InputHandler(cli, defaultValue)
	{ }

void CLIView::SMSMessageInputHandler::WaitForInput()
{
	cli.SetPrompt("Enter your SMS message: ");
}

void CLIView::SMSMessageInputHandler::ReceiveInput(PString input)
{
	InputHandler::ReceiveInput(input);
	cli.PrintMessage("\nSending SMS...");
	if (cli.SendSMS(cli.smsDestInputHandler->inputValue, inputValue)) {
		cli.PrintMessage("Done.\n\n");
	} else {
		cli.PrintMessage("Failed\n\n");
	}
	cli.SetInputHandler(cli.defaultInputHandler);
}

CLIView::Command::Command(const char * command, const PNotifier notifier, const char * help, const char * usage) :
	notifier(notifier),
	help(help),
	usage(usage),
	command(command),
	enabled(true)
	{ }

CLIView::CLIView() :
	View(),
	PCLIStandard(),
	destInputHandler(NULL),
	defaultInputHandler(NULL),
	currentInputHandler(NULL),
	state(NULL),
	dialCommand("c", PCREATE_NOTIFIER(Dial), "Place a call"),
	holdCommand("h", PCREATE_NOTIFIER(Hold), "Hold call"),
	autoHoldCommand("w", PCREATE_NOTIFIER(AutoHold), "Hold until human detected"),
	retrieveCommand("r", PCREATE_NOTIFIER(Retrieve), "Retrieve call"),
	disconnectCommand("d", PCREATE_NOTIFIER(Disconnect), "Disconnect"),
	quitCommand("q", PCREATE_NOTIFIER(Quit), "Quit"),
	stateMutex(1,1),
	commandMutex(1,1)

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
	smsDestInputHandler = new SMSDestInputHandler(*this);
	smsMessageInputHandler = new SMSMessageInputHandler(*this);

	SetInputHandler(stunInputHandler);
	SetExitCommand(PString::Empty());

	Start();

	while(true) {
		State * state = model->DequeueState();
		if(state) {
			enum StateID stateID = state->id;
			enum StatusID statusID = state->status;
			SetState(state);
			if (statusID == STATUS_AUTO_RETRIEVE || statusID == STATUS_RETRIEVE) {
				PlaySound(HUMAN_DETECTED_WAV);
			}
			if (statusID == STATUS_UNSPECIFIED) {
				UpdateHelp(stateID);
			}
			if (stateID == STATE_TERMINATING) {
				PrintMessage("\nCleaning up... (this may take a few moments)\n");
			}
			if (stateID == STATE_TERMINATED) {
				break;
			}
		}
		PThread::Sleep(100);
	}
	currentContext->Stop();
}

void CLIView::AddCommand(Command command)
{
	if (command.enabled) {
		SetCommand(command.command, command.notifier, command.help, command.usage);
	}
}

void CLIView::UpdateHelp(enum StateID stateID)
{
	bool displayHelp = false;
	dialCommand.enabled = false;
	holdCommand.enabled = false;
	autoHoldCommand.enabled = false;
	retrieveCommand.enabled = false;
	disconnectCommand.enabled = false;

	quitCommand.enabled = true;

	switch(stateID) {
		case STATE_REGISTERED:
			displayHelp = true;
			dialCommand.enabled = true;
			break;
		case STATE_HOLD:
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
			displayHelp = true;
			retrieveCommand.enabled = true;
			disconnectCommand.enabled = true;
			break;
		case STATE_CONNECTED:
			displayHelp = true;
			holdCommand.enabled = true;
			autoHoldCommand.enabled = true;
			disconnectCommand.enabled = true;
			break;
		default:
			return;
	}
	commandMutex.Wait();
	m_commands.clear();
	AddCommand(dialCommand);
	AddCommand(holdCommand);
	AddCommand(autoHoldCommand);
	AddCommand(retrieveCommand);
	AddCommand(disconnectCommand);
	AddCommand(quitCommand);
	commandMutex.Signal();
	if (displayHelp) {
		ShowHelp(*currentContext);
	}
}

void CLIView::ShowHelp(Context & context)
{
	commandMutex.Wait();
	PrintMessage("\n\n");
	PCLIStandard::ShowHelp(context);
	PrintMessage(GetPrompt());
	commandMutex.Signal();
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
		commandMutex.Wait();
		PCLIStandard::OnReceivedLine(line);
		commandMutex.Signal();
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

void CLIView::PlaySound(const PString & fileName)
{
	DoAction(new PlaySoundAction(fileName, GetTurn()));
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
	while(true);
}

bool CLIView::SendSMS(PString dest, PString message) {
	SMS * sms = new SMS(dest, message);
	sms->Send();
	delete sms;
	return false;	// added by MV - return value req'd by compiler
}

void CLIView::SendSMS(PCLI::Arguments & args, INT) {
	SetInputHandler(smsDestInputHandler);
}

PCLI::Context * CLIView::CreateContext()
{
	currentContext = new CLIContext(*this);
	return currentContext;
}
