#include "app.h"
#include <iostream>

namespace psc {
	namespace deploy {
		void app::show_help() {
			std::cout	<< "psc deploy [command] <command_options>\n"
						<< "Commands:\n"
						<< "    help         Show this help" << std::endl;
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
			} else {
				std::cout << "Unknown command" << std::endl;
				return -2;
			}
		}
	}
}