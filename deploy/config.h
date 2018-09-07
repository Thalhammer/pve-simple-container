#pragma once
#include <vector>
#include <string>
#include <set>
#include <tuple>

namespace pvesc {
	namespace deploy {
		typedef struct {
			std::string hostname;
			std::string username = "root";
			std::string password;
			std::string realm = "pam";
		} login_t;

		struct config {
			login_t login;
			std::string node = "pve";
			std::string storage = "local-lvm";
			size_t vmid = 100;
			std::string imagestorage = "local";
			std::string image;
			bool start = false;

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