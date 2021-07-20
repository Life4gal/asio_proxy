#pragma once

#include <string>

#include "../utils/singleton.hpp"

namespace proxy
{
	namespace logger
	{
		enum class log_level
		{
			info,
			warning,
			error
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

	class logger_manager : utils::singleton<logger_manager>
	{
	public:
		using singleton<logger_manager>::instance;

	private:
	};
}
