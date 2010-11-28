/*
 * model.h
 *
 * Definition of Model superclass.
 *
 */

#ifndef _MODEL_H_
#define _MODEL_H_

/** The default capacity of the action and error message queues. */
#define QUEUE_SIZE 50

class Action;
class State;


/**
 * <p>
 * Interface for a class that can register itself with the Model for
 * notification upon state changes.
 * </p>
 */
class ModelListener
{
	public:
		/**
		 * Abstract method gets called when 
		 * {@link Model::SetState(State *)} is called.
		 */
		virtual void OnStateChange() = 0;
};

/**
 * <p>
 * The TeleKarma NG Model superclass.
 * </p><p>
 * The Model holds system state and related data. Notably, the model 
 * stores the current state of the system, a queue of action requests
 * generated by the View and a queue of error messages generated by
 * the Controller.
 * </p><p>
 * The Model is designed to be robust and easy to use in multithreaded
 * environments. All mutable internal fields are protected by private
 * mutexes to prevent reading of those fields in an inconsistent state.
 * </p><p>
 * <strong>Memory Management</strong>
 * </p><p>
 * It is critically important for developers utilizing methods of this 
 * class to read and understand the documentation for each method used. 
 * Method documentation discusses the caller's memory management 
 * responsibilities. These responsibilities vary depending on which
 * method is being used.
 * </p><p>
 * <strong>Thread Safety</strong>
 * </p><p>
 * This class is thread-safe when used properly. Extend this class with 
 * great care to preserve its thread-safe attributes. This class is 
 * designed to share information between threads. This mission motivates
 * a concurrency-aware design that makes a best effort to prevent 
 * concurrency errors. However, this class does not prevent all possible
 * concurrency errors that are potentially associated with its use. 
 * Developers must read and understand the documentation associated with 
 * each method.
 * </p><p>
 * <strong>Dependencies</strong>
 * </p><p>
 * All State classes must correctly implement a clone method that
 * returns a pointer to a new and complete copy of the object. This
 * method must never return a pointer to a copy of the object shared
 * with another caller and must not retain a copy of the pointer
 * returned.
 * </p>
 */
class Model
{

	public:

		/**
		 * Default constructor. Creates a model with action
		 * and error message queues of fixed size equal to 
		 * {@link #QUEUE_SIZE}.
		 */
		Model();

		/**
		 * Optional constructor. Creates a model with action
		 * and error message queues of fixed size specified by the 
		 * caller using the queueSize parameter.
		 * @param queueSize how big to make the queues.
		 */
		Model(int queueSize);

		/**
		 * <p>
		 * Virtual destructor. Deallocates:
		 * <ol>
		 * <li>All internally allocated memory.
		 * <li>The memory allocated to the current state.
		 * <li>The memory allocated to any Action objects
		 *    remaining in the action queue.
		 * <li>The memory allocated to any error messages
		 *    remaining in the error message queue.
		 * </ol>
		 * </p>
		 */
		virtual ~Model();

		/**
		 * <p>
		 * Enqueues an action.
		 * </p><p>
		 * <strong>Memory Management</strong>
		 * </p><p>
		 * This class assumes responsibility for deallocating the
		 * memory addressed by the action parameter if and only if
		 * this method returns true. The caller retains 
		 * responsibility for deallocating the memory addressed by
		 * the action parameter if this method returns false, and
		 * is free to do so at a time of the caller's choosing.
		 * </p><p>
		 * </p><p>
		 * <strong>Thread Safety</strong>
		 * </p><p>
		 * This method is intended for use by a single producer
		 * thread (the View). It may be used by multiple producer
		 * threads without endangering its thread safety features.
		 * Callers <strong>must not modify or deallocate</strong>
		 * the memory allocated to the pointer passed into this
		 * method after calling this method. The pointer passed 
		 * into this method must be a pointer to memory on the 
		 * heap. Failure to adhere to these guidelines may defeat 
		 * the thread safety features of this class.
		 * </p><p>
		 * This class stores the pointer passed to this method 
		 * and may share that pointer with a consumer. This method 
		 * does not make a copy the memory addressed by the pointer.
		 * </p><p>
		 * Override this method at your own risk!!
		 * </p>
		 * @param action pointer to an Action object on the heap.
		 * @return false if the pointer cannot be enqueued because
		 *         the queue is full; true otherwise.
		 */
		virtual bool EnqueueAction(Action * action);

