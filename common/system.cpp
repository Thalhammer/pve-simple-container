#include "system.h"

namespace pvesc {
	namespace common {
		std::string system::run_command(const std::string& cmd) {
			std::string res;

			/* Open the command for reading. */
			auto fp = popen(cmd.c_str(), "r");
			if (fp == NULL) {
				throw std::runtime_error("Failed to run command");
			}

			std::string buf(1024, '\0');
			while(!feof(fp) && !ferror(fp)) {
				auto s = fread(const_cast<char*>(buf.data()), 1, buf.size(), fp);
				res += buf.substr(0, s);
			}

			/* close */
			pclose(fp);

			return res;
		}
	}
}
