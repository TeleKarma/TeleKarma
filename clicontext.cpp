
#include "clicontext.h"

/* This and ProcessInput are needed because otherwise the base PCLI::Context
 * class will ignore empty lines.  We interpret an empty input as
 * 'use the default value' (e.g. registrar, username, etc.) so we can't ignore
 * empty lines.
 */
void CLIContext::OnCompletedLine()
{
	PCaselessString line = m_commandLine.Trim();
	if (line.IsEmpty()) {
		PCLI::Arguments args = PCLI::Arguments(*this, line);
		m_cli.OnReceivedLine(args);
	} else {
		Context::OnCompletedLine();
	}
}

bool CLIContext::ProcessInput(int ch)
{
	if ((ch == '\n' || ch == '\r') && m_commandLine.IsEmpty()) {
		OnCompletedLine();
		return WritePrompt();
	}
	return Context::ProcessInput(ch);
}