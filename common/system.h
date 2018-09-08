#pragma once
#include <string>
#include <iostream>

namespace pvesc {
	namespace common {
		struct system {
			// Create a temp directory
			static std::string run_command(const std::string& cmd);
		};
	}
}