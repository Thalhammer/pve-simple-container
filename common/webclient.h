#pragma once
#include "cookie.h"
#include <map>

namespace pvesc {
	namespace common {
		struct response {
			std::string data;
			cookie_list cookies;
			std::multimap<std::string, std::string> headers;
			long status_code;
			std::string status_line;
		};

		struct request {
			std::string url;
			std::string method;
			std::string data;
			cookie_list cookies;
			std::multimap<std::string, std::string> headers;
			bool follow_redirect;

			static request default_get(std::string url);
			static request default_post(std::string url, std::string data);
		};

		class webclient {
			void* curl;
		public:
			webclient();
			~webclient();

			webclient(const webclient&) = delete;
			webclient& operator=(const webclient&) = delete;

			void set_verbose(bool v);

			response execute(const request& req);
		};
	}
}