#include "tcp_session.hpp"

#include "../logger/logger.hpp"
#include "../utils/tcp_flow_statistics.hpp"
#include "../utils/utils.hpp"
#include <format>
#include <boost/asio/write.hpp>

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

	void tcp_session::start(const common::forward_addresses& target_addresses)
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

	bool tcp_session::connect_target_server(const common::forward_addresses& target_addresses)
	{
		using namespace boost::asio;

		for (const auto& address : target_addresses)
		{
			auto& target = target_socket_stream_.emplace_back(io_context_);

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
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;

		client_socket_.socket_.async_read_some(
												buffer(client_socket_.buffer_),
												[this](const error_code_t& error_code, const size_type size)
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
																			"client: [{}] -> size: [{}] -> data: [{}]",
																			client_socket_.get_session_id(),
																			size,
																			utils::bin_to_hex(reinterpret_cast<const
																								char*>(client_socket_.
																										buffer_.data()))
																			)
																);

														tcp_flow_statistics::instance().
															add_packet("receive_from_client");

														send_to_target(size);
														async_read_client();
													}
													else if (error_code != error::operation_aborted)
													{
														log_warning(
																	std::format(
																				"unable to receive client message -> client: [{}] -> error: [{}]",
																				client_socket_.get_session_id(),
																				error_code.message()
																				)
																	);
														close();
													}
												}
											);
	}

	void tcp_session::async_read_target()
	{
		for (auto& target : target_socket_stream_)
		{
			async_read_target(target);
		}
	}

	void tcp_session::async_read_target(socket_type& target)
	{
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;
		target.socket_.async_read_some(
										buffer(target.buffer_),
										[this, &target](const error_code_t& error_code, const size_type size)
										{
											if (closed_)
											{
												return;
											}

											if (!error_code)
											{
												log_info(
														std::format(
																	"client: [{}] -> target: [{}] -> size: [{}] -> data: [{}]",
																	client_socket_.get_session_id(),
																	target.get_session_id(),
																	size,
																	utils::bin_to_hex(reinterpret_cast<const
																						char*>(client_socket_.
																								buffer_.data()))
																	)
														);

												tcp_flow_statistics::instance().
													add_packet(std::format("receive_from_target_{}",
																			target.get_remote_address_string()));

												send_to_client(target, size);
												async_read_target(target);
											}
											else if (error_code != error::operation_aborted)
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
										}
									);
	}

	void tcp_session::send_to_client(socket_type& target, const size_type size)
	{
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;

		error_code_t error_code;
		const auto   written = write(client_socket_.socket_, buffer(target.buffer_), error_code);

		if (!error_code)
		{
			log_info(
					std::format(
								"client: [{}] -> target: [{}] -> size: [{}] -> data: [{}]",
								client_socket_.get_session_id(),
								target.get_session_id(),
								written,
								utils::bin_to_hex(reinterpret_cast<const
													char*>(client_socket_.
															buffer_.data()))
								)
					);

			tcp_flow_statistics::instance().
				add_packet("send_to_client");
		}
		else
		{
			log_warning(
						std::format(
									"unable to write client message -> client: [{}] -> target: [{}] -> error: [{}] -> size: [{}] -> data: [{}]",
									client_socket_.get_session_id(),
									target.get_session_id(),
									error_code.message(),
									size,
									utils::bin_to_hex(reinterpret_cast<const
														char*>(client_socket_.buffer_.data()))
									)
						);
		}
	}

	void tcp_session::send_to_target(const size_type size)
	{
		using namespace boost::asio;
		using error_code_t = boost::system::error_code;

		for (auto& target : target_socket_stream_)
		{
			error_code_t error_code;
			const auto   written = write(client_socket_.socket_, buffer(target.buffer_), error_code);

			if (!error_code)
			{
				log_info(
						std::format(
									"client: [{}] -> target: [{}] -> size: [{}] -> data: [{}]",
									client_socket_.get_session_id(),
									target.get_session_id(),
									written,
									utils::bin_to_hex(reinterpret_cast<const
														char*>(client_socket_.
																buffer_.data()))
									)
						);

				tcp_flow_statistics::instance().
					add_packet(std::format("send_to_target_{}", target.get_remote_address_string()));
			}
			else
			{
				log_warning(
							std::format(
										"unable to write target message -> client: [{}] target: [{}] -> error: [{}] -> size: [{}] -> data: [{}]",
										client_socket_.get_session_id(),
										target.get_session_id(),
										error_code.message(),
										size,
										utils::bin_to_hex(reinterpret_cast<const
															char*>(client_socket_.buffer_.data()))
										)
							);
			}
		}
	}

	void tcp_session::keep_alive()
	{
		using error_code_t = boost::system::error_code;

		try
		{
			heartbeat_timer_.expires_from_now(static_cast<boost::posix_time::seconds>(heartbeat_interval));
			heartbeat_timer_.async_wait(
										[self = shared_from_this()](const error_code_t& error_code)
										{
											if (!self->closed_ && !error_code)
											{
												log_warning(
															std::format(
																		"[{}] seconds did not receive any information, close the tcp connection -> client: [{}]",
																		heartbeat_interval,
																		self->client_socket_.get_session_id()));
												self->close();
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
		for (auto& target : target_socket_stream_)
		{
			if (auto& socket = target.socket_; socket.is_open())
			{
				boost::system::error_code dummy;
				socket.shutdown(boost::asio::socket_base::shutdown_both, dummy);
				socket.close(dummy);
			}
		}

		target_socket_stream_.clear();
	}

	void tcp_session::close_client_connection()
	{
		if (auto& socket = client_socket_.socket_; socket.is_open())
		{
			boost::system::error_code dummy;
			socket.shutdown(boost::asio::socket_base::shutdown_both, dummy);
			socket.close(dummy);
		}
	}
}
