#include "io_context_pool.hpp"
#include <thread>

namespace proxy
{
	io_context_pool::io_context_pool(const size_type init_size)
		: next_valid_context_(0)
	{
		io_contexts_.reserve(init_size);
		// Give all the io_contexts work to do so that their run() functions will not
		// exit until they are explicitly stopped.
		for (size_type i = 0; i < init_size; ++i)
		{
			auto io_context = std::make_shared<io_context_type>();
			io_contexts_.push_back(io_context);
			io_works_.push_back(boost::asio::require(io_context->get_executor(),
													boost::asio::execution::outstanding_work_t::tracked));
		}
	}

	void io_context_pool::run()
	{
		// Create a pool of threads to run all of the io_contexts.
		const auto               total = io_contexts_.size();
		std::vector<std::thread> threads;
		threads.reserve(total);

		for (auto& io_context : io_contexts_)
		{
			threads.emplace_back(
								[&io_context]() -> void
								{
									io_context->run();
								});
		}

		// Wait for all threads in the pool to exit.
		for (auto& thread : threads)
		{
			thread.join();
		}
	}

	void io_context_pool::shutdown()
	{
		// Explicitly stop all io_contexts.
		for (auto& io_context : io_contexts_)
		{
			io_context->stop();
		}
	}

	io_context_pool::io_context_handle_type& io_context_pool::get_io_context()
	{
		// Use a round-robin scheme to choose the next io_context to use.
		auto& io_context = io_contexts_[next_valid_context_];
		++next_valid_context_;
		if (next_valid_context_ == io_contexts_.size())
		{
			next_valid_context_ = 0;
		}
		return io_context;
	}
}
