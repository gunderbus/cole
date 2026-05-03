#ifndef NODES_H
#define NODES_H

#include <string>
#include <vector>

class node {

public:

    // Constructor
    node(std::string conditional, int prev, std::vector<int> connectors);

    std::string getConditional() const;

    int getPrev() const;
    
    std::vector<int> getNext() const;

private:
    std::string conditional;
    int prev;
    std::vector<int> connectors;

};

#endif
