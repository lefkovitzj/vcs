/**
* @file io.cpp
 * @brief Handle I/O utilities for VCS.
 *
 * Implement both user-facing and internal Input/Output
 * functionality in reusable functions.
 *
 * @author Joseph Lefkovitz (httsp://github.com/lefkovitzj)
 */

#include <string>
#include <iostream>

void err_out(std::string err_msg) {
    /* Output an error from VCS. */
    std::cerr << "vcs/> Error: " << err_msg << "\n";
}
void info_out(std::string info_msg, bool no_prefix=false) {
    /* Output info from VCS. */
    if (no_prefix) {
        std::cout << info_msg << "\n";
    } else {
        std::cout << "vcs/> " << info_msg << "\n";
    }
}
