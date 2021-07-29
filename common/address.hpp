#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json_fwd.hpp>

namespace proxy::common
{
	struct address
	{
		using host_type = std::string;
		using port_type = unsigned short;

		[[nodiscard]] std::string to_string() const;

		[[nodiscard]] bool is_valid() const;

		static host_type make_ip(std::string_view ip);

		host_type ip;
		port_type port;
	};

	using forward_addresses = std::vector<address>;
	using local_listen_ports = std::vector<address::port_type>;

	struct address_set
	{
		forward_addresses tcp;
		forward_addresses udp;
	};

	void from_json(const nlohmann::json& j, address& data);
	void to_json(nlohmann::json& j, const address& data);
	void from_json(const nlohmann::json& j, address_set& data);
	void to_json(nlohmann::json& j, const address_set& data);
}
