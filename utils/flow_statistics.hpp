#pragma once

#include <map>
#include <mutex>

namespace proxy::statistics
{
	class flow_statistics
	{
	public:
		// using duration_type = task_timer<boost::posix_time::seconds>::timer_duration_type;
		// constexpr static duration_type statistics_interval = boost::posix_time::seconds{30 * 60};

		using size_type = std::size_t;
		using duration_type = size_type;

		constexpr static duration_type statistics_interval = 30 * 60;

		flow_statistics() = default;
		// flow_statistics(const flow_statistics& other)                = delete;
		// flow_statistics& operator=(const flow_statistics& other)     = delete;
		// flow_statistics(flow_statistics&& other) noexcept            = delete;
		// flow_statistics& operator=(flow_statistics&& other) noexcept = delete;
	protected:
		virtual ~flow_statistics() = default;

		void log_flow();

		void log_packets();

		std::mutex mutex_;

		std::map<std::string, size_type> packet_count_;

	private:
		virtual void log_something() {}
	};
}
