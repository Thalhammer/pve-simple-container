#pragma once
#include <string>
#include <iostream>

namespace psc {
	namespace common {
		struct filesystem {
			// Create a temp directory
			static std::string create_temporary_directory(const std::string& tpl);
			static void delete_directory(const std::string& dir);
			static void copy_file(const std::string& source, const std::string& dest);
			static void create_directories(const std::string& path);
			static std::string current_directory();
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