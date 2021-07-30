#include "tcp_session_manager.hpp"

#include "tcp_session.hpp"
#include "../logger/logger.hpp"

namespace proxy::tcp
{
	tcp_session_manager::tcp_session_manager(
		const common::address::port_type listen_port,
		common::address_set              target_addresses,
		const size_type                  pool_size)
		: io_context_pool_(pool_size),
		target_addresses_(std::move(target_addresses)),
		acceptor_(
				io_context_pool_.get_io_context().operator*(),
				boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), listen_port)
				) { }

	void tcp_session_manager::run()
	{
		start_acceptor();

		log_info(std::format("TCP connection is working -> listen port: {}", acceptor_.local_endpoint().port()));

		io_context_pool_.run();
	}

	void tcp_session_manager::start_acceptor()
	{
		auto session = std::make_shared<tcp_session>(io_context_pool_.get_io_context().operator*());

		acceptor_.async_accept(
								session->get_client_socket().socket_,
								[this, session](const boost::system::error_code& error_code)
								{
									if (!error_code)
									{
										session->start(target_addresses_);
									}
									else
									{
										log_warning(std::format("unable to receive client message -> error: {}",
																error_code.message()));
									}

									start_acceptor();
								}
							);
	}
}
