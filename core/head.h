#ifndef VCS_HEAD_H
#define VCS_HEAD_H

#include <map>

std::map<std::string, std::string> get_head_as_map(std::filesystem::path vcs_dir);
std::string get_ref(std::filesystem::path vcs_dir);
std::string get_head_commit_hash(std::filesystem::path vcs_dir);

#endif //VCS_HEAD_H
