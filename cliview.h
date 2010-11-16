
#ifndef _CLIVIEW_H_
#define _CLIVIEW_H_

#include <ptclib/cli.h>

#include "view.h"

class TeleKarma;

class CLIView;

enum CLIViewInputState {
	CLIVIEW_INPUT_AUTO,
	CLIVIEW_INPUT_DEST
};

class CLIView : public PCLIStandard,  public View {

	PCLASSINFO(CLIView, PCLIStandard);

	public:
		CLIView(); 
		void Main();

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

		enum CLIViewInputState inputState;
		PString dest;
		void EnterState(CLIViewInputState state);
		
};

#endif //_CLIVIEW_H_
