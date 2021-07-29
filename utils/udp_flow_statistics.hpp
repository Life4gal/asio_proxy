#pragma once

#include "task_timer.hpp"
#include "flow_statistics.hpp"

namespace proxy
{
	class udp_flow_statistics final : public statistics::flow_statistics
	{
	public:
		udp_flow_statistics();

		void add_packet(const std::string& context);

	private:
		task_timer<boost::posix_time::seconds> timer_;
	};
}
