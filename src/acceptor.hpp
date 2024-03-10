#pragma once

#include "stdlibs.hpp"

struct Server;
struct IncomingConnection;

struct Acceptor :
	std::enable_shared_from_this<IncomingConnection> {
	Acceptor(Server &server, tcp::endpoint endpoint);
	void async_accept();
	void handle_accept(std::shared_ptr<Acceptor> ah,
					   std::shared_ptr<IncomingConnection> ch,
					   const error_code &error);
	Server &server;
	tcp::acceptor acceptor;
	bool closing = false;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
