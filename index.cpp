/**
* @file compress.cpp
 * @brief Handle compression for VCS.
 *
 * Implement huffman coding for compression of objects.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <array>
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <vector>

#include "io.h"
#include <algorithm>
#include <iostream>

typedef struct IndexEntryHeader {
    uint32_t ctime_sec;
    uint32_t ctime_nsec;
    uint32_t mtime_sec;
    uint32_t mtime_nsec;
    uint32_t dev;
    uint32_t ino;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t file_size;

    std::array<uint8_t, 20> sha1;
    uint16_t flags;
};
typedef struct IndexEntry {
    IndexEntryHeader header;
    std::string path;
};
typedef struct IndexData {
    char signature[4]; // "DIRC".
    uint32_t version;
    uint32_t num_entries;
    std::vector<IndexEntry> entries;
};
uint16_t swap_uint16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

uint32_t swap_uint32(uint32_t val) {
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
}

IndexData load_index(std::filesystem::path index_path) {
    IndexData index;
    std::ifstream ifs(index_path.string(), std::ios::binary);
    // Check size.
    auto f_size = std::filesystem::file_size(index_path);
    if (f_size < 32) {
        // File is too small to even contain a header and a hash.
        return index;
    }

    if (ifs.is_open()) {
        // Read in the signature.
        ifs.read(index.signature, 4);
        if (std::string(index.signature, 4) != "DIRC") {
            err_out("Current vcs index is corrupted.");
            return index;
        }

        // Read in the version and number of entries.
        uint32_t version, num_entries;
        ifs.read(reinterpret_cast<char*>(&version), 4);
        ifs.read(reinterpret_cast<char*>(&num_entries), 4);

        for (int i = 0; i < num_entries; i++) {
            IndexEntry entry;
            ifs.read(reinterpret_cast<char*>(&entry.header), 62);
            uint16_t flags = swap_uint16(entry.header.flags);
            uint16_t path_len = flags & 0x0FFF;

            // Read the variable-length path and resize a string to the length and read directly into its buffer
            entry.path.resize(path_len);
            ifs.read(&entry.path[0], path_len);

            // Calculate and skip the padding - entries are padded with 0-8 null bytes to a multiple of 8.
            size_t bytes_read = 62 + path_len;
            size_t padding = 8 - (bytes_read % 8);

            // Jump the file pointer over the null bytes.
            ifs.seekg(padding, std::ios::cur);

            // Add to the vector.
            index.entries.push_back(entry);
        }
    }
}

void add_to_index(std::filesystem::path index_path, std::filesystem::path source_path, int source_perms, std::string source_hash) {

}

void make_index(std::filesystem::path index_path, std::string user_name, std::string user_email, std::vector<std::string> file_paths, std::vector<std::string> file_blob_hashes) {
    assert(file_paths.size() == file_blob_hashes.size());
    // Read in existing index.
    std::ifstream oldIndexFile(index_path);
    std::vector<std::string> oldIndexFile_paths;
    std::vector<std::string> oldIndexFile_blob_hashes;
    std::string old_user_name, old_user_email;
    if (oldIndexFile.is_open()) {
        std::getline(oldIndexFile, old_user_name);
        std::getline(oldIndexFile, old_user_email);
        std::string line;
        while (std::getline(oldIndexFile, line)) {
            oldIndexFile_paths.push_back(line.substr(0, line.find("\x1f")));
            oldIndexFile_blob_hashes.push_back(line.substr(line.find("\x1f") + 1));
        }
    }

    // Remove any now-obsolete entries (newer version added).
    for (int i = 0; i < oldIndexFile_paths.size(); i++) {
        if (std::find(file_paths.begin(), file_paths.end(), oldIndexFile_paths[i]) != file_paths.end()) {
            std::filesystem::path oldBlobPath = std::filesystem::path(oldIndexFile_blob_hashes[i]);
            std::filesystem::remove(oldBlobPath);
            std::filesystem::path parent = oldBlobPath.parent_path();
            std::cout << parent.string() << std::endl;
            if (!parent.empty() && std::filesystem::exists(parent) && std::filesystem::is_empty(parent)) {
                std::filesystem::remove(parent);
            }
        }
        else {
            file_paths.push_back(oldIndexFile_paths[i]);
            file_blob_hashes.push_back(oldIndexFile_blob_hashes[i]);
        }
    }

    std::string index_content = "";
    index_content += user_name + "\n";
    index_content += user_email + "\n";
    for (int i = 0; i < file_paths.size(); i++) {
        index_content += file_paths[i] + "\x1f" + file_blob_hashes[i] + "\n";
    }
    std::ofstream indexFile(index_path);
    if (indexFile.is_open()) {
        indexFile << index_content;
    }
}