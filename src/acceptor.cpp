#include "acceptor.hpp"
#include "server.hpp"
#include "incoming-connection.hpp"
#include "logger.hpp"

Acceptor::Acceptor(Server &server, tcp::endpoint endpoint)
	: server(server), acceptor(server.ios, endpoint)
{
}

void Acceptor::close() {
	closing = true;
	acceptor.cancel();
}

void Acceptor::async_accept() {
	IncomingConnection *cp;
	std::shared_ptr<IncomingConnection>
		ch(cp = new IncomingConnection(server));
	acceptor.async_accept(cp->socket, boost::bind(&Acceptor::handle_accept,
				this, shared_from_this(), ch, ph::error));
}

void Acceptor::handle_accept(std::shared_ptr<Acceptor> ah,
							 std::shared_ptr<IncomingConnection> ch,
							 const error_code &error) {
	if(closing || server.closing || error) return;
	async_accept();
	error_code ec;
	VLTF("remote:" << ch->socket.remote_endpoint(ec));
	ch->async_read();
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
