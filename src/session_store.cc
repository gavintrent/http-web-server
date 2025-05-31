#include "session_store.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <boost/log/trivial.hpp>
#include <mutex>

std::string generate_session_token() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";

    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << hex[dis(gen)];
    }
    return ss.str();
}

std::string SessionStore::create_session(const std::string& user_id) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    std::string token = generate_session_token(); // generate a unique session token
    
    // create new session data
    auto session_data = std::make_shared<SessionData>(user_id);
    sessions_[token] = session_data;
    
    BOOST_LOG_TRIVIAL(info) << "Created new session for user " << user_id;
    return token;
}

std::shared_ptr<SessionData> SessionStore::get_session(const std::string& token) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it == sessions_.end()) {
        return nullptr;
    }
    
    // if session has expired
    if (std::chrono::system_clock::now() > it->second->expires_at) {
        // need exclusive lock to remove expired session
        lock.unlock();
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        sessions_.erase(token);
        return nullptr;
    }
    
    return it->second;
}

void SessionStore::invalidate_session(const std::string& token) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    sessions_.erase(token);
    BOOST_LOG_TRIVIAL(info) << "Invalidated session " << token;
}

void SessionStore::update_session_data(const std::string& token,
                                     const std::string& key,
                                     const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = sessions_.find(token);
    if (it != sessions_.end()) {
        it->second->data[key] = value;
    }
}

void SessionStore::cleanup_expired_sessions() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto now = std::chrono::system_clock::now();
    
    for (auto it = sessions_.begin(); it != sessions_.end();) {
        if (now > it->second->expires_at) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
} 