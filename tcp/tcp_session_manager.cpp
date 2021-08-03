#include "tcp_session_manager.hpp"

#include "tcp_session.hpp"
#include "../logger/logger.hpp"

namespace proxy::tcp
{
	tcp_session_manager::tcp_session_manager(
		const common::address&    listen_address,
		common::forward_addresses target_addresses,
		const size_type           pool_size)
		: io_context_pool_(pool_size),
		target_addresses_(std::move(target_addresses)),
		acceptor_(
				io_context_pool_.get_io_context_handle().operator*(),
				tcp_session::socket_type::make_endpoint(listen_address)
				) { }

	void tcp_session_manager::run()
	{
		start_acceptor();

		log_info(std::format("TCP connection is working -> listening: {}",
							tcp_session::socket_type::make_address(acceptor_.local_endpoint()).to_string()));

		io_context_pool_.run();
	}

	void tcp_session_manager::start_acceptor()
	{
		auto session = std::make_shared<tcp_session>(io_context_pool_.get_io_context());

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
