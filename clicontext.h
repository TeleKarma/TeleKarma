
#ifndef _CLICONTEXT_H_
#define _CLICONTEXT_H_

#include <ptclib/cli.h>

#include "cliview.h"

class CLIContext : public PCLI::Context
{
	PCLASSINFO(CLIContext, PCLI::Context);

	CLIContext(CLIView & cli) : PCLI::Context(cli) { }
	public:
		void OnCompletedLine();
		bool ProcessInput(int ch);
};

#endif // _CLICONTEXT_H_
