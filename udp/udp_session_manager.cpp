#include "udp_session_manager.hpp"

#include "../logger/logger.hpp"
#include "../utils/udp_flow_statistics.hpp"
#include "udp_session.hpp"

namespace proxy::udp
{
	udp_session_manager::udp_session_manager(
		const address_type&       listen_address,
		common::forward_addresses target_addresses,
		const size_type           pool_size)
		: io_context_pool_(pool_size),
		client_socket_(io_context_pool_.get_io_context(), listen_address),
		target_addresses_(std::move(target_addresses)) { }

	void udp_session_manager::run()
	{
		async_receive_client();

		log_info(std::format("UDP connection is working -> listen: [{}]", client_socket_.get_local_address_string()));

		io_context_pool_.run();
	}

	void udp_session_manager::send_to_client(
		udp_socket&     target_socket,
		const size_type size)
	{
		[[clang::no_destroy]] static std::string statistics_packet{"send_to_client"};

		const auto address = target_socket.get_local_address();

		try
		{
			const auto written = client_socket_.send_to(target_socket.buffer_, size, address);

			log_info(
					std::format(
								"client port: [{}] -> target: [{}] -> size: [{}] -> sample data: [{}]",
								address.to_string(),
								target_socket.get_remote_address_string(),
								written,
								target_socket.sample_buffer(written % socket_type::basic_sample_size)
								)
					);

			udp_flow_statistics::instance().add_packet(statistics_packet);
		}
		catch (const std::exception& e)
		{
			log_warning(
						std::format(
									"unable to write client message -> client port: [{}] -> target: [{}] -> error: [{}] -> size: [{}] -> data: [{}]",
									address.to_string(),
									target_socket.get_remote_address_string(),
									e.what(),
									size,
									client_socket_.sample_buffer()
									)
						);
		}
	}

	void udp_session_manager::close_session(const address_type& address)
	{
		std::scoped_lock lock(mutex_);

		udp_sessions_.erase(address.to_string());

		log_info(std::format("close: [{}], remainder: [{}]", address.to_string(), udp_sessions_.size()));
	}

	void udp_session_manager::async_receive_client()
	{
		[[clang::no_destroy]] static std::string statistics_packet{"receive_from_client"};

		client_socket_.async_receive([this](const boost::system::error_code& error_code, const size_type size)
		{
			if (!error_code)
			{
				const auto address = client_socket_.get_local_address();

				auto session = get_or_create_udp_session(address);

				udp_flow_statistics::instance().
					add_packet(statistics_packet);

				session->send_to_target(client_socket_, size);
				async_receive_client();
			}
			else if (error_code == boost::asio::error::operation_aborted)
			{
				log_warning(
							std::format(
										"unable to receive target message -> target: {} -> error: {}",
										client_socket_.
										get_remote_address_string(),
										error_code.message()
										)
							);
			}
		});
	}

	udp_session_manager::udp_session_handle udp_session_manager::get_or_create_udp_session(const address_type& address)
	{
		std::scoped_lock lock(mutex_);

		udp_session_handle ret;

		if (const auto it = udp_sessions_.find(address.to_string());
			it == udp_sessions_.end())
		{
			ret = std::make_shared<udp_session>(io_context_pool_.get_io_context(),
												address,
												target_addresses_,
												*this);
			ret->async_receive_target();

			udp_sessions_.emplace(address.to_string(), ret);
		}
		else
		{
			ret = it->second;
		}

		return ret;
	}
}
