#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include <map>
#include <string>
#include "session_context.h"

struct HttpRequest {
  std::string method;
  std::string path;
  std::map<std::string, std::string> headers;
  std::string body;
  std::string raw;
  std::string client_ip;
  SessionContext session_context;
};

struct HttpResponse {
  int status_code;
  std::map<std::string, std::string> headers;
  std::string body;
};
#endif