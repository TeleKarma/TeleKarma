
#ifndef _CLIVIEW_H_
#define _CLIVIEW_H_

#include <ptclib/cli.h>

#include "state.h"
#include "view.h"

class CLIView;

class CLIContext;
class PSemaphore;
class TeleKarma;

class CLIView : public PCLIStandard,  public View {

	PCLASSINFO(CLIView, PCLIStandard);

	class InputHandler {
		public:
			InputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			virtual void WaitForInput();
			virtual void ReceiveInput(PString input);
			PString inputValue;

		protected:
			CLIView & cli;
	};

	class STUNInputHandler : public InputHandler {
		public:
			STUNInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			void WaitForInput();
			void ReceiveInput(PString input);
	};

	class RegistrarInputHandler : public InputHandler {
		public:
			RegistrarInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			void WaitForInput();
			void ReceiveInput(PString input);
	};

	class UserInputHandler : public InputHandler {
		public:
			UserInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			void WaitForInput();
			void ReceiveInput(PString input);
	};

	class PasswordInputHandler : public InputHandler {
		public:
			PasswordInputHandler(CLIView &cli, PString defaultValue = PString::Empty());
			void ReceiveInput(PString input);
			void WaitForInput();
	};

	class DestInputHandler : public InputHandler {
		public:
			DestInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			void ReceiveInput(PString input);
			void WaitForInput();
	};

	class SMSDestInputHandler : public InputHandler {
		public:
			SMSDestInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			void ReceiveInput(PString input);
			void WaitForInput();
	};

	class SMSMessageInputHandler : public InputHandler {
		public:
			SMSMessageInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
			void ReceiveInput(PString input);
			void WaitForInput();
	};

	class Command
	{
		public:
			Command(const char * command, const PNotifier notifier, const char * help, const char * usage = NULL);
			Command() { }
		private:
			const char * command;
			const PNotifier notifier;
			const char * help;
			const char * usage;
			bool enabled;

		friend class CLIView;
	};

	public:
		CLIView(); 
		~CLIView() { }
		void Main();

	protected:
		PCLI::Context * CreateContext();

	private:
		void PrintMessage(PString message);
		void SetInputHandler(InputHandler * handler);
		void AddCommand(Command command);
		void UpdateHelp(enum StateID stateID);
		void ShowHelp(Context & context);
		void OnReceivedLine(Arguments & line);
		bool WaitForState(enum StateID stateToWaitFor, int timeout);
		void Initialize(PString & stunServer);
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(PString & dest);
		void PlaySound(const PString & fileName);
		void SendTone(char tone);
		bool SendSMS(PString dest, PString message);

		void SetState(State * newState);
		State * GetStateWithLock();
		void ReleaseStateLock();

		int GetTurn();

		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Dial);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Hold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, AutoHold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Retrieve);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Disconnect);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Quit);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, SendSMS);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, NoAction);

		InputHandler * defaultInputHandler;
		STUNInputHandler * stunInputHandler;
		RegistrarInputHandler * registrarInputHandler;
		UserInputHandler * userInputHandler;
		PasswordInputHandler * passwordInputHandler;
		DestInputHandler * destInputHandler;
		SMSDestInputHandler * smsDestInputHandler;
		SMSMessageInputHandler * smsMessageInputHandler;
		InputHandler * currentInputHandler;

		Command dialCommand;
		Command holdCommand;
		Command autoHoldCommand;
		Command retrieveCommand;
		Command disconnectCommand;
		Command quitCommand;
		Command toneCommand;

		State * state;
		PSemaphore stateMutex;
		PSemaphore commandMutex;
		CLIContext * currentContext;

	friend class InputHandler;
	friend class STUNInputHandler;
	friend class RegistrarInputHandler;
	friend class UserInputHandler;
	friend class PasswordInputHandler;
	friend class DestInputHandler;
	friend class SMSDestInputHandler;
	friend class SMSMessageInputHandler;
};

#endif //_CLIVIEW_H_
