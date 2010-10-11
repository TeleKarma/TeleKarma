/*
 *
 * Timestamp.cpp
 *
 * Provides timestamp utilities.
 *
 */


#include <time.h>
#include <cstdlib>

#include "Timestamp.h"


void Timestamp::appendTo(PString & str) {
	char * ts = new char[21];
	struct tm *tmp;
	tmp = NULL;
	time_t t;
	t = time(NULL);
	tmp = localtime(&t);
	// _yyyy.mm.dd_hh.mm.ss
	sprintf(ts, "_%04d.%02d.%02d_%02d.%02d.%02d",
		tmp->tm_year+1900, tmp->tm_mon+1, tmp->tm_mday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	str += ts;
	delete[] ts;
}