		/**
		 * <p>
		 * Dequeues and returns a pointer to an Action.
		 * </p><p>
		 * <strong>Memory Management</strong>
		 * </p><p>
		 * The caller assumes responsibility for deallocating the
		 * memory addressed by the pointer returned by this method.
		 * That memory should be deallocated using delete. The 
		 * caller may deallocate the memory at a time of the
		 * caller's choosing.
		 * </p><p>
		 * </p><p>
		 * <strong>Thread Safety</strong>
		 * </p><p>
		 * This method is intended for use by a single consumer
		 * thread (the Controller). It may be used by multiple
		 * consumer threads without endangering built-in thread
		 * safety features. An enqueued pointer is never returned
		 * more than once, so multiple consumers cannot gain 
		 * access to the same object solely by using this method.
		 * Callers must deallocate, and are free to modify, the
		 * memory addressed by the pointer returned by this
		 * method. Failure to adhere to these guidelines may 
		 * defeat the thread safety features of this class.
		 * </p><p>
		 * Override this method at your own risk!!
		 * </p>
		 * @return a pointer to a previously enqueued Action or
		 *         NULL if the queue is empty.
		 */
		virtual Action * DequeueAction();

		/**
		 * <p>
		 * Dequeues and returns a pointer to a state. States are
		 * dequeued in the order they were set.
		 * </p><p>
		 * <strong>Memory Management</strong>
		 * </p><p>
		 * The caller assumes responsibility for deallocating the
		 * memory addressed by the pointer returned by this method.
		 * That memory should be deallocated using delete. The 
		 * caller may deallocate the memory at a time of the
		 * caller's choosing.
		 * </p><p>
		 * </p><p>
		 * <strong>Thread Safety</strong>
		 * </p><p>
		 * This method is intended for use by a single consumer
		 * thread (the View). It may be used by multiple
		 * consumer threads without endangering built-in thread
		 * safety features. An enqueued pointer is never returned
		 * more than once, so multiple consumers cannot gain 
		 * access to the same object solely by using this method.
		 * Callers must deallocate, and are free to modify, the
		 * memory addressed by the pointer returned by this
		 * method. Failure to adhere to these guidelines may 
		 * defeat the thread safety features of this class.
		 * </p><p>
		 * Override this method at your own risk!!
		 * </p>
		 * @return a pointer to a previously enqueued state
		 *         or NULL if the queue is empty.
		 */
		virtual State * DequeueState();

		/**
		 * <p>
		 * Returns a copy of the current State object.
		 * </p><p>
		 * <strong>Memory Management</strong>
		 * </p><p>
		 * The caller assumes responsibility for deallocating the
		 * memory addressed by the pointer returned by this method.
		 * That memory should be deallocated using delete. The 
		 * caller may deallocate the memory at a time of the
		 * caller's choosing.
		 * </p><p>
		 * </p><p>
		 * <strong>Thread Safety</strong>
		 * </p><p>
		 * This method makes a copy of the current State on the heap
		 * using that State's clone() method and returns a pointer
		 * to the copy. Provided that the State is implemented 
		 * properly (see class overview documentation), it is not
		 * possible for two calls to this method to obtain a
		 * pointer to the same copy of a State. This assures thread
		 * safety outside of this method and relieves the caller of
		 * any responsibility for synchronizing access to the 
		 * returned memory block.
		 * </p>
		 * @return a pointer to a unique copy of the current State 
		 *         object or NULL if the current State is undefined.
		 */
		virtual State * GetState();

