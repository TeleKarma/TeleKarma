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
	squeue(NULL),
	sqhead(0),
	sqtail(0),
	sqsize(QUEUE_SIZE),
	state(NULL),
	aMutex(1,1),
	sMutex(1,1),
	iMutex(1,1),
	listener(NULL),
	stunServer("Unspecified"),
	stunType("Unknown"),
	server("Unspecified"),
	user("Unspecified"),
	destination("None")
{
	// Action queue allocation & initialization
	aqueue = new Action *[aqsize];
	for (int i = 0; i < aqsize; ++i) {
		aqueue[i] = NULL;
	}
	// state queue allocation & initialization
	squeue = new State *[sqsize];
	for (int i = 0; i < sqsize; ++i) {
		squeue[i] = NULL;
	}
}

// Optional constructor
Model::Model(int queueSize) :
	aqueue(NULL),
	aqhead(0),
	aqtail(0),
	aqsize(queueSize),
	squeue(NULL),
	sqhead(0),
	sqtail(0),
	sqsize(queueSize),
	state(NULL),
	aMutex(1,1),
	sMutex(1,1),
	iMutex(1,1),
	stunServer("Unspecified"),
	stunType("Unknown"),
	server("Unspecified"),
	user("Unspecified"),
	destination("None")
{
	// Action queue allocation & initialization
	aqueue = new Action *[aqsize];
	for (int i = 0; i < aqsize; ++i) {
		aqueue[i] = NULL;
	}
	// state queue allocation & initialization
	squeue = new State *[sqsize];
	for (int i = 0; i < sqsize; ++i) {
		squeue[i] = NULL;
	}
}

// Destructor.
Model::~Model() {
	delete state;
	state = NULL;
	for (int i = 0; i < aqsize; ++i) {
		delete aqueue[i];
		aqueue[i] = NULL;
	}
	delete [] aqueue;
	aqueue = NULL;
	for (int i = 0; i < sqsize; ++i) {
		delete squeue[i];
		squeue[i] = NULL;
	}
	delete [] squeue;
	squeue = NULL;
}

// Add an action to the queue.
bool Model::EnqueueAction(Action * action)
{
	// create result
	bool result = true;
	// enter mutex
	aMutex.Wait();
	// enqueue
	if (aqhead == aqtail) {
		if (aqueue[aqhead] == NULL) {
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
	aMutex.Signal();
	return result;
}

// Remove an action from the queue
Action * Model::DequeueAction()
{
	// set up result
	Action * result = NULL;
	// enter mutex
	aMutex.Wait();
	// dequeue
	if (aqueue[aqhead] != NULL) {
		result = aqueue[aqhead];
		aqueue[aqhead] = NULL;
		aqhead = (aqhead + 1) % aqsize;
	}
	// exit mutex
	aMutex.Signal();
	return result;
}

// Remove a state from the queue
State * Model::DequeueState()
{
	// set up result
	State * result = NULL;
	// enter mutex
	sMutex.Wait();
	// dequeue
	if (squeue[sqhead] != NULL) {
		result = squeue[sqhead];
		squeue[sqhead] = NULL;
		sqhead = (sqhead + 1) % sqsize;
	}
	// exit mutex
	sMutex.Signal();
	return result;
}

// Return a pointer to a copy of the current state
State * Model::GetState()
{
	State * result = NULL;
	sMutex.Wait();
	if (state != NULL) {
		result = state->Clone();
	}
	sMutex.Signal();
	return result;
}

// Set the current state.
bool Model::SetState(State * newState)
{
	// create result
	bool result = true;
	sMutex.Wait();
	State * oldState = state;
	state = newState;
	// enqueue
	State * scopy = newState->Clone();
	if (sqhead == sqtail) {
		if (squeue[sqhead] == NULL) {
			// the queue is empty
			squeue[sqtail] = scopy;
			sqtail = (sqtail + 1) % sqsize;
		} else {
			// the queue is full
			result = false;
		}
	} else {
		// the queue is neither empty nor full
		squeue[sqtail] = scopy;
		sqtail = (sqtail + 1) % sqsize;
	}
	delete oldState;
	sMutex.Signal();
	if (listener != NULL) {
		listener->OnStateChange();
	}
	return result;
}

void Model::SetListener(ModelListener * l)
{
	listener = l;
}

void Model::SetStunServer(const PString & val)
{
	iMutex.Wait();
	stunServer = val;
	iMutex.Signal();
}

PString Model::GetStunServer()
{
	iMutex.Wait();
	PString res(stunServer);
	iMutex.Signal();
	return res;
}

void Model::SetStunType(const PString & val)
{
	iMutex.Wait();
	stunType = val;
	iMutex.Signal();
}

PString Model::GetStunType()
{
	iMutex.Wait();
	PString res(stunType);
	iMutex.Signal();
	return res;
}

void Model::SetServer(const PString & val)
{
	iMutex.Wait();
	server = val;
	iMutex.Signal();
}

PString Model::GetServer()
{
	iMutex.Wait();
	PString res(server);
	iMutex.Signal();
	return res;
}

void Model::SetUserName(const PString & val)
{
	iMutex.Wait();
	user = val;
	iMutex.Signal();
}

PString Model::GetUserName()
{
	iMutex.Wait();
	PString res(user);
	iMutex.Signal();
	return res;
}

void Model::SetDestination(const PString & val)
{
	iMutex.Wait();
	destination = val;
	iMutex.Signal();
}

PString Model::GetDestination()
{
	iMutex.Wait();
	PString res(destination);
	iMutex.Signal();
	return res;
}