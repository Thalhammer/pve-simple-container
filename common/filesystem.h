#pragma once
#include <string>
#include <iostream>

namespace pvesc {
	namespace common {
		struct filesystem {
			// Create a temp directory
			static std::string create_temporary_directory(const std::string& tpl);
			static void delete_directory(const std::string& dir);
			static void copy_file(const std::string& source, const std::string& dest);
			static void create_directories(const std::string& path);
			static std::string current_directory();
			static size_t tree_size(const std::string& i);
			static bool try_read_file(const std::string& fname, std::string& out);

			static std::string read_stream(std::istream& stream);
			static std::string read_file(const std::string& path);

			static std::string get_home_directory();
			static bool exists(const std::string& path);

			static void make_executable(const std::string& path);

			struct scoped_directory {
				scoped_directory(std::string s)
					: dir(std::move(s))
				{}
				~scoped_directory() {
					try{
						filesystem::delete_directory(dir);
					} catch(const std::exception& e) {
						std::cout << e.what() << std::endl;
					}
				}
			private:
				std::string dir;
			};
		};
	}
}