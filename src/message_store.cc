#include "message_store.h"

// We’ll use Boost.PropertyTree to serialize/deserialize each Message → JSON
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

void MessageStore::add(const std::string& content) {
  std::lock_guard<std::mutex> guard(lock_);
  Message msg;
  msg.content   = content;
  msg.timestamp = current_timestamp();
  messages_.push_back(std::move(msg));
}

std::vector<Message> MessageStore::get_all() {
  std::lock_guard<std::mutex> guard(lock_);
  return messages_;
}
