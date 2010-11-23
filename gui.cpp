#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <wx/wx.h>
#include <sstream>	// stringstream
#include <cstring>
#include "state.h"
#include "action.h"
#include "model.h"
#include "telekarma.h"
#include <opal/mediafmt.h>  // required for wxstring.h
#include <ptlib/wxstring.h>
#include "wxevt.h"
#include "gui.h"

//#ifdef WIN32
//	PlaySound(TEXT("alert.wav"), NULL, SND_FILENAME);
//#endif

#if wxUSE_UNICODE
typedef wstringstream tstringstream;
#else
typedef  stringstream tstringstream;
#endif

//////////////////////////////////////////////////////////////////////////////

/*
wxStateChangeEvent::wxStateChangeEvent( wxObject * origin, const StateID & stateId, int turn, const StatusID & statusId, const wxString & message )
	: wxNotifyEvent(wxEVT_STATE_CHANGE), stateId(stateId), statusId(statusId), turn(turn), msg(message)
{
	SetEventObject(origin);
}

StateID wxStateChangeEvent::GetStateID() { return stateId; }

StatusID wxStateChangeEvent::GetStatusID() { return statusId; }

int wxStateChangeEvent::GetTurn() { return turn; }

const wxString & wxStateChangeEvent::GetMessage() { return msg; }

wxEvent * wxStateChangeEvent::Clone()
{
	wxEvent * clone = new wxStateChangeEvent( GetEventObject(), stateId, turn, statusId, msg );
	return clone;
}
*/

wxStateChangeEvent::wxStateChangeEvent( wxObject * origin )
	: wxNotifyEvent(wxEVT_STATE_CHANGE)
{
	SetEventObject(origin);
}

wxEvent * wxStateChangeEvent::Clone()
{
	return ( new wxStateChangeEvent( GetEventObject() ) );
}

//////////////////////////////////////////////////////////////////////////////

// wxWidget's launch macro - replaces PProcess launch macro.
// generates main method.
IMPLEMENT_APP(TeleKarmaNG)

TeleKarmaNG::~TeleKarmaNG()
{
	// signal controller to stop
	Action * a = new QuitAction(-1);
	model->EnqueueAction(a);
	// wait (blocking) for controller to stop
	controller->WaitForTermination(PTimeInterval(8000));
	// wait (blocking) for monitor to stop
	monitor->WaitForTermination(PTimeInterval(8000));
	// Theory says these should be deleted, but in practice,
	// doing so produces segfaults - probably related to
	// assert fail (long-lived child processes)
	//delete controller;
	//delete monitor;
	//delete model;
	delete smutex;
	delete tmutex;
	// delete any states still queued (should be none...)
	for (int i = 0; i < sqsize; ++i) {
		delete squeue[i];
		squeue[i] = NULL;
	}
	delete [] squeue;
	squeue = NULL;
}

void TeleKarmaNG::Main()
{
	// a dummy method for the benefit of PProcess
}

bool TeleKarmaNG::OnInit()
{
	// initialize pointers
	model = NULL;
	controller = NULL;
	monitor = NULL;
	win = NULL;
	smutex = NULL;
	tmutex = NULL;
	squeue = NULL;

	// create mutux
	tmutex = new PSemaphore(1,1);
	smutex = new PSemaphore(1,1);

	// set up state queue
	sqsize = QUEUE_SIZE;			// from model.h
	sqhead = 0;
	sqtail = 0;
	squeue = new State *[sqsize];
	for (int i = 0; i < sqsize; ++i) {
		squeue[i] = NULL;
	}

	// construct model
	model = new Model();

	// launch GUI
    win = new MainFrame( this, model, _("TeleKarma NG"), wxDefaultPosition, wxSize(450,340) );

	// launch controller thread
	controller = new TeleKarma(model);

	// launch monitor thread
	monitor = new StateMonitor(this, model);

	// display the window
	win->Show(true);
    SetTopWindow(win);

	// Start service threads
	controller->Resume();
	monitor->Resume();

	// remnants from testing/debugging of failure to pass state data with event...
	//wxStateChangeEvent evt(win);
	//win->AddPendingEvent(evt);
	//win->Trace(StateHelper::ToTrace(evt.GetStateID(), evt.GetStatusID(), evt.GetMessage()));
	//wxStateChangeEvent * evt2 = dynamic_cast<wxStateChangeEvent *>(evt.Clone());
	//win->Trace(StateHelper::ToTrace(evt2->GetStateID(), evt2->GetStatusID(), evt2->GetMessage()));

	// signal successful setup
    return true;
}

void TeleKarmaNG::OnStateChange(State * s)
{
	tmutex->Wait();
	if (win != NULL && s != NULL) {
		smutex->Wait();
		// enqueue
		if (sqhead == sqtail) {
			if (squeue[sqhead] == NULL) {
				// the queue is empty
				squeue[sqtail] = s;
				sqtail = (sqtail + 1) % sqsize;
			}
		} else {
			// the queue is neither empty nor full
			squeue[sqtail] = s;
			sqtail = (sqtail + 1) % sqsize;
		}
		smutex->Signal();
		//win->Trace(_("New state\n"));
		wxStateChangeEvent evt( win );
		win->AddPendingEvent(evt);
		//win->Trace(_("***\n"));
		//win->Trace(StateHelper::ToTrace(s->id, s->status, _("CANNOT SET")));
		//win->Trace(_("***\n"));
	}
	tmutex->Signal();
	// delete s;
}

// Remove a state from the queue
State * TeleKarmaNG::DequeueState()
{
	// set up result
	State * result = NULL;
	// enter mutex
	smutex->Wait();
	// dequeue
	if (squeue[sqhead] != NULL) {
		result = squeue[sqhead];
		squeue[sqhead] = NULL;
		sqhead = (sqhead + 1) % sqsize;
	}
	// exit mutex
	smutex->Signal();
	return result;
}


void TeleKarmaNG::OnWindowDone()
{
	tmutex->Wait();
	win = NULL;
	tmutex->Signal();
}


//////////////////////////////////////////////////////////////////////////////

