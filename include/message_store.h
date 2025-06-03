#pragma once

#include <string>
#include <vector>
#include <mutex>

struct Message {
  std::string content;
  std::string timestamp;
  std::string username;
};

class MessageStore {
public:
  static MessageStore& instance();
  void add(const std::string& username, const std::string& content);
  std::vector<Message> get_all();
  void load_from_file(const std::string& path);
  void persist_to_file(const std::string& path);

private:
  MessageStore() = default;
  ~MessageStore() = default;
  MessageStore(const MessageStore&) = delete;
  MessageStore& operator=(const MessageStore&) = delete;
  std::vector<Message> messages_;
  std::mutex lock_;
};