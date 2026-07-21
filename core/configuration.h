#ifndef VCS_CONFIGURATION_H
#define VCS_CONFIGURATION_H
#include <filesystem>
#include <string>

struct vcs_config {
    std::string user_name;
    std::string user_email;
};

void store_config(std::filesystem::path vcs_dir, vcs_config config);

vcs_config load_config(std::filesystem::path vcs_dir);

#endif //VCS_CONFIGURATION_H
