#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <wx/wx.h>
#include <sstream>				// stringstream
#include <cstring>
#include "state.h"
#include "action.h"
#include "model.h"
#include "view.h"
#include "telekarma.h"
#include "account.h"
#include <opal/mediafmt.h>		// required for wxstring.h
#include <ptlib/wxstring.h>
#include "gui.h"

//	::PlaySound(TEXT("alert.wav"), NULL, SND_FILENAME);
//	::PlaySound(TEXT("alert.wav"), NULL, SND_FILENAME);

#if wxUSE_UNICODE
typedef wstringstream tstringstream;
#else
typedef  stringstream tstringstream;
#endif

//////////////////////////////////////////////////////////////////////////////

PString _wxStr2Pstr(const wxString & str)
{
	PwxString middleman(str);
	return middleman.p_str();
};

wxString _Pstr2wxStr(const PString & str)
{
	tstringstream middleman;
	middleman << str;
	wxString result(middleman.str().c_str());
	return result;
};

//////////////////////////////////////////////////////////////////////////////

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

wxRegisterDialogClosedEvent::wxRegisterDialogClosedEvent( wxObject * origin )
	: wxNotifyEvent(wxEVT_REGISTER_DIALOG_CLOSED)
{
	SetEventObject(origin);
}

wxEvent * wxRegisterDialogClosedEvent::Clone()
{
	return ( new wxRegisterDialogClosedEvent( GetEventObject() ) );
}

//////////////////////////////////////////////////////////////////////////////

wxDialPadClosedEvent::wxDialPadClosedEvent( wxObject * origin )
	: wxNotifyEvent(wxEVT_DIAL_PAD_CLOSED)
{
	SetEventObject(origin);
}

wxEvent * wxDialPadClosedEvent::Clone()
{
	return ( new wxDialPadClosedEvent( GetEventObject() ) );
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
	// Theory says these should be deleted, but in practice,
	// doing so produces segfaults - probably related to
	// assert fail (long-lived child processes)
	//delete controller;
	//delete model;
	delete mutex;
}

bool TeleKarmaNG::OnInit()
{
	// initialize pointers
	model = NULL;
	controller = NULL;
	win = NULL;
	mutex = NULL;

	// create mutux
	mutex = new PSemaphore(1,1);

	// construct model
	model = new Model();

	// register event listener with model
	model->SetListener(this);

	// launch GUI
    win = new MainFrame( this, model, _("TeleKarma NG"), wxDefaultPosition, wxSize(450,340) );

	// launch controller thread
	controller = new TeleKarma(model);

	// display the window
	win->Show(true);
    SetTopWindow(win);

	// Start controller thread
	controller->Resume();

	// signal successful setup
    return true;
}

void TeleKarmaNG::OnStateChange()
{
	mutex->Wait();
	if (win != NULL) {
		wxStateChangeEvent evt( win );
		win->AddPendingEvent(evt);
	}
	mutex->Signal();
}

void TeleKarmaNG::OnCloseApplication()
{
	mutex->Wait();
	win = NULL;
	mutex->Signal();
}


//////////////////////////////////////////////////////////////////////////////

