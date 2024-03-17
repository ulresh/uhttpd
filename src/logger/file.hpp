#pragma once

#include "../stdlibs.hpp"
#include "textshp.hpp"

namespace Logger {
struct File {
	File() {}
	void open(io_context &ios, const std::string &filepath);
	void post(TextShp text);
	io_context *iosp = nullptr;
	std::unique_ptr<ai::stream_file> stream;
	bool busy = true, disabled = false;
	ptime skipped_start;
	long skipped_events = 0, skipped_size = 0;
	std::array<char, 1024*1024> buffers[2];
	int current = 0;
	char *ptr = buffers[0].data();
	const char *end = buffers[0].data() + buffers[0].size();
};

extern File err, out, access;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
