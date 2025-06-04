
#pragma once
#include "request_handler.h"   // defines RequestHandler
#include "http_types.h"        // defines HttpRequest + HttpResponse
#include <string>
class PostMessageHandler : public RequestHandler {
public:
  static const std::string kName;
  std::string get_kName() override { return kName; }
  
  explicit PostMessageHandler(const std::string& data_path);
  virtual ~PostMessageHandler() = default;
  
  virtual std::unique_ptr<HttpResponse> handle_request(const HttpRequest& request) override;
private:
  std::string messages_dir_;
  static std::string parse_content(const std::string& body);
};
