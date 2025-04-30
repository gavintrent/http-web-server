#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include <map>
#include <string>

struct HttpRequest {
  std::string method;
  std::string path;
  std::map<std::string, std::string> headers;
  std::string body;
  std::string raw;
};

struct HttpResponse {
  int status_code;
  std::map<std::string, std::string> headers;
  std::string body;
};
#endif