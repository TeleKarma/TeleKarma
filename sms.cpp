
#include "sms.h"

SMS::SMS(PString dest, PString message) :
	dest(dest),
	message(message)
	{ }

SMS::~SMS() { }

bool SMS::Send()
{
	//XXX Put code to send SMS messages here.
	cout << endl << "'" << message << "' sent to " << dest << endl;
	return false;	// added by MV - return value req'd by compiler
}
