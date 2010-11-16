/*
 * controller.h
 *
 * Definition of Controller super class.
 *
 */

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

class Model;

class Controller{
	public:
	
	Controller(Model * model);
	virtual ~Controller();
	virtual void Main();
	
	private:
	
		virtual void ProcessNextEvent();
	/**
		 * Explicitely disabled copy constructor.
		 * No implementation, by design.
		 */
		Controller(const Controller & source);

		/**
		 * Explicitely disabled assignment operator.
		 * No implementation, by design.
		 */
		Controller & operator=(const Controller & rhs);
		
		Model * model;	
};

#endif // _CONTROLLER_H_
