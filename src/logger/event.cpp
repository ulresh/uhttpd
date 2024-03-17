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
		if(!text->empty()) {
			text->append_eol();
			file.post(text);
		}
	}
}

Logger::Event::operator bool () const { return file.disabled; }

Logger::DirectEvent::DirectEvent(File &file)
	: file(file), text(new Text), buffer(*text)
{
	init(&buffer);
}

Logger::DirectEvent::~DirectEvent() {
	if(!file.disabled) {
		flush();
		if(!text->empty()) {
			text->append_eol();
			file.write(text);
		}
	}
}

Logger::DirectEvent::operator bool () const { return file.disabled; }

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
