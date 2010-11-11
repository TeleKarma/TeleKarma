/*
 *
 * model.h
 *
 * The TeleKarma NG model component.
 */
 
class Model{
	public:
		void EnqueueAction(Action*);
		Action * DequeueAction();
		State * GetState();
		void SetState(State *);
		PSemaphore * GetStateMutex();
	
	
	
	
	
	
	private:
		//ActionQueue actionQueue;
		PSemaphore actionMutex;
		State * state;
		PSemaphore * stateMutex;
		
};