#ifndef OLLAMA_H
#define OLLAMA_H

#include <string>

class ollama {
public:
    // Constructor
    ollama(std::string modelName);

    // Method to "ask" the AI
    std::string prompt(std::string userInput, std::string context);
    
    bool getIsTrue(std::string isTrue);
    

private:
    std::string model;
    std::string endpoint = "http://localhost:11434/api/generate";
};

#endif
