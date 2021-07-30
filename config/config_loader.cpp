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
	void from_json(const nlohmann::json& j, local_config& data)
	{
		data.io_context_pool_size = j["io_context_pool_size"];
		if (const auto it = j.find("listen_ip"); it != j.end())
		{
			data.local_listen_address.ip = it.value();
		}
		data.local_listen_address.port = j["listen_port"];
		data.log_level                 = j["log_level"];
	}

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
										remote_config,
										type,
										forward_addresses
									)

	local_config load_local_config(const std::string_view config_path)
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

	remote_config load_remote_config(const std::string_view config_path)
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
