#include "overlay.h"
#include "../common/json_helper.h"
#include "../common/filesystem.h"

namespace pvesc {
	namespace container {
		void overlay::from_json(const std::string& json) {
			using common::json_get;
			picojson::value val;
			auto err = picojson::parse(val, json);
			if(!err.empty()) throw std::runtime_error("failed to parse json: " + err);
			this->description = json_get(val, "description", this->description);
			this->version = common::version(json_get(val, "version", this->version.to_string()));
			for(auto& d : json_get<picojson::object>(val, "depends", {})) {
				this->depends[d.first] = common::version_check::parse(d.second.get<std::string>());
			}
			this->image = json_get(val, "image", this->image);
		}

		std::string overlay::to_json() const {
			picojson::object obj;
			obj["description"] = picojson::value(this->description);
			obj["version"] = picojson::value(this->version.to_string());
			picojson::object depends;
			for(auto& d : this->depends) depends[d.first] = picojson::value(d.second.to_string());
			obj["depends"] = picojson::value(depends);
			obj["image"] = picojson::value(this->image);
			return picojson::value(obj).serialize(true);
		}

		void overlay::reset() {
			this->depends.clear();
			this->description.clear();
			this->version = common::version();
		}

		bool overlay::find_overlay_by_name(const std::string& overlay) {
			auto homedir = common::filesystem::get_home_directory();
			std::string paths[] = {
				overlay + ".json",
				"overlays/" + overlay + ".json",
				homedir + "/.pvesc/overlays/" + overlay + ".json",
				"/usr/share/pve-simple-container/overlays/" + overlay + ".json"
			};
			for(auto& p : paths) {
				std::string json;
				if(common::filesystem::try_read_file(p, json)) {
					this->from_json(json);
					if(this->image.empty()) this->image = p.substr(0, p.size() - 4) + "tar.gz";
					return true;
				}
			}
			return false;
		}

		void overlay::load_dependencies(std::set<std::string>& deps) {
			std::map<std::string, std::vector<common::version_check>> checks;
			for(auto& d : deps) checks[d] = { common::version_check{common::version_check::any, common::version("0")} };
			load_dependencies(checks);
			for(auto& d : checks) deps.insert(d.first);
		}

		void overlay::load_dependencies(std::map<std::string, std::vector<common::version_check>>& deps) {
			bool recheck = false;
			std::set<std::string> checked;
			do {
				recheck = false;
				auto temp = deps;
				for(auto& d : temp) {
					if(checked.count(d.first) != 0) continue;
					overlay o;
					if(!o.find_overlay_by_name(d.first)) throw std::runtime_error("could not find overlay:" + d.first);
					for(auto& check : d.second)
						if(!check(o.version)) throw std::runtime_error("overlay " + d.first + " (" + o.version.to_string() + ") does not satisfy version selector " + check.to_string());
					for(auto& nd : o.depends) {
						if(deps.count(nd.first) == 0) {
							deps[nd.first].push_back(nd.second);
							recheck = true;
						} else deps[nd.first].push_back(nd.second);
					}
				}
			} while(recheck);
		}
	}
}