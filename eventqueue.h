
#ifndef _EVENTQUEUE_H_
#define _EVENTQUEUE_H_

#include <list>
#include "action.h"
class Action;
class TeleKarma;

class EventQueue {
	public:
		//XXX Once the new architecture is complete, this won't be necessary.
		EventQueue(TeleKarma & controller) : controller(controller) { }
		void Add(Action * action);
		Action * GetNext();

	private:
		std::list<Action *> queue;
		//XXX Once the new architecture is complete, this won't be necessary.
		TeleKarma & controller;
};

#endif // _EVENTQUEUE_H_
