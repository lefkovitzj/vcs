/**
* @file config.cpp
 * @brief System configuration command for VCS.
 *
 * Implement both command for read and write, wrapping functionality from core/config.cpp.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <format>
#include <string>
#include <vector>

#include "../utils/io.h"
#include "../core/configuration.h"

void handle_config(std::filesystem::path vcs_dir, std::vector<std::string> args, std::string& user_name, std::string& user_email) {
    if (args.size() == 2) {
        // Display current config.
        if (args.at(1) =="user.name" ) {
            info_out(std::format("user.name = {}", user_name), true);
        }
        if (args.at(1) =="user.email" ) {
            info_out(std::format("user.email = {}", user_email), true);
        }
    }
    else if (args.size() == 3) {
        if (args.at(1) == "user.name") {
            user_name = args.at(2);
        }
        if (args.at(1) == "user.email") {
            user_email = args.at(2);
        }
        store_config(vcs_dir, {user_name, user_email});
    }
}
