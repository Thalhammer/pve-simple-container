#pragma once
#include <vector>
#include <string>

namespace pvesc {
	namespace container {
		struct recipe;
		class app {
			static void show_help();
			static int build_container_from_file();
			static int build_container(const recipe& i);
			static int check_config();
			static int write_pveconf(const recipe& i, const std::string& dir);
			static int write_init_rcS(const recipe& i, const std::string& dir);
			static int write_inittab(const recipe& i, const std::string& dir);
			static int write_resolv_conf(const recipe& i, const std::string& dir);
			static size_t get_image_size(size_t s);
			static std::string find_baseimage();
			static std::string find_overlay(const std::string& name);
		public:
			static int run(const std::vector<std::string>& args);
		};
	}
}