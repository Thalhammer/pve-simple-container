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
			this->force = json_get<bool>(val, "force", this->force);
		}
		
		std::string config::to_json() const {
			picojson::value result;
			this->store_config(result);
			return picojson::value(result).serialize(true);
		}

		std::string config::get_global_config() const {
			picojson::value result;
			this->store_global_config(result);
			return result.serialize(true);
		}

		void config::store_config(picojson::value& val) const {
			this->store_global_config(val);
			this->store_local_config(val);
		}

		void config::store_global_config(picojson::value& val) const {
			if(!val.is<picojson::object>()) val = picojson::value(picojson::object());
			auto& obj = val.get<picojson::object>();
			picojson::object res;
			if(obj.count("deploy") && obj["deploy"].is<picojson::object>())
				res = obj["deploy"].get<picojson::object>();
			{
				picojson::object login;
				if(res.count("login") && res["login"].is<picojson::object>())
					login = res["login"].get<picojson::object>();
				login["hostname"] = picojson::value(this->login.hostname);
				login["username"] = picojson::value(this->login.username);
				login["password"] = picojson::value(this->login.password);
				login["realm"] = picojson::value(this->login.realm);
				res["login"] = picojson::value(login);
			}
			res["node"] = picojson::value(this->node);
			res["storage"] = picojson::value(this->storage);
			res["imagestorage"] = picojson::value(this->imagestorage);
			obj["deploy"] = picojson::value(res);
		}

		void config::store_local_config(picojson::value& val) const {
			if(!val.is<picojson::object>()) val = picojson::value(picojson::object());
			auto& obj = val.get<picojson::object>();
			picojson::object res;
			if(obj.count("deploy") && obj["deploy"].is<picojson::object>())
				res = obj["deploy"].get<picojson::object>();
			
			res["vmid"] = picojson::value((int64_t)this->vmid);
			res["image"] = picojson::value(this->image);
			res["start"] = picojson::value(this->start);
			res["force"] = picojson::value(this->force);

			obj["deploy"] = picojson::value(res);
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