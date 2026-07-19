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

std::map<std::string, std::string> get_head_as_map(std::filesystem::path vcs_dir) {
    /* Get a map of the files and blobs in the HEAD ref */

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

    // Get the list of change since head ref.
    std::map<std::string, std::string> changed_files_since_head;
    std::ifstream refHeadFile(vcs_dir / head_ref);
    if (refHeadFile.is_open()) {
        std::string line;
        std::getline(refHeadFile, line); // Skip the second line (user name).
        std::getline(refHeadFile, line); // Skip the third line (user email).
        while (std::getline(refHeadFile, line)) {
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
