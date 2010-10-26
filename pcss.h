/*
 *
 * pcss.h
 *
 * TeleKarma's extension of Opal's OpalPCSSEndPoint.
 *
 */

#ifndef _PCSS_H_
#define _PCSS_H_

#include <opal/manager.h>
#include <opal/pcss.h>

class TelephonyIfc;


class TkPCSSEndPoint : public OpalPCSSEndPoint {

  PCLASSINFO(TkPCSSEndPoint, OpalPCSSEndPoint);

  public:
    TkPCSSEndPoint(TelephonyIfc & manager);

    virtual PBoolean OnShowIncoming(const OpalPCSSConnection & connection);
    virtual PBoolean OnShowOutgoing(const OpalPCSSConnection & connection);

	//These are no longer used, but may be useful as a reference for later development.
    //PBoolean SetSoundDevice(PArgList & args, const char * optionName, PSoundChannel::Directions dir);
    //PString incomingConnectionToken;
    //bool    autoAnswer;
};


#endif // _PCSS_H_

// End of File ///////////////////////////////////////////////////////////////
