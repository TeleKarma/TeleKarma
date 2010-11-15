
#include "view.h"

#include "action.h"
#include "model.h"
#include "eventqueue.h"
#include "telekarma.h"


void View::DoAction(Action * action) {
	model->EnqueueAction(action);
}

State * GetState() {
	return model->GetState();
}
