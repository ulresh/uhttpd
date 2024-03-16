#pragma once

#include <iostream>
#include <map>
#include <forward_list>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/dll.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;
namespace bp = boost::process;
using boost::system::error_code;
namespace ai = boost::asio;
namespace ph = boost::asio::placeholders;
using boost::asio::io_context;
namespace ip = boost::asio::ip;
using boost::asio::ip::tcp;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::second_clock;
using boost::posix_time::microseconds;
using boost::posix_time::milliseconds;
using boost::posix_time::seconds;
using boost::posix_time::minutes;
using boost::posix_time::time_duration;
using boost::posix_time::from_time_t;

extern std::ofstream ulog;
typedef boost::iostreams::tee_device<std::ostream,std::ostream> TeeDevice;
extern boost::iostreams::stream<TeeDevice> uout;

inline ptime utc2local(ptime t) {
	return boost::date_time::c_local_adjustor<ptime>::utc_to_local(t); }
inline ptime utc2local(std::time_t t) { return utc2local(from_time_t(t)); }

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
