/**
* @file hashing.cpp
 * @brief Hashing utilities for the VCS.
 *
 * Implement a SHA-1 hash algorithm for use in the VCS
 * tool. This is a utility used in computing the filepath
 * at which to save ojects.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

std::vector<uint8_t> sha1_pad(std::string input_str) {
 uint64_t len_bits = input_str.length() * 8;

 // Copy input str into buffer.
 std::vector<uint8_t> buffer(input_str.begin(), input_str.end());

 // Add '1' bit to the buffer (10000000 byte = 128 in base_10).
 buffer.push_back(128);

 // Add '0' bits until buffer length has final chunk of length 448 (56*8).
 while ((buffer.size() % 64) != 56) {
  buffer.push_back(0);
 }
 for (int i=7; i>0; i--) {
  // Get the 8-bit (1-byte) chunk to gradually form the 64-bit (8-byte) big endian int.
  buffer.push_back(len_bits >> (i*8) & 0xFF);
 }
 return buffer;
}

std::string sha1(std::string input_str) {
 // Step 1: Pad the input and create the byte buffer.
 std::vector<uint8_t> buffer = sha1_pad(input_str);

 // Step 2: Define constants
 int h0 = 0x67452301;
 int h1 = 0xEFCDAB89;
 int h2 = 0x98BADCFE;
 int h3 = 0x10325476;
 int h4 = 0xC3D2E1F0;

 int t0_20 = 0x5A827999;
 int t21_40 = 0x6ED9EBA1;
 int t41_60 = 0x8F1BBCDC;
 int t61_80 = 0xCA62C1D6;
}
