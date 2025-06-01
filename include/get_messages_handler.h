// include/get_messages_handler.h

#ifndef GET_MESSAGES_HANDLER_H
#define GET_MESSAGES_HANDLER_H

#include "request_handler.h"
#include "file_store.h"
#include <memory>
#include <map>
#include <string>
#include <mutex>

class GetMessagesHandler : public RequestHandler {
public:
    static const std::string kName;
    std::string get_kName() override { return kName; }

    GetMessagesHandler(const std::string& path, FileStore* store);

    static RequestHandler* Init(const std::string& path, const std::map<std::string, std::string>& args);

    std::unique_ptr<HttpResponse>
    handle_request(const HttpRequest& request) override;

    ~GetMessagesHandler() override;
private:
    std::string path_;
    FileStore* store_{nullptr};
    static std::mutex store_mtx_;
};

#endif // GET_MESSAGES_HANDLER_H