MainFrame::MainFrame(TeleKarmaNG * parent, Model * model, const wxString & title, const wxPoint & pos, const wxSize & size)
	: wxFrame( NULL, -1, title, pos, size ),
	  parent(parent),
	  model(model),
	  menuFile(NULL),
	  menuEdit(NULL),
	  menuCall(NULL),
	  menuSMS(NULL),
	  menuHelp(NULL),
	  menuBar(NULL),
	  btnContacts(NULL),
	  btnDial(NULL),
	  tcDest(NULL),
	  tcTrace(NULL),
	  turn(-65536),
	  state(STATE_ERROR)
{

	// set minimum size
	SetMinSize(size);

	// define the file menu...
    menuFile = new wxMenu;
    menuFile->Append( wxID_EXIT, _("E&xit") );

	// define the edit menu...
	menuEdit = new wxMenu;
    menuEdit->Append( evAccounts, _("&Accounts...") );
    menuEdit->Append( evContacts, _("&Contacts...") );

	// define the call menu...
	menuCall = new wxMenu;
    menuCall->Append( evDial, _("&Dial...") );
    menuCall->Append( evHold, _("H&old") );
    menuCall->Append( evAutoHold, _("&AutoHold") );
    menuCall->Append( evRetrieve, _("&Retrieve") );
    menuCall->AppendSeparator();
    menuCall->Append( evHangUp, _("&Hang Up") );

	// define the sms menu...
	menuSMS = new wxMenu;
    menuSMS->Append( evSendSMS, _("&Send Text...") );

	// define the help menu...
	menuHelp = new wxMenu;
    menuHelp->Append( wxID_ABOUT, _("&About...") );

    menuBar = new wxMenuBar;
    menuBar->Append( menuFile, _("&File") );
    menuBar->Append( menuEdit, _("&Edit") );
    menuBar->Append( menuCall, _("&Call") );
    menuBar->Append( menuSMS,  _("&SMS" ) );
    menuBar->Append( menuHelp, _("&Help") );

    SetMenuBar( menuBar );

	// store menu item id's
	fileExit = menuBar->FindMenuItem(_("File"), _("Exit"));
	editAccounts = menuBar->FindMenuItem(_("Edit"), _("Accounts..."));
	editContacts = menuBar->FindMenuItem(_("Edit"), _("Contacts..."));
	callDial = menuBar->FindMenuItem(_("Call"), _("Dial"));
	callHold = menuBar->FindMenuItem(_("Call"), _("Hold"));
	callAutoHold = menuBar->FindMenuItem(_("Call"), _("AutoHold"));
	callRetrieve = menuBar->FindMenuItem(_("Call"), _("Retrieve"));
	callHangUp = menuBar->FindMenuItem(_("Call"), _("Hang Up"));
	smsSendText = menuBar->FindMenuItem(_("SMS"), _("Send Text..."));
	helpAbout = menuBar->FindMenuItem(_("Help"), _("About"));

	wxPanel * panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	panel->SetBackgroundColour(wxColour(237,237,237));	// light gray
	wxBoxSizer * vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);

	// Create status & dial panel
	btnContacts = new wxButton(panel, NULL, wxT("Contacts..."));
	//btnContacts->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnQuit));
	btnContacts->SetFocus();

	// Address
	tcDest = new wxTextCtrl(panel, wxID_ANY, _("sip:"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	tcDest->SetForegroundColour(wxColour(0,0,0));	// black
	tcDest->SetBackgroundColour(wxColour(255,255,255));	// white

	// Dial button
	btnDial = new wxButton(panel, NULL, wxT("&Dial"));
	btnDial->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDial));

	// Trace window
	// wxTE_RICH: on Windows, enables >64kb of data
	tcTrace = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP | wxSUNKEN_BORDER | wxTE_RICH | wxTE_READONLY);
	tcTrace->SetForegroundColour(wxColour(53,255,53)); // green
	tcTrace->SetBackgroundColour(wxColour(0,0,0)); // black
	//wxFont font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	//tcTrace->SetFont(font);

	// add the dial bar
	hbox->Add(btnContacts, 0, wxALIGN_LEFT | wxALL, 5);
	hbox->Add(tcDest, 1, wxALIGN_LEFT | wxTOP | wxBOTTOM | wxEXPAND, 5);
	hbox->Add(btnDial, 0, wxALIGN_RIGHT | wxALL, 5);
	vbox->Add(hbox, 0, wxALIGN_TOP | wxEXPAND, 0);

	// add the trace window
	vbox->Add(tcTrace, 1, wxALIGN_BOTTOM | wxEXPAND | wxLEFT, 1);

	// add the sizer to the main panel in the main frame
	panel->SetSizer(vbox);

	// add a status bar to the bottom of the window
    CreateStatusBar();
    SetStatusText( _("Welcome to TeleKarma NG!") );

	// Track close
	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MainFrame::OnClose));

	// Receive state change events
//	Connect(wxEVT_STATE_CHANGE, wxStateChangeEventFunction(MainFrame::OnStateChange));

	// initialize GUI 
	OnAdjustControls(STATE_UNINITIALIZED);

}

