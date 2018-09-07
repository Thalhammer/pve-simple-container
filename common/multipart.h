#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace pvesc {
	namespace common {
		struct form_part {
			std::string name;
			std::string data;
			std::string content_type;
			std::string filename;
		};

		inline std::string find_multipart_boundary(const std::vector<form_part>& parts)
		{
			static const size_t max_length = 50;

			auto random_string = []( size_t length) -> std::string
			{
				auto randchar = []() -> char
				{
					const char charset[] =
					"0123456789"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz";
					const size_t max_index = (sizeof(charset) - 1);
					return charset[ rand() % max_index ];
				};
				std::string str(length,0);
				std::generate_n( str.begin(), length, randchar );
				return str;
			};

			auto random = random_string(12);
			for(auto& p : parts) {
				auto pos = p.data.find(random);
				while(pos != std::string::npos) {
					if(random.size() == max_length) throw std::runtime_error("failed to find multipart boundary");
					random += random_string(1);
					pos = p.data.find(random, pos);
				}
			}
			return random;
		}

		inline std::string build_multipart(const std::vector<form_part>& parts, const std::string& boundary)
		{
			std::string res;
			for(auto& p : parts) {
				res += "--" + boundary + "\r\n";
				res += "Content-Disposition: form-data; name=\"" + p.name + "\"";
				if(!p.filename.empty()) res += "; filename=\"" + p.filename + "\"";
				res += "\r\n";
				if(!p.content_type.empty()) {
					res += "Content-Type: " + p.content_type + "\r\n";
				}
				res += "\r\n";
				res += p.data;
				res += "\r\n";
			}
			res += "--" + boundary + "--\r\n";
			return res;
		}
	}
}