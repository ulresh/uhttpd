#include "event.hpp"
#include "file.hpp"
#include "text.hpp"

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




const char* Logger::Exception::what() const noexcept {
	if(auto tp = text.get()) {
		if(tp->empty()) return "uhttpd Exception text:empty";
		else if(tp->sequence_size == 1 &&
				tp->last_block_size < tp->tail->size()) {
			tp->tail->at(tp->last_block_size) = 0;
			return tp->tail->data();
		}
		else return "uhttpd Exception big text";
	}
	else return "uhttpd Exception text:0";
}



Logger::Event2::Event2(File &file1, File &file2)
	: file1(file1), file2(file2), text(new Text), buffer(*text)
{
	init(&buffer);
}

Logger::Event2::~Event2() {
	if(!file1.disabled || !file2.disabled) {
		flush();
		if(!text->empty()) {
			text->append_eol();
			if(!file1.disabled) file1.post(text);
			if(!file2.disabled) file2.post(text);
		}
	}
}

Logger::Event2::operator bool () const {
	return file1.disabled && file2.disabled; }



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
			text->reduce_to_one_buffer_with_eol(max_size());
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
