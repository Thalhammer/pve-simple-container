#pragma once
#include <vector>
#include <string>

namespace psc {
	namespace container {
		struct recipe;
		class app {
			static void show_help();
			static int build_container_from_file();
			static int build_container(const recipe& i);
			static int write_pveconf(const recipe& i, const std::string& dir);
			static int write_init_rcS(const recipe& i, const std::string& dir);
			static int write_resolv_conf(const recipe& i, const std::string& dir);
		public:
			static int run(const std::vector<std::string>& args);
		};
	}
}