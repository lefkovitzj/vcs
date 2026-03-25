#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>

const float VCS_VERSION_NUM = 0.1;

void err_out(std::string err_msg) {
    std::cerr << "vcs/> Error: " << err_msg;
}

void help_menu() {
    std::cout << "VCS - Help Menu";
}
void version_menu() {
    std::cout << "VCS - Version " << VCS_VERSION_NUM << "\n";
    std::cout << "Find the most up-to-date version of VCS at https://github.com/lefkovitzj/vcs\n";
}

int main(int argc, char **argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    if (args.empty()) {
        err_out("No argument given");
        return 0;
    }
    std::string base_cmd = args.at(0);


    if (base_cmd == "-h" || base_cmd == "help") {
        help_menu();
    }
    else if (base_cmd == "-v" || base_cmd == "version") {
        version_menu();
    }
    else if (base_cmd == "add") {
        std::cout << "Args: ";
        std::vector<std::string> filesToAdd;
        bool addAll = false;
        if (args.size() > 1) {
            for (int i = 1; i < args.size(); i++) {
                if (args[i] == ".") {
                    addAll = true;
                }
                else {
                    filesToAdd.push_back(args[i]);
                }
            }
            if (addAll) {
                {
                    for (std::filesystem::recursive_directory_iterator i("."), end; i != end; ++i)
                        if (!is_directory(i->path()))
                            std::cout << i->path() << "\n";
                }
            }
            else {
                for (int i=0; i<filesToAdd.size(); i++) {
                    std::cout << filesToAdd.at(i) << "\n";
                }
            }
        }
        else {
            err_out("'add' requires one or more arguments");
        }
    }
    std::cout << "\n";
    return 0;
}