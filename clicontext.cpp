
#include "clicontext.h"

/* This and ProcessInput are needed because otherwise the base PCLI::Context
 * class will ignore empty lines.  We interpret an empty input as
 * 'use the default value' (e.g. registrar, username, etc.) so we can't ignore
 * empty lines.
 */
void CLIContext::OnCompletedLine()
{
	PCaselessString line = m_commandLine.Trim();
	if (line.IsEmpty() || IsTouchTone(line)) {
		PCLI::Arguments args = PCLI::Arguments(*this, line);
		m_cli.OnReceivedLine(args);
	} else {
		Context::OnCompletedLine();
	}
}

bool CLIContext::ProcessInput(int ch)
{
	if (ch == '\r') {
		return true;
	}
	if (ch == '\n' && m_commandLine.IsEmpty()) {
		OnCompletedLine();
		return WritePrompt();
	}
	return Context::ProcessInput(ch);
}

bool CLIContext::SetLocalEcho(bool localEcho)
{
	return readChannel->SetLocalEcho(localEcho);
}

bool CLIContext::IsTouchTone(PString line)
{
	if (line.GetLength() != 1) {
		return false;
	}
	switch(line[0]) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '#':
	case '*':
		return true;
	default:
		return false;
	}
}
