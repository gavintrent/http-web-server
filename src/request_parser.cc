#include "request_parser.h"
#include <boost/system/error_code.hpp>
#include <system_error>
#include <string>
#include <sstream>

HttpRequest RequestParser::parse(const char* data, std::size_t len, boost::system::error_code& ec) {
  ec.clear();                   
  std::string raw(data, len);
  HttpRequest req;

  //raw string for echo
  req.raw.assign(data, len);
  // Split head & body
  auto split = raw.find("\r\n\r\n");
  std::string head = (split == std::string::npos ? raw : raw.substr(0, split));
  if (split != std::string::npos) {
    req.body = raw.substr(split + 4);
  }

  // Parse the request‐line: METHOD SP PATH SP VERSION
  std::istringstream lines(head);
  std::string request_line;
  if (!std::getline(lines, request_line) || request_line.empty()) {
    ec = make_error_code(std::errc::protocol_error);
    return {};
  }
  // Strip trailing '\r'
  if (request_line.back() == '\r')
    request_line.pop_back();

  std::istringstream rl(request_line);
  std::string version;
  if (!(rl >> req.method >> req.path >> version)) {
    ec = make_error_code(std::errc::protocol_error);
    return {};
  }

  // Validate HTTP version token: must start with "HTTP/"; everything else is malformed
  if (version.rfind("HTTP/", 0) != 0) {
    ec = make_error_code(std::errc::protocol_error);
    return {};
  }

  // Parse headers: “Name: Value”
  std::string line;
  while (std::getline(lines, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.empty())
      break;  // blank line ends headers

    auto colon = line.find(':');
    if (colon == std::string::npos)
      continue;
    std::string name  = line.substr(0, colon);
    std::string value = line.substr(colon + 1);
    if (!value.empty() && value[0] == ' ')
      value.erase(0,1);
    req.headers[name] = value;
  }

  return req;
}
