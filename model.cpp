/*
 * model.cpp
 *
 * Implementation of Model superclass.
 *
 */

#include <ptlib.h>
#include "model.h"
#include "state.h"
#include "action.h"

// Default constructor
Model::Model() :
	aqueue(NULL),
	aqhead(0),
	aqtail(0),
	aqsize(QUEUE_SIZE),
	mqueue(NULL),
	mqhead(0),
	mqtail(0),
	mqsize(QUEUE_SIZE),
	state(NULL),
	aqMutex(1,1),
	mqMutex(1,1),
	sMutex(1,1)
{
	// Action queue allocation & initialization
	aqueue = new Action *[aqsize];
	for (int i = 0; i < aqsize; ++i) {
		aqueue[i] = NULL;
	}
	// Error message queue allocation & initialization
	mqueue = new PString *[mqsize];
	for (int i = 0; i < mqsize; ++i) {
		mqueue[i] = NULL;
	}
}

// Optional constructor
Model::Model(int queueSize) :
	aqueue(NULL),
	aqhead(0),
	aqtail(0),
	aqsize(queueSize),
	mqueue(NULL),
	mqhead(0),
	mqtail(0),
	mqsize(queueSize),
	state(NULL),
	aqMutex(1,1),
	mqMutex(1,1),
	sMutex(1,1)
{
	// Action queue allocation & initialization
	aqueue = new Action *[aqsize];
	for (int i = 0; i < aqsize; ++i) {
		aqueue[i] = NULL;
	}
	// Error message queue allocation & initialization
	mqueue = new PString *[mqsize];
	for (int i = 0; i < mqsize; ++i) {
		mqueue[i] = NULL;
	}
}

// Destructor.
Model::~Model() {
	delete state;
	state = NULL;
	delete [] aqueue;
	aqueue = NULL;
	delete [] mqueue;
	mqueue = NULL;
}

// Add an action to the queue.
bool Model::EnqueueAction(Action * action)
{
	// create result
	bool result = true;
	// enter mutex
	aqMutex.Wait();
	// enqueue
	if (aqhead == aqtail) {
		if (aqhead == NULL) {
			// the queue is empty
			aqueue[aqtail] = action;
			aqtail = (aqtail + 1) % aqsize;
		} else {
			// the queue is full
			result = false;
		}
	} else {
		// the queue is neither empty nor full
		aqueue[aqtail] = action;
		aqtail = (aqtail + 1) % aqsize;
	}
	// exit mutex
	aqMutex.Signal();
	return result;
}

// Remove an action from the queue
Action * Model::DequeueAction()
{
	// set up result
	Action * result = NULL;
	// enter mutex
	aqMutex.Wait();
	// dequeue
	if (aqhead != NULL) {
		result = aqueue[aqhead];
		aqueue[aqhead] = NULL;
		aqhead = (aqhead + 1) % aqsize;
	}
	// exit mutex
	aqMutex.Signal();
	return result;
}

// Add an error message to the queue.
bool Model::EnqueueErrMsg(PString * msg)
{
	// create result
	bool result = true;
	// enter mutex
	mqMutex.Wait();
	// enqueue
	if (mqhead == mqtail) {
		if (mqhead == NULL) {
			// the queue is empty
			mqueue[mqtail] = msg;
			mqtail = (mqtail + 1) % mqsize;
		} else {
			// the queue is full
			result = false;
		}
	} else {
		// the queue is neither empty nor full
		mqueue[mqtail] = msg;
		mqtail = (mqtail + 1) % mqsize;
	}
	// exit mutex
	mqMutex.Signal();
	return result;
}

// Remove an error message from the queue
PString * Model::DequeueErrMsg()
{
	// set up result
	PString * result = NULL;
	// enter mutex
	mqMutex.Wait();
	// dequeue
	if (mqhead != NULL) {
		result = mqueue[mqhead];
		mqueue[mqhead] = NULL;
		mqhead = (mqhead + 1) % mqsize;
	}
	// exit mutex
	mqMutex.Signal();
	return result;
}

// Return a pointer to a copy of the current state
State * Model::GetState()
{
	State * result = NULL;
	sMutex.Wait();
	if (state != NULL) {
		result = state->clone();
	}
	sMutex.Signal();
	return result;
}

// Set the current state.
void Model::SetState(State * newState)
{
	sMutex.Wait();
	state = newState;
	sMutex.Signal();
}
