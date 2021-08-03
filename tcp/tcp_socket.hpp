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
		using value_type = unsigned char;
		template <size_type N>
		using stream_type = std::array<value_type, N>;

		constexpr static size_type buffer_size       = 1460; // MTU=MSS+TCP Header+IP Header. 1500=1460+20+20
		constexpr static size_type basic_sample_size = 16;

		using buffer_type = stream_type<buffer_size>;

		using address_type = common::address;

		explicit tcp_socket(io_context_type& io_context);

		void connect(const address_type& address);

		void connect(const endpoint_type& endpoint);

		bool close(boost::system::error_code& shutdown_ec, boost::system::error_code& close_ec);

		void close();

		template <typename Handler>
			requires requires(Handler&& h)
			{
				{
					h(std::declval<const boost::system::error_code&>(), std::declval<size_type>())
				} -> std::same_as<void>;
			}
		void async_read(Handler&& handler)
		{
			socket_.async_read_some(boost::asio::buffer(buffer_), std::forward<Handler>(handler));
		}

		size_type write(const buffer_type& buffer, size_type max_size_in_bytes, boost::system::error_code& error_code);

		[[nodiscard]] std::string sample_buffer(size_type size = basic_sample_size) const;

		[[nodiscard]] address_type get_remote_address() const;

		[[nodiscard]] std::string get_remote_address_string() const;

		[[nodiscard]] address_type get_local_address() const;

		[[nodiscard]] std::string get_local_address_string() const;

		[[nodiscard]] std::string get_session_id() const;

		socket_type socket_;
		buffer_type buffer_;

		static endpoint_type make_endpoint(const address_type& address);

		static address_type make_address(const endpoint_type& endpoint);
	};
}
