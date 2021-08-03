#include "udp_session.hpp"

#include "../logger/logger.hpp"
#include "../utils/udp_flow_statistics.hpp"
#include "udp_session_manager.hpp"
#include <format>

namespace proxy::udp
{
	udp_session::udp_session(
		io_context_type&       io_context,
		const address_type&    listen_address,
		forward_addresses_type target_addresses,
		udp_session_manager&   manager)
		: io_context_(io_context),
		heartbeat_timer_(io_context),
		target_socket_(io_context, listen_address),
		manager_(manager),
		target_addresses_(std::move(target_addresses)),
		closed_(false) { }

	void udp_session::send_to_target(const socket_type& client, const size_type size)
	{
		[[clang::no_destroy]] static std::string statistics_packet{"send_to_target_{}"};

		if (closed_)
		{
			return;
		}

		keep_alive();

		for (const auto& address : target_addresses_)
		{
			try
			{
				const auto written = target_socket_.send_to(client.buffer_, size, address);

				log_info(
						std::format(
									"client: [{}] -> target: [{}] -> size: [{}] -> sample data: [{}]",
									client.get_session_id(),
									target_socket_.get_session_id(),
									written,
									client.sample_buffer(written % socket_type::basic_sample_size)
									)
						);

				udp_flow_statistics::instance().add_packet(std::format(statistics_packet, address));
			}
			catch (const std::exception& e)
			{
				log_info(
						std::format(
									"client: [{}] -> target: [{}] -> size: [{}] -> data: [{}] -> failed: [{}]",
									client.get_session_id(),
									target_socket_.get_session_id(),
									size,
									client.sample_buffer(),
									e.what()
									)
						);
			}
		}
	}

	void udp_session::async_receive_target()
	{
		[[clang::no_destroy]] static std::string statistics_packet{"receive_from_target_{}"};

		if (closed_)
		{
			return;
		}

		target_socket_.async_receive([this](const boost::system::error_code& error_code, const size_type size)
		{
			if (closed_)
			{
				return;
			}

			if (!error_code)
			{
				log_info(
						std::format(
									"target: [{}] -> size: [{}] -> sample data: [{}]",
									target_socket_.get_session_id(),
									size,
									target_socket_.sample_buffer()
									)
						);

				udp_flow_statistics::instance().
					add_packet(std::format(statistics_packet,
											target_socket_.
											get_remote_address_string()));

				manager_.send_to_client(target_socket_, size);
				async_receive_target();
			}
			else if (error_code != boost::asio::error::operation_aborted)
			{
				log_warning(
							std::format(
										"unable to receive client message -> target: [{}] -> error: [{}]",
										target_socket_.get_session_id(),
										error_code.message()
										)
							);
			}
		});
	}

	void udp_session::keep_alive()
	{
		try
		{
			heartbeat_timer_.expires_from_now(static_cast<boost::posix_time::seconds>(heartbeat_interval));
			heartbeat_timer_.async_wait(
										[this](const boost::system::error_code& error_code)
										{
											if (!closed_ && !error_code)
											{
												log_warning(
															std::format(
																		"[{}] seconds did not receive any information, close the udp connection -> target: [{}]",
																		heartbeat_interval,
																		target_socket_.get_session_id()));
												close();
											}
										});
		}
		catch (const std::exception& e)
		{
			log_warning(
						std::format(
									"heartbeat error: [{}]",
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

			target_socket_.close();

			manager_.close_session(target_socket_.get_local_address());
		}
	}
}
