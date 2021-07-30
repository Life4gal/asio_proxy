#pragma once

#include "../common/address.hpp"
#include "../common/io_context_pool.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace proxy::tcp
{
	class tcp_session_manager
	{
	public:
		using acceptor_type = boost::asio::ip::tcp::acceptor;
		using size_type = common::io_context_pool::size_type;

		tcp_session_manager(
			const common::address&    listen_address,
			common::forward_addresses target_addresses,
			size_type                 pool_size
			);

		void run();

	private:
		void start_acceptor();

		common::io_context_pool   io_context_pool_;
		common::forward_addresses target_addresses_;

		acceptor_type acceptor_;
	};
}
