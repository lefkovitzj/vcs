/**
 * @file head.cpp
 * @brief Get the HEAD reference.
 *
 * Implement version control HEAD with blob indices for the version control state and ref tracking.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <filesystem>
#include <format>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>

#include "../utils/io.h"

std::string get_ref(std::filesystem::path vcs_dir) {
    // Get the current HEAD file.
    std::ifstream headFile(vcs_dir / "HEAD");
    std::string head_ref;
    if (headFile.is_open()) {
        std::getline(headFile, head_ref);
    }

    // Get info after 'ref: '
    if (head_ref.find("ref: ") != std::string::npos) {
        head_ref = head_ref.substr(5);
    }
    else {
        throw std::runtime_error("HEAD file is malformed");
    }
    return head_ref;
}

std::map<std::string, std::string> get_head_as_map(std::filesystem::path vcs_dir) {
    /* Get a map of the files and blobs in the HEAD ref */

    // Get the list of change since head ref.
    std::map<std::string, std::string> changed_files_since_head;
    std::ifstream refHeadFile(vcs_dir / get_ref(vcs_dir));
    std::string line;
    if (refHeadFile.is_open()) {
        std::getline(refHeadFile, line); // Read the path to the commit blob.
    }
    if (line.empty()) {
        return changed_files_since_head; // No commit yet, return empty map.
    }
    std::filesystem::path commit_blob_path = std::filesystem::absolute(line);
    std::ifstream commitBlobFile(commit_blob_path);
    if (commitBlobFile.is_open()) {
        std::getline(commitBlobFile, line); // Skip the second line (user name).
        std::getline(commitBlobFile, line); // Skip the third line (user email).
        while (std::getline(commitBlobFile, line)) {
            size_t delimiter_pos = line.find('\x1f');
            if (delimiter_pos != std::string::npos) {
                std::string file_path = line.substr(0, delimiter_pos);
                std::string blob_path = line.substr(delimiter_pos + 1);
                info_out(std::format("\t{}", file_path), true);
                changed_files_since_head.insert({file_path, blob_path});
            }
        }
    }
    return changed_files_since_head;
}

std::string get_head_commit_hash(std::filesystem::path vcs_dir) {
    std::string head_ref = get_ref(vcs_dir);
    std::ifstream refHeadFile(vcs_dir / get_ref(vcs_dir));
    std::string line;
    if (refHeadFile.is_open()) {
        std::getline(refHeadFile, line);
    }
    if (line.empty()) {
        return "";
    }
    // Get last chunk and the two-char prefix, concat to string.
    std::filesystem::path commit_blob_path = std::filesystem::absolute(line);
    std::string last_chunk = commit_blob_path.filename().string();
    std::string prefix =  commit_blob_path.parent_path().filename().string();

    std::string complete_hash = prefix + last_chunk;
    return complete_hash;
}