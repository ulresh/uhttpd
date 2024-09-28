#pragma once

#include "stdlibs.hpp"

struct IncomingConnection :
	std::enable_shared_from_this<IncomingConnection> {

	struct VarValue {
		virtual void append(const char *ptr, std::size_t size) {}
		virtual void finish() {}
	};

	struct VarList {
		virtual VarValue *start(const std::string &name) {
			return new VarValue; }
	};

	struct Request {
		std::unique_ptr<VarList> query, headers, data;
	};

	struct VarStr {
		std::string name, value;
	};

	struct VarValueStrRef : VarValue {
		VarValueStrRef(std::string &value) : value(value) {}
		void append(const char *ptr, std::size_t size) override {
			value.append(ptr, size);
		}
		std::string &value;
	};

	struct VarStrMap : VarList {
		std::map<std::string, VarStr> map;
		VarValue *start(const std::string &name) override {
			auto key = name;
			boost::to_lower(key);
			auto &r = map[key];
			r.name = name;
			return new VarValueStrRef(r.value);
		}
	};

	struct RequestStrMap : Request {
		RequestStrMap() {
			query.reset(new VarStrMap);
			headers.reset(new VarStrMap);
			data.reset(new VarStrMap);
		}
	};

	IncomingConnection(Server &server);
	void close();
	void async_start();
	void clear_state() {
		state = 0;
		header_size = 0;
		http_version = 0;
		method.clear();
		proto.clear();
		path.clear();
		keep_alive = false;
	}
	void async_read_header(int offset = 0, int mark_offset = 0);
	static int header_buffer_size() { return 4096; }
	void handle_read_header(IncomingConnectionShp,
							int offset, int mark_offset,
							const error_code& error,
							std::size_t bytes_transferred);
	void parsed_string_append(std::string &s,
							  const char *mark, const char *end);
	bool path_slice() { return true; }
	bool path_finish() { request.reset(new RequestStrMap); return true; }
	Server &server;
	tcp::socket socket;
	bool closing = false;
	IncomingConnectionList::iterator anchor;
	deadline_timer timer;
	std::unique_ptr<char[]> buffer;
	int state, up_state, header_size, http_version;
	std::string method, proto;
	std::list<std::string> path;
	bool keep_alive;
	int decoded_char;
	std::unique_ptr<Request> request;
	std::string var_name;
	std::unique_ptr<VarValue> var_value;
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
