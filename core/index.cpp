/**
* @file index.cpp
 * @brief Create blob index.
 *
 * Implement staging area with blob index for the version control state tracking.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <array>
#include <assert.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>

void make_index(std::filesystem::path index_path, std::string user_name, std::string user_email, std::vector<std::string> file_paths, std::vector<std::string> file_blob_hashes) {
    /* Create a blob index from the args passed from the main VCS loop. */
    assert(file_paths.size() == file_blob_hashes.size());
    // Store data from existing index.
    std::ifstream oldIndexFile(index_path);
    std::vector<std::string> oldIndexFile_paths;
    std::vector<std::string> oldIndexFile_blob_hashes;
    std::string old_user_name, old_user_email;

    // Read in the current index file contents.
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
            // File had old blob and has new blob - replace the old.
            std::filesystem::path oldBlobPath = std::filesystem::path(oldIndexFile_blob_hashes[i]);
            // Remove the old blob file.
            std::filesystem::remove(oldBlobPath);
            std::filesystem::path parent = oldBlobPath.parent_path();
            if (!parent.empty() && std::filesystem::exists(parent) && std::filesystem::is_empty(parent)) {
                // If it was the last blob in its .vcs/objects subdirectory, delete the subdirectory.
                std::filesystem::remove(parent);
            }
        }
        else {
            // File had an old blob with no update - keep it.
            file_paths.push_back(oldIndexFile_paths[i]);
            file_blob_hashes.push_back(oldIndexFile_blob_hashes[i]);
        }
    }

    // Concatenate contents for the file.
    std::string index_content = "";
    index_content += user_name + "\n";
    index_content += user_email + "\n";
    for (int i = 0; i < file_paths.size(); i++) {
        index_content += file_paths[i] + "\x1f" + file_blob_hashes[i] + "\n";
    }

    // Write contents to the file.
    std::ofstream indexFile(index_path);
    if (indexFile.is_open()) {
        indexFile << index_content;
    }
}

std::map<std::string, std::string> get_index_as_map(std::filesystem::path index_path) {
    /* Get a map of the files and blobs in the index */

    // Get the list of files in index.
    std::map<std::string, std::string> indexed_files;
    std::ifstream indexFile(index_path);
    if (indexFile.is_open()) {
        // Parse the file contents.
        std::string line;
        std::getline(indexFile, line); // Skip the second line (user name).
        std::getline(indexFile, line); // Skip the third line (user email).
        while (std::getline(indexFile, line)) {
            // Split each line into file path and blob path about the Unit Separator.
            size_t delimiter_pos = line.find('\x1f');
            if (delimiter_pos != std::string::npos) {
                std::string file_path = line.substr(0, delimiter_pos);
                std::string blob_path = line.substr(delimiter_pos + 1);

                // Add the line to the map.
                indexed_files.insert({file_path, blob_path});
            }
        }
    }
    return indexed_files;
}
