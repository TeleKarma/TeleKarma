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
#include <ptlib/sound.h>
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

// Main program
void TeleKarma::Main() {
	// enter main control loop
	State * currentState = model->GetState();
	while (currentState && currentState->id != STATE_TERMINATING) {
		currentState = UpdateState(currentState);
		currentState = DoAction(model->DequeueAction(), currentState);
		currentState = model->GetState();	// Mike's attempt to patch a bug reported by Tom
		PThread::Sleep(SLEEP_DURATION);
	}
	// handle extreme case
	if (!currentState) {
		SetState(new State(STATE_ERROR, -1, STATUS_UNSPECIFIED, "NULL State detected"));
		SetState(new State(STATE_TERMINATING, -2));
	}

	// PThread does NOT have a virtual destructor - only guarantee of normal
	// cleanup is to cleanup on the way out of MAIN
	delete phone;

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
				if (countdown <= 0) {
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
					result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Call failed"));
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
				result = SetState(new State(result->id, result->turn, STATUS_AUTO_RETRIEVE, "Human detected"));
				phone->StopWAV();
				if (phone->IsRecording()) phone->StopRecording();
				result = SetState(new State(result->id, result->turn, STATUS_DONE_RECORDING));
				phone->TurnOnMicrophone();
				// XXX enable the speaker - implementation to go
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
	// process "global" (turn-independent) actions independant of turn #
	switch (a->id) {
		case ACTION_QUIT:
			result = Quit(a, s);
			break;
		case ACTION_PLAY_SOUND:
			result = PlaySound(a, s);
			break;
		case ACTION_SEND_TONE:
			result = SendTone(a, s);
			break;
		default:
			// process turn-independent actions if applicable
			if (a->turn == s->turn) {
				switch (a->id) {
					case ACTION_INITIALIZE:
						result = Initialize(a, s);
						break;
					case ACTION_REGISTER:
						result = Register(a, s);
						break;
					case ACTION_DIAL:
						result = Dial(a, s);
						break;
					case ACTION_HOLD:
						result = Hold(a, s);
						break;
					case ACTION_AUTOHOLD:
						result = AutoHold(a, s);
						break;
					case ACTION_MUTE:
						result = MuteAutoHold(a, s);
						break;
					case ACTION_RETRIEVE:
						result = Retrieve(a, s);
						break;
					case ACTION_DISCONNECT:
						result = Disconnect(a, s);
						break;
					default:
						// simply ignore unknown actions
						// (and global actions)
						break;
				}
			} else {
				if (s != NULL) {
					result = SetState(new State(s->id, s->turn, STATUS_TURN_MISMATCH));
				}
			}
			break;
	}
	delete a;
	return result;
}

// Initialize the application and telephony API. Blocks.
State * TeleKarma::Initialize(Action * a, State * s)
{
	if (s->id != STATE_UNINITIALIZED) return s;
	bool flag = false;
	InitializeAction * ia = dynamic_cast<InitializeAction *>(a);
	State * result = NULL;
	if (ia == NULL) {
		result = SetState(new State(STATE_INITIALIZING, s->turn+1));
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		// set stun server **before** entering INITIALIZING state
		model->SetStunServer(ia->stunServer);
		result = SetState(new State(STATE_INITIALIZING, s->turn+1));
		// verify existence and type of 'logs' folder
		const char * strPath1 = "logs";
		struct stat status;
		stat(strPath1, &status);
		bool existsLogs = !((ACCESS(strPath1, 0) == -1) || !(status.st_mode & S_IFDIR));
		// verify existence and type of 'recordings' folder
		const char * strPath2 = "recordings";
		stat(strPath2, &status);
		bool existsRecordings = !((ACCESS(strPath2, 0) == -1) || !(status.st_mode & S_IFDIR));
		if (!(existsLogs || existsRecordings)){
			result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Please create \"logs\" and \"recordings\" folders in your TeleKarma program folder."));
			flag = true;
		} else if (!existsLogs) {
			result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Please create a \"logs\" folder in your TeleKarma program folder."));
			flag = true;
		} else if (!existsRecordings) {
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
			PTrace::Initialise(3, logFName);
			// initialize telephony (blocking call)
			phone->Initialise(ia->stunServer);
			PSTUNClient * stunClient = phone->GetSTUNClient();
			if (stunClient != NULL) {
				model->SetStunType(stunClient->GetNatTypeName());
			} else {
				model->SetStunType("none");
			}
			result = SetState(new State(STATE_INITIALIZED, result->turn+1));
		}
	}
	return result;
}

