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
	/*
	incomingConnectionToken = connection.GetToken();
	if (autoAnswer) {
		AcceptIncomingConnection(incomingConnectionToken);
	} else {
		PTime now;
		cout << "\nCall on " << now.AsString("w h:mma")
			 << " from " << connection.GetRemotePartyName()
			 << ", answer (Y/N)? " << flush;
	}
	*/
	return PTrue;
}


PBoolean TkPCSSEndPoint::OnShowOutgoing(const OpalPCSSConnection & connection) {
	/*
	PTime now;
	cout << connection.GetRemotePartyName() << " is ringing on "
		 << now.AsString("w h:mma") << " ..." << endl;
	*/
	return PTrue;
}


/*
PBoolean TkPCSSEndPoint::SetSoundDevice(PArgList & args, const char * optionName, PSoundChannel::Directions dir) {
  if (!args.HasOption(optionName))
    return PTrue;

  PString dev = args.GetOptionString(optionName);

  if (dir == PSoundChannel::Player) {
    if (SetSoundChannelPlayDevice(dev))
      return PTrue;
  }
  else {
    if (SetSoundChannelRecordDevice(dev))
      return PTrue;
  }

  cerr << "Device for " << optionName << " (\"" << dev << "\") must be one of:\n";

  PStringArray names = PSoundChannel::GetDeviceNames(dir);
  for (PINDEX i = 0; i < names.GetSize(); i++)
    cerr << "  \"" << names[i] << "\"\n";

  return PFalse;
}
*/

// End of File ///////////////////////////////////////////////////////////////
