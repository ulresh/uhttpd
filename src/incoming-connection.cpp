#include "incoming-connection.hpp"
#include "server.hpp"
#include "logger.hpp"

IncomingConnection::IncomingConnection(Server &server)
	: server(server), socket(server.ios)
{
}

void IncomingConnection::async_read() {
	THRTF("TODO");
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
