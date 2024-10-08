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

/* method /uri+uri%aburi?name=value&name2=value#anchor proto \r \n
  0 1    2 3      4 5   6
   header: value
     continued value

   data
 */

#define CHECK_HEADER_SIZE \
	header_size += bytes_transferred; \
	if(header_size >= server.config.max_request_header_size) { \
		LOGTF("too big header size:" << header_size); \
		close(); return; }

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
				case '+':
					path.back().append(mark, ptr - mark);
					path.back().append(1, ' ');
					if(ptr == end1) goto read_next_chunk;
					break;
				case '%':
					path.back().append(mark, ptr - mark);
					up_state = state; state = 4;
					if(ptr == end1) goto read_next_chunk;
					else { ++ptr; goto state_4; }
				default:
					if(ptr == end1) {
						CHECK_HEADER_SIZE;
						parsed_string_append(path.back(), mark, end);
						return;
					}
					break;
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
			case 3: path.back().append(1, decoded_char); break;
			}
			if(ptr == end1) goto read_next_chunk;
			else { ++ptr; goto retry; }
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

void IncomingConnection::parsed_string_append(std::string &s,
			const char *mark, const char *end) {
	if(auto size = end - mark) {
		auto start = buffer.get();
		if(end - start <= header_buffer_size() / 4 * 3) {
			async_read_header(end - start, mark - start);
			return;
		}
		else if(size > header_buffer_size() / 2 ||
				size <= s.capacity() - s.size()) {
			s.append(mark, size);
		}
		else {
			memcpy(start, mark, size);
			async_read_header(size);
			return;
		}
	}
	async_read_header();
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
