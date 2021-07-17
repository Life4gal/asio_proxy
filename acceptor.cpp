#include "acceptor.hpp"

#include "bridge.hpp"
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <cassert>

namespace proxy
{
	using namespace boost::asio;

	acceptor::acceptor(
		io_service_type&       io_service,
		const std::string_view local_host,
		const port_type        local_port,
		const std::string_view forward_host,
		const port_type        forward_port)
		: io_service_(io_service),
		local_host_(local_host),
		local_port_(local_port),
		forward_host_(forward_host),
		forward_port_(forward_port),
		acceptor_(io_service_, ip::tcp::endpoint(ip::make_address(local_host_), local_port_))
	/*, session_(nullptr)*/ // todo: this will raise an std::bad_weak_ptr exception
	{}

	bool acceptor::connect()
	{
		try
		{
			session_ = make_bridge(io_service_);

			assert(session_ != nullptr);

			acceptor_.async_accept(
									session_->down_socket_,
									boost::bind(&acceptor::handle_connect,
												this,
												placeholders::error));
		}
		catch (const std::exception& e)
		{
			std::cerr << "acceptor exception: " << e.what() << '\n';
			return false;
		}

		return true;
	}

	void acceptor::handle_connect(const boost::system::error_code& error)
	{
		if (!error)
		{
			session_->run(forward_host_, forward_port_);

			if (!connect())
			{
				std::cerr << "failure during call to acceptor::connect\n";
			}
		}
		else
		{
			std::cerr << "Error: " << error.message() << '\n';
		}
	}
}
