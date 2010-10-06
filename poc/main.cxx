#include <opal/manager.h>
#include <opal/pcss.h>
#include <sip/sipep.h>

class MyPCSSEndPoint : public OpalPCSSEndPoint
{
	PCLASSINFO(MyPCSSEndPoint, OpalPCSSEndPoint);
	public:
		MyPCSSEndPoint(OpalManager & manager);
		virtual PBoolean OnShowIncoming(const OpalPCSSConnection & connection);
		virtual PBoolean OnShowOutgoing(const OpalPCSSConnection & connection);
};

MyPCSSEndPoint::MyPCSSEndPoint(OpalManager & manager) : OpalPCSSEndPoint(manager)
{
}

PBoolean MyPCSSEndPoint::OnShowIncoming(const OpalPCSSConnection & connection)
{
	return PTrue;
}

PBoolean MyPCSSEndPoint::OnShowOutgoing(const OpalPCSSConnection & connection)
{
	return PTrue;
}

class MySIPEndPoint : public SIPEndPoint
{
	PCLASSINFO(MySIPEndPoint, SIPEndPoint);
	public:
		MySIPEndPoint(OpalManager & manager);
		void OnRegistrationStatus(const RegistrationStatus & status);
};

MySIPEndPoint::MySIPEndPoint(OpalManager & manager)
	: SIPEndPoint(manager)
{

}

void MySIPEndPoint::OnRegistrationStatus(const RegistrationStatus & status)
{
	cout << "New SIP Endpoint status is " << 
			SIP_PDU::GetStatusCodeDescription(status.m_reason) << "\n";
	SIPEndPoint::OnRegistrationStatus(status);
}

class MyManager: public OpalManager
{
	PCLASSINFO(MyManager, OpalManager);
	public:
		MyManager();
		void OnConnected(OpalConnection & connection);
		void OnAlerting(OpalConnection & connection);
		void OnNewConnection(OpalConnection & connection);
		void OnReleased(OpalConnection & connection);
};

MyManager::MyManager()
	: OpalManager()
{
}

void MyManager::OnConnected(OpalConnection & connection)
{
	printf("%s\n", __func__);
	OpalManager::OnConnected(connection);
}

void MyManager::OnAlerting(OpalConnection & connection)
{
	printf("%s\n", __func__);
	OpalManager::OnAlerting(connection);
}

void MyManager::OnNewConnection(OpalConnection & connection)
{
	printf("%s\n", __func__);
	OpalManager::OnNewConnection(connection);
}

void MyManager::OnReleased(OpalConnection & connection)
{
	printf("%s\n", __func__);
	OpalManager::OnReleased(connection);
}

class MyProcess: public PProcess
{
	//This macro defines some necessary functions
	//See: http://www.opalvoip.org/docs/ptlib-v2_8/d6/d1e/object_8h.html
	PCLASSINFO(MyProcess, PProcess);
	public:
		MyProcess();

		void Main();
};


MyProcess::MyProcess()
	: PProcess("Test")
{
}

/* This macro:
 * 1. Defines the main() function.
 * 2. Creates an instance of MyProcess.
 * 3. Calls instance->PreInitialise() which is inherited from PProcess.
 * 4. Calls instance->InternalMain() which id inherited from PPrcoess
 * 	4 a) instance->InternalMain() calls instance->Main() which we define.
 */
PCREATE_PROCESS(MyProcess)

void MyProcess::Main()
{
	PString aor;
	PString USER;
	PString PASSWORD;
	if (USER.GetLength() == 0 || PASSWORD.GetLength() == 0) {
		cout << "Please set your username and password in the code.\n";
		return;
	}
	PTrace::Initialise(5, "log");

	MyManager manager = MyManager();
	//TODO: Research why the NAT type is important.
	PSTUNClient::NatTypes nat = manager.SetSTUNServer("stun.ekiga.net");
	MyPCSSEndPoint soundEp = MyPCSSEndPoint(manager);
	cout << "Sound output = " << soundEp.GetSoundChannelPlayDevice() << "\n";
	cout << "Media formats = " << soundEp.GetMediaFormats() << "\n";
	cout << "Registered formats = " << OpalMediaFormat::GetAllRegisteredMediaFormats() << "\n";

	MySIPEndPoint endPoint = MySIPEndPoint(manager);
	endPoint.SetDefaultLocalPartyName(USER);
	endPoint.StartListeners(endPoint.GetDefaultListeners());
	SIPRegister::Params params;
	params.m_registrarAddress = "ekiga.net";
	params.m_addressOfRecord = USER;
	params.m_password = PASSWORD;
	cout << "Registering with " << params.m_registrarAddress <<
					" this may take a while...\n";
	endPoint.Register(params, aor);
	cout << "timeout in ..." << flush;
	int i;
	for(i = 20; i > 0; i--) {
		if (endPoint.IsRegistered(aor)) {
			break;
		}
		cout << i << " " << flush;
		sleep(1);
	}
	if (i > 0) {
			cout << "Succeeded aor=" << aor << "\n";
			cout << "Calling...\n";
			PSafePtr<OpalCall> call = manager.SetUpCall("pc:*", "sip:500@ekiga.net");
			while(!call);
			call->StartRecording("test.wav");
			while(1);
	} else {
		cout << "Failed\n";
	}
}
