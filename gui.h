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
class DialPad;

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

class wxRegisterDialogClosedEvent : public wxNotifyEvent
{
public:
    wxRegisterDialogClosedEvent( wxObject * origin );
    virtual wxEvent * Clone();
};

DECLARE_EVENT_TYPE( wxEVT_REGISTER_DIALOG_CLOSED, -1 )

typedef void (wxEvtHandler::*wxRegisterDialogClosedEventFunction)(wxRegisterDialogClosedEvent&);

#define EVT_REGISTER_DIALOG_CLOSED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_REGISTER_DIALOG_CLOSED, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( wxRegisterDialogClosedEventFunction, & fn ), (wxObject *) NULL ),

DEFINE_EVENT_TYPE( wxEVT_REGISTER_DIALOG_CLOSED )

//////////////////////////////////////////////////////////////////////////////

class wxDialPadClosedEvent : public wxNotifyEvent
{
public:
    wxDialPadClosedEvent( wxObject * origin );
    virtual wxEvent * Clone();
};

DECLARE_EVENT_TYPE( wxEVT_DIAL_PAD_CLOSED, -1 )

typedef void (wxEvtHandler::*wxDialPadClosedEventFunction)(wxDialPadClosedEvent&);

#define EVT_DIAL_PAD_CLOSED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( wxEVT_DIAL_PAD_CLOSED, id, -1, \
    (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) (wxNotifyEventFunction) \
    wxStaticCastEvent( wxDialPadClosedEventFunction, & fn ), (wxObject *) NULL ),

DEFINE_EVENT_TYPE( wxEVT_DIAL_PAD_CLOSED )

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
		void OnCloseApplication();
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
		MainFrame(TeleKarmaNG * view, Model * model, const wxString & title, const wxPoint & pos, const wxSize & size);
		/** Handles registration initiation request */
		void OnOpenRegisterDialog(wxCommandEvent & event);
		/** Handles opening of dial pad dialog. */
		void OnOpenDialPad(wxCommandEvent & event);
		/** Handles registration information submission */
		void OnRegister(const wxString & s, const wxString & r, const wxString & u, const wxString & p);
		/** Handles evExit (file->exit). */
		void OnQuit(wxCommandEvent& event);
		/** Handles system-prompted close events */
		void OnClose(wxCloseEvent & event);
		/** Handles evAbout. */
		void OnAbout(wxCommandEvent & event);
		/** Responds to state changes */
		void OnStateChange(wxStateChangeEvent & event);
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
		/** Adjust to closure of register dialog. */
		void OnCloseRegisterDialog(wxRegisterDialogClosedEvent & event);
		/** Adjust to closure of dial pad. */
		void OnCloseDialPad(wxDialPadClosedEvent & event);
		/** Adjust the enabled/disabled states of controls */
		void OnAdjustControls(const StateID state);
		/** Add a trace message to the console */
		void Trace(const wxString & msg);
		// macro binds event handlers
		DECLARE_EVENT_TABLE()
	private:
		TeleKarmaNG * view;
		Model * model;
		wxMenu * menuFile;
		wxMenu * menuEdit;
		wxMenu * menuCall;
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
		int callDialPad;
		int callHold;
		int callAutoHold;
		int callRetrieve;
		int callHangUp;
		int helpAbout;
		StateID previousStateID;
		StateID currentStateID;
		int currentTurn;
		RegisterAction * regAction;
		AccountList * accounts;
		bool initialized;			// true after first STATE_INITIALIZED
		bool initAttempted;			// true after first STATE_INITIALIZING
		bool dialPadIsOpen;
		bool registerDialogIsOpen;
		//PSemaphore mutex;
};

//////////////////////////////////////////////////////////////////////////////

