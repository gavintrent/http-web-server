#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "static_handler.h"
#include "handler_registry.h"

const std::string StaticHandler::kName = "StaticHandler";

StaticHandler::StaticHandler(const std::string& path, const std::string& root_dir)
  : path_(path), root_dir_(root_dir) {}

RequestHandler* StaticHandler::Create(const std::string& path, const std::map<std::string, std::string>& args) {
  auto it = args.find("root");
  if (it == args.end()) return nullptr;
  return new StaticHandler(path, it->second);
}

// for once registry is implemented:
//static bool registered_static = Registry::RegisterHandler(StaticHandler::kName, StaticHandler::Create);


std::unique_ptr<HttpResponse> StaticHandler::handle_request(const HttpRequest& req) {
  //setup server response
  auto res = std::make_unique<HttpResponse>();
  
  //accept only HTTP GET requests and setup response accordingly
  if (req.method == "GET") {
    std::string path = req.path;
    path.replace(0, path_.length(), root_dir_); //replace url prefix with root

    //find mime_type
    std::string mime_type;
    auto ext_pos = path.rfind('.');
    if (ext_pos == std::string::npos) { 
        res->status_code = 404;
        return res;
    }
    auto ext = path.substr(ext_pos);
    if      (ext == ".html") mime_type = "text/html";
    else if (ext == ".txt" ) mime_type = "text/plain";
    else if (ext == ".jpg" ) mime_type = "image/jpeg";
    else if (ext == ".zip" ) mime_type = "application/zip";
    if (mime_type.empty()) {
        res->status_code = 404;
    } else {
      // else try open file at configured path
      std::string file_path = path;
      std::ifstream file(file_path, std::ios::in | std::ios::binary);
      // If root_dir_ is absolute, also try sibling static_files from build directory
      if (!file && !root_dir_.empty() && root_dir_[0] == '/') {
          std::string alt_path = ".." + file_path;
          file.open(alt_path, std::ios::in | std::ios::binary);
          if (file) {
              file_path = alt_path;
          }
      }
      if (!file) {
        res->status_code = 404;
      } else {
        res->status_code = 200;
        res->headers["Content-Type"] = mime_type;

        std::stringstream buf;
        buf << file.rdbuf();
        res->body = buf.str();
      }
    }
    
  }
  else {res->status_code = 400;}

  //return HTTP response
  return res;
}

static const bool staticRegistered =
  HandlerRegistry::instance()
    .registerHandler(
      StaticHandler::kName,       
      [](auto const& args) {     
        return std::make_unique<StaticHandler>(
          args.at(0), args.at(1)
        );
      }
    );