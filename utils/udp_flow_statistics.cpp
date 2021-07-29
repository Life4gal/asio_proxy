#include "udp_flow_statistics.hpp"

namespace proxy
{
	udp_flow_statistics::udp_flow_statistics()
	{
		timer_.add_callback([this]
		{
			log_flow();
		});
		timer_.start(static_cast<task_timer<boost::posix_time::seconds>::timer_duration_type>(statistics_interval));
	}

	void udp_flow_statistics::add_packet(const std::string& context)
	{
		std::scoped_lock lock(mutex_);

		++packet_count_[context];
	}
}
