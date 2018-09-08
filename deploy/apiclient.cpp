#include "apiclient.h"
#define PICOJSON_USE_INT64
#include "../common/webclient.h"
#include "../common/utils.h"
#include "../common/picojson.h"
#include "../common/multipart.h"
#include "../common/filesystem.h"
#include "../common/string_helper.h"
#include <thread>
#include <fstream>

namespace pvesc {
	namespace deploy {
		picojson::value apiclient::json_get(const std::string& url, auth_mode auth) {
			common::webclient wc;
			wc.set_verbose(false);
			auto req = common::request::default_get(hostname + url);
			if(auth == auth_mode::auth) {
				req.headers.insert({"Cookie", "PVEAuthCookie=" + common::urlencode(this->authcookie)});
			}
			auto res = wc.execute(req);
			if(res.status_code != 200) throw std::runtime_error("Request failed, status code: " + std::to_string(res.status_code));
			picojson::value val;
			auto err = picojson::parse(val, res.data);
			if(!err.empty()) throw std::runtime_error("Request failed, could not parse json:" + err);
			return val.get("data");
		}

		picojson::value apiclient::json_post(const std::string& url, const std::string& data, auth_mode auth) {
			common::webclient wc;
			wc.set_verbose(false);
			auto req = common::request::default_post(hostname + url, data);
			if(auth == auth_mode::auth) {
				req.headers.insert({"Cookie", "PVEAuthCookie=" + common::urlencode(this->authcookie)});
				req.headers.insert({"CSRFPreventionToken", this->csrftoken});
			}
			auto res = wc.execute(req);
			if(res.status_code != 200) throw std::runtime_error("Request failed, status code: " + std::to_string(res.status_code));
			picojson::value val;
			auto err = picojson::parse(val, res.data);
			if(!err.empty()) throw std::runtime_error("Request failed, could not parse json:" + err);
			return val.get("data");
		}

		picojson::value apiclient::json_delete(const std::string& url, auth_mode auth) {
			common::webclient wc;
			wc.set_verbose(false);
			auto req = common::request::default_get(hostname + url);
			req.method = "DELETE";
			if(auth == auth_mode::auth) {
				req.headers.insert({"Cookie", "PVEAuthCookie=" + common::urlencode(this->authcookie)});
				req.headers.insert({"CSRFPreventionToken", this->csrftoken});
			}
			auto res = wc.execute(req);
			if(res.status_code != 200) throw std::runtime_error("Request failed, status code: " + std::to_string(res.status_code));
			picojson::value val;
			auto err = picojson::parse(val, res.data);
			if(!err.empty()) throw std::runtime_error("Request failed, could not parse json:" + err);
			return val.get("data");
		}

		void apiclient::login(const std::string& user, const std::string& pass, const std::string& realm)
		{
			auto data = this->json_post("/api2/json/access/ticket",
							"username=" + common::urlencode(user) + "&password=" + common::urlencode(pass) + "&realm=" + common::urlencode(realm), auth_mode::none);
			this->username = data.get("username").get<std::string>();
			this->authcookie = data.get("ticket").get<std::string>();
			this->csrftoken = data.get("CSRFPreventionToken").get<std::string>();
			for(auto& g : data.get("cap").get<picojson::object>()) {
				for(auto& e : g.second.get<picojson::object>()) {
					if(e.second.get<int64_t>() == 1) {
						this->capabilities.insert(g.first + "/" + e.first);
					}
				}
			}
		}

		std::vector<pve::node> apiclient::get_nodes()
		{
			auto data = this->json_get("/api2/json/nodes", auth_mode::auth);
			std::vector<pve::node> result;
			for(auto& e : data.get<picojson::array>()) {
				pve::node n;
				n.node = e.get("node").get<std::string>();
				n.status = e.get("status").get<std::string>();
				n.id = e.get("id").get<std::string>();
				n.ssl_fingerprint = e.get("ssl_fingerprint").get<std::string>();
				if(n.status == "online") {
					n.uptime = std::chrono::seconds(e.get("uptime").get<int64_t>());
					n.cpu = e.get("cpu").get<double>();
					n.maxcpu = e.get("maxcpu").get<double>();
					n.maxmem = e.get("maxmem").get<int64_t>();
					n.mem = e.get("mem").get<int64_t>();
					n.level = e.get("level").get<std::string>();
					n.disk = e.get("disk").get<int64_t>();
					n.maxdisk = e.get("maxdisk").get<int64_t>();
				}
				result.push_back(n);
			}
			return result;
		}

