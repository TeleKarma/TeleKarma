/*
 * controller.h
 *
 * Definition of Controller super class.
 *
 */

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

class Model;

class Controller : public PThread
{
	PCLASSINFO(Controller, PThread); //maybe subclasses need this
	public:
	
		Controller(Model * model);
		virtual ~Controller();
		virtual void Main()=0;
	
	private:
	
		/**
		 * Explicitly disabled copy constructor.
		 * No implementation, by design.
		 */
		Controller(const Controller & source);

		/**
		 * Explicitly disabled assignment operator.
		 * No implementation, by design.
		 */
		Controller & operator=(const Controller & rhs);
	
	protected:
		Model * model;	
};

#endif // _CONTROLLER_H_
