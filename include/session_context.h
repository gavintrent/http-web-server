#ifndef SESSION_CONTEXT_H
#define SESSION_CONTEXT_H

#include <string>
#include <optional>
#include <unordered_map>

class SessionContext {
public:
    SessionContext() = default;
    ~SessionContext() = default;

    // Session token
    std::optional<std::string> session_token;

    // User information
    std::optional<std::string> user_id;

    // Session data storage
    std::unordered_map<std::string, std::string> data;

    // Check if user is authenticated
    bool is_authenticated() const { 
        return session_token.has_value() && user_id.has_value(); 
    }

    // Clear all session data
    void clear() {
        session_token.reset();
        user_id.reset();
        data.clear();
    }

    // Get session data
    std::optional<std::string> get_data(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Set session data
    void set_data(const std::string& key, const std::string& value) {
        data[key] = value;
    }
};

#endif // SESSION_CONTEXT_H 