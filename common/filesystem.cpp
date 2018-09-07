#include "filesystem.h"
#include <cstdlib>
#include <experimental/filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace fs = std::experimental::filesystem;

namespace pvesc {
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

		size_t filesystem::tree_size(const std::string& dir) {
			size_t size = 0;
			for(auto& p : fs::recursive_directory_iterator(dir)) {
				if(p.symlink_status().type() == fs::file_type::regular) {
					auto s = fs::file_size(p.path());
					size += s;
				} else size += 4096;
			}
			return size;
		}

		bool filesystem::try_read_file(const std::string& fname, std::string& out) {
			if(fs::exists(fname)) {
				std::ifstream stream(fname, std::ios::binary);
				if(stream) {
					out = read_stream(stream);
					return true;
				}
			}
			return false;
		}

		std::string filesystem::read_stream(std::istream& stream) {
			std::string res;
			while(!stream.eof()) {
				char buf[4096];
				auto size = stream.read(buf, sizeof(buf)).gcount();
				res += std::string(buf, buf + size);
			}
			return res;
		}

		std::string filesystem::read_file(const std::string& path) {
			std::ifstream stream(path, std::ios::binary);
			if(!stream) throw std::runtime_error("Failed to open file");
			return read_stream(stream);
		}

		std::string filesystem::get_home_directory() {
			auto homedir = getenv("HOME");
			if(homedir == nullptr) {
				struct passwd pwd;
				struct passwd *result;
				int s;
				size_t bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
				if (bufsize == size_t(-1))
					bufsize = 0x4000; // = all zeroes with the 14th bit set (1 << 14)
				std::string buf(bufsize, '\0');
				s = getpwuid_r(getuid(), &pwd, (char*)buf.data(), buf.size(), &result);
				if (result == NULL) {
					if (s == 0)
						throw std::runtime_error("Failed to get home directory: user not found");
					else {
						throw std::runtime_error("Failed to get home directory:" + std::to_string(s));
					}
				}
				homedir = result->pw_dir;
			}
			return homedir;
		}
	}
}