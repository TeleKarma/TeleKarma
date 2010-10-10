/*
 *
 * TkPCSSEndPoint.cpp
 *
 * TeleKarma's extension of Opal's OpalPCSSEndPoint.
 *
 */


#include <ptlib.h>
#include <sip/sip.h>

#include "TkPCSSEndPoint.h"
#include "TkOpalManager.h"


TkPCSSEndPoint::TkPCSSEndPoint(TkOpalManager & mgr) : OpalPCSSEndPoint(mgr) { }


PBoolean TkPCSSEndPoint::OnShowIncoming(const OpalPCSSConnection & connection) {
	return PTrue;
}


PBoolean TkPCSSEndPoint::OnShowOutgoing(const OpalPCSSConnection & connection) {
	return PTrue;
}

// End of File ///////////////////////////////////////////////////////////////
