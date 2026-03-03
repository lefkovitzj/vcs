#include <iostream>
#include <string>
#include <sstream>
#include <vector>

const float VCS_VERSION_NUM = 0.1;
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
std::vector<std::string> command_prompt() {
    std::string command;
    std::vector<std::string> args;

    // Prompt and read command.
    std::cout << "\\> ";
    std::getline(std::cin, command);

    // Read args from command.
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

int main() {
    // TIP Press <shortcut actionId="RenameElement"/> when your caret is at the <b>lang</b> variable name to see how CLion can help you rename it.
    std::cout << "VCS - an open source version control system by Joseph Lefkovitz\n";

    while (true) {
        std::vector<std::string> args = command_prompt();
        if (! args.empty()) {
            if (args.at(0) == "help") {
                help_menu();
            }
            else if (args.at(0) == "version") {
                version_menu();
            }
            else if (args.at(0) == "exit") {
                break;
            }
        }
    }
    return 0;
    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}