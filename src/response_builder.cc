#include "response_builder.h"

HttpResponse ResponseBuilder::echo(const HttpRequest& req) {
  HttpResponse res;
  if (req.method == "GET") {
    res.status_code = 200;
    res.body = req.body;
    res.headers["Content-Type"] = "text/plain";
  } else {
    res.status_code = 400;
    res.body = "Bad Request";
  }
  res.headers["Content-Length"] = std::to_string(res.body.size());
  return res;
}
