#pragma once

#include "logger/event.hpp"
#include "logger/file.hpp"
#include "ss.hpp"

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
#define ERRCF(msg) LOGERR STRINGIFY_MACRO(LOGGER_CLASS_NAME) "::" \
	<< __func__ << " " SLINE " " msg
#define LOGTF(msg) LOG(out) << this << "::" << __func__ << " " SLINE " " msg
#define LOGCF(msg) LOG(out) STRINGIFY_MACRO(LOGGER_CLASS_NAME) "::" \
	<< __func__ << " " SLINE " " msg
#ifdef UHTTPD_VERBOSE
#	define VLTF(msg) LOGTF(msg)
#	define VLCF(msg) LOGCF(msg)
#else
#	define VLTF(msg)
#	define VLCF(msg)
#endif
#ifdef _DEBUG
#	define DLTF(msg) LOGTF(msg)
#	define DLCF(msg) LOGCF(msg)
#else
#	define DLTF(msg)
#	define DLCF(msg)
#endif

#ifdef _DEBUG
#define THRTF(msg) \
	{	LOGTF("[THROW] " msg); \
		throw Logger::Exception(ss() << microsec_clock::local_time() \
								<< ' ' << getpid() << " " << this << "::" \
								<< __func__ << " " SLINE " " msg); }
#else
#define THRTF(msg) \
	throw Logger::Exception(ss() << microsec_clock::local_time() \
							<< ' ' << getpid() << " " << this << "::" \
							<< __func__ << " " SLINE " " msg)
#endif
#define CATCH_ERRTF \
	catch(const Logger::Exception &e) { ERRTF("exception:" << e); } \
	catch(const std::exception &e) { ERRTF("exception:" << e.what()); }	\
	catch(...) { ERRTF("unknown exception"); }
#define CATCH_ERRCF \
	catch(const Logger::Exception &e) { ERRCF("exception:" << e); } \
	catch(const std::exception &e) { ERRCF("exception:" << e.what()); }	\
	catch(...) { ERRCF("unknown exception"); }

struct LogErr {
	LogErr(const error_code &error) : error(error) {}
	const error_code &error;
};
inline std::ostream & operator << (std::ostream &out, const LogErr &e) {
	if(e.error) out << " error:" << e.error << '/' << e.error.message();
	return out;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
