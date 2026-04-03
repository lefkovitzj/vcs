/**
* @file hashing.cpp
 * @brief Hashing utilities for the VCS.
 *
 * Implement a SHA-1 hash algorithm for use in the VCS
 * tool. This is a utility used in computing the filepath
 * at which to save objects.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <bitset>
#include <cstdint>
#include <format>
#include <iomanip>
#include <string>
#include <vector>

/* Important Context:
 * Wikipedia article on SHA-1 standard: https://en.wikipedia.org/wiki/SHA-1
 *
 * Big Endian: Bitwise mirrored order of Little Endian data (the default on most modern systems).
 * (value << n) -> Shift value left by n bits, replace vacant space with 0s.
 * (value >> n) -> Shift value right by n bits, replace vacant space with 0s.
 * (value0 | value1) -> bitwise OR
 * (value0 ^ value1) -> bitwise XOR
 * (value0 & value1) -> bitwise AND
 */

uint32_t rotateLeftN(uint32_t value, unsigned int n) {
    /* Bitwise rotation left by n bits. */
    // (value << n) -> Shift value left by n bits, replace vacant space with 0s.
    // (value >> (32-n) -> Shift value right by 32-n bits, replace vacant space with 0s (rotated bits are moved to end).
    // | -> bitwise or, so the 0s from both shifts are replaced by the non-zero content from the other shift.
    return (value << n) | (value >> (32 - n));
}

std::vector<uint8_t> sha1_pad(std::string input_str) {
    /* The initial padding phase of the SHA-1 hash algorithm. */
    uint64_t len_bits = input_str.length() * 8;

    // Copy input str into buffer.
    std::vector<uint8_t> buffer(input_str.begin(), input_str.end());

    // Add '1' bit to the buffer (10000000 byte = 128 in base_10).
    buffer.push_back(128);

    // Add '0' bits until buffer length has final chunk of length 448 (56*8).
    while ((buffer.size() % 64) != 56) {
        buffer.push_back(0);
    }
    // Get the 8-bit (1-byte) chunks to gradually form the 64-bit (8-byte) Big-Endian int.
    for (int i = 7; i >= 0; i--) {
        buffer.push_back(len_bits >> (i * 8) & 0xFF);
    }
    return buffer;
}

std::string sha1_digest_to_string(uint32_t hash_digest[5]) {
    /* The final hexidecimal representation of the digest for the SHA-1 hash algorithm. */
    std::stringstream return_stream;

    // Output to the stream as hex, padded with 0s.
    return_stream << std::hex << std::setfill('0');

    // Output each of the 5 32-bit chunks to cover the entier 160-bit big-endian hash value.
    for (int i=0; i < 5; i++) {
        return_stream << std::setw(8) << hash_digest[i];
    }
    return return_stream.str();
}

std::string sha1(std::string input_str) {
    // Step 1: Pad the input and create the byte buffer.
    std::vector<uint8_t> buffer = sha1_pad(input_str);

    // Define constants - specified by SHA-1 Standard
    // State variables h0-h4.
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xEFCDAB89;
    unsigned int h2 = 0x98BADCFE;
    unsigned int h3 = 0x10325476;
    unsigned int h4 = 0xC3D2E1F0;

    // K for round index slices.
    unsigned int t0_20 = 0x5A827999;
    unsigned int t21_40 = 0x6ED9EBA1;
    unsigned int t41_60 = 0x8F1BBCDC;
    unsigned int t61_80 = 0xCA62C1D6;

    // Iteratively process 512-bit chunks.
    for (int offset = 0; offset < buffer.size(); offset += 64) {
        // Create an 80-byte "word".
        uint32_t W[80];

        // Copy 16 32-bit big-endian words
        for (int t = 0; t < 16; t++) {
            W[t] = (buffer[offset + t * 4] << 24) |
                (buffer[offset + t * 4 + 1] << 16) |
                (buffer[offset + t * 4 + 2] << 8) |
                (buffer[offset + t * 4 + 3]);
        }
        // Expand for W_16 through W_80.
        for (int t = 16; t < 80; t++) {
            W[t] = rotateLeftN(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);
        }

        // Store the hash values.
        unsigned int A = h0;
        unsigned int B = h1;
        unsigned int C = h2;
        unsigned int D = h3;
        unsigned int E = h4;

        // Initialize the loop values.
        int k;
        int f;
        // Main Loop.
        for (int i = 0; i < 80; i++) {
            // Iterations 20 through 39.
            if (i < 20) {
                // Run the specified bitwise operations to update the loop values.
                f = (B & C) | ((~B) & D);
                k = t0_20;
            }
            // Iterations 0 through 19.
            else if (i < 40) {
                // Run the specified bitwise operations to update the loop values.
                f = (B ^ C ^ D);
                k = t21_40;
            }
            // Iterations 40 through 59.
            else if (i < 60) {
                // Run the specified bitwise operations to update the loop values.
                f = ((B & C) | (B & D) | (C & D));
                k = t41_60;
            }
            // Iterations 60 through 79.
            else {
                // Run the specified bitwise operations to update the loop values.
                f = (B ^ C ^ D);
                k = t61_80;
            }

            // Update the hash values using the old hash values and the computed loop values.
            uint32_t temp = rotateLeftN(A, 5) + f + E + k + W[i];
            E = D;
            D = C;
            C = rotateLeftN(B, 30);
            B = A;
            A = temp;
        }
        // Update the state variables following the processing of each chunk.
        h0 = A + h0;
        h1 = B + h1;
        h2 = C + h2;
        h3 = D + h3;
        h4 = E + h4;
    }

    // Compute the final 160-bit hash digest as a 5-entry array of 32-bit integers.
    uint32_t hash_digest[5];
    hash_digest[0] = h0;
    hash_digest[1] = h1;
    hash_digest[2] = h2;
    hash_digest[3] = h3;
    hash_digest[4] = h4;

    // Return the hash digest as a hexidecimal string.
    return sha1_digest_to_string(hash_digest);
}
