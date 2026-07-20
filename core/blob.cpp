/* @file blob.cpp
* @brief Handle blob operations for VCS.
*
* Implement the blob functionality in reusable functions.
*
* @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
*/

#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>

#include "../utils/io.h"
#include "../utils/hashing.h"

std::string hash_file_blob(std::filesystem::path local_file) {
    /* Create the hash for a blob at the given path. */
    uintmax_t f_size = std::filesystem::file_size(local_file);
    std::string header = std::format("blob {}", f_size) + '\0';

    std::vector<uint8_t> blob;
    for (char c : header) {
        blob.push_back(static_cast<uint8_t>(c));
    }
    std::ifstream file(local_file, std::ios::binary);
    std::vector<uint8_t> f_bytes(f_size);

    if (f_size > 0) {
        if (!file.read(reinterpret_cast<char *>(f_bytes.data()), f_size)) {
            err_out(std::format("File {} could not be read.", local_file.string()));
            return "";
        }
    }
    blob.insert(blob.end(), f_bytes.begin(), f_bytes.end());
    return sha1(blob);
}

