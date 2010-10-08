/*
 *
 * TkSIPEndPoint.h
 *
 * The TeleKarma SIP VoIP call end point component.
 *
 */

#ifndef _TKSIPENDPOINT_H_
#define _TKSIPENDPOINT_H_

#include <opal/manager.h>
#include <sip/sipep.h>


class TkSIPEndPoint : public SIPEndPoint {
	PCLASSINFO(TkSIPEndPoint, SIPEndPoint);
	public:
		TkSIPEndPoint(OpalManager & manager);
		virtual ~TkSIPEndPoint();
		void OnRegistrationStatus(const RegistrationStatus & status);
};


#endif // _TKSIPENDPOINT_H_

// End of File ///////////////////////////////////////////////////////////////
