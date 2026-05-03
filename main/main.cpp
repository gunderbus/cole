#include <iostream>
#include <string>

#include "person/person.h"

namespace {
std::string joinArgs(int argc, char* argv[], int start) {
    std::string joined;

    for (int i = start; i < argc; i++) {
        if (!joined.empty()) {
            joined += " ";
        }

        joined += argv[i];
    }

    return joined;
}

void runTerminalTrainingMode(person& cole) {
    std::string input;

    std::cout << "Terminal training mode. Type quit to stop. Type /flowchart to view memory nodes." << std::endl;

    while (true) {
        std::cout << "\nYou: ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "quit" || input == "exit") {
            break;
        }

        if (input == "/flowchart") {
            std::cout << "\n" << cole.viewFlowchart() << std::endl;
            continue;
        }

        std::cout << "\nCole: " << cole.chat(input) << std::endl;
    }
}
}

int main(int argc, char* argv[]) {
    person cole("Cole", "cole_context");

    if (argc >= 2) {
        std::string mode = argv[1];

        if (mode == "--chat-readonly") {
            std::cout << cole.chatReadOnly(joinArgs(argc, argv, 2)) << std::endl;
            return 0;
        }

        if (mode == "--flowchart") {
            std::cout << cole.viewFlowchart() << std::endl;
            return 0;
        }

        if (mode == "--terminal") {
            runTerminalTrainingMode(cole);
            return 0;
        }
    }

    runTerminalTrainingMode(cole);
    
    return 0;
}
