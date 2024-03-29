#pragma once

#include "stdlibs.hpp"
#include "umake/custom.hpp"
#include "uhttpd/config.hpp"

struct Acceptor;

struct Server {
	Server();
	~Server();
	void close();
	void load_config(const char *config_filename);
	void async_start();
	void sighup_handler(const error_code &error);
	void sigterm_handler(const error_code &error);
	bp::filesystem::path cc;
	umake::Custom umake_custom;
	Config config;
	io_context ios;
	bool closing = false;
	ai::signal_set sighup, sigterm;
	std::shared_ptr<Acceptor> acceptor;
	IncomingConnectionList incomings;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
