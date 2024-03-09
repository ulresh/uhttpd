#include "server.hpp"

void Server::load_config(const char *config_filename) {
	cc = bp::search_path("g++");
	fs::path filename = config_filename;
	filename.replace_filename("build.umake");
	if(fs::exists(filename)) {
		auto source = filename.string();
		filename.replace_extension(".umake.so");
		std::string inc;
		{	auto env = boost::this_process::environment();
			auto inc_env = env["UMAKE_CUSTOM_INCLUDE_PATH"];
			if(inc_env.empty()) {
#ifdef UMAKE_DEFAULT_CUSTOM_INCLUDE_PATH
#define SM2(x) #x
#define SM(x) SM2(x)
				inc = "-I" SM(UMAKE_DEFAULT_CUSTOM_INCLUDE_PATH);
#else
				inc = "-Isrc";
#endif
			}
			else {
				inc = "-I";
				inc.append(inc_env.to_string());
				cout << "inc:" << inc << endl;
			}
		}
		int result = bp::system(cc, "-x", "c++", "-shared", inc,
								"-o", filename.string(), source);
		if(result != 0) {
			cerr << "Ошибка компиляции файла " << source << endl;
			exit(1);
		}
		boost::dll::shared_library lib(filename.string());
		boost::function<void(umake::Custom&)> load;
		try { load = lib.get<void(umake::Custom&)
							 >("_Z11build_umakeR6Custom"); }
		catch(const std::exception &e) {
			cerr << "exception: " << e.what() << endl; }
		if(!load) { cerr << "Не найдена функция build_umake в файле "
						 << source << endl; exit(1); }
		load(umake_custom);
		lib.unload();
	}
	filename = config_filename;
	filename.replace_extension(".config.so");
	std::list<std::string> ccargs;
	ccargs.emplace_back("-x");
	ccargs.emplace_back("c++");
	ccargs.emplace_back("-shared");
	for(auto &&p : custom.system_include_pathes) {
		ccargs.emplace_back("-isystem");
		ccargs.push_back(p);
	}
	for(auto &&p : custom.include_pathes)
		ccargs.push_back(std::string("-I")+p);
	ccargs.emplace_back("-o");
	ccargs.push_back(filename.string());
	ccargs.emplace_back(config_filename);
	int result = bp::system(bp::exe=cc, bp::args=ccargs);
	if(result != 0) {
		cerr << "Ошибка компиляции файла " << config_filename << endl;
		exit(1);
	}
	boost::dll::shared_library lib(filename.string());
	boost::function<void(Config&)> load;
	try { load = lib.get<void(Config&)
							 >("uhttpd_config"); }
	catch(const std::exception &e) {
		cerr << "exception: " << e.what() << endl; }
	if(!load) { cerr << "Не найдена функция uhttpd_config в файле "
					 << config_filename << endl; exit(1); }
	load(config);
	lib.unload();
	cout << "listen:" << config.listen_endpoint << endl;
}

void Server::async_start() {
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