MainFrame::MainFrame(TeleKarmaNG * view, Model * model, const wxString & title, const wxPoint & pos, const wxSize & size)
	: wxFrame( NULL, -1, title, pos, size ),
	  view(view),
	  model(model),
	  menuFile(NULL),
	  menuEdit(NULL),
	  menuCall(NULL),
	  menuHelp(NULL),
	  menuBar(NULL),
	  btnContacts(NULL),
	  btnDial(NULL),
	  tcDest(NULL),
	  tcTrace(NULL),
	  currentTurn(-65536),
	  previousStateID(STATE_ERROR),
	  currentStateID(STATE_ERROR),
	  regAction(NULL),
	  accounts(NULL),
	  initialized(false),
	  registerDialogIsOpen(false),
	  dialPadIsOpen(false)
{

	// set minimum size
	SetMinSize(size);

	// define the file menu...
    menuFile = new wxMenu;
    menuFile->Append( evRegister, _("&Register...") );
    menuFile->AppendSeparator();
    menuFile->Append( wxID_EXIT, _("E&xit") );

	// define the edit menu...
	menuEdit = new wxMenu;
    menuEdit->Append( evAccounts, _("&Accounts...") );
    //menuEdit->Append( evContacts, _("&Contacts...") );

	// define the call menu...
	menuCall = new wxMenu;
    menuCall->Append( evDial, _("&Dial") );
    menuCall->AppendSeparator();
    menuCall->Append( evDialPad, _("&Touch Tones...") );
    menuCall->AppendSeparator();
    menuCall->Append( evHold, _("H&old") );
    menuCall->Append( evAutoHold, _("&AutoHold") );
    menuCall->Append( evRetrieve, _("&Retrieve") );
    menuCall->AppendSeparator();
    menuCall->Append( evHangUp, _("&Hang Up") );

	// define the help menu...
	menuHelp = new wxMenu;
    menuHelp->Append( wxID_ABOUT, _("&About...") );

    menuBar = new wxMenuBar;
    menuBar->Append( menuFile, _("&File") );
    menuBar->Append( menuEdit, _("&Edit") );
    menuBar->Append( menuCall, _("&Call") );
    menuBar->Append( menuHelp, _("&Help") );

    SetMenuBar( menuBar );

	// store menu item id's
	fileExit = menuBar->FindMenuItem(_("File"), _("Exit"));
	fileRegister = menuBar->FindMenuItem(_("File"), _("Register..."));
	editAccounts = menuBar->FindMenuItem(_("Edit"), _("Accounts..."));
	//editContacts = menuBar->FindMenuItem(_("Edit"), _("Contacts..."));
	callDial = menuBar->FindMenuItem(_("Call"), _("Dial"));
	callDialPad = menuBar->FindMenuItem(_("Call"), _("Touch Tones..."));
	callHold = menuBar->FindMenuItem(_("Call"), _("Hold"));
	callAutoHold = menuBar->FindMenuItem(_("Call"), _("AutoHold"));
	callRetrieve = menuBar->FindMenuItem(_("Call"), _("Retrieve"));
	callHangUp = menuBar->FindMenuItem(_("Call"), _("Hang Up"));
	helpAbout = menuBar->FindMenuItem(_("Help"), _("About"));

	wxPanel * panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	panel->SetBackgroundColour(wxColour(237,237,237));	// light gray
	wxBoxSizer * vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);

	// Create status & dial panel
	//btnContacts = new wxButton(panel, NULL, wxT("Contacts..."));
	//btnContacts->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnQuit));
	//btnContacts->SetFocus();

	// Address
	tcDest = new wxTextCtrl(panel, wxID_ANY, _("sip:"), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	tcDest->SetForegroundColour(wxColour(0,0,0));	// black
	tcDest->SetBackgroundColour(wxColour(255,255,255));	// white

	// Dial button
	btnDial = new wxButton(panel, tkID_DIAL_BTN, wxT("&Dial"));
	Connect(tkID_DIAL_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDial));
	btnDial->SetFocus();

	// Trace window
	// wxTE_RICH: on Windows, enables >64kb of data
	tcTrace = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP | wxSUNKEN_BORDER | wxTE_RICH | wxTE_READONLY);
	wxFont font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	tcTrace->SetFont(font);
	tcTrace->SetForegroundColour(wxColour(53,255,53)); // green
	tcTrace->SetBackgroundColour(wxColour(0,0,0)); // black

	// add the dial bar
	//hbox->Add(btnContacts, 0, wxALIGN_LEFT | wxALL, 5);
	hbox->Add(tcDest, 1, wxALIGN_LEFT | wxTOP | wxBOTTOM | wxLEFT | wxEXPAND, 5);
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

	// read accounts & establish default if none present
	accounts = model->GetAccountList(ACCOUNTS_FILE);
	if (accounts->GetCount() == 0) {
		Account * a = new Account("","","stun.ekiga.net");
		a->setName("Default");
		accounts->AddAccount(a);
	}

	// use icon
	//SetIcon(wxIcon(wxT("tkng.xpm")));

	// initialize GUI 
	OnAdjustControls(STATE_UNINITIALIZED);

}

