#include "recipe.h"
#define PICOJSON_USE_INT64
#include "../common/json_helper.h"
#include "../common/string_helper.h"
#include <sstream>

using pvesc::common::json_get;
using pvesc::common::replace_copy;

namespace pvesc {
	namespace container {
		void recipe::from_json(const std::string& json) {
			std::istringstream ss(json);
			this->from_json(ss);
		}

		void recipe::from_json(std::istream& json) {
			this->reset();
			picojson::value val;
			auto err = picojson::parse(val, json);
			if(!err.empty()) throw std::logic_error("Invalid json");
			val = val.get("container");
			this->name = json_get<std::string>(val,"name");
			this->id = json_get<int64_t>(val,"id");
			this->memory = json_get<int64_t>(val,"memory");
			this->root_size = json_get<int64_t>(val,"root_size", 0);
			this->root_readonly = json_get<bool>(val,"root_readonly", true);
			{
				auto out = json_get<picojson::object>(val,"output", {});
				this->output.filename = json_get<std::string>(out, "filename", this->name + ".tar.gz");
			}
			this->main = json_get<std::string>(val,"main");
			for(auto& e : json_get<picojson::object>(val,"environment", {})) {
				this->environment[e.first] = e.second.get<std::string>();
			}
			for(auto& e : json_get<picojson::array>(val,"files")) {
				file_t f;
				f.source =  json_get<std::string>(e,"source");
				f.destination = json_get<std::string>(e,"destination");
				f.check_dependencies = json_get<bool>(e, "check_dependencies", false);
				this->files.push_back(f);
			}
			for(auto& e: json_get<picojson::array>(val, "overlays", {})) {
				this->overlays.insert(e.get<std::string>());
			}
			{
				auto& net = json_get<picojson::object>(val,"network");
				for(auto& e: json_get<picojson::object>(net,"interfaces")) {
					network_t::interface_t iface;
					iface.id = e.first;
					iface.name = json_get<std::string>(e.second, "name", replace_copy(iface.id, "net", "eth"));
					iface.bridge = json_get<std::string>(e.second, "bridge", "vmbr0");
					iface.gateway = json_get<std::string>(e.second, "gateway"); // TODO: First device in Network segment
					iface.mac = json_get<std::string>(e.second, "mac"); // TODO: Random mac
					iface.ip = json_get<std::string>(e.second, "ip");
					iface.broadcast = json_get<std::string>(e.second, "broadcast"); // TODO: Autogenerate based on IP & netmask
					iface.netmask = json_get<int64_t>(e.second, "netmask", 24);
					iface.is_default = json_get<bool>(e.second, "is_default", e.first == "net0");
					this->network.interfaces.push_back(iface);
				}
				for(auto& e : json_get<picojson::array>(net,"nameservers", {})) {
					this->network.nameservers.push_back(e.get<std::string>());
				}
			}
			for(auto& e : json_get<picojson::array>(val,"mounts", {})) {
				mount_t m;
				m.type = json_get<std::string>(e,"type");
				for(auto& o : json_get<picojson::array>(e,"options", {}))
					m.options.insert(o.get<std::string>());
				m.source =json_get<std::string>(e,"source");
				m.path = json_get<std::string>(e,"path");
				this->mounts.push_back(m);
			}
			for(auto& e : json_get<picojson::array>(val,"options", {})) {
				option_t o;
				o.name = e.get("name").get<std::string>();
				o.value = e.get("value").get<std::string>();
				this->options.push_back(o);
			}
		}
		
		std::string recipe::to_json() const {
			picojson::object res;
			res["name"] = picojson::value(this->name);
			res["id"] = picojson::value((int64_t)this->id);
			res["memory"] = picojson::value((int64_t)this->memory);
			res["root_size"] = picojson::value((int64_t)this->root_size);
			res["root_readonly"] = picojson::value(this->root_readonly);
			{
				picojson::object output;
				output["filename"] = picojson::value(this->output.filename);
				res["output"] = picojson::value(output);
			}
			res["main"] = picojson::value(this->main);
			{
				picojson::object environment;
				for(auto & e : this->environment) {
					environment[e.first] = picojson::value(e.second);
				}
				res["environment"] = picojson::value(environment);
			}
			{
				picojson::array files;
				for(auto & f : this->files) {
					picojson::object obj;
					obj["source"] = picojson::value(f.source);
					obj["destination"] = picojson::value(f.destination);
					obj["check_dependencies"] = picojson::value(f.check_dependencies);
					files.push_back(picojson::value(obj));
				}
				res["files"] = picojson::value(files);
			}
			{
				picojson::array overlays;
				for(auto & f : this->overlays) {
					overlays.push_back(picojson::value(f));
				}
				res["overlays"] = picojson::value(overlays);
			}
			{
				picojson::object network;
				picojson::object interfaces;
				picojson::array nameservers;
				for(auto & i : this->network.interfaces) {
					picojson::object obj;
					obj["name"] = picojson::value(i.name);
					obj["bridge"] = picojson::value(i.bridge);
					obj["gateway"] = picojson::value(i.gateway);
					obj["mac"] = picojson::value(i.mac);
					obj["ip"] = picojson::value(i.ip);
					obj["broadcast"] = picojson::value(i.broadcast);
					obj["netmask"] = picojson::value((int64_t)i.netmask);
					obj["is_default"] = picojson::value(i.is_default);
					interfaces.insert({i.id, picojson::value(obj)});
				}
				for(auto & n : this->network.nameservers)
					nameservers.push_back(picojson::value(n));
				network["interfaces"] = picojson::value(interfaces);
				network["nameservers"] = picojson::value(nameservers);
				res["network"] = picojson::value(network);
			}
			{
				picojson::array mounts;
				for(auto & m : this->mounts) {
					picojson::object obj;
					obj["type"] = picojson::value(m.type);
					picojson::array options;
					for(auto& o : m.options) options.push_back(picojson::value(o));
					obj["options"] = picojson::value(options);
					obj["source"] = picojson::value(m.source);
					obj["path"] = picojson::value(m.path);
					mounts.push_back(picojson::value(obj));
				}
				res["mounts"] = picojson::value(mounts);
			}
			{
				picojson::array options;
				for(auto & o : this->options) {
					picojson::object obj;
					obj["name"] = picojson::value(o.name);
					obj["value"] = picojson::value(o.value);
					options.push_back(picojson::value(obj));
				}
				res["options"] = picojson::value(options);
			}

			return picojson::value(res).serialize(true);
		}

		void recipe::reset() {
			this->name.clear();
			this->id = 0;
			this->memory = 0;
			this->root_size = 0;
			this->root_readonly = 0;
			this->output.filename.clear();
			this->main.clear();
			this->environment.clear();
			this->files.clear();
			this->overlays.clear();
			this->network.interfaces.clear();
			this->network.nameservers.clear();
			this->mounts.clear();
			this->options.clear();
		}

		bool recipe::ok() const {
			return 
				id != 0
				&& !name.empty()
				&& memory != 0
				&& !output.filename.empty()
				&& !main.empty();
		}
	}
}