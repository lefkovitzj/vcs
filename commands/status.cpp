/* @file status.cpp
* @brief Handle status operations for VCS.
*
* Implement the status functionality in reusable functions.
*
* @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
*/

#include <filesystem>
#include <format>
#include <map>
#include <string>
#include <vector>

#include "../utils/io.h"
#include "../core/index.h"
#include "../core/blob.h"
#include "../core/head.h"
#include "../core/vcsno.h"


void vcs_status(std::filesystem::path vcs_dir) {
    /* Display the status of the VCS. */

    std::map<std::string, std::string> changed_files_since_head = get_head_as_map(vcs_dir);
    std::map<std::string, std::string> indexed_files = get_index_as_map(vcs_dir / "index");

    std::vector<std::string> untracked_files;
    std::vector<std::string> staged_files;
    std::vector<std::string> unstaged_files;

    for (auto entry = std::filesystem::recursive_directory_iterator(std::filesystem::current_path());
         entry != std::filesystem::recursive_directory_iterator(); ++entry) {

            // 2. Check the directory itself BEFORE entering it
            if (std::filesystem::is_directory(entry->path())) {
                if (in_vcsno(entry->path())) {
                    entry.disable_recursion_pending(); // <--- Crucial: Tells the iterator NOT to go inside this folder
                    continue;
                }
            }

        if (std::filesystem::is_regular_file(entry->path()) and !in_vcsno(entry->path())) {
            std::string blob_hash = hash_file_blob(entry->path());
            std::string hash_prefix = blob_hash.substr(0, 2);
            std::string hash_noprefix = blob_hash.substr(2);

            std::filesystem::path blob_path = vcs_dir / "objects" / hash_prefix / hash_noprefix;
            std::string blob_path_str = blob_path.string();
            std::string file_path_str = entry->path().string();

            if (indexed_files.find(file_path_str) != indexed_files.end()) {
                // File is staged.
                staged_files.push_back(file_path_str);

                // Check whether the blob path matches the index.
                if (indexed_files[file_path_str] != blob_path_str) {
                    // Blob doesn't match index. Newer version exists than in index. File is both staged and unstaged.
                    unstaged_files.push_back(file_path_str);
                }
            }
            else if (changed_files_since_head.find(file_path_str) != changed_files_since_head.end()) {
                // File is unstaged.
                unstaged_files.push_back(file_path_str);
            }
            else {
                // File is untracked.
                untracked_files.push_back(file_path_str);
            }
        }
    }

    // Display the status.
    info_out("Untracked files:");
    for (const auto& file : untracked_files) {
        info_out(std::format("\t{}", file), true);
    }

    info_out("Staged files:");
    for (const auto& file : staged_files) {
        info_out(std::format("\t{}", file), true);
    }

    info_out("Unstaged files:");
    for (const auto& file : unstaged_files) {
        info_out(std::format("\t{}", file), true);
    }
}
