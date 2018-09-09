#include "app.h"
#include "../common/filesystem.h"
#include "../common/system.h"
#include "../common/string_helper.h"
#include <iostream>
#include "recipe.h"
#include <fstream>
#include "overlay.h"

namespace pvesc {
	namespace container {
		
		void app::show_help() {
			std::cout	<< "pvesc container [command] <command_options>\n"
						<< "Commands:\n"
						<< "    help         Show this help\n"
						<< "    build        Build a container based on pvesc.json\n"
						<< "    check        Check pvesc.json without building anything" << std::endl;
		}

		int app::build_container_from_file() {
			std::ifstream conf("pvesc.json", std::ios::binary);
			if(!conf) {
				std::cout << "Failed to open config file" << std::endl;
				return -3;
			}
			recipe r;
			r.from_json(conf);

			if(!r) {
				std::cout << "Invalid config" << std::endl;
				return -3;
			}

			return build_container(r);
		}

		int app::build_container(const recipe& i) {
			// Create temp directory
			auto dir = common::filesystem::create_temporary_directory("/tmp/pvescXXXXXX");
			common::filesystem::scoped_directory auto_del(dir);

			auto baseimage = find_baseimage();
			if(baseimage.empty()) {
				std::cout << "Failed to find base image" << std::endl;
				return -3;
			}
			// Unpack baseimage into temp directory
			std::string cmd = "tar -xzf " + baseimage + " -C " + dir;
			auto res = system(cmd.c_str());
			if(res != 0) {
				std::cout << "Failed to unpack base image" << std::endl;
				return -3;
			}

			// Unpack overlays first as some dependencies might already be fixed by overlays
			auto overlays = i.overlays;
			try {
			overlay::load_dependencies(overlays);
			} catch(const std::exception& e) {
				std::cout << e.what() << std::endl;
				return -3;
			}
			for(auto& o : overlays) {
				std::cout << "Adding overlay " << o << "...       " << std::flush;
				overlay obj;
				if(!obj.find_overlay_by_name(o)) {
					std::cout << "Failed to find overlay " << o << std::endl;
					return -3;
				}
				std::string cmd = "tar -xzf " + obj.image + " -C " + dir;
				auto res = system(cmd.c_str());
				if(res != 0) {
					std::cout << "Failed to unpack overlay " << o << std::endl;
					return -3;
				}
				std::cout << "OK" << std::endl;
			}

			for(auto& f : i.files) {
				for(auto& pair : common::filesystem::glob_files(f.source, f.destination)) {
					std::cout << "Adding " << pair.first << "...       " << std::flush;
					common::filesystem::copy_file(pair.first, dir + pair.second);
					std::cout << "OK" << std::endl;
					if(f.check_dependencies) {
						auto deps = common::system::run_command("ldd " + pair.first + " | awk '/=>/{print $(NF-1)}' | sed 1d | sort -u");
						for(auto& dep : common::lines(deps)) {
							if(common::filesystem::exists(dir + dep)) continue;
							std::cout << "Adding " << dep << " as dependency...       " << std::flush;
							common::filesystem::copy_file(dep, dir + dep);
							std::cout << "OK" << std::endl;
						}
					}
				}
			}

			res = write_pveconf(i, dir);
			if(res != 0) return res;
			res = write_init_rcS(i, dir);
			if(res != 0) return res;
			res = write_inittab(i, dir);
			if(res != 0) return res;
			res = write_resolv_conf(i, dir);
			if(res != 0) return res;

			for(auto& m : i.mounts) {
				common::filesystem::create_directories(dir + m.path);
			}
			auto cwd = common::filesystem::current_directory();
			cmd = "(cd " + dir + " && tar --owner=0 --group=0 -czf " + cwd + "/" + i.output.filename + " .)";
			res = system(cmd.c_str());
			if(res != 0) {
				std::cout << "Failed to pack image" << std::endl;
				return -3;
			}
			return 0;
		}

		int app::check_config() {
			std::ifstream conf("pvesc.json", std::ios::binary);
			if(!conf) {
				std::cout << "Failed to open config file" << std::endl;
				return -3;
			}
			recipe r;
			r.from_json(conf);

			if(!r) {
				std::cout << "Invalid config" << std::endl;
				return -3;
			}
			std::cout << "Config file valid" << std::endl;
			return 0;
		}

		int app::write_pveconf(const recipe& i, const std::string& dir) {
			common::filesystem::create_directories(dir + "/etc/vzdump");
			std::ofstream pveconf(dir + "/etc/vzdump/pct.conf", std::ios::binary | std::ios::trunc);
			if(!pveconf) {
				std::cout << "Failed to write pve config" << std::endl;
				return -3;
			}

			auto size = i.root_size;
			if(size == 0) size = get_image_size(common::filesystem::tree_size(dir));
			std::cout << "Using rootfs size of " << size << "Mbytes" << std::endl;

			pveconf
				<< "arch: amd64\n"
				<< "hostname: " << i.name << "\n"
				<< "memory: " << i.memory << "\n"
				<< "ostype: unmanaged\n"
				<< "rootfs: local-lvm:vm-" << i.id << "-disk-1,size=" << size << "M,ro=" << (i.root_readonly?"1":"0") << "\n"
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
			common::filesystem::make_executable(dir + "/etc/init.d/rcS");
			return 0;
		}

		int app::write_inittab(const recipe& i, const std::string& dir) {
			std::ofstream inittab(dir + "/etc/inittab", std::ios::binary | std::ios::trunc);
			if(!inittab) {
				std::cout << "Failed to write inittab" << std::endl;
				return -3;
			}
			inittab
				<< "::sysinit:/etc/init.d/rcS\n"
				<< "tty1::respawn:/bin/getty -L tty1 115200 vt100\n"
				<< "console::respawn:" << i.main << "\n";
			inittab.flush();
			inittab.close();
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

		size_t app::get_image_size(size_t s) {
			if(s % 1024) s = s/1024 + 1;
			else s = s/1024;
			if(s % 1024) s = s/1024 + 1;
			else s = s/1024;
			s = (s*15)/10; // Add 50% as safety margin
			size_t multiple = 1;
			while(s > multiple) multiple = multiple << 1;
			return multiple;
		}

		std::string app::find_baseimage() {
			auto homedir = common::filesystem::get_home_directory();
			std::string paths[] = {
				"baseimage.tar.gz",
				homedir + "/.pvesc/baseimage.tar.gz",
				"/usr/share/pve-simple-container/baseimage.tar.gz"
			};
			for(auto& p : paths) {
				if(common::filesystem::exists(p))
					return p;
			}
			return "";
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
				// Build a container based on pvesc.json
				return build_container_from_file();
			} else if(args[0] == "check") {
				// Build a container based on pvesc.json
				return check_config();
			} else {
				std::cout << "Unknown command" << std::endl;
				return -2;
			}
		}
	}
}