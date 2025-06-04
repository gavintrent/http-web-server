#include "disk_file_store.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

DiskFileStore::DiskFileStore(std::string root_dir)
    : root_(std::move(root_dir)) {}

std::string DiskFileStore::get_root() const {
    return root_;
}

int DiskFileStore::next_id(const std::string& entity) {
    fs::path dir = fs::path(root_) / entity;
    if (!fs::exists(dir)) return 0;

    int max_id = -1;
    for (auto const& ent : fs::directory_iterator(dir)) {
        if (!ent.is_regular_file()) continue;
        try {
            int id = std::stoi(ent.path().filename().string());
            max_id = std::max(max_id, id);
        } catch (...) { }
    }
    return max_id + 1;
}

bool DiskFileStore::write(const std::string& entity, int id, const std::string& data) {
    fs::path dir =  fs::path(root_) / entity;
    std::error_code ec;
    fs::create_directories(dir, ec);
    if (ec) return false;
    fs::path file = dir / std::to_string(id);
    std::ofstream out(file, std::ios::binary);
    if (!out) return false;
    out << data;
    return bool(out);
}

std::optional<std::string> DiskFileStore::read(const std::string& entity, int id) {
    fs::path file =  fs::path(root_) / entity / std::to_string(id);
    if (!fs::exists(file)) return std::nullopt;
    std::ifstream in(file, std::ios::binary);
    if (!in) return std::nullopt;
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::optional<std::vector<int>> DiskFileStore::read_directory(const std::string& entity) {
    fs::path dir = fs::path(root_) / entity;
    if (!fs::exists(dir))
        return std::nullopt;

    std::vector<int> filenames;
    for (auto const& ent : fs::directory_iterator(dir))
        try {
            filenames.push_back(std::stoi(ent.path().filename().string()));
        } catch (...) { }
    std::sort(filenames.begin(), filenames.end());
    return filenames;
}

bool DiskFileStore::remove(const std::string& entity, int id) {
    fs::path file = fs::path(root_) / entity / std::to_string(id);
    // Check if the file exists before attempting to remove it
    if (!fs::exists(file)) {
        return false; // Can't remove what doesn't exist
    }
    // Attempt to remove the file
    std::error_code ec;
    bool success = fs::remove(file, ec);
    // Return true only if removal was successful and no error occurred
    return success && !ec;
}