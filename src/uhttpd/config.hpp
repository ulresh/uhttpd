#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
namespace ip = boost::asio::ip;
using boost::asio::ip::tcp;
using boost::posix_time::milliseconds;
using boost::posix_time::seconds;
using boost::posix_time::minutes;

struct Config {
	tcp::endpoint listen_endpoint;
	std::string documents_root, pid_file;
	struct Log {
		std::string err, out, access;
	};
	Log log;
	boost::posix_time::time_duration request_timeout = seconds(10);
	std::size_t max_request_header_size = 128 * 1024,
		max_request_body_size = 128 * 1024;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
