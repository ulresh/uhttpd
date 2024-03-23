#pragma once

#include "stdlibs.hpp"
#include "logger/textshp.hpp"
#include "logger/text.hpp"

struct ss : std::ostream {
	ss() : text(new Logger::Text), buffer(*text) { init(&buffer); }
	operator Logger::TextShp () { flush(); return text; }
	operator std::string () { flush(); return text->to_std_string(); }
	const Logger::TextShp text;
	Logger::Streambuf buffer;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
