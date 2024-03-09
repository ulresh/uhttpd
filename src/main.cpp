#include "stdlibs.hpp"
#include "server.hpp"

int main(int argc, const char **argv) {
	cout << "https://github.com/ulresh/umake" << endl;
	if(argc != 2) {
		cerr << "Требуется указать имя конфигурационного файла"
			" в качестве параметра" << endl;
		return 1;
	}
	Server server;
	server.load_config(argv[1]);
	server.async_start();
	server.ios.run();
	return 0;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
