#include "get_messages_handler.h"
#include "session_middleware_handler.h"
#include "disk_file_store.h"
#include "handler_registry.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <mutex>
#include <algorithm>

using json = nlohmann::json;
namespace fs = std::filesystem;
const std::string GetMessagesHandler::kName = "GetMessagesHandler";
std::mutex GetMessagesHandler::store_mtx_;

GetMessagesHandler::GetMessagesHandler(const std::string& path, FileStore* store) : path_(path), store_(store) {}

GetMessagesHandler::~GetMessagesHandler() { delete store_; }

RequestHandler* GetMessagesHandler::Init(
    const std::string& path,
    const std::map<std::string, std::string>& args)
{
    auto it = args.find("data_path");
    if (it == args.end()) {
        return new GetMessagesHandler(path, nullptr);
    }

    // Build "<data_path>/messages" (e.g. "../data/messages")
    std::string base = it->second + "/messages";

    // Let DiskFileStore create the directory on demand:
    try {
        FileStore* store = new DiskFileStore(base);
        return new GetMessagesHandler(path, store);
    } catch (const std::exception&) {
        return new GetMessagesHandler(path, nullptr);
    }
}

std::unique_ptr<HttpResponse> GetMessagesHandler::handle_request(
    const HttpRequest& request
) {
    // if Init never gave us a valid store_, return 500:
    if (!store_) {
        auto response = std::make_unique<HttpResponse>();
        response->status_code = 500;
        return response;
    }

    auto response = std::make_unique<HttpResponse>();

    // only GET is allowed
    if (request.method != "GET") {
        response->status_code = 405;
        response->headers["Allow"] = "GET";
        return response;
    }

    // path must exactly match path_
    if (request.path != path_) {
        response->status_code = 404;
        return response;
    }

    // enumerate all files under "<data_path>/messages"
    auto filenames_opt = store_->read_directory("");
    if (!filenames_opt) {
        response->status_code = 500;
        return response;
    }

    std::vector<json> messages;
    {
        std::lock_guard<std::mutex> lock(store_mtx_);
        for (int file_id : *filenames_opt) {
            // Try both with and without .json extension
            auto content_opt = store_->read("", file_id);
            if (!content_opt) {
                // Try with .json extension
                std::string filename = std::to_string(file_id) + ".json";
                fs::path file_path = fs::path(store_->get_root()) / filename;
                if (fs::exists(file_path)) {
                    std::ifstream in(file_path);
                    if (in.is_open()) {
                        std::ostringstream ss;
                        ss << in.rdbuf();
                        content_opt = ss.str();
                    }
                }
            }
            
            if (!content_opt) {
                continue;
            }
            
            try {
                json single = json::parse(*content_opt);
                messages.push_back(single);
            } catch (const json::parse_error&) {
                continue;
            }
        }
    }

    // Sort messages by timestamp
    std::sort(messages.begin(), messages.end(), 
        [](const json& a, const json& b) {
            return a["timestamp"].get<std::string>() < b["timestamp"].get<std::string>();
        });

    response->status_code = 200;
    response->headers["Content-Type"] = "application/json";
    response->body = json(messages).dump();
    return response;
}

static const bool getMessagesRegistered =
  HandlerRegistry::instance().registerHandler(
    GetMessagesHandler::kName,
    [](const std::vector<std::string>& args) -> std::unique_ptr<RequestHandler> {
      // build a FileStore rooted at <data_path>/messages
      FileStore* store = new DiskFileStore(args.at(1) + "/messages");
      auto realHandler = std::make_unique<GetMessagesHandler>(args.at(0), store);
      return std::make_unique<SessionMiddlewareHandler>(std::move(realHandler));
    });
