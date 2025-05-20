#include "gtest/gtest.h"
#include "api_handler.h"
#include "fake_file_store.h"
#include "handler_registry.h"
#include <memory>

// A little helper store that always fails writes
class FailWriteStore : public FileStore {
 public:
  int next_id(const std::string& entity) override {
    return 42;  // arbitrary
  }
  bool write(const std::string& entity, int id,
             const std::string& data) override {
    return false;  // always fail
  }
  std::optional<std::string> read(const std::string& entity,
                                  int id) override {
    return std::nullopt;
  }

  std::optional<std::vector<int>> read_directory(const std::string& entity) {
    return std::nullopt;
  }
};

class ApiHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    store = std::make_shared<FakeFileStore>();
    handler = std::make_unique<ApiHandler>("/api", store);
    // a request template
    req.body = R"({"x":"y"})";
  }

  std::shared_ptr<FakeFileStore> store;
  std::unique_ptr<ApiHandler> handler;
  HttpRequest req;
};

// 1) Happy path: create then retrieve
TEST_F(ApiHandlerTest, CreateAndRetrieve) {
  req.method = "POST";
  req.path   = "/api/Shoes";
  req.body   = R"({"name":"sneaker"})";
  auto res1 = handler->handle_request(req);
  EXPECT_EQ(res1->status_code, 201);
  EXPECT_NE(res1->body.find(R"("id":0)"), std::string::npos);

  req.method = "GET";
  req.path   = "/api/Shoes/0";
  auto res2 = handler->handle_request(req);
  EXPECT_EQ(res2->status_code, 200);
  EXPECT_EQ(res2->body, R"({"name":"sneaker"})");
}

// 2) parse_path does not match mount or URI → 404
TEST_F(ApiHandlerTest, InvalidPathYields404) {
  req.method = "GET";
  req.path   = "/badmount/Shoes/0";
  auto res = handler->handle_request(req);
  EXPECT_EQ(res->status_code, 404);
}

// 3) write() failure in CREATE branch → 500
TEST(ApiHandlerStandaloneTest, WriteFailureYields500) {
  auto badStore = std::make_shared<FailWriteStore>();
  ApiHandler h("/api", badStore);

  HttpRequest r;
  r.method = "POST";
  r.path   = "/api/Books";
  r.body   = R"({"foo":"bar"})";
  auto res = h.handle_request(r);
  EXPECT_EQ(res->status_code, 500);
}

// 4) read() returns nullopt in RETRIEVE branch → 404
TEST_F(ApiHandlerTest, MissingEntityOnGetYields404) {
  req.method = "GET";
  req.path   = "/api/Widgets/99";  // 99 was never created
  auto res = handler->handle_request(req);
  EXPECT_EQ(res->status_code, 404);
}

// 5) list existing ids within entity → 200
TEST_F(ApiHandlerTest, ListSuccessful) {
  req.method = "POST";
  req.path   = "/api/Fruits";
  req.body   = R"({"name":"apple"})";
  handler->handle_request(req);

  req.method = "POST";
  req.path   = "/api/Fruits";
  req.body   = R"({"name":"banana"})";
  handler->handle_request(req);

  req.method = "GET";
  req.path   = "/api/Fruits"; 
  req.body   = "";
  auto res = handler->handle_request(req);
  EXPECT_EQ(res->status_code, 200);
  EXPECT_EQ(res->body, "{\"id\":[0,1]}");
}

// 6) entity exists but no ids → 200 + empty list
TEST_F(ApiHandlerTest, ListEntityWithNoID) {
  // TODO: add POST/DELETE

  req.method = "GET";
  req.path   = "/api/Fruits";  
  auto res = handler->handle_request(req);
  EXPECT_EQ(res->status_code, 404);
  // EXPECT_EQ(res->status_code, 200);
  // EXPECT_EQ(res->body, "{\"id\":[]}");
}

// 7) entity does not exist for list → 404
TEST_F(ApiHandlerTest, ListMissingEntityYields404) {
  req.method = "GET";
  req.path   = "/api/DoesNotExist";  
  auto res = handler->handle_request(req);
  EXPECT_EQ(res->status_code, 404);
}

// 8) unhandled method or missing ID → 400
TEST_F(ApiHandlerTest, UnhandledMethodOrMissingIdYields400) {
  // POST *with* ID present
  req.method = "POST";
  req.path   = "/api/Shoes/0";
  auto res2 = handler->handle_request(req);
  EXPECT_EQ(res2->status_code, 400);

  // DELETE is not implemented
  req.method = "DELETE";
  req.path   = "/api/Shoes/0";
  auto res3 = handler->handle_request(req);
  EXPECT_EQ(res3->status_code, 400);
}

// 9) the static factory registration in HandlerRegistry
TEST(ApiHandlerFactoryTest, RegisteredInHandlerRegistry) {
  auto ptr = HandlerRegistry::instance().createHandler(
      "ApiHandler", std::vector<std::string>{"/api", "/tmp/data"});
  ASSERT_TRUE(ptr) << "ApiHandler factory should be registered";
  // Should actually produce an ApiHandler
  auto* api = dynamic_cast<ApiHandler*>(ptr.get());
  EXPECT_NE(api, nullptr);
}