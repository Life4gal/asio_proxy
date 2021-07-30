#pragma once

#include "task_timer.hpp"
#include "flow_statistics.hpp"

namespace proxy
{
	class udp_flow_statistics final : public statistics::flow_statistics
	{
	public:
		static udp_flow_statistics& instance()
		{
			static udp_flow_statistics udp_flow_statistics_;
			return udp_flow_statistics_;
		}

		void add_packet(const std::string& context);

	private:
		udp_flow_statistics();

		task_timer<boost::posix_time::seconds> timer_;
	};
}
