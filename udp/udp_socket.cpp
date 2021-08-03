#include "udp_socket.hpp"

#include "../utils/utils.hpp"

namespace proxy::udp
{
	udp_socket::udp_socket(io_context_type& io_context, const address_type& listen_address)
		: socket_(
				io_context,
				make_endpoint(listen_address)
				),
		buffer_() { }

	bool udp_socket::close(boost::system::error_code& shutdown_ec, boost::system::error_code& close_ec)
	{
		if (!socket_.is_open())
		{
			return false;
		}

		socket_.shutdown(boost::asio::socket_base::shutdown_both, shutdown_ec);
		socket_.close(close_ec);
		return true;
	}

	void udp_socket::close()
	{
		boost::system::error_code shutdown_dummy;
		boost::system::error_code close_dummy;
		close(shutdown_dummy, close_dummy);
	}


	udp_socket::size_type udp_socket::send_to(const buffer_type& buffer, const size_type max_size_in_bytes,
											const address_type&  address)
	{
		return socket_.send_to(boost::asio::buffer(buffer, max_size_in_bytes), make_endpoint(address));
	}

	std::string udp_socket::sample_buffer(const size_type size) const
	{
		return utils::bin_to_hex({buffer_.data(), size});
	}


	udp_socket::address_type udp_socket::get_remote_address() const
	{
		return make_address(remote_);
	}

	std::string udp_socket::get_remote_address_string() const
	{
		return get_remote_address().to_string();
	}

	udp_socket::address_type udp_socket::get_local_address() const
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

	std::string udp_socket::get_local_address_string() const
	{
		return get_local_address().to_string();
	}

	std::string udp_socket::get_session_id() const
	{
		{
			return std::format("local: {} - remote: {}",
								get_local_address_string(),
								get_remote_address_string());
		}
	}

	udp_socket::endpoint_type udp_socket::make_endpoint(const address_type& address)
	{
		return boost::asio::ip::udp::endpoint(
											boost::asio::ip::make_address(address.ip),
											address.port
											);
	}

	udp_socket::address_type udp_socket::make_address(const endpoint_type& endpoint)
	{
		return {
			.ip = endpoint.address().to_string(),
			.port = endpoint.port()
		};
	}
}
