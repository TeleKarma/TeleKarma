/*
 *
 * TkCentralManager.h
 *
 * The TeleKarma Central Manager component.
 *
 */

#ifndef _TKCENTRALMANAGER_H_
#define _TKCENTRALMANAGER_H_

#include <ptlib/pprocess.h>

#include "TkOpalManager.h"


class TkCentralManager : public PProcess {

	PCLASSINFO(TkCentralManager, PProcess);

	public:
		TkCentralManager();
		virtual ~TkCentralManager();
		
		void Main();
		void Console();

	private:
		// use enum states in the future...
		char state;
		// use telephony superclass in future...
		TkOpalManager * opal;
		//TkView * view;
		PConsoleChannel * console;

};


#endif // _TKCENTRALMANAGER_H_

// End of File ///////////////////////////////////////////////////////////////
