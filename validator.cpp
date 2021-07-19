#include "validator.hpp"

namespace proxy
{
	void validator::load_config(std::string_view config)
	{
		// todo: config file format?
		(void)this;
	}

	validator::proxy_type validator::check_host(const host_type& host)
	{
		if (auto it = proxy_hosts_.find(host); it != proxy_hosts_.end())
		{
			return proxy_type::proxy;
		}
		else
		{
			if (it = black_hole_host_.find(host); it != black_hole_host_.end())
			{
				return proxy_type::black_hole;
			}
			return proxy_type::direct;
		}
	}
}
