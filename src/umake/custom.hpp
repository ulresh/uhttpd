#pragma once

#include <list>
#include <string>

struct Custom {
	void add_system_include_path(const std::string &s) {
		system_include_pathes.push_back(s); }
	void add_sysinc(const std::string &s) {
		system_include_pathes.push_back(s); }
	void add_include_path(const std::string &s) {
		include_pathes.push_back(s); }
	void add_library(const std::string &s) {
		libraries.push_back(s); }
	void add_library_file(const std::string &s) {
		library_files.push_back(s); }
	std::list<std::string> system_include_pathes,
		include_pathes, libraries, library_files;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
