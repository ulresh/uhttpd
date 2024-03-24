#pragma once

#include <string>
#include <boost/asio.hpp>
namespace ip = boost::asio::ip;
using boost::asio::ip::tcp;

struct Config {
	tcp::endpoint listen_endpoint;
	std::string documents_root, pid_file;
	struct Log {
		std::string err, out, access;
	};
	Log log;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
