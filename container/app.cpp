#include "app.h"
#include "../common/filesystem.h"
#include <iostream>
#include "recipe.h"
#include <fstream>

#define BASEIMAGE_PATH "../container/baseimage.tar.gz"

namespace psc {
	namespace container {
		
		void app::show_help() {
			std::cout	<< "psc container [command] <command_options>\n"
						<< "Commands:\n"
						<< "    help         Show this help\n"
						<< "    build        Build a container based on psc.json" << std::endl;
		}

		int app::build_container_from_file() {
			std::ifstream conf("psc.json", std::ios::binary);
			if(!conf) {
				std::cout << "Failed to open config file" << std::endl;
				return -3;
			}
			recipe r;
			r.from_json(conf);

			return build_container(r);
		}

		int app::build_container(const recipe& i) {
			// Create temp directory
			auto dir = common::filesystem::create_temporary_directory("/tmp/pscXXXXXX");
			common::filesystem::scoped_directory auto_del(dir);

			// Unpack baseimage into temp directory
			std::string cmd = "tar -xzf " BASEIMAGE_PATH " -C " + dir;
			auto res = system(cmd.c_str());
			if(res != 0) {
				std::cout << "Failed to unpack base image" << std::endl;
				return -3;
			}

			for(auto& f : i.files) {
				common::filesystem::copy_file(f.source, dir + f.destination);
			}

			res = write_pveconf(i, dir);
			if(res != 0) return res;
			res = write_init_rcS(i, dir);
			if(res != 0) return res;
			res = write_resolv_conf(i, dir);
			if(res != 0) return res;

			for(auto& m : i.mounts) {
				common::filesystem::create_directories(dir + m.path);
			}
			auto cwd = common::filesystem::current_directory();
			cmd = "(cd " + dir + " && tar --lzop -cf " + cwd + "/vzdump-lxc-" + std::to_string(i.id) + "-`date +%Y`_`date +%m`_`date +%d`-`date +%H`_`date +%M`_`date +%S`.tar.lzo .)";
			res = system(cmd.c_str());
			if(res != 0) {
				std::cout << "Failed to pack image" << std::endl;
				return -3;
			}

			std::cout << dir << std::endl;
			return 0;
		}

		int app::write_pveconf(const recipe& i, const std::string& dir) {
			common::filesystem::create_directories(dir + "/etc/vzdump");
			std::ofstream pveconf(dir + "/etc/vzdump/pct.conf", std::ios::binary | std::ios::trunc);
			if(!pveconf) {
				std::cout << "Failed to write pve config" << std::endl;
				return -3;
			}
			pveconf
				<< "arch: amd64\n"
				<< "hostname: " << i.name << "\n"
				<< "memory: " << i.memory << "\n"
				<< "ostype: unmanaged\n"
				<< "rootfs: local-lvm:vm-" << i.id << "-disk-1,size=128M,ro=1\n"
				<< "swap: 0\n"
				<< "\n";
			for(auto& iface : i.network.interfaces) {
				pveconf
					<< iface.id << ": name=" << iface.name
					<< ",bridge=" << iface.bridge
					<< ",gw=" << iface.gateway
					<< ",hwaddr=" << iface.mac
					<< ",ip=" << iface.ip
					<< "/" << iface.netmask
					<< ",type=veth\n";
			}
			pveconf << "\n";
			for(auto& o : i.options) {
				pveconf << o.name << ": " << o.value << "\n";
			}
			pveconf.flush();
			pveconf.close();
			return 0;
		}

		int app::write_init_rcS(const recipe& i, const std::string& dir) {
			common::filesystem::create_directories(dir + "/etc/init.d");
			std::ofstream rcS(dir + "/etc/init.d/rcS", std::ios::binary | std::ios::trunc);
			if(!rcS) {
				std::cout << "Failed to write init rcS" << std::endl;
				return -3;
			}
			rcS
				<< "#!/bin/sh\n"
				<< "/bin/syslogd\n"
				<< "/bin/mount -a\n"
				<< "\n";
			for(auto& iface : i.network.interfaces) {
				rcS
					<< "ip addr add " << iface.ip << "/" << iface.netmask << " broadcast " << iface.broadcast << " dev " << iface.name << "\n"
					<< "ip link set " << iface.name << " up\n";
				if(iface.is_default)
					rcS << "ip route add default via " << iface.gateway << " dev " << iface.name << "\n";
			}
			rcS << "\n";
			for(auto& m : i.mounts) {
				rcS << "mount -t " << m.type;
				if(!m.options.empty()) {
					rcS << " -o";
					bool first = true;
					for(auto& o: m.options) {
						if(!first) rcS << ",";
						first = false;
						rcS << o;
					}
				}
				rcS << " " << m.source << " "<< m.path << "\n";
			}
			rcS.flush();
			rcS.close();
			return 0;
		}

		int app::write_resolv_conf(const recipe& i, const std::string& dir) {
			std::ofstream resolv(dir + "/etc/resolv.conf", std::ios::binary | std::ios::trunc);
			if(!resolv) {
				std::cout << "Failed to write resolv.conf" << std::endl;
				return -3;
			}
			for(auto& n : i.network.nameservers) {
				resolv << "nameserver " << n << "\n";
			}
			resolv.flush();
			resolv.close();
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
			} else if(args[0] == "build") {
				// Build a container based on psc.json
				return build_container_from_file();
			} else {
				std::cout << "Unknown command" << std::endl;
				return -2;
			}
		}
	}
}