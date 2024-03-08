#pragma once

#include "stdlibs.hpp"

struct Config {
	tcp::endpoint listen_endpoint;
	std::string documents_root;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
