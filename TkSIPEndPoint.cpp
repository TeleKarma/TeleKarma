/*
 *
 * TkSIPEndPoint.cpp
 *
 * The TeleKarma SIP VoIP call end point component.
 *
 */

#include "TkSIPEndPoint.h"


TkSIPEndPoint::TkSIPEndPoint(OpalManager & manager) : SIPEndPoint(manager) {
	PTRACE(3, "TkSIPEndPoint constructed.");
}


TkSIPEndPoint::~TkSIPEndPoint() {
	PTRACE(3, "TkSIPEndPoint destroyed.");
}


void TkSIPEndPoint::OnRegistrationStatus(const RegistrationStatus & status) {
	PTRACE(3, "   *** New SIP Endpoint status is " << SIP_PDU::GetStatusCodeDescription(status.m_reason));
	SIPEndPoint::OnRegistrationStatus(status);
}


// End of File ///////////////////////////////////////////////////////////////
