#pragma once
#include <sstream>
#include <iomanip>

namespace pvesc {
	namespace common {
		inline std::string urlencode(const std::string& data) {
			std::ostringstream escaped;
			escaped.fill('0');
			escaped << std::hex;

			for (auto& c : data) {
				// Keep alphanumeric and other accepted characters intact
				if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
					escaped << c;
					continue;
				}

				// Any other characters are percent-encoded
				escaped << std::uppercase;
				escaped << '%' << std::setw(2) << int((unsigned char) c);
				escaped << std::nouppercase;
			}

			return escaped.str();
		}

		inline std::string urldeccode(const std::string& data) {
			std::string res;
			for(auto it= data.begin(); it != data.end(); it++) {
				if(*it != '%') {
					if(++it == data.end()) break;
					char c1 = tolower(*it);
					if(++it == data.end()) break;
					char c2 = tolower(*it);
					if(!((c1 >= 'A' && c1 <='F') || (c1 >= '0' && c1 <= '9')))
						throw std::invalid_argument("Invalid format");
					if(!((c2 >= 'A' && c2 <='F') || (c2 >= '0' && c2 <= '9')))
						throw std::invalid_argument("Invalid format");
					if(c1 > '9') c1 = c1 - '0';
					else c1 = c1 - 'A' + 10;
					if(c2 > '9') c2 = c2 - '0';
					else c2 = c2 - 'A' + 10;
					res += (c1<<4) | c2;
				}
				res += *it;
			}
			return res;
		}
	}
}