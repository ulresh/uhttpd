#pragma once

#include "../stdlibs.hpp"
#include "streambuf.hpp"
#include "textshp.hpp"

namespace Logger {
struct File;

struct Event : std::ostream {
	Event(File &file);
	~Event();
	operator bool () const;
	// void send() { flush(); Event sender(file); std::swap(*this, sender); }
	File &file;
	const TextShp text;
	Streambuf buffer;
};

struct Event2 : std::ostream {
	Event2(File &file1, File &file2);
	Event2(std::pair<File&, File&> p) : Event2(p.first, p.second) {}
	~Event2();
	operator bool () const;
	File &file1, &file2;
	const TextShp text;
	Streambuf buffer;
};

struct DirectEvent : std::ostream {
	DirectEvent(File &file);
	~DirectEvent();
	static int max_size() { return 256; }
	operator bool () const;
	File &file;
	const TextShp text;
	Streambuf buffer;
};
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
