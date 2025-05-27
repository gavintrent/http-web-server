#include "handler_registry.h"
#include "disk_file_store.h"
#include "api_handler.h"
#include <nlohmann/json.hpp>
#include <regex>
#include <vector>
#include <mutex>

const std::string ApiHandler::kName = "ApiHandler";

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
    std::lock_guard<std::mutex> lock(store_mutex_);
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
    std::lock_guard<std::mutex> lock(store_mutex_);
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

  // ----- LIST (GET /api/Entity) -----
  if (req.method == "GET" && !id) {
    std::lock_guard<std::mutex> lock(store_mutex_);
    auto filenames = store_->read_directory(entity);
    if (!filenames) {
      res->status_code = 404;
      return res;
    }
    nlohmann::json j;
    j["id"] = *filenames;
    res->body = j.dump();
    res->status_code = 200;
    res->headers["Content-Type"] = "application/json";
    res->headers["Content-Length"] = std::to_string(res->body.size());
    return res;
  }

  // ----- UPDATE (PUT /api/Entity/{id}) -----
  if (req.method == "PUT" && id) {
    // Validate JSON format
    try {
      // Parse the JSON to verify it's valid (but store the original string)
      nlohmann::json::parse(req.body);
    } catch (const nlohmann::json::parse_error& e) {
      // Return bad request if JSON is invalid
      res->status_code = 400; // Bad Request
      nlohmann::json j;
      j["error"] = "Invalid JSON format";
      res->body = j.dump();
      res->headers["Content-Type"] = "application/json";
      res->headers["Content-Length"] = std::to_string(res->body.size());
      return res;
    }

    std::lock_guard<std::mutex> lock(store_mutex_);
    // Write data to storage
    if (!store_->write(entity, *id, req.body)) {
      res->status_code = 500; // Internal Server Error
      nlohmann::json j;
      j["error"] = "Failed to write data";
      res->body = j.dump();
      res->headers["Content-Type"] = "application/json";
      res->headers["Content-Length"] = std::to_string(res->body.size());
      return res;
    }

    // Return success response
    res->status_code = 200; // OK
    nlohmann::json j;
    j["success"] = true;
    j["id"] = *id;
    res->body = j.dump();
    res->headers["Content-Type"] = "application/json";
    res->headers["Content-Length"] = std::to_string(res->body.size());
    return res;
  }

  // ----- DELETE (DELETE /api/Entity/{id}) -----
  if (req.method == "DELETE" && id) {
    std::lock_guard<std::mutex> lock(store_mutex_);
    // First check if the entity exists
    auto data = store_->read(entity, *id);
    if (!data) {
      res->status_code = 404; // Not Found
      nlohmann::json j;
      j["error"] = "Entity not found";
      res->body = j.dump();
      res->headers["Content-Type"] = "application/json";
      res->headers["Content-Length"] = std::to_string(res->body.size());
      return res;
    }

    // Delete the entity
    if (!store_->remove(entity, *id)) {
      res->status_code = 500; // Internal Server Error
      nlohmann::json j;
      j["error"] = "Failed to delete entity";
      res->body = j.dump();
      res->headers["Content-Type"] = "application/json";
      res->headers["Content-Length"] = std::to_string(res->body.size());
      return res;
    }

    // Return success response
    res->status_code = 200; // OK
    nlohmann::json j;
    j["success"] = true;
    res->body = j.dump();
    res->headers["Content-Type"] = "application/json";
    res->headers["Content-Length"] = std::to_string(res->body.size());
    return res;
  }

  // Unhandled method or missing ID
  res->status_code = 400;
  return res;
}

static bool apiRegistered = HandlerRegistry::instance()
  .registerHandler(ApiHandler::kName,
    [](const std::vector<std::string>& args) {
      // args[0] = mount, args[1] = data_path
      auto store = std::make_shared<DiskFileStore>(args.at(1));
      return std::make_unique<ApiHandler>(args.at(0), store);
    });