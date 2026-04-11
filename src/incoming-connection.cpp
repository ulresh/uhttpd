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
	clear_state();
	async_read_header();
}

void IncomingConnection::async_read_header(int offset, int mark_offset) {
	socket.async_read_some(ai::buffer(buffer.get() + offset,
									  header_buffer_size() - offset),
			boost::bind(&IncomingConnection::handle_read_header,
						this, shared_from_this(), offset, mark_offset,
						ph::error, ph::bytes_transferred));
}

#define CHECK_HEADER_SIZE \
	header_size += bytes_transferred; \
	if(header_size >= server.config.max_request_header_size) { \
		LOGTF("too big header size:" << header_size); \
		close(); return; }

#define DEFAULT_STRING(s) \
	if(ptr == end1) { \
		CHECK_HEADER_SIZE; \
		parsed_string_append(s, mark, end); \
		return; \
	} \
	break

#define SPACE_CODE(s) \
	(s).append(mark, ptr - mark); \
	(s).append(1, ' '); \
	if(ptr == end1) goto read_next_chunk; \
	break

#define HEX_CODE(s) \
	(s).append(mark, ptr - mark); \
	up_state = state; state = 4; \
	if(ptr == end1) goto read_next_chunk; \
	++ptr; goto state_4

#define DECODED_CHAR(s,l) \
	(s).append(1, decoded_char); \
	if(ptr == end1) goto read_next_chunk; \
	mark = ++ptr; goto l

/* method /uri+uri%aburi?name=value&name2=value#fragment proto/1.1     \r \n
  0 1    2 3      4 5   6    7     6           8        9 10  11 12 13 14
   header: value
 15 16   17 18
     continued value
 19 17
   header: value
 19 16   17 18
     continued value
    17
   data
 20
 */

void IncomingConnection::handle_read_header(IncomingConnectionShp,
		int offset, int mark_offset,
		const error_code& error, std::size_t bytes_transferred) {
	if(closing || server.closing) return;
	else if(error) { LOGTF( << LogErr(error)); close(); return; }
	if(!bytes_transferred) goto read_next_chunk;
	{char *mark = buffer.get(), *ptr = mark + offset,
			*end = ptr + bytes_transferred, *end1 = end - 1;
	mark += mark_offset;
	for(; ptr < end; ++ptr)
	retry:
		switch(state) {
		default: VLTF("unknown state:" << state); close(); return;
		case 0:
			if(*ptr < 'A' || *ptr > 'Z') {
				VLTF("bad start char:" << LogChar(*ptr)); close(); return; }
			++state;
			if(ptr < end1) ++ptr; else {
				header_size += bytes_transferred;
				async_read_header(1);
				return;
			}
		case 1:
			while(*ptr >= 'A' && *ptr <= 'Z') {
				if(ptr < end1) ++ptr; else {
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
				VLTF("bad method char:" << LogChar(*ptr));
				close(); return; }
		case 2:
			while(*ptr == ' ')
				if(ptr < end1) ++ptr; else goto read_next_chunk;
			mark = ptr;
			path.emplace_back();
			++state;
		case 3:
			for(;; ++ptr)
			retry_path:
				switch(*ptr) {
				case '?':
				case '#':
				case ' ':
				case '\r':
				case '\n':
					if(ptr == mark) {
						if(path.back().empty() && path.size() == 1) {
							VLTF("empty path"); close(); return; }
					}
					else path.back().append(mark, ptr - mark);
					if(!path_finish()) return;
					switch(*ptr) {
					default:
						VLTF("bad algorithm char:" << LogChar(*ptr));
						close(); return;
					case '?':
						state = 6;
						if(ptr == end1) goto read_next_chunk;
						else { ++ptr; goto state_6; }
					case '#':
					case ' ':
					case '\r':
					case '\n':
						ERRTF("TODO"); close(); return;
					}
				case '/':
					path.back().append(mark, ptr - mark);
					if(!path_slice()) return;
					path.emplace_back();
					if(ptr == end1) goto read_next_chunk;
					mark = ++ptr;
					goto retry_path;
				case '+': SPACE_CODE(path.back());
				case '%': HEX_CODE(path.back());
				default: DEFAULT_STRING(path.back());
				}
		case 4:
		state_4:
#define BAD VLTF("bad char in hex code:" << LogChar(*ptr)); close(); return;
			{	auto c = *ptr;
				if(c < '0') {BAD}
				else if(c <= '9') decoded_char = (c - '0') << 4;
				else if(c < 'A') {BAD}
				else if(c <= 'F') decoded_char = (c - 'A' + 10) << 4;
				else if(c < 'a') {BAD}
				else if(c <= 'f') decoded_char = (c - 'a' + 10) << 4;
				else {BAD}
			}
			++state;
			if(ptr == end1) goto read_next_chunk;
			++ptr;
		case 5:
			{	auto c = *ptr;
				if(c < '0') {BAD}
				else if(c <= '9') decoded_char += c - '0';
				else if(c < 'A') {BAD}
				else if(c <= 'F') decoded_char += c - 'A' + 10;
				else if(c < 'a') {BAD}
				else if(c <= 'f') decoded_char += c - 'a' + 10;
				else {BAD}
			}
			state = up_state;
			switch(state) {
			default: VLTF("bad state:" << state); close(); return;
			case 3: DECODED_CHAR(path.back(), retry_path);
			}
#undef BAD
		case 6:
		state_6:
			ERRTF("TODO"); close(); return;
		}
	}
 read_next_chunk:
	CHECK_HEADER_SIZE;
	async_read_header();
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
