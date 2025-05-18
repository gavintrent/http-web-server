#pragma once
#include "file_store.h"
#include <map>
#include <string>
#include <optional>

// An in‐memory FileStore for unit tests.
class FakeFileStore : public FileStore {
public:
  FakeFileStore() = default;
  int next_id(const std::string& entity) override;
  bool write(const std::string& entity,
             int id,
             const std::string& data) override;
  std::optional<std::string> read(const std::string& entity,
                                  int id) override;

private:
  std::map<std::string, int> next_id_;
  std::map<std::string, std::map<int, std::string>> store_;
};