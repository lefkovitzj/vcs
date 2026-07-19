//
// Created by lefko on 4/3/2026.
//

#ifndef VCS_COMPRESS_H
#define VCS_COMPRESS_H

void compressFile(std::filesystem::path src_file, std::filesystem::path dst_file);
void decompressFile(std::filesystem::path src_file, std::filesystem::path dst_file);

#endif //VCS_COMPRESS_H
