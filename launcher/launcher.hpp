#pragma once

#include <string_view>

namespace proxy
{
	class launcher
	{
	public:
		static void start(std::string_view local_config_path, std::string_view remote_config_path);
	};
}
