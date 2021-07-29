#include "tcp_flow_statistics.hpp"

#include "../logger/logger.hpp"

namespace proxy
{
	tcp_flow_statistics::tcp_flow_statistics()
		: count_(0)
	{
		timer_.add_callback([this]
		{
			log_flow();
		});
		timer_.start(static_cast<task_timer<boost::posix_time::seconds>::timer_duration_type>(statistics_interval));
	}


	void tcp_flow_statistics::add_connection()
	{
		++count_;
		log_something();
	}


	void tcp_flow_statistics::remove_connection()
	{
		--count_;
		log_something();
	}

	void tcp_flow_statistics::add_packet(const std::string& context)
	{
		std::scoped_lock lock(mutex_);

		++packet_count_[context];
	}

	void tcp_flow_statistics::log_something()
	{
		log_info(std::format("current connections : {}", count_.load()));
	}
}
