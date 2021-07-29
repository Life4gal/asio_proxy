#include "pch.h"

#include "../logger/logger.hpp"
#include <format>

#include <regex>
#include <iostream>

TEST(TestLogger, LogInfo)
{
	//proxy::log_info(std::format("{} {}", "Hello", "World"));

	std::string ip{"aaa192.168.1.2bbb192.168"};

	std::regex regex{
		"(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
	};

	std::cmatch result;
	if (std::regex_search(ip.data(), result, regex))
	{
		for (const auto& s : result)
		{
			std::cout << s << '\n';
		}
	}
}
