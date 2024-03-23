#pragma once

#include "../stdlibs.hpp"

namespace Logger {
struct Text {
	typedef std::array<char, 128> Block;
	typedef std::forward_list<Block> List;
	typedef std::vector<ai::const_buffer> AsioVector;

	Text() : size(0), sequence_size(0), last_block_size(0),
			 tail(sequence.before_begin()) {}
	Text(Text &&rhs)
		: size(rhs.size), sequence_size(rhs.sequence_size),
		  last_block_size(rhs.last_block_size),
		  sequence(std::move(rhs.sequence)), tail(rhs.tail) {}
	Text(const Text &rhs)
		: size(rhs.size), sequence_size(rhs.sequence_size),
		  last_block_size(rhs.last_block_size),
		  sequence(rhs.sequence)
	{ init_tail(); }
	Text & operator = (Text &&rhs) {
		size = rhs.size;
		sequence_size = rhs.sequence_size;
		last_block_size = rhs.last_block_size;
		sequence.swap(rhs.sequence);
		tail = rhs.tail;
		return *this;
	}
	Text & operator = (const Text &rhs) {
		size = rhs.size;
		sequence_size = rhs.sequence_size;
		last_block_size = rhs.last_block_size;
		sequence = rhs.sequence;
		init_tail();
		return *this;
	}
	void init_tail() {
		for(auto prev = sequence.before_begin(), ptr = sequence.begin(),
				end = sequence.end(); ; prev = ptr++)
			if(ptr == end) { tail = prev; return; }
	}
	void extend() {
		tail = sequence.emplace_after(tail);
		++sequence_size;
		last_block_size = 0;
	}
	bool empty() const { return !size; }
	std::string to_std_string() const {
		std::string res;
		if(size) {
			res.reserve(size);
			for(auto ptr = sequence.begin(); ptr != tail; ++ptr)
				res.append(ptr->data(), ptr->size());
			res.append(tail->data(), last_block_size);
		}
		return res;
	}
	void write_to_stream(std::ostream &out) const {
		if(empty()) return;
		for(auto ptr = sequence.begin(); ptr != tail; ++ptr)
			out.write(ptr->data(), ptr->size());
		out.write(tail->data(), last_block_size);
	}
	void write_to_memory(char *mem) const {
		if(empty()) return;
		for(auto ptr = sequence.begin(); ptr != tail; ++ptr) {
			memcpy(mem, ptr->data(), ptr->size());
			mem += ptr->size();
		}
		memcpy(mem, tail->data(), last_block_size);
	}
	void init_asio_vector(AsioVector &a) const {
		if(empty()) return;
		a.reserve(sequence_size);
		for(auto ptr = sequence.begin(); ptr != tail; ++ptr)
			a.emplace_back(ptr->data(), ptr->size());
		a.emplace_back(tail->data(), last_block_size);
	}
	void append_eol() {
		if(empty()) {
			extend();
			tail->at(last_block_size++) = '\n';
			++size;
		}
		else if(last_block_size) {
			if(tail->at(last_block_size - 1) != '\n') {
				if(last_block_size == tail->size()) extend();
				tail->at(last_block_size++) = '\n';
				++size;
			}
		}
		else {
			bool found = false;
			for(auto ptr = sequence.begin(); ptr != tail; ++ptr)
				found = (ptr->at(ptr->size() - 1) == '\n');
			if(!found) {
				tail->at(last_block_size++) = '\n';
				++size;
			}
		}
	}
	void reduce_to_one_buffer_with_eol(int max_size) {
		if(size > max_size) {
			if(sequence_size > 1) {
				tail = sequence.begin();
				sequence.erase_after(tail, sequence.end());
				sequence_size = 1;
			}
			if(max_size < tail->size())
				size = last_block_size = max_size;
			else size = last_block_size = tail->size();
			tail->at(last_block_size - 1) = '\n';
		}
	}

	int size, sequence_size, last_block_size;
	// last_block_size == sequence_size ? size - (sequence_size - 1) * tuple_size<Block>::value : 0
	List sequence;
	List::iterator tail;
};
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
