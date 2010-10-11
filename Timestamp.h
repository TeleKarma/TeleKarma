/*
 *
 * Timestamp.h
 *
 * Provides timestamp utilities.
 *
 */

#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include <ptlib.h>


class Timestamp {

	public:

		/**
		 * Appends a timestamp in "_yyyy.mm.dd_hh.mm.ss" format
		 * to the argument.
		 */
		static void Timestamp::appendTo(PString & str);

};


#endif	// _TIMESTAMP_H_