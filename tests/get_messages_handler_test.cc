#include "get_messages_handler.h"
#include "gtest/gtest.h"
#include "config_parser.h"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <unistd.h>
#include <map>

using json = nlohmann::json;
namespace fs = std::filesystem;

// helper to build a map containing data_path
static std::map<std::string, std::string> makeConfigWithDataPath(const std::string& dir) {
    std::map<std::string, std::string> config;
    config["data_path"] = dir;
    return config;
}

class GetMessagesHandlerTest : public ::testing::Test {
protected:
    // Root of our temporary directory, e.g. "/tmp/gmhXXXXXX"
    std::string temp_root;
    std::string data_path_str;

    std::unique_ptr<GetMessagesHandler> handler;

    void SetUp() override {
        // make a template for mkdtemp(): it must be writable and end with "XXXXXX"
        char tmpl[] = "/tmp/gmhXXXXXX";
        char* dir_cstr = mkdtemp(tmpl);
        ASSERT_NE(dir_cstr, nullptr) << "mkdtemp failed";

        temp_root = std::string(dir_cstr);
        data_path_str = temp_root;

        // under it, create "messages" subdirectory
        fs::path messages_dir = fs::path(temp_root) / "messages";
        fs::create_directories(messages_dir);

        // create two files: <temp_root>/messages/1.json and /messages/2.json
        {
            std::ofstream((messages_dir / "1.json").string()) << R"({"username":"alice","content":"Hello","timestamp":"2024-01-01T00:00:00Z"})";
            std::ofstream((messages_dir / "2.json").string()) << R"({"username":"bob","content":"World","timestamp":"2024-01-01T00:00:01Z"})";
        }

        // build a config map with data_path
        auto config = makeConfigWithDataPath(data_path_str);

        // create the handler using the static factory
        RequestHandler* raw_handler = GetMessagesHandler::Init("/messages/get", config);
        ASSERT_NE(raw_handler, nullptr) << "GetMessagesHandler::Init failed";
        handler.reset(dynamic_cast<GetMessagesHandler*>(raw_handler));
        ASSERT_NE(handler, nullptr) << "dynamic_cast to GetMessagesHandler* failed";
    }

    void TearDown() override {
        // recursively remove everything under temp_root
        std::error_code ec;
        fs::remove_all(temp_root, ec);
    }
};

TEST_F(GetMessagesHandlerTest, ReturnsAllMessages) {
    HttpRequest req;
    req.method = "GET";
    req.path   = "/messages/get";

    auto resp = handler->handle_request(req);
    ASSERT_NE(resp, nullptr);

    EXPECT_EQ(resp->status_code, 200);
    ASSERT_TRUE(resp->headers.count("Content-Type"));
    EXPECT_EQ(resp->headers["Content-Type"], "application/json");

    // parse the body as JSON array
    json arr = json::parse(resp->body);
    ASSERT_TRUE(arr.is_array());
    EXPECT_EQ(arr.size(), 2u);

    // collect (username, content) pairs in a set for comparison
    std::set<std::pair<std::string, std::string>> seen;
    for (auto& elem : arr) {
        std::string username = elem.at("username").get<std::string>();
        std::string content = elem.at("content").get<std::string>();
        seen.insert({username, content});
    }

    std::set<std::pair<std::string, std::string>> expected = {
        {"alice", "Hello"},
        {"bob", "World"}
    };
    EXPECT_EQ(seen, expected);
}

TEST_F(GetMessagesHandlerTest, NonGetMethodNotAllowed) {
    HttpRequest req;
    req.method = "POST";
    req.path   = "/messages/get";

    auto resp = handler->handle_request(req);
    ASSERT_NE(resp, nullptr);

    EXPECT_EQ(resp->status_code, 405);
    ASSERT_TRUE(resp->headers.count("Allow"));
    EXPECT_EQ(resp->headers["Allow"], "GET");
}

TEST_F(GetMessagesHandlerTest, WrongPathReturns404) {
    HttpRequest req;
    req.method = "GET";
    req.path   = "/not-messages";  // anything other than "/messages/get"

    auto resp = handler->handle_request(req);
    ASSERT_NE(resp, nullptr);

    EXPECT_EQ(resp->status_code, 404);
}

TEST(GetMessagesHandlerNoInitTest, NoDataPathConfigsLeadsTo500) {
    std::map<std::string, std::string> empty_config;
    RequestHandler* raw_handler = GetMessagesHandler::Init("/messages/get", empty_config);
    ASSERT_NE(raw_handler, nullptr) << "GetMessagesHandler::Init failed";
    auto handler = std::unique_ptr<GetMessagesHandler>(dynamic_cast<GetMessagesHandler*>(raw_handler));
    ASSERT_NE(handler, nullptr) << "dynamic_cast to GetMessagesHandler* failed";

    HttpRequest req;
    req.method = "GET";
    req.path   = "/messages/get";

    auto resp = handler->handle_request(req);
    ASSERT_NE(resp, nullptr);
    EXPECT_EQ(resp->status_code, 500);
}
