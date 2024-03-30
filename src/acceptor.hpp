#pragma once

#include "stdlibs.hpp"

struct Server;
struct IncomingConnection;

struct Acceptor : std::enable_shared_from_this<Acceptor> {
	Acceptor(Server &server, tcp::endpoint endpoint);
	void close();
	void async_accept();
	void handle_accept(std::shared_ptr<Acceptor> ah,
					   std::shared_ptr<IncomingConnection> ch,
					   const error_code &error);
	Server &server;
	tcp::acceptor acceptor;
	bool closing = false;
};

inline std::ostream & operator << (std::ostream &out, const Acceptor *a) {
	out << static_cast<const void *>(a);
	if(a) {
		error_code ec;
		out << "(loc:" << a->acceptor.local_endpoint(ec) << ')';
	}
	return out << " Acceptor";
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
