#include "sdkddkver.h"

#include "def.hpp"
#include "acceptor.hpp"
#include <iostream>
/*
int main(const int argc, const char* argv[])
{
	if (argc != 5)
	{
		std::cerr << "usage: " << argv[0] << " <local host ip> <local port> <forward host ip> <forward port>" <<
			std::endl;
		return -1;
	}

	const std::string_view local_host   = argv[1];
	const auto             local_port   = static_cast<proxy::port_type>(std::strtol(argv[2], nullptr, 10));
	const std::string_view forward_host = argv[3];
	const auto             forward_port = static_cast<proxy::port_type>(std::strtol(argv[4], nullptr, 10));

	std::cout << "forward " << local_host << ':' << local_port << " to " << forward_host << ':' << forward_port <<
		'\n';

	try
	{
		proxy::io_service_type io_service;
		proxy::acceptor        acceptor{io_service, local_host, local_port, forward_host, forward_port};

		if (!acceptor.connect())
		{
			std::cerr << "setup acceptor failed\n";
			return -1;
		}

		io_service.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
		return -1;
	}

	return 0;
}
*/

int main()
{
	std::cout << "hello";
}
