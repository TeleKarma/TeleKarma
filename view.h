
#ifndef _VIEW_H_
#define _VIEW_H_
#include <stdlib.h>

class Action;
class Controller;
class Model;
class State;

class View : public PProcess {
	
	PCLASSINFO(View, PProcess);
	
	public:
		View() : controller(NULL), model(NULL) { }
		~View();
		virtual State * GetState();

	protected:
		void DoAction(Action * action);

		Model * model;
		Controller * controller;
};

#endif //_VIEW_H_
