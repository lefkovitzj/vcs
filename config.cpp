/**
* @file io.cpp
 * @brief System configuration utilities for VCS.
 *
 * Implement both read and write of vcs configuration
 * stored in the .vcs/config file.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <string>
#include <filesystem>
#include <fstream>
#include "io.h"

void store_config(std::filesystem::path vcs_dir, std::string user_name, std::string user_email) {
    std::ofstream vcsConfigFile(vcs_dir / "config");
    if (vcsConfigFile.is_open()) {
        vcsConfigFile << "[user]\n\tname = " << user_name << "\n\temail = " << user_email << "\n";
    }
    else {
        err_out("Could not create VCS index file");
    }
}