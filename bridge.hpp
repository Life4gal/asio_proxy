#pragma once

#include "def.hpp"
#include <memory>

namespace proxy
{
	class acceptor;

	class bridge : std::enable_shared_from_this<bridge>
	{
	public:
		friend class acceptor;

		constexpr static size_type max_data_length = 8192; // 8 KB

		explicit bridge(io_service_type& io_service);

		void run(const host_type& forward_host, port_type forward_port);

	private:
		void handle_up_connect(const boost::system::error_code& error);

		void close();

		void handle_up_write(const boost::system::error_code& error);

		void handle_up_read(const boost::system::error_code& error, size_type bytes_transferred);

		void handle_down_write(const boost::system::error_code& error);

		void handle_down_read(const boost::system::error_code& error, size_type bytes_transferred);

		socket_type up_socket_;
		socket_type down_socket_;

		stream_type<max_data_length> up_data_;
		stream_type<max_data_length> down_data_;

		std::mutex mutex_;
	};
}
