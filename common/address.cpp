#include "address.hpp"

#include <format>
#include <regex>

#include <nlohmann/json.hpp>

namespace proxy::common
{
	std::string address::to_string() const
	{
		return std::format("{}:{}", ip, port);
	}

	bool address::is_valid() const
	{
		return !ip.empty();
	}

	address::host_type address::make_ip(const std::string_view ip)
	{
		const std::regex regex{
			"(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
		};

		if (std::cmatch result; std::regex_search(ip.data(), result, regex))
		{
			return result[0];
		}
		return "";
	}

	void from_json(const nlohmann::json& j, address& data)
	{
		data = {.ip = address::make_ip(j["ip"]), .port = j["port"]};
	}

	void to_json(nlohmann::json& j, const address& data)
	{
		j["ip"]   = data.ip;
		j["port"] = data.port;
	}

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
										address_set,
										tcp,
										udp)
}
