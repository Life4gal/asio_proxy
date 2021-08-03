#pragma once

#include <memory>
#include <vector>
#include <boost/asio/deadline_timer.hpp>
#include "tcp_socket.hpp"

namespace proxy::tcp
{
	class tcp_session : public std::enable_shared_from_this<tcp_session>
	{
	public:
		using socket_type = tcp_socket;

		using io_context_type = socket_type::io_context_type;
		using size_type = socket_type::size_type;
		using sockets_type = std::vector<socket_type>;
		using timer_type = boost::asio::deadline_timer;

		using forward_addresses_type = common::forward_addresses;

		// using duration_type = task_timer<boost::posix_time::seconds>::timer_duration_type;
		// constexpr static duration_type statistics_interval = boost::posix_time::seconds{30 * 60};

		using duration_type = size_type;

		constexpr static duration_type heartbeat_interval = 8 * 60;

		explicit tcp_session(io_context_type& io_context);

		socket_type& get_client_socket();

		void start(const forward_addresses_type& target_addresses);

	private:
		bool connect_target_server(const forward_addresses_type& target_addresses);

		void async_read_client();
		void async_read_target();
		void async_read_target(socket_type& target);

		void send_to_client(socket_type& target, size_type size);
		void send_to_target(size_type size);

		void keep_alive();
		void close();
		void close_target_connection();
		void close_client_connection();

		io_context_type& io_context_;
		timer_type       heartbeat_timer_;

		socket_type  client_socket_;
		sockets_type target_sockets_;

		bool closed_;
	};
}
