#include "app.h"
#include "apiclient.h"
#include <iostream>
#include "config.h"
#include "../common/filesystem.h"
#include <fstream>
#include "../common/picojson.h"
#include <memory>

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
						<< "    deploy          Deploy image to PVE host\n"
						<< "    global-config   Build and write the global configuration\n"
						<< "    help            Show this help" << std::endl;
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
			if(config.force) {
				auto containers = client.get_lxcs(config.node);
				for(auto& c : containers) {
					if(c.vmid == config.vmid) {
						if(c.status == "running") {
							std::cout << "Stopping existing Container... " << std::flush;
							task = client.stop_lxc(config.node, config.vmid);
							info = client.await_task_done(config.node, task);
							std::cout << info.exitstatus << std::endl;
							if(!info.ok()) {
								std::cerr << "Failed to stop vm" << std::endl;
								client.delete_file(config.node, config.imagestorage, "vztmpl", tmpfilename);
								return -3;
							}
						}
						break;
					}
				}
			}
			std::cout << "Creating Container...          " << std::flush;
			try {
				task = client.restore_lxc(config.node, config.imagestorage, tmpfilename, config.vmid, config.storage, config.force, config.unprivileged);
				info = client.await_task_done(config.node, task);
				std::cout << info.exitstatus << std::endl;
				if(!info.ok()) {
					std::cerr << "Restore task failed" << std::endl;
					client.delete_file(config.node, config.imagestorage, "vztmpl", tmpfilename);
					return -3;
				}
			} catch(const std::exception& e) {
				client.delete_file(config.node, config.imagestorage, "vztmpl", tmpfilename);
				throw;
			}
			if(config.start) {
				std::cout << "Starting container...          " << std::flush;
				task = client.start_lxc(config.node, config.vmid);
				std::cout << info.exitstatus << std::endl;
				if(!info.ok()) {
					std::cerr << "Start task failed" << std::endl;
					client.delete_file(config.node, config.imagestorage, "vztmpl", tmpfilename);
					return -3;
				}
			}
			std::cout << "Removing temporary images...   " << std::flush;
			client.delete_file(config.node, config.imagestorage, "vztmpl", tmpfilename);
			std::cout << "OK" << std::endl;
			return 0;
		}

		template<typename Func>
		static void ask_property(const std::string& question, std::string& prop, Func f) {
			while(true) {
				std::cout << question << "[" << prop << "] ";
				std::string line;
				std::getline(std::cin, line);
				if(line.empty()) line = prop;
				if(f(line)) {
					prop = line;
					break;
				}
				std::cout << "Invalid input, try again" << std::endl;
			}
		}

		int app::global_config(const std::vector<std::string>& args) {
			config result;
			// Search for config in multiple paths
			auto homedir = common::filesystem::get_home_directory();
			std::string json;
			if(common::filesystem::try_read_file(homedir + "/.config/pvesc.json", json)) {
				result.from_json(json);
			}
			bool login_ok = false;
			std::unique_ptr<apiclient> client;
			while(!login_ok) {
				ask_property("Proxmox hostname", result.login.hostname, [](const std::string& f) {
					return !f.empty();
				});
				ask_property("Proxmox username", result.login.username, [](const std::string& f) {
					return !f.empty();
				});
				ask_property("Proxmox password", result.login.password, [](const std::string& f) {
					return !f.empty();
				});
				ask_property("Proxmox login realm", result.login.realm, [](const std::string& f) {
					return !f.empty();
				});
				client = std::make_unique<apiclient>("https://" + result.login.hostname);
				try {
					client->login(result.login.username, result.login.password, result.login.realm);
					login_ok = true;
				} catch(const std::exception& e) {
					client.reset();
					std::cout << "Failed to login, try again:" << e.what() << std::endl;
				}
			}
			auto nodes = client->get_nodes();
			if(result.node.empty() && !nodes.empty()) result.node = nodes.begin()->node;
			ask_property("Node to deploy to", result.node, [&nodes](const std::string& f) {
				if(nodes.empty()) return !f.empty();
				for(auto& n : nodes) if(n.node == f) return true;
				return false;
			});
			bool online = false;
			for(auto& n : nodes) { if(n.node == result.node) { online = n.status=="online"; break; } }
			auto storages = online ? client->get_storages(result.node) : std::vector<pve::storage>{};
			for(auto& s: storages) {
				if(result.storage.empty() && s.content.count("rootdir")) result.storage = s.storage;
				if(result.imagestorage.empty() && s.content.count("vztmpl")) result.imagestorage = s.storage;
			}
			ask_property("Storage used for container", result.storage, [&storages](const std::string& f) {
				if(storages.empty()) return !f.empty();
				for(auto& s: storages) if(s.storage == f && s.content.count("rootdir")) return true;
				return false;
			});
			ask_property("Temporary storage for upload", result.imagestorage, [&storages](const std::string& f) {
				if(storages.empty()) return !f.empty();
				for(auto& s: storages) if(s.storage == f && s.content.count("vztmpl")) return true;
				return false;
			});
			std::ofstream out(homedir + "/.config/pvesc.json", std::ios::trunc | std::ios::binary);
			out << result.get_global_config();
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
				return app::deploy(args);
			} else if(args[0] == "global-config") {
				return app::global_config(args);
			} else {
				std::cout << "Unknown command" << std::endl;
				return -2;
			}
		}
	}
}