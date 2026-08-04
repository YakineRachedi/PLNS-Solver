#pragma once
/******************************************************************************
 * Timer : A basic chrono for performance. 
 *****************************************************************************/

#include <sys/time.h>

struct Timer {
	bool launched = false;
	timeval start_time;
	void start();
	unsigned int stop(const char *str);
};