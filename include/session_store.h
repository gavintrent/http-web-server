#ifndef SESSION_STORE_H
#define SESSION_STORE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <chrono>

struct SessionData {
    std::string user_id;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point expires_at;
    std::unordered_map<std::string, std::string> data;

    SessionData(const std::string& uid, 
                std::chrono::system_clock::duration expiry = std::chrono::hours(24))
        : user_id(uid),
          created_at(std::chrono::system_clock::now()),
          expires_at(std::chrono::system_clock::now() + expiry) {}
};

class SessionStore {
public:
    static SessionStore& get_instance() {
        static SessionStore instance;
        return instance;
    }

    // create a new session for a user
    std::string create_session(const std::string& user_id);

    // get session data by token
    std::shared_ptr<SessionData> get_session(const std::string& token);

    // invalidate a session
    void invalidate_session(const std::string& token);

    // update session data
    void update_session_data(const std::string& token, 
                           const std::string& key, 
                           const std::string& value);

    // clean up expired sessions
    void cleanup_expired_sessions();

private:
    SessionStore() = default;
    ~SessionStore() = default;
    SessionStore(const SessionStore&) = delete;
    SessionStore& operator=(const SessionStore&) = delete;

    std::unordered_map<std::string, std::shared_ptr<SessionData>> sessions_;
    mutable std::shared_mutex mutex_; // mutable allows const methods to lock
};

#endif // SESSION_STORE_H 