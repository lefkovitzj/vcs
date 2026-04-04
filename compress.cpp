/**
* @file compress.cpp
 * @brief Handle compression for VCS.
 *
 * Implement huffman coding for compression of objects.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <format>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>

struct Node {
    /* Store a node in the Huffman tree. */
    char ch;
    int fr;
    Node *left;
    Node *right;
    Node(char ch, int fr) : ch(ch), fr(fr), left(nullptr), right(nullptr) {}
    Node(char ch, int fr, Node* left, Node* right) : ch(ch), fr(fr), left(left), right(right) {}
};

struct compareNodes {
    /* Compare two nodes */
    bool operator()(Node *left, Node *right) {
        return left->fr > right->fr;
    }
};

void getCodes(Node* root, std::string codeStr, std::unordered_map<char, std::string>& huffmanCode) {
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

Node* buildHuffmanTree(std::string input_str) {
    // Store all characters in a frequency map.
    std::unordered_map<char, int> fr;
    for (char ch :input_str) {
        fr[ch]++;
    }

    // Create and store all characters in a priority queue.
    std::priority_queue<Node*, std::vector<Node*>, compareNodes> pq;
    for (auto pair : fr) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Process all nodes in the priority queue.
    while (pq.size() != 1) {
        Node* l = pq.top();
        pq.pop();
        Node* r = pq.top();
        pq.pop();

        int f_sum = l->fr + r->fr;
        pq.push(new Node('\0', f_sum, l, r));
    }

    // Return the root of the Huffman tree.
    Node* root = pq.top();
    return root;
}

std::string huffmanEncode(std::string input_str, Node* huffmanTree) {
    std::unordered_map<char, std::string> huffmanCode;

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