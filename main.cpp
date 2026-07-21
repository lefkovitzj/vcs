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

/* Utils Imports */
#include "utils/io.h"

/* Core Imports */
#include "core/configuration.h"
#include "core/index.h"

/* Command Imports */
#include "commands/add.h"
#include "commands/config.h"
#include "commands/init.h"
#include "commands/status.h"

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
        handle_config(vcs_dir, args, user_name, user_email);
    }
    else if (base_cmd == "init") {
        init_vcs(vcs_inited, vcs_dir, user_name, user_email);
    }
    else if (base_cmd == "status") {
        if (! vcs_inited) {
            err_out("Cannot check status - VCS not initialized yet");
        }

        vcs_status(vcs_dir);
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

                add_file(vcs_dir, p, vcs_inited, index_file_paths, index_file_blobs);
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