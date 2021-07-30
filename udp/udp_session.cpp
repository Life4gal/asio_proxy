#include "udp_session.hpp"

#include "../logger/logger.hpp"
#include "../utils/udp_flow_statistics.hpp"
#include "../utils/utils.hpp"
#include "udp_session_manager.hpp"
#include <format>

namespace proxy::udp
{
	udp_session::udp_session(
		io_context_type&          io_context,
		const address_type&       listen_address,
		common::forward_addresses target_addresses,
		udp_session_manager&      manager)
		: io_context_(io_context),
		heartbeat_timer_(io_context),
		target_socket_(io_context, listen_address),
		manager_(manager),
		target_addresses_(std::move(target_addresses)),
		closed_(false) { }

	void udp_session::send_to_target(const buffer_type& buffer, size_type size)
	{
		if (closed_)
		{
			return;
		}

		keep_alive();

		for (auto& target : target_addresses_)
		{
			try
			{
				auto written = target_socket_.socket_.send_to(
															boost::asio::buffer(buffer),
															socket_type::make_endpoint(target)
															);

				log_info(
						std::format(
									"client: {} -> target: {} -> size: {} -> data: {}",
									target_socket_.get_local_address_string(),
									target.to_string(),
									written,
									utils::bin_to_hex(reinterpret_cast<const char*>(buffer.data()))
									)
						);

				udp_flow_statistics::instance().add_packet(std::format("send_to_target_{}", target.to_string()));
			}
			catch (const std::exception& e)
			{
				log_info(
						std::format(
									"client: {} -> target: {} -> size: {} -> data: {} -> failed: {}",
									target_socket_.get_local_address_string(),
									target.to_string(),
									size,
									utils::bin_to_hex(reinterpret_cast<const char*>(buffer.data())),
									e.what()
									)
						);
			}
		}
	}

	void udp_session::async_receive_target()
	{
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;

		if (closed_)
		{
			return;
		}

		target_socket_.socket_.async_receive_from(
												buffer(target_socket_.buffer_),
												target_socket_.remote_,
												[this](const error_code_t& error_code, const size_type size)
												{
													if (closed_)
													{
														return;
													}

													if (!error_code)
													{
														log_info(
																std::format(
																			"client: {} -> target: {} -> size: {} -> data: {}",
																			target_socket_.get_local_address_string(),
																			target_socket_.get_remote_address_string(),
																			size,
																			utils::bin_to_hex(reinterpret_cast<const
																								char*>(target_socket_.
																										buffer_.data()))
																			)
																);

														udp_flow_statistics::instance().
															add_packet(std::format("receive_from_target_{}",
																					target_socket_.
																					get_remote_address_string()));

														manager_.send_to_client(target_socket_, size);
														async_receive_target();
													}
													else if (error_code != boost::asio::error::operation_aborted)
													{
														log_warning(
																	std::format(
																				"unable to receive client message -> client: {} -> target: {} -> error: {}",
																				target_socket_.
																				get_local_address_string(),
																				target_socket_.
																				get_remote_address_string(),
																				error_code.message()
																				)
																	);
													}
												}
												);
	}

	void udp_session::keep_alive()
	{
		using error_code_t = boost::system::error_code;

		try
		{
			heartbeat_timer_.expires_from_now(static_cast<boost::posix_time::seconds>(heartbeat_interval));
			heartbeat_timer_.async_wait(
										[this](const error_code_t& error_code)
										{
											if (!closed_ && !error_code)
											{
												log_warning(
															std::format(
																		"{} seconds did not receive any information, close the udp connection -> client: {}",
																		heartbeat_interval,
																		target_socket_.get_local_address_string()));
												close();
											}
										});
		}
		catch (const std::exception& e)
		{
			log_warning(
						std::format(
									"heartbeat error: {}",
									e.what()
									)
						);
		}
	}

	void udp_session::close()
	{
		if (!closed_)
		{
			closed_ = true;

			boost::system::error_code dummy;
			heartbeat_timer_.cancel(dummy);

			if (target_socket_.socket_.is_open())
			{
				target_socket_.socket_.shutdown(boost::asio::socket_base::shutdown_both, dummy);
				target_socket_.socket_.close(dummy);
			}

			manager_.close_session(target_socket_.get_local_address());
		}
	}
}
