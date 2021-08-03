#include "tcp_session.hpp"

#include "../logger/logger.hpp"
#include "../utils/tcp_flow_statistics.hpp"
#include <format>

namespace proxy::tcp
{
	tcp_session::tcp_session(io_context_type& io_context)
		: io_context_(io_context),
		heartbeat_timer_(io_context),
		client_socket_(io_context),
		closed_(false) { }

	tcp_session::socket_type& tcp_session::get_client_socket()
	{
		return client_socket_;
	}

	void tcp_session::start(const forward_addresses& target_addresses)
	{
		log_info(std::format("client connection received: [{}]", client_socket_.get_session_id()));

		tcp_flow_statistics::instance().add_connection();

		keep_alive();
		if (connect_target_server(target_addresses))
		{
			async_read_client();
			async_read_target();
		}
	}

	bool tcp_session::connect_target_server(const forward_addresses& target_addresses)
	{
		for (const auto& address : target_addresses)
		{
			auto& target = target_sockets_.emplace_back(io_context_);

			try
			{
				target.connect(address);

				log_info(
						std::format(
									"successfully connected to target -> client: [{}] -> target: [{}]",
									client_socket_.get_session_id(),
									target.get_session_id()
									)
						);
			}
			catch (const std::exception& e)
			{
				log_warning(
							std::format(
										"connected to target -> client: [{}] -> target: [{}] -> failed: [{}]",
										client_socket_.get_session_id(),
										target.get_session_id(),
										e.what()
										)
							);
				close();
				return false;
			}
		}

		return true;
	}

	void tcp_session::async_read_client()
	{
		[[clang::no_destroy]] static std::string statistics_packet{"receive_from_client"};

		client_socket_.async_read(
								[this](const boost::system::error_code& error_code, const size_type size)
								{
									if (closed_)
									{
										return;
									}

									if (!error_code)
									{
										keep_alive();

										log_info(
												std::format(
															"client: [{}] -> size: [{}] -> sample data: [{}]",
															client_socket_.get_session_id(),
															size,
															client_socket_.sample_buffer()
															)
												);

										tcp_flow_statistics::instance().add_packet(statistics_packet);

										send_to_target(size);
										async_read_client();
									}
									else if (error_code != boost::asio::error::operation_aborted)
									{
										log_warning(
													std::format(
																"unable to receive client message, close tcp connect -> client: [{}] -> error: [{}]",
																client_socket_.get_session_id(),
																error_code.message()
																)
													);
										close();
									}
								});
	}

	void tcp_session::async_read_target()
	{
		for (auto& target : target_sockets_)
		{
			async_read_target(target);
		}
	}

	void tcp_session::async_read_target(socket_type& target)
	{
		[[clang::no_destroy]] static std::string statistics_packet{"receive_from_target_{}"};

		target.async_read(
						[this, &target](const boost::system::error_code& error_code, const size_type size)
						{
							if (closed_)
							{
								return;
							}

							if (!error_code)
							{
								log_info(
										std::format(
													"client: [{}] -> target: [{}] -> size: [{}] -> sample data: [{}]",
													client_socket_.get_session_id(),
													target.get_session_id(),
													size,
													client_socket_.sample_buffer())
										);

								tcp_flow_statistics::instance().
									add_packet(std::format(statistics_packet,
															target.get_remote_address_string()));

								send_to_client(target, size);
								async_read_target(target);
							}
							else if (error_code != boost::asio::error::operation_aborted)
							{
								log_warning(
											std::format(
														"unable to receive target message -> client: [{}] -> target: [{}] -> error: [{}]",
														client_socket_.get_session_id(),
														target.get_session_id(),
														error_code.message()
														)
											);
								close();
							}
						});
	}

	void tcp_session::send_to_client(socket_type& target, const size_type size)
	{
		[[clang::no_destroy]] static std::string statistics_packet{"send_to_client"};

		boost::system::error_code error_code;
		const auto                written = client_socket_.write(target.buffer_, size, error_code);

		if (!error_code)
		{
			log_info(
					std::format(
								"client: [{}] -> target: [{}] -> size: [{}] -> sample data: [{}]",
								client_socket_.get_session_id(),
								target.get_session_id(),
								written,
								client_socket_.sample_buffer(written % socket_type::basic_sample_size)
								)
					);

			tcp_flow_statistics::instance().
				add_packet(statistics_packet);
		}
		else
		{
			log_warning(
						std::format(
									"unable to write client message -> client: [{}] -> target: [{}] -> error: [{}] -> size: [{}] -> sample data: [{}]",
									client_socket_.get_session_id(),
									target.get_session_id(),
									error_code.message(),
									size,
									client_socket_.sample_buffer()
									)
						);
		}
	}

	void tcp_session::send_to_target(const size_type size)
	{
		[[clang::no_destroy]] static std::string statistics_packet{"send_to_target_{}"};

		for (auto& target : target_sockets_)
		{
			boost::system::error_code error_code;
			const auto                written = target.write(client_socket_.buffer_, size, error_code);

			if (!error_code)
			{
				log_info(
						std::format(
									"client: [{}] -> target: [{}] -> size: [{}] -> sample data: [{}]",
									client_socket_.get_session_id(),
									target.get_session_id(),
									written,
									client_socket_.sample_buffer(written % socket_type::basic_sample_size)
									)
						);

				tcp_flow_statistics::instance().
					add_packet(std::format(statistics_packet, target.get_remote_address_string()));
			}
			else
			{
				log_warning(
							std::format(
										"unable to write target message -> client: [{}] target: [{}] -> error: [{}] -> size: [{}] -> sample data: [{}]",
										client_socket_.get_session_id(),
										target.get_session_id(),
										error_code.message(),
										size,
										client_socket_.sample_buffer()
										)
							);
			}
		}
	}

	void tcp_session::keep_alive()
	{
		try
		{
			auto dummy = shared_from_this();

			heartbeat_timer_.expires_from_now(static_cast<boost::posix_time::seconds>(heartbeat_interval));
			heartbeat_timer_.async_wait(
										[this, dummy](const boost::system::error_code& error_code)
										{
											if (!closed_ && !error_code)
											{
												log_warning(
															std::format(
																		"[{}] seconds did not receive any information, close the tcp connection -> client: [{}]",
																		heartbeat_interval,
																		client_socket_.get_session_id()));
												close();
											}

											(void)dummy;
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

	void tcp_session::close()
	{
		if (!closed_)
		{
			closed_ = true;

			boost::system::error_code dummy;
			heartbeat_timer_.cancel(dummy);

			close_target_connection();
			close_client_connection();

			tcp_flow_statistics::instance().remove_connection();
		}
	}

	void tcp_session::close_target_connection()
	{
		for (auto& target : target_sockets_)
		{
			target.close();
		}

		target_sockets_.clear();
	}

	void tcp_session::close_client_connection()
	{
		client_socket_.close();
	}
}
