#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <array>
#include "../common/address.hpp"

namespace proxy::tcp
{
	struct tcp_socket
	{
		using socket_type = boost::asio::ip::tcp::socket;
		using io_context_type = boost::asio::io_context;
		using endpoint_type = boost::asio::ip::tcp::endpoint;
		using size_type = size_t;
		template <size_type N>
		using stream_type = std::array<unsigned char, N>;

		constexpr static size_type buffer_size = 1460; // MTU=MSS+TCP Header+IP Header. 1500=1460+20+20
		using buffer_type = stream_type<buffer_size>;

		using address_type = common::address;

		explicit tcp_socket(
			io_context_type& io_context
			)
			: socket_(io_context), buffer_() { }

		void connect(const address_type& address)
		{
			socket_.connect(make_endpoint(address));
		}

		void connect(const endpoint_type& endpoint)
		{
			socket_.connect(endpoint);
		}

		[[nodiscard]] address_type get_remote_address() const
		{
			if (socket_.is_open())
			{
				try
				{
					return make_address(socket_.remote_endpoint());
				}
				catch (const std::exception&) { }
			}

			return {};
		}

		[[nodiscard]] std::string get_remote_address_string() const
		{
			return get_remote_address().to_string();
		}

		[[nodiscard]] address_type get_local_address() const
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

		[[nodiscard]] std::string get_session_id() const
		{
			return std::format("local: {} - remote: {}",
								get_local_address_string(),
								get_remote_address_string());
		}

		socket_type socket_;
		buffer_type buffer_;

		static endpoint_type make_endpoint(const address_type& address)
		{
			return boost::asio::ip::tcp::endpoint(
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
