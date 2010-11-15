
#ifndef _VIEW_H_
#define _VIEW_H_
#include <stdlib.h>

class Action;
class Controller;
class Model;
class State;

class View {
	public:
		View() : controller(NULL), model(NULL) { }
		~View();
		virtual void Main() = 0;
		virtual State * GetState();

	protected:
		void DoAction(Action * action);

		Model * model;
		Controller * controller;
};

#endif //_VIEW_H_
