
#include "view.h"

#include "action.h"
#include "eventqueue.h"
#include "telekarma.h"


void View::DoAction(Action * action) {
	controller->eventQueue->Add(action);
}
