#pragma once

#include "../stdlibs.hpp"
#include "streambuf.hpp"

namespace Logger {
struct File;
struct Event : std::ostream {
	explicit Event(File &file);
	~Event();
	operator bool () const;
	// void send() { flush(); Event sender(file); std::swap(*this, sender); }
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
