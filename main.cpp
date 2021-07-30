#include "launcher/launcher.hpp"
#include "logger/logger.hpp"
#include <iostream>

int main(const int argc, char* argv[])
{
	// if (argc != 4 && argc != 3)
	// {
	// 	std::cerr << "invalid arguments\nusage: " << argv[0] <<
	// 		" ./where_is_local_config ./where_is_remote_config ./where_to_put_the_log_file(default: ./log)\n";
	// }
	//
	// if (argc == 4)
	// {
	// 	proxy::logger_manager::instance().set_all_log_directory(argv[3]);
	// }
	//
	// proxy::launcher::start(argv[1], argv[2]);

	proxy::launcher::start(".\\conf\\local.json", ".\\conf\\remote.json");

	return 0;
}
