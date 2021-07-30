#pragma once

#include <atomic>
#include "task_timer.hpp"
#include "flow_statistics.hpp"

namespace proxy
{
	class tcp_flow_statistics final : public statistics::flow_statistics
	{
	public:
		static tcp_flow_statistics& instance()
		{
			static tcp_flow_statistics tcp_flow_statistics_;
			return tcp_flow_statistics_;
		}

		void add_connection();

		void remove_connection();

		void add_packet(const std::string& context);

	private:
		tcp_flow_statistics();

		std::atomic<int>                       count_;
		task_timer<boost::posix_time::seconds> timer_;

		void log_something() override;
	};
}
