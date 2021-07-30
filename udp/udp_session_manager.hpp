#pragma once

#include <mutex>
#include <unordered_map>
#include <memory>
#include "../common/io_context_pool.hpp"
#include "udp_socket.hpp"
#include "../common/address.hpp"

namespace proxy::udp
{
	class udp_session;

	class udp_session_manager
	{
	public:
		using socket_type = udp_socket;
		using udp_session_handle = std::shared_ptr<udp_session>;
		using size_type = common::io_context_pool::size_type;
		using port_type = common::address::port_type;

		udp_session_manager(
			port_type                 listen_port,
			common::forward_addresses target_addresses,
			size_type                 pool_size
			);

		void run();
		void send_to_client(udp_socket& target_socket, size_type size, port_type port);
		void close_session(port_type port);

	private:
		void               async_receive_client();
		udp_session_handle get_or_create_udp_session(port_type port);

		common::io_context_pool io_context_pool_;

		socket_type client_socket_;

		common::forward_addresses target_addresses_;

		std::mutex mutex_;

		std::unordered_map<port_type, udp_session_handle> udp_sessions_;
	};
}
