#include "incoming-connection.hpp"
#include "server.hpp"

IncomingConnection::IncomingConnection(Server &server)
	: server(server), socket(server.ios)
{
}

void IncomingConnection::async_read() {
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
