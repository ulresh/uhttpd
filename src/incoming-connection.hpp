#pragma once

#include "stdlibs.hpp"

struct Server;

struct IncomingConnection :
	std::enable_shared_from_this<IncomingConnection> {

	struct Variable {
		std::string name, value;
	};

	IncomingConnection(Server &server);
	void close();
	void async_read();
	void async_read_header();
	static int header_buffer_size() { return 4096; }
	static int max_request_header_size() { return 128 * 1024; }
	static int max_request_body_size() { return 128 * 1024; }
	void handle_read_header(std::shared_ptr<IncomingConnection>,
							const error_code& error,
							std::size_t bytes_transferred);
	Server &server;
	tcp::socket socket;
	bool closing = false;
	std::unique_ptr<char[]> buffer;
	int state, up_state, header_size;
	std::string method, url, proto, connection;
	std::list<Variable> query, headers, post;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
