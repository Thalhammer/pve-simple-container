#include "filesystem.h"
#include <cstdlib>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace psc {
	namespace common {
		std::string filesystem::create_temporary_directory(const std::string& tpl)
		{
			char buf[tpl.size() + 1];
			std::string::traits_type::copy(buf, tpl.c_str(), tpl.size());
			auto res = mkdtemp(buf);
			if(res == nullptr) throw std::runtime_error("Failed to create directory");
			else return res;
		}

		void filesystem::delete_directory(const std::string& dir)
		{
			for(auto& p: fs::directory_iterator(dir)) {
				if(p.status().type() == fs::file_type::directory) {
					delete_directory(p.path());
				} else {
					if(!fs::remove(p.path())) throw std::runtime_error("Failed to remove directory");
				}
			}
			if(!fs::remove(dir)) throw std::runtime_error("Failed to remove directory");
		}

		void filesystem::copy_file(const std::string& source, const std::string& dest) {
			fs::path pdest(dest);
			if (!fs::is_directory(pdest.parent_path()) && !fs::create_directories(pdest.parent_path())) throw std::runtime_error("Failed to create directory");
			if (!fs::copy_file(source, dest)) throw std::runtime_error("Failed to copy file");
		}

		void filesystem::create_directories(const std::string& dir) {
			if (!fs::is_directory(dir) && !fs::create_directories(dir)) throw std::runtime_error("Failed to create directory");
		}

		std::string filesystem::current_directory() {
			return fs::current_path().string();
		}
	}
}