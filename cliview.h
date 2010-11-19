
#ifndef _CLIVIEW_H_
#define _CLIVIEW_H_

#include <ptclib/cli.h>

#include "view.h"

class CLIView;

class CLIContext;
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

	public:
		CLIView(); 
		~CLIView() { }
		void Main();

	protected:
		PCLI::Context * CreateContext();

	private:
		void PrintMessage(PString message);
		void SetInputHandler(InputHandler * handler);
		void OnReceivedLine(Arguments & line);
		void Initialize(PString & stunServer);
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(PString & dest);

		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Dial);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Hold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, AutoHold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Retrieve);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Disconnect);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Quit);

		InputHandler * defaultInputHandler;
		STUNInputHandler * stunInputHandler;
		RegistrarInputHandler * registrarInputHandler;
		UserInputHandler * userInputHandler;
		PasswordInputHandler * passwordInputHandler;
		DestInputHandler * destInputHandler;
		InputHandler * currentInputHandler;

		int turn;
	friend class InputHandler;
	friend class STUNInputHandler;
	friend class RegistrarInputHandler;
	friend class UserInputHandler;
	friend class PasswordInputHandler;
	friend class DestInputHandler;
};

#endif //_CLIVIEW_H_
