#include "config.h"
#define PICOJSON_USE_INT64
#include "../common/json_helper.h"
#include <sstream>

using pvesc::common::json_get;

namespace pvesc {
	namespace deploy {
		void config::from_json(const std::string& json) {
			std::istringstream ss(json);
			this->from_json(ss);
		}

		void config::from_json(std::istream& json) {
			picojson::value val;
			auto err = picojson::parse(val, json);
			if(!err.empty()) throw std::logic_error("Invalid json");
			val = val.get("deploy");
			if(val.is<picojson::null>()) return;
			if(val.get("login").is<picojson::object>()) {
				auto& login = val.get("login");
				this->login.hostname = json_get<std::string>(login, "hostname", this->login.hostname);
				this->login.username = json_get<std::string>(login, "username", this->login.username);
				this->login.password = json_get<std::string>(login, "password", this->login.password);
				this->login.realm = json_get<std::string>(login, "realm", this->login.realm);
			}
			this->node = json_get<std::string>(val, "node", this->node);
			this->storage = json_get<std::string>(val, "storage", this->storage);
			this->imagestorage = json_get<std::string>(val, "imagestorage", this->imagestorage);
			this->vmid = json_get<int64_t>(val, "vmid", this->vmid);
			this->image = json_get<std::string>(val, "image", this->image);
			this->start = json_get<bool>(val, "start", this->start);
		}
		
		std::string config::to_json() const {
			picojson::object res;
			{
				picojson::object login;
				login["hostname"] = picojson::value(this->login.hostname);
				login["username"] = picojson::value(this->login.username);
				login["password"] = picojson::value(this->login.password);
				login["realm"] = picojson::value(this->login.realm);
				res["login"] = picojson::value(login);
			}
			res["node"] = picojson::value(this->node);
			res["storage"] = picojson::value(this->storage);
			res["imagestorage"] = picojson::value(this->imagestorage);
			res["vmid"] = picojson::value((int64_t)this->vmid);
			res["image"] = picojson::value(this->image);
			res["start"] = picojson::value(this->start);
			picojson::object result;
			result["deploy"] = picojson::value(res);
			return picojson::value(result).serialize(true);
		}

		void config::reset() {
			this->login = {};
		}

		bool config::ok() const {
			return !this->login.hostname.empty()
				&& !this->login.password.empty()
				&& !this->login.realm.empty()
				&& !this->login.username.empty()
				&& !this->node.empty()
				&& !this->storage.empty()
				&& !this->imagestorage.empty()
				&& !this->image.empty()
				&& this->vmid != 0;
		}
	}
}