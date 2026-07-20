//
// Created by lefko on 7/19/2026.
//

#include <filesystem>
#include <format>
#include <vector>

#include "../core/vcsno.h"
#include "../core/blob.h"
#include "../utils/io.h"
#include "../utils/compress.h"

void add_file(std::filesystem::path vcs_dir, std::filesystem::path file, bool vcs_inited, std::vector<std::string>& index_file_paths, std::vector<std::string>& index_file_blobs) {
    if (! vcs_inited) {
        err_out("Cannot add file - VCS not initialized yet");
    }
    if (! std::filesystem::exists(file)) {
        err_out(std::format("File {} does not exist", file.string()));
        return;
    }
    // Check that the file is not a part of the .vcs data.
    if (std::filesystem::exists(vcs_dir) && std::filesystem::equivalent(file, vcs_dir)) {
        return;
    }
    if (file.string().find(".vcs") != std::string::npos) {
        return;
    }

    // Check that the file is not set to ignore based on the .vcsno file.
    if (in_vcsno(file)) {
        return;
    }
    if (std::filesystem::is_directory(file)) {
        // Handle directories.
        for (const auto& entry : std::filesystem::directory_iterator(file)) {
            add_file(vcs_dir, entry.path(), vcs_inited, index_file_paths, index_file_blobs);
        }
    }
    else {
        // Handle files.
        std::string blob_hash = hash_file_blob(file);
        if (blob_hash.empty()) {
            err_out(std::format("{}",blob_hash));
        }
        else {
            // Get the directory (prefix) and filepath (remainder) from the blob hash string.
            std::string hash_prefix = blob_hash.substr(0, 2);
            std::string hash_noprefix = blob_hash.substr(2);

            std::filesystem::path blob_path = vcs_dir / "objects" / hash_prefix / hash_noprefix;
            index_file_paths.push_back(file.string());
            index_file_blobs.push_back(blob_path.string());

            if (! std::filesystem::exists(vcs_dir / "objects"/ hash_prefix)) {
                std::filesystem::create_directory(vcs_dir / "objects" / hash_prefix);
            }
            compressFile(file, blob_path);
        }
    }
}