		std::vector<pve::storage> apiclient::get_storages(const std::string& node)
		{
			auto data = this->json_get("/api2/json/nodes/" + node + "/storage", auth_mode::auth);
			std::vector<pve::storage> result;
			for(auto& e : data.get<picojson::array>()) {
				pve::storage s;
				s.active = e.get("active").get<int64_t>() != 0;
				s.avail = e.get("avail").get<int64_t>();
				for(auto& c : common::split(e.get("content").get<std::string>(), ",")) s.content.insert(c);
				s.enabled = e.get("enabled").get<int64_t>() != 0;
				s.shared = e.get("shared").get<int64_t>() != 0;
				s.storage = e.get("storage").get<std::string>();
				s.total = e.get("total").get<int64_t>();
				s.type = e.get("type").get<std::string>();
				s.used = e.get("used").get<int64_t>();
				result.push_back(s);
			}
			return result;
		}

		std::string apiclient::upload_file(const std::string& node, const std::string& storage, const std::string& type, const std::string& filename)
		{
			std::ifstream stream(filename, std::ios::binary);
			if(!stream) throw std::runtime_error("Failed to open file");
			return upload_file(node, storage, type, filename, stream);
		}

		std::string apiclient::upload_file(const std::string& node, const std::string& storage, const std::string& type, const std::string& filename, std::istream& stream)
		{
			std::vector<common::form_part> parts;
			{
				common::form_part p;
				p.name = "content";
				p.data = type;
				parts.push_back(p);
			}
			{
				common::form_part p;
				p.name = "filename";
				p.filename = filename;
				p.data = common::filesystem::read_stream(stream);
				p.content_type = "application/octet-stream";
				parts.push_back(std::move(p));
			}

			common::webclient wc;
			std::string boundary = common::find_multipart_boundary(parts);
			auto req = common::request::default_post(hostname + "/api2/json/nodes/" + node + "/storage/" + storage + "/upload",
													common::build_multipart(parts, boundary));
			req.headers.insert({"Cookie", "PVEAuthCookie=" + this->authcookie});
			req.headers.insert({"Content-Type", "multipart/form-data; boundary=" + boundary });
			auto res = wc.execute(req);
			if(res.status_code != 200) throw std::runtime_error("Request failed, status code: " + std::to_string(res.status_code));
			picojson::value val;
			auto err = picojson::parse(val, res.data);
			if(!err.empty()) throw std::runtime_error("Request failed, could not parse json:" + err);
			return val.get("data").get<std::string>();
		}

		void apiclient::delete_file(const std::string& node, const std::string& storage, const std::string& type, const std::string& filename) {
			this->json_delete("/api2/json/nodes/" + node + "/storage/" + storage + "/content//" + storage + ":" + type + "/" + filename, auth_mode::auth);
		}

		pve::task_status apiclient::get_task_status(const std::string& node, const std::string& upid)
		{
			auto data = this->json_get("/api2/json/nodes/" + node + "/tasks/" + upid + "/status", auth_mode::auth);
			pve::task_status t;
			t.node = data.get("node").get<std::string>();
			t.user = data.get("user").get<std::string>();
			t.id = data.get("id").get<std::string>();
			t.status = data.get("status").get<std::string>();
			t.upid = data.get("upid").get<std::string>();
			t.type = data.get("type").get<std::string>();
			t.starttime = data.get("starttime").get<int64_t>();
			t.pstart = data.get("pstart").get<int64_t>();
			t.pid = data.get("pid").get<int64_t>();
			if(t.status == "stopped") {
				t.exitstatus = data.get("exitstatus").get<std::string>();
			}
			return t;
		}

		pve::task_status apiclient::await_task_done(const std::string& node, const std::string& upid)
		{
			auto info = this->get_task_status(node, upid);
			while(!info.done()) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				info = this->get_task_status(node, upid);
			}
			return info;
		}

		std::string apiclient::restore_lxc(const std::string& node, const std::string& imagestorage, const std::string& image, size_t vmid, const std::string& storage)
		{
			auto info = this->json_post("/api2/json/nodes/" + node + "/lxc",
						"ostemplate=" + common::urlencode(imagestorage + ":vztmpl/" + image) + "&vmid=" + std::to_string(vmid) + "&restore=1&storage=" + storage, auth_mode::auth);
			return info.get<std::string>();
		}

		std::string apiclient::start_lxc(const std::string& node, size_t vmid)
		{
			auto info = this->json_post("/api2/json/nodes/" + node + "/lxc/" + std::to_string(vmid) + "/status/start", "", auth_mode::auth);
			return info.get<std::string>();
		}
	}
}