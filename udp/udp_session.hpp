#pragma once

#include <vector>
#include <boost/asio/deadline_timer.hpp>
#include "udp_socket.hpp"

namespace proxy::udp
{
	class udp_session_manager;

	class udp_session
	{
	public:
		using socket_type = udp_socket;

		using io_context_type = socket_type::io_context_type;
		using size_type = socket_type::size_type;
		using buffer_type = socket_type::buffer_type;
		using timer_type = boost::asio::deadline_timer;

		// using duration_type = task_timer<boost::posix_time::seconds>::timer_duration_type;
		// constexpr static duration_type statistics_interval = boost::posix_time::seconds{30 * 60};

		using duration_type = size_type;

		constexpr static duration_type heartbeat_interval = 8 * 60;

		explicit udp_session(
			io_context_type&           io_context,
			common::address::port_type listen_port,
			common::forward_addresses  target_addresses,
			udp_session_manager&       manager);

		void send_to_target(const buffer_type& buffer, size_type size);

		void async_receive_target();

	private:
		void keep_alive();
		void close();

		io_context_type& io_context_;
		timer_type       heartbeat_timer_;

		socket_type                target_socket_;
		common::address::port_type listen_port_;

		udp_session_manager&      manager_;
		common::forward_addresses target_addresses_;

		bool closed_;
	};
}
