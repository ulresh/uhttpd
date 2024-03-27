#include "server.hpp"
#include "acceptor.hpp"
#include "logger.hpp"
#include <sys/file.h>

#define LOGGER_CLASS_NAME Server

Server::Server()
	: sighup(ios, SIGHUP)
{}

Server::~Server() {
	if(!config.pid_file.empty()) {
		auto e = unlink(config.pid_file.c_str());
		if(e) cerr << "unlink pid_file:" << config.pid_file
				   << ErrNo() << endl;
	}
}

void Server::close() {
	closing = true;
	sighup.cancel();
}

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
		int result = bp::system(cc, "-x", "c++", "-shared", "-fPIC", inc,
								"-o", filename.string(), source);
		if(result != 0) {
			cerr << "Ошибка компиляции файла " << source << endl;
			exit(1);
		}
		boost::dll::shared_library lib(filename.string());
		boost::function<void(umake::Custom&)> load;
		try { load = lib.get<void(umake::Custom&)
							 >("_Z11build_umakeRN5umake6CustomE"); }
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
	ccargs.emplace_back("-fPIC");
	for(auto &&p : umake_custom.system_include_pathes) {
		ccargs.emplace_back("-isystem");
		ccargs.push_back(p);
	}
	for(auto &&p : umake_custom.include_pathes)
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
	try { load = lib.get<void(Config&)>("_Z13uhttpd_configR6Config"); }
	catch(const std::exception &e) {
		cerr << "exception: " << e.what() << endl; }
	if(!load) { cerr << "Не найдена функция uhttpd_config в файле "
					 << config_filename << endl; exit(1); }
	load(config);
	lib.unload();
	if(!config.pid_file.empty()) {
		int f = open(config.pid_file.c_str(), O_WRONLY | O_CREAT, 0666);
		if(f < 0) {
			cerr << "open pid_file:" << config.pid_file << ErrNo() << endl;
			exit(1);
		}
		int e = flock(f, LOCK_EX | LOCK_NB);
		if(e) {
			cerr << "flock pid_file:" << config.pid_file << ErrNo() << endl;
			exit(1);
		}
		e = ftruncate(f, 0);
		if(e) {
			cerr << "ftruncate pid_file:" << config.pid_file
				 << ErrNo() << endl;
			exit(1);
		}
		ss::write_to_file(f, ss() << getpid() << "\n");
	}
#define TUNE_LOG(n) \
	if(config.log.n.empty()) Logger::n.disabled = true; \
	else Logger::n.open(ios, config.log.n)
	TUNE_LOG(err); TUNE_LOG(out); TUNE_LOG(access);
#undef TUNE_LOG
	LOG(out) "listen:" << config.listen_endpoint;
}

void Server::async_start() {
	sighup.async_wait(boost::bind(&Server::sighup_handler, this, ph::error));
	Acceptor *p;
	acceptor.reset(p = new Acceptor(*this, config.listen_endpoint));
	p->async_accept();
}

void Server::sighup_handler(const error_code &error) {
	LOGCF( << LogErr(error));
	if(!error) {
#define TUNE_LOG(n) \
	if(!config.log.n.empty()) Logger::n.reopen(config.log.n)
		TUNE_LOG(err); TUNE_LOG(out); TUNE_LOG(access);
#undef TUNE_LOG
		ERRCF();
		sighup.async_wait(boost::bind(&Server::sighup_handler,
									  this, ph::error));
	}
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
