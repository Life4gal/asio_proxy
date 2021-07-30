#include "launcher.hpp"

#include "../config/config_loader.hpp"
#include "../tcp/tcp_session_manager.hpp"
#include "../udp/udp_session_manager.hpp"
#include "../logger/logger.hpp"

namespace proxy
{
	void launcher::start(const std::string_view local_config_path, const std::string_view remote_config_path)
	{
		const auto& [io_context_pool_size, local_listen_address, log_level, local_ready] =
			config::load_local_config(local_config_path);

		const auto& [type, forward_addresses, remote_ready] =
			config::load_remote_config(remote_config_path);

		if (!local_ready || !remote_ready)
		{
			throw std::runtime_error{"config error"};
		}

		logger_manager::instance().set_log_level(log_level);

		if (type == "tcp")
		{
			tcp::tcp_session_manager manager(local_listen_address,
											forward_addresses,
											io_context_pool_size);
			manager.run();
		}
		else if (type == "udp")
		{
			udp::udp_session_manager manager(local_listen_address,
											forward_addresses,
											io_context_pool_size);
			manager.run();
		}
	}
}
