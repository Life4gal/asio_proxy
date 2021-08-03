#include "tcp_socket.hpp"

#include "../utils/utils.hpp"
#include <boost/asio/write.hpp>

namespace proxy::tcp
{
	tcp_socket::tcp_socket(io_context_type& io_context)
		: socket_(io_context), buffer_() { }

	void tcp_socket::connect(const address_type& address)
	{
		socket_.connect(make_endpoint(address));
	}

	void tcp_socket::connect(const endpoint_type& endpoint)
	{
		socket_.connect(endpoint);
	}

	bool tcp_socket::close(boost::system::error_code& shutdown_ec, boost::system::error_code& close_ec)
	{
		if (!socket_.is_open())
		{
			return false;
		}

		socket_.shutdown(boost::asio::socket_base::shutdown_both, shutdown_ec);
		socket_.close(close_ec);
		return true;
	}

	void tcp_socket::close()
	{
		boost::system::error_code shutdown_dummy;
		boost::system::error_code close_dummy;
		close(shutdown_dummy, close_dummy);
	}

	tcp_socket::size_type tcp_socket::write(const buffer_type&         buffer, const size_type max_size_in_bytes,
											boost::system::error_code& error_code)
	{
		return boost::asio::write(socket_, boost::asio::buffer(buffer, max_size_in_bytes), error_code);
	}

	std::string tcp_socket::sample_buffer(const size_type size) const
	{
		return utils::bin_to_hex({buffer_.data(), size});
	}

	tcp_socket::address_type tcp_socket::get_remote_address() const
	{
		if (socket_.is_open())
		{
			try
			{
				return make_address(socket_.remote_endpoint());
			}
			catch (const std::exception&) {}
		}

		return {};
	}

	std::string tcp_socket::get_remote_address_string() const
	{
		return get_remote_address().to_string();
	}

	tcp_socket::address_type tcp_socket::get_local_address() const
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

	std::string tcp_socket::get_local_address_string() const
	{
		return get_local_address().to_string();
	}

	std::string tcp_socket::get_session_id() const
	{
		{
			return std::format("local: {} - remote: {}",
								get_local_address_string(),
								get_remote_address_string());
		}
	}

	tcp_socket::endpoint_type tcp_socket::make_endpoint(const address_type& address)
	{
		return boost::asio::ip::tcp::endpoint(
											boost::asio::ip::make_address(address.ip),
											address.port
											);
	}

	tcp_socket::address_type tcp_socket::make_address(const endpoint_type& endpoint)
	{
		return {
			.ip = endpoint.address().to_string(),
			.port = endpoint.port()
		};
	}
}
