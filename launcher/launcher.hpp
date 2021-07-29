#pragma once

#include "../common/address.hpp"
#include <vector>

namespace proxy
{
	class launcher
	{
	public:
		void start();

	private:
		static std::vector<common::address> read_forward_address();
	};
}
