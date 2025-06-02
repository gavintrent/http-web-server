#pragma once

#include <string>
#include <vector>
#include <mutex>

struct Message {
  std::string content;
  std::string timestamp;
};

class MessageStore {
public:
  void add(const std::string& message);
  std::vector<Message> get_all();
  void load_from_file(const std::string& path);
  void persist_to_file(const std::string& path);

private:
  std::vector<Message> messages_;
  std::mutex lock_;
};
