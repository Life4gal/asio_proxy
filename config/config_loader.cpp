#include "config_loader.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <format>
#include "../logger/logger.hpp"

namespace
{
	bool do_file_validate(const std::string_view filename, std::ifstream& out)
	{
		if (filename.empty())
		{
			proxy::log_error("empty filename");
			return false;
		}

		out.open(filename);

		if (!out.is_open())
		{
			proxy::log_error(std::format("cannot open file: {}", filename));
			return false;
		}

		return true;
	}
}

namespace proxy::config
{
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
										local_config,
										io_context_pool_size,
										local_listen_ports);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
										remote_config,
										forward_addresses
									)

	local_config config_loader::load_local_config(const std::string_view config_path)
	{
		std::ifstream config;
		if (!do_file_validate(config_path, config))
		{
			return {};
		}

		nlohmann::json json;
		config >> json;
		auto ret  = json.get<local_config>();
		ret.ready = true;

		return ret;
	}

	remote_config config_loader::remote_local_config(const std::string_view config_path)
	{
		std::ifstream config;
		if (!do_file_validate(config_path, config))
		{
			return {};
		}

		nlohmann::json json;
		config >> json;
		auto ret  = json.get<remote_config>();
		ret.ready = true;

		return ret;
	}
}
