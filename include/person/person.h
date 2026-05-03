#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <vector>
#include "nodes/nodes.h"

class person {
public:
    // Constructor
    person(std::string name, std::string contextPath);

    std::string chat(std::string input);
    std::string viewFlowchart();
    std::string getNeededPrompt(std::string Input);
    std::string getContextPath();
    std::string getConditionalPath(std::string Input);
    void deleteNode(int loc);
    void appendNode(node hi);
    node getNode(int loc);

private:
    void loadMemory();
    void saveMemory();
    void learnFromUserMessage(std::string input);
    std::vector<node> getRelevantMemories(std::string input);
    std::vector<node> getRecentMemories();
    std::string getBroadVoiceProfile();
    
    std::string name;
    std::vector<node> connections;
    std::string contextPath;
    std::vector<node> connectionsPath;

};

#endif
