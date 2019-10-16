#pragma once
#include <sstream>
#include <iomanip>
#include <random>

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

		inline std::string fmt_bytes(uint64_t psize) {
			constexpr const char* const powers[] = {
				"b",
				"KB",
				"MB",
				"GB",
				"TB",
				"PB"	
			};
			double size = psize;
			size_t unit = 0;
			for(; unit<sizeof(powers)/sizeof(const char*); unit++) {
				if(size < 1024) break;
				size /= 1024;
			}
			std::string res = std::to_string(static_cast<uint64_t>(size*10)/10);
			if(static_cast<uint64_t>(size*10)%10 != 0) res += "." + std::to_string(static_cast<uint64_t>(size*10)%10);
			res += " ";
			res += powers[unit];
			return res;
		}

		inline std::string generate_random_mac() {
			std::random_device dev;
			std::mt19937 rng(dev());
			std::uniform_int_distribution<std::mt19937::result_type> dist(0,255);

			std::array<uint8_t, 6> mac;
			for(int i=0; i<6; i++) {
				auto r = dist(rng);
				if(i == 0) r |= 0x02; // Make sure it is a localy administered mac
				mac[i] = r;
			}
			std::string buf;
			buf.resize(18);
			snprintf(const_cast<char*>(buf.data()), buf.size(), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			buf.resize(17);
			return buf;
		}
	}
}