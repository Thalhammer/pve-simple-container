#pragma once
#include <vector>
#include <string>
#include <set>
#include <tuple>

namespace pvesc {
	namespace container {
		typedef struct {
			std::string source;
			std::string destination;
		} file_t;
		typedef struct {
			typedef struct {
				std::string id;
				std::string name;
				std::string bridge;
				std::string gateway;
				std::string mac;
				std::string ip;
				std::string broadcast;
				size_t netmask;
				bool is_default;
			} interface_t;

			std::vector<interface_t> interfaces;
			std::vector<std::string> nameservers;
		} network_t;
		typedef struct {
			std::string type;
			std::set<std::string> options;
			std::string source;
			std::string path;
		} mount_t;
		typedef struct {
			std::string name;
			std::string value;
		} option_t;
		typedef struct {
			std::string filename;
		} output_t;

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