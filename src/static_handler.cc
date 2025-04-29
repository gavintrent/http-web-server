#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "static_handler.h"

HttpResponse StaticHandler::handleRequest(const HttpRequest& req) {
  //setup server response
  HttpResponse res;
  
  //accept only HTTP GET requests and setup response accordingly
  if (req.method == "GET") {
    std::string path = ".." + req.path; //map path

    //find mime_type
    std::string mime_type = "";
    if (path.rfind(".html") != std::string::npos) {mime_type = "text/html";}
    if (path.rfind(".txt") != std::string::npos) {mime_type = "text/plain";}
    if (path.rfind(".jpg") != std::string::npos) {mime_type = "image/jpeg";}
    if (path.rfind(".zip") != std::string::npos) {mime_type = "application/zip";}
    if (mime_type == "") {
        res.status_code = 404;
    }
    else {
      std::ifstream file(path, std::ios::in | std::ios::binary);

      //check if failed to open file
      if (!file) {res.status_code = 404;}
      else {
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
