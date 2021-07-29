#include "launcher.hpp"

#include "../utils/config_loader.hpp"

namespace proxy
{
	void launcher::start() { }


	std::vector<common::address> launcher::read_forward_address()
	{
		std::vector<common::address> forward_address;

		for (auto&      config = utils::config::instance();
			const auto& [ip, port] : config.get_config())
		{
			auto real_ip = common::address::make_ip(ip);
			if (real_ip.empty())
			{
				continue;
			}


			common::address::port_type real_port{};
			utils::parse(port, real_port);

			forward_address.emplace_back(real_ip, real_port);
		}

		return forward_address;
	}
}
