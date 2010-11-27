#ifndef _VIEW_H_
#define _VIEW_H_

#include <ptlib/pprocess.h>
#include "model.h"

class Action;
class Controller;
class State;

/**
 * Abstract superclass for all TeleKarma views. Subclass
 * this class to create a new view.
 */
class View : public PProcess, public ModelListener {
	
	// PTLib macro - establishes required methods to
	// support PProcess.
	PCLASSINFO(View, PProcess);

	public:

		/**
		 * Default constructor. Initializes controller
		 * and model pointers to NULL. Note that
		 * subclasses are responsible for instantiating
		 * an appropriate model and controller.
		 */
		View() : controller(NULL), model(NULL) { }

		/**
		 * Virtual destructor, does nothing.
		 */
		virtual ~View() { }

		/**
		 * Implementation of ModelListener. Does nothing.
		 * Subclasses may or may not wish to reimplement
		 * this method.
		 */
		virtual void OnStateChange() { }

	protected:

		/**
		 * Returns the current State of the Model.
		 * @return the current State of the Model.
		 */
		virtual State * GetState();

		/**
		 * Dequeues a state from the model and returns it.
		 * @return a pointer to an instance of a state that 
		 *         the caller is responsible for deleting,
		 *         or NULL if the there have already been 
		 *         at least as many {@link Model#DequeueState())
		 *         calls as {@link Model#SetState(State *)} calls.
		 */
		virtual State * DequeueState();

		/**
		 * Enqueues an Action in the Model. Caller should not
		 * modify the Action after enqueuing it.
		 * @param action an action for the controller to take.
		 */
		virtual void DoAction(Action * action);

		/**
		 * The model used by the view. Subclasses are responsible
		 * for instantiation.
		 */
		Model * model;

		/**
		 * The controller used by the view. Subclasses are 
		 * responsible for instantiation.
		 */
		Controller * controller;

};

#endif //_VIEW_H_
