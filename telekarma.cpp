/*
 *
 * telekarma.cpp
 *
 * Implementation of TeleKarma main application.
 *
 * See telekarma.h for full API documentation.
 *
 */

#include <ptlib.h>
#include <sys/types.h>  		// for struct stat
#include <sys/stat.h>   		// for struct stat
#include "action.h"
#include "state.h"
#include "model.h"
#include "telephony.h"
#include "telekarma.h"

#ifdef WIN32
	#define ACCESS _access		// MSFT proprietary version of C-standard access function
#else
	#define ACCESS access		// C-standard access function (for linux)
#endif

// Constructor - field initialization.
TeleKarma::TeleKarma(Model * model) :
	Controller(model),
	countdown(0),
	phone(new TelephonyIfc())
{
	// initialize the model's state
	model->SetState(new State(STATE_UNINITIALIZED, 0));
}

// Destructor - heap memory management.
TeleKarma::~TeleKarma()
{
	delete phone;
}

// Main program
void TeleKarma::Main() {
	// enter main control loop
	State * currentState = model->GetState();
	while (currentState && currentState->id != STATE_TERMINATING) {
		currentState = UpdateState(currentState);
		currentState = DoAction(model->DequeueAction(), currentState);
		PThread::Sleep(SLEEP_DURATION);
	}
	// handle extreme case
	if (!currentState) {
		SetState(new State(STATE_ERROR, -1, STATUS_UNSPECIFIED, "NULL State detected"));
		SetState(new State(STATE_TERMINATING, -2));
	}

	// TO GO: cleanup??
	// may want to delete phone explicitely here, before destructor...

	// final step
	SetState(new State(STATE_TERMINATED, -3));
}

State * TeleKarma::SetState(State * s)
{
	model->SetState(s);
	return s;
}

// React to changes in the telephony service states
State * TeleKarma::UpdateState(State * s)
{
	State * result = s;
	switch (result->id) {
		case STATE_UNINITIALIZED:
			// no state change possible
			break;
		case STATE_INITIALIZED:
			// no state change possible
			break;
		case STATE_REGISTERING:
			if (phone && phone->IsRegistered()) {
				result = SetState(new State(STATE_REGISTERED, result->turn+1));
			} else {
				if (countdown == 0) {
					// timeout
					result = SetState(new State(result->id, result->turn, STATUS_FAILED));
					result = SetState(new State(STATE_INITIALIZED, result->turn+1));
				} else {
					--countdown;
				}
			}
			break;
		case STATE_REGISTERED:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				// might do better with a more specific error
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			}
			break;
		case STATE_DIALING:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				// might do better with a more specific error
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			} else if (phone->IsConnected()) {
				result = SetState(new State(STATE_CONNECTED, result->turn+1));
			} else if (!phone->IsDialing()) {
				// concurrency kludge - note expectation in TelephonyIfc:
				// IsConnected true before IsDialing false
				if (!phone->IsConnected()) {
					result = SetState(new State(STATE_DIALING, result->turn, STATUS_FAILED, "Call failed"));
					result = SetState(new State(STATE_REGISTERED, result->turn+1));
				}
			}
			break;
		case STATE_CONNECTED:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			} else if (!phone->IsConnected()) {
				result = SetState(new State(STATE_DISCONNECTED, result->turn+1, STATUS_UNSPECIFIED, phone->DisconnectReason()));
			}
			break;
		case STATE_DISCONNECTING:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			} else if (!phone->IsConnected()) {
				result = SetState(new State(STATE_REGISTERED, result->turn+1));
			}
			break;
		case STATE_DISCONNECTED:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			} else if (!phone->IsConnected()) {
				result = SetState(new State(STATE_REGISTERED, result->turn+1));
			}
			break;
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			} else if (!phone->IsConnected()) {
				result = SetState(new State(STATE_DISCONNECTED, result->turn+1));
			} else if (phone->ToneReceived(IS_HUMAN_TONE)) {
				// XXX action required here (more to this than just updating state)
				result = SetState(new State(result->id, result->turn, STATUS_AUTO_RETRIEVE, "Human detected"));
				result = SetState(new State(STATE_CONNECTED, result->turn+1));
			}
			break;
		case STATE_HOLD:
			if (!phone) {
				result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Telephony service failed"));
				result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
			} else if (!phone->IsRegistered()) {
				result = SetState(new State(result->id, result->turn, STATUS_FAILED, "No longer registered"));
				result = SetState(new State(STATE_INITIALIZED, result->turn+1));
			} else if (!phone->IsConnected()) {
				result = SetState(new State(STATE_DISCONNECTED, result->turn+1));
			}
			break;
		case STATE_TERMINATING:
			// insensitive to telephony state
			break;
		case STATE_TERMINATED:
			// insensitive to telephony state
			break;
		case STATE_INITIALIZING:
			// initialization is blocking - if we get here, it's an error
			// fall through to default & exit with message
		default:
			result = SetState(new State(STATE_ERROR, result->turn+1, STATUS_UNSPECIFIED, "Illegal state"));
			result = SetState(new State(STATE_TERMINATING, result->turn+1));
	}
	return result;
}