		/**
		 * <p>
		 * Sets and enqueues the current state.
		 * </p><p>
		 * <strong>Memory Management</strong>
		 * </p><p>
		 * This class assumes responsibility for deallocating the
		 * memory addressed by the pointer passed to this method.
		 * </p><p>
		 * </p><p>
		 * <strong>Thread Safety</strong>
		 * </p><p>
		 * Callers <strong>must not modify or deallocate</strong>
		 * the memory allocated to the pointer passed into this
		 * method after calling this method. The pointer passed 
		 * into this method must be a pointer to memory on the 
		 * heap. Failure to adhere to these guidelines may defeat 
		 * the thread safety features of this class.
		 * </p><p>
		 * This class stores the pointer passed to this method
		 * and makes that pointer available via GetState(). This
		 * call also invokes the clone() method of State to make
		 * a distinct copy of the state for the queue.
		 * </p>
		 * @param newState the new state.
		 * @return if the new state was successfully enqueued; false
		 *         indicates the current state was set but could not
		 *         be enqueued (the state queue is full).
		 */
		virtual bool SetState(State * newState);

		/**
		 * <p>
		 * Sets the sole listener class. The OnStateChange() method
		 * of this class will be called each time the state is 
		 * updated using SetState(State *).
		 * </p>
		 * @param l a listener class.
		 */
		virtual void SetListener(ModelListener * l);

		/**
		 * Sets the stun server address or hostname.
		 * Copies the provided value.
		 * @param val stun server address or hostname.
		 */
		virtual void SetStunServer(const PString & val);

		/**
		 * Returns the stun server address or hostname.
		 * Makes a unique copy for the exclusive use of
		 * the caller. Caller is responsible for deleting
		 * the copy produced.
		 */
		virtual PString GetStunServer();

		/**
		 * Sets the stun server type description.
		 * Copies the provided value.
		 * @param val stun server type description.
		 */
		virtual void SetStunType(const PString & val);

		/**
		 * Returns the stun server type description.
		 * Makes a unique copy for the exclusive use of
		 * the caller. Caller is responsible for deleting
		 * the copy produced.
		 */
		virtual PString GetStunType();

		/**
		 * Sets the SIP server address or hostname.
		 * Copies the provided value.
		 * @param val SIP server address or hostname.
		 */
		virtual void SetServer(const PString & val);

		/**
		 * Returns the stun server type description.
		 * Makes a unique copy for the exclusive use of
		 * the caller. Caller is responsible for deleting
		 * the copy produced.
		 */
		virtual PString GetServer();

		/**
		 * Sets the user name.
		 * Copies the provided value.
		 * @param val user name.
		 */
		virtual void SetUserName(const PString & val);

		/**
		 * Returns the user name.
		 * Makes a unique copy for the exclusive use of
		 * the caller. Caller is responsible for deleting
		 * the copy produced.
		 */
		virtual PString GetUserName();

		/**
		 * Sets the call destination.
		 * Copies the provided value.
		 * @param val call destination.
		 */
		virtual void SetDestination(const PString & val);

		/**
		 * Returns the call destination.
		 * Makes a unique copy for the exclusive use of
		 * the caller. Caller is responsible for deleting
		 * the copy produced.
		 */
		virtual PString GetDestination();

	private:
		Action ** aqueue;			// array implementation of action queue
		int aqhead, aqtail;			// pointers into the array of actions
		int aqsize;					// size of the array implementing the aqueue
		State ** squeue;			// array implementation of state queue
		int sqhead, sqtail;			// pointers into the array of states
		int sqsize;					// size of the array implementing the squeue
		State * state;				// current state
		PSemaphore aMutex;			// mutex for action queue
		PSemaphore sMutex;			// mutex for state & state queue
		PSemaphore iMutex;			// mutex for misc info setter/getter methods
		ModelListener * listener;	// listener function
		PString stunServer;			// stun server address or hostname
		PString stunType;			// stun type
		PString server;				// SIP server
		PString user;				// username
		PString destination;		// active destination

		/**
		 * Explicitly disabled copy constructor.
		 * No implementation, by design.
		 */
		Model(const Model & source);

		/**
		 * Explicitly disabled assignment operator.
		 * No implementation, by design.
		 */
		Model & operator=(const Model & rhs);

};


#endif // _MODEL_H_