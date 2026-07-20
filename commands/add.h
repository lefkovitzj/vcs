//
// Created by lefko on 7/19/2026.
//

#ifndef VCS_ADD_H
#define VCS_ADD_H

void add_file(std::filesystem::path vcs_dir, std::filesystem::path file, bool vcs_inited, std::vector<std::string>& index_file_paths, std::vector<std::string>& index_file_blobs);

#endif //VCS_ADD_H