// Register with SIP registrar. Asynchronous.
State * TeleKarma::Register(Action * a, State * s)
{
	if (s->id != STATE_INITIALIZED) return s;
	State * result = NULL;
	RegisterAction * ra = dynamic_cast<RegisterAction *>(a);
	if (ra == NULL) {
		result = SetState(new State(STATE_REGISTERING, s->turn+1));
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
		result = SetState(new State(STATE_INITIALIZED, result->turn+1));
	} else if (phone == NULL) {
		result = SetState(new State(STATE_REGISTERING, s->turn+1));
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		// set model data before changing state to REGISTERING
		model->SetRegistrar(ra->registrar);
		model->SetUserName(ra->user);
		result = SetState(new State(STATE_REGISTERING, s->turn+1));
		countdown = (REGISTRATION_TIMEOUT)/(SLEEP_DURATION);
		phone->Register(ra->registrar, ra->user, ra->password);
	}
	return result;
}

// Dial the indicated SIP address. Format sip addr as sip:user@domain. Asynchronous.
State * TeleKarma::Dial(Action * a, State * s)
{
	if (s->id != STATE_REGISTERED) return s;
	State * result = NULL;
	DialAction * da = dynamic_cast<DialAction *>(a);
	if (da == NULL) {
		result = SetState(new State(STATE_DIALING, s->turn+1));
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Programming error: action cast failed"));
		result = SetState(new State(STATE_REGISTERED, result->turn+1));
	} else if (phone == NULL) {
		result = SetState(new State(STATE_DIALING, s->turn+1));
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		// set model data before changing state to DIALING
		model->SetDestination(da->dest);
		result = SetState(new State(STATE_DIALING, s->turn+1));
		phone->TurnOnMicrophone();
		// XXX enable the speaker - implementation to go
		phone->Dial(da->dest);
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

// Enter normal hold mode
State * TeleKarma::Hold(Action * a, State * s)
{
	if (s->id != STATE_CONNECTED) return s;
	State * result = s;
	if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		result = SetState(new State(STATE_HOLD, result->turn+1));
		// XXX disable the speaker - implementation to go
		// disable the microphone
		phone->TurnOffMicrophone();
		// play notification of recording IF not already recording
		if (!phone->IsRecording()) {
			result = StartRecording(result);
		}
		result = SetState(new State(result->id, result->turn, STATUS_RECORDING));
		phone->PlayWAVCall(HOLD_WAV, IVR_REPEATS, PAUSE_TIME);
	}
	return result;
}

// Enter autohold mode
State * TeleKarma::AutoHold(Action * a, State * s)
{
	State * result = s;
	if (s->id == STATE_CONNECTED) {
		if (phone == NULL) {
			result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
			result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
		} else {
			result = SetState(new State(STATE_AUTOHOLD, result->turn+1));
			// clear the touch tone queue
			phone->ClearTones();
			// disable the microphone
			phone->TurnOffMicrophone();
			// play notification of recording IF not already recording
			if (!phone->IsRecording()) {
				result = StartRecording(result);
			}
			result = SetState(new State(result->id, result->turn, STATUS_RECORDING));
			phone->PlayWAVCall(AUTO_HOLD_WAV, IVR_REPEATS, PAUSE_TIME);
		}
	} else if (s->id == STATE_MUTEAUTOHOLD) {
		result = SetState(new State(STATE_AUTOHOLD, result->turn+1));
		// XXX enable the speaker - implementation to go
	}
	return result;
}

State * TeleKarma::MuteAutoHold(Action * a, State * s)
{
	State * result = s;
	if (s->id == STATE_CONNECTED) {
		if (phone == NULL) {
			result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
			result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
		} else {
			result = SetState(new State(STATE_MUTEAUTOHOLD, result->turn+1));
			// clear the touch tone queue
			phone->ClearTones();
			// XXX disable the speaker - implementation to go
			// disable the microphone
			phone->TurnOffMicrophone();
			// play notification of recording IF not already recording
			if (!phone->IsRecording()) {
				result = StartRecording(result);
			}
			result = SetState(new State(STATE_MUTEAUTOHOLD, result->turn, STATUS_RECORDING));
			phone->PlayWAVCall(AUTO_HOLD_WAV, IVR_REPEATS, PAUSE_TIME);
		}
	} else if (s->id == STATE_AUTOHOLD) {
		result = SetState(new State(STATE_MUTEAUTOHOLD, result->turn+1));
		// XXX disable the speaker - implementation to go
	}
	return result;
}

// Retrieve a call from hold, autohold or muteautohold
State * TeleKarma::Retrieve(Action * a, State * s)
{
	if (!IsHoldingState(s)) return s;
	State * result = s;
	if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		result = SetState(new State(result->id, result->turn, STATUS_RETRIEVE));
		phone->StopWAV();
		if (phone->IsRecording()) phone->StopRecording();
		result = SetState(new State(result->id, result->turn, STATUS_DONE_RECORDING));
		phone->TurnOnMicrophone();
		// XXX enable the speaker - implementation to go
		result = SetState(new State(STATE_CONNECTED, result->turn+1));
	}
	return result;
}