/** Modal registration dialog box. */
class RegisterDialog : public wxDialog
{
	public:
		/**
		 * Constructor.
		 * @param pwin the parent window.
		 * @param accounts user's account list.
		 */
		RegisterDialog(MainFrame * pwin, AccountList * accounts);
		/**
		 * Handles system-prompted close events. Notifies pwin. 
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnClose(wxCloseEvent & event);
		/**
		 * Initiates registration via pwin and closes the dialog.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnRegister(wxCommandEvent & event);
		/**
		 * Closes this dialog when the cancel button is clicked.
		 * Notifies pwin. Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnCancel(wxCommandEvent & event);
	private:
		MainFrame * pwin;
		AccountList * accounts;
		wxTextCtrl * tcStun;
		wxTextCtrl * tcRegistrar;
		wxTextCtrl * tcUser;
		wxTextCtrl * tcPassword;
};

//////////////////////////////////////////////////////////////////////////////

/**
 * Dial pad for touch tone transmission. Sends fixed-duration touch tones when
 * buttons are released, rather sending a touch tone continuously while the
 * button is depressed.
 */
class DialPad : public wxDialog
{
	public:
		/**
		 * Constructor.
		 * @param pwin parent window.
		 * @param view for access to {@link View#DoAction(Action *)}.
		 */
		DialPad(MainFrame * pwin, View * view);
		/**
		 * Responds to system close events. Notifies pwin.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnClose(wxCloseEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressOne(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressTwo(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressThree(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressFour(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressFive(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressSix(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressSeven(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressEight(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressNine(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressStar(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressZero(wxCommandEvent & event);
		/**
		 * Sends a touch tone action to model upon click of 
		 * associated button.
		 * Exclusively for use by wxWidgets event dispatcher.
		 */
		void OnPressPound(wxCommandEvent & event);
	private:
		MainFrame * pwin;
		View * view;
		wxButton * dummy;
		/**
		 * Sends a touch tone action to model.
		 */
		void SendTouchTone(char ch);
};

//////////////////////////////////////////////////////////////////////////////

/** Events the GUI can generate */
enum
{
	/** File -> Register... */
	evRegister     = 1,
	/** Edit -> Accounts... */
	evAccounts     = 2,
	/** Edit -> Contacts... */
	evContacts     = 3,
	/** Call -> Dial */
	evDial         = 4,
	/** Call -> Touch Tones... */
	evDialPad      = 5,
	/** Call -> Hold */
	evHold         = 6,
	/** Call -> AutoHold */
	evAutoHold     = 7,
	/** Call -> Retrieve */
	evRetrieve     = 8,
	/** Call -> Hang Up */
	evHangUp       = 9
};

/** Unique widget identifiers. */
enum
{
	/** Register button on register dialog. */
	tkID_REGISTER_BTN  = 101,
	/** Dial pad button. */
	tkID_DIALPAD_ONE   = 102,
	/** Dial pad button. */
	tkID_DIALPAD_TWO   = 103,
	/** Dial pad button. */
	tkID_DIALPAD_THREE = 104,
	/** Dial pad button. */
	tkID_DIALPAD_FOUR  = 105,
	/** Dial pad button. */
	tkID_DIALPAD_FIVE  = 106,
	/** Dial pad button. */
	tkID_DIALPAD_SIX   = 107,
	/** Dial pad button. */
	tkID_DIALPAD_SEVEN = 108,
	/** Dial pad button. */
	tkID_DIALPAD_EIGHT = 109,
	/** Dial pad button. */
	tkID_DIALPAD_NINE  = 110,
	/** Dial pad button. */
	tkID_DIALPAD_ZERO  = 111,
	/** Dial pad button. */
	tkID_DIALPAD_POUND = 112,
	/** Dial pad button. */
	tkID_DIALPAD_STAR  = 113,
	/** Dial/Hang Up button in main frame. */
	tkID_DIAL_BTN      = 114,
	/** SIP phone number text box */
	tkID_DESTINATION   = 115
};

// Binds events generated in the main window to event handlers
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(evRegister, MainFrame::OnOpenRegisterDialog)
    EVT_MENU(wxID_EXIT, MainFrame::OnQuit)
    EVT_MENU(evDial, MainFrame::OnDial)
    EVT_MENU(evDialPad, MainFrame::OnOpenDialPad)
    EVT_MENU(evHangUp, MainFrame::OnHangUp)
    EVT_MENU(evHold, MainFrame::OnHold)
    EVT_MENU(evAutoHold, MainFrame::OnAutoHold)
    EVT_MENU(evRetrieve, MainFrame::OnRetrieve)
    EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
	EVT_STATE(wxID_ANY, MainFrame::OnStateChange)
	EVT_REGISTER_DIALOG_CLOSED(wxID_ANY, MainFrame::OnCloseRegisterDialog)
	EVT_DIAL_PAD_CLOSED(wxID_ANY, MainFrame::OnCloseDialPad)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////////

/**
 * Utilities for mapping state id and other state-derived data to meaningful
 * semantics and messages.
 */
class StateHelper
{
	public:
		/**
		 * Translate from state id to status bar message.
		 * @param state from {@link State#id}.
		 */
		static wxString ToStatus(const StateID & state);
		/**
		 * Generate a message suitable for display in the trace window.
		 * @param state from {@link State#id}.
		 * @param status from {@link State#id}.
		 * @param message from {@link State#id}., converted to wxString.
		 * @param model the {@link Model}.
		 * @param initialized true if {@link State#STATE_INITIALIZED} has
		 *                    already been seen at least once.
		 */
		static wxString ToTrace(const StateID & state, const StatusID & status, const wxString & message, Model * model, bool initialized);
		/**
		 * Determine whether the registration is allowed.
		 * @param state from {@link State#id}.
		 */
		static bool CanRegister(const StateID & state);
		/**
		 * Determine whether a call can be placed in the current state.
		 * Calls can be placed only in {@link STATE_REGISTERED} state.
		 * @param state from {@link State#id}.
		 */
		static bool CanDial(const StateID & state);
		/**
		 * Determine whether a connection exists and can be put on hold.
		 * A call already on hold or autohold or not connected cannot be
		 * put on hold.
		 * @param state from {@link State#id}.
		 */
		static bool CanHold(const StateID & state);
		/**
		 * Determine whether hanging up is allowed. Hanging up is allowed
		 * at any time during dialing and any form of connected state,
		 * including hold and autohold, but not during the disconnection
		 * process or if it is known that the remote party has disconnected.
		 * @param state from {@link State#id}.
		 */
		static bool CanHangUp(const StateID & state);
		/**
		 * Determine whether it is possible to retrieve a call from any form
		 * of hold. A call must be on hold or autohold to be retrievable.
		 * @param state from {@link State#id}.
		 */
		static bool CanRetrieve(const StateID & state);
		/**
		 * Determine whether the user is registered with a SIP registrar.
		 * @param state from {@link State#id}.
		 */
		static bool IsRegistered(const StateID & state);
};

//////////////////////////////////////////////////////////////////////////////

#endif // _GUI_H_