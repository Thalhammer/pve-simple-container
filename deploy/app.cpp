#include "app.h"
#include "apiclient.h"
#include <iostream>
#include "config.h"
#include "../common/filesystem.h"
#include <fstream>

namespace pvesc {
	namespace deploy {
		config app::read_config() {
			config result;
			// Search for config in multiple paths
			auto homedir = common::filesystem::get_home_directory();
			std::string json;
			if(common::filesystem::try_read_file(homedir + "/.config/pvesc.json", json)) {
				result.from_json(json);
			}
			if(common::filesystem::try_read_file(homedir + "/.pvesc.json", json)) {
				result.from_json(json);
			}
			if(common::filesystem::try_read_file("pvesc.json", json)) {
				result.from_json(json);
			}
			return result;
		}

		void app::show_help() {
			std::cout	<< "pvesc deploy [command] <command_options>\n"
						<< "Commands:\n"
						<< "    help         Show this help" << std::endl;
		}
		
		int app::deploy(const std::vector<std::string>& args) {
			auto config = read_config();
			if(!config.ok()) {
				std::cout << "Invalid config" << std::endl;
				return -3;
			}
			apiclient client("https://" + config.login.hostname);
			client.login(config.login.username, config.login.password, config.login.realm);
			std::ifstream image(config.image, std::ios::binary);
			std::string tmpfilename = "pvesc_deploy_" + std::to_string(time(nullptr)) + "_" + std::to_string(rand()) + ".tar.gz";
			if(!image) {
				std::cout << "Failed to open image" << std::endl;
				return -3;
			}
			std::cout << "Uploading image...             " << std::flush;
			auto task = client.upload_file(config.node, config.imagestorage, "vztmpl", tmpfilename, image);
			auto info = client.await_task_done(config.node, task);
			std::cout << info.exitstatus << std::endl;
			if(!info.ok()) {
				std::cerr << "Upload task failed" << std::endl;
				return -3;
			}
			std::cout << "Creating Container...          " << std::flush;
			task = client.restore_lxc(config.node, config.imagestorage, tmpfilename, config.vmid, config.storage);
			info = client.await_task_done(config.node, task);
			std::cout << info.exitstatus << std::endl;
			if(!info.ok()) {
				std::cerr << "Restore task failed" << std::endl;
				return -3;
			}
			std::cout << "Removing temporary images...   " << std::flush;
			client.delete_file(config.node, config.imagestorage, "vztmpl", tmpfilename);
			std::cout << "OK" << std::endl;
			if(config.start) {
				std::cout << "Starting container...          " << std::flush;
				task = client.start_lxc(config.node, config.vmid);
				std::cout << info.exitstatus << std::endl;
				if(!info.ok()) {
					std::cerr << "Start task failed" << std::endl;
					return -3;
				}
			}
			return 0;
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
			} else if(args[0] == "deploy") {
				app::deploy(args);
				return 0;
			} else {
				std::cout << "Unknown command" << std::endl;
				return -2;
			}
		}
	}
}