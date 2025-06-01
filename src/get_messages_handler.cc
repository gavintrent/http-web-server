#include "get_messages_handler.h"
#include "disk_file_store.h"
#include "handler_registry.h"
#include <nlohmann/json.hpp>
#include <sstream>
#include <filesystem>
#include <mutex>

using json = nlohmann::json;
namespace fs = std::filesystem;
const std::string GetMessagesHandler::kName = "GetMessagesHandler";
std::mutex GetMessagesHandler::store_mtx_;

GetMessagesHandler::GetMessagesHandler(const std::string& path, FileStore* store) : path_(path), store_(store) {}
GetMessagesHandler::~GetMessagesHandler() { delete store_; }

RequestHandler* GetMessagesHandler::Init(const std::string& path, const std::map<std::string, std::string>& args) {
    auto it = args.find("data_path");
    if (it == args.end()) {
        return new GetMessagesHandler(path, nullptr);
    }

    // build a DiskFileStore rooted at "<data_path>/messages"
    std::string base = it->second + "/messages";

    // check if directory exists and is accessible
    fs::path dir_path(base);
    std::error_code ec;
    if (!fs::exists(dir_path, ec)) {
        return new GetMessagesHandler(path, nullptr);
    }

    if (!fs::is_directory(dir_path, ec)) {
        return new GetMessagesHandler(path, nullptr);
    }

    try {
        FileStore* store = new DiskFileStore(base);
        return new GetMessagesHandler(path, store);
    } catch (const std::exception& e) {
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

    json messages = json::array();
    {
        std::lock_guard<std::mutex> lock(store_mtx_);
        for (int file_id : *filenames_opt) {
            auto content_opt = store_->read("", file_id);
            if (!content_opt) {
                continue;
            }
            try {
                json single = json::parse(*content_opt);
                messages.push_back(single);
            } catch (const json::parse_error& e) {
                continue;
            }
        }
    }
    response->status_code = 200;
    response->headers["Content-Type"] = "application/json";
    response->body = messages.dump();
    return response;
}

static const bool getMessagesRegistered =
  HandlerRegistry::instance().registerHandler(
    GetMessagesHandler::kName,
    [](const std::vector<std::string>& args) -> std::unique_ptr<RequestHandler> {

      // build a FileStore rooted at <data_path>/messages
      FileStore* store = new DiskFileStore(args.at(1) + "/messages");
      return std::make_unique<GetMessagesHandler>(args.at(0), store);
    });
