#pragma once
#include "file_store.h"
#include <string>

// A FileStore that uses the real filesystem under `root_dir`.
class DiskFileStore : public FileStore {
public:
  explicit DiskFileStore(std::string root_dir);
  int next_id(const std::string& entity) override;
  bool write(const std::string& entity, int id,
             const std::string& data) override;
  std::optional<std::string> read(const std::string& entity,
                                  int id) override;

private:
  std::string root_;
};