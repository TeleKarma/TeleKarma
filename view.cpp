
#include "view.h"

#include "action.h"
#include "eventqueue.h"
#include "telekarma.h"


void View::Register(const PString & registrar, const PString & user, const PString & password) {
	RegisterAction * registerAction = new RegisterAction(registrar, user, password);
	controller->eventQueue->Add(registerAction);
}

void View::Dial(const PString & dest) {
	DialAction * dialAction = new DialAction(dest);
	controller->eventQueue->Add(dialAction);
}

void View::Hold() {
	HoldAction * holdAction = new HoldAction();
	controller->eventQueue->Add(holdAction);
}

void View::AutoHold() {
	AutoHoldAction * autoHoldAction = new AutoHoldAction();
	controller->eventQueue->Add(autoHoldAction);
}

void View::Retrieve() {
	RetrieveAction * retrieveAction = new RetrieveAction();
	controller->eventQueue->Add(retrieveAction);
}

void View::Disconnect() {
	DisconnectAction * disconnectAction = new DisconnectAction();
	controller->eventQueue->Add(disconnectAction);
}

void View::Quit() {
	QuitAction * quitAction = new QuitAction();
	controller->eventQueue->Add(quitAction);
}
