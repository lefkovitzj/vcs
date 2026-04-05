#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <format>

#include "config.h"
#include "io.h"
#include "hashing.h"
#include "compress.h"

const float VCS_VERSION_NUM = 0.1;
const std::string VCS_SOURCE_URL = "https://github.com/lefkovitzj/vcs";

// Store the path at which the VCS data is found.
const std::filesystem::path vcs_dir = std::filesystem::current_path() / ".vcs";
bool vcs_inited = std::filesystem::is_directory(vcs_dir);
std::string user_name =  "";
std::string user_email = "";

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

    // Create the HEAD file.
    std::ofstream headFile(vcs_dir / "HEAD");
    if (headFile.is_open()) {
        headFile << "main";
    }
    else {
        err_out("Could not create head file");
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

std::string hash_blob(std::filesystem::path local_file) {
    /* Create the hash for a blob at the given path. */
    int f_size = std::filesystem::file_size(local_file);
    std::string header = std::format("blob {}\0", f_size);

    std::vector<uint8_t> blob;
    for (char c : header) {
        blob.push_back(static_cast<uint8_t>(c));
    }
    std::ifstream file(local_file, std::ios::binary);

    std::vector<uint8_t> f_bytes(f_size);
    if (file.read(reinterpret_cast<char *>(f_bytes.data()), f_size)) {
        blob.insert(blob.end(), f_bytes.begin(), f_bytes.end());
        return sha1(blob);
    }
    else {
        err_out(std::format("File {} could not be hashed as blob.", local_file.string()));
    }
    return "";
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
    else if (base_cmd == "add") {
        std::cout << "Args: ";
        std::vector<std::string> filesToAdd;
        bool addAll = false;
        if (! vcs_inited) {
            err_out("Cannot add files - VCS not initialized yet");
        }
        if (args.size() > 1) {
            for (int i = 1; i < args.size(); i++) {
                if (args[i] == ".") {
                    addAll = true;
                }
                else {
                    filesToAdd.push_back(args[i]);
                }
            }
            if (addAll) {
                {
                    for (std::filesystem::recursive_directory_iterator i("."), end; i != end; ++i) {
                        if (!is_directory(i->path())) {
                            std::cout << i->path() << "\n";
                        }
                    }
                }
            }
            else {
                for (int i=0; i<filesToAdd.size(); i++) {
                    std::cout << filesToAdd.at(i) << "\n";
                }
            }
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