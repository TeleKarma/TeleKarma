
#include <ptlib.h> 

#include "view.h"

#include "model.h"


void View::DoAction(Action * action) {
	model->EnqueueAction(action);
}

State * View::GetState() {
	return model->GetState();
}
