#pragma once

#include "def.hpp"
#include <string_view>

namespace proxy
{
	class acceptor
	{
	public:
		acceptor(
			io_service_type& io_service,
			std::string_view local_host,
			port_type        local_port,
			std::string_view forward_host,
			port_type        forward_port);

		bool connect();

	private:
		void handle_connect(const boost::system::error_code& error);

		io_service_type& io_service_;

		const host_type local_host_;
		const port_type local_port_;

		const host_type forward_host_;
		const port_type forward_port_;

		acceptor_type acceptor_;
		bridge_type   session_;
	};
}
