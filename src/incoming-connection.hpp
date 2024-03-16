#pragma once

#include "stdlibs.hpp"

struct Server;

struct IncomingConnection :
	std::enable_shared_from_this<IncomingConnection> {
	IncomingConnection(Server &server);
	void async_read();
	Server &server;
	tcp::socket socket;
	bool closing = false;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
