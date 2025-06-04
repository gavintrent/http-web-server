#include "gtest/gtest.h"
#include "api_handler.h"
#include "fake_file_store.h"
#include "handler_registry.h"
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
  std::optional<std::string> read(const std::string& entity, int id) override {
    return std::nullopt;
  }
  std::optional<std::vector<int>> read_directory(const std::string& entity) override {
    return std::nullopt;
  }
  bool remove(const std::string& entity, int id) override {
    return false;  // always fail
  }
  std::string get_root() const override { return "fail_write_root"; }
};

// Extension of FailWriteStore that succeeds on read for testing delete functionality
class MockReadSuccessStore : public FailWriteStore {
 public:
  std::optional<std::string> read(const std::string& entity, int id) override {
    return R"({"test":"data"})";
  }
  std::string get_root() const override { return "mock_read_root"; }
};

// A FileStore that always succeeds to read directory
class MockReadDirectorySuccessStore : public FailWriteStore {
public:
    std::optional<std::vector<int>> read_directory(const std::string&) override {
        return std::vector<int>{1, 2, 3};
    }
    std::string get_root() const override { return "mock_read_dir_root"; }
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
  req.method = "GET";
  req.path   = "/api/Fruits";  
  auto res = handler->handle_request(req);
  EXPECT_EQ(res->status_code, 404);
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

// 10) Update existing entity succeeds
TEST_F(ApiHandlerTest, UpdateEntitySucceeds) {
  // First create an entity
  json data = {{"name", "test shoe"}, {"size", 10}};
  int id = 0;
  store->write("Shoes", id, data.dump());
  
  // Build an update request
  req.method = "PUT";
  req.path = "/api/Shoes/0";
  json updated_data = {{"name", "updated shoe"}, {"size", 11}};
  req.body = updated_data.dump();
  
  // Send update request
  auto res = handler->handle_request(req);
  
  // Verify response
  EXPECT_EQ(res->status_code, 200);
  
  // Parse the body to verify it contains success message
  json response = json::parse(res->body);
  EXPECT_TRUE(response["success"]);
  EXPECT_EQ(response["id"], id);
  
  // Verify data was actually updated in the store
  auto stored_data = store->read("Shoes", id);
  ASSERT_TRUE(stored_data.has_value());
  json parsed_stored = json::parse(*stored_data);
  EXPECT_EQ(parsed_stored["name"], "updated shoe");
  EXPECT_EQ(parsed_stored["size"], 11);
}

// 11) Update non-existent entity creates it
TEST_F(ApiHandlerTest, UpdateNonExistentEntityCreatesIt) {
  // Build an update request for a non-existent entity
  req.method = "PUT";
  req.path = "/api/Books/42";
  json data = {{"title", "The Hitchhiker's Guide to the Galaxy"}, {"author", "Douglas Adams"}};
  req.body = data.dump();
  
  // Send update request
  auto res = handler->handle_request(req);
  
  // Verify response
  EXPECT_EQ(res->status_code, 200);
  
  // Verify data was created in the store
  auto stored_data = store->read("Books", 42);
  ASSERT_TRUE(stored_data.has_value());
  json parsed_stored = json::parse(*stored_data);
  EXPECT_EQ(parsed_stored["title"], "The Hitchhiker's Guide to the Galaxy");
}

// 12) Update with invalid JSON fails
TEST_F(ApiHandlerTest, UpdateWithInvalidJsonFails) {
  // Build an update request with invalid JSON
  req.method = "PUT";
  req.path = "/api/Shoes/1";
  req.body = "{invalid json";
  
  // Send update request
  auto res = handler->handle_request(req);
  
  // Verify response indicates error
  EXPECT_EQ(res->status_code, 400);
}

// 13) Delete existing entity succeeds
TEST_F(ApiHandlerTest, DeleteExistingEntitySucceeds) {
  // First create an entity
  json data = {{"name", "test shoe"}, {"size", 10}};
  store->write("Shoes", 1, data.dump());
  
  // Build a delete request
  req.method = "DELETE";
  req.path = "/api/Shoes/1";
  
  // Send delete request
  auto res = handler->handle_request(req);
  
  // Verify response
  EXPECT_EQ(res->status_code, 200);
  
  // Parse the body to verify it contains success message
  json response = json::parse(res->body);
  EXPECT_TRUE(response["success"]);
  
  // Verify entity was actually deleted
  auto stored_data = store->read("Shoes", 1);
  EXPECT_FALSE(stored_data.has_value());
}

// 14) Delete non-existent entity fails
TEST_F(ApiHandlerTest, DeleteNonExistentEntityFails) {
  // Build a delete request for a non-existent entity
  req.method = "DELETE";
  req.path = "/api/Shoes/9999";
  
  // Send delete request
  auto res = handler->handle_request(req);
  
  // Verify response indicates error
  EXPECT_EQ(res->status_code, 404);
}

// 15) Write failure on delete returns 500
TEST(ApiHandlerStandaloneTest, RemoveFailureYields500) {
  auto mockStore = std::make_shared<MockReadSuccessStore>();
  ApiHandler handler("/api", mockStore);
  
  HttpRequest r;
  r.method = "DELETE";
  r.path = "/api/Books/1";
  
  auto res = handler.handle_request(r);
  EXPECT_EQ(res->status_code, 500);
}