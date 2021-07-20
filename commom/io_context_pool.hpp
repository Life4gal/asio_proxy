#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <vector>
#include <list>
#include <memory>

namespace proxy
{
	class io_context_pool : boost::noncopyable
	{
	public:
		using io_context_type = boost::asio::io_context;
		using io_context_handle_type = std::shared_ptr<io_context_type>;
		using io_executor_type = boost::asio::any_io_executor;

		using context_pool = std::vector<io_context_handle_type>;
		using executor_pool = std::list<io_executor_type>;

		using size_type = context_pool::size_type;

		explicit io_context_pool(size_type init_size);

		// Run all io_context objects in the pool.
		void run();

		// Stop all io_context objects in the pool.
		void shutdown();

		// Get an io_context to use.
		io_context_handle_type& get_io_context();

	private:
		// The pool of io_contexts.
		context_pool io_contexts_;
		// The work-tracking executors that keep the io_contexts running.
		executor_pool io_works_;
		// The next io_context to use for a connection.
		size_type next_valid_context_;
	};
}
