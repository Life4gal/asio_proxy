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

		using address_type = common::address;

		udp_socket(
			io_context_type&    io_context,
			const address_type& listen_address
			)
			: socket_(
					io_context,
					make_endpoint(listen_address)
					),
			buffer_() {}

		[[nodiscard]] common::address get_remote_address() const
		{
			return make_address(remote_);
		}

		[[nodiscard]] std::string get_remote_address_string() const
		{
			return get_remote_address().to_string();
		}

		[[nodiscard]] common::address get_local_address() const
		{
			if (socket_.is_open())
			{
				try
				{
					return make_address(socket_.local_endpoint());
				}
				catch (const std::exception&) {}
			}

			return {};
		}

		[[nodiscard]] std::string get_local_address_string() const
		{
			return get_local_address().to_string();
		}

		socket_type   socket_;
		endpoint_type remote_;
		buffer_type   buffer_;

		static endpoint_type make_endpoint(const address_type& address)
		{
			return boost::asio::ip::udp::endpoint(
												boost::asio::ip::make_address(address.ip),
												address.port
												);
		}

		static address_type make_address(const endpoint_type& endpoint)
		{
			return {
				.ip = endpoint.address().to_string(),
				.port = endpoint.port()
			};
		}
	};
}
