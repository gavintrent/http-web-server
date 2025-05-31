#include "session_store.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>

class SessionStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // get a fresh instance of SessionStore for each test
        store = &SessionStore::get_instance();
    }

    void TearDown() override {
        // clean up any remaining sessions
        store->cleanup_expired_sessions();
    }

    SessionStore* store;
};

// test session creation and retrieval
TEST_F(SessionStoreTest, CreateAndGetSession) {
    std::string user_id = "test_user";
    std::string token = store->create_session(user_id);
    
    ASSERT_FALSE(token.empty());
    
    auto session = store->get_session(token);
    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->user_id, user_id);
}

// test session invalidation
TEST_F(SessionStoreTest, InvalidateSession) {
    std::string user_id = "test_user";
    std::string token = store->create_session(user_id);
    
    ASSERT_NE(store->get_session(token), nullptr);
    
    store->invalidate_session(token);
    EXPECT_EQ(store->get_session(token), nullptr);
}

// test session data storage and retrieval
TEST_F(SessionStoreTest, SessionDataStorage) {
    std::string user_id = "test_user";
    std::string token = store->create_session(user_id);
    
    store->update_session_data(token, "key1", "value1");
    store->update_session_data(token, "key2", "value2");
    
    auto session = store->get_session(token);
    ASSERT_NE(session, nullptr);
    
    EXPECT_EQ(session->data["key1"], "value1");
    EXPECT_EQ(session->data["key2"], "value2");
}

TEST_F(SessionStoreTest, SessionExpiration) {
    std::string user_id = "test_user";
    std::string token = store->create_session(user_id);
    
    // create a session with a very short expiration time
    auto session = store->get_session(token);
    session->expires_at = std::chrono::system_clock::now() + std::chrono::milliseconds(100);
    
    // wait for the session to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // session should be expired
    EXPECT_EQ(store->get_session(token), nullptr);
}

TEST_F(SessionStoreTest, ConcurrentSessionCreation) {
    const int num_threads = 10;
    const int sessions_per_thread = 100;
    std::vector<std::thread> threads;
    std::vector<std::string> tokens;
    std::mutex tokens_mutex;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < sessions_per_thread; ++j) {
                std::string user_id = "user_" + std::to_string(i) + "_" + std::to_string(j);
                std::string token = store->create_session(user_id);
                
                std::lock_guard<std::mutex> lock(tokens_mutex);
                tokens.push_back(token);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // verify all sessions were created successfully
    EXPECT_EQ(tokens.size(), num_threads * sessions_per_thread);
    
    // verify all sessions are valid and contain correct user IDs
    for (size_t i = 0; i < tokens.size(); ++i) {
        auto session = store->get_session(tokens[i]);
        ASSERT_NE(session, nullptr);
        EXPECT_FALSE(session->user_id.empty());
    }
}

TEST_F(SessionStoreTest, SessionCleanup) {
    std::string token1 = store->create_session("user1");
    std::string token2 = store->create_session("user2");
    
    // make one session expire
    auto session = store->get_session(token1);
    session->expires_at = std::chrono::system_clock::now() - std::chrono::hours(1);
    
    store->cleanup_expired_sessions();
    
    // verify expired session is removed and valid remains
    EXPECT_EQ(store->get_session(token1), nullptr);
    EXPECT_NE(store->get_session(token2), nullptr);
}

TEST_F(SessionStoreTest, SessionDataUpdates) {
    std::string user_id = "test_user";
    std::string token = store->create_session(user_id);
    
    // update session data multiple times
    store->update_session_data(token, "counter", "1");
    store->update_session_data(token, "counter", "2");
    store->update_session_data(token, "counter", "3");
    
    auto session = store->get_session(token);
    ASSERT_NE(session, nullptr);
    EXPECT_EQ(session->data["counter"], "3");
}

TEST_F(SessionStoreTest, InvalidSessionToken) {
    EXPECT_EQ(store->get_session("invalid_token"), nullptr);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
