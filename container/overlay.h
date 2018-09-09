#pragma once
#include <string>
#include <map>
#include <set>

namespace pvesc {
	namespace container {
		struct version_t {
			std::string string;
		};
		inline bool operator<(const version_t& a, const version_t& b) {
			return a.string < b.string;
		}

		struct overlay {
			version_t version;
			std::string description;
			std::map<std::string, std::string> depends;
			std::string image;

			void from_json(const std::string& json);
			std::string to_json() const;

			void reset();

			bool find_overlay_by_name(const std::string& overlay);
			static void load_dependencies(std::set<std::string>& deps);
		};
	}
}