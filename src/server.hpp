#pragma once

#include "stdlibs.hpp"
#include "uhttpd/config.hpp"

struct Server {
	void load_config(const char *config_filename);
	Config config;
	io_context ios;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
