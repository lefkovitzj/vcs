/**
* @file compress.cpp
 * @brief Handle compression for VCS.
 *
 * Implement huffman coding for compression of objects.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <bitset>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <map>

#include "io.h"

struct Node {
    // Store character and frequency.
    char ch;
    int fr;
    // Unique ID to break comparison ties
    uint64_t sequence_id;
    // Store child nodes.
    Node *left;
    Node *right;

    // Constructors.
    Node(char ch, int fr, uint64_t id)
        : ch(ch), fr(fr), sequence_id(id), left(nullptr), right(nullptr) {}
    Node(char ch, int fr, uint64_t id, Node* left, Node* right)
        : ch(ch), fr(fr), sequence_id(id), left(left), right(right) {}
};

struct compareNodes {
    bool operator()(Node *left, Node *right) {
        // Compare by frequency.
        if (left->fr != right->fr) {
            return left->fr > right->fr;
        }
        // If frequencies are tied, consistently give lower ID higher priority.
        return left->sequence_id > right->sequence_id;
    }
};

void getCodes(Node* root, std::string codeStr, std::map<char, std::string>& huffmanCode) {
    /* Get all the Huffman codes from the root of a Huffman tree and insert into the map. */
    // Handle null case.
    if (root == nullptr) {
        return;
    }

    // Catch leaf case.
    if (!root->left && !root->right) {
        huffmanCode[root->ch] = codeStr;
    }

    // Recursively generate codes for left and right Huffman subtrees.
    getCodes(root->left, codeStr + "0", huffmanCode);
    getCodes(root->right, codeStr + "1", huffmanCode);
}

std::map<char, int> buildHuffmanFrequencyMap(std::string input_str) {
    /* Build a Huffman frequency map from a string. */
    // Store all characters in a frequency map.
    std::map<char, int> fr;
    for (char ch :input_str) {
        fr[ch]++;
    }
    return fr;
}

Node* buildHuffmanTree(std::map<char, int> fr) {
    /* Create a Huffman tree from a frequency map. */
    // Create and store all characters in a priority queue.
    std::priority_queue<Node*, std::vector<Node*>, compareNodes> pq;
    uint64_t id_counter = 0;
    for (auto pair : fr) {
        // Increase the ID counter and add the node to the queue.
        id_counter++;
        pq.push(new Node(pair.first, pair.second, id_counter));
    }

    // Process all nodes in the priority queue.
    while (pq.size() != 1) {
        Node* l = pq.top();
        pq.pop();
        Node* r = pq.top();
        pq.pop();

        // Create the internal node for these two child nodes (and increase the ID counter).
        int f_sum = l->fr + r->fr;
        id_counter++;
        pq.push(new Node('\0', f_sum, id_counter, l, r));
    }

    // Return the root of the Huffman tree.
    Node* root = pq.top();
    return root;
}

std::string huffmanEncode(std::string input_str, Node* huffmanTree) {
    /* Encode a string based on a Huffman tree. */
    std::map<char, std::string> huffmanCode;

    // Get the Huffman codes.
    getCodes(huffmanTree, "", huffmanCode);
    std::string encoded = "";

    // Use the huffman codes to encode the string.
    for (char ch : input_str) {
        encoded += huffmanCode[ch];
    }

    return encoded;
}

std::string huffmanDecode(std::string encoded_str, Node* huffmanTreeRoot) {
    /* Decode a string based on a Huffman tree. */
    // Begin Huffman tree traversal from the root.
    Node* current = huffmanTreeRoot;

    // Store the complete string.
    std::string decoded_str = "";
    for (char bit: encoded_str) {
        // Travers left Huffman subtree.
        if (bit == '0') {
            current = current->left;
        }
        // Traverse right Huffman subtree.
        else {
            current = current->right;
        }
        // Leaf of Huffman tree reached - record character.
        if (!current->left && !current->right) {
            decoded_str += current->ch;
            current = huffmanTreeRoot;
        }
    }
    return decoded_str;
}

struct binConversion {
    // Store both a vector of bytes and the number of padding bits added to the last byte.
    std::vector<uint8_t> bytes;
    int padding_bits;
};

binConversion convertBinStringToBytes(std::string input_str) {
    /* Convert a string representation of the binary to an array of bytes. */
    std::vector<uint8_t> bytes;
    uint8_t current_byte = 0;
    int processed_bits = 0;

    // Process a byte at a time.
    for (char ch : input_str) {
        // Shift left one bit.
        current_byte <<= 1;
        if (ch == '1') {
            current_byte |= 1;
        }
        else if (ch != '0') {
            err_out("Invalid Binary String - terminating early.");
            return binConversion(bytes, 0);
        }
        // Mark another bit processed.
        processed_bits++;

        // If we have processed a full byte, move on to the next byte.
        if (processed_bits == 8) {
            bytes.push_back(current_byte);
            current_byte = 0;
            processed_bits = 0;
        }
    }

    // Handle any padding bits to fill the final byte.
    int padding_bits = 0;
    if (processed_bits < 8 && processed_bits != 0) {
        // Shift left to fill the byte.
        current_byte <<= (8-processed_bits);
        padding_bits = 8-processed_bits;
        bytes.push_back(current_byte);
    }
    return binConversion(bytes, padding_bits);
}

std::string convertBytestToBinString(std::vector<uint8_t> input_bytes) {
    /* Convert an array of bytes to a string representation of the binary. */
    std::string binString = "";
    for (uint8_t b : input_bytes) {
        for (int i = 7; i >= 0; i--) {
            bool lastBit = (b >> i) & 1;
            if (lastBit) {
                binString += "1";
            }
            else {
                binString += "0";
            }
        }
    }
    return binString;
}

void compressFile(std::filesystem::path src_file, std::filesystem::path dst_file) {
    /* Compress a source file into a destination file using Huffman coding. */
    if (std::filesystem::exists(src_file)) {
        std::ifstream srcFile(src_file, std::ios::binary);
        if (srcFile.is_open()) {
            std::string lines((std::istreambuf_iterator<char>(srcFile)),
                                   std::istreambuf_iterator<char>());
            srcFile.close();
            std::map<char, int> fr = buildHuffmanFrequencyMap(lines);
            Node* huffmanTree = buildHuffmanTree(fr);
            std::string encoded = huffmanEncode(lines, huffmanTree);
            binConversion encoded_bin = convertBinStringToBytes(encoded);

            std::ofstream dstFile(dst_file, std::ios::binary);
            if (dstFile.is_open()) {
                // Write map size.
                uint32_t mapSize = static_cast<uint32_t>(fr.size());
                dstFile.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

                // Write padding bits.
                dstFile.write(reinterpret_cast<const char*>(&encoded_bin.padding_bits), sizeof(encoded_bin.padding_bits));

                // Write huffman tree data.
                for (auto const& [ch, count] : fr) {
                    dstFile.put(ch);
                    dstFile.write(reinterpret_cast<const char*>(&count), sizeof(count));
                }

                // Write encoded data.
                dstFile.write(reinterpret_cast<const char*>(encoded_bin.bytes.data()), encoded_bin.bytes.size());
            }
            else {
                err_out("Could not open destination file.");
            }
        }
        else {
            err_out("Could not open source file.");
        }
    }
    else {
        err_out("Source file does not exist.");
    }
}

void decompressFile(std::filesystem::path src_file, std::filesystem::path dst_file) {
    /* Decompress a source file into a destination file using Huffman coding. */
    if (std::filesystem::exists(src_file)) {
        std::ifstream srcFile(src_file, std::ios::binary);
        if (srcFile.is_open()) {
            // Read map size.
            uint32_t mapSize = 0;
            srcFile.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

            // Read the padding bits from the top of the file.
            int padding_bits_value = 0;
            srcFile.read(reinterpret_cast<char*>(&padding_bits_value), sizeof(padding_bits_value));

            // Read the frequency map back in.
            std::map<char, int> fr;
            for (uint32_t i = 0; i < mapSize; i++) {
                char ch;
                int count;

                srcFile.get(ch);
                srcFile.read(reinterpret_cast<char*>(&count), sizeof(count));
                fr[ch] = count;
            }
            // Use that read frequency map to regenerate the Huffman tree.
            Node* huffmanTree = buildHuffmanTree(fr);

            // Read the rest of the file as binary.
            std::vector<uint8_t> compressedBytes((std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());

            // Convert to a binary string.
            std::string binString = convertBytestToBinString(compressedBytes);

            // Remove padding.
            if (padding_bits_value > 0 && binString.size() >= padding_bits_value) {
                binString.erase(binString.size() - padding_bits_value);
            }

            // Decode the binary string using the Huffman tree.
            std::string decoded = huffmanDecode(binString, huffmanTree);

            // Write the decoded data to the dstFile.
            std::ofstream dstFile(dst_file, std::ios::binary);
            if (dstFile.is_open()) {
                dstFile<<decoded;
                dstFile.close();
            }
            else {
                err_out("Could not open destination file.");
            }
        }
        else {
            err_out("Could not open source file.");
        }
    }
    else {
        err_out("Source file does not exist.");
    }
}