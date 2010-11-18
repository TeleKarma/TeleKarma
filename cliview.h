
#ifndef _CLIVIEW_H_
#define _CLIVIEW_H_

#include <ptclib/cli.h>

#include "view.h"

class CLIView;
class TeleKarma;

enum CLIViewInputState {
	CLIVIEW_INPUT_AUTO,
	CLIVIEW_INPUT_DEST
};

class CLIViewInputHandler {
	public:
		CLIViewInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
		virtual void WaitForInput();
		virtual void ReceiveInput(PString input);
		PString inputValue;

	protected:
		CLIView & cli;
};

class CLIViewSTUNInputHandler : public CLIViewInputHandler {
	public:
		CLIViewSTUNInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
		void WaitForInput();
		void ReceiveInput(PString input);
};

class CLIViewRegistrarInputHandler : public CLIViewInputHandler {
	public:
		CLIViewRegistrarInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
		void WaitForInput();
		void ReceiveInput(PString input);
};

class CLIViewUserInputHandler : public CLIViewInputHandler {
	public:
		CLIViewUserInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
		void WaitForInput();
		void ReceiveInput(PString input);
};

class CLIViewPasswordInputHandler : public CLIViewInputHandler {
	public:
		CLIViewPasswordInputHandler(CLIView &cli, PString defaultValue = PString::Empty());
		void ReceiveInput(PString input);
		void WaitForInput();
};

class CLIViewDestInputHandler : public CLIViewInputHandler {
	public:
		CLIViewDestInputHandler(CLIView & cli, PString defaultValue = PString::Empty());
		void ReceiveInput(PString input);
		void WaitForInput();
};

class CLIView : public PCLIStandard,  public View {

	PCLASSINFO(CLIView, PCLIStandard);

	public:
		CLIView(); 
		~CLIView() { }
		void Main();
		void SetInputHandler(CLIViewInputHandler * handler);

	protected:
		PCLI::Context * CreateContext();

	private:

		void OnReceivedLine(Arguments & line);
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(PString & dest);

		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Dial);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Hold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, AutoHold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Retrieve);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Disconnect);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Quit);

		CLIViewInputHandler * defaultInputHandler;
		CLIViewSTUNInputHandler * stunInputHandler;
		CLIViewRegistrarInputHandler * registrarInputHandler;
		CLIViewUserInputHandler * userInputHandler;
		CLIViewPasswordInputHandler * passwordInputHandler;
		CLIViewDestInputHandler * destInputHandler;
		CLIViewInputHandler * currentInputHandler;

	friend class CLIViewInputHandler;
	friend class CLIViewSTUNInputHandler;
	friend class CLIViewRegistrarInputHandler;
	friend class CLIViewUserInputHandler;
	friend class CLIViewPasswordInputHandler;
	friend class CLIViewDestInputHandler;
};

#endif //_CLIVIEW_H_
