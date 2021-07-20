#pragma once

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>

namespace proxy
{
	template <typename Duration = boost::posix_time::milliseconds>
	class task_timer
	{
	public:
		using io_context_type = boost::asio::io_context;
		using io_executor_type = boost::asio::any_io_executor;

		using timer_type = boost::asio::deadline_timer;
		using timer_duration_type = Duration;

		using callback_type = std::function<void()>;
		using callback_pool = std::vector<callback_type>;

		task_timer()
			: io_work_(boost::asio::require(io_context_.get_executor(),
											boost::asio::execution::outstanding_work_t::tracked)),
			timer_(io_context_),
			duration_(0),
			is_single_shot_(false) {}

		void start()
		{
			timer_.expires_from_now(duration_);
			timer_.async_wait(
							[this](const boost::system::error_code& error)
							{
								if (!error)
								{
									for (auto& callback : callbacks_)
									{
										callback();
									}

									if (!is_single_shot_)
									{
										start();
									}
								}
							});
		}

		void start(timer_duration_type duration)
		{
			if (!io_context_.stopped())
			{
				duration_ = duration;
				thread_   = std::thread{
					[this]
					{
						this->io_context_.run();
					}
				};
				start();
			}
		}

		void stop()
		{
			timer_.cancel();
		}

		void add_callback(callback_type&& callback)
		{
			callbacks_.emplace_back(callback);
		}

		void set_single_shot(const bool single)
		{
			is_single_shot_ = single;
		}

	private:
		io_context_type  io_context_;
		io_executor_type io_work_;

		timer_type timer_;

		callback_pool callbacks_;

		std::thread thread_;

		timer_duration_type duration_;

		std::atomic<bool> is_single_shot_;
	};
}
