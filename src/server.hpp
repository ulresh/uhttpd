#pragma once

#include "stdlibs.hpp"
#include "umake/custom.hpp"
#include "uhttpd/config.hpp"

struct Server {
	void load_config(const char *config_filename);
	void async_start();
	bp::filesystem::path cc;
	umake::Custom umake_custom;
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
