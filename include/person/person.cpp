#include "person.h"
#include "nodes/nodes.h"
#include "ollama/ollama.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

person::person(std::string name, std::string contextPath)
    : name(name), contextPath(contextPath) {
    loadMemory();
}

std::string person::chat(std::string input) {
    if (input.empty()) {
        return "Say something and I will respond.";
    }

    ollama agent = ollama("llama3");
    std::string response = agent.prompt(input, getNeededPrompt(input));

    learnFromConversation(input, response);

    return response;
}

std::string person::getNeededPrompt(std::string Input){
    std::ostringstream neededPrompt;

    neededPrompt << "You are talking as " << name << ".\n";
    neededPrompt << "Use this context path as the main identity/context source: " << contextPath << "\n";
    neededPrompt << "Reply naturally, and let the saved memories shape your tone and preferences.\n";

    if (!connections.empty()) {
        neededPrompt << "\nSaved memories and style notes:\n";
    }

    for (size_t i = 0; i < connections.size(); i++) {
        neededPrompt << "- " << connections[i].getConditional() << "\n";
    }

    neededPrompt << "\nCurrent user message: " << Input;

    return neededPrompt.str();
}

std::string person::getContextPath(){
    return contextPath;
}

std::string person::getConditionalPath(std::string Input){
    return getNeededPrompt(Input);
}

void person::deleteNode(int loc) {
    if (loc < 0) {
        return;
    }

    if (static_cast<size_t>(loc) < connections.size()) {
        connections.erase(connections.begin() + loc);
        saveMemory();
    }
}

void person::appendNode(node hi){
    connections.push_back(hi);
    saveMemory();
}

node person::getNode(int loc){
    if (loc < 0 || static_cast<size_t>(loc) >= connections.size()) {
        throw std::out_of_range("node location is out of range");
    }

    return connections[loc];
}

void person::loadMemory() {
    std::ifstream memoryFile(contextPath + ".memory");
    std::string memory;

    while (std::getline(memoryFile, memory)) {
        if (!memory.empty()) {
            connections.push_back(node(memory, -1, std::vector<int>{}));
        }
    }
}

void person::saveMemory() {
    std::ofstream memoryFile(contextPath + ".memory");

    for (size_t i = 0; i < connections.size(); i++) {
        memoryFile << connections[i].getConditional() << "\n";
    }
}

void person::learnFromConversation(std::string input, std::string response) {
    if (response.find("I did not get a response from Ollama") != std::string::npos) {
        return;
    }

    ollama agent = ollama("llama3");
    std::string learningPrompt =
        "Turn this conversation into one short memory or style note that will help you talk more like "
        + name +
        " in the future. Return only the note. If there is nothing useful, return NONE.\n\nUser: "
        + input +
        "\nAssistant: "
        + response;

    std::string memory = agent.prompt(learningPrompt, "You create concise chatbot memory notes.");

    if (
        memory.empty()
        || memory == "NONE"
        || memory == "None"
        || memory == "none"
        || memory.find("I did not get a response from Ollama") != std::string::npos
    ) {
        return;
    }

    connections.push_back(node(memory, -1, std::vector<int>{}));
    saveMemory();
}
