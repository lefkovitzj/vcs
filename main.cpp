#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <format>
#include <map>

#include "config.h"
#include "io.h"
#include "hashing.h"
#include "head.h"
#include "compress.h"
#include "index.h"

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

bool in_vcsno(std::filesystem::path file) {
    /* Parse the root .vcsno file into rules, ignoring comments. */
    static bool rules_preloaded = false;
    static std::vector<std::string> vcsno_indirs;
    static std::vector<std::string> vcsno_files;
    static std::vector<std::string> vcsno_wilds;

    if (! rules_preloaded) {
        std::ifstream vcsnoFile(std::filesystem::current_path() / ".vcsno", std::ios::binary);
        std::string line;
        if (vcsnoFile.is_open()) {
            // Read non-comment (marked by #) lines into the vcsno_rules.
            while (std::getline(vcsnoFile, line)) {
                if (! line.starts_with("#")) {
                    // Split by " ".
                    std::stringstream ss(line);
                    std::string rule;
                    while (ss >> rule) {
                        std::replace(rule.begin(), rule.end(), '\\', '/');
                        if (rule.starts_with("*")) {
                            // Wildcard rule.
                            vcsno_wilds.push_back(rule);
                        }
                        else if (rule.find("/") != std::string::npos) {
                            // Directory rule.
                            vcsno_indirs.push_back(rule);
                        }
                        else {
                            // File rule.
                            vcsno_files.push_back(rule);
                        }
                    }
                }
            }
        }
        rules_preloaded = true;
    }

    // Compare file against rules.
    std::filesystem::path rel_path = std::filesystem::relative(file, std::filesystem::current_path());
    std::string f_name = file.filename().generic_string();
    std::string f_relative_path = rel_path.generic_string();

    // File (local path) rule.
    for (std::string vcsno_f : vcsno_files) {
        // Check that the path matches the rule.
        if (vcsno_f == f_name) {
            return true;
        }
    }
    // Directory (or absolute path) rule.
    for (std::string vcsno_d : vcsno_indirs) {
        // Normalize the rule: "bin/" becomes "bin".
        std::string clean_rule = vcsno_d;
        if (!clean_rule.empty() && clean_rule.back() == '/') {
            clean_rule.pop_back();
        }

        // Match if it's the directory itself or a child of the directory.
        if (f_relative_path == clean_rule || f_relative_path.starts_with(clean_rule + "/")) {
            return true;
        }
    }
    for (std::string vcsno_w : vcsno_wilds) {
        // Remove the '*' character.
        vcsno_w.erase(0, 1);
        // Check that the path ends with the wildcard.
        if (f_relative_path.ends_with(vcsno_w)) {
            return true;
        }
    }

    // Not targeted by any rule in the .vcsno file.
    return false;
}

void init_vcs() {
    info_out("Initializing VCs...");

    // Ensure vcs not yet init. If not, create the .vcs directory.
    if (vcs_inited) {
        err_out("Could not initialize VCS - already present in this directory");
        return;
    }
    std::filesystem::create_directory(vcs_dir);

    // Create objects subfolder.
    std::filesystem::create_directory(vcs_dir / "objects");

    // Create refs/heads subfolder.
    std::filesystem::create_directories(vcs_dir / "refs/heads");

    // Create the .vcsno file.
    std::ofstream vcsnoFile(std::filesystem::current_path() / ".vcsno");
    if (vcsnoFile.is_open()) {
        vcsnoFile << "# VCS - Add any files you want to ignore to this file.";
    }

    // Create the HEAD file(s).
    std::ofstream headFile(vcs_dir / "HEAD");
    if (headFile.is_open()) {
        headFile << "ref: refs/heads/main";
    }
    else {
        err_out("Could not create head file");
        return;
    }
    std::ofstream headFileMain(vcs_dir / "refs/heads/main");
    if (headFileMain.is_open()) {
        headFileMain << "";
    }
    else {
        err_out("Could not create head file for main branch");
        return;
    }


    // Create the index file.
    std::ofstream vcsIndexFile(vcs_dir / "index");
    if (vcsIndexFile.is_open()) {
        vcsIndexFile << "VCS - " << VCS_VERSION_NUM << "\n";
    }
    else {
        err_out("Could not create VCS index file");
        return;
    }

    // Create the config file.
    vcs_config init_config = {user_name, user_email};
    store_config(vcs_dir, init_config);

    vcs_inited = true;
    info_out("VCS initialized successfully");
}

std::string hash_file_blob(std::filesystem::path local_file) {
    /* Create the hash for a blob at the given path. */
    uintmax_t f_size = std::filesystem::file_size(local_file);
    std::string header = std::format("blob {}", f_size) + '\0';

    std::vector<uint8_t> blob;
    for (char c : header) {
        blob.push_back(static_cast<uint8_t>(c));
    }
    std::ifstream file(local_file, std::ios::binary);
    std::vector<uint8_t> f_bytes(f_size);

    if (f_size > 0) {
        if (!file.read(reinterpret_cast<char *>(f_bytes.data()), f_size)) {
            err_out(std::format("File {} could not be read.", local_file.string()));
            return "";
        }
    }
    blob.insert(blob.end(), f_bytes.begin(), f_bytes.end());
    return sha1(blob);

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
        init_vcs();
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
                    : std::filesystem::path(args[i]);

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