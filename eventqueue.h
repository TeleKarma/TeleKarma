
#include "telekarma.h"
#include "action.h"

#include <list>

class EventQueue {
	public:
		//XXX Once the new architecture is complete, this won't be necessary.
		EventQueue(TeleKarma & controller) : controller(controller) { }
		void Add(Action * action) {
			queue.push_back(action);
			//XXX Once the new architecture is complete, this won't be necessary.
			controller.ProcessNextEvent();
		}

		Action * GetNext() {
			Action * action = queue.front();
			queue.pop_front();
			return action;
		}

	private:
		list<Action *> queue;
		//XXX Once the new architecture is complete, this won't be necessary.
		TeleKarma & controller;
};
