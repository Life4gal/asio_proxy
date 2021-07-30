#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include "../common/address.hpp"

namespace proxy::udp
{
	struct udp_socket
	{
		using socket_type = boost::asio::ip::udp::socket;
		using io_context_type = boost::asio::io_context;
		using endpoint_type = boost::asio::ip::udp::endpoint;
		using size_type = size_t;
		template <size_type N>
		using stream_type = std::array<unsigned char, N>;

		constexpr static size_type buffer_size = 4 * 1024;
		using buffer_type = stream_type<buffer_size>;

		udp_socket(
			io_context_type&                 io_context,
			const common::address::port_type listen_port
			)
			: socket_(
					io_context,
					boost::asio::ip::udp::endpoint(
													boost::asio::ip::make_address("127.0.0.1"),
													listen_port
												)
					),
			buffer_() {}

		[[nodiscard]] std::string get_remote_address() const
		{
			return std::format("{}:{}",
								remote_.address().to_string(),
								remote_.port());
		}

		[[nodiscard]] std::string get_local_address() const
		{
			if (socket_.is_open())
			{
				try
				{
					return std::format("{}:{}",
										socket_.local_endpoint().address().to_string(),
										socket_.local_endpoint().port());
				}
				catch (const std::exception&) {}
			}

			return {};
		}

		socket_type   socket_;
		endpoint_type remote_;
		buffer_type   buffer_;
	};
}
