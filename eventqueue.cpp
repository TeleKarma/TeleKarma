
#include "eventqueue.h"

#include "telekarma.h"

#include <list>

void EventQueue::Add(Action * action) {
	queue.push_back(action);
	//XXX Once the new architecture is complete, this won't be necessary.
	controller.ProcessNextEvent();
}

Action * EventQueue::GetNext() {
	Action * action = queue.front();
	queue.pop_front();
	return action;
}
