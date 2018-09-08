#pragma once
#include <vector>
#include <string>

namespace pvesc {
	namespace deploy {
		struct config;
		class app {
			static config read_config();

			static void show_help();
			static int deploy(const std::vector<std::string>& args);
			static int global_config(const std::vector<std::string>& args);
		public:
			static int run(const std::vector<std::string>& args);
		};
	}
}