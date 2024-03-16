#include "event.hpp"
#include "file.hpp"

Logger::Event::Event(File &file)
	: file(file), text(new Text), buffer(*text)
{
	init(&buffer);
}

Logger::Event::~Event() {
	if(!file.disabled) {
		flush();
		file.post(text);
	}
}

Logger::Event::operator bool () const { return file.disabled; }

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
