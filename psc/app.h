#pragma once
#include <vector>
#include <string>

namespace psc {
	class app {
		static void show_help();
		static void show_version();
	public:
		static int run(const std::vector<std::string>& args);
	};
}