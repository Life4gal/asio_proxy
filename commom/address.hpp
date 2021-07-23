#pragma once

#include <string>
#include <format>

namespace proxy
{
	struct address
	{
		using host_type = std::string;
		using port_type = unsigned short;

		[[nodiscard]] std::string to_string() const
		{
			return std::format("{}:{}", ip, port);
		}

		host_type ip;
		port_type port;
	};
}
