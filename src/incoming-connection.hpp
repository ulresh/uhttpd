#pragma once

#include "stdlibs.hpp"

struct IncomingConnection :
	std::enable_shared_from_this<IncomingConnection> {
	IncomingConnection(Server &server);
	void close();
	void async_start();
	void async_read_header();
	static int header_buffer_size() { return 4096; }
	void handle_read_header(IncomingConnectionShp,
							const error_code& error,
							std::size_t bytes_transferred);
	Server &server;
	tcp::socket socket;
	bool closing = false;
	IncomingConnectionList::iterator anchor;
	deadline_timer timer;
	std::unique_ptr<char[]> buffer;
	int state, up_state, header_size;
};

inline std::ostream & operator << (std::ostream &out,
								   const IncomingConnection *icon) {
	out << static_cast<const void *>(icon);
	if(icon) {
		if(icon->closing) out << "(closing)";
		else {
			error_code ec;
			out << "(loc:" << icon->socket.local_endpoint(ec)
				<< ",rem:" << icon->socket.remote_endpoint(ec) << ')';
		}
	}
	return out << " IncomingConnection";
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
