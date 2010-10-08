/*
 *
 * TkPCSSEndPoint.h
 *
 * TeleKarma's extension of Opal's OpalPCSSEndPoint.
 *
 */

#ifndef _TKPCSSENDPOINT_H_
#define _TKPCSSENDPOINT_H_

#include <opal/manager.h>
#include <opal/pcss.h>

class TkOpalManager;


class TkPCSSEndPoint : public OpalPCSSEndPoint {

  PCLASSINFO(TkPCSSEndPoint, OpalPCSSEndPoint);

  public:
    TkPCSSEndPoint(TkOpalManager & manager);

    virtual PBoolean OnShowIncoming(const OpalPCSSConnection & connection);
    virtual PBoolean OnShowOutgoing(const OpalPCSSConnection & connection);

    //PBoolean SetSoundDevice(PArgList & args, const char * optionName, PSoundChannel::Directions dir);

    //PString incomingConnectionToken;
    //bool    autoAnswer;
};


#endif // _TKPCSSENDPOINT_H_

// End of File ///////////////////////////////////////////////////////////////
