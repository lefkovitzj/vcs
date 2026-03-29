/**
* @file config.cpp
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
#include <map>

#include "config.h"
#include "io.h"

void store_config(std::filesystem::path vcs_dir, vcs_config config) {
    /* Store to the .vcs/config file. */
    std::ofstream vcsConfigFile(vcs_dir / "config");
    if (vcsConfigFile.is_open()) {
        vcsConfigFile << "[user]\n\tname = " << config.user_name << "\n\temail = " << config.user_email << "\n";
    }
    else {
        err_out("Could not create VCS index file");
    }
}

vcs_config load_config(std::filesystem::path vcs_dir) {
    /* Load from the .vcs/config file. */
    vcs_config config;
    if (std::filesystem::exists(vcs_dir / "config")) {
        std::ifstream vcsConfigFile(vcs_dir / "config");
        std::string line;

        std::map<std::string, std::string> configFileMap;
        if (vcsConfigFile.is_open()) {
            while (std::getline(vcsConfigFile, line)) {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    // Extract the data from the line.
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);

                    // Add the key, value to the map.
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);

                    configFileMap[key] = value;
                }
            }
        }
        config.user_name = configFileMap["name"];
        config.user_email = configFileMap["email"];
    }
    else {
        // No config file.
        config.user_name = "";
        config.user_email = "";
    }
    return config;
}