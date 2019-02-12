#pragma once
#include <string>
#include <map>
#include <set>
#include <vector>
#include "../common/version.h"

namespace pvesc {
	namespace container {
		struct overlay {
			common::version version;
			std::string description;
			std::map<std::string, common::version_check> depends;
			std::string image;

			void from_json(const std::string& json);
			std::string to_json() const;

			void reset();

			bool find_overlay_by_name(const std::string& overlay);
			static void load_dependencies(std::set<std::string>& deps);
			static void load_dependencies(std::map<std::string, std::vector<common::version_check>>& deps);
		};
	}
}