void MainFrame::OnStateChange(wxStateChangeEvent & evt)
{
	State * s = parent->DequeueState();
	if (s == NULL) return;
	tstringstream text;
	text << s->message;
	wxString msg;
	msg = text.str().c_str();
	if (turn == s->turn) {
		// state hasn't changed, but status probably has
		Trace(StateHelper::ToTrace(s->id, s->status, msg));
	} else {
		// state has changed
		SetStatusText(StateHelper::ToStatus(s->id));
		Trace(StateHelper::ToTrace(s->id, s->status, msg));
		turn = s->turn;
		OnAdjustControls(s->id);
		// XXX temporary
		Action * a = NULL;
		switch (s->id) {
			case STATE_UNINITIALIZED:
				// XXX temporary
				Trace(_("State Uninitialized\n"));
				a = new InitializeAction("stun.ekiga.net", s->turn);
				model->EnqueueAction(a);
				a = NULL;
				break;
			case STATE_INITIALIZING:
				
				break;
			case STATE_INITIALIZED:
				// XXX temporary
				a = new RegisterAction("ekiga.net", "mvolk@ekiga.net", "rockin4test", s->turn);
				model->EnqueueAction(a);
				a = NULL;
				//{
				//	RegisterDialog * r = new RegisterDialog(this, model, s->turn);
				//	r->Show(true);
				//}
				break;
			case STATE_REGISTERING:
				
				break;
			case STATE_REGISTERED:

				break;
			case STATE_DISCONNECTING:
				
				break;
			case STATE_DISCONNECTED:
				
				break;
			case STATE_TERMINATING:
				
				break;
			case STATE_DIALING:
				
				break;
			case STATE_CONNECTED:
				
				break;
			case STATE_AUTOHOLD:
				
				break;
			case STATE_MUTEAUTOHOLD:
				
				break;
			case STATE_HOLD:
				
				break;
			default:
				SetStatusText(_("Unrecognized state type."));
				break;
		}
	}
	state = s->id;
}

void MainFrame::OnAdjustControls(const StateID state)
{
	menuBar->Enable(fileExit, true);
	menuBar->Enable(editAccounts, false);
	menuBar->Enable(editContacts, false);
	menuBar->Enable(callDial, StateHelper::CanDial(state));
	menuBar->Enable(callHold, StateHelper::CanHold(state));
	menuBar->Enable(callAutoHold, StateHelper::CanHold(state));
	menuBar->Enable(callRetrieve, StateHelper::CanRetrieve(state));
	menuBar->Enable(callHangUp, StateHelper::CanHangUp(state));
	menuBar->Enable(smsSendText, StateHelper::IsRegistered(state));
	menuBar->Enable(helpAbout, true);
	if (StateHelper::CanDial(state)) {
		btnDial->Enable(true);
		Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnHangUp));
		Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDial));
		btnDial->SetLabel(_("&Dial"));
	} else if (StateHelper::CanHangUp(state)) {
		btnDial->Enable(true);
		Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDial));
		Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnHangUp));
		btnDial->SetLabel(_("&Hang Up"));
	} else {
		btnDial->Enable(false);
	}
	btnContacts->Enable(false);
}

void MainFrame::OnDial(wxCommandEvent & event)
{
	Action * a = new DialAction("sip:500@ekiga.net", turn);
	model->EnqueueAction(a);
}

void MainFrame::OnHangUp(wxCommandEvent & event)
{
	Action * a = new DisconnectAction(turn);
	model->EnqueueAction(a);
}

void MainFrame::OnHold(wxCommandEvent & event)
{
	Action * a = new HoldAction(turn);
	model->EnqueueAction(a);
}

void MainFrame::OnAutoHold(wxCommandEvent & event)
{
	Action * a = new AutoHoldAction(turn);
	model->EnqueueAction(a);
}

void MainFrame::OnMuteAutoHold(wxCommandEvent & event)
{
//	Action * a = new MuteAutoHoldAction(turn);
//	model->EnqueueAction(a);
}

void MainFrame::OnRetrieve(wxCommandEvent & event)
{
	Action * a = new RetrieveAction(turn);
	model->EnqueueAction(a);
}

void MainFrame::OnQuit(wxCommandEvent & WXUNUSED(event))
{
	Action * a = new QuitAction(turn);
	model->EnqueueAction(a);
	parent->OnWindowDone();
	Close(true);
}

void MainFrame::OnClose(wxCloseEvent & event)
{
	Action * a = new QuitAction(turn);
	model->EnqueueAction(a);
	parent->OnWindowDone();
	Destroy();
}

void MainFrame::OnAbout(wxCommandEvent & WXUNUSED(event))
{
	tstringstream title;
	title << "About " << PRODUCT_NAME;
	tstringstream text;
	text  << PRODUCT_NAME << " Version " << VERSION << "\n"
		  "\n"
		  PRODUCT_NAME << " is a limited SIP telephony application with experimental\n"
		  "autohold functionality designed to free the user from the phone when\n"
		  "placed on hold.\n"
		  "\n"
		  "Copyright (c) 2010 " << COPYRIGHT_HOLDER << ", All rights reserved.\n"
		  "\n";
	wxMessageBox(text.str().c_str(), title.str().c_str(), wxOK, this);
}

