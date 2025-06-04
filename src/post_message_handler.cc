#include "post_message_handler.h"
#include "session_middleware_handler.h"
#include "handler_registry.h"
#include "message_store.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <mutex>
#include <sys/stat.h>         
#include <sys/types.h>       

using json = nlohmann::json;
namespace fs = std::filesystem;
const std::string PostMessageHandler::kName = "PostMessageHandler";

PostMessageHandler::PostMessageHandler(const std::string& data_path) {
  // Use the same data directory structure as login
  fs::path abs_path;
  if (data_path.empty()) {
    abs_path = "data/messages";
  } else {
    // The data_path from config is "../data", so we need to resolve it
    fs::path exe_path = fs::current_path();
    abs_path = exe_path.parent_path() / "data" / "messages";
  }
  
  messages_dir_ = abs_path.string();

  // Create directory if it doesn't exist
  std::error_code ec;
  if (!fs::exists(messages_dir_, ec)) {
    fs::create_directories(messages_dir_, ec);
  }
}

std::unique_ptr<HttpResponse> PostMessageHandler::handle_request(
    const HttpRequest& request) {
  auto resp = std::make_unique<HttpResponse>();
  // Only accept POST
  if (request.method != "POST") {
    resp->status_code = 405;  // Method Not Allowed
    resp->headers["Allow"] = "POST";
    resp->body = "Only POST is allowed on /messages/post\n";
    return resp;
  }
  // Check valid user_id
  const auto& session = request.session_context;
  if (!session.user_id.has_value()) {
    resp->status_code = 401; // Unauthorized
    resp->body = "User not authenticated\n";
    return resp;
  }
  const std::string username = session.user_id.value();
  
  // Get content
  std::string content = parse_content(request.body);
  if (content.empty()) {
    resp->status_code = 400; // Bad Request
    resp->body = "Expected JSON { \"content\": \"<message>\" }\n";
    return resp;
  }
  
  // Add message and persist
  auto& store = MessageStore::instance();
  store.add(username, content);
  store.persist_to_file(messages_dir_);
  
  resp->status_code = 201; // Created
  resp->body = "Message stored\n";
  return resp;
}

std::string PostMessageHandler::parse_content(const std::string& body) {
  if (body.empty()) {
    return "";
  }
  try {
    auto obj = json::parse(body);
    if (!obj.is_object() || !obj.contains("content") || !obj["content"].is_string()) {
      return "";
    }
    return obj["content"].get<std::string>();
  } catch (const json::parse_error&) {
    return "";
  }
}

static const bool postMessageRegistered =
  HandlerRegistry::instance().registerHandler(
    PostMessageHandler::kName,
    [](const std::vector<std::string>& args) -> std::unique_ptr<RequestHandler> {
      auto realHandler = std::make_unique<PostMessageHandler>(args.at(0));
      return std::make_unique<SessionMiddlewareHandler>(std::move(realHandler));
    });