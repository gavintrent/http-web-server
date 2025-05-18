#include "handler_registry.h"
#include "disk_file_store.h"
#include "api_handler.h"
#include <nlohmann/json.hpp>

bool ApiHandler::parse_path(const std::string& p,
                            std::string& entity,
                            std::optional<int>& id) {
  // strip mount_ prefix:
  if (p.rfind(mount_, 0) != 0) return false;
  auto rest = p.substr(mount_.size()); // e.g. "/Shoes/3"
  std::regex re(R"(^/([^/]+)(?:/(\d+))?$)");
  std::smatch m;
  if (!std::regex_match(rest, m, re)) return false;
  entity = m[1];
  if (m[2].matched) id = std::stoi(m[2]);
  return true;
}

std::unique_ptr<HttpResponse> ApiHandler::handle_request(const HttpRequest& req) {
  auto res = std::make_unique<HttpResponse>();
  std::string entity;
  std::optional<int> id;
  if (!parse_path(req.path, entity, id)) {
    res->status_code = 404;
    return res;
  }

  // ----- CREATE (POST /api/Entity) -----
  if (req.method == "POST" && !id) {
    int new_id = store_->next_id(entity);
    if (!store_->write(entity, new_id, req.body)) {
      res->status_code = 500;
      return res;
    }
    // return {"id": new_id} as JSON
    res->status_code = 201;
    nlohmann::json j; 
    j["id"] = new_id;
    res->body = j.dump();
    res->headers["Content-Type"] = "application/json";
    res->headers["Content-Length"] = std::to_string(res->body.size());
    return res;
  }

  // ----- RETRIEVE (GET /api/Entity/{id}) -----
  if (req.method == "GET" && id) {
    auto data = store_->read(entity, *id);
    if (!data) {
      res->status_code = 404;
      return res;
    }
    res->status_code = 200;
    res->body = *data;
    res->headers["Content-Type"] = "application/json";
    res->headers["Content-Length"] = std::to_string(res->body.size());
    return res;
  }

  // Unhandled method or missing ID
  res->status_code = 400;
  return res;
}

static bool apiRegistered = HandlerRegistry::instance()
  .registerHandler("ApiHandler",
    [](const std::vector<std::string>& args) {
      // args[0] = mount, args[1] = data_path
      auto store = std::make_shared<DiskFileStore>(args.at(1));
      return std::make_unique<ApiHandler>(args.at(0), store);
    });