State * TeleKarma::DoAction(Action * a, State * s)
{
	if (a == NULL) return s;
	State * result = s;
	if (a->turn == s->turn) {
		switch (a->id) {
			case ACTION_INITIALIZE:
				s = Initialize(a, s);
				break;
			case ACTION_REGISTER:
				s = Register(a, s);
				break;
			case ACTION_DIAL:
				s = Dial(a, s);
				break;
			case ACTION_HOLD:
				s = Hold(a, s);				// XXX Not implemented yet
				break;
			case ACTION_AUTOHOLD:
				s = AutoHold(a, s);			// XXX Not implemented yet
				break;
			case ACTION_MUTE:
				s = MuteAutoHold(a, s);		// XXX Not implemented yet
				break;
			case ACTION_RETRIEVE:
				s = Retrieve(a, s);			// XXX Not implemented yet
				break;
			case ACTION_DISCONNECT:
				s = Disconnect(a, s);
				break;
			case ACTION_QUIT:
				s = Quit(a, s);
				break;
			default:
				// simply ignore unknown actions
				break;
		}
	}
	delete a;
	return result;
}

// Initialize the application and telephony API. Blocks.
State * TeleKarma::Initialize(Action * a, State * s)
{
	if (s->id != STATE_UNINITIALIZED) return s;
	bool flag = false;
	State * result = SetState(new State(STATE_INITIALIZING, result->turn+1));
	// verify existence and type of 'logs' folder
	const char * strPath1 = "logs";
	struct stat status;
	stat(strPath1, &status);
	if ((ACCESS(strPath1, 0) == -1) || !(status.st_mode & S_IFDIR)){
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Please create a \"logs\" folder in your TeleKarma program folder."));
		flag = true;
	}
	// verify existence and type of 'recordings' folder
	const char * strPath2 = "recordings";
	stat(strPath2, &status);
	if ((ACCESS(strPath2, 0) == -1) || !(status.st_mode & S_IFDIR)){
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Please create a \"recordings\" folder in your TeleKarma program folder."));
		flag = true;
	}
	if (flag) {
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		// create log file
		PTime now;
		PString logFName("logs/log");
		logFName += now.AsString("_yyyy.MM.dd_hh.mm.ss");
		logFName += ".txt";
		PTrace::Initialise(5, logFName);
		// initialize telephony (blocking call)
		InitializeAction * ia = dynamic_cast<InitializeAction *>(a);
		if (ia != NULL) {
			phone->Initialise(ia->stunServer);
			// XXX push stun server type to model
			/*
			PSTUNClient * stunClient = phone->GetSTUNClient();
			if (stunClient != NULL) {
				model->SetStunServerType(stunClient->GetNatTypeName());
			} else {
				model->SetStunServerType("none");
			}
			*/
			result = SetState(new State(STATE_INITIALIZED, result->turn+1));
		} else {
			result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
			result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
		}
	}
	return result;
}

// Register with SIP registrar. Asynchronous.
State * TeleKarma::Register(Action * a, State * s)
{
	if (s->id != STATE_INITIALIZED) return s;
	State * result = SetState(new State(STATE_REGISTERING, result->turn+1));
	RegisterAction * ra = dynamic_cast<RegisterAction *>(a);
	if (ra == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
		result = SetState(new State(STATE_INITIALIZED, result->turn+1));
	} else if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		countdown = (REGISTRATION_TIMEOUT)/(SLEEP_DURATION);
		phone->Register(ra->registrar, ra->user, ra->password);
		// XXX push data to model
		/*
		model->SetRegistrar(ra->registrar);
		model->SetUser(ra->user);
		model->SetPassword(ra->password);
		*/
	}
	return result;
}

// Dial the indicated SIP address. Format sip addr as sip:user@domain. Asynchronous.
State * TeleKarma::Dial(Action * a, State * s)
{
	if (s->id != STATE_REGISTERED) return s;
	State * result = SetState(new State(STATE_DIALING, result->turn+1));
	DialAction * da = dynamic_cast<DialAction *>(a);
	if (da == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
		result = SetState(new State(STATE_REGISTERED, result->turn+1));
	} else if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		phone->Dial(da->dest);
		// XXX push data to model
		/*
		model->SetDestination(da->dest);
		*/
	}
	return result;
}

