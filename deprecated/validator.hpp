#pragma once

#include "def.hpp"
#include <set>
#include <string_view>

namespace proxy
{
	class validator
	{
	public:
		enum class proxy_type
		{
			direct,
			// direct connect
			proxy,
			// proxy connect
			black_hole // refuse connect
		};

		void load_config(std::string_view config);

		proxy_type check_host(const host_type& host);

	private:
		std::set<host_type> proxy_hosts_;
		std::set<host_type> black_hole_host_;
	};
}
