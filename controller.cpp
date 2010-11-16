/*
 * controller.cpp
 *
 * Implementation of the controller super class.
 *
 */
 #include "telekarma.h"
 #include "controller.h"
 #include "model.h"
 #include "view.h"
 
 //Only constructor for the Controller class?
 Controller::Controller(View * view, Model * model) {
	 
 }
 
 Controller::~Controller() { 
	//Don't do anything in here
 }
 
 Controller::Main() {
	
	//main loop
	//Attemps to dequeue an action and then does the associated action
	while(true) { /*
		switch(model->DequeueAction()->id) {
			case NULL: 
			{
				//if there is nothing in the queue, just sleep for a bit
				Sleep(10);
			} //end NULL case
		}//end switch */

		Telekarma::ProcessNextEvent();
	}//end while
 }//end Main