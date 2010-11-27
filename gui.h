/*
 * gui.h
 *
 * Graphical User Interface for TeleKarma NG
 *
 */

#ifndef _GUI_H_
#define _GUI_H_

<<<<<<< HEAD
=======
#include "wxevt.h"

>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
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
<<<<<<< HEAD
class View;
class ModelListener;
=======

>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410

//////////////////////////////////////////////////////////////////////////////

class wxStateChangeEvent : public wxNotifyEvent
{
public:
<<<<<<< HEAD
    wxStateChangeEvent( wxObject * origin );
    virtual wxEvent * Clone();
=======
	// Note that original concept - passing new state as part of the event - failed for reasons as yet unknown
    //wxStateChangeEvent( wxObject * origin, const StateID & stateId, int turn, const StatusID & statusId, const wxString & message );
    wxStateChangeEvent( wxObject * origin );

    // accessors
	//StateID GetStateID();
	//StatusID GetStatusID();
	//int GetTurn();
	//const wxString & GetMessage();

    // required for sending with wxPostEvent(), wxAddPendingEvent()
    virtual wxEvent * Clone();

private:
    //StateID stateId;
	//StatusID statusId;
	//int turn;
	//wxString msg;
>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
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
<<<<<<< HEAD
class TeleKarmaNG: public wxApp, public View
=======
class TeleKarmaNG: public wxApp, public PProcess
>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
{
	PCLASSINFO(TeleKarmaNG, PProcess);
	public:
		/** Destructor waits for child threads. */
		~TeleKarmaNG();
		/** A dummy method for the benefit of PProcess. */
<<<<<<< HEAD
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

=======
		void Main();
		/** Fulfills wxApp requirement. */
		bool OnInit();
		/** Interface for StateMonitor */
		void OnStateChange(State * s);
		/** Interface for MainFrame */
		void OnWindowDone();
		/** Exit handler. */
		void Quit();
		/** For use by main window in dequeuing states */
		virtual State * DequeueState();
	private:
		Model * model;				// ptr to the model
		Controller * controller;	// ptr to the controller
		StateMonitor * monitor;		// ptr to state monitoring thread
		MainFrame * win;			// ptr to main window
		PSemaphore * tmutex;		// mutex protecting termination order
		State ** squeue;			// array implementation of state queue
		int sqhead, sqtail;			// pointers into the array of states
		int sqsize;					// size of the array implementing the sQ
		PSemaphore * smutex;		// mutex for state queue
};


//////////////////////////////////////////////////////////////////////////////


>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
/**
 * The main window.
 */
class MainFrame: public wxFrame
{
	public:
		/** Constructs the main window. */
<<<<<<< HEAD
		MainFrame(TeleKarmaNG * parent, const wxString & title, const wxPoint & pos, const wxSize & size);
=======
		MainFrame(TeleKarmaNG * parent, Model * model, const wxString & title, const wxPoint & pos, const wxSize & size);
>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
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
<<<<<<< HEAD
=======

>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
		// macro binds event handlers
		DECLARE_EVENT_TABLE()
	private:
		TeleKarmaNG * parent;
<<<<<<< HEAD
=======
		Model * model;
>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
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
};

<<<<<<< HEAD
//////////////////////////////////////////////////////////////////////////////

=======

//////////////////////////////////////////////////////////////////////////////


>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
/**
 * The registration window.
 */
class RegisterDialog : public wxDialog
{
	public:
		/** Constructs the main window. */
		RegisterDialog(MainFrame * parent, Model * model, int turn);
		/** Handles system-prompted close events */
		void OnClose(wxCloseEvent & event);
		/** Initiates registration and closes the dialog. */
		void OnRegister(wxCommandEvent & event);
		/** Close the dialog. */
		void OnCancel(wxCommandEvent & event);
	private:
		Model * model;
		wxTextCtrl * tcSIP;
		wxTextCtrl * tcUser;
		wxTextCtrl * tcPassword;
		wxButton * btnCancel;
		wxButton * btnRegister;
		int turn;
};

<<<<<<< HEAD
//////////////////////////////////////////////////////////////////////////////

// Events the GUI can generate
=======

//////////////////////////////////////////////////////////////////////////////


// Defines events that GUI can generate
>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
enum
{
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
	evSendSMS = 7,
};

// Binds events generated in the main window to event handlers
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
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

<<<<<<< HEAD
=======

>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
class StateHelper
{
	public:
		static wxString ToStatus(const StateID & state);
		static wxString ToTrace(const StateID & state, const StatusID & status, const wxString & message);
		static bool CanRegister(const StateID & state);
		static bool CanDial(const StateID & state);
		static bool CanHold(const StateID & state);
		static bool CanHangUp(const StateID & state);
		static bool CanRetrieve(const StateID & state);
		static bool IsRegistered(const StateID & state);
};

<<<<<<< HEAD
//////////////////////////////////////////////////////////////////////////////

=======

//////////////////////////////////////////////////////////////////////////////


/**
 * State Monitor Thread. Monitors model state and drives
 * events into the GUI when state changes.
 */
class StateMonitor : public PThread
{
	PCLASSINFO(StateMonitor, PThread);
	public:
		StateMonitor(TeleKarmaNG * parent, Model * model);
		void Main();
	private:
		Model * model;
		TeleKarmaNG * parent;
};



>>>>>>> bb8fbf66b06fd6c81c152271f16402663b1ee410
#endif // _GUI_H_