#include "recipe.h"
#define PICOJSON_USE_INT64
#include "../common/picojson.h"
#include <sstream>

namespace psc {
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
			this->name = val.get("name").get<std::string>();
			this->id = val.get("id").get<int64_t>();
			this->memory = val.get("memory").get<int64_t>();
			for(auto& e : val.get("files").get<picojson::array>()) {
				file_t f;
				f.source = e.get("source").get<std::string>();
				f.destination = e.get("destination").get<std::string>();
				this->files.push_back(f);
			}
			{
				auto& net = val.get("network").get<picojson::object>();
				for(auto& e: net["interfaces"].get<picojson::object>()) {
					network_t::interface_t iface;
					iface.id = e.first;
					iface.name = e.second.get("name").get<std::string>();
					iface.bridge = e.second.get("bridge").get<std::string>();
					iface.gateway = e.second.get("gateway").get<std::string>();
					iface.mac = e.second.get("mac").get<std::string>();
					iface.ip = e.second.get("ip").get<std::string>();
					iface.broadcast = e.second.get("broadcast").get<std::string>();
					iface.netmask = e.second.get("netmask").get<int64_t>();
					iface.is_default = e.second.get("is_default").get<bool>();
					this->network.interfaces.push_back(iface);
				}
				for(auto& e : net["nameservers"].get<picojson::array>()) {
					this->network.nameservers.push_back(e.get<std::string>());
				}
			}
			for(auto& e : val.get("mounts").get<picojson::array>()) {
				mount_t m;
				m.type = e.get("type").get<std::string>();
				for(auto& o : e.get("options").get<picojson::array>())
					m.options.insert(o.get<std::string>());
				m.source = e.get("source").get<std::string>();
				m.path = e.get("path").get<std::string>();
				this->mounts.push_back(m);
			}
			for(auto& e : val.get("options").get<picojson::array>()) {
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
			{
				picojson::array files;
				for(auto & f : this->files) {
					picojson::object obj;
					obj["source"] = picojson::value(f.source);
					obj["destination"] = picojson::value(f.destination);
					files.push_back(picojson::value(obj));
				}
				res["files"] = picojson::value(files);
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
			this->files.clear();
			this->network.interfaces.clear();
			this->network.nameservers.clear();
			this->mounts.clear();
			this->options.clear();
		}
	}
}