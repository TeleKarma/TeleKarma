
#ifndef _CLIVIEW_H_
#define _CLIVIEW_H_

#include <ptclib/cli.h>

#include "view.h"

class TeleKarma;

class CLIView : public PCLIStandard,  public View {

	PCLASSINFO(CLIView, PCLIStandard);

	public:
		CLIView(TeleKarma * controller) : View(controller) { }
		void Run();

	private:
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(const PString & dest);

//		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Dial);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Hold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, AutoHold);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Retrieve);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Disconnect);
		PDECLARE_NOTIFIER(PCLI::Arguments, CLIView, Quit);

};

#endif //_CLIVIEW_H_
