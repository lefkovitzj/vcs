#ifndef VCS_CONFIG_H
#define VCS_CONFIG_H
#include <filesystem>
#include <string>

struct vcs_config {
    std::string user_name;
    std::string user_email;
};

void store_config(std::filesystem::path vcs_dir, vcs_config config);

vcs_config load_config(std::filesystem::path vcs_dir);

#endif //VCS_CONFIG_H
