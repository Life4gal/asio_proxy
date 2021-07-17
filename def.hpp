#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <array>
#include <string>
#include <memory>

namespace proxy
{
	using io_service_type = boost::asio::io_service;
	using socket_type = boost::asio::ip::tcp::socket;
	using size_type = size_t;
	template <size_type N>
	using stream_type = std::array<unsigned char, N>;

	using host_type = std::string;
	using port_type = unsigned short;

	class bridge;
	using bridge_type = std::shared_ptr<bridge>;
	using acceptor_type = boost::asio::ip::tcp::acceptor;

	bridge_type make_bridge(io_service_type& io_service);
}
