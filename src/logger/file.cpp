#include "file.hpp"
#include "text.hpp"
#include "event.hpp"

namespace Logger { File err, out, access; }

void Logger::File::open(io_context &ios, const std::string &filepath) {
	iosp = &ios;
	stream.reset(new ai::stream_file(ios, filepath,
									 ai::stream_file::write_only |
									 ai::stream_file::create |
									 ai::stream_file::append));
	ready(TextShp(), error_code());
}

void Logger::File::post(TextShp text) {
	if(iosp) iosp->post(boost::bind(&File::write, this, text));
	else write(text); // предполагаем, что open для всех журналов вызывается до создания тредов, так что пока мы работаем только в одном треде.
}

void Logger::File::write(TextShp text) {
	if(busy) {
		if(skipped_events) {
			++skipped_events;
			skipped_size += text->size;
		}
		else if(text->size > end - ptr) {
			if(end - ptr > DirectEvent::max_size()) {
				if(DirectEvent e = *this) ;
				else e << microsec_clock::local_time() << ' ' << getpid()
					   << "[logger] too big message size:" << text->size;
			}
			else {
				skipped_start = microsec_clock::local_time();
				skipped_events = 1;
				skipped_size = text->size;
			}
		}
		else {
			text->write_to_memory(ptr);
			ptr += text->size;
		}
	}
	else {
		busy = true;
		Text::AsioVector a;
		text->init_asio_vector(a);
		ai::async_write(*stream, a,
						boost::bind(&File::ready, this, text, ph::error));
	}
}

void Logger::File::ready(TextShp text, const error_code& error) {
	if(error) {
		cerr << "Logger::File::ready error:" << error << '/'
			 << error.message() << endl;
	}
	if(ptr == buffers[current].data()) busy = false;
	else {
		auto start = buffers[current].data();
		ai::async_write(*stream, ai::buffer(start, ptr - start),
				boost::bind(&File::ready, this, TextShp(), ph::error));
		current = !current;
		ptr = buffers[current].data();
		end = ptr + buffers[current].size();
		if(int s = skipped_events) {
			skipped_events = 0;
			if(DirectEvent e = *this) ;
			else e << microsec_clock::local_time() << ' ' << getpid()
				   << "[logger] skip:" << s << '/' << skipped_size
				   << " from:" << skipped_start;
		}
	}
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
