/**
 * <p>
 * The TeleKarma NG Model superclass.
 * </p><p>
 * The Model holds system state and data. Notably, the model stores
 * the current state of the system and a queue of actions requested
 * by the View.
 * </p><p>
 * The Model is designed to be robust and easy to use in multithreaded 
 * environments. When provided with data, the Model makes and manages
 * its own internal copies of that data to avoid concurrent access
 * problems and to ensure that callers cannot deallocate or change
 * the contents of memory that the Model depends upon. Mutable
 * fields are protected by internal mutexes to prevent reading of
 * those fields in an inconsistent state. When asked to provide
 * callers with data, the Model returns a copy of its data constructed
 * exclusively for the caller.
 * </p><p>
 * <strong>Memory Management</strong>
 * </p><p>
 * The Model creates copies of objects. The Model's internal copies are
 * cleaned up by the Model. Responsibility for deallocating objects 
 * passed to the Model by callers and passed to callers by the Model are
 * the exclusive responsibility of the caller.
 * </p><p>
 * <strong>Thread Safety</strong>
 * </p><p>
 * This class is thread-safe and extensible. Extend this class with care.
 * </p><p>
 * <strong>Dependencies</strong>
 * </p><p>
 * The Action and State classes and subclasses passed to this class must 
 * implement a copy constructor and assignment operator.
 * </p>
 */

// **************************************************************
// Developement uncertainty: does polymorphism in C++ work
// such that a subclass of Action that has a larger size on
// the stack than Action can be assigned to Action with
// the assignment operator (presuming a properly implemented
// assignment operator) such that the result can be cast back
// to the subclass without data loss?
// **************************************************************

#ifndef _MODEL_H_
#define _MODEL_H_

#include "ptlib.h"

/** The maximum capacity of the action queue. */
#define QUEUE_SIZE 50


class Model {

	public:

		/**
		 * Default constructor. Creates a model with an action
		 * queue of fixed size {@link #QUEUE_SIZE}.
		 */
		Model();

		/**
		 * Optional constructor. Creates a model with an action
		 * queue of fixed size specified by the caller.
		 * @param queueSize how big to make the queue.
		 */
		Model(int queueSize);

		/**
		 * Virtual destructor. Deallocates the current state and
		 * any enqueued actions.
		 */
		virtual ~Model();

		/**
		 * Enqueues an action for later processing. Atomic.
		 * This method makes a complete copy of the Action
		 * passed to it. The caller retains responsibility for
		 * deallocating the Action passed as a parameter.
		 * Override this method at your own risk.
		 * @return false if the Action cannot be enqueued because
		 *         the queue is full; true otherwise.
		 */
		virtual bool EnqueueAction(const Action & action);

		/**
		 * Dequeues an action from the queue of Actions and returns
		 * a copy of that Action. Atomic.
		 * Override this method at your own risk.
		 * @param none the Action to return if the queue is empty.
		 * @return a copy of either the Action dequeued or, if the
		 *         queue is empty, the Action passed in as a 
		 *         parameter.
		 */
		virtual Action DequeueAction(const Action & none);

		/**
		 * Returns a copy of the current State object. Atomic. 
		 * Note that the current state might change immediately
		 * after a call to this method, in which case the State
		 * copy returned will describe a historical state of the
		 * system. The Controller should inspect the turn number 
		 * (serial number) of the State associated with an Action
		 * to ensure that the requested Action was intended to be 
		 * applied to the current State.
		 * @param defaultState the State to return if the model
		 *                     does not know the current state 
		 *                     (which indicates an error in the
		 *                     implementation of the model).
		 * @return a copy of the current State object or, if the
		 *         model is implemented inappropriately and does
		 *         not have a current state, a copy of the State
		 *         passed to this method as a parameter.
		 */
		virtual const State GetState(const State & defaultState) const;

		/**
		 * Sets the current state. Atomic. Makes a copy of the
		 * argument. The caller retains responsiblity for
		 * deallocating memory allocated to the parameter. The
		 * caller is also responsible for constructing a State
		 * with the correct turn number.
		 * @param state the new state.
		 */
		virtual void SetState(const State & state);

	private:
		Action * queue;		// array implementation of action queue
		int qhead, qtail;	// pointers into the array of actions
		int qsize;			// size of the array implementing the queue
		State * state;		// current state
		PSemaphore qMutex;	// mutex for action queue
		PSemaphore sMutex;	// mutex for current state

		/**
		 * Explicitely disabled copy constructor.
		 * No implementation, by design.
		 */
		Model & Model(const Model & source);
		
		/**
		 * Explicitely disabled assignment operator.
		 * No implementation, by design.
		 */
		Model & operator=(const Model & rhs);

};


#endif // _MODEL_H_