void MainFrame::OnAdjustControls(const StateID state)
{
	menuBar->Enable(fileExit, true);
	menuBar->Enable(fileRegister, StateHelper::CanRegister(state));
	menuBar->Enable(editAccounts, false);
	//menuBar->Enable(editContacts, false);
	menuBar->Enable(callDial, StateHelper::CanDial(state));
	menuBar->Enable(callDialPad, (state == STATE_CONNECTED));
	menuBar->Enable(callHold, StateHelper::CanHold(state));
	menuBar->Enable(callAutoHold, StateHelper::CanHold(state));
	menuBar->Enable(callRetrieve, StateHelper::CanRetrieve(state));
	menuBar->Enable(callHangUp, StateHelper::CanHangUp(state));
	menuBar->Enable(helpAbout, true);
	if (StateHelper::CanDial(state)) {
		btnDial->Enable(true);
		Disconnect(tkID_DIAL_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnHangUp));
		Connect(tkID_DIAL_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDial));
		btnDial->SetLabel(_("&Dial"));
	} else if (StateHelper::CanHangUp(state)) {
		btnDial->Enable(true);
		Disconnect(tkID_DIAL_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnDial));
		Connect(tkID_DIAL_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainFrame::OnHangUp));
		btnDial->SetLabel(_("&Hang Up"));
	} else {
		btnDial->Enable(false);
	}
	//btnContacts->Enable(false);
}

void MainFrame::OnStateChange(wxStateChangeEvent & event)
{
	State * s = view->DequeueState();
	if (s == NULL) return;
	previousStateID = currentStateID;
	currentStateID = s->id;
	if (!(initialized && currentStateID == STATE_INITIALIZED)) {
		Trace(StateHelper::ToTrace(currentStateID, s->status, _Pstr2wxStr(s->message), model, initialized));
	}
	if (s->status == STATUS_FAILED) {
		if (currentStateID == STATE_INITIALIZING) {
			// registration attempt failed - alert user forcefully
			wxString msg(_("Unable to to initialize telephony subsystems.\nCheck STUN server, network connection, and NAT type."));
			wxMessageDialog * md = new wxMessageDialog(this, msg, _("Registration failed"), wxOK | wxICON_EXCLAMATION | wxCENTRE);
			md->ShowModal();
			if (!registerDialogIsOpen) {
				registerDialogIsOpen = true;
				new RegisterDialog(this, accounts);
			}
		} else if (currentStateID == STATE_REGISTERING) {
			// registration attempt failed - alert user forcefully
			wxString msg(_("Unable to register.\nCheck your account details and network connection."));
			wxMessageDialog * md = new wxMessageDialog(this, msg, _("Registration failed"), wxOK | wxICON_EXCLAMATION | wxCENTRE);
			md->ShowModal();
			if (!registerDialogIsOpen) {
				registerDialogIsOpen = true;
				new RegisterDialog(this, accounts);
			}
		}
	}
	if (currentTurn != s->turn) {
		// state has changed (not just a status change)
		currentTurn = s->turn;
		SetStatusText(StateHelper::ToStatus(currentStateID));
		OnAdjustControls(currentStateID);
		if (wxIsBusy()) wxEndBusyCursor();
		switch (currentStateID) {

			case STATE_UNINITIALIZED:
				if (!registerDialogIsOpen) {
					registerDialogIsOpen = true;
					new RegisterDialog(this, accounts);
				}
				break;

			case STATE_INITIALIZING:
				wxBeginBusyCursor();
				break;

			case STATE_INITIALIZED:
				if (regAction != NULL) {
					// ready to register
					RegisterAction * r = regAction;
					regAction = new RegisterAction(r->registrar, r->user, r->password, currentTurn);
					delete r;
					view->DoAction(regAction);
					regAction = NULL;
				}
				initialized = true;
				break;

			case STATE_REGISTERING:
				wxBeginBusyCursor();
				break;

			case STATE_DIALING:
				wxBeginBusyCursor();
				break;

			case STATE_AUTOHOLD:
			case STATE_MUTEAUTOHOLD:
				if (s->status == STATUS_AUTO_RETRIEVE) {
					// human was detected - play sound
					view->DoAction(new PlaySoundAction(HUMAN_DETECTED_WAV, currentTurn));
				}
				break;
				
			case STATE_DISCONNECTING:
				wxBeginBusyCursor();
				break;

			default:
				break;

		}
	}
	delete s;
}

void MainFrame::OnOpenRegisterDialog(wxCommandEvent & event)
{
	if (!registerDialogIsOpen) {
		registerDialogIsOpen = true;
		new RegisterDialog(this, accounts);
	}
}

void MainFrame::OnCloseRegisterDialog(wxRegisterDialogClosedEvent & event)
{
	registerDialogIsOpen = false;
}

void MainFrame::OnOpenDialPad(wxCommandEvent & event)
{
	if (!dialPadIsOpen) {
		dialPadIsOpen = true;
		new DialPad(this, view);
	}
}