// Play a DTMF tone over phone connection.
State * TeleKarma::SendTone(Action * a, State * s)
{
	if (s->id != STATE_CONNECTED) return s;
	State * result = s;
	SendToneAction * sta = dynamic_cast<SendToneAction *>(a);
	if (sta == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
	} else if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		phone->SendTone(sta->tone);
	}
	return result;
}

// XXX implementation to go
State * Hold(Action * a, State * s)
{

}

// XXX implementation to go
State * AutoHold(Action * a, State * s)
{

}

// XXX implementation to go
State * MuteAutoHold(Action * a, State * s)
{

}

// XXX implementation to go
State * Retrieve(Action * a, State * s)
{

}

// Disconnect the current call. Asynchronous.
State * TeleKarma::Disconnect(Action * a, State * s)
{
	if (IsConnectedState(s)) return s;
	State * result = SetState(new State(STATE_DISCONNECTING, result->turn+1));
	if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		phone->Disconnect();
	}
	return result;
}

// Determine whether we can hang up in the given state
bool TeleKarma::IsConnectedState(State * s)
{
	if (s == NULL) return false;
	switch (s->id) {
		case STATE_UNINITIALIZED:
		case STATE_INITIALIZING:
		case STATE_INITIALIZED:
		case STATE_REGISTERING:
		case STATE_REGISTERED:
		case STATE_DISCONNECTING:
		case STATE_DISCONNECTED:
		case STATE_TERMINATING:
		case STATE_TERMINATED:
			// cannot hang up in these states
			return false;
		case STATE_DIALING:
			// debatable - for now, can try to hang up in this state
			return true;
		case STATE_CONNECTED:
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
		case STATE_HOLD:
			// can hang up in these states
			return true;
		default:
			// not a known state; disallow hang up
			return false;
	}
}

// Unconditionally signal program termination
State * TeleKarma::Quit(Action * a, State * s)
{
	return SetState(new State(STATE_TERMINATING, s->turn+1));
}

/*

// Play a WAV file over phone connection.
void TeleKarma::PlayWAV(const PString & filename, int repeat, int delay)
{
	if (phone != NULL) {
		phone->PlayWAV(filename, repeat, delay);
	}
}

void TeleKarma::StopWAV()
{
	if (phone != NULL) {
		phone->StopWAV();
	}
}

void TeleKarma::StartIVR(const PString &fname)
{
	if (phone != NULL) {
		PString assuranceName = "assurance.wav";
		PlayWAV(assuranceName, 0,0);
		cout << "Please wait while a mandatory prompt is played..." << flush;
		PThread::Sleep(4500);
		StopWAV();
		cout << "finished.\n" << flush;
		StartRecording();
		phone->PlayWAV(fname, IVR_REPEATS, PAUSE_TIME);
		phone->TurnOffMicrophone();
	}
}

void TeleKarma::StopIVR()
{
	if (phone != NULL) {
		phone->StopWAV();
		phone->TurnOnMicrophone();
	}
}

//Only used for debugging.  Potentially for later features also.
void TeleKarma::ToggleRecording()
{
	if (!phone->IsRecording()) {
		StartRecording();
	} else {
		phone->StopRecording();
	}
}

void TeleKarma::StartRecording() {
	PTime now;
	PString recFName("recordings/rec");
	recFName += now.AsString("_yyyy.MM.dd_hh.mm.ss");
	recFName += ".wav";
	phone->StartRecording(recFName);
}

bool TeleKarma::IsPlayingWAV(bool onLine, bool onSpeakers)
{
	if (onLine && onSpeakers) {
		return false;	// TO GO, since we currently don't play WAVs over speaker
	} else if (onLine) {
		return (phone != NULL && phone->IsPlayingWav());
	} else if (onSpeakers) {
		return false;	// TO GO, since we currently don't play WAVs over speaker
	} else {
		return false;
	}
}

void TeleKarma::SetMicVolume(unsigned int volume)
{
	// not functioning per Opal spec
	if (phone != NULL) return phone->SetMicVolume(volume);
#ifdef WIN32
	if (volume > 0) {
		//UnMuteMic();
	} else {
		//MuteMic();
	}

#endif
}

void TeleKarma::SetSpeakerVolume(unsigned int volume)
{
	// not functioning per Opal spec
	if (phone != NULL) return phone->SetSpeakerVolume(volume);
}
*/

// End of File ///////////////////////////////////////////////////////////////
