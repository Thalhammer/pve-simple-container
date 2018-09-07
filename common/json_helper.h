#pragma once
#include "picojson.h"

namespace pvesc {
	namespace common {
		template<typename T>
		inline const T json_get(const picojson::object& obj, const std::string& elem, const T& def) {
			if(obj.count(elem)) return obj.at(elem).get<T>();
			return def;
		}

		template<typename T>
		inline const T& json_get(const picojson::object& obj, const std::string& elem) {
			if(obj.count(elem)) return obj.at(elem).get<T>();
			throw std::runtime_error("Missing required property: " + elem);
		}

		template<typename T>
		inline const T json_get(const picojson::value& val, const std::string& elem, const T& def) {
			if(val.is<picojson::object>()) {
				auto& obj = val.get<picojson::object>();
				if(obj.count(elem)) return obj.at(elem).get<T>();
			}
			return def;
		}

		template<typename T>
		inline const T& json_get(const picojson::value& val, const std::string& elem) {
			if(val.is<picojson::object>()) {
				auto& obj = val.get<picojson::object>();
				if(obj.count(elem)) return obj.at(elem).get<T>();
			}
			throw std::runtime_error("Missing required property: " + elem);
		}
	}
}