#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#include "http_types.h"
#include <boost/system/error_code.hpp>

class RequestParser {
public:
  // Parse up to `len` bytes in `data`. On success, return a filled Request. On parse error, set `ec`.
  HttpRequest parse(const char* data, std::size_t len, boost::system::error_code& ec);
};
#endif