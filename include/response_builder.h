#ifndef RESPONSE_BUILDER_H
#define RESPONSE_BUILDER_H
#include "http_types.h"

// Builds an HttpResponse given a request or other inputs.
class ResponseBuilder {
public:
  // Simple echo: return 200+body for GET, 400 otherwise.
  static HttpResponse echo(const HttpRequest& req);

  // helpers?
};
#endif
