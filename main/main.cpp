#include <iostream>
#include <string>

#include "person/person.h"

int main() {
    person cole("Cole", "cole_context");
    std::string input;

    std::cout << "Talk to Cole. Type quit to stop." << std::endl;

    while (true) {
        std::cout << "\nYou: ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "quit" || input == "exit") {
            break;
        }

        std::cout << "\nCole: " << cole.chat(input) << std::endl;
    }
    
    return 0;
}
