#include <iostream>
#include <string>
#include <sstream>
#include <vector>

const float VCS_VERSION_NUM = 0.1;

std::vector<std::string> parse_command(std::string command) {
    // Read args from command.
    std::vector<std::string> args;
    std::istringstream iss(command);
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    return args;
}

void help_menu() {
    std::cout << "VCS - Help Menu";
}
void version_menu() {
    std::cout << "VCS - Version" << VCS_VERSION_NUM << "\n";
    std::cout << "Find the most up-to-date version of VCS at https://github.com/lefkovitzj/vcs\n";
}

int main(int argc, char **argv) {
    std::string command_str;
    for (int i = 1; i < argc; i++) {
        command_str = argv[i];
    }
    std::vector<std::string> args = parse_command(command_str);

    if (! args.empty() and args.size() == 1) {
        if (args.at(0) == "help") {
            help_menu();
        }
        else if (args.at(0) == "version") {
            version_menu();
        }
        else if (args.at(0) == "exit") {
            // Terminate the program.
            return 0;
        }
    }
    std::cout << "\n";
    return 0;
}