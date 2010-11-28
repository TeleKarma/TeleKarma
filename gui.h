/*
 * gui.h
 *
 * Graphical User Interface for TeleKarma NG
 *
 */

#ifndef _GUI_H_
#define _GUI_H_

#define PRODUCT_NAME "TeleKarma NG"
#define VERSION "0.1.002"
#define COPYRIGHT_HOLDER "Thomas Stellard, Michael Volk, Nikhil Tripathi and Peter Batzel"

class wxApp;
class PProcess;
class PString;
class State;
class StateMonitor;
class MainFrame;
class PSemaphore;
enum  StateID;
class RegisterDialog;
class View;
class ModelListener;
class AccountList;

//////////////////////////////////////////////////////////////////////////////

/**
 * Helper function for conversion of wxString to Pstring.
 * Uses PwxString.
 */
//PString _wxStr2Pstr(const wxString &);

/**
 * Helper function for conversion of PString to wxString.
 * Lacks efficiency and elegance, but gets the job done.
 */
//wxString _Pstr2wxStr(const PString &);

//////////////////////////////////////////////////////////////////////////////

class wxStateChangeEvent : public wxNotifyEvent
{
public:
    wxStateChangeEvent( wxObject * origin );
    virtual wxEvent * Clone();
};

DECLARE_EVENT_TYPE( wxEVT_STATE_CHANGE, -1 )

typedef void (wxEvtHandler::*wxStateChangeEventFunction)(wxStateChangeEvent&);

#define EVT_STATE(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_STATE_CHANGE, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( wxStateChangeEventFunction, & fn ), (wxObject *) NULL ),

DEFINE_EVENT_TYPE( wxEVT_STATE_CHANGE )

//////////////////////////////////////////////////////////////////////////////

/**
 * The main process.
 */
class TeleKarmaNG: public wxApp, public View
{
	PCLASSINFO(TeleKarmaNG, PProcess);
	public:
		/** Destructor waits for child threads. */
		~TeleKarmaNG();
		/** A dummy method for the benefit of PProcess. */
		void Main() { }
		/** Fulfills wxApp requirement. */
		bool OnInit();
		/** Callback for Model upon state change (see ModelListener). */
		void OnStateChange();
		/** Callback for MainFrame upon termination. */
		void OnWindowDone();
		/** Exit handler. */
		void Quit();
	private:
		MainFrame * win;		// ptr to main window
		PSemaphore * mutex;		// mutex protecting termination order
};

//////////////////////////////////////////////////////////////////////////////

/**
 * The main window.
 */
class MainFrame: public wxFrame
{
	public:
		/** Constructs the main window. */
		MainFrame(TeleKarmaNG * parent, Model * model, const wxString & title, const wxPoint & pos, const wxSize & size);
		/** Handles registration initiation request */
		void OnOpenRegisterDialog(wxCommandEvent & event);
		/** Handles registration information submission */
		void OnRegister(const wxString & s, const wxString & r, const wxString & u, const wxString & p);
		/** Handles evExit (file->exit). */
		void OnQuit(wxCommandEvent& event);
		/** Handles system-prompted close events */
		void OnClose(wxCloseEvent & event);
		/** Handles evAbout. */
		void OnAbout(wxCommandEvent & event);
		/** Responds to state changes */
		void OnStateChange(wxStateChangeEvent & evt);
		/** Dials */
		void OnDial(wxCommandEvent & event);
		/** Hangs Up */
		void OnHangUp(wxCommandEvent & event);
		/** Place call on hold */
		void OnHold(wxCommandEvent & event);
		/** Place call on autohold */
		void OnAutoHold(wxCommandEvent & event);
		/** Place call on mute autohold */
		void OnMuteAutoHold(wxCommandEvent & event);
		/** Retrieve call from autohold */
		void OnRetrieve(wxCommandEvent & event);
		/** Adjust the enabled/disabled states of controls */
		void OnAdjustControls(const StateID state);
		/** Add a trace message to the console */
		void Trace(const wxString & msg);
		// macro binds event handlers
		DECLARE_EVENT_TABLE()
	private:
		TeleKarmaNG * parent;
		Model * model;
		wxMenu * menuFile;
		wxMenu * menuEdit;
		wxMenu * menuCall;
		wxMenu * menuSMS;
		wxMenu * menuHelp;
		wxMenuBar * menuBar;
		wxButton * btnContacts;
		wxButton * btnDial;
		wxTextCtrl * tcDest;
		wxTextCtrl * tcTrace;
		// menu item ids
		int fileExit;
		int fileRegister;
		int editAccounts;
		int editContacts;
		int callDial;
		int callHold;
		int callAutoHold;
		int callRetrieve;
		int callHangUp;
		int smsSendText;
		int helpAbout;
		int turn;
		StateID state;
		RegisterAction * reg;
		AccountList * accounts;
		bool initialized;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * The registration window.
 */
class RegisterDialog : public wxDialog
{
	public:
		/** Constructs the main window. */
		RegisterDialog(MainFrame * parent, AccountList * accounts);
		/** Handles system-prompted close events */
		void OnClose(wxCloseEvent & event);
		/** Initiates registration and closes the dialog. */
		void OnRegister(wxCommandEvent & event);
		/** Close the dialog. */
		void OnCancel(wxCommandEvent & event);
	private:
		MainFrame * mainFrame;
		AccountList * accounts;
		wxTextCtrl * tcStun;
		wxTextCtrl * tcRegistrar;
		wxTextCtrl * tcUser;
		wxTextCtrl * tcPassword;
		wxButton * btnCancel;
		wxButton * btnRegister;
};

//////////////////////////////////////////////////////////////////////////////

// Events the GUI can generate
enum
{
	// File menu
	evRegister = 11,
	// Edit menu
	evAccounts = 9,
	evContacts = 10,
	// Call menu
	evDial = 2,
	evHangUp = 3,
	evHold = 4,
	evAutoHold = 5,
	evRetrieve = 6,
	// SMS menu
	evSendSMS = 7
};

/** Unique Widget IDs */
enum
{
	tkID_REGISTER_BTN = 101
};

// Binds events generated in the main window to event handlers
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(evRegister, MainFrame::OnOpenRegisterDialog)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(evDial, MainFrame::OnDial)
    EVT_MENU(evHangUp, MainFrame::OnHangUp)
    EVT_MENU(evHold, MainFrame::OnHold)
    EVT_MENU(evAutoHold, MainFrame::OnAutoHold)
    EVT_MENU(evRetrieve, MainFrame::OnRetrieve)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
	EVT_STATE(wxID_ANY, MainFrame::OnStateChange)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////////

class StateHelper
{
	public:
		static wxString ToStatus(const StateID & state);
		static wxString ToTrace(const StateID & state, const StatusID & status, const wxString & message, Model * model, bool initialized);
		static bool CanRegister(const StateID & state);
		static bool CanDial(const StateID & state);
		static bool CanHold(const StateID & state);
		static bool CanHangUp(const StateID & state);
		static bool CanRetrieve(const StateID & state);
		static bool IsRegistered(const StateID & state);
};

//////////////////////////////////////////////////////////////////////////////

#endif // _GUI_H_