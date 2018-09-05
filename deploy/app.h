#pragma once
#include <vector>
#include <string>

namespace psc {
	namespace deploy {

		class app {
		public:
			static int run(const std::vector<std::string>& args);
		};
	}
}