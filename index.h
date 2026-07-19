#ifndef VCS_INDEX_H
#define VCS_INDEX_H

void make_index(std::filesystem::path index_path, std::string user_name, std::string user_email, std::vector<std::string> file_paths, std::vector<std::string> file_blob_hashes);

std::map<std::string, std::string> get_index_as_map(std::filesystem::path index_path);

#endif //VCS_INDEX_H
