#ifndef PERSON_H
#define PERSON_H

#include <string>
#include "nodes/nodes.h"

class person {
public:
    // Constructor
    person(std::string name, std::string contextPath);

    std::string getNeededPrompt(std::string Input);
    std::string getContextPath();
    std::string getConditionalPath(std:string Input);

private:
    
    std::vector<node> connections;
    str::string contextPath;

};

#endif