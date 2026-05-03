#include "person.h"
#include "nodes/nodes.h"
#include "ollama/ollama.h"

person::person(std::string name, std::string contextPath)
    : name(name), contextPath(contextPath) {}

std::string person::getNeededPrompt(std::string Input){
    if (connections.empty()) {
        return contextPath + "\n" + Input;
    }

    ollama agent = ollama("llama3");
    std::string neededPrompt = contextPath + "\n" + Input;
    int currentIndex = 0;

    while (currentIndex >= 0 && currentIndex < static_cast<int>(connections.size())) {
        node currentNode = connections[currentIndex];

        if (!agent.getIsTrue(currentNode.getConditional())) {
            break;
        }

        neededPrompt += "\n" + currentNode.getConditional();

        std::vector<int> nextNodes = currentNode.getNext();
        if (nextNodes.empty()) {
            break;
        }

        currentIndex = nextNodes[0];
    }

    return neededPrompt + "this is the final prompt.";
}

std::string person::getContextPath(){
    return contextPath;
}

std::string person::getConditionalPath(std::string Input){

}

void person::deleteNode(int loc) {
    if (loc < 0) {
        return;
    }

    if (static_cast<size_t>(loc) < connections.size()) {
        connections.erase(connections.begin() + loc);
    }
}

void person::appendNode(node hi){
    connections.push_back(hi);
}

void person::getNode(int loc){
    return connections[loc];
}
