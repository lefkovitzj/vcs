#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>

const float VCS_VERSION_NUM = 0.1;

void err_out(std::string err_msg) {
    std::cerr << "vcs/> Error: " << err_msg;
}

void help_menu() {
    std::cout << "vcs/> Help Menu";
}
void version_menu() {
    std::cout << "vcs/> Version " << VCS_VERSION_NUM << "\n";
    std::cout << "Find the most up-to-date version of VCS at https://github.com/lefkovitzj/vcs\n";
}
void init_vcs() {
    std::cout << "vcs/> Initialize VCS\n";
    // Get the path at which to build the filestructure within the .vcs directory.
    std::filesystem::path vcs_dir = std::filesystem::current_path() / ".vcs";

    // Ensure vcs not yet init. If not, create the .vcs directory.
    if (std::filesystem::exists(vcs_dir) || std::filesystem::is_directory(vcs_dir)) {
        err_out("Could not initialize VCS - already present in this directory");
        return;
    }
    std::filesystem::create_directory(vcs_dir);

    // Create objects subfolder.
    std::filesystem::create_directory(vcs_dir / "objects");

    // Create refs/heads subfolder.
    std::filesystem::create_directories(vcs_dir / "refs/heads");

    // Create the HEAD file.
    std::ofstream headFile(vcs_dir / "HEAD");
    if (headFile.is_open()) {
        headFile << "main";
    }
    else {
        err_out("Could not create head file");
        return;
    }

    // Create the index file.
    std::ofstream vcsIndexFile(vcs_dir / "index");
    if (vcsIndexFile.is_open()) {
        vcsIndexFile << "VCS - " << VCS_VERSION_NUM << "\n";
    }
    else {
        err_out("Could not create VCS index file");
        return;
    }
    std::cout << "vcs/> VCS initialized successfully";
}

int main(int argc, char **argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }
    if (args.empty()) {
        err_out("No argument given\n");
        return 0;
    }
    std::string base_cmd = args.at(0);


    if (base_cmd == "-h" || base_cmd == "help") {
        help_menu();
    }
    else if (base_cmd == "-v" || base_cmd == "version") {
        version_menu();
    }
    else if (base_cmd == "init") {
        init_vcs();
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
                    for (std::filesystem::recursive_directory_iterator i("."), end; i != end; ++i) {
                        if (!is_directory(i->path())) {
                            std::cout << i->path() << "\n";
                        }
                    }
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