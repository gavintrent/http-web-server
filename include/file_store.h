#pragma once
#include <string>
#include <optional>
#include <vector>

// Abstract filesystem store for JSON blobs, keyed by entity name + numeric ID.
struct FileStore {
    virtual ~FileStore() = default;

    // Return the next ID to use for `entity`.
    virtual int next_id(const std::string& entity) = 0;

    // Write `data` under filesystem path: root/entity/id
    // Returns true on success.
    virtual bool write(const std::string& entity,
                       int id,
                       const std::string& data) = 0;

    // Read the blob for entity/id, or nullopt if missing.
    virtual std::optional<std::string> read(const std::string& entity,
                                            int id) = 0;

    // Remove the blob for entity/id
    // Returns true on success, false if entity doesn't exist or removal fails
    virtual bool remove(const std::string& entity, int id) = 0;

    // Read the directory for entity, returning list of IDs or names.
    virtual std::optional<std::vector<int>> read_directory(const std::string& entity) = 0;

    // Get the root directory path
    virtual std::string get_root() const = 0;
};