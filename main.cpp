/* @file main.cpp
* @brief Main entry point for the VCS application.
*
* Initialize the VCS environment and handle user commands.
*
* @author Joseph Lefkovitz (https://github.com/lefkovitzj)
*/

#include <string>
#include <vector>
#include <filesystem>
#include <format>
#include <map>

/* Utils Imports */
#include "utils/compress.h"
#include "utils/io.h"

/* Core Imports */
#include "core/blob.h"
#include "core/config.h"
#include "core/head.h"
#include "core/index.h"
#include "core/vcsno.h"

/* Command Imports */
#include "commands/init.h"

const float VCS_VERSION_NUM = 0.1;
const std::string VCS_SOURCE_URL = "https://github.com/lefkovitzj/vcs";

// Store the path at which the VCS data is found.
const std::filesystem::path vcs_dir = std::filesystem::current_path() / ".vcs";
bool vcs_inited = std::filesystem::is_directory(vcs_dir);
std::string user_name =  "";
std::string user_email = "";

std::vector<std::string> index_file_paths;
std::vector<std::string> index_file_blobs;

void help_menu() {
    info_out("Help menu");
}
void version_menu() {
    info_out(std::format("Version {}", VCS_VERSION_NUM));
    info_out(std::format("Find the most up-to-date version of VCS at {}", VCS_SOURCE_URL));
}
void handle_config(std::vector<std::string> args) {
    if (args.size() <= 2) {
        // Display current config.
    }
    else {
        if (args.at(1) == "user.name") {
            user_name = args.at(2);
        }
        if (args.at(1) == "user.email") {
            user_email = args.at(2);
        }
    }
}

void add_file(std::filesystem::path file) {
    if (! vcs_inited) {
        err_out("Cannot add file - VCS not initialized yet");
    }
    if (! std::filesystem::exists(file)) {
        err_out(std::format("File {} does not exist", file.string()));
        return;
    }
    // Check that the file is not a part of the .vcs data.
    if (std::filesystem::exists(vcs_dir) && std::filesystem::equivalent(file, vcs_dir)) {
        return;
    }
    if (file.string().find(".vcs") != std::string::npos) {
        return;
    }

    // Check that the file is not set to ignore based on the .vcsno file.
    if (in_vcsno(file)) {
        return;
    }
    if (std::filesystem::is_directory(file)) {
        // Handle directories.
        for (const auto& entry : std::filesystem::directory_iterator(file)) {
            add_file(entry.path());
        }
    }
    else {
        // Handle files.
        std::string blob_hash = hash_file_blob(file);
        if (blob_hash.empty()) {
            err_out(std::format("{}",blob_hash));
        }
        else {
            // Get the directory (prefix) and filepath (remainder) from the blob hash string.
            std::string hash_prefix = blob_hash.substr(0, 2);
            std::string hash_noprefix = blob_hash.substr(2);

            std::filesystem::path blob_path = vcs_dir / "objects" / hash_prefix / hash_noprefix;
            index_file_paths.push_back(file.string());
            index_file_blobs.push_back(blob_path.string());

            if (! std::filesystem::exists(vcs_dir / "objects"/ hash_prefix)) {
                std::filesystem::create_directory(vcs_dir / "objects" / hash_prefix);
            }
            compressFile(file, blob_path);
        }
    }
}

void vcs_status() {
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

int main(int argc, char **argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    if (args.empty()) {
        err_out("No argument given\n");
        return 0;
    }

    // Load config, if applicable.
    vcs_config read_config = load_config(vcs_dir);
    user_name = read_config.user_name;
    user_email = read_config.user_email;

    std::string base_cmd = args.at(0);

    if (base_cmd == "-h" || base_cmd == "help") {
        help_menu();
    }
    else if (base_cmd == "-v" || base_cmd == "version") {
        version_menu();
    }
    else if (base_cmd == "--config") {
        handle_config(args);
    }
    else if (base_cmd == "init") {
        init_vcs(vcs_inited, vcs_dir, user_name, user_email);
    }
    else if (base_cmd == "status") {
        if (! vcs_inited) {
            err_out("Cannot check status - VCS not initialized yet");
        }

        vcs_status();
    }
    else if (base_cmd == "add") {
        if (! vcs_inited) {
            err_out("Cannot add files - VCS not initialized yet");
        }
        if (args.size() > 1) {
            for (size_t i = 1; i < args.size(); i++) {
                // If they pass '.', we use current_path(), otherwise the specific path
                std::filesystem::path p = (args[i] == ".")
                    ? std::filesystem::current_path()
                    : std::filesystem::absolute(std::filesystem::path(args[i]));

                add_file(p);
            }
            make_index(vcs_dir / "index", user_name, user_email, index_file_paths, index_file_blobs);
        }
        else {
            err_out("'add' requires one or more arguments");
        }
    }
    else {
        err_out(std::format("No such argument {}", base_cmd));
    }

    // Store final state of configuration variables.
    vcs_config final_config = {user_name, user_email};
    store_config(vcs_dir, final_config);

    // Exit successfully.
    return 0;
}