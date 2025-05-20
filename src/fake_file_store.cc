#include "fake_file_store.h"
#include <vector>

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

std::optional<std::string> FakeFileStore::read(const std::string& entity, int id) {
    auto ent_it = store_.find(entity);
    if (ent_it == store_.end()) return std::nullopt;
    auto obj_it = ent_it->second.find(id);
    if (obj_it == ent_it->second.end()) return std::nullopt;
    return obj_it->second;
}

std::optional<std::vector<std::string>> FakeFileStore::read_directory(const std::string& entity) {
    std::vector<std::string> filenames;
    auto ent_it = store_.find(entity);
    if (ent_it == store_.end()) return std::nullopt;
    for (const auto& obj_it : ent_it->second)
        filenames.push_back(std::to_string(obj_it.first));
    return filenames;
}

bool FakeFileStore::remove(const std::string& entity, int id) {
    // Check if the entity exists
    auto ent_it = store_.find(entity);
    if (ent_it == store_.end()) {
        return false;
    }
    // Check if the ID exists within the entity
    auto& entity_map = ent_it->second;
    auto obj_it = entity_map.find(id);
    if (obj_it == entity_map->second.end()) {
        return false;
    }
    // Remove the entity and return true to indicate success
    return entity_map.erase(id) > 0;
}