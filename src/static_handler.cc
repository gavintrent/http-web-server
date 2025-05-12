#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "static_handler.h"

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


HttpResponse StaticHandler::handleRequest(const HttpRequest& req) {
  //setup server response
  HttpResponse res;
  
  //accept only HTTP GET requests and setup response accordingly
  if (req.method == "GET") {
    // get final directory name from root_dir_ (e.g. "static_files")
    std::string base_dir = root_dir_;
    auto pos = base_dir.find_last_of("/\\");
    if (pos != std::string::npos) {
      base_dir = base_dir.substr(pos + 1);
    }
    
    // strip "/static_files" from the front of req.path
    std::string prefix = "/" + base_dir;
    std::string req_path;
    if (req.path.rfind(prefix, 0) == 0) {
      req_path = req.path.substr(prefix.size());
    } else {
      req_path = req.path;
    }
    // make sure we have leading slash
    if (req_path.empty() || req_path[0] != '/') 
      req_path = "/" + req_path;
    
    // actual file to open
    std::string path = root_dir_ + req_path;//map path


    //find mime_type
    std::string mime_type;
    auto ext = path.substr(path.rfind('.'));
    if      (ext == ".html") mime_type = "text/html";
    else if (ext == ".txt" ) mime_type = "text/plain";
    else if (ext == ".jpg" ) mime_type = "image/jpeg";
    else if (ext == ".zip" ) mime_type = "application/zip";
    if (mime_type.empty()) {
        res.status_code = 404;
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
        res.status_code = 404;
      } else {
        res.status_code = 200;
        res.headers["Content-Type"] = mime_type;

        std::stringstream buf;
        buf << file.rdbuf();
        res.body = buf.str();
      }
    }
    
  }
  else {res.status_code = 400;}

  //return HTTP response
  return res;
}