void MainFrame::Trace(const wxString & msg)
{
	tcTrace->WriteText(msg);
	/* The code below caused flickering...
	// HACK: Under Windows (using wxTE_RICH2) we have trouble ensuring that the last
	// entered line is really at the bottom of the screen. We jump through some
	// hoops to get this working.

	// Count number of newlines (i.e lines)
	int lines = 0;
	const char * cstr = (const char *)msg.c_str();
	for ( ; *cstr ; ++cstr )
		if ( *cstr == '\n' )
			++lines;

	// Dance...
	tcTrace->Freeze();                 // Freeze the window to prevent scrollbar jumping
	tcTrace->AppendText( msg );        // Add the text
	tcTrace->ScrollLines( lines + 1 ); // Scroll down correct number of lines + one (the extra line is important for some cases!)
	tcTrace->ShowPosition( tcTrace->GetLastPosition() ); // Ensure the last line is shown at the very bottom of the window
	tcTrace->Thaw();                   // Allow the window to redraw
	*/
}

//////////////////////////////////////////////////////////////////////////////
// RegisterDialog

RegisterDialog::RegisterDialog(MainFrame * parent, Model * model, int turn)
	: wxDialog(parent, wxID_ANY, _("Register with SIP service"), wxDefaultPosition, wxSize(250,230)),//, ( wxFRAME_FLOAT_ON_PARENT | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxBORDER_SIMPLE ) ),
	  model(model),
	  tcSIP(NULL),
	  tcUser(NULL),
	  tcPassword(NULL),
	  btnCancel(NULL),
	  btnRegister(NULL),
	  turn(turn)
{

/*
	wxBoxSizer * vbox1 = new wxBoxSizer(wxVERTICAL);
	wxPanel * panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	panel->SetBackgroundColour(wxColour(237,237,237));	// light gray
	wxBoxSizer * vbox2 = new wxBoxSizer(wxVERTICAL);

	// SIP Server
	wxBoxSizer * hbox1 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText * lblSIP = new wxStaticText(panel, wxID_ANY, _("SIP Server:"));
	tcSIP = new wxTextCtrl(panel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	hbox1->Add(lblSIP, 0, wxLEFT | wxRIGHT | wxALIGN_LEFT, 5);
	hbox1->Add(tcSIP, 0, wxLEFT | wxRIGHT | wxALIGN_LEFT, 5);
	vbox2->Add(hbox1, 0, wxALIGN_LEFT | wxEXPAND, 0);

	// User name
	wxBoxSizer * hbox2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText * lblUser = new wxStaticText(this, wxID_ANY, _("Account name:"));
	tcUser = new wxTextCtrl(this, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	hbox2->Add(lblUser, 0, wxLEFT | wxRIGHT | wxALIGN_LEFT, 5);
	hbox2->Add(tcUser, 0, wxLEFT | wxRIGHT | wxALIGN_LEFT, 5);
	vbox2->Add(hbox2, 0, wxALIGN_LEFT | wxEXPAND, 0);

	// Password
	wxBoxSizer * hbox3 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText * lblPassword = new wxStaticText(panel, wxID_ANY, _("Password:"));
	tcPassword = new wxTextCtrl(panel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER | wxTE_PASSWORD);
	hbox3->Add(lblPassword, 0, wxLEFT | wxRIGHT | wxALIGN_LEFT, 5);
	hbox3->Add(tcPassword, 0, wxLEFT | wxRIGHT | wxALIGN_LEFT, 5);
	vbox2->Add(hbox3, 0, wxALIGN_LEFT | wxEXPAND, 0);

	vbox1->Add(panel);

	// Buttons
	wxBoxSizer * hbox4 = new wxBoxSizer(wxHORIZONTAL);
	btnRegister = new wxButton(this, NULL, wxT("&Register"));
	//btnRegister->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RegisterDialog::OnRegister));
	btnCancel = new wxButton(this, NULL, wxT("&Cancel"));
	//btnCancel->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RegisterDialog::OnCancel));
	btnCancel->SetFocus();
	hbox4->Add(btnCancel, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
	hbox4->Add(btnRegister, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 5);
	vbox1->Add(hbox4, 0, wxALIGN_CENTER | wxEXPAND, 0);

	// add the sizer to the main panel in the main frame
	SetSizer(vbox1);

	// Track close
	//Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(RegisterDialog::OnClose));

	//Centre();
	//ShowModal();
	//Destroy();
*/
	wxPanel *panel = new wxPanel(this, -1);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBox *st = new wxStaticBox(panel, -1, wxT("Colors"), 
	  wxPoint(5, 5), wxSize(240, 150));
	wxRadioButton *rb = new wxRadioButton(panel, -1, 
	  wxT("256 Colors"), wxPoint(15, 30), wxDefaultSize, wxRB_GROUP);

	wxRadioButton *rb1 = new wxRadioButton(panel, -1, 
	  wxT("16 Colors"), wxPoint(15, 55));
	wxRadioButton *rb2 = new wxRadioButton(panel, -1, 
	  wxT("2 Colors"), wxPoint(15, 80));
	wxRadioButton *rb3 = new wxRadioButton(panel, -1, 
	  wxT("Custom"), wxPoint(15, 105));
	wxTextCtrl *tc = new wxTextCtrl(panel, -1, wxT(""), 
	  wxPoint(95, 105));

	wxButton *okButton = new wxButton(this, -1, wxT("Ok"), 
		wxDefaultPosition, wxSize(70, 30));
	wxButton *closeButton = new wxButton(this, -1, wxT("Close"), 
		wxDefaultPosition, wxSize(70, 30));

	hbox->Add(okButton, 1);
	hbox->Add(closeButton, 1, wxLEFT, 5);

	vbox->Add(panel, 1);
	vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);

	SetSizer(vbox);
	Centre();
	ShowModal();
	Destroy(); 

}

void RegisterDialog::OnClose(wxCloseEvent & event)
{
	//Destroy();
}

void RegisterDialog::OnRegister(wxCommandEvent & event)
{
	/*
	PwxString sip = tcSIP->GetValue();
	PwxString usr = tcUser->GetValue();
	PwxString pwd = tcPassword->GetValue();
	Action * a = new RegisterAction(sip.p_str(), usr.p_str(), pwd.p_str(), turn);
	model->EnqueueAction(a);
	Destroy();
	*/
}

void RegisterDialog::OnCancel(wxCommandEvent & event)
{
	//Destroy();
}

//////////////////////////////////////////////////////////////////////////////
// StateHelper
//

wxString StateHelper::ToStatus(const StateID & state)
{
	wxString str;
	switch (state) {
		case STATE_UNINITIALIZED:
			str = _("Welcome to TeleKarma NG!");
			break;
		case STATE_INITIALIZING:
			str = _("Initializing...");
			break;
		case STATE_INITIALIZED:
			str = _("Ready for SIP registration.");
			break;
		case STATE_REGISTERING:
			str = _("Registering with SIP provider...");
			break;
		case STATE_REGISTERED:
			str = _("Ready.");
			break;
		case STATE_DISCONNECTING:
			str = _("Disconnecting call...");
			break;
		case STATE_DISCONNECTED:
			str = _("Call disconnected by remote party.");
			break;
		case STATE_TERMINATING:
			str = _("Shutting down...");
			break;
		case STATE_DIALING:
			str = _("Dialing...");
			break;
		case STATE_CONNECTED:
			str = _("Call connected.");
			break;
		case STATE_AUTOHOLD:
			str = _("Autoholding for human...");
			break;
		case STATE_MUTEAUTOHOLD:
			str = _("Autoholding for human (muted)...");
			break;
		case STATE_HOLD:
			str = _("Call on hold.");
			break;
		default:
			str = _("Unexpected state!");
			break;
	}
	return str;
}

wxString StateHelper::ToTrace(const StateID & state, const StatusID & status, const wxString & message)
{
	wxString str;
	tstringstream text;
	switch (state) {
		case STATE_UNINITIALIZED:
			text << "Welcome to TeleKarma NG!";
			break;
		case STATE_INITIALIZING:
			text << "Initializing...";
			break;
		case STATE_INITIALIZED:
			text << "Ready for SIP registration...";
			break;
		case STATE_REGISTERING:
			text << "Registering with SIP provider...";
			break;
		case STATE_REGISTERED:
			text << "Ready to dial...";
			break;
		case STATE_DISCONNECTING:
			text << "Disconnecting call...";
			break;
		case STATE_DISCONNECTED:
			text << "Call disconnected by remote party...";
			break;
		case STATE_TERMINATING:
			text << "Shutting down...";
			break;
		case STATE_DIALING:
			text << "Dialing...";
			break;
		case STATE_CONNECTED:
			text << "Call connected...";
			break;
		case STATE_AUTOHOLD:
			text << "Autoholding for human...";
			break;
		case STATE_MUTEAUTOHOLD:
			text << "Autoholding for human (muted)...";
			break;
		case STATE_HOLD:
			text << "Call on hold...";
			break;
		default:
			text << "Unexpected state!";
			break;
	}
	if (status != STATUS_UNSPECIFIED) {
		switch (status) {
			case STATUS_FAILED:
				text << " (failed)";
				break;
			case STATUS_AUTO_RETRIEVE:
				text << " (human detected)";
				break;
			case STATUS_NOTIFY_RECORD:
				text << " (playing mandatory recording notification)";
				break;
			case STATUS_RECORDING:
				text << " (started recording)";
				break;
			case STATUS_DONE_RECORDING:
				text << " (stopped recording)";
				break;
			case STATUS_TURN_MISMATCH:
				text << " (request rejected: turn mismatch)";
				break;
			case STATUS_RETRIEVE:
				text << " (retrieving)";
				break;
			default:
				text << " (unknown status)";
				break;
		}
		if (!(message.IsEmpty())) {
			text << ": " << message;
		}
	} else {
		if (!(message.IsEmpty())) {
			text << " " << message;
		}
	}
	text << "\n";
	str = text.str().c_str();
	return str;
}

bool StateHelper::CanRegister(const StateID & state)
{
	return (state == STATE_INITIALIZED);
}

bool StateHelper::CanDial(const StateID & state)
{
	return (state == STATE_REGISTERED);
}

bool StateHelper::CanHold(const StateID & state)
{
	return (state == STATE_CONNECTED);
}

bool StateHelper::CanHangUp(const StateID & state)
{
	switch (state) {
		case STATE_CONNECTED:
		case STATE_HOLD:
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
			return true;
		default:
			return false;
	}
}

bool StateHelper::CanRetrieve(const StateID & state)
{
	switch (state) {
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
		case STATE_HOLD:
			return true;
		default:
			return false;
	}
}

bool StateHelper::IsRegistered(const StateID & state)
{
	switch (state) {
		case STATE_REGISTERED:
		case STATE_DISCONNECTING:
		case STATE_DISCONNECTED:
		case STATE_DIALING:
		case STATE_CONNECTED:
		case STATE_AUTOHOLD:
		case STATE_MUTEAUTOHOLD:
		case STATE_HOLD:
			return true;
		default:
			return false;
	}
}

//////////////////////////////////////////////////////////////////////////////

StateMonitor::StateMonitor(TeleKarmaNG * parent, Model * model)
	: PThread(65536, NoAutoDeleteThread), model(model), parent(parent)
{ }

void StateMonitor::Main()
{
	State * s;
	bool run = true;
	while (run) {
		PThread::Sleep(50);
		s = model->DequeueState();
		if (s != NULL) {
			parent->OnStateChange(s);
			if (s->id == STATE_TERMINATED) {
				run = false;
			}
		}
	}
}
