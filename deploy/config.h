#pragma once
#include <vector>
#include <string>
#include <set>
#include <tuple>

namespace picojson { class value; }
namespace pvesc {
	namespace deploy {
		typedef struct {
			std::string hostname;
			std::string username = "root";
			std::string password;
			std::string realm = "pam";
			bool ignore_ssl = false;
		} login_t;

		struct config {
			login_t login;
			std::string node = "pve";
			std::string storage = "local-lvm";
			size_t vmid = 100;
			std::string imagestorage = "local";
			std::string image;
			bool start = false;
			bool force = false;
			bool unprivileged = false;

			void from_json(const std::string& json);
			void from_json(std::istream& json);
			std::string to_json() const;
			std::string get_global_config() const;
			void store_config(picojson::value& val) const;
			void store_global_config(picojson::value& val) const;
			void store_local_config(picojson::value& val) const;

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