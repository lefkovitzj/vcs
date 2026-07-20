/* @file vcsno.cpp
* @brief Handle vcsno operations for VCS.
*
* Implement the vcsno functionality in reusable functions.
*
* @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
*/

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

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

    // Disallow self-tracking of the .vcs directory.
    if (f_relative_path == ".vcs" || f_relative_path.starts_with(".vcs/")) {
        return true;
    }

    // Not targeted by any rule in the .vcsno file.
    return false;
}
