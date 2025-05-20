#include "fake_file_store.h"

int FakeFileStore::next_id(const std::string& entity) {
  int& counter = next_id_[entity];
  return counter++;
}

bool FakeFileStore::write(const std::string& entity,
                          int id,
                          const std::string& data) {
  store_[entity][id] = data;
  return true;
}

std::optional<std::string> FakeFileStore::read(const std::string& entity,
                                               int id) {
  auto ent_it = store_.find(entity);
  if (ent_it == store_.end()) return std::nullopt;
  auto obj_it = ent_it->second.find(id);
  if (obj_it == ent_it->second.end()) return std::nullopt;
  return obj_it->second;
}

std::optional<std::vector<int>> FakeFileStore::read_directory(const std::string& entity) {
  
  std::vector<int> filenames;
  auto ent_it = store_.find(entity);
  if (ent_it == store_.end()) return std::nullopt;
  for (const auto& obj_it : ent_it->second)
    filenames.push_back(obj_it.first);

  return filenames;
}