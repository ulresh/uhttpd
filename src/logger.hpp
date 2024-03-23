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

#define STRINGIFY_MACRO2(x) #x
#define STRINGIFY_MACRO(x) STRINGIFY_MACRO2(x)
#define SLINE STRINGIFY_MACRO(__LINE__)

#define ERRTF(msg) LOGERR << this << "::" << __func__ << " " SLINE " " msg
#define LOGTF(msg) LOG(out) << this << "::" << __func__ << " " SLINE " " msg
#ifdef UHTTPD_VERBOSE
#	define VLTF(msg) LOGTF(msg)
#else
#	define VLTF(msg)
#endif
#ifdef _DEBUG
#	define DLTF(msg) LOGTF(msg)
#else
#	define DLTF(msg)
#endif

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */