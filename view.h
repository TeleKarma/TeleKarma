
#ifndef _VIEW_H_
#define _VIEW_H_

class PString;
class TeleKarma;

class View {
	public:
		View(TeleKarma * controller) : controller(controller) { }
		virtual void Run() = 0;

	protected:
		void Register(const PString & registrar, const PString & user, const PString & password);
		void Dial(const PString & dest);
		void Hold();
		void AutoHold();
		void Retrieve();
		void Disconnect();
		void Quit();

		TeleKarma * controller;
};

#endif //_VIEW_H_
