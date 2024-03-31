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

void IncomingConnection::async_read_header(int offset, int mark_offset) {
	socket.async_read_some(ai::buffer(buffer.get() + offset,
									  header_buffer_size() - offset),
			boost::bind(&IncomingConnection::handle_read_header,
						this, shared_from_this(), offset, mark_offset,
						ph::error, ph::bytes_transferred));
}

/* method /uri+uri%aburi?name=value&name2=value#anchor proto \r \n
  0 1    2 3      4     5
   header: value
     continued value

   data
 */

void IncomingConnection::handle_read_header(IncomingConnectionShp,
		int offset, int mark_offset,
		const error_code& error, std::size_t bytes_transferred) {
	if(closing || server.closing) return;
	else if(error) { LOGTF( << LogErr(error)); close(); return; }
	if(!bytes_transferred) goto read_next_chunk;
	char *mark = buffer.get(), *ptr = mark + offset,
		*end = ptr + bytes_transferred, *end1 = end - 1;
	mark += mark_offset;
	for(; ptr < end; ++ptr)
		switch(state) {
		default: VLTF("unknown state:" << state); close(); return;
		case 0:
			if(*ptr < 'A' || *ptr > 'Z') {
				VLTF("bad start char:" << int(*ptr)); close(); return; }
			++state;
			if(ptr < end1) ++ptr; else {
				header_size += bytes_transferred;
				async_read_header(1);
				return;
			}
		case 1:
			while(*ptr >= 'A' && *ptr <= 'Z') {
				if(++ptr == end) {
					header_size += bytes_transferred;
					if(end == buffer.get() + header_buffer_size()) {
						VLTF("too big method name");
						close(); return;
					}
					async_read_header(end - buffer.get());
					return;
				}
			}
			if(*ptr == ' ') {
				method.assign(mark, ptr - mark);
				++state;
				if(ptr < end1) ++ptr; else goto read_next_chunk;
			}
			else {
				VLTF("bad method char:" << int(*ptr));
				close(); return; }
		case 2:
			while(*ptr == ' ') if(++ptr == end) goto read_next_chunk;
			mark = ptr;
			url.emplace_back();
			++state;
		case 3:
			switch(*ptr) {
			case ' ':
			case '?':
			case '#':
			case '\r':
			case '\n':
				if(ptr == mark) {
					if(url.back().empty() && url.size() == 1) {
						VLTF("empty url"); close(); return; }
				}
				else url.back().append(mark, ptr - mark);
				// EDIT POINT
				ERRTF("TODO"); close(); return;
			case '/':
			case '+':
			case '%':
			default:
				ERRTF("TODO"); close(); return;
			}
		}
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
