#pragma once

#include "logger/event.hpp"
#include "logger/file.hpp"

#define LOG(n) if(Logger::Event __logger_event = Logger::n) ; \
	else __logger_event << microsec_clock::local_time() \
						<< ' ' << getpid() << " "
#define LOG2(a,b) if(Logger::Event2 __logger_event = \
		std::pair<Logger::File&,Logger::File&>(Logger::a, Logger::b)) ; \
	else __logger_event << microsec_clock::local_time() \
						<< ' ' << getpid() << " "
#define LOGERR LOG2(err,out)

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
