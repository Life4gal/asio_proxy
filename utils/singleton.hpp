#pragma once

namespace proxy::utils
{
	template <typename T>
	class singleton
	{
	public:
		using instance_type = T;

		static instance_type& instance()
		{
			static instance_type instance_;
			return instance_;
		}

		singleton()                            = delete;
		singleton(const singleton&)            = delete;
		singleton& operator=(const singleton&) = delete;

		~singleton()                               = default;
		singleton(singleton&&) noexcept            = default;
		singleton& operator=(singleton&&) noexcept = default;
	};
}
