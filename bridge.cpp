#include "bridge.hpp"

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <mutex>

namespace proxy
{
	using namespace boost::asio;

	bridge_type make_bridge(io_service_type& io_service)
	{
		return std::make_shared<bridge>(io_service);
	}

	bridge::bridge(io_service_type& io_service)
		: up_socket_(io_service),
		down_socket_(io_service),
		up_data_(),
		down_data_() { }

	void bridge::run(const host_type& forward_host, port_type forward_port)
	{
		up_socket_.async_connect(
								ip::tcp::endpoint(ip::make_address(forward_host), forward_port),
								boost::bind(
											&bridge::handle_up_connect,
											shared_from_this(),
											placeholders::error)
								);
	}

	void bridge::handle_up_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			up_socket_.async_read_some(
										buffer(up_data_, max_data_length),
										boost::bind(&bridge::handle_up_read,
													shared_from_this(),
													placeholders::error,
													placeholders::bytes_transferred)
									);

			down_socket_.async_read_some(
										buffer(down_data_, max_data_length),
										boost::bind(&bridge::handle_down_read,
													shared_from_this(),
													placeholders::error,
													placeholders::bytes_transferred));
		}
		else
		{
			close();
		}
	}

	void bridge::close()
	{
		std::scoped_lock lock(mutex_);

		if (down_socket_.is_open())
		{
			down_socket_.close();
		}
		if (up_socket_.is_open())
		{
			up_socket_.close();
		}
	}

	void bridge::handle_up_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			down_socket_.async_read_some(
										buffer(down_data_, max_data_length),
										boost::bind(&bridge::handle_down_read,
													shared_from_this(),
													placeholders::error,
													placeholders::bytes_transferred));
		}
		else
		{
			close();
		}
	}

	void bridge::handle_up_read(const boost::system::error_code& error, size_type bytes_transferred)
	{
		if (!error)
		{
			async_write(
						down_socket_,
						buffer(up_data_, bytes_transferred),
						boost::bind(
									&bridge::handle_down_write,
									shared_from_this(),
									placeholders::error)
						);
			// down_socket_.async_write_some(
			// 							buffer(up_data_, bytes_transferred),
			// 							boost::bind(&bridge::handle_down_write,
			// 										shared_from_this(),
			// 										placeholders::error));
		}
		else
		{
			close();
		}
	}

	void bridge::handle_down_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			up_socket_.async_read_some(
										buffer(up_data_, max_data_length),
										boost::bind(&bridge::handle_up_read,
													shared_from_this(),
													placeholders::error,
													placeholders::bytes_transferred)
									);
		}
		else
		{
			close();
		}
	}

	void bridge::handle_down_read(const boost::system::error_code& error, size_type bytes_transferred)
	{
		if (!error)
		{
			async_write(up_socket_,
						buffer(down_data_, bytes_transferred),
						boost::bind(
									&bridge::handle_up_write,
									shared_from_this(),
									placeholders::error)
						);
			// up_socket_.async_write_some(
			// 							buffer(down_data_, bytes_transferred),
			// 							boost::bind(&bridge::handle_up_write,
			// 										shared_from_this(),
			// 										placeholders::error));
		}
		else
		{
			close();
		}
	}
}
