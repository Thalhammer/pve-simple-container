#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <set>

namespace pvesc {
	namespace container {
		struct file_t {
			std::string source;
			std::string destination;
			bool check_dependencies;
		};
		struct network_t {
			struct interface_t {
				std::string id;
				std::string name;
				std::string bridge;
				std::string gateway;
				std::string mac;
				std::string ip;
				std::string broadcast;
				size_t netmask;
				bool is_default;
			};
			std::vector<interface_t> interfaces;
			std::vector<std::string> nameservers;
		};
		struct mount_t {
			std::string type;
			std::set<std::string> options;
			std::string source;
			std::string path;
		};
		struct option_t {
			std::string name;
			std::string value;
		};
		struct output_t {
			std::string filename;
		};

		struct recipe {
			std::string name;
			size_t id;
			size_t memory;
			size_t root_size;
			bool root_readonly;
			output_t output;
			std::string main;
			std::vector<file_t> files;
			network_t network;
			std::vector<mount_t> mounts;
			std::vector<option_t> options;

			void from_json(const std::string& json);
			void from_json(std::istream& json);
			std::string to_json() const;

			void reset();

			bool ok() const;

			bool operator!() const {
				return !ok();
			}
			operator bool() const {
				return ok();
			}
		};
	}
}