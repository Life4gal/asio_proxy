#pragma once

#include "../common/address.hpp"
#include "../common/io_context_pool.hpp"

namespace proxy::config
{
	struct local_config
	{
		using io_context_pool_size_type = common::io_context_pool::size_type;
		using local_listen_ports_type = common::local_listen_ports;

		io_context_pool_size_type io_context_pool_size{};
		local_listen_ports_type   local_listen_ports{};

		bool ready = false;
	};

	struct remote_config
	{
		using forward_addresses_type = common::address_set;

		forward_addresses_type forward_addresses{};

		bool ready = false;
	};

	void from_json(const nlohmann::json& j, local_config& data);
	void to_json(nlohmann::json& j, const local_config& data);
	void from_json(const nlohmann::json& j, remote_config& data);
	void to_json(nlohmann::json& j, const remote_config& data);

	class config_loader
	{
	public:
		static local_config  load_local_config(std::string_view config_path);
		static remote_config remote_local_config(std::string_view config_path);
	};
}
