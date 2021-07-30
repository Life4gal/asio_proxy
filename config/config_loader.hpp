#pragma once

#include "../common/address.hpp"
#include "../common/io_context_pool.hpp"
#include "../logger/log_forward.hpp"

namespace proxy::config
{
	struct local_config
	{
		using io_context_pool_size_type = common::io_context_pool::size_type;
		io_context_pool_size_type io_context_pool_size{};

		common::address local_listen_address{"127.0.0.1"};

		std::underlying_type_t<logger::log_level> log_level{};

		bool ready = false;
	};

	struct remote_config
	{
		using forward_addresses_type = common::forward_addresses;

		std::string            type;
		forward_addresses_type forward_addresses{};

		bool ready = false;
	};

	void from_json(const nlohmann::json& j, local_config& data);
	void from_json(const nlohmann::json& j, remote_config& data);

	local_config  load_local_config(std::string_view config_path);
	remote_config load_remote_config(std::string_view config_path);
}
