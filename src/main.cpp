#include "stdlibs.hpp"
#include "server.hpp"
#include "logger.hpp"

#define LOGGER_CLASS_NAME M

int main(int argc, const char **argv) {
	LOGERR << "started https://github.com/ulresh/uhttpd";
	if(argc != 2) {
		cerr << "Требуется указать имя конфигурационного файла"
			" в качестве параметра" << endl;
		return 1;
	}
	Server server;
	server.load_config(argv[1]);
	server.async_start();
	for(;;) try { server.ios.run(); return 0; } CATCH_ERRCF
	return 0;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
