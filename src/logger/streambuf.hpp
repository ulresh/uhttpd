#pragma once

#include "../stdlibs.hpp"
#include "text.hpp"

namespace Logger {
struct Streambuf : std::streambuf {
	explicit Streambuf(Text &text) : text(text) {}
	Streambuf(Streambuf&&) = delete;
	Streambuf(const Streambuf&) = delete;
	Streambuf & operator = (Streambuf&&) = delete;
	Streambuf & operator = (const Streambuf&) = delete;
	int_type overflow(int_type __c  = traits_type::eof()) override;
	int sync() override {
		overflow();
		setp(pptr(), epptr());
		return 0;
	}
	Text &text;
};
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
