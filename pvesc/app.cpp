#include "app.h"
#include <iostream>
#include "../container/app.h"
#include "../deploy/app.h"
#include "version.h"

namespace pvesc {
	void app::show_help() {
		std::cout	<< "pvesc [command] <command_options>\n"
					<< "Commands:\n"
					<< "    help         Show this help\n"
					<< "    container    Create containers for deployment to pve (i.e. exported lxc vm)\n"
					<< "    deploy       Deploy a container to a specific pve host\n"
					<< "    version      Print version" << std::endl;
	}

	void app::show_version() {
		std::cout << "version " << PSC_FULL_VERSION << std::endl;
	}

	int app::run(const std::vector<std::string>& args)
	{
		if(args.empty()) {
			show_help();
			return -1;
		}

		if(args[0] == "help") {
			show_help();
			return 0;
		} else if(args[0] == "container") {
			std::vector<std::string> nargs(args.begin() + 1, args.end());
			return container::app::run(nargs);
		} else if(args[0] == "deploy") {
			std::vector<std::string> nargs(args.begin() + 1, args.end());
			return deploy::app::run(nargs);
		} else if(args[0] == "version") {
			show_version();
			return 0;
		} else if(args[0] == "build") {
			// Shorthand for container build
			std::vector<std::string> nargs(args.begin() + 1, args.end());
			nargs.insert(nargs.begin(), "build");
			return container::app::run(nargs);
		} else {
			std::cout << "Unknown command" << std::endl;
			return -2;
		}
	}
}

extern "C" int main(int argc, const char** argv) {
	std::vector<std::string> args(argv + 1, argv + argc);
	srand(time(nullptr));
	try {
		return pvesc::app::run(args);
	} catch(const std::exception& e) {
		std::cout << "An error occured: " << e.what() << std::endl;
		return -3;
	}
}