#include "udp_session_manager.hpp"

#include "../logger/logger.hpp"
#include "../utils/udp_flow_statistics.hpp"
#include "../utils/utils.hpp"
#include "udp_session.hpp"

namespace proxy::udp
{
	udp_session_manager::udp_session_manager(
		const port_type           listen_port,
		common::forward_addresses target_addresses,
		const size_type           pool_size)
		: io_context_pool_(pool_size),
		client_socket_(io_context_pool_.get_io_context().operator*(), listen_port),
		target_addresses_(std::move(target_addresses)) { }

	void udp_session_manager::run()
	{
		async_receive_client();

		log_info(std::format("UDP connection is working -> listen: {}", client_socket_.get_local_address()));

		io_context_pool_.run();
	}

	void udp_session_manager::send_to_client(
		udp_socket& target_socket,
		size_type   size,
		port_type   port)
	{
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;

		try
		{
			const auto written = client_socket_.socket_.send_to(
																buffer(target_socket.buffer_),
																ip::udp::endpoint(
																				ip::make_address("127.0.0.1"),
																				port
																				)
																);

			log_info(
					std::format(
								"client port: {} -> target: {} -> size: {} -> data: {}",
								port,
								target_socket.get_remote_address(),
								written,
								utils::bin_to_hex(reinterpret_cast<const
													char*>(target_socket.
															buffer_.data()))
								)
					);

			udp_flow_statistics::instance().add_packet("send_to_client");
		}
		catch (const std::exception& e)
		{
			log_warning(
						std::format(
									"unable to write client message -> client port: {} -> target: {} -> error: {} -> size: {} -> data: {}",
									port,
									target_socket.get_remote_address(),
									e.what(),
									size,
									utils::bin_to_hex(reinterpret_cast<const
														char*>(client_socket_.buffer_.data()))
									)
						);
		}
	}

	void udp_session_manager::close_session(const port_type port)
	{
		std::scoped_lock lock(mutex_);

		udp_sessions_.erase(port);

		log_info(std::format("close port: {}, remainder: {}", port, udp_sessions_.size()));
	}

	void udp_session_manager::async_receive_client()
	{
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;

		client_socket_.socket_.async_receive_from(
												buffer(client_socket_.buffer_),
												client_socket_.remote_,
												[this](const error_code_t& error_code, size_type size)
												{
													if (!error_code)
													{
														const auto port = client_socket_.socket_.local_endpoint().
																						port();
														auto session = get_or_create_udp_session(port);

														log_info(
																std::format(
																			"client port: {} -> size: {} -> data: {}",
																			port,
																			size,
																			utils::bin_to_hex(reinterpret_cast<const
																								char*>(client_socket_.
																										buffer_.data()))
																			)
																);

														udp_flow_statistics::instance().
															add_packet("receive_from_client");

														session->send_to_target(client_socket_.buffer_, size);
														async_receive_client();
													}
													else if (error_code == error::operation_aborted)
													{
														log_warning(
																	std::format(
																				"unable to receive target message -> target: {} -> error: {}",
																				client_socket_.get_remote_address(),
																				error_code.message()
																				)
																	);
													}
												}
												);
	}

	udp_session_manager::udp_session_handle udp_session_manager::get_or_create_udp_session(port_type port)
	{
		std::scoped_lock lock(mutex_);

		udp_session_handle ret;

		if (const auto it = udp_sessions_.find(port);
			it == udp_sessions_.end())
		{
			ret = std::make_shared<udp_session>(io_context_pool_.get_io_context().operator*(),
												port,
												target_addresses_,
												*this);
			ret->async_receive_target();

			udp_sessions_.emplace(port, ret);
		}
		else
		{
			ret = it->second;
		}

		return ret;
	}
}
