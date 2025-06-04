#include "post_message_handler.h"
#include "handler_registry.h"
#include "session_middleware_handler.h"
#include "message_store.h"
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <sys/types.h>

using json = nlohmann::json;

const std::string PostMessageHandler::kName = "PostMessageHandler";

PostMessageHandler::PostMessageHandler(const std::string& data_path) {
  // Build "<data_path>/messages"
  if (data_path.empty()) {
    messages_dir_ = "messages";
  } else if (data_path.back() == '/' || data_path.back() == '\\') {
    messages_dir_ = data_path + "messages";
  } else {
    messages_dir_ = data_path + "/messages";
  }

  struct stat st;
  if (stat(messages_dir_.c_str(), &st) != 0) {
    mkdir(messages_dir_.c_str(), 0755);
  }
}

std::unique_ptr<HttpResponse> PostMessageHandler::handle_request(
    const HttpRequest& request) {
  auto resp = std::make_unique<HttpResponse>();

  // Only accept POST
  if (request.method != "POST") {
    resp->status_code = 405;  // Method Not Allowed
    resp->headers["Allow"] = "POST";
    resp->body = "Only POST is allowed on /submit\n";
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
        [](const auto& args) {
            const std::string& data_path = args.at(1);
            auto realPost = std::make_unique<PostMessageHandler>(data_path);
            return std::make_unique<SessionMiddlewareHandler>(std::move(realPost));
        }
    );