void MainFrame::OnCloseDialPad(wxDialPadClosedEvent & event)
{
	dialPadIsOpen = false;
}

void MainFrame::OnRegister(const wxString & s, const wxString & r, const wxString & u, const wxString & p)
{
	if (regAction != NULL) {
		// regAction set NULL immediately after send, so if not NULL then not sent,
		// and if not sent, this thread still "owns" responsibility for delete
		delete regAction;
	}
	regAction = new RegisterAction(_wxStr2Pstr(r), _wxStr2Pstr(u), _wxStr2Pstr(p), currentTurn);
	if (currentStateID == STATE_UNINITIALIZED) {
		Action * a = new InitializeAction(_wxStr2Pstr(s), currentTurn);
		view->DoAction(a);
	} else if (currentStateID == STATE_INITIALIZED) {
		view->DoAction(regAction);
		regAction = NULL;
	}
	registerDialogIsOpen = false;
}

void MainFrame::OnDial(wxCommandEvent & event)
{
	Action * a = new DialAction(_wxStr2Pstr(tcDest->GetValue()), currentTurn);
	view->DoAction(a);
}

void MainFrame::OnHangUp(wxCommandEvent & event)
{
	view->DoAction(new DisconnectAction(currentTurn));
}

void MainFrame::OnHold(wxCommandEvent & event)
{
	view->DoAction(new HoldAction(currentTurn));
}

void MainFrame::OnAutoHold(wxCommandEvent & event)
{
	view->DoAction(new AutoHoldAction(currentTurn));
}

void MainFrame::OnMuteAutoHold(wxCommandEvent & event)
{
//	view->DoAction(new MuteAutoHoldAction(currentTurn));
}

void MainFrame::OnRetrieve(wxCommandEvent & event)
{
	view->DoAction(new RetrieveAction(currentTurn));
}

void MainFrame::OnQuit(wxCommandEvent & WXUNUSED(event))
{
	view->DoAction(new QuitAction(currentTurn));
	view->OnCloseApplication();
	Destroy();
}

void MainFrame::OnClose(wxCloseEvent & event)
{
	view->DoAction(new QuitAction(currentTurn));
	view->OnCloseApplication();
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
	// freeze the control to prevent scrollbar jumping
	tcTrace->Freeze();
	// print line-by-line to avoid scroll bugs in MSW
	wxString rhs = msg;
	rhs.Trim();
	wxString lhs;
	int lines = 0;
	while (!rhs.IsEmpty()) {
		lhs = rhs.BeforeFirst('\n');
		lhs += _("\n");
		rhs = rhs.AfterFirst('\n').Trim();
		if (lhs.IsEmpty()) continue;
		++lines;
		tcTrace->AppendText(lhs);
	}
	// scroll down by the correct number of lines + 1
	// (the extra line is important in special cases)
	tcTrace->ScrollLines( lines + 1);
	// Ensure that the last line is shown at the very bottom of the window
	tcTrace->ShowPosition( tcTrace->GetLastPosition() );
	// now thaw
	tcTrace->Thaw();
}

//////////////////////////////////////////////////////////////////////////////
// RegisterDialog

RegisterDialog::RegisterDialog(MainFrame * pwin, AccountList * accounts)
	: wxDialog(pwin, wxID_ANY, _("Register with SIP service"), wxDefaultPosition, wxSize(300, 212)),
	  pwin(pwin),
	  accounts(accounts),
	  tcStun(NULL),
	  tcRegistrar(NULL),
	  tcUser(NULL),
	  tcPassword(NULL)
{

	wxPanel * panel = new wxPanel(this, wxID_ANY);

	//
	// Text Boxes
	//

	Account * acct = accounts->GetAccount();
	wxString stun(_("stun.ekiga.net"));
	wxString registrar(_("ekiga.net"));
	wxString user(_("name@ekiga.net"));
	if (acct != NULL) {
		stun = _Pstr2wxStr(acct->GetStunServer());
		registrar = _Pstr2wxStr(acct->GetRegistrar());
		user = _Pstr2wxStr(acct->GetUser());
	}

	wxStaticText * lblStun = new wxStaticText(panel, wxID_ANY, _("STUN Server"));
	wxStaticText * lblRegistrar = new wxStaticText(panel, wxID_ANY, _("Registrar"));
	wxStaticText * lblUser = new wxStaticText(panel, wxID_ANY, _("User Name"));
	wxStaticText * lblPassword = new wxStaticText(panel, wxID_ANY, _("Password"));
	
	tcStun = new wxTextCtrl(panel, wxID_ANY, stun, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	tcRegistrar = new wxTextCtrl(panel, wxID_ANY, registrar, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	tcUser = new wxTextCtrl(panel, wxID_ANY, user, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER);
	tcPassword = new wxTextCtrl(panel, wxID_ANY, _(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxSUNKEN_BORDER | wxTE_PASSWORD);

	wxBoxSizer * vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer * hbox = new wxBoxSizer(wxHORIZONTAL);
	wxFlexGridSizer * fgs = new wxFlexGridSizer(4, 2, 10, 15);
	fgs->Add(lblStun);
	fgs->Add(tcStun, 1, wxEXPAND);
	fgs->Add(lblRegistrar);
	fgs->Add(tcRegistrar, 1, wxEXPAND);
	fgs->Add(lblUser);
	fgs->Add(tcUser, 1, wxEXPAND);
	fgs->Add(lblPassword);
	fgs->Add(tcPassword, 1, wxEXPAND);
	fgs->AddGrowableCol(1, 1);
	hbox->Add(fgs, 1, wxALL | wxEXPAND | wxALIGN_CENTER, 15);
	panel->SetSizer(hbox);
	vbox->Add(panel, 0, wxEXPAND);

	//
	// Buttons
	//

	wxButton * btnRegister = new wxButton(this, tkID_REGISTER_BTN, wxT("&Register"));
	Connect(tkID_REGISTER_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RegisterDialog::OnRegister));
	wxButton * btnCancel = new wxButton(this, wxID_CANCEL, wxT("&Cancel"));
	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(RegisterDialog::OnCancel));
	
	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(btnRegister, 0, wxRIGHT | wxTOP, 5);
	hbox->Add(btnCancel, 0, wxLEFT | wxTOP, 5);
	vbox->Add(hbox, 0, wxALIGN_RIGHT | wxRIGHT | wxLEFT | wxBOTTOM, 15);
	
	SetSizer(vbox);

	// Track close
	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(RegisterDialog::OnClose));
	
	// Tie enter in ANY text control to register
	Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(RegisterDialog::OnRegister));

	btnRegister->SetFocus();

	Centre();
	ShowModal();
}

void RegisterDialog::OnClose(wxCloseEvent & event)
{
	wxRegisterDialogClosedEvent evt( this );
	pwin->AddPendingEvent(evt);
	Destroy();
}

void RegisterDialog::OnRegister(wxCommandEvent & event)
{
	pwin->OnRegister(tcStun->GetValue(), tcRegistrar->GetValue(), tcUser->GetValue(), tcPassword->GetValue());
	Destroy();
}

void RegisterDialog::OnCancel(wxCommandEvent & event)
{
	wxRegisterDialogClosedEvent evt( this );
	pwin->AddPendingEvent(evt);
	Destroy();
}


//////////////////////////////////////////////////////////////////////////////
// DialPad

DialPad::DialPad(MainFrame * pwin, View * view)
	: wxDialog(pwin, wxID_ANY, _("Touch tone dial pad"), wxDefaultPosition, wxSize(200, 200), wxDEFAULT_DIALOG_STYLE | wxFRAME_FLOAT_ON_PARENT),
	  pwin(pwin),
	  view(view)
{
	
	wxGridSizer * grid = new wxGridSizer(4, 3, 3, 3);

	// row 1 buttons
	grid->Add(new wxButton(this, tkID_DIALPAD_ONE, wxT("1\n ")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_TWO, wxT("2\nabd")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_THREE, wxT("3\ndef")), 0, wxEXPAND);

	// row 2 buttons
	grid->Add(new wxButton(this, tkID_DIALPAD_FOUR, wxT("4\nghi")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_FIVE, wxT("5\njkl")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_SIX, wxT("6\nmno")), 0, wxEXPAND);

	// row 3 buttons
	grid->Add(new wxButton(this, tkID_DIALPAD_SEVEN, wxT("7\npqrs")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_EIGHT, wxT("8\ntuv")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_NINE, wxT("9\nwxyz")), 0, wxEXPAND);

	// row 4 buttons
	grid->Add(new wxButton(this, tkID_DIALPAD_STAR, wxT("*")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_ZERO, wxT("0")), 0, wxEXPAND);
	grid->Add(new wxButton(this, tkID_DIALPAD_POUND, wxT("#")), 0, wxEXPAND);

	wxBoxSizer * vbox = new wxBoxSizer(wxVERTICAL);
	vbox->Add(grid, 1, wxEXPAND | wxALL, 5);
	SetSizer(vbox);
	SetMinSize(wxSize(200,200));

	// Set up a "dummy" button to take the focus
	dummy = new wxButton(this, wxID_ANY, wxT(" "), wxPoint(-500,-500));
	dummy->SetFocus();

	// Track close
	Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(DialPad::OnClose));

	// Track button presses
	Connect(tkID_DIALPAD_ONE,   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressOne  ));
	Connect(tkID_DIALPAD_TWO,   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressTwo  ));
	Connect(tkID_DIALPAD_THREE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressThree));
	Connect(tkID_DIALPAD_FOUR,  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressFour ));
	Connect(tkID_DIALPAD_FIVE,  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressFive ));
	Connect(tkID_DIALPAD_SIX,   wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressSix  ));
	Connect(tkID_DIALPAD_SEVEN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressSeven));
	Connect(tkID_DIALPAD_EIGHT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressEight));
	Connect(tkID_DIALPAD_NINE,  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressNine ));
	Connect(tkID_DIALPAD_STAR,  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressStar ));
	Connect(tkID_DIALPAD_ZERO,  wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressZero ));
	Connect(tkID_DIALPAD_POUND, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(DialPad::OnPressPound));

	// Reveal the dialog
	Show(true);

}

void DialPad::OnClose(wxCloseEvent & event)
{
	wxDialPadClosedEvent evt( this );
	pwin->AddPendingEvent(evt);
	Destroy();
}

void DialPad::OnPressOne(wxCommandEvent & event)
{
	SendTouchTone('1');
}

void DialPad::OnPressTwo(wxCommandEvent & event)
{
	SendTouchTone('2');
}

void DialPad::OnPressThree(wxCommandEvent & event)
{
	SendTouchTone('3');
}

void DialPad::OnPressFour(wxCommandEvent & event)
{
	SendTouchTone('4');
}

void DialPad::OnPressFive(wxCommandEvent & event)
{
	SendTouchTone('5');
}

void DialPad::OnPressSix(wxCommandEvent & event)
{
	SendTouchTone('6');
}

void DialPad::OnPressSeven(wxCommandEvent & event)
{
	SendTouchTone('7');
}

void DialPad::OnPressEight(wxCommandEvent & event)
{
	SendTouchTone('8');
}

void DialPad::OnPressNine(wxCommandEvent & event)
{
	SendTouchTone('9');
}

void DialPad::OnPressStar(wxCommandEvent & event)
{
	SendTouchTone('*');
}

void DialPad::OnPressZero(wxCommandEvent & event)
{
	SendTouchTone('0');
}

void DialPad::OnPressPound(wxCommandEvent & event)
{
	SendTouchTone('#');
}

void DialPad::SendTouchTone(char ch)
{
	State * s = view->GetState();
	if (s != NULL && s->id == STATE_CONNECTED) {
		Action * a = new SendToneAction(s->turn, ch);
		view->DoAction(a);
	}
	dummy->SetFocus();
	delete s;
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

wxString StateHelper::ToTrace(const StateID & state, const StatusID & status, const wxString & message, Model * model, bool initialized)
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
			if (!initialized) {
				text << "Using STUN Server " << model->GetStunServer() << " of type " << model->GetStunType() << ".\n";
			}
			text << "Ready for SIP registration...";
			break;
		case STATE_REGISTERING:
			text << "Registering with " << model->GetRegistrar() << " as " << model->GetUserName() << "...";
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
			text << "Dialing " << model->GetDestination() << "...";
			break;
		case STATE_CONNECTED:
			text << "Connected to " << model->GetDestination() << ".";
			break;
		case STATE_AUTOHOLD:
			text << "Waiting for human to pick up " << model->GetDestination() << "...";
			break;
		case STATE_MUTEAUTOHOLD:
			text << "Waiting for human to pick up " << model->GetDestination() << " (muted)...";
			break;
		case STATE_HOLD:
			text << "Call to " << model->GetDestination() << " holding...";
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
	return (state == STATE_INITIALIZED || state == STATE_UNINITIALIZED);
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
		case STATE_DIALING:
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
