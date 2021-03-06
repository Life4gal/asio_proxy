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
		using address_type = socket_type::address_type;
		using io_context_type = socket_type::io_context_type;
		using size_type = socket_type::size_type;
		using buffer_type = socket_type::buffer_type;
		using timer_type = boost::asio::deadline_timer;

		using forward_addresses_type = common::forward_addresses;

		// using duration_type = task_timer<boost::posix_time::seconds>::timer_duration_type;
		// constexpr static duration_type statistics_interval = boost::posix_time::seconds{30 * 60};

		using duration_type = size_type;

		constexpr static duration_type heartbeat_interval = 8 * 60;

		explicit udp_session(
			io_context_type&       io_context,
			const address_type&    listen_address,
			forward_addresses_type target_addresses,
			udp_session_manager&   manager);

		void send_to_target(const socket_type& client, size_type size);

		void async_receive_target();

	private:
		void keep_alive();
		void close();

		io_context_type& io_context_;
		timer_type       heartbeat_timer_;

		socket_type target_socket_;

		udp_session_manager&   manager_;
		forward_addresses_type target_addresses_;

		bool closed_;
	};
}
