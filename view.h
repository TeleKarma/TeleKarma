
#ifndef _VIEW_H_
#define _VIEW_H_

class Action;
class PString;
class TeleKarma;

class View {
	public:
		View(TeleKarma * controller) : controller(controller) { }
		virtual void Run() = 0;

	protected:
		void DoAction(Action * action);
		TeleKarma * controller;
};

#endif //_VIEW_H_
