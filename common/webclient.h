#pragma once
#include "cookie.h"
#include <map>
#include <functional>

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

		struct progress_info {
			uint64_t download_total;
			uint64_t download_done;
			uint64_t upload_total;
			uint64_t upload_done;

			double get_download_percent() const noexcept { return static_cast<double>(download_done) / static_cast<double>(download_total); }
			double get_upload_percent() const noexcept { return static_cast<double>(upload_done) / static_cast<double>(upload_total); }
		};

		class webclient {
			void* curl;
			std::function<void(const progress_info&)> progress_cb;
		public:
			struct exception : std::runtime_error {
				exception(int error);
				int get_curl_code() const noexcept { return m_code; }
			private:
				int m_code;
			};
			webclient();
			~webclient();

			webclient(const webclient&) = delete;
			webclient& operator=(const webclient&) = delete;

			void set_verbose(bool v);
			void set_ignore_ssl(bool v);
			void set_progress_cb(std::function<void(const progress_info&)> cb) { progress_cb = cb; }

			response execute(const request& req);
		};
	}
}