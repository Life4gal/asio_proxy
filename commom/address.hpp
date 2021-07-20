#pragma once

#include <string>

namespace proxy
{
	struct address
	{
		using host_type = std::string;
		using port_type = unsigned short;

		[[nodiscard]] std::string to_string() const
		{
			return ip + ':' + std::to_string(port);
		}

		host_type ip;
		port_type port;
	};
}
