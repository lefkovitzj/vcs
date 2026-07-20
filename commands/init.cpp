/* @file init.cpp
* @brief Handle initialization commands for VCS.
*
* Implement the initialization functionality in reusable functions.
*
* @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
*/

#include <filesystem>
#include <fstream>

#include "../utils/io.h"

#include "../core/config.h"


void init_vcs(bool vcs_inited, std::filesystem::path vcs_dir, std::string user_name, std::string user_email) {
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
    // Check for existing .vcsno
    if (std::filesystem::exists(std::filesystem::current_path() / ".vcsno")) {
        info_out("Could not create .vcsno file - already present in this directory");
    }
    else {
        std::ofstream vcsnoFile(std::filesystem::current_path() / ".vcsno");
        if (vcsnoFile.is_open()) {
            vcsnoFile << "# VCS - Add any files you want to ignore to this file.";
        }
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
        vcsIndexFile << user_name << "\n" << user_email << "\n";
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
