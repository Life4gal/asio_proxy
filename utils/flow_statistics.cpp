#include "flow_statistics.hpp"

#include "../logger/logger.hpp"

namespace proxy::statistics
{
	void flow_statistics::log_flow()
	{
		log_something();
		log_packets();
	}

	void flow_statistics::log_packets()
	{
		std::scoped_lock lock(mutex_);

		std::string str;
		for (const auto& [context, count] : packet_count_)
		{
			str += "--" + context + "[" + std::to_string(count) + "]";
		}

		if (str.empty())
		{
			str = "No messages received";
		}

		log_info(std::format("statistics interval: {} -> {}", statistics_interval, str));
		packet_count_.clear();
	}
}
