#pragma once

#include <memory>
#include <vector>
#include <boost/asio/deadline_timer.hpp>
#include "tcp_socket.hpp"
#include "../common/address.hpp"

namespace proxy::tcp
{
	class tcp_session : public std::enable_shared_from_this<tcp_session>
	{
	public:
		using io_context_type = tcp_socket::io_context_type;
		using size_type = tcp_socket::size_type;
		using socket_type = tcp_socket;
		using socket_stream_type = std::vector<socket_type>;
		using timer_type = boost::asio::deadline_timer;

		// using duration_type = task_timer<boost::posix_time::seconds>::timer_duration_type;
		// constexpr static duration_type statistics_interval = boost::posix_time::seconds{30 * 60};

		using duration_type = size_type;

		constexpr static duration_type heartbeat_interval = 8 * 60;

		explicit tcp_session(io_context_type& io_context);

		socket_type& get_client_socket();

		void start(const common::address_set& target_addresses);

	private:
		void init_target_socket(const common::address_set& target_addresses);

		bool connect_target_server();

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

		socket_type        client_socket_;
		socket_stream_type target_socket_stream_;

		bool closed_;
	};
}