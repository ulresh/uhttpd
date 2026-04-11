#pragma once

#include <string>

struct Config {
	std::string ip;
	int port;
	std::string pid_file;
	struct Log {
		std::string err, out, access;
	};
	Log log;
	int request_timeout_seconds = 10;
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
