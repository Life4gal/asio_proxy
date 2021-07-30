#pragma once

#include <string>

namespace proxy::logger
{
	enum class log_level
	{
		info = 0,
		warning = 1,
		error = 2
	};

	inline std::string log_level_to_string(const log_level level)
	{
		switch (level)
		{
			case log_level::info:
				return "info";
			case log_level::warning:
				return "warning";
			case log_level::error:
				return "error";
			default:
				return "unknown";
		}
	}
}
