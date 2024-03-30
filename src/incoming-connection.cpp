#include "incoming-connection.hpp"
#include "server.hpp"
#include "logger.hpp"

IncomingConnection::IncomingConnection(Server &server)
	: server(server), socket(server.ios), timer(server.ios)
{
}

void IncomingConnection::close() {
	if(closing) return;
	closing = true;
	server.incomings.erase(anchor);
	timer.cancel();
	error_code ec;
	socket.close(ec);
	if(ec) { LOGTF("socket.close" << LogErr(ec)); }
}

void IncomingConnection::async_start() {
	auto h = shared_from_this();
	anchor = server.incomings.insert(server.incomings.end(), h);
	timer.expires_from_now(server.config.request_timeout);
	timer.async_wait([h,this](const error_code &error) -> void {
		if(closing || server.closing || error) return;
		LOGTF("request_timeout");
		close();
	});
	buffer.reset(new char[header_buffer_size()]);
	state = 0; header_size = 0;
	async_read_header();
}

void IncomingConnection::async_read_header() {
	socket.async_read_some(ai::buffer(buffer.get(), header_buffer_size()),
			boost::bind(&IncomingConnection::handle_read_header,
			this, shared_from_this(), ph::error, ph::bytes_transferred));
}

void IncomingConnection::handle_read_header(IncomingConnectionShp,
		const error_code& error, std::size_t bytes_transferred) {
	if(closing || server.closing) return;
	else if(error) { LOGTF( << LogErr(error)); close(); return; }
	if(!bytes_transferred) goto read_next_chunk;
	ERRTF("TODO");
 read_next_chunk:
	header_size += bytes_transferred;
	if(header_size >= server.config.max_request_header_size) {
		LOGTF("too big header size:" << header_size);
		close(); return; }
	async_read_header();
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