// Disconnect the current call. Asynchronous.
State * TeleKarma::Disconnect(Action * a, State * s)
{
	if (!IsConnectedState(s)) return s;
	State * result = SetState(new State(STATE_DISCONNECTING, s->turn+1));
	if (phone == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Telephony service failed"));
		result = SetState(new State(STATE_UNINITIALIZED, result->turn+1));
	} else {
		phone->Disconnect();
		if (IsHoldingState(s)) {
			phone->StopWAV();
			if (phone->IsRecording()) phone->StopRecording();
			phone->TurnOnMicrophone();
			// XXX enable the speaker - implementation to go
		}
	}
	return result;
}

// Play a WAV file over PC sound system
State * TeleKarma::PlaySound(Action * a, State * s)
{
	State * result = s;
	PlaySoundAction * psa = dynamic_cast<PlaySoundAction *>(a);
	if (psa == NULL) {
		result = SetState(new State(result->id, result->turn, STATUS_FAILED, "Unable to play WAV file"));
	} else {
#ifdef WIN32
		PlaySound(TEXT(psa->fname), NULL, psa->fname);
#else
		phone->PlayWAVSpeaker(psa->fname);
#endif

		result = SetState(new State(result->id, result->turn, STATUS_UNSPECIFIED, "Playing WAV file"));
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

// Determine whether the call can be retrieved
bool TeleKarma::IsHoldingState(State * s)
{
	if (s == NULL) return false;
	switch (s->id) {
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
		case STATE_HOLD:
			return true;
		case STATE_UNINITIALIZED:
		case STATE_INITIALIZING:
		case STATE_INITIALIZED:
		case STATE_REGISTERING:
		case STATE_REGISTERED:
		case STATE_DISCONNECTING:
		case STATE_DISCONNECTED:
		case STATE_TERMINATING:
		case STATE_TERMINATED:
		case STATE_DIALING:
		case STATE_CONNECTED:
		default:
			return false;
	}
}

State * TeleKarma::StartRecording(State * currentState)
{
	State * result = NULL;	
	PString assuranceName = "assurance.wav";
	phone->PlayWAVCall(assuranceName, 0, 0);
	/* XXX Do we care about what the return value. */
	result = SetState(new State(currentState->id,
		currentState->turn, STATUS_NOTIFY_RECORD));
	while(phone->IsPlayingWav());
	PTime now;
	PString recFName("recordings/rec");
	recFName += now.AsString("_yyyy.MM.dd_hh.mm.ss");
	recFName += ".wav";
	phone->StartRecording(recFName);
	return result;
}


// Unconditionally signal program termination
State * TeleKarma::Quit(Action * a, State * s)
{
	return SetState(new State(STATE_TERMINATING, s->turn+1));
}

// End of File ///////////////////////////////////////////////////////////////
