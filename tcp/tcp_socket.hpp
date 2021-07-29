#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <utility>
#include "../common/address.hpp"

namespace proxy::tcp
{
	struct tcp_socket
	{
		using socket_type = boost::asio::ip::tcp::socket;
		using io_context_type = boost::asio::io_context;
		using size_type = size_t;
		template <size_type N>
		using stream_type = std::array<unsigned char, N>;

		constexpr static size_type buffer_size = 1460; // MTU=MSS+TCP Header+IP Header. 1500=1460+20+20
		using buffer_type = stream_type<buffer_size>;

		tcp_socket(
			io_context_type& io_context,
			const int        serial,
			common::address  remote
			) : socket_(io_context), serial_(serial), remote_(std::move(remote)), buffer_() {}

		explicit tcp_socket(io_context_type& io_context)
			: tcp_socket(io_context, -1, {"", 0}) {}

		[[nodiscard]] std::string get_remote_address() const
		{
			if (socket_.is_open())
			{
				try
				{
					return std::format("{}:{}",
										socket_.remote_endpoint().address().to_string(),
										socket_.remote_endpoint().port());
				}
				catch (const std::exception&) { }
			}

			return {};
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

		[[nodiscard]] std::string get_session_id() const
		{
			return std::format("{}_{}", get_local_address(), get_remote_address());
		}

		socket_type     socket_;
		int             serial_;
		common::address remote_;
		buffer_type     buffer_;
	};
}
