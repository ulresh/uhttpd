#include "incoming-connection.hpp"
#include "server.hpp"

IncomingConnection::IncomingConnection(Server &server)
	: server(server), socket(server.ios)
{
}

void IncomingConnection::close() {
	// TODO остановить таймер чтения, удалиться из списков
}

void IncomingConnection::async_read() {
	buffer.reset(new char[header_buffer_size()]);
	state = 0; header_size = 0;
	async_read_header();
}

void IncomingConnection::async_read_header() {
	socket.async_read_some(ai::buffer(buffer.get(), header_buffer_size()),
			boost::bind(&IncomingConnection::handle_read_header,
			this, shared_from_this(), ph::error, ph::bytes_transferred));
}

/* method /uri+uri%aburi?name=value&name2=value#anchor proto
  0 1    2 3      4     5
   header: value
     continued value

   data
 */

void IncomingConnection::handle_read_header(
		std::shared_ptr<IncomingConnection>,
		const error_code& error, std::size_t bytes_transferred) {
	if(closing || server.closing) return;
	else if(error) { close(); return; }
	if(!bytes_transferred) goto read_next_chunk;
	char *ptr = beffer.get(), *mark = ptr, *end = ptr + bytes_transferred,
		*end1 = end - 1;
	for(; ptr < end; ++ptr)
		switch(state) {
		default: close(); return;
		case 0:
			if(*ptr < 'A' || *ptr > 'Z') { close(); return; }
			++state;
			if(ptr < end1) ++ptr; else {
				method.append(mark, end - mark);
				goto read_next_chunk;
			}
		case 1:
			while(*ptr >= 'A' && *ptr <= 'Z') {
				if(++ptr == end) {
					method.append(mark, end - mark);
					goto read_next_chunk;
				}
			}
			if(*ptr == ' ') {
				++state;
				if(ptr < end1) ++ptr; else goto read_next_chunk;
			}
			else { close(); return; }
		case 2:
			while(*ptr == ' ') if(++ptr == end) goto read_next_chunk;
			mark = ptr;
			++state;
		case 3:
			switch(*ptr) {
			case ' ':
			case '?':
			case '#':
			case '\r':
			case '\n':
				if(ptr == mark) {
					if(url.empty()) { close(); return; }
				}
				else url.append(mark, ptr - mark);
			// EDIT POINT
			case '+':
			case '%':
			default:
				;
			}
		}
	switch(state) {
	default: close(); return;
	}
 read_next_chunk:
	header_size += bytes_transferred;
	if(header_size >= max_request_header_size()) { close(); return; }
	async_read_header();
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
