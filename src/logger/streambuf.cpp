#include "streambuf.hpp"

Logger::Streambuf::int_type Logger::Streambuf::overflow(int_type __c) {
    const bool __testeof = traits_type::eq_int_type(__c, traits_type::eof());
    if(__builtin_expect(__testeof, false)) {
		pos_type s = pptr() - pbase();
		if(s) {
			text.size += s;
			text.last_block_size += s;
		}
		return traits_type::not_eof(__c);
	}
	const bool __testput = pptr() < epptr();
    if(!__testput) {
		text.size +=  pptr() - pbase();
		text.extend();
		Text::Block &last = *text.tail;
		setp(last.data(), last.data() + last.size());
	}
	*pptr() = traits_type::to_char_type(__c);
	pbump(1);
    return __c;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
