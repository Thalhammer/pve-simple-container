#pragma once
#include <string>
#include <vector>
#include <set>
#include <chrono>

namespace picojson { class value; }

namespace pvesc {
	namespace deploy {

		namespace pve {
			struct node {
				std::string node;
				std::string status;
				std::string id;
				std::string ssl_fingerprint;
				// Only valid if status == "online"
				std::chrono::seconds uptime;
				double cpu;
				double maxcpu;
				size_t maxmem;
				size_t mem;
				std::string level;
				size_t disk;
				size_t maxdisk;
			};
			struct task_status {
				std::string type;
				std::string user;
				std::string status;
				std::string upid;
				std::string node;
				time_t starttime;
				std::string id;
				size_t pstart;
				pid_t pid;
				std::string exitstatus;

				bool done() const { return status == "stopped"; }
				bool ok() const { return done() && exitstatus == "OK"; }
			};
		}

		class apiclient {
			std::string hostname;
			std::string authcookie;
			std::string csrftoken;
			std::string username;
			std::set<std::string> capabilities;

			enum class auth_mode {
				none,
				auth,
				//auth_autorenew
			};

			picojson::value json_get(const std::string& url, auth_mode auth);
			picojson::value json_post(const std::string& url, const std::string& data, auth_mode auth);
			picojson::value json_delete(const std::string& url, auth_mode auth);
		public:
			apiclient(std::string host)
				: hostname(std::move(host))
			{}
			~apiclient()
			{}

			void login(const std::string& user, const std::string& pass, const std::string& realm = "pam");
			void logout() {
				authcookie.clear();
				csrftoken.clear();
				username.clear();
				capabilities.clear();
			}
			bool is_logged_in() const noexcept { return !authcookie.empty(); }
			void set_login(std::string ac, std::string csrf, std::string user) {
				authcookie = std::move(ac);
				csrftoken = std::move(csrf);
				username = std::move(user);
			}

			const std::string& get_hostname() const noexcept { return hostname; }
			const std::string& get_authcookie() const noexcept { return authcookie; }
			const std::string& get_csrftoken() const noexcept { return csrftoken; }
			const std::string& get_username() const noexcept { return username; }
			bool has_capability(const std::string& group, const std::string& id) const {
				return capabilities.count(group + "/" + id) != 0;
			}

			// Nodes
			std::vector<pve::node> get_nodes();

			// Fileupload
			std::string upload_file(const std::string& node, const std::string& storage, const std::string& type, const std::string& filename);
			std::string upload_file(const std::string& node, const std::string& storage, const std::string& type, const std::string& filename, std::istream& str);
			void delete_file(const std::string& node, const std::string& storage, const std::string& type, const std::string& filename);

			// Tasks
			pve::task_status get_task_status(const std::string& node, const std::string& upid);
			pve::task_status await_task_done(const std::string& node, const std::string& upid);

			// LXC
			std::string restore_lxc(const std::string& node, const std::string& imagestorage, const std::string& image, size_t vmid, const std::string& storage);
			std::string start_lxc(const std::string& node, size_t vmid);
		};
	}
}