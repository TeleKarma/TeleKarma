/*
 *
 * pcsst.cpp
 *
 * TeleKarma's extension of Opal's OpalPCSSEndPoint.
 *
 */


#include <ptlib.h>
#include <sip/sip.h>

#include "pcss.h"
#include "telephony.h"


TkPCSSEndPoint::TkPCSSEndPoint(TelephonyIfc & mgr) : OpalPCSSEndPoint(mgr) { }


PBoolean TkPCSSEndPoint::OnShowIncoming(const OpalPCSSConnection & connection) {
	return AcceptIncomingCall(connection.GetToken());
}


PBoolean TkPCSSEndPoint::OnShowOutgoing(const OpalPCSSConnection & connection) {
	return PTrue;
}

// End of File ///////////////////////////////////////////////////////////////
