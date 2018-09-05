#include "app.h"

namespace psc {
	int app::run(const std::vector<std::string>& args)
	{
		return 0;
	}
}

extern "C" int main(int argc, const char** argv) {
	std::vector<std::string> args(argv, argv + argc);
	return psc::app::run(args);
}