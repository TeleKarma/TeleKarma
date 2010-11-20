
#ifndef _SMS_H_
#define _SMS_H_

#include <ptlib.h>

class SMS {
	public:
		SMS(PString dest, PString message);
		~SMS();
		/**
		 * Send and SMS message.
		 * @return true if the message was sent
		 * @return false if sending the message failed
		 */
		bool Send();

	private:
		PString dest;
		PString message;
};

#endif //_SMS_H_
