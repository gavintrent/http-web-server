#include "message_store.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>

using boost::property_tree::ptree;
namespace fs = std::filesystem;

MessageStore& MessageStore::instance() {
  static MessageStore _instance;
  return _instance;
}

// Helper: return current UTC time as ISO-8601 string (e.g. "2025-06-02T20:15:30Z").
static std::string current_timestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  std::time_t t = system_clock::to_time_t(now);
  std::tm tm_buf;

  // Convert to UTC in a thread-safe way
  #ifdef _MSC_VER
    gmtime_s(&tm_buf, &t);
  #else
    gmtime_r(&t, &tm_buf);
  #endif

  std::ostringstream ss;
  ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
  return ss.str();
}

void MessageStore::add(const std::string& username, const std::string& content) {
  std::lock_guard<std::mutex> guard(lock_);
  Message msg;
  msg.username  = username;
  msg.content   = content;
  msg.timestamp = current_timestamp();
  messages_.push_back(std::move(msg));
}

std::vector<Message> MessageStore::get_all() {
  std::lock_guard<std::mutex> guard(lock_);
  return messages_;
}

void MessageStore::load_from_file(const std::string& path) {
  std::lock_guard<std::mutex> guard(lock_);
  messages_.clear();

  fs::path dir(path);
  std::error_code ec;

  if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
    return;
  }
  for (auto& entry : fs::directory_iterator(dir, ec)) {
    if (ec) {
      break;
    }

    if (!entry.is_regular_file())
      continue;

    std::ifstream inF(entry.path());
    if (!inF.is_open())
      continue;

    std::ostringstream buffer;
    buffer << inF.rdbuf();
    std::string jsonText = buffer.str();
    inF.close();

    try {
      ptree tree;
      std::istringstream iss(jsonText);
      read_json(iss, tree);

      Message msg;
      msg.username  = tree.get<std::string>("username",  "");
      msg.content   = tree.get<std::string>("content", "");
      msg.timestamp = tree.get<std::string>("timestamp", "");
      messages_.push_back(std::move(msg));
    } catch (const std::exception&) {
      continue;
    }
  }
}

void MessageStore::persist_to_file(const std::string& path) {
  std::lock_guard<std::mutex> guard(lock_);
  fs::path dir(path);
  std::error_code ec;

  if (!fs::exists(dir, ec)) {
    fs::create_directories(dir, ec);
    if (ec) {
      return;
    }
  } else if (!fs::is_directory(dir, ec)) {
    return;
  }

  // Remove everything currently in that directory
  for (auto& entry : fs::directory_iterator(dir, ec)) {
    if (ec) {
      break;
    }
    if (entry.is_regular_file()) {
      fs::remove(entry.path(), ec);
    }
  }

  // Write each Message into a new file named “<idx>.json”
  int idx = 1;
  for (const auto& msg : messages_) {
    ptree tree;
    tree.put("username",  msg.username);
    tree.put("timestamp", msg.timestamp);
    tree.put("content",   msg.content);

    fs::path outPath = dir / (std::to_string(idx) + ".json");
    std::ofstream outF(outPath);
    if (!outF.is_open()) {
      ++idx;
      continue;
    }
    write_json(outF, tree, true);
    outF.close();
    ++idx